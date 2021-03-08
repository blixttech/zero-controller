#pragma once
#include <QByteArray>

namespace smp {

class SmpPayload
{
public:
    virtual bool deserialize(const QByteArray& data) = 0;
};

} // end namespace
