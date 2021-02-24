#include "smpclient.hpp"

#include <QByteArray> 
#include <QNetworkDatagram>

namespace smp {

SmpClient::SmpClient(const QString& smpServer, uint16_t port,
            QObject* parent) : QObject(parent),
            host(smpServer), port(port),
            seqCounter(0), socket(nullptr),
            tracker(nullptr)
{
    connect();
}

bool SmpClient::connected() const
{
    return (nullptr != socket);
}

void SmpClient::disconnect()
{
    tracker.reset();
    delete socket;
    socket = nullptr;
}

std::shared_ptr<SmpReply> SmpClient::send(SmpRequest& msg)
{
    if (nullptr != tracker)
    {
        qDebug() << "Request still in process";
        return nullptr;
    }

    if (!connected()) 
    {
        qDebug() << "Smpclient not connected";
        return nullptr;
    }

    msg.setSeq(66);

    QByteArray data;
    qDebug() << "Cbor data: " << data.length();
    msg.serialize(data);
    qDebug() << "Cbor data after: " << data.length();

    socket->write(data);
    tracker = std::make_shared<SmpReply>(msg.getResponseHdr(), this);
    QObject::connect(tracker.get(), &SmpReply::finished, this, &SmpClient::requestCompleted);
    return tracker;
    // TODO
    // 1. track data from socket 
    // check for valid mgmtheader
    // and if so, fwd to reply
    // 2. connect to reply object's delete signal 
    // release tracker if
//    return tracker; */
}

void SmpClient::connect()
{
    if (nullptr != socket) disconnect();

    socket = new QUdpSocket(this);
    QObject::connect(socket, &QUdpSocket::readyRead, this, &SmpClient::onReadyRead);

    if (!socket->bind()) {
        qWarning() << "SmpClient: Cannot bind to Zero";
        return;
    }

    socket->connectToHost(host, port);

}

void SmpClient::onReadyRead()
{
    qDebug() << "GOT UDP DATA";
    auto datagram = socket->receiveDatagram();
    if (nullptr == tracker) return;

    tracker->addData(datagram.data());
} 

void SmpClient::requestCompleted()
{
    qDebug() << "Smp request completed";
    QObject::disconnect(tracker.get(), &SmpReply::finished, this, &SmpClient::requestCompleted);
    tracker.reset();
}

} // end of namespace
