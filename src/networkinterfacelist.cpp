#include "networkinterfacelist.hpp"
#include <QNetworkInterface>


namespace zero {

NetworkInterfaceList::NetworkInterfaceList(QObject *parent) : 
    QAbstractListModel(parent),
    interfaces()

{
    auto interfaceList = QNetworkInterface::allInterfaces();

    //add an empty one as the first one
    interfaces.append(QNetworkInterface()); 

    for (auto i = interfaceList.begin(); i != interfaceList.end(); ++i)
    {
        if (!((QNetworkInterface::Ethernet == i->type()) 
                || (QNetworkInterface::Virtual == i->type())))
            continue;

        int mask = QNetworkInterface::IsUp | QNetworkInterface::CanBroadcast;
        if (i->flags() & mask == 0) continue;

        if (i->addressEntries().size() == 0) continue; 

        interfaces.append(*i);
    }
}

int NetworkInterfaceList::rowCount(const QModelIndex &parent) const
{
    return interfaces.size();
}

QVariant NetworkInterfaceList::data(const QModelIndex &index, int role) const
{
    if ((!index.isValid()) || (index.row() >= interfaces.size()))
        return QVariant();

    if ((Qt::DisplayRole != role) && (Qt::ToolTipRole != role) && (Qt::WhatsThisRole != role))
        return QVariant();
   
    auto item = interfaces[index.row()];

    auto addresses = item.addressEntries();
    if (0 == addresses.size())
        return item.name();
    else
        return item.name() + " : [" + item.addressEntries()[0].ip().toString() + "]";
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
    emit scan(interfaces[index].addressEntries()[0].broadcast(), 5683);
}

} //end namespace
