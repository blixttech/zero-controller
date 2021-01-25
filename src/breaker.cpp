#include "breaker.hpp"
#include <QCoapClient>
#include <QCoapReply>
#include <QtEndian>
#include <QVariant>

class Breaker::PrivateData : QObject
{
public:
    PrivateData(QObject *parent = nullptr) : QObject(parent)
    {
        coapClient = new QCoapClient(QtCoap::SecurityMode::NoSecurity, parent);
    }

    ~PrivateData()
    {
        if (coapClient) {
            delete coapClient;
            coapClient = nullptr;
        }
    }

    QCoapClient *coapClient;
    QMap<QString, QVariant> properties;
};

Breaker::Breaker(QObject *parent) : QObject(parent)
{
    pData_ = new PrivateData(parent);
}

Breaker::~Breaker()
{
    if (pData_ != nullptr) {
        delete pData_;
        pData_ = nullptr;
    }
}

Breaker* Breaker::create(QCoapReply *reply, QObject *parent)
{
    Q_ASSERT(reply != nullptr);

    Breaker* breaker = new Breaker(parent);
    if (!validateVersion(reply, breaker->pData_->properties)) {
        delete breaker;
        breaker = nullptr;
        return nullptr;
    }

    return breaker;
}

bool Breaker::validateVersion(QCoapReply *reply, QMap<QString, QVariant> &properties)
{
    Q_ASSERT(reply != nullptr);
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

    QUrl url;
    url.setScheme("coap");
    url.setHost(reply->url().host());
    url.setPort(reply->url().port());
    properties["url"] = url;

    properties["uuid"] = QString::fromUtf8(values[0]);
    properties["macaddress"] = QString::fromUtf8(values[1]);

    QString hwVersion;
    hwVersion = QString::fromUtf8(values[2]) + "." + 
                QString::fromUtf8(values[3]) + "." + 
                QString::fromUtf8(values[4]);
    properties["hwversion"] = hwVersion;
    
    return true;
}

QHostAddress Breaker::host()
{
    if (pData_->properties.contains("url")) {
        return QHostAddress(pData_->properties.value("url").toUrl().host());
    }
    return QHostAddress();
}

QUrl Breaker::url()
{
    if (pData_->properties.contains("url")) {
        return pData_->properties.value("url").toUrl();
    }
    return QUrl();
}

QString Breaker::uuid()
{
    if (pData_->properties.contains("uuid")) {
        return pData_->properties.value("uuid").toString();
    }
    return "";
}

QString Breaker::hardwareVersion()
{
    if (pData_->properties.contains("hwversion")) {
        return pData_->properties.value("hwversion").toString();
    }
    return "";
}

QString Breaker::hardwareAddress()
{
    if (pData_->properties.contains("macaddress")) {
        return pData_->properties.value("macaddress").toString();
    }
    return "";
}