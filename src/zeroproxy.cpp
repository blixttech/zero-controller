#include "zeroproxy.hpp"

#include <qcoapoption.h>
#include <qcoapreply.h>
#include <qcoaprequest.h>
#include <qglobal.h>
#include <qpoint.h>
#include <qwt_plot_curve.h>

#include <QFinalState>
#include <QState>
#include <QUrlQuery>
#include <QVariant>
#include <QtEndian>
#include <string>

#include "common.hpp"
#include "zc_messages.pb.h"

#ifdef USE_WINSOCK2
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

namespace zero {

ZeroProxy::ZeroProxy(const QUrl& url, const QString& uuid, const QString& hardwareVersion,
                     const QString& macAddress, uint32_t updateIntervalVal, bool new_protocol,
                     bool pullStatusUpdate, QObject* parent)
    : QObject(parent),
      coapClient(QtCoap::SecurityMode::NoSecurity),
      observerReply(nullptr),
      proxyState(),
      liveTimer(),
      state_(ConnectionState::Offline),  // toggleTimer(),
      url_(url),
      uuid_(uuid),
      hardwareVersion_(hardwareVersion),
      macAddress_(macAddress),
      name_(""),
      updateInterval_(updateIntervalVal),
      closed_(false),
      ocpActivated_(false),
      otpActivated_(false),
      uptime_(0),
      vRms_(0),
      cRms_(0),
      powerInTemp_(0),
      powerOutTemp_(0),
      ambientTemp_(0),
      mcuTemp_(0),
      lastTransReason_(OpenCloseTransition::NONE),
      vSeries_(),
      cSeries_(),
      pSeries_(),
      fSeries_(),
      vCurve_("Voltage"),
      cCurve_("Current"),
      pCurve_("Power"),
      fCurve_("Frequency"),
      tripCurve_(),
      smpClient(url.host(), 1337),
      fwSlots(),
      useGetForStatus(pullStatusUpdate),
      new_protocol(new_protocol),
      hasInitState_(false),
      initState_(),
      ocp_hw_limit_(30),
      jitter_filter_(100),
      recover_delay_(10),
      recover_attempts_(5),
      autoreclose_enabled_(false)
{
    connect(&smpClient, &smp::Client::receivedGetStateOfImagesResp, this,
            &ZeroProxy::processSmpGetStateOfImagesResp);
    connect(&smpClient, &smp::Client::firmwareUpdateProgressing, this, &ZeroProxy::StatusUpdated);
    connect(&smpClient, &smp::Client::firmwareUpdateCompleted, this, &ZeroProxy::StatusUpdated);
    connect(&smpClient, &smp::Client::firmwareUpdateFailed, this, &ZeroProxy::StatusUpdated);
    initStaleDetection();

    vCurve_.setData(&vSeries_);
    cCurve_.setData(&cSeries_);
    pCurve_.setData(&pSeries_);
    fCurve_.setData(&fSeries_);
}

ZeroProxy::~ZeroProxy() { unsubscribe(); }

void ZeroProxy::subscribe()
{
    if (useGetForStatus) {
        pullStatusUpdate();
        return;
    }

    auto oUrl = url_;
    oUrl.setPath("/status");

    if (nullptr != observerReply) {
        //        coapClient.cancelObserve(observerReply);
        observerReply->disconnect();
        observerReply->deleteLater();
    }

    if (new_protocol) {
        qDebug() << "Sending new protocol subscribe request";
        ZCMessage req;
        req.mutable_req()->mutable_set_config()->mutable_config()->mutable_notif()->set_interval(
            updateInterval_);

        std::string serialised = req.SerializeAsString();
        QByteArray body(serialised.c_str(), serialised.length());

        auto creq = QCoapRequest(oUrl);
        QCoapOption option(QCoapOption::OptionName::ContentFormat, htons(NANOPB_CONTENT_FORMAT));
        creq.addOption(option);
        creq.setPayload(body);
        QCoapOption option2(QCoapOption::OptionName::Observe, 0);
        creq.addOption(option2);
        observerReply = coapClient.post(creq);

    } else {
        qDebug() << "Sending old protocol subscribe request";
        QUrlQuery params;
        params.addQueryItem("p", QString::number(updateInterval_));
        oUrl.setQuery(params);

        observerReply = coapClient.observe(oUrl);
    }

    connect(observerReply, &QCoapReply::notified, this, &ZeroProxy::onStatusUpdate);
}

void ZeroProxy::getConfig()
{
    getTripCurveConfig();

    getInitStateConfig();

    getOcpHWConfig();
}

void ZeroProxy::getTripCurveConfig()
{
    if (!new_protocol) return;

    auto oUrl = url_;
    oUrl.setPath("/config");

    // trip curve config
    ZCMessage msg;
    msg.mutable_req()->mutable_get_config()->mutable_curve()->set_direction(
        ZCFlowDirection::ZC_FLOW_DIRECTION_BACKWARD);

    QCoapReply* reply = sendCoapMsg(oUrl, msg);

    connect(reply, &QCoapReply::finished, this, &ZeroProxy::onGetTripCurveFinished);
}

void ZeroProxy::getInitStateConfig()
{
    if (!new_protocol) return;

    auto oUrl = url_;
    oUrl.setPath("/config");

    // switch position config
    ZCMessage msg;
    msg.clear_req();
    msg.mutable_req()->mutable_get_config()->mutable_ini();

    QCoapReply* replyIni = sendCoapMsg(oUrl, msg);

    connect(replyIni, &QCoapReply::finished, this, &ZeroProxy::onGetSwitchIniConfigFinished);
}

void ZeroProxy::getOcpHWConfig()
{
    if (!new_protocol) return;

    auto oUrl = url_;
    oUrl.setPath("/config");

    ZCMessage msg;
    msg.clear_req();
    msg.mutable_req()->mutable_get_config()->mutable_ocp_hw();

    QCoapReply* replyIni = sendCoapMsg(oUrl, msg);

    connect(replyIni, &QCoapReply::finished, this, &ZeroProxy::onGetOcpConfigFinished);
}

QCoapReply* ZeroProxy::sendCoapMsg(QUrl const& oUrl, ZCMessage const& msg)
{
    std::string serialised = msg.SerializeAsString();
    QByteArray body(serialised.c_str(), serialised.length());

    QCoapRequest req(oUrl);
    QCoapOption option(QCoapOption::OptionName::ContentFormat, htons(NANOPB_CONTENT_FORMAT));
    req.addOption(option);
    req.setPayload(body);

    return coapClient.post(req);
}

bool ZeroProxy::parseCoapResp(ZCMessage& msg, QCoapReply* reply)
{
    if (reply->errorReceived() != QtCoap::Error::Ok) return false;

    qDebug() << "Responsecode: " << reply->responseCode();

    auto message = reply->message();
    if (!message.hasOption(QCoapOption::OptionName::ContentFormat)) return false;

    QCoapOption format = message.option(QCoapOption::OptionName::ContentFormat);
    bool is_protob = ntohs(format.uintValue()) == NANOPB_CONTENT_FORMAT;
    if (!format.isValid() || (format.uintValue() != 0 && !is_protob)) {
        qDebug() << "Status message: Invalid content format";
        return false;
    }

    qDebug() << "Message payload " << message.payload();

    msg.ParseFromArray(reinterpret_cast<void*>(message.payload().data()),
                       message.payload().length());

    return true;
}

void ZeroProxy::onGetSwitchIniConfigFinished(QCoapReply* reply)
{
    hasInitState_ = false;
    ZCMessage msg;
    if (!parseCoapResp(msg, reply)) {
        qWarning() << "Invalid CoAP message received";
        return;
    }

    if (!(msg.res().has_config() && msg.res().config().has_ini())) {
        qWarning() << "Config reply message does not contain switch init config";
        return;
    }

    hasInitState_ = true;
    initState_ = static_cast<InitStateConfig>(msg.res().config().ini().state());

    emit ParameterChanged(uuid_, Parameter::InitialSwitchState);
}

void ZeroProxy::onGetOcpConfigFinished(QCoapReply* reply)
{
    ZCMessage msg;
    if (!parseCoapResp(msg, reply)) {
        qWarning() << "Invalid CoAP message received";
        return;
    }

    if (!(msg.res().has_config() && msg.res().config().has_ocp_hw())) {
        qWarning() << "Config reply message does not Ocp Config";
        return;
    }

    auto ocp_hw = msg.res().config().ocp_hw();
    autoreclose_enabled_ = ocp_hw.rec_en();
    ocp_hw_limit_ = ocp_hw.limit();
    jitter_filter_ = ocp_hw.filter();
    recover_delay_ = ocp_hw.rec_delay();
    recover_attempts_ = ocp_hw.rec_attempts();

    qDebug() << "OCPConfigMessage Payload - AutoReclose Enabled " << autoreclose_enabled_
             << " Recovery Delay " << recover_delay_ << " Attempts " << recover_attempts_
             << " OCP HW Limit " << ocp_hw_limit_;

    emit ParameterChanged(uuid_, Parameter::OcpHwConfig);
}

void ZeroProxy::onGetTripCurveFinished(QCoapReply* reply)
{
    ZCMessage msg;
    if (!parseCoapResp(msg, reply)) {
        qWarning() << "Invalid CoAP message received";
        return;
    }

    if (!(msg.res().has_config() && msg.res().config().has_curve())) {
        qWarning() << "Config reply message does not contain tripping curve";
        return;
    }

    auto curve = msg.res().config().curve();
    auto psize = curve.points_size();

    // in case of reload
    tripCurve_.clear();
    for (int i = 0; i < psize; ++i) {
        auto p = curve.points(i);
        tripCurve_.push_back(QPointF(p.limit(), p.duration()));
    }

    emit TripCurveUpdated();
    emit ParameterChanged(uuid_, Parameter::TripCurve);
}

void ZeroProxy::pullStatusUpdate()
{
    qDebug() << "Using get for status update";
    updateTimer.setInterval(updateInterval_);
    connect(&updateTimer, &QTimer::timeout, [&]() {
        qDebug() << "executing pull update";
        auto oUrl = url_;
        oUrl.setPath("/status");
        coapClient.get(oUrl);
    });

    connect(&coapClient, &QCoapClient::finished, [&](QCoapReply* reply) {
        if (QtCoap::ResponseCode::Content == reply->responseCode()) {
            this->onStatusUpdate(reply, reply->message());
        }
        reply->deleteLater();
    });
    updateTimer.start();
}

void setupTimer(QTimer& timer, uint32_t interval, QState* from, QAbstractState* to)
{
    timer.setInterval(interval);
    from->addTransition(&timer, &QTimer::timeout, to);
}

void ZeroProxy::initStaleDetection()
{
    qDebug() << "Configure statemachine";
    auto connect_state = new QState();
    auto smpinfo_state = new QState();
    QState* get_config_state = nullptr;

    if (new_protocol) get_config_state = new QState();

    auto subscribe_state = new QState();
    auto stale_state = new QState();
    auto live_state = new QState();
    auto shutdown_state = new QState();
    auto stopped_state = new QFinalState();

    // 1. ConnectState (also starting state)
    setupTimer(connectTimer, 10000, connect_state, connect_state);
    connect_state->addTransition(&smpClient, &smp::Client::connectionEstablished, smpinfo_state);
    connect(connect_state, &QState::entered, [&]() {
        qDebug() << "Entering connect";
        state_ = ConnectionState::Offline;
        connectTimer.start();
        smpClient.connectToHost();
    });
    connect(connect_state, &QState::exited, [&]() {
        qDebug() << "Leaving connect";
        connectTimer.stop();
    });
    proxyState.addState(connect_state);

    // 2. SmpInfo State
    setupTimer(smpTimer, 10000, smpinfo_state, smpinfo_state);
    if (new_protocol)
        smpinfo_state->addTransition(this, &ZeroProxy::ReceivedSmpInfo, get_config_state);
    else
        smpinfo_state->addTransition(this, &ZeroProxy::ReceivedSmpInfo, subscribe_state);

    connect(smpinfo_state, &QState::entered, [&]() {
        qDebug() << "Entering smpinfo";
        smpTimer.start();
        requestSmpInfo();
        state_ = ConnectionState::Connected;
    });
    connect(smpinfo_state, &QState::exited, [&]() {
        qDebug() << "Exiting smpinfo";
        smpTimer.stop();
    });
    proxyState.addState(smpinfo_state);

    // 3a. get_config state
    if (new_protocol) {
        setupTimer(getConfigTimer, 10000, get_config_state, get_config_state);
        get_config_state->addTransition(this, &ZeroProxy::TripCurveUpdated, subscribe_state);

        connect(get_config_state, &QState::entered, [&]() {
            qDebug() << "Entering get_config";
            getConfigTimer.start();
            getConfig();
        });
        connect(get_config_state, &QState::exited, [&]() {
            qDebug() << "Exiting get_config";
            getConfigTimer.stop();
        });
        proxyState.addState(get_config_state);
    }

    // 3b. the subscribe state
    setupTimer(subscribeTimer, 10000, subscribe_state, stale_state);
    subscribe_state->addTransition(this, &ZeroProxy::Alive, live_state);
    subscribe_state->addTransition(this, &ZeroProxy::SubscriptionRefused, stale_state);
    connect(subscribe_state, &QState::entered, [&]() {
        qDebug() << "Entered subscribe";
        subscribeTimer.start();
        subscribe();
    });
    connect(subscribe_state, &QState::exited, [&]() {
        qDebug() << "Exiting subscribe";
        subscribeTimer.stop();
    });
    proxyState.addState(subscribe_state);

    // 4. stale state
    QTimer* st = new QTimer(stale_state);
    setupTimer(*st, 10 * 1000, stale_state, stopped_state);
    stale_state->addTransition(this, &ZeroProxy::Alive, live_state);
    // device might have rebooted
    stale_state->addTransition(this, &ZeroProxy::NewUrl, stopped_state);
    stale_state->addTransition(this, &ZeroProxy::ShutdownRequested, stopped_state);

    connect(stale_state, &QState::entered, st, static_cast<void (QTimer::*)()>(&QTimer::start));
    connect(stale_state, &QState::exited, st, &QTimer::stop);

    connect(stale_state, &QState::entered, [&]() {
        qDebug() << "Entering stale";
        state_ = ConnectionState::Stale;
        emit this->StatusUpdated();
    });
    connect(stale_state, &QState::exited, [&]() { qDebug() << "Leaving stale"; });
    proxyState.addState(stale_state);

    // 5. live state
    setupTimer(liveTimer, 20 * updateInterval_, live_state, stale_state);
    live_state->addTransition(this, &ZeroProxy::Alive, live_state);
    live_state->addTransition(this, &ZeroProxy::ShutdownRequested, shutdown_state);
    connect(live_state, &QState::entered, [&]() {
        qDebug() << "Entering live";
        liveTimer.start();
        state_ = ConnectionState::Live;
    });
    connect(live_state, &QState::exited, [&]() {
        qDebug() << "Leaving live";
        liveTimer.stop();
    });
    proxyState.addState(live_state);

    // 6. Shutdown state
    QTimer* shutdownTimer = new QTimer(shutdown_state);
    // as the QCoapReply gets disposed immediately , eventhough
    // the subscription cancel message has not been sent
    // It will be send once the next subscription update arrives -
    // But only if the original CoapReply object has not been deleted
    // (via deleteLater() ...)
    // (Go figure...)
    // Therefore, we need to stay in this state for a bit longer than the
    // subscription interval to make sure the subscription gets cancelled
    // And then also clean the observerReply in the exit event
    //
    setupTimer(*shutdownTimer, 5 * updateInterval_, shutdown_state, stopped_state);
    connect(shutdown_state, &QState::entered, shutdownTimer,
            static_cast<void (QTimer::*)()>(&QTimer::start));
    connect(shutdown_state, &QState::exited, shutdownTimer, &QTimer::stop);

    connect(shutdown_state, &QState::entered, [&]() {
        qDebug() << "Entering shutdown";
        this->unsubscribe();
    });

    connect(shutdown_state, &QState::exited, [&]() {
        qDebug() << "Entering shutdown";
        if (useGetForStatus) return;
        this->observerReply->deleteLater();
        this->observerReply = nullptr;
    });
    proxyState.addState(shutdown_state);

    // 7. Stopped state
    connect(stopped_state, &QState::entered, [&]() {
        state_ = ConnectionState::Stopped;
        qDebug() << "Entering stopped";
    });
    proxyState.addState(stopped_state);
    connect(&proxyState, &QStateMachine::finished, this, &ZeroProxy::Stop);

    proxyState.setInitialState(connect_state);
    proxyState.start();
}

void ZeroProxy::unsubscribe()
{
    if (useGetForStatus) {
        updateTimer.stop();
        return;
    }

    if (nullptr == observerReply) return;

    qDebug() << "Cancel Observer";
    coapClient.cancelObserve(observerReply);
}

uint32_t ZeroProxy::updateInterval() const { return updateInterval_; }

QString ZeroProxy::host() const { return url_.host(); }

QUrl ZeroProxy::url() const { return url_; }

void ZeroProxy::updateUrl(const QUrl& nUrl)
{
    if (nUrl == url_) return;
    url_ = nUrl;
    emit NewUrl();
}

QString ZeroProxy::uuid() const { return uuid_; }

QString ZeroProxy::hardwareVersion() const { return hardwareVersion_; }

QString ZeroProxy::hardwareAddress() const { return macAddress_; }

uint32_t ZeroProxy::uptime() const { return uptime_; }

bool ZeroProxy::closed() const { return closed_; }

ZeroProxy::OpenCloseTransition ZeroProxy::lastTransitionReason() const { return lastTransReason_; }

QString ZeroProxy::lastTransitionReasonStr() const
{
    switch (lastTransReason_) {
        case OpenCloseTransition::NONE:
            return "Initialization";
        case OpenCloseTransition::EXT:
            return "User requested";
        case OpenCloseTransition::OCP_HW:
            return "Hardware overcurrent protection";
        case OpenCloseTransition::OCP_SW:
            return "Software overcurrent protection";
        case OpenCloseTransition::OTP:
            return "Overtemperature protection";
        case OpenCloseTransition::OCP_TEST:
            return "Overcurrent protection test";
        case OpenCloseTransition::UVP:
            return "Undervoltage protection";
        case OpenCloseTransition::OVP:
            return "Over voltage protection";
        case OpenCloseTransition::UFP:
            return "Under frequency protection";
        case OpenCloseTransition::OFP:
            return "Over frequency protection";
    }
    return "Unknown";
}

uint32_t ZeroProxy::voltageRms() const { return vRms_; }

uint32_t ZeroProxy::currentRms() const { return cRms_; }

bool ZeroProxy::isStale() const { return (ConnectionState::Stale == state_); }

bool ZeroProxy::isStopped() const { return (ConnectionState::Stopped == state_); }

void ZeroProxy::onStatusUpdate(QCoapReply* reply, const QCoapMessage& message)
{
    if (reply->errorReceived() != QtCoap::Error::Ok) return;

    qDebug() << "Responsecode: " << reply->responseCode();

    if (!message.hasOption(QCoapOption::OptionName::ContentFormat)) return;

    QCoapOption format = message.option(QCoapOption::OptionName::ContentFormat);
    bool is_protob = ntohs(format.uintValue()) == NANOPB_CONTENT_FORMAT;
    if (!format.isValid() || (format.uintValue() != 0 && !is_protob)) {
        qDebug() << "Status message: Invalid content format";
        return;
    }

    qDebug() << "Message payload " << message.payload();

    if (is_protob && new_protocol) {
        ZCMessage msg;
        msg.ParseFromArray(reinterpret_cast<void*>(message.payload().data()),
                           message.payload().length());
        if (!msg.res().has_status()) {
            qDebug() << "Status update message does not contain status";
            return;
        }
        auto status = msg.res().status();

        uptime_ = status.uptime();
        closed_ = status.switch_state() == ZC_SWITCH_STATE_CLOSED;
        uint8_t tmp = status.cause();
        if (tmp > 9) {
            qWarning() << "Invalid OpenCloseTransition reason";
            lastTransReason_ = OpenCloseTransition::NONE;
        } else {
            lastTransReason_ = static_cast<OpenCloseTransition>(tmp);
        }

        cRms_ = status.current();
        vRms_ = status.voltage();

        for (int j = 0; j < status.temp_size(); ++j) {
            auto ts = status.temp(j);
            if (ts.loc() == ZC_TEMP_LOC_AMB)
                ambientTemp_ = ts.value();
            else if (ts.loc() == ZC_TEMP_LOC_MCU_1)
                mcuTemp_ = ts.value();
            else if (ts.loc() == ZC_TEMP_LOC_BRD_1)
                powerInTemp_ = ts.value();
            else if (ts.loc() == ZC_TEMP_LOC_BRD_2)
                powerOutTemp_ = ts.value();
        }

        auto fcRms = cRms_ / 1000.0;
        auto fvRms = vRms_ / 1000.0;
        auto ffreq = status.freq() / 1000.0;
        cSeries_.append(QPointF(uptime_, fcRms));
        vSeries_.append(QPointF(uptime_, fvRms));
        pSeries_.append(QPointF(uptime_, fcRms * fvRms));
        fSeries_.append(QPointF(uptime_, ffreq));

    } else {
        /* Message format is a CSV, fields are
         * 0      k_uptime_get_32(),
         * 1      (uint8_t)bcb_is_on(),
         * 2      (uint8_t), cause for last switch state transition
         * 3      (uint8_t), trip curve state
         * 4      bcb_get_current(),
         * 5      bcb_get_current_rms(),
         * 6      bcb_get_voltage(),
         * 7      bcb_get_voltage_rms(),
         * 8      bcb_get_temp(BCB_TEMP_SENSOR_PWR_IN),
         * 9      bcb_get_temp(BCB_TEMP_SENSOR_PWR_OUT),
         * 10     bcb_get_temp(BCB_TEMP_SENSOR_AMB),
         * 11     bcb_get_temp(BCB_TEMP_SENSOR_MCU));
         */

        const QList<QByteArray> values = message.payload().split(',');
        if (values.length() < 12) {
            qWarning() << "Invalid version format in Zero Status update message";
            return;
        }

        uptime_ = QString::fromUtf8(values[0]).toUInt();
        closed_ = QString::fromUtf8(values[1]).toInt();
        uint8_t tmp = QString::fromUtf8(values[2]).toUInt();
        if (tmp > 9) {
            qWarning() << "Invalid OpenCloseTransition reason";
        } else {
            lastTransReason_ = static_cast<OpenCloseTransition>(tmp);
        }

        cRms_ = QString::fromUtf8(values[5]).toUInt();
        vRms_ = QString::fromUtf8(values[7]).toUInt();

        auto fcRms = cRms_ / 1000.0;
        auto fvRms = vRms_ / 1000.0;
        cSeries_.append(QPointF(uptime_, fcRms));
        vSeries_.append(QPointF(uptime_, fvRms));
        pSeries_.append(QPointF(uptime_, fcRms * fvRms));

        powerInTemp_ = QString::fromUtf8(values[8]).toUInt();
        powerOutTemp_ = QString::fromUtf8(values[9]).toUInt();
        ambientTemp_ = QString::fromUtf8(values[10]).toUInt();
        mcuTemp_ = QString::fromUtf8(values[11]).toUInt();
    }

    emit StatusUpdated();

    if (!useGetForStatus) {
        qDebug() << "Has observe option: " << message.hasOption(QCoapOption::OptionName::Observe);
        QCoapOption observe = message.option(QCoapOption::OptionName::Observe);
        if (!observe.isValid()) {
            qDebug() << "Subscription refused";
            emit SubscriptionRefused();
        } else {
            qDebug() << "Subscription online";
            emit Alive();
        }
    } else {
        qDebug() << "Subscription online";
        emit Alive();
    }
}

void ZeroProxy::toggle()
{
    qDebug() << "Toggeling the breaker";
    QUrl url(url_);

    QCoapReply* reply = nullptr;

    if (new_protocol) {
        url.setPath("/device");
        ZCMessage msg;
        msg.mutable_req()->mutable_cmd()->set_cmd(ZC_DEVICE_CMD_TOGGLE);
        std::string serialised = msg.SerializeAsString();
        QByteArray body(serialised.c_str(), serialised.length());

        QCoapRequest req(url);
        QCoapOption option(QCoapOption::OptionName::ContentFormat, htons(NANOPB_CONTENT_FORMAT));
        req.addOption(option);
        req.setPayload(body);

        reply = coapClient.post(req);
    } else {
        url.setPath("/switch");
        QUrlQuery params;

        if (closed()) {
            params.addQueryItem("a", "open");
        } else {
            params.addQueryItem("a", "close");
        }

        url.setQuery(params);
        reply = coapClient.post(url);
    }

    connect(reply, &QCoapReply::finished, this, &ZeroProxy::onSwitchReplyFinished);
    // emit toggling();
}

void ZeroProxy::sendFirmwareUpdate(const QByteArray& fw) { smpClient.sendFirmwareUpdate(fw); }

uint16_t ZeroProxy::firmwareUpdateProgress() { return smpClient.firmwareUpdateProgress(); }

bool ZeroProxy::isFirmwareUpdateOngoing() { return smpClient.isFirmwareUpdateOngoing(); }

std::optional<bool> ZeroProxy::didFirmwareUpdateSucceed()
{
    return smpClient.didFirmwareUpdateSucceed();
}

void ZeroProxy::onSwitchReplyFinished(QCoapReply* reply)
{
    reply->disconnect();
    reply->deleteLater();
    if (reply->errorReceived() == QtCoap::Error::Ok) {
        qDebug() << "TOGGLE OK";
        return;
    }

    emit ToggleError();
}

void ZeroProxy::requestSmpInfo()
{
    qDebug() << "Sending SMP request";
    smp::GetStateOfImagesReq req;
    auto sst = smpClient.send(req);
}

void ZeroProxy::processSmpGetStateOfImagesResp(std::shared_ptr<smp::GetStateOfImagesResp> reply)
{
    qDebug() << "Loading image informations";

    fwSlots = reply->images();
    emit ReceivedSmpInfo();
}

QString ZeroProxy::fwSlotInfo(uint32_t sidx) const
{
    if (sidx > 1) return "";
    if (fwSlots.size() < sidx + 1) return "Empty";

    auto& s = fwSlots[sidx];
    return "Version: " + s.version + " active: " + (s.active ? "Yes" : "No");
}

uint32_t ZeroProxy::powerInTemp() const { return powerInTemp_; }

uint32_t ZeroProxy::powerOutTemp() const { return powerOutTemp_; }

uint32_t ZeroProxy::ambientTemp() const { return ambientTemp_; }

uint32_t ZeroProxy::mcuTemp() const { return mcuTemp_; }

void ZeroProxy::stop() { emit ShutdownRequested(); }

QwtPlotCurve* ZeroProxy::voltageSeries() { return &vCurve_; }

QwtPlotCurve* ZeroProxy::currentSeries() { return &cCurve_; }

QwtPlotCurve* ZeroProxy::powerSeries() { return &pCurve_; }

QwtPlotCurve* ZeroProxy::frequencySeries() { return &fCurve_; }

// TODO: *cringe* not a nice API at all, need to clean this up
std::vector<QPointF>* ZeroProxy::tripCurve() { return &tripCurve_; }

void ZeroProxy::setTripCurve(std::vector<QPointF>& curve)
{
    if (!new_protocol) return;

    auto oUrl = url_;
    oUrl.setPath("/config");

    ZCMessage msg;
    ZCCurveConfig* curveP =
        msg.mutable_req()->mutable_set_config()->mutable_config()->mutable_curve();
    curveP->set_direction(ZCFlowDirection::ZC_FLOW_DIRECTION_BACKWARD);

    for (int i = 0; i < curve.size(); ++i) {
        auto p = curveP->add_points();
        p->set_limit(curve[i].rx());
        p->set_duration(curve[i].ry());
    }

    QCoapReply* reply = sendCoapMsg(oUrl, msg);
    connect(reply, &QCoapReply::finished, this, &ZeroProxy::onSetTripCurveFinished);
}

void ZeroProxy::onSetTripCurveFinished(QCoapReply* reply)
{
    qDebug() << "SetTripCurve Responsecode: " << reply->responseCode();
    if (reply->errorReceived() != QtCoap::Error::Ok) {
        qWarning() << "Error while trying to set trip curve";
        emit SendStatusMessage("Setting Trip Curve Failed");
        return;
    }
    qDebug() << "Setting trip curve successful";

    emit SendStatusMessage("Successful update of trip curve");

    // Request trip curve reload
    getTripCurveConfig();
}

void ZeroProxy::setInitStateConfig(InitStateConfig state)
{
    if (!new_protocol) return;

    auto oUrl = url_;
    oUrl.setPath("/config");

    ZCMessage msg;
    ZCIniStateConfig* iSC =
        msg.mutable_req()->mutable_set_config()->mutable_config()->mutable_ini();

    iSC->set_state(static_cast<ZCSDeviceIniState>(state));

    QCoapReply* reply = sendCoapMsg(oUrl, msg);
    connect(reply, &QCoapReply::finished, this, &ZeroProxy::onSetInitStateConfigFinished);
}

void ZeroProxy::onSetInitStateConfigFinished(QCoapReply* reply)
{
    qDebug() << "SetInitStateConfig Responsecode: " << reply->responseCode();
    if (reply->errorReceived() != QtCoap::Error::Ok) {
        qWarning() << "Error while trying to set initial switch state";
        emit SendStatusMessage("Setting Initial Switch State Failed");
        return;
    }
    qDebug() << "Setting initial switch state successful";

    emit SendStatusMessage("Successful update of Initial Switch State");

    // Request init switch state
    getInitStateConfig();
}

void ZeroProxy::setOcpHwConfig(uint32_t ocp_hw_limit, uint32_t jitter_filter,
                               uint32_t recover_delay, uint32_t recover_attemtps,
                               bool autoreclose_enabled)
{
    if (!new_protocol) return;

    auto oUrl = url_;
    oUrl.setPath("/config");

    ZCMessage msg;
    auto oCPC = msg.mutable_req()->mutable_set_config()->mutable_config()->mutable_ocp_hw();

    oCPC->set_limit(ocp_hw_limit);
    oCPC->set_filter(jitter_filter);
    oCPC->set_rec_delay(recover_delay);
    oCPC->set_rec_attempts(recover_attemtps);
    oCPC->set_rec_en(autoreclose_enabled);

    QCoapReply* reply = sendCoapMsg(oUrl, msg);
    connect(reply, &QCoapReply::finished, this, &ZeroProxy::onSetOcpHwConfigFinished);
}

void ZeroProxy::onSetOcpHwConfigFinished(QCoapReply* reply)
{
    qDebug() << "SetOcpHwConfig Responsecode: " << reply->responseCode();
    if (reply->errorReceived() != QtCoap::Error::Ok) {
        qWarning() << "Error while trying to set ocphw config";
        emit SendStatusMessage("Setting OcpHW Config Failed");
        return;
    }
    qDebug() << "Setting Ocp HW Config successful";

    emit SendStatusMessage("Successful update of Ocp Hw Config");

    // Request init switch state
    getOcpHWConfig();
}

}  // namespace zero
