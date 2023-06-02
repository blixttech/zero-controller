#include "imagemgmt.hpp"
#include <QCborStreamReader>
#include <QCborStreamWriter>
#include "msgutil.hpp"

namespace smp {
    
bool deserializeImageSlot(QCborStreamReader& reader, ImageSlot& slot)
{
    if (!reader.isMap())
    {
        qDebug() << "No map";
        return false;
    }

    reader.enterContainer();

    while (reader.hasNext())
    {
        QString kv;
        if (!getString(reader, kv)) 
        {
            qDebug() << "Cannot get key";
            return false;
        }
        
        qDebug() << "ImageSlot Key: " << kv;    
        if ("slot" == kv)
        {
            if (!getInt(reader, slot.slot)) return false;
        }
        else if ("version" == kv)
        {
            if (!getString(reader, slot.version)) return false;
        }
        else if ("hash" == kv)
        {
            if (!getByteArray(reader, slot.hash)) return false;
        }
        else if ("bootable" == kv)
        {
            if (!getBool(reader, slot.bootable)) return false;
        }
        else if ("pending" == kv)
        {
            if (!getBool(reader, slot.pending)) return false;
        }
        else if ("confirmed" == kv)
        {
            if (!getBool(reader, slot.confirmed)) return false;
        }
        else if ("active" ==  kv)
        {
            if (!getBool(reader, slot.active)) return false;
        }
        else if ("permanent" == kv)
        {
            if (!getBool(reader, slot.permanent)) return false;
        }
    }
    
    if (reader.lastError() != QCborError::NoError) 
    {
        qDebug() << "Had an error";
        return false;
    }

    reader.leaveContainer();
    qDebug() << "Completed parsing ImageSlot";
    
    return true;
}
    
bool deserializeStateOfImagesResp(const QByteArray& data, std::vector<ImageSlot>& imagesVec)
{
    QCborStreamReader reader(data);
    /* Layout
     *  images: Array[2] 
     *  {
     *      map {
     *      slot: int,
     *      version: string,
     *      hash: string,
     *      bootable: bool,
     *      pending: bool,
     *      confirmed: bool,
     *      active: bool,
     *      permanent: bool
     *      },
     *      map {
     *      slot: int,
     *      version: string,
     *      hash: string,
     *      bootable: bool,
     *      pending: bool,
     *      confirmed: bool,
     *      active: bool,
     *      permanent: bool
     *      }
     *  }
     */

    if (!reader.isMap())
    {
        qWarning() << "Invalid payload for SMP response";
        return false;
    }
    
    reader.enterContainer();
    
    QString kv;
    if (!getString(reader, kv)) 
    {
        qWarning() << "Cannot get key 'images' key";
        return false;
    }
    if ("images" != kv)
    {
        qWarning() << "'images' key not present in SMP repsonse";
        return false;   
    }
    
    if (!reader.isArray())
    {
        qWarning() << "No array";
        return false; 
    }
    
    reader.enterContainer();

    for (int i = 0; (i < 2) && reader.hasNext(); ++i)
    {
        ImageSlot sl;
        qDebug() << "Parsing ImageSlot " << i;
        if (!deserializeImageSlot(reader, sl)) return false;
        imagesVec.push_back(sl);
    }
    
    if (reader.lastError() != QCborError::NoError) return false;

    reader.leaveContainer();
    reader.leaveContainer();

    return true;
}
    
GetStateOfImagesReq::GetStateOfImagesReq() :
    Request(MgmtOp::READ, 0, 0,
        1, MgmtGroupId::ID_IMAGE,
        0, ImgMgmtId::ID_STATE)
{
}

void GetStateOfImagesReq::serialize(QByteArray& writer)
{

    // this request seem to require a dummy payload
    writer.append(0xa0);

    // always needs to come last as it needs to 
    // know the payload length
    Request::serialize(writer);
}

GetStateOfImagesResp::GetStateOfImagesResp(const Header& header, const QByteArray& payload) :
    Response(header),
    imagesVec()
{
    if (!deserialize(payload))
    {
        throw std::runtime_error("Invalid GetStateOfImagesResp");        
    }
}

bool GetStateOfImagesResp::deserialize(const QByteArray& data)
{
    return deserializeStateOfImagesResp(data, imagesVec);
}

SetStateOfImagesReq::SetStateOfImagesReq(bool confirm, const QByteArray& hash) :
    Request(MgmtOp::WRITE, 0, 0,
        1, MgmtGroupId::ID_IMAGE,
        0, ImgMgmtId::ID_STATE),
    confirm(confirm),
    hash(hash)
{
}

void SetStateOfImagesReq::serialize(QByteArray& writer)
{
    QCborStreamWriter cwrite(&writer);
    cwrite.startMap();
    cwrite.append("confirm");
    cwrite.append(confirm);

    if (confirm && (hash.size() != 0))
    {
        cwrite.append("hash");
        cwrite.append(hash);
    }
    
    cwrite.endMap();

    // always needs to come last as it needs to 
    // know the payload length
    Request::serialize(writer);
}

SetStateOfImagesResp::SetStateOfImagesResp(const Header& header, const QByteArray& payload) :
    Response(header),
    imagesVec()
{
    if (!deserialize(payload))
    {
        throw std::runtime_error("Invalid SetStateOfImagesResp");        
    }
}

bool SetStateOfImagesResp::deserialize(const QByteArray& data)
{
    return deserializeStateOfImagesResp(data, imagesVec);
}

ImageUploadReq::ImageUploadReq(uint8_t image, uint32_t off, const QByteArray& data, 
        const QByteArray& sha,
        uint32_t length,
        bool upgrade) :
    Request(MgmtOp::WRITE, 0, 0,
        1, MgmtGroupId::ID_IMAGE,
        0, ImgMgmtId::ID_UPLOAD),
    image(image),
    off(off),
    data(std::move(data)),
    sha(std::move(sha)),
    len(length),
    upgrade(upgrade)
{
}

void ImageUploadReq::serialize(QByteArray& writer)
{
    QCborStreamWriter cwrite(&writer);

    cwrite.startMap();
        
    cwrite.append("image");
    cwrite.append(image);

    if (0 == off)
    {

        cwrite.append("len");
        cwrite.append(len);
    }

    cwrite.append("off");
    cwrite.append(off);

    
    if (0 == off)
    {
        cwrite.append("sha");
        cwrite.append(sha);

//        cwrite.append("upgrade");
//        cwrite.append(upgrade);
    }

    cwrite.append("data");
    cwrite.append(data);
    cwrite.endMap();
        
    // always needs to come last as it needs to 
    // know the payload length
    Request::serialize(writer);
}

ImageUploadResp::ImageUploadResp(const Header& header, const QByteArray& payload) :
    Response(header),
    _off(0),
    _match(false),
    _rc(0),
    _rsn("")
{
    if (!deserialize(payload))
    {
        throw std::runtime_error("Invalid ImageUploadResp");        
    }
}

bool ImageUploadResp::deserialize(const QByteArray& payload)
{
    QCborStreamReader reader(payload);
    
    if (!reader.isMap())
    {
        qWarning() << "Invalid payload for SMP response";
        return false;
    }
    
    reader.enterContainer();
    
    while (reader.hasNext())
    {
        QString kv;
        if (!getString(reader, kv)) 
        {
            qWarning() << "Cannot get key";
            return false;
        }
        
        qDebug() << "ImageUploadResp Key: " << kv;    
        if ("rc" == kv)
        {
            if (!getInt(reader, _rc)) return false;
        }
        else if ("rsn" == kv)
        {
            if (!getString(reader, _rsn)) return false;
        }
        else if ("off" == kv)
        {
            if (!getInt(reader, _off)) return false;
        }
        else if ("match" == kv)
        {
            if (!getBool(reader, _match)) return false;                
        }
    }
    reader.leaveContainer();
    return true;
}

} // end of namespace
