#include "zerotablemodel.hpp"

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
    connect(zList.get(), &ZeroList::newZeroAdded,
            this, &ZeroTableModel::newZeroAdded);
    connect(zList.get(), &ZeroList::zeroUpdated,
            this, &ZeroTableModel::zeroUpdated);
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
            break;
        case Qt::BackgroundRole:
            if (row == 1 && col == 2)  //change background only for cell(1,2)
                return QBrush(Qt::red);
            break;
        case Qt::TextAlignmentRole:
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

void ZeroTableModel::newZeroAdded(int newRow)
{
    beginInsertRows(QModelIndex(), newRow, newRow);
    // datasource was externally updated
    endInsertRows();
}

void ZeroTableModel::zeroUpdated(int updatedRow)
{
    auto topLeft = createIndex(updatedRow,0);
    auto bottomRight = createIndex(updatedRow,columnCount());

    emit dataChanged(topLeft, bottomRight, {Qt::DisplayRole}); 
}

} // end of namespace
