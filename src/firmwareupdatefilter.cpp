#include "firmwareupdatefilter.hpp"

#include "zeromanagementviewmodel.hpp"

namespace zero {

FirmwareUpdateFilter::FirmwareUpdateFilter(QObject *parent) :
    QSortFilterProxyModel(parent),
    selectedZeros(),
    firmwareUpdateInProgress(false)
{
    // when the source model changes, connect to the dataChange
    // so that the filter can 
    connect(this, &QAbstractProxyModel::sourceModelChanged, 
            [&]()
            {
              connect(this->sourceModel(), &QAbstractItemModel::dataChanged,
                    [&](const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> &roles)
                    {
                        // check if display role is present
                        auto i = roles.begin();
                        for (; i != roles.end(); ++i)
                        {
                            if (*i == Qt::DisplayRole) break;
                        }

                        if (i == roles.end()) return;

                        int top_row = topLeft.row();
                        int bottom_row = bottomRight.row();

                        for (int a = top_row; a <= bottom_row; ++a)
                        {
                            if (selectedZeros.find(a) == selectedZeros.end()) continue;
                            auto idx = sourceModel()->index(a, ZeroManagementViewModel::ManagementColumns::UPDATE_STATUS);
                            
                            auto status = sourceModel()->data(idx).toString();

                            if (("100" == status) || ("Failed" == status))
                            {
                                // Update is completed
                                selectedZeros.erase(a);
                            }
                        }
                        
                        if (selectedZeros.size() == 0)
                            firmwareUpdateInProgress = false;
                    }
             ); 

            }
    );
}

void FirmwareUpdateFilter::setSelectionModel(QItemSelectionModel *selectionModel)
{
    connect(selectionModel, &QItemSelectionModel::selectionChanged,
        [&](const QItemSelection &selected, const QItemSelection &deselected)
        {
            if (firmwareUpdateInProgress) return;
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
    firmwareUpdateInProgress = true;
    QVariant ff(firmwareFile);
    updatingZeros = selectedZeros;
    for (auto i = updatingZeros.begin(); i != updatingZeros.end(); ++i)
    {        
        auto idx = sourceModel()->index(*i, ZeroManagementViewModel::ManagementColumns::INIT_UPDATE);
        sourceModel()->setData(idx, ff);
    }
}

} // end of namespace
