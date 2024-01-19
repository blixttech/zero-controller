#include "networkinterfacelist.hpp"
#include <QNetworkInterface>


namespace zero {

NetworkInterfaceList::NetworkInterfaceList(QObject *parent) : 
    QAbstractListModel(parent),
    interfaces()

{
    auto interfaceList = QNetworkInterface::allInterfaces();

    for (auto i = interfaceList.begin(); i != interfaceList.end(); ++i)
    {
        if (!((QNetworkInterface::Ethernet == i->type()) 
                || (QNetworkInterface::Virtual == i->type())
                || (QNetworkInterface::Wifi == i->type())
            ))
            continue;

        int mask = QNetworkInterface::IsUp | QNetworkInterface::CanBroadcast;
        if (i->flags() & mask == 0) continue;

        if (i->addressEntries().size() == 0) continue; 

        auto addresses = i->addressEntries();
        // searching for a valid IPv4 address
        auto addr = findValidIPv4Address(addresses);
        
        if (addr.ip().isNull()) continue; //no valid address, skip interface
            
        interfaces.append(*i);
    }
}

int NetworkInterfaceList::rowCount(const QModelIndex &parent) const
{
    return interfaces.size()+1;
}

QNetworkAddressEntry NetworkInterfaceList::findValidIPv4Address( QList<QNetworkAddressEntry>&  addresses) const
{
    for (int i = 0; i < addresses.size(); ++i)
    {
        auto ipAddr = addresses[i].ip();
        if ((ipAddr.protocol() == QAbstractSocket::IPv4Protocol) 
            && (ipAddr.isGlobal()))
        return addresses[i]; // we found a valid addres
    }
    return QNetworkAddressEntry(); // no valid address, return null 
} 

QVariant NetworkInterfaceList::data(const QModelIndex &index, int role) const
{
    if ((!index.isValid()) || (index.row()-1 >= interfaces.size()))
        return QVariant();

    if ((Qt::DisplayRole != role) && (Qt::ToolTipRole != role) && (Qt::WhatsThisRole != role))
        return QVariant();
  
    int row = index.row();
    if (0 == row)
       return "Select Interface...";
    else row--;

    auto item = interfaces[row];

    auto aE = item.addressEntries();
    auto address = findValidIPv4Address(aE);
    if (address.ip().isNull())
        return item.name();
    else
        return item.name() + " : [" + address.ip().toString() + "]";
}

QVariant NetworkInterfaceList::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) 
    {
        return QString("Network Interface");
    }
    return QVariant();
}
        
void NetworkInterfaceList::selectedInterface(int index)
{
    if (0 == index) return;
    auto ae = interfaces[index-1].addressEntries();
    auto va = findValidIPv4Address(ae);
    emit scan(va.broadcast(), 5683);
}

} //end namespace
