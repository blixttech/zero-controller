#pragma once

#include <QSortFilterProxyModel> 
#include <QItemSelectionModel>

#include <set>

namespace zero {

    class FirmwareUpdateFilter : public QSortFilterProxyModel
    {
        public:
            FirmwareUpdateFilter(QObject *parent = nullptr);

            void setSelectionModel(QItemSelectionModel *selectionModel);

            // Initiates the firmware update for the selection
            void initiateFwUpdate(const QByteArray& firmwareFile);
        protected:
            virtual bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const;
            virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

        private:
            std::set<int> selectedZeros;
            std::set<int> updatingZeros;
            bool firmwareUpdateInProgress;

    };

} // end of namespace
