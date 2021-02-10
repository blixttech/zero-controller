#pragma once

#include <QObject>
#include <QCoapResource>
#include <QtCoap>

#include "coapresourcediscovery.hpp"

namespace zero {

class ZeroCoapScanner : public QObject
{
    Q_OBJECT
    public:
        ZeroCoapScanner(QObject* parent = nullptr);

        void startScanning(uint32_t interval=1000);
        void stopScanning();
        void addScanTarget(QHostAddress host, int port=5683);
    
    signals:
        void newZeroDetected(const QString& uuid, const QUrl& url, 
                             const QString& hwversion, const QString& macaddress);

    private slots:
        void onVersionReply(QCoapReply *reply);
        void onVersionError(QCoapReply *reply, QtCoap::Error error);
        void onCoapDiscovered(const QVector<QCoapResource> &resources, 
                                const QHostAddress &host, int port);
        bool parseVersion(QCoapReply *reply, QUrl& url, QString& uuid, 
                            QString& hwversion, QString macaddress);

    private:
        CoapResourceDiscovery resourceDiscovery_;
        QCoapClient coapClient_;
};

} // end namespace
