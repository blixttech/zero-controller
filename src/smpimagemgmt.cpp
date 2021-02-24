#include "smpimagemgmt.hpp"
#include <QCborStreamReader>

namespace smp {

ImageMgmtStateReq::ImageMgmtStateReq() :
    SmpRequest(MgmtOp::READ, 0, 0,
        1, MgmtGroupId::ID_IMAGE,
        0, ImgMgmtId::ID_STATE)
{
}

void ImageMgmtStateReq::serialize(QByteArray& writer)
{
    SmpRequest::serialize(writer);

    // this request seem to require a dummy payload
    writer.append(0xa0);
}

QString decodeString(QCborStreamReader &reader)
{
   QString result;
   auto r = reader.readString();
   while (r.status == QCborStreamReader::Ok) {
       result += r.data;
       r = reader.readString();
   }

   if (r.status == QCborStreamReader::Error) {
       // handle error condition
       result.clear();
   }
   return result;
}

bool getString(QCborStreamReader& reader, QString& key)
{
    if (!reader.isString()) return false;
    key = decodeString(reader);
    return true;
}

QByteArray decodeByteArray(QCborStreamReader &reader)
{
   QByteArray result;
   auto r = reader.readByteArray();
   while (r.status == QCborStreamReader::Ok) {
       result += r.data;
       r = reader.readByteArray();
   }

   if (r.status == QCborStreamReader::Error) {
       // handle error condition
       result.clear();
   }
   return result;
}

bool getByteArray(QCborStreamReader& reader, QByteArray& array)
{
    if (!reader.isByteArray()) return false;
    array = decodeByteArray(reader);
    return true;
}

bool getInt(QCborStreamReader& reader, int& value)
{
    if (!reader.isInteger()) return false;
    value = reader.toInteger();
    reader.next();
    return true;
}

bool getBool(QCborStreamReader& reader, bool& value)
{
    if (!reader.isBool()) return false;
    value = reader.toBool();
    reader.next();
    return true;
}

bool checkKey(QCborStreamReader& reader, const QString& key)
{
    QString kv;
    if (!getString(reader,kv)) return false;
    qDebug() << "Got key " << kv;
    if (kv != key) return false;
    return true;
}

bool deserializeImageSlot(QCborStreamReader& reader, ImageSlot& slot)
{
    if (!reader.isMap())
        return false;

    reader.enterContainer();

    while (reader.hasNext())
    {
        QString kv;
        if (!getString(reader, kv)) return false;
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
    
    if (reader.lastError() != QCborError::NoError) return false;

    reader.leaveContainer();
    qDebug() << "Completed parsing ImageSlot";
    
    return true;
}

bool ImageMgmtStatePayload::deserialize(QByteArray& data)
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
   
    if (!reader.isArray())
       return false; 
    
    reader.enterContainer();

    for (int i = 0; (i < 2) && reader.hasNext(); ++i)
    {
        ImageSlot sl;
        qDebug() << "Parsing ImageSlot " << i;
        if (!deserializeImageSlot(reader, sl)) return false;
        fwSlots.push_back(sl);
    }
    
    if (reader.lastError() != QCborError::NoError) return false;

    reader.leaveContainer();

    return true;
}

} // end of namespace
