#include "zeroproxy.hpp"
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
    observerReply(nullptr), staleDetection(),
    liveTimer(), stale_(false), //staleTimer(),
    url_(url), uuid_(uuid), hardwareVersion_(hardwareVersion),
    macAddress_(macAddress), name_(""),
    updateInterval_(100),
    closed_(false), ocpActivated_(false), otpActivated_(false),
    uptime_(0), vRms_(0), cRms_(0)
{
    subscribe();
    initStaleDetection();
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
    QState* live = new QState(); 
    QState* stale = new QState(); 
//    QFinalState* dead = new QFinalState();

    liveTimer.setSingleShot(true);
    liveTimer.setInterval(5*updateInterval_);
    live->addTransition(&liveTimer, &QTimer::timeout, stale);
    live->addTransition(this, &ZeroProxy::statusUpdated, live);
    connect(live, &QState::entered, 
            [&]()
            {
                qDebug() << "Entering live";
                liveTimer.start();
            }
    );

/*    staleTimer.setSingleShot(true); 
    staleTimer.setInterval(10*updateInterval_);
    stale->addTransition(&staleTimer, &QTimer::timeout, dead);*/
    stale->addTransition(this, &ZeroProxy::statusUpdated, live);
    connect(stale, &QState::entered, 
            [&]()
            {
                qDebug() << "Entering stale";
                stale_ = true;
                emit this->stale();
//                staleTimer.start();
            }
    );
    connect(stale, &QState::exited, 
            [&]()
            {
                stale_ = false;
                qDebug() << "Leaving stale";
//                staleTimer.stop();
            }
    );

    staleDetection.addState(live);
    staleDetection.addState(stale);
 //   staleDetection.addState(dead);

    staleDetection.setInitialState(live);
    staleDetection.start();
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

QHostAddress ZeroProxy::host() const
{
    return QHostAddress();
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
     * 		     k_uptime_get_32(), (uint8_t)bcb_is_on(), (uint8_t)0U, (uint8_t)0U,
		     bcb_get_current(), bcb_get_current_rms(), bcb_get_voltage(),
		     bcb_get_voltage_rms(), bcb_get_temp(BCB_TEMP_SENSOR_PWR_IN),
		     bcb_get_temp(BCB_TEMP_SENSOR_PWR_OUT), bcb_get_temp(BCB_TEMP_SENSOR_AMB),
		     bcb_get_temp(BCB_TEMP_SENSOR_MCU));
      */

    const QList<QByteArray> values = message.payload().split(',');
    if (values.length() < 12) {
        qWarning() << "Invalid version format in Zero Status update message";
        return;
    }

    uptime_ = QString::fromUtf8(values[0]).toUInt();
    closed_ = QString::fromUtf8(values[1]).toInt();
    cRms_ = QString::fromUtf8(values[5]).toUInt();
    vRms_ = QString::fromUtf8(values[7]).toUInt();


    emit statusUpdated();
}

} // end namespace
