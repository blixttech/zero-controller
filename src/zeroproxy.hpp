#pragma once

#include <QObject>
#include <QString>
#include <QHostAddress>
#include <QCoapReply>
#include <QUrl>
#include <QCoapClient>

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

    uint32_t uptime() const;

signals:
    void statusUpdated(const QString& uuid);

private slots:
    void onStatusUpdate(QCoapReply *reply, const QCoapMessage &message);

private:
    QCoapClient coapClient;
    QCoapReply* observerReply;

    QUrl url_;
    QString uuid_;
    QString hardwareVersion_;
    QString macAddress_;
    QString name_;

    bool closed_;
    bool ocpActivated_;
    bool otpActivated_;

    uint32_t uptime_;

};

} // end namespace
