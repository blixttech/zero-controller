#include "breakermanager.hpp"
#include "coapresourcediscovery.hpp"
#include "breaker.hpp"
#include <QCoapClient>
#include <QCoapReply>
#include <QUrl>
#include <QMap>
#include <QSet>

class BreakerManager::PrivateData : QObject
{
public:
    PrivateData(QObject *parent=nullptr) : QObject(parent)
    {
        resourceDiscovery = new CoapResourceDiscovery(parent);
        coapClient = new QCoapClient(QtCoap::SecurityMode::NoSecurity, parent);
    }

    ~PrivateData()
    {
        if (resourceDiscovery != nullptr) {
            delete resourceDiscovery;
            resourceDiscovery = nullptr;
        }

        if (coapClient) {
            delete coapClient;
            coapClient = nullptr;
        }

        foreach (Breaker *breaker, breakers) {
            delete breaker;
            breaker = nullptr;
        }

        foreach (QCoapReply* reply, tobeAdded) {
            delete reply;
            reply = nullptr;
        }
    }

    CoapResourceDiscovery *resourceDiscovery;
    QCoapClient *coapClient;
    QMap<QString, Breaker*> breakers; // IP:port address is the key
    QMap<QString, QCoapReply*> tobeAdded;
};

BreakerManager::BreakerManager(QObject *parent) : QObject(parent)
{
    pData_ = new PrivateData(this);
    QObject::connect(pData_->resourceDiscovery, &CoapResourceDiscovery::discovered, 
                    this, &BreakerManager::onCoapDiscovered);
    QObject::connect(pData_->coapClient, &QCoapClient::finished, 
                    this, &BreakerManager::onVersionReply);
    QObject::connect(pData_->coapClient, &QCoapClient::error, 
                    this, &BreakerManager::onVersionError);
}

BreakerManager::~BreakerManager()
{
    if (pData_ != nullptr) {
        delete pData_;
        pData_ = nullptr;
    }
}

void BreakerManager::onCoapDiscovered(const QVector<QCoapResource> &resources, 
                                        const QHostAddress &host, int port)
{
    QUrl url;
    url.setScheme("coap");
    url.setHost(host.toString());
    url.setPort(port);

    if (pData_->breakers.contains(url.toString())) {
        qDebug() << "Already managed" << url.toString();
        return;
    }

    bool versionFound = false;
    foreach (QCoapResource resource, resources) {
        if (resource.path().compare("/version")) {
            versionFound = true;
        }
    }

    if (versionFound) {
        url.setPath("/version");
        pData_->coapClient->get(url);
    }
}

void BreakerManager::onVersionReply(QCoapReply *reply)
{
    if (reply->errorReceived() == QtCoap::Error::Ok) {

        Breaker* breaker = Breaker::create(reply, this);
        if (breaker) {
            if (pData_->breakers.contains(breaker->url().toString())) {
                qDebug() << "Already managed" << breaker->url().toString();
                delete breaker;
                breaker = nullptr;
                return;
            }

            pData_->breakers.insert(breaker->url().toString(), breaker);

            const QMetaMethod signal = QMetaMethod::fromSignal(&BreakerManager::breakerFound);
            if (QObject::isSignalConnected(signal)) {
                emit breakerFound(breaker);
            } else {
                delete breaker;
                breaker = nullptr;
            }
        }
        reply->deleteLater();
    }
}

void BreakerManager::onVersionError(QCoapReply *reply, QtCoap::Error error)
{
    auto it = pData_->tobeAdded.begin();
    for (; it != pData_->tobeAdded.end(); it++) {
        if (it.value() == reply) {
            QHostAddress host(reply->url().host());
            emit breakerNotFound(host, reply->url().port());
            pData_->tobeAdded.erase(it);
            reply->deleteLater();
            break;
        }
    }
}

void BreakerManager::searchStart(uint32_t interval)
{
    pData_->resourceDiscovery->start(interval);
}

void BreakerManager::searchStop()
{
    pData_->resourceDiscovery->stop();
}

void BreakerManager::addToSearch(QHostAddress &host, int port)
{
    pData_->resourceDiscovery->addLocation(host, port);
}

void BreakerManager::addBreaker(QHostAddress &host, int port)
{
    QUrl url;
    url.setScheme("coap");
    url.setHost(host.toString());
    url.setPort(port);
    url.setPath("/version");

    if (pData_->tobeAdded.contains(url.toString())) {
        qDebug() << "Already waiting to be added" << url.toString();
        return;
    }

    QCoapReply *reply = pData_->coapClient->get(url);
    pData_->tobeAdded.insert(url.toString(), reply);
}

QList<Breaker*> BreakerManager::breakers() const
{
    return pData_->breakers.values();
}

void BreakerManager::removeBreaker(Breaker *breaker)
{
    Q_ASSERT(breaker != nullptr);
    if (pData_->breakers.contains(breaker->url().toString())) {
        pData_->breakers.remove(breaker->url().toString());
        delete breaker;
        breaker = nullptr;
    }
}