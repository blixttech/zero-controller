#pragma once
#include <QObject>
#include <QByteArray>
#include "smpheader.hpp"
#include "smppayload.hpp"

namespace smp {

class SmpReply : public QObject
{
    Q_OBJECT
public:
    SmpReply(SmpHeader reqHdr, QObject* parent = nullptr); 

    bool deserialize(QByteArray& data, bool checkHeader = true);

    void addData(const QByteArray& data);

    bool getPayload(SmpPayload& payload);

signals:
    void finished();

private:
    SmpHeader header;
    QByteArray payload; 
    QByteArray encodedMsg;

    bool checkData();
    
};

} // end of namespace 
