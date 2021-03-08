#pragma once
#include <QByteArray>
#include "smpheader.hpp"
#include "smppayload.hpp"

namespace smp {

class SmpReply 
{
public:
    enum Status 
    {
        Unknown = 0,
        Ok = 1,
        Invalid = 2,
        Invalid_Header = 3,
        Missing_Payload = 4
    };

    SmpReply(const QByteArray& data); 

    bool getPayload(SmpPayload& payload) const;

    SmpReply::Status status() const;

private:
    void deserialize(const QByteArray& data);
    SmpHeader header;
    QByteArray payload; 
    QByteArray encodedMsg;
    Status status_;

    bool checkData();
    
};

} // end of namespace 
