#include "msgutil.hpp"

namespace smp
{
	
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
}
