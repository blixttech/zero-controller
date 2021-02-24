#pragma once
#include <QByteArray>

namespace smp {

class SmpPayload
{
public:
    virtual bool deserialize(QByteArray& data) = 0;
};

} // end namespace
