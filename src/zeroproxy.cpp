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
            QObject *parent) : QObject(parent),
    coapClient(QtCoap::SecurityMode::NoSecurity),
    observerReply(nullptr), proxyState(),
    liveTimer(), stale_(false), live_(false), //toggleTimer(),
    url_(url), uuid_(uuid), hardwareVersion_(hardwareVersion),
    macAddress_(macAddress), name_(""),
    updateInterval_(100),
    closed_(false), ocpActivated_(false), otpActivated_(false),
    uptime_(0), vRms_(0), cRms_(0),
    powerInTemp_(0), powerOutTemp_(0), ambientTemp_(0), mcuTemp_(0),
    lastTransReason_(OpenCloseTransition::NONE),
    smpClient(url.host(),1337),
    smpReply(), fwSlots()
{
    getSmpDetails();
}

ZeroProxy::~ZeroProxy()
{
    unsubscribe();
}

void ZeroProxy::subscribe()
{
    auto oUrl = url_;
    oUrl.setPath("/status");
    QUrlQuery params;
    params.addQueryItem("p", QString::number(updateInterval_));
    oUrl.setQuery(params);

    observerReply = coapClient.observe(oUrl);
    connect(observerReply, &QCoapReply::notified, this, &ZeroProxy::onStatusUpdate);
}

void ZeroProxy::initStaleDetection()
{
    qDebug() << "Configure statemachine";
    QState* live_state = new QState(); 
    QState* stale_state = new QState();
    QState* toggling_state = new QState();
//    QFinalState* dead = new QFinalState();

    liveTimer.setSingleShot(true);
    liveTimer.setInterval(20*updateInterval_);
    live_state->addTransition(&liveTimer, &QTimer::timeout, stale_state);
    live_state->addTransition(this, &ZeroProxy::statusUpdated, live_state);
    live_state->addTransition(this, &ZeroProxy::toggling, toggling_state);
    connect(live_state, &QState::entered, 
            [&]()
            {
                qDebug() << "Entering live";
                liveTimer.start();
                live_ = true;
            }
    );
    
    connect(live_state, &QState::exited, 
            [&]()
            {
                qDebug() << "Leaving live";
                live_ = false;
            }
    );

    stale_state->addTransition(this, &ZeroProxy::statusUpdated, live_state);
    connect(stale_state, &QState::entered, 
            [&]()
            {
                qDebug() << "Entering stale";
                stale_ = true;
                emit this->stale();
            }
    );
    connect(stale_state, &QState::exited, 
            [&]()
            {
                stale_ = false;
                qDebug() << "Leaving stale";
            }
    );

    toggleTimer.setSingleShot(true); 
    toggleTimer.setInterval(20*updateInterval_);
    toggling_state->addTransition(&toggleTimer, &QTimer::timeout, stale_state);
    toggling_state->addTransition(this, &ZeroProxy::statusUpdated, live_state);
    toggling_state->addTransition(this, &ZeroProxy::toggleError, live_state);
    connect(toggling_state, &QState::entered, 
            [&]()
            {
                qDebug() << "Entering toggling";
                toggleTimer.start();
            }
    );
    connect(toggling_state, &QState::exited, 
            [&]()
            {
                qDebug() << "Leaving toggling";
                toggleTimer.stop();
            }
    );
    

    proxyState.addState(live_state);
    proxyState.addState(stale_state);
    proxyState.addState(toggling_state);

    proxyState.setInitialState(live_state);
    proxyState.start();
}

void ZeroProxy::unsubscribe()
{
    if (nullptr == observerReply) return;

    coapClient.cancelObserve(observerReply);
    connect(observerReply, &QCoapReply::finished, this, &ZeroProxy::onUnsubscribe);
}

uint32_t ZeroProxy::updateInterval() const
{
    return updateInterval_;
}

void ZeroProxy::onUnsubscribe(QCoapReply *reply)
{
    if (reply->errorReceived() != QtCoap::Error::Ok)
    {
       return;
    }
    observerReply = nullptr;
    emit unsubscribed();
}

QString ZeroProxy::host() const
{
    return url_.host();
}

QUrl ZeroProxy::url() const
{
    return url_;
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
        case OpenCloseTransition::OCP:
            return "Overcurrent protection";
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
    return stale_;
}

bool ZeroProxy::isLive() const
{
    return live_;
}

void ZeroProxy::onStatusUpdate(QCoapReply *reply, const QCoapMessage &message)
{
    if (reply->errorReceived() != QtCoap::Error::Ok)
       return;

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
}

void ZeroProxy::toggle()
{
    if (!isLive()) return;

    QUrl url(url_);
    url.setPath("/switch");
    QUrlQuery params;
    params.addQueryItem("a", "toggle");
    url.setQuery(params);
    
    auto reply = coapClient.post(url);
    connect(reply, &QCoapReply::finished, this, &ZeroProxy::onSwitchReplyFinished);
    emit toggling();
}

void ZeroProxy::onSwitchReplyFinished(QCoapReply *reply)
{
    reply->deleteLater();
    if (reply->errorReceived() == QtCoap::Error::Ok)
    {
        qDebug() << "TOGGLE OK";
       return;
    }

    emit toggleError();
}

void ZeroProxy::getSmpDetails()
{
    qDebug() << "Sending SMP request";
    smp::ImageMgmtStateReq req;
    smpReply = smpClient.send(req);
    connect(smpReply.get(), &smp::SmpReply::finished, 
            [=]() {
             loadImages();

            subscribe();
            initStaleDetection();
            });
}

void ZeroProxy::loadImages()
{
    qDebug() << "Loading image informations";
    smp::ImageMgmtStatePayload pL;
    if (!smpReply->getPayload(pL)) 
    {
        qDebug() << "Failure when parsing Image payload";
        smpReply.reset();
        return;
    }

    fwSlots = pL.fwSlots;
    smpReply.reset();
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

} // end namespace
