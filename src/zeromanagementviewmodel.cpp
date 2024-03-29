#include "zeromanagementviewmodel.hpp"
#include <QBrush>

namespace zero {

std::vector<QString> ZeroManagementViewModel::headers({
        QT_TR_NOOP("UUID"),
        QT_TR_NOOP("IP"),
        QT_TR_NOOP("Firmware Slot 0"),
        QT_TR_NOOP("Firmware Slot 1"),
        QT_TR_NOOP("Update Status"),
        QT_TR_NOOP("Initiate Update") // this column should never be displayed, to be used for starting update
});

ZeroManagementViewModel::ZeroManagementViewModel(std::shared_ptr<ZeroList> zList, QObject *parent) :
    QAbstractTableModel(parent),
    zList(zList)
{
    connect(zList.get(), &ZeroList::beforeAddingZero,
            this, &ZeroManagementViewModel::beforeAddingZero);
    connect(zList.get(), &ZeroList::zeroAdded,
            this, &ZeroManagementViewModel::zeroAdded);
    connect(zList.get(), &ZeroList::zeroUpdated,
            this, &ZeroManagementViewModel::zeroUpdated);
    connect(zList.get(), &ZeroList::beforeErasingZero,
            this, &ZeroManagementViewModel::beforeErasingZero);
    connect(zList.get(), &ZeroList::zeroErased,
            this, &ZeroManagementViewModel::zeroErased);
}

int ZeroManagementViewModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    
    return zList->zeros().size();
}

int ZeroManagementViewModel::columnCount(const QModelIndex &parent) const
{
    return headers.size();
}

QVariant ZeroManagementViewModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();
    // generate a log message when this method gets called
    // qDebug() << QString("row %1, col%2, role %3").arg(row).arg(col).arg(role);

    switch (role) 
    {
        case Qt::DisplayRole:
        {
            switch(col) 
            {
                case Z_UUID:
                    return zList->zeros()[row]->uuid(); 
                case Z_IP:
                    return zList->zeros()[row]->host();// ? tr("Closed") : tr("Open");
                case FW_SLOT1:
                    return zList->zeros()[row]->fwSlotInfo(0);
                case FW_SLOT2:
                    return zList->zeros()[row]->fwSlotInfo(1);
                case UPDATE_STATUS:
                {
                    auto z = zList->zeros()[row];
                    if (z->isFirmwareUpdateOngoing()) return z->firmwareUpdateProgress();
                    if (!z->didFirmwareUpdateSucceed().has_value()) return "--";
                    else if (z->didFirmwareUpdateSucceed().value()) return "Completed";
                    return "Failed";
                }
                case INIT_UPDATE:
                    return QVariant();
                default:
                   return QVariant(); 
            }
        }
#if 0
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
        case Qt::ForegroundRole:
            if (1 == col) 
            {
                if (zList->zeros()[row]->closed())
                    return QBrush(Qt::red);
                else
                    return QBrush(Qt::green);
            }
            break;
        case Qt::CheckStateRole:
            if (4 == col)
                return Qt::Checked;
            else if (5 == col)
                return Qt::Unchecked;
            break;
#endif            
    }
    return QVariant();
}

bool ZeroManagementViewModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (Qt::EditRole != role) return false;

    int row = index.row();
    int col = index.column();

    if (INIT_UPDATE != col) return false;

    zList->zeros()[row]->sendFirmwareUpdate(value.toByteArray());

    return true;
}

#if 0
Qt::ItemFlags ZeroManagementViewModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    int col = index.column();
    int row = index.row();

    if (zList->zeros()[row]->isLive() && (1 == col))
        return Qt::ItemIsEditable | QAbstractItemModel::flags(index);

    if (1 == col)
        qDebug() << "Stale";
    return QAbstractItemModel::flags(index);

}
#endif

QVariant ZeroManagementViewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) 
    {
        if (section > (headers.size()-1))
            return QVariant();
        
        return headers[section];    
    }
    if (orientation == Qt::Vertical) 
    {
        return QString::number(section+1) ;
    }
    return QVariant();
}

void ZeroManagementViewModel::beforeAddingZero(int newRow)
{
    beginInsertRows(QModelIndex(), newRow, newRow);
}

void ZeroManagementViewModel::zeroAdded(int newRow)
{
    // datasource was externally updated
    endInsertRows();
}

void ZeroManagementViewModel::zeroUpdated(int updatedRow)
{
    auto topLeft = createIndex(updatedRow,0);
    auto bottomRight = createIndex(updatedRow,columnCount()-1);

    emit dataChanged(topLeft, bottomRight, {Qt::DisplayRole}); 
}

void ZeroManagementViewModel::beforeErasingZero(int removedRow)
{
    beginRemoveRows(QModelIndex(), removedRow, removedRow);
}

void ZeroManagementViewModel::zeroErased(int removedRow)
{
    // datasource was externally updated
    endRemoveRows();
}

} // end of namespace
