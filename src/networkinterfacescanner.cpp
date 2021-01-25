#include "networkinterfacescanner.hpp"
#include <QNetworkInterface>


namespace zero {

NetworkInterfaceScanner::NetworkInterfaceScanner(QObject *parent) : 
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

        interfaces.append(*i);
    }
}


int NetworkInterfaceScanner::rowCount(const QModelIndex &parent) const
{
    return interfaces.size();
}

QVariant NetworkInterfaceScanner::data(const QModelIndex &index, int role) const
{
    if ((!index.isValid()) || (index.row() >= interfaces.size()))
        return QVariant();

    if ((Qt::DisplayRole != role) && (Qt::ToolTipRole != role) && (Qt::WhatsThisRole != role))
        return QVariant();
   
    auto item = interfaces[index.row()];

    return item.name();
}

/*QVariant NetworkInterfaceScanner::headerData(int section, Qt::Orientation orientation, int role) const
{
}*/

} //end namespace
