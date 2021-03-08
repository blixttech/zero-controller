#include "smpheader.hpp"

#include <QByteArray>

#include <arpa/inet.h>


namespace smp {

SmpHeader::SmpHeader() :
    header { 0, 0, 0, 0, 0, 0, 0 }
{
}

SmpHeader::SmpHeader(uint8_t nh_op, uint8_t _res1, uint8_t nh_flags,
                uint16_t nh_len, uint16_t nh_group, 
                uint8_t nh_seq, uint8_t nh_id) :
    header{ nh_op = nh_op, _res1 = _res1, nh_flags = nh_flags,
            nh_len = nh_len, nh_group = nh_group,
            nh_seq = nh_seq, nh_id = nh_id }

{
}

void SmpHeader::serialize(QByteArray& writer)
{
    MgmtHeader hdr = header;
    hdr.nh_len = htons(header.nh_len);
    hdr.nh_group = htons(header.nh_group);

    writer.append(reinterpret_cast<char*>(&hdr),sizeof(MgmtHeader));
}

bool SmpHeader::deserialize(QByteArray& reader)
{
    if (reader.length() < sizeof(MgmtHeader)) return false;

    std::copy(reader.data(), reader.data() + sizeof(MgmtHeader), reinterpret_cast<char*>(&header));

    qDebug() << "Len: " << header.nh_len << " Group: " << header.nh_group;
    header.nh_len = ntohs(header.nh_len);
    header.nh_group = ntohs(header.nh_group);
    qDebug() << "Len: " << header.nh_len << " Group: " << header.nh_group;

    reader.remove(0, sizeof(MgmtHeader));

    return true;
}

} // end of namespace
