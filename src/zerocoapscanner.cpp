#include "zerocoapscanner.hpp"

namespace zero {

ZeroCoapScanner::ZeroCoapScanner(QObject* parent) : QObject(parent),
    resourceDiscovery_(),
    coapClient_(QtCoap::SecurityMode::NoSecurity)
{
    QObject::connect(&resourceDiscovery_, &CoapResourceDiscovery::discovered, 
                    this, &ZeroCoapScanner::onCoapDiscovered);
    QObject::connect(&coapClient_, &QCoapClient::finished, 
                    this, &ZeroCoapScanner::onVersionReply);
    QObject::connect(&coapClient_, &QCoapClient::error, 
                    this, &ZeroCoapScanner::onVersionError);
}

void ZeroCoapScanner::startScanning(uint32_t interval)
{
    resourceDiscovery_.start(interval);
}

void ZeroCoapScanner::stopScanning()
{
    resourceDiscovery_.stop();
}

void ZeroCoapScanner::addScanTarget(QHostAddress host, int port)
{
    resourceDiscovery_.addLocation(host, port, true);
}

void ZeroCoapScanner::onCoapDiscovered(const QVector<QCoapResource> &resources, 
                                        const QHostAddress &host, int port)
{
    QUrl url;
    url.setScheme("coap");
    url.setHost(host.toString());
    url.setPort(port);

    bool versionFound = false;
    foreach (QCoapResource resource, resources) {
        if (resource.path().compare("/version")) {
            versionFound = true;
            break;
        }
    }

    if (versionFound) {
        url.setPath("/version");
        coapClient_.get(url);
    }
}

void ZeroCoapScanner::onVersionReply(QCoapReply *reply)
{
    // no matter what free the response
    reply->deleteLater();

    if (reply->errorReceived() != QtCoap::Error::Ok) 
        return;

    QString uuid, hwversion, macaddress;
    QUrl url;
    if (!parseVersion(reply, url, uuid, hwversion, macaddress))
        return;

    emit newZeroDetected(uuid, url, hwversion, macaddress); 
}

void ZeroCoapScanner::onVersionError(QCoapReply *reply, QtCoap::Error error)
{
    qWarning() << "Invalid version reply received";
}

bool ZeroCoapScanner::parseVersion(QCoapReply *reply, QUrl& url, QString& uuid, 
                                    QString& hwversion, QString macaddress)
{
    if (!reply->isSuccessful()) {
        return false;
    }

    const QCoapMessage &message = reply->message();
    QCoapOption format = message.option(QCoapOption::OptionName::ContentFormat);
    if(!format.isValid() || format.uintValue() != 0) {
        qDebug() << "Invalid content format";
        return false;
    }

    const QList<QByteArray> values = message.payload().split(',');
    if (values.length() < 5) {
        qDebug() << "Invalid version format";
        return false;
    }

    url.setScheme("coap");
    url.setHost(reply->url().host());
    url.setPort(reply->url().port());

    uuid = QString::fromUtf8(values[0]);
    macaddress = QString::fromUtf8(values[1]);

    hwversion = QString::fromUtf8(values[2]) + "." + 
                QString::fromUtf8(values[3]) + "." + 
                QString::fromUtf8(values[4]);
    
    return true;
}

} // end of namespace
