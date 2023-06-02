#pragma once

#include <sys/types.h>
#include <cstdint>
#include <QByteArray>
#include <QCborStreamWriter>
#include <QCborStreamReader>


namespace smp {

struct SmpHeader {
#ifdef USE_LITTLE_ENDIAN
    uint8_t  nh_op:3;           /* MGMT_OP_[...] */
    uint8_t  _res1:5;
#endif
#ifdef USE_BIG_ENDIAN
    uint8_t  _res1:5;
    uint8_t  nh_op:3;           /* MGMT_OP_[...] */
#endif
    uint8_t  nh_flags;          /* Reserved for future flags */
    uint16_t nh_len;            /* Length of the payload */
    uint16_t nh_group;          /* MGMT_GROUP_ID_[...] */
    uint8_t  nh_seq;            /* Sequence number */
    uint8_t  nh_id;             /* Message ID within group */
};

class Header
{
    public:
    Header();
    Header(uint8_t nh_op, uint8_t _res1, uint8_t nh_flags,
                uint16_t nh_len, uint16_t nh_group, 
                uint8_t nh_seq, uint8_t nh_id);

    Header(QByteArray& data);

    void serialize(QByteArray& writer);

    // Note: If successful deserialised,
    // the number of used bytes is removed
    // from the reader
    bool deserialize(QByteArray& reader);

    uint8_t op()     const    { return header.nh_op; }
    
    uint8_t group()  const    { return header.nh_group; }
    uint8_t id()     const    { return header.nh_id; }

    uint8_t seq()    const    { return header.nh_seq; }
    uint8_t len()    const    { return header.nh_len; }
    
    void setSeq(uint8_t seq) { header.nh_seq = seq; }
    void setLen(uint16_t len) { header.nh_len = len; }
    uint32_t msgType() const;

    private:
    SmpHeader header;
};

} // end of namespace
