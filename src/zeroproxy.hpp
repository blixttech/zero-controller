#pragma once

#include <QCoapClient>
#include <QCoapReply>
#include <QHostAddress>
#include <QObject>
#include <QStateMachine>
#include <QString>
#include <QTimer>
#include <QUrl>

#include "smpclient.hpp"
#include "smpimagemgmt.hpp"

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

    ZeroProxy(const QUrl& url,
            const QString& uuid,
            const QString& hardwareVersion,
            const QString& macAddress,
            QObject *parent = nullptr);

    virtual ~ZeroProxy();

    QString uuid() const ;
    QString hardwareVersion() const ;
    QString hardwareAddress() const;
    QString host() const;
    
    // TODO: return tuple of timestamp and voltage
    uint32_t voltage() const;
    uint32_t current() const;
    uint32_t voltageRms() const;
    uint32_t currentRms() const;
    bool closed() const;


    QString name() const;
//    uint32_t lastTimestamp();
    QUrl url() const;
    uint32_t updateInterval() const;

    uint32_t uptime() const;

    QString fwSlotInfo(uint32_t slot) const;
    
    uint32_t powerInTemp() const;
    uint32_t powerOutTemp() const;
    uint32_t ambientTemp() const;
    uint32_t mcuTemp() const;

    bool isStale() const;
    bool isLive() const;


    void unsubscribe();
    /*
     * Sends a request to the associated Zero
     * to toggle the switches
     */
    void toggle();

signals:
    void statusUpdated();
    void unsubscribed();

    // emitted when the zero has been stale for too long
    // the proxy should be deleted after this
//    void dead();

    // emitted when the zero has not send status data in a while
    void stale();

    /*
     * Emitted when a switch toggle is requested
     */
    void toggling();
    void toggleError();


private slots:
    void onStatusUpdate(QCoapReply *reply, const QCoapMessage &message);
    void onUnsubscribe(QCoapReply *reply);

    void onSwitchReplyFinished(QCoapReply *reply);

private:
    void subscribe();
    void initStaleDetection();
    void getSmpDetails();
    void loadImages();

    QCoapClient coapClient;
    QCoapReply* observerReply;

    QStateMachine proxyState;
    QTimer liveTimer;
    QTimer toggleTimer;
    
    bool stale_;
    bool live_;
    

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

    uint32_t powerInTemp_;
    uint32_t powerOutTemp_;
    uint32_t ambientTemp_;
    uint32_t mcuTemp_;

    ::smp::SmpClient smpClient;
    std::shared_ptr<smp::SmpReply> smpReply;
    std::vector<::smp::ImageSlot> fwSlots;
};

} // end namespace
