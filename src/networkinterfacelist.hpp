#pragma once

#include <QAbstractListModel>
#include <QNetworkInterface>

namespace zero {

class NetworkInterfaceList : public QAbstractListModel
{
    Q_OBJECT
    public:
        explicit NetworkInterfaceList(QObject *parent = nullptr);

        int rowCount(const QModelIndex &parent = QModelIndex()) const;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    public slots:
        void selectedInterface(int index);

    signals:
        void scan(QHostAddress broadcastAddr, int port);
    

    private:
        QNetworkAddressEntry findValidIPv4Address( QList<QNetworkAddressEntry> & addresses) const;
         QList<QNetworkInterface> interfaces;
};

} //end namespace
