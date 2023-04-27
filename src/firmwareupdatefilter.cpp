#include "firmwareupdatefilter.hpp"

#include "zeromanagementviewmodel.hpp"

namespace zero {

FirmwareUpdateFilter::FirmwareUpdateFilter(QObject *parent) :
    QSortFilterProxyModel(parent),
    selectedZeros()
{
    
}

void FirmwareUpdateFilter::setSelectionModel(QItemSelectionModel *selectionModel)
{
    connect(selectionModel, &QItemSelectionModel::selectionChanged,
        [&](const QItemSelection &selected, const QItemSelection &deselected)
        {
            selectedZeros.clear();

            auto selection = selected.indexes();
            for (int i = 0; i < selection.size(); ++i)
            {
                if (0 == selection[i].column())
                {
                    selectedZeros.insert(selection[i].row());
                }
            }        
        }
    );
}

bool FirmwareUpdateFilter::filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const
{
    // include UUID
    if (ZeroManagementViewModel::ManagementColumns::Z_UUID == source_column) return true;
    // include IP
    if (ZeroManagementViewModel::ManagementColumns::Z_IP == source_column) return true; 
    // include update status
    if (ZeroManagementViewModel::ManagementColumns::UPDATE_STATUS == source_column) return true;
    
    return false;
}

bool FirmwareUpdateFilter::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    return (selectedZeros.find(source_row) != selectedZeros.end());
}

void FirmwareUpdateFilter::initiateFwUpdate(const QByteArray& firmwareFile)
{
    QVariant ff(firmwareFile);
    updatingZeros = selectedZeros;
    for (auto i = updatingZeros.begin(); i != updatingZeros.end(); ++i)
    {        
        auto idx = sourceModel()->index(*i, ZeroManagementViewModel::ManagementColumns::INIT_UPDATE);
        sourceModel()->setData(idx, ff);
    }
}

} // end of namespace
