#pragma once
#include <QCborStreamWriter>

#include "smpheader.hpp"
namespace smp {

class SmpRequest 
{
    public:
    SmpRequest(uint8_t nh_op, uint8_t _res1, uint8_t nh_flags,
                uint16_t nh_len, uint16_t nh_group, 
                uint8_t nh_seq, uint8_t nh_id);

    virtual void serialize(QByteArray& writer);

    SmpHeader getResponseHdr() const;

    void setSeq(uint8_t seq);

    protected:
    SmpHeader header; 
};
} // end of namespace
