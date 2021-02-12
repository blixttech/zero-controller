#pragma once

#include <QCoapClient>
#include <QCoapReply>
#include <QHostAddress>
#include <QObject>
#include <QStateMachine>
#include <QString>
#include <QTimer>
#include <QUrl>

namespace zero {

class ZeroProxy : public QObject
{
    Q_OBJECT
public:

    enum class Status : uint8_t {
        NONE = 0,
        OPEN,
        CLOSE,
        OCP_TEST
    };

    enum class TemperatureSensor : uint8_t {
        POWER_IN = 0,
        POWER_OUT,
        AMBIENT,
        MCU
    };

    enum class Emergency : uint8_t {
        OVERCURRENT = 0,
        OVERTEMPERATURE
    };

    ZeroProxy(const QUrl& url,
            const QString& uuid,
            const QString& hardwareVersion,
            const QString& macAddress,
            QObject *parent = nullptr);

    virtual ~ZeroProxy();

    QString uuid() const ;
    QString hardwareVersion() const ;
    QString hardwareAddress() const;
    QHostAddress host() const;
    
    // TODO: return tuple of timestamp and voltage
    uint32_t voltage() const;
    uint32_t current() const;
    uint32_t voltageRms() const;
    uint32_t currentRms() const;
//    int8_t temperature(TemperatureSensor sensor);
    bool ocpActivated() const;
    bool otpActivated() const;
    bool closed() const;

    QString name() const;
//    uint32_t lastTimestamp();
    QUrl url() const;
    uint32_t updateInterval() const;

    uint32_t uptime() const;
    

    void unsubscribe();

    bool isStale() const;


signals:
    void statusUpdated();
    void unsubscribed();

    // emitted when the zero has been stale for too long
    // the proxy should be deleted after this
//    void dead();

    // emitted when the zero has not send status data in a while
    void stale();



private slots:
    void onStatusUpdate(QCoapReply *reply, const QCoapMessage &message);
    void onUnsubscribe(QCoapReply *reply);

private:
    void subscribe();
    void initStaleDetection();

    QCoapClient coapClient;
    QCoapReply* observerReply;

    QStateMachine staleDetection;
    QTimer liveTimer;
 //   QTimer staleTimer;
 //
    bool stale_;
    

    QUrl url_;
    QString uuid_;
    QString hardwareVersion_;
    QString macAddress_;
    QString name_;
    uint32_t updateInterval_;

    bool closed_;
    bool ocpActivated_;
    bool otpActivated_;

    uint32_t uptime_;
    uint32_t vRms_;
    uint32_t cRms_;

};

} // end namespace
