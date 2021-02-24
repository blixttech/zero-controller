#include "smprequest.hpp"
#include "smp.hpp"

namespace smp {

SmpRequest::SmpRequest(uint8_t nh_op, uint8_t _res1, uint8_t nh_flags,
                uint16_t nh_len, uint16_t nh_group, 
                uint8_t nh_seq, uint8_t nh_id) :
                header(nh_op, _res1, nh_flags,
                    nh_len, nh_group, nh_seq, nh_id)
{
}

void SmpRequest::setSeq(uint8_t seq)
{
    header.header.nh_seq = seq;
}

void SmpRequest::serialize(QByteArray& writer)
{
   header.serialize(writer); 
}

SmpHeader SmpRequest::getResponseHdr() const
{
    auto &hdr = header.header;
    SmpHeader resp((MgmtOp::READ == hdr.nh_op) ? MgmtOp::READ_RSP : MgmtOp::WRITE_RSP,
                    0, 0, 0, hdr.nh_group, hdr.nh_seq, hdr.nh_id);
    return resp;
}

} // end of namespace
