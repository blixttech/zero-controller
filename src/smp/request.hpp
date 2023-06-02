#pragma once
#include <QCborStreamWriter>

#include "header.hpp"
namespace smp {

class Request 
{
    public:
    Request(uint8_t nh_op, uint8_t _res1, uint8_t nh_flags,
                uint16_t nh_len, uint16_t nh_group, 
                uint8_t nh_seq, uint8_t nh_id);

    virtual void serialize(QByteArray& writer);

    Header getResponseHdr() const;

    void setSeq(uint8_t seq);

    protected:
    Header header; 
};
} // end of namespace
