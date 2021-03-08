#include "smpreply.hpp"

#include <QCborStreamReader>

namespace smp {

SmpReply::SmpReply(const QByteArray& data) : 
    encodedMsg(data),
    status_(Status::Unknown)
{
    deserialize(data);
}

void printHdr(MgmtHeader& thd)
{
    qDebug() << "nh_op: " << thd.nh_op << " "
             << "nh_group: " << thd.nh_group << " "
             << "nh_seq: " << thd.nh_seq << " "
             << "nh_id: " << thd.nh_id;
}

void SmpReply::deserialize(const QByteArray& di)
{
    QByteArray data(di);
    auto &rhd = header.header;
    if (!header.deserialize(data)) 
    {
        qDebug() << "SmpReply: Invalid header";
        status_ = Status::Invalid_Header;
        return;
    } 

    if (data.length() < rhd.nh_len)
    {
        qDebug() << "Payload is missing data";
        status_ = Status::Missing_Payload;
        return;
    } 
   
    qDebug() << "Completed the message parsing"; 
    payload.append(data.data()+sizeof(rhd), rhd.nh_len);
    status_ = Status::Ok; 
}

bool SmpReply::getPayload(SmpPayload& pyl) const
{
    return pyl.deserialize(payload);
}

SmpReply::Status SmpReply::status() const
{
    return status_;
}

} // end of namespace
