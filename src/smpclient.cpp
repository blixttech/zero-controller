#include "smpclient.hpp"

#include <QByteArray> 
#include <QNetworkDatagram>

namespace smp {

SmpClient::SmpClient(const QString& smpServer, uint16_t port,
            QObject* parent) : QObject(parent),
            host(smpServer), port(port),
            seqCounter(1), socket()
{
}

bool SmpClient::connected() const
{
    return (nullptr != socket);
}

void SmpClient::disconnect()
{
    socket.reset();
}

std::pair<bool, uint8_t> SmpClient::send(SmpRequest& msg)
{
    if (!connected()) 
    {
        qDebug() << "Smpclient not connected";
        return std::make_pair(false, 0);
    }

    msg.setSeq(seqCounter);
    seqCounter++;

    QByteArray data;
    msg.serialize(data);

    socket->write(data);
    return std::make_pair(true, seqCounter);
}

bool SmpClient::connect()
{
    if (connected()) disconnect();

    socket = std::make_unique<QUdpSocket>(this);
    QObject::connect(socket.get(), &QUdpSocket::readyRead, this, &SmpClient::onReadyRead);
    QObject::connect(socket.get(), &QUdpSocket::connected, this, &SmpClient::connectionEstablished);

    if (!socket->bind()) {
        qWarning() << "SmpClient: Cannot bind to port";
        return false;
    }

    socket->connectToHost(host, port);
    return true;
}

void SmpClient::onReadyRead()
{
    auto datagram = socket->receiveDatagram();
    auto rply = std::make_shared<SmpReply>(datagram.data());
    emit replyReceived(rply);
} 

} // end of namespace
