#include "zerotablemodel.hpp"
#include <QBrush>

namespace zero {

std::vector<QString> ZeroTableModel::headers({
        QT_TR_NOOP("UUID"),
        QT_TR_NOOP("Status"),
        QT_TR_NOOP("Vᵣₘₛ"),
        QT_TR_NOOP("Iᵣₘₛ"),
        QT_TR_NOOP("OCP"),
        QT_TR_NOOP("OTP"),
        QT_TR_NOOP("Uptime(s)")
});

ZeroTableModel::ZeroTableModel(std::shared_ptr<ZeroList> zList, QObject *parent) :
    QAbstractTableModel(parent),
    zList(zList)
{
    connect(zList.get(), &ZeroList::beforeAddingZero,
            this, &ZeroTableModel::beforeAddingZero);
    connect(zList.get(), &ZeroList::zeroAdded,
            this, &ZeroTableModel::zeroAdded);
    connect(zList.get(), &ZeroList::zeroUpdated,
            this, &ZeroTableModel::zeroUpdated);
    connect(zList.get(), &ZeroList::beforeErasingZero,
            this, &ZeroTableModel::beforeErasingZero);
    connect(zList.get(), &ZeroList::zeroErased,
            this, &ZeroTableModel::zeroErased);
}

int ZeroTableModel::rowCount(const QModelIndex &parent) const
{
    return zList->zeros().size();
}

int ZeroTableModel::columnCount(const QModelIndex &parent) const
{
    return headers.size();
}

QVariant ZeroTableModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();
    // generate a log message when this method gets called
/*    qDebug() << QString("row %1, col%2, role %3")
            .arg(row).arg(col).arg(role);*/

    switch (role) 
    {
        case Qt::DisplayRole:
        {
            switch(col) 
            {
                case 0:
                    return zList->zeros()[row]->uuid(); 
                case 1:
                    return zList->zeros()[row]->closed() ? tr("Closed") : tr("Open"); 
                case 2:
                    return QString::number(zList->zeros()[row]->voltageRms());
                case 3:
                    return QString::number(zList->zeros()[row]->currentRms());
                case 6:
                    return QString::number(zList->zeros()[row]->uptime());
                default:
                   return QVariant(); 
            }
        }
        /*case Qt::FontRole:
            if (row == 0 && col == 0) { //change font only for cell(0,0)
                QFont boldFont;
                boldFont.setBold(true);
                return boldFont;
            }
            break;*/
        case Qt::BackgroundRole:
            if (zList->zeros()[row]->isStale())
                return QBrush(Qt::gray);
            break;
/*        case Qt::TextAlignmentRole:
            if (row == 1 && col == 1) //change text alignment only for cell(1,1)
                return int(Qt::AlignRight | Qt::AlignVCenter);
            break;*/
        case Qt::CheckStateRole:
            if (4 == col)
                return Qt::Checked;
            else if (5 == col)
                return Qt::Unchecked;
            break;
    }
    return QVariant();
}

QVariant ZeroTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        if (section > (headers.size()-1))
            return QVariant();
        
        return headers[section];    
    }
    return QVariant();
}

void ZeroTableModel::beforeAddingZero(int newRow)
{
    beginInsertRows(QModelIndex(), newRow, newRow);
}

void ZeroTableModel::zeroAdded(int newRow)
{
    // datasource was externally updated
    endInsertRows();
}

void ZeroTableModel::zeroUpdated(int updatedRow)
{
    auto topLeft = createIndex(updatedRow,0);
    auto bottomRight = createIndex(updatedRow,columnCount());

    emit dataChanged(topLeft, bottomRight, {Qt::DisplayRole}); 
}

void ZeroTableModel::beforeErasingZero(int removedRow)
{
    beginRemoveRows(QModelIndex(), removedRow, removedRow);
}

void ZeroTableModel::zeroErased(int removedRow)
{
    // datasource was externally updated
    endRemoveRows();
}

} // end of namespace
