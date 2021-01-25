#ifndef __BREAKER_H__
#define __BREAKER_H__

#include <QObject>
#include <QString>
#include <QHostAddress>
#include <QUrl>

class QCoapReply;
class Breaker : public QObject
{
public:

    enum class Command : uint8_t {
        NONE = 0,
        ON,
        OFF,
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

    virtual ~Breaker();
    bool sendCommand(Command command);
    bool startObserving(uint32_t interval=1000);
    void setName(QString &name);
    bool setObservationInterval(uint32_t interval);
    QString uuid();
    QString hardwareVersion();
    QString hardwareAddress();
    QHostAddress host();
    uint32_t voltage();
    uint32_t current();
    uint32_t voltageRms();
    uint32_t currentRms();
    int8_t temperature(TemperatureSensor sensor);
    bool isOcpActivated();
    bool isOtpActivated();
    bool isOn();
    bool isObserved();
    QString name();
    uint32_t observationInterval();
    uint32_t lastTimestamp();
    QUrl url();
    static Breaker* create(QCoapReply *reply, QObject *parent=nullptr);

signals:
    void onCommandCompleted(Breaker *breaker, Command command);
    void onCommandFailed(Breaker *breaker, Command command);
    void onStatusUpdate(Breaker *breaker);
    void onEmergency(Breaker *breaker, Emergency emergency);

private:
    Q_DISABLE_COPY(Breaker);
    explicit Breaker(QObject *parent=nullptr);
    static bool validateVersion(QCoapReply *reply, QMap<QString, QVariant> &properties);
    class PrivateData;
    PrivateData* pData_;
};

#endif // __BREAKER_H__