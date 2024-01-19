#include "zerocoapscanner.hpp"
#include <netinet/in.h>
#include "zc_messages.pb.h"
#include "common.hpp"


#ifdef USE_WINSOCK2
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif


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
    bool new_protocol = false;
    if (!parseVersion(reply, url, uuid, hwversion, macaddress, new_protocol))
        return;

    emit newZeroDetected(uuid, url, hwversion, macaddress, new_protocol); 
}

void ZeroCoapScanner::onVersionError(QCoapReply *reply, QtCoap::Error error)
{
    qWarning() << "Invalid version reply received";
}

bool ZeroCoapScanner::parseVersion(QCoapReply *reply, QUrl& url, QString& uuid, 
                                    QString& hwversion, QString& macaddress, bool& new_protocol)
{
    if (!reply->isSuccessful()) {
        return false;
    }

    const QCoapMessage &message = reply->message();
    QCoapOption format = message.option(QCoapOption::OptionName::ContentFormat);
    if(!format.isValid() || !(format.uintValue() == 0 || ntohs(format.uintValue()) == NANOPB_CONTENT_FORMAT))
    {
        qDebug() << "Invalid content format " << ntohs(format.uintValue());
        return false;
    }

    if (ntohs(format.uintValue()) == NANOPB_CONTENT_FORMAT)
    {
        new_protocol = true;        
    }
    else
    {
        new_protocol = false;
    }
    
    url.setScheme("coap");
    url.setHost(reply->url().host());
    url.setPort(reply->url().port());


    if (!new_protocol) {            
        const QList<QByteArray> values = message.payload().split(',');
        if (values.length() < 5) {
            qDebug() << "Invalid version format '" << message.payload() << "'";
            return false;
        }


        uuid = QString::fromUtf8(values[0]);
        macaddress = QString::fromUtf8(values[1]);

        hwversion = QString::fromUtf8(values[2]) + "." + 
                    QString::fromUtf8(values[3]) + "." + 
                    QString::fromUtf8(values[4]);
    }
    else {
        ZCMessage msg;
        if (!msg.ParseFromArray(message.payload().constData(), message.payload().size())) {
            qDebug() << "Invalid protobuf format in version message";
            return false;
        }

        if (!(msg.has_res() && msg.res().has_version())) {
            qDebug() << "Invalid version message";
            return false;
        }

        auto version = msg.res().version();
        uuid = QString::fromStdString(version.uuid());
        macaddress = "N/A";
        hwversion = "N/A";
    }
    
    return true;
}

} // end of namespace
