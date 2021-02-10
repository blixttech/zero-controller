#pragma once

#include <QAbstractTableModel>
#include "zerolist.hpp"

namespace zero {

    class ZeroTableModel : public QAbstractTableModel
    {
        Q_OBJECT
        public:
            ZeroTableModel(std::shared_ptr<ZeroList> zList, QObject *parent = nullptr);

            int rowCount(const QModelIndex &parent = QModelIndex()) const override;
            int columnCount(const QModelIndex &parent = QModelIndex()) const override;
            QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
            QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
       
        private slots:
            void newZeroAdded(int newRow); 
            void zeroUpdated(int updatedRow); 

        private:
            std::shared_ptr<ZeroList> zList;
            static std::vector<QString> headers;
    };

} // end of namespace
