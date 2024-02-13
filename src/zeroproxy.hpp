#pragma once

#include <QPointF>
#include <QCoapClient>
#include <QCoapReply>
#include <QHostAddress>
#include <QObject>
#include <QStateMachine>
#include <QString>
#include <QTimer>
#include <QUrl>

#include <QwtPlotCurve>

#include "smp/client.hpp"
#include "smp/imagemgmt.hpp"

#include "zerodatastream.hpp"

#include <vector>

namespace zero {

class ZeroProxy : public QObject
{
    Q_OBJECT
public:

    enum class OpenCloseTransition : uint8_t {
        NONE = 0, // only during initialisation
        EXT = 1, // external request, button or coap
        OCP_HW = 2, // over current protection
        OCP_SW = 3, // over current protection
        OCP_TEST = 4, // over current test
        OTP = 5, // over temperature protection
        UVP = 6, // under voltage protection
        OVP	= 7, /* Over voltage protection */
        UFP	= 8, /* Under frequency protection */
        OFP	= 9 /* Over frequency protection */
    };

    ZeroProxy(const QUrl& url,
            const QString& uuid,
            const QString& hardwareVersion,
            const QString& macAddress,
            uint32_t updateIntervalVal = 100,
            bool new_protocol = false,
            bool pullStatusUpdate = false,
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

    QwtPlotCurve* voltageSeries();
    QwtPlotCurve* currentSeries();
    QwtPlotCurve* powerSeries();
    QwtPlotCurve* frequencySeries();

    std::vector<QPointF>* tripCurve();

    OpenCloseTransition lastTransitionReason() const;
    QString lastTransitionReasonStr() const;

    QString name() const;
//    uint32_t lastTimestamp();
    QUrl url() const;
    void updateUrl(const QUrl& url);

    uint32_t updateInterval() const;

    uint32_t uptime() const;

    QString fwSlotInfo(uint32_t slot) const;
    
    uint32_t powerInTemp() const;
    uint32_t powerOutTemp() const;
    uint32_t ambientTemp() const;
    uint32_t mcuTemp() const;

    bool isStale() const;
    bool isStopped() const;

    void stop();


    /*
     * Sends a request to the associated Zero
     * to toggle the switches
     */
    void toggle();

    /*
     * Sends a firmware update
     */
    void sendFirmwareUpdate(const QByteArray& fw);


    /*
     * Returns the firmware update status
     * value < 0 means not upload ongoing
    */
    uint16_t firmwareUpdateProgress();

    
    bool isFirmwareUpdateOngoing();
    /*
        Returns true if successful,
        Otherwise false,
        std::nullopt if never ran
    */
    std::optional<bool> didFirmwareUpdateSucceed();

signals:
    void statusUpdated();
    void unsubscribed();
    void stopped();

    /*
     * Emitted when a switch toggle is requested
     */
    void toggling();
    void toggleError();

    /* for internal use
     */

    void receivedSmpInfo();
    void receivedConfig();
    void live();
    void newUrl();
    void subscriptionRefused();
    void shutdownRequested();

private slots:
    void onStatusUpdate(QCoapReply *reply, const QCoapMessage &message);

    void onSwitchReplyFinished(QCoapReply *reply);
    void onGetConfigFinished(QCoapReply *reply);
        
    void processSmpGetStateOfImagesResp(std::shared_ptr<smp::GetStateOfImagesResp> reply);

private:
    enum ConnectionState
    {
        Offline = 0,
        Connected = 1,
        Live = 2,
        Stale = 3,
        Stopped = 4
    };

    void subscribe();
    void getConfig();
    void pullStatusUpdate();
    void unsubscribe();
    void initStaleDetection();
    void requestSmpInfo();
    void loadImages();

    QCoapClient coapClient;
    QCoapReply* observerReply;

    QStateMachine proxyState;
    QTimer connectTimer;
    QTimer smpTimer;
    QTimer getConfigTimer;
    QTimer subscribeTimer;
    QTimer liveTimer;
    QTimer updateTimer;

    ConnectionState state_;
    

    // Data 
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
    OpenCloseTransition lastTransReason_;

    ZeroDataStream vSeries_;
    ZeroDataStream cSeries_;
    ZeroDataStream pSeries_;
    ZeroDataStream fSeries_;

    QwtPlotCurve vCurve_;
    QwtPlotCurve cCurve_;
    QwtPlotCurve pCurve_;
    QwtPlotCurve fCurve_;

    std::vector<QPointF> tripCurve_;
        
    ::smp::Client smpClient;
    std::vector<::smp::ImageSlot> fwSlots;
    bool useGetForStatus;

    bool new_protocol;
};

} // end namespace
