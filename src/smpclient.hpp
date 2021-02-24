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

        std::shared_ptr<SmpReply> send(SmpRequest& msg);

        bool connected() const;

    private:
        void connect();
        void disconnect();

        void onReadyRead();
        void requestCompleted();


        QHostAddress host;
        uint16_t port;

        uint8_t seqCounter;
        
        QUdpSocket* socket;
        std::shared_ptr<SmpReply> tracker;
        
};

} // end of namespace
