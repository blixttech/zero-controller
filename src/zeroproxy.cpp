#include "zeroproxy.hpp"
#include "smpimagemgmt.hpp"

#include <QtEndian>
#include <QVariant>
#include <QUrlQuery>
#include <QState>
#include <QFinalState>

namespace zero {

ZeroProxy::ZeroProxy(const QUrl& url, const QString& uuid,
            const QString& hardwareVersion,
            const QString& macAddress,
            uint32_t updateIntervalVal,
            bool pullStatusUpdate,
            QObject *parent) : QObject(parent),
    coapClient(QtCoap::SecurityMode::NoSecurity),
    observerReply(nullptr), proxyState(),
    liveTimer(),  state_(ConnectionState::Offline), //toggleTimer(),
    url_(url), uuid_(uuid), hardwareVersion_(hardwareVersion),
    macAddress_(macAddress), name_(""),
    updateInterval_(updateIntervalVal),
    closed_(false), ocpActivated_(false), otpActivated_(false),
    uptime_(0), vRms_(0), cRms_(0),
    powerInTemp_(0), powerOutTemp_(0), ambientTemp_(0), mcuTemp_(0),
    lastTransReason_(OpenCloseTransition::NONE),
    smpClient(url.host(),1337),
    fwSlots(), useGetForStatus(pullStatusUpdate)
{
    connect(&smpClient, &smp::SmpClient::replyReceived, this, &ZeroProxy::processSmpReply);
    initStaleDetection();
}

ZeroProxy::~ZeroProxy()
{
    unsubscribe();
}

void ZeroProxy::subscribe()
{
    if (useGetForStatus)
    {
        pullStatusUpdate();
        return;
    }

    qDebug() << "Sending subscribe request";
    auto oUrl = url_;
    oUrl.setPath("/status");
    QUrlQuery params;
    params.addQueryItem("p", QString::number(updateInterval_));
    oUrl.setQuery(params);

    if (nullptr != observerReply)
    {
        observerReply->disconnect();
        observerReply->deleteLater();
    }

    observerReply = coapClient.observe(oUrl);
    connect(observerReply, &QCoapReply::notified, this, &ZeroProxy::onStatusUpdate);
}

void ZeroProxy::pullStatusUpdate()
{
    qDebug() << "Using get for status update";
    updateTimer.setInterval(updateInterval_);
    connect(&updateTimer, &QTimer::timeout, 
            [&]()
            {
                qDebug() << "executing pull update";
                auto oUrl = url_;
                oUrl.setPath("/status");
                coapClient.get(oUrl);
            }
    );

    connect(&coapClient, &QCoapClient::finished, 
            [&](QCoapReply* reply)
            {
                if (QtCoap::ResponseCode::Content == reply->responseCode()) 
                {
                    this->onStatusUpdate(reply, reply->message());
                }
                reply->deleteLater();
            }
    );
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
    auto subscribe_state = new QState();
    auto stale_state = new QState();
    auto live_state = new QState(); 
    auto shutdown_state = new QState(); 
    auto stopped_state = new QFinalState();

    // 1. ConnectState (also starting state)
    setupTimer(connectTimer, 10000, connect_state, connect_state);
    connect_state->addTransition(&smpClient, &smp::SmpClient::connectionEstablished, smpinfo_state);
    connect(connect_state, &QState::entered,
            [&]()
            {
                qDebug() << "Entering connect";
                state_ = ConnectionState::Offline;
                connectTimer.start();
                smpClient.connect();
            }
    );
    connect(connect_state, &QState::exited,
            [&]()
            {
                qDebug() << "Leaving connect";
                connectTimer.stop();
            }
    );
    proxyState.addState(connect_state);

    // 2. SmpInfo State
    setupTimer(smpTimer, 10000, smpinfo_state, smpinfo_state);
    smpinfo_state->addTransition(this, &ZeroProxy::receivedSmpInfo, subscribe_state);
    connect(smpinfo_state, &QState::entered,
            [&]() 
            {
                qDebug() << "Entering smpinfo";
                smpTimer.start();
                requestSmpInfo();
                state_ = ConnectionState::Connected;
            }
    );
    connect(smpinfo_state, &QState::exited,
            [&]() 
            {
                qDebug() << "Exiting smpinfo";
                smpTimer.stop();
            }
    );
    proxyState.addState(smpinfo_state);
   
    // 3. the subscribe state
    setupTimer(subscribeTimer, 10000, subscribe_state, stale_state); 
    subscribe_state->addTransition(this, &ZeroProxy::live, live_state);
    subscribe_state->addTransition(this, &ZeroProxy::subscriptionRefused, stale_state);
    connect(subscribe_state, &QState::entered,
            [&]()
            {
                qDebug() << "Entered subscribe";
                subscribeTimer.start();
                subscribe();    
            }
    );
    connect(subscribe_state, &QState::exited,
            [&]()
            {
                qDebug() << "Exiting subscribe";
                subscribeTimer.stop();
            }
    );
    proxyState.addState(subscribe_state);
  
    // 4. stale state
    QTimer *st = new QTimer(stale_state); 
    setupTimer(*st, 10*1000, stale_state, stopped_state);
    stale_state->addTransition(this, &ZeroProxy::live, live_state);
    // device might have rebooted
    stale_state->addTransition(this, &ZeroProxy::newUrl, stopped_state);
    stale_state->addTransition(this, &ZeroProxy::shutdownRequested, stopped_state);

    connect(stale_state, &QState::entered, st, static_cast<void (QTimer::*)()>(&QTimer::start)); 
    connect(stale_state, &QState::exited, st, &QTimer::stop); 

    connect(stale_state, &QState::entered, 
            [&]()
            {
                qDebug() << "Entering stale";
                state_ = ConnectionState::Stale;
                emit this->statusUpdated();
            }
    );
    connect(stale_state, &QState::exited, 
            [&]()
            {
                qDebug() << "Leaving stale";
            }
    );
    proxyState.addState(stale_state);

    // 5. live state
    setupTimer(liveTimer, 20*updateInterval_, live_state, stale_state);
    live_state->addTransition(this, &ZeroProxy::live, live_state);
    live_state->addTransition(this, &ZeroProxy::shutdownRequested, shutdown_state);
    connect(live_state, &QState::entered, 
            [&]()
            {
                qDebug() << "Entering live";
                liveTimer.start();
                state_ = ConnectionState::Live;
            }
    );
    connect(live_state, &QState::exited, 
            [&]()
            {
                qDebug() << "Leaving live";
                liveTimer.stop();
            }
    );
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
    connect(shutdown_state, &QState::entered, shutdownTimer, static_cast<void (QTimer::*)()>(&QTimer::start)); 
    connect(shutdown_state, &QState::exited, shutdownTimer, &QTimer::stop); 
    
    connect(shutdown_state, &QState::entered, 
            [&]()
            {
                qDebug() << "Entering shutdown";
                this->unsubscribe();
            }
    );
    
    connect(shutdown_state, &QState::exited, 
            [&]()
            {
                qDebug() << "Entering shutdown";
                if (useGetForStatus) return;
                this->observerReply->deleteLater();
                this->observerReply = nullptr;
            }
    );
    proxyState.addState(shutdown_state);

    // 7. Stopped state
    connect(stopped_state, &QState::entered, 
            [&]()
            {
                state_ = ConnectionState::Stopped;
                qDebug() << "Entering stopped";
            }
    );
    proxyState.addState(stopped_state);
    connect(&proxyState, &QStateMachine::finished, this, &ZeroProxy::stopped);

    proxyState.setInitialState(connect_state);
    proxyState.start();
}

void ZeroProxy::unsubscribe()
{
    if (useGetForStatus)
    {
        updateTimer.stop();
        return;
    }
        
    if (nullptr == observerReply) return;

    qDebug() << "Cancel Observer";
    coapClient.cancelObserve(observerReply);
}

uint32_t ZeroProxy::updateInterval() const
{
    return updateInterval_;
}

QString ZeroProxy::host() const
{
    return url_.host();
}

QUrl ZeroProxy::url() const
{
    return url_;
}

void ZeroProxy::updateUrl(const QUrl& nUrl)
{
    if (nUrl == url_) return;
    url_ = nUrl;
    emit newUrl();
}

QString ZeroProxy::uuid() const
{
    return uuid_;
}

QString ZeroProxy::hardwareVersion() const
{
    return hardwareVersion_;
}

QString ZeroProxy::hardwareAddress() const
{
    return macAddress_;
}

uint32_t ZeroProxy::uptime() const
{
    return uptime_;
}

bool ZeroProxy::closed() const
{
    return closed_;
}

ZeroProxy::OpenCloseTransition ZeroProxy::lastTransitionReason() const
{
    return lastTransReason_;
}
    
QString ZeroProxy::lastTransitionReasonStr() const
{
    switch (lastTransReason_)
    {
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
    }
    return "Unknown";
}

uint32_t ZeroProxy::voltageRms() const
{
    return vRms_;
}

uint32_t ZeroProxy::currentRms() const
{
    return cRms_;
}

bool ZeroProxy::isStale() const
{
    return (ConnectionState::Stale == state_);
}

bool ZeroProxy::isStopped() const
{
    return (ConnectionState::Stopped == state_);
}

void ZeroProxy::onStatusUpdate(QCoapReply *reply, const QCoapMessage &message)
{
    if (reply->errorReceived() != QtCoap::Error::Ok)
       return;

    qDebug() << "Responsecode: " << reply->responseCode();


    if (!message.hasOption(QCoapOption::OptionName::ContentFormat)) return;

    QCoapOption format = message.option(QCoapOption::OptionName::ContentFormat);
    if(!format.isValid() || format.uintValue() != 0) {
        qDebug() << "Invalid content format";
        return;
    }

    qDebug() << "Message payload " << message.payload();

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
    if (values.length() < 12) 
    {
        qWarning() << "Invalid version format in Zero Status update message";
        return;
    }

    uptime_ = QString::fromUtf8(values[0]).toUInt();
    closed_ = QString::fromUtf8(values[1]).toInt();
    uint8_t tmp = QString::fromUtf8(values[2]).toUInt();
    if (tmp > 5)
    {
        qWarning() << "Invalid OpenCloseTransition reason";
    }
    else 
    {
        lastTransReason_ = static_cast<OpenCloseTransition>(tmp);
    }

    cRms_ = QString::fromUtf8(values[5]).toUInt();
    vRms_ = QString::fromUtf8(values[7]).toUInt();

    powerInTemp_ = QString::fromUtf8(values[8]).toUInt();
    powerOutTemp_ = QString::fromUtf8(values[9]).toUInt();
    ambientTemp_ = QString::fromUtf8(values[10]).toUInt();
    mcuTemp_ = QString::fromUtf8(values[11]).toUInt();

    emit statusUpdated();
   
    if (!useGetForStatus)
    {
        QCoapOption observe = message.option(QCoapOption::OptionName::Observe);
        if (!observe.isValid())
        {
            qDebug() << "Subscription refused";
            emit subscriptionRefused();
        }
    }
    else
    {
        qDebug() << "Subscription online";
        emit live();
    }
}

void ZeroProxy::toggle()
{
    QUrl url(url_);
    url.setPath("/switch");
    QUrlQuery params;

    if (closed()) {
        params.addQueryItem("a", "open");
    } else {
        params.addQueryItem("a", "close");
    }

    url.setQuery(params);
    
    auto reply = coapClient.post(url);
    connect(reply, &QCoapReply::finished, this, &ZeroProxy::onSwitchReplyFinished);
    //emit toggling();
}

void ZeroProxy::onSwitchReplyFinished(QCoapReply *reply)
{
    reply->disconnect();
    reply->deleteLater();
    if (reply->errorReceived() == QtCoap::Error::Ok)
    {
        qDebug() << "TOGGLE OK";
       return;
    }

    emit toggleError();
}

void ZeroProxy::requestSmpInfo()
{
    qDebug() << "Sending SMP request";
    smp::ImageMgmtStateReq req;
    auto sst = smpClient.send(req);
}

void ZeroProxy::processSmpReply(std::shared_ptr<smp::SmpReply> reply)
{
    if (reply->status() != smp::SmpReply::Status::Ok)
    {
        qWarning() << "Received invalid SmpReply";    
        return;
    }

    qDebug() << "Loading image informations";
    smp::ImageMgmtStatePayload pL;
    if (!reply->getPayload(pL)) 
    {
        qDebug() << "Failure when parsing Image payload";
        return;
    }

    fwSlots = pL.fwSlots;
    emit receivedSmpInfo();
}

QString ZeroProxy::fwSlotInfo(uint32_t sidx) const
{
    if (sidx > 1) return "";
    if (fwSlots.size() < sidx+1) return "Empty";

    auto &s = fwSlots[sidx];
    return "Version: " + s.version + " active: " + (s.active ? "Yes" : "No");
}

uint32_t ZeroProxy::powerInTemp() const
{
    return powerInTemp_;
}

uint32_t ZeroProxy::powerOutTemp() const
{
    return powerOutTemp_;
}

uint32_t ZeroProxy::ambientTemp() const
{
    return ambientTemp_;
}

uint32_t ZeroProxy::mcuTemp() const
{
    return mcuTemp_;
}

void ZeroProxy::stop()
{
    emit shutdownRequested();
}

} // end namespace
