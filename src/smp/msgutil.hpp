#pragma once
#include <QCborStreamReader>
#include <QString>
#include <QByteArray>


namespace smp 
{
	
QString decodeString(QCborStreamReader &reader);

bool getString(QCborStreamReader& reader, QString& key);

QByteArray decodeByteArray(QCborStreamReader &reader);

bool getByteArray(QCborStreamReader& reader, QByteArray& array);

bool getInt(QCborStreamReader& reader, int& value);

bool getBool(QCborStreamReader& reader, bool& value);

bool checkKey(QCborStreamReader& reader, const QString& key);

}
