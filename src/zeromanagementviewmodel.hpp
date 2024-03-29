#pragma once

#include <QAbstractTableModel>
#include "zerolist.hpp"

namespace zero {

    class ZeroManagementViewModel : public QAbstractTableModel
    {
        Q_OBJECT
        public:
            ZeroManagementViewModel(std::shared_ptr<ZeroList> zList, QObject *parent = nullptr);

            int rowCount(const QModelIndex &parent = QModelIndex()) const override;
            int columnCount(const QModelIndex &parent = QModelIndex()) const override;
            QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
            QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

 //           Qt::ItemFlags flags(const QModelIndex &index) const override;
            bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

            enum ManagementColumns
            {
                Z_UUID = 0,
                Z_IP = 1,
                FW_SLOT1 = 2,
                FW_SLOT2 = 3,
                UPDATE_STATUS = 4,
                INIT_UPDATE = 5
            };
       
        private slots:
            void beforeAddingZero(int newRow); 
            void zeroAdded(int newRow); 

            void zeroUpdated(int updatedRow); 

            void beforeErasingZero(int removedRow); 
            void zeroErased(int removedRow); 

        private:
            std::shared_ptr<ZeroList> zList;
            static std::vector<QString> headers;
    };

} // end of namespace
