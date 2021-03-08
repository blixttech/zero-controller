#pragma once

#include "smp.hpp"
#include "smprequest.hpp"
#include "smpreply.hpp"

#include <QObject>
#include <QHostAddress>
#include <QUdpSocket>

namespace smp {

class SmpClient : public QObject
{
    Q_OBJECT

public:
    SmpClient(const QString& smpServer, uint16_t port = 1337,
                QObject* parent = nullptr);

    // sends the request
    // returns a pair of <send successful, seq numb>
    // if send successful, the seq number is the valid and will be in the matching reply
    std::pair<bool,uint8_t> send(SmpRequest& msg);

    bool connect();
    void disconnect();
    bool connected() const;

signals:
    void replyReceived(std::shared_ptr<SmpReply> reply);
    void connectionEstablished();

private:
    void onReadyRead();

    QHostAddress host;
    uint16_t port;

    uint8_t seqCounter;
    
    std::unique_ptr<QUdpSocket> socket;
    
};

} // end of namespace
