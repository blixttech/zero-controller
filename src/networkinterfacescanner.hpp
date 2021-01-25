#pragma once

#include <QAbstractListModel>
#include <QNetworkInterface>

namespace zero {

class NetworkInterfaceScanner : public QAbstractListModel
{
    Q_OBJECT
    public:
        explicit NetworkInterfaceScanner(QObject *parent = nullptr);

        int rowCount(const QModelIndex &parent = QModelIndex()) const;// overwrite;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const; // overwrite;
//        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const overwrite;

    private:
         QList<QNetworkInterface> interfaces;
};

} //end namespace
