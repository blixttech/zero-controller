#include "smpreply.hpp"

#include <QCborStreamReader>

namespace smp {

SmpReply::SmpReply(SmpHeader reqHdr, QObject* parent) : 
    QObject(parent),   
    header(reqHdr),
    payload(),
    encodedMsg()
{
}

void printHdr(MgmtHeader& thd)
{
    qDebug() << "nh_op: " << thd.nh_op << " "
             << "nh_group: " << thd.nh_group << " "
             << "nh_seq: " << thd.nh_seq << " "
             << "nh_id: " << thd.nh_id;
}

bool SmpReply::deserialize(QByteArray& data, bool checkHeader)
{
    SmpHeader tHdr;

    if (!tHdr.deserialize(data)) 
    {
        qDebug() << "SmpRely: Invalid header";
        return false;
    }
    
    auto &thd = tHdr.header;
    auto &rhd = header.header;
    if (checkHeader)
    {
        if ((thd.nh_op != rhd.nh_op) ||
            (thd.nh_group != rhd.nh_group) ||
            (thd.nh_seq != rhd.nh_seq) ||
            (thd.nh_id != rhd.nh_id))
        {
            qDebug() << "SmpRely: Packet not matching expect header";
            qDebug() << "Received:";
            printHdr(thd);
            qDebug() << "Expected:";
            printHdr(rhd);        
            return false;
        }
    }
    rhd = thd;

    if (data.length() < rhd.nh_len)
    {
        qDebug() << "Payload is missing data";
        return false;
    } 
   
    qDebug() << "Completed the message parsing"; 
    payload.append(data.data()+sizeof(rhd), rhd.nh_len); 
    return true;
}

void SmpReply::addData(const QByteArray& data)
{
    encodedMsg.append(data); 
    if (!deserialize(encodedMsg)) return;
    emit finished();
    
}

bool SmpReply::getPayload(SmpPayload& pyl)
{
    return pyl.deserialize(payload);
}

} // end of namespace
