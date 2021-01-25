#include "coapresourcediscovery.hpp"
#include "coapmessage.hpp"
#include <cstdint>
#include <QUdpSocket>
#include <QNetworkDatagram>
#include <QTimer>
#include <QMap>
#include <QUrl>
#include <QHostAddress>
#include <QRandomGenerator>
#include <QtEndian>
#include <QDebug>

class CoapResourceDiscovery::LocationData
{
public:
    QHostAddress host;
    uint16_t port;
    QString path;
    uint16_t lastMassageId;
};


class CoapResourceDiscovery::PrivateData : QObject
{
public:
    PrivateData(QObject *parent = nullptr) : QObject(parent),
                                            randomGenerator(QRandomGenerator::securelySeeded())
    {
        socket = nullptr;
        timer = nullptr;
    }

    ~PrivateData()
    {
        if (socket) {
            socket->close();
            delete socket;
            socket = nullptr;
        }

        if (timer) {
            timer->stop();
            delete timer;
            timer = nullptr;
        }

        foreach (LocationData *location, locations) {
            delete location;
            location = nullptr;
        }
    }
    QUdpSocket* socket;
    QTimer *timer;
    QMap<QString, LocationData*> locations;
    QRandomGenerator randomGenerator;
};

CoapResourceDiscovery::CoapResourceDiscovery(QObject* parent)
{
    pData_ = new PrivateData(parent);
}

CoapResourceDiscovery::~CoapResourceDiscovery()
{
    if (pData_) {
        delete pData_;
        pData_ = nullptr;
    }
}

bool CoapResourceDiscovery::start(uint32_t interval)
{

    if (pData_->socket) {
        pData_->socket->close();
        delete pData_->socket;
    }

    if (pData_->timer) {
        pData_->timer->stop();
        delete pData_->timer;
    }

    pData_->socket = new QUdpSocket(this);
    if (!pData_->socket->bind()) {
        qWarning() << "Cannot bind";
        return false;
    }
    QObject::connect(pData_->socket, &QUdpSocket::readyRead, this, &CoapResourceDiscovery::onReadyRead);

    pData_->timer = new QTimer(this);
    QObject::connect(pData_->timer, &QTimer::timeout, this, &CoapResourceDiscovery::onDiscoveryTimer);
    pData_->timer->start(interval);

    return true;
}

void CoapResourceDiscovery::stop()
{
    if (pData_->socket) {
        pData_->socket->close();
        delete pData_->socket;
        pData_->socket = nullptr;
    }

    if (pData_->timer) {
        pData_->timer->stop();
        delete pData_->timer;
        pData_->timer = nullptr;
    }
}

bool CoapResourceDiscovery::addLocation(const QHostAddress &host, int port, const QString &path)
{
    QUrl url;
    url.setScheme("coap");
    url.setHost(host.toString());
    url.setPort(port);
    url.setPath(path);
    url.setQuery("");

    auto it = pData_->locations.find(url.toString());
    if (it == pData_->locations.end()) {
        LocationData *location = new LocationData;
        location->host = host;
        location->port = port;
        location->path = path;
        location->lastMassageId = static_cast<uint16_t>(pData_->randomGenerator.bounded(0x10000));
        pData_->locations.insert(url.toString(), location);
    }

    return true;
}

bool CoapResourceDiscovery::addLocation(const QString &host, int port, const QString &path)
{
    QHostAddress hostAddress;
    if (!hostAddress.setAddress(host)) {
        qCritical() << "Not a host address" << host;
        return false;
    }

    return addLocation(hostAddress, port, path);
}

void CoapResourceDiscovery::processDiscoveryResponse(const QHostAddress &sender, int port, const QByteArray &data)
{
    QVector<QCoapResource> resourceList;
    QLatin1String quote = QLatin1String("\"");
    const QList<QByteArray> links = data.split(',');

    for (QByteArray link : links) {
        QCoapResource resource;
        resource.setHost(sender);

        const QList<QByteArray> parameterList = link.split(';');
        for (QByteArray parameter : parameterList) {
            if (parameter.length() == 0) {
                continue;
            }
            QString parameterString = QString::fromUtf8(parameter);
            int length = parameterString.length();
            if (parameter.startsWith('<')) {
                resource.setPath(parameterString.mid(1, length - 2));
            } else if (parameter.startsWith("title=")) {
                resource.setTitle(parameterString.mid(6).remove(quote));
            } else if (parameter.startsWith("rt=")) {
                resource.setResourceType(parameterString.mid(3).remove(quote));
            } else if (parameter.startsWith("if=")) {
                resource.setInterface(parameterString.mid(3).remove(quote));
            } else if (parameter.startsWith("sz=")) {
                resource.setMaximumSize(parameterString.mid(3).remove(quote).toInt());
            } else if (parameter.startsWith("ct=")) {
                resource.setContentFormat(parameterString.mid(3).remove(quote).toUInt());
            } else if (parameter == "obs") {
                resource.setObservable(true);
            }
        }

        if (!resource.path().isEmpty()) {
            resourceList.push_back(resource);
        }
    }

    if (!resourceList.isEmpty()) {
        emit discovered(resourceList, sender, port);
    }

}

void CoapResourceDiscovery::processMessage(const QHostAddress &sender, int port, CoapMessage *message)
{
    if(message->type() == CoapMessage::Type::COAP_TYPE_ACKNOWLEDGEMENT) {

        CoapOption::ContentFormat format = 
                            CoapOption::ContentFormat::COAP_CONTENT_FORMAT_TEXT_PLAIN;
        QList<CoapOption*> &options = message->options();
        foreach (CoapOption *option, options) {
            if (option->name() == CoapOption::Option::COAP_OPTION_CONTENT_FORMAT) {
                format = 
                static_cast<CoapOption::ContentFormat>(qFromLittleEndian<uint16_t>(option->value()));
            }
        }

        if (format == CoapOption::ContentFormat::COAP_CONTENT_FORMAT_APP_LINKFORMAT) {
            processDiscoveryResponse(sender, port, message->payload());
        }

    } else {
        /* Some other message that we don't care for the moment */
    }
}

void CoapResourceDiscovery::onReadyRead()
{
    while (pData_->socket->hasPendingDatagrams()) {

        const auto &datagram = pData_->socket->receiveDatagram();
        CoapMessage *message = CoapMessage::createFromByteArray(datagram.data(), this);
        if (message) {
            processMessage(datagram.senderAddress(), datagram.senderPort(), message);
            delete message; 
        }   

    }
}

void CoapResourceDiscovery::onDiscoveryTimer()
{
        foreach (LocationData *location, pData_->locations) {
            QByteArray data;
            if (createDiscoveryFrame(data, location)) {
                pData_->socket->writeDatagram(data, location->host, location->port);
            }
        }
}

bool CoapResourceDiscovery::createDiscoveryFrame(QByteArray &data, LocationData* location)
{
    Q_ASSERT(location != nullptr);

    CoapMessage *message = new CoapMessage(CoapMessage::Type::COAP_TYPE_NON_CONFIRMABLE, 
                                            CoapMessage::Code::COAP_CODE_GET, this);
    message->setMessageId(location->lastMassageId);
    location->lastMassageId++;

    QStringList pathList = location->path.split(QLatin1Char('/'), Qt::SkipEmptyParts);
    foreach (QString segment, pathList) {
        CoapOption *option = new CoapOption(CoapOption::Option::COAP_OPTION_URI_PATH, message);
        QByteArray value = segment.toUtf8();
        option->setValue(value);
        message->addOption(option);
    }

    bool status = message->toByteArray(data);
    delete message;

    return status;
}