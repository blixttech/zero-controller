#include "response.hpp"

#include <QCborStreamReader>
#include "header.hpp"

namespace smp {

Response::Response(const Header& hdr) : 
    header(hdr)
{
}


} // end of namespace
