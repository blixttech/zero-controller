#pragma once

#include <QCborStreamWriter>
#include <QCborStreamReader>


namespace smp {

struct MgmtHeader {
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

class SmpHeader
{
public:
    SmpHeader();
    SmpHeader(uint8_t nh_op, uint8_t _res1, uint8_t nh_flags,
                uint16_t nh_len, uint16_t nh_group, 
                uint8_t nh_seq, uint8_t nh_id);

    void serialize(QByteArray& writer);
    bool deserialize(QByteArray& reader);

    MgmtHeader header;
};

} // end of namespace
