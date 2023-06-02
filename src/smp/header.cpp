#include "header.hpp"

#include <QByteArray>

#ifdef USE_WINSOCK2
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif


namespace smp {

Header::Header() :
    header { 0, 0, 0, 0, 0, 0, 0 }
{
}

Header::Header(uint8_t nh_op, uint8_t _res1, uint8_t nh_flags,
                uint16_t nh_len, uint16_t nh_group, 
                uint8_t nh_seq, uint8_t nh_id) :
    header{ nh_op = nh_op, _res1 = _res1, nh_flags = nh_flags,
            nh_len = nh_len, nh_group = nh_group,
            nh_seq = nh_seq, nh_id = nh_id }

{
}

Header::Header(QByteArray& data)
{
    deserialize(data);
}
    
void Header::serialize(QByteArray& writer)
{
    // assuming that the payload was already written to the writer
    header.nh_len = writer.length();
    qDebug() << "Payload len when sending: " << header.nh_len;
        
    SmpHeader hdr = header;
    hdr.nh_len = htons(header.nh_len);
    hdr.nh_group = htons(header.nh_group);

    writer.prepend(reinterpret_cast<char*>(&hdr),sizeof(SmpHeader));
}

bool Header::deserialize(QByteArray& reader)
{
    if (reader.length() < sizeof(SmpHeader)) return false;

    std::copy(reader.constData(), reader.constData() + sizeof(SmpHeader), reinterpret_cast<char*>(&header));

    header.nh_len = ntohs(header.nh_len);
    header.nh_group = ntohs(header.nh_group);
    qDebug() << "Len: " << header.nh_len << " Group: " << header.nh_group;

    reader.remove(0, sizeof(SmpHeader));

    return true;
}

uint32_t Header::msgType() const
{
    return ((header.nh_op << 24) | (header.nh_group << 8) | header.nh_id);
}

    
} // end of namespace
