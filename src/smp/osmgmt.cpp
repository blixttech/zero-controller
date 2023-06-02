#include "osmgmt.hpp"
#include <QCborStreamReader>
#include <QCborStreamWriter>
#include "msgutil.hpp"

namespace smp {
    
ResetReq::ResetReq(bool force) :
    Request(MgmtOp::WRITE, 0, 0,
        1, MgmtGroupId::ID_OS,
        0, OsMgmtId::ID_RESET),
    force(force)
{
}

void ResetReq::serialize(QByteArray& writer)
{

    QCborStreamWriter cwrite(&writer);
    cwrite.startMap();
    if (force)
    {
        cwrite.append("force");
        cwrite.append(1);        
    }
    cwrite.endMap();

    // always needs to come last as it needs to 
    // know the payload length
    Request::serialize(writer);
}

ResetResp::ResetResp(const Header& header, const QByteArray& payload) :
    Response(header),
    m_rc(0)
{
    if (!deserialize(payload))
    {
        throw std::runtime_error("Invalid ResetResp");        
    }
}

bool ResetResp::deserialize(const QByteArray& data)
{
    QCborStreamReader reader(data);
        
    if (!reader.isMap())
    {
        qWarning() << "Invalid payload for SMP response";
        return false;
    }
    
    reader.enterContainer();
    if (!reader.hasNext())
    {
        reader.leaveContainer();
        return true;
    }
            
    QString kv;
    if (!getString(reader, kv)) 
    {
        qWarning() << "Cannot get key 'rc' key";
        return false;
    }
    if ("rc" != kv)
    {
        qWarning() << "'rc' key not present in SMP repsonse";
        return false;   
    }

    if (!getInt(reader, m_rc)) return false;
    
    reader.leaveContainer();
    return true;
}

} // end of namespace
