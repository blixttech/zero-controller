#ifndef __COAP_RESOURCE_DISCOVERY_H__
#define __COAP_RESOURCE_DISCOVERY_H__

#include <cstdint>
#include <QObject>
#include <QHostAddress>
#include <QCoapResource>

class CoapMessage;
class CoapResourceDiscovery : public QObject
{
    Q_OBJECT
public:
    CoapResourceDiscovery(QObject* parent=nullptr);
    virtual ~CoapResourceDiscovery();
    bool start(uint32_t interval=1000);
    void stop();
    bool addLocation(const QString &host, int port=5683, 
                    const QString &path=QLatin1String("/.well-known/core"));
    bool addLocation(const QHostAddress &host, int port=5683, 
                    const QString &path=QLatin1String("/.well-known/core"));

signals:
    void discovered(const QVector<QCoapResource> &resources, const QHostAddress &host, int port);

private slots:
    void onReadyRead();
    void onDiscoveryTimer();

private:
    Q_DISABLE_COPY(CoapResourceDiscovery);
    class LocationData;
    class PrivateData;
    void processDiscoveryResponse(const QHostAddress &sender, int port, const QByteArray &data);
    void processMessage(const QHostAddress &sender, int port, CoapMessage *message);
    bool createDiscoveryFrame(QByteArray &data, LocationData* location);
    PrivateData* pData_; 
};

#endif // __COAP_RESOURCE_DISCOVERY_H__