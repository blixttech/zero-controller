#pragma once
#include <QByteArray>
#include "header.hpp"

namespace smp {

class Response 
{
public:
    Response(const Header& header); 

    virtual bool deserialize(const QByteArray& data) = 0;

private:
    Header header;
};

} // end of namespace 
