#include "request.hpp"
#include "smp.hpp"

namespace smp {

Request::Request(uint8_t nh_op, uint8_t _res1, uint8_t nh_flags,
                uint16_t nh_len, uint16_t nh_group, 
                uint8_t nh_seq, uint8_t nh_id) :
                header(nh_op, _res1, nh_flags,
                    nh_len, nh_group, nh_seq, nh_id)
{
}

void Request::setSeq(uint8_t seq)
{
    header.setSeq(seq);
}

void Request::serialize(QByteArray& writer)
{
   header.serialize(writer); 
}

Header Request::getResponseHdr() const
{
    Header resp((MgmtOp::READ == header.op()) ? MgmtOp::READ_RSP : MgmtOp::WRITE_RSP,
                    0, 0, 0, header.group(), header.seq(), header.id());
    return resp;
}

} // end of namespace
