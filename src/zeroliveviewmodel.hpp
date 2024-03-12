#pragma once

#include <QAbstractTableModel>
#include "zerolist.hpp"

namespace zero {
    enum ItemDataRole
    {
        VoltageSeries = 0x0101,
        CurrentSeries = 0x0102,
        PowerSeries   = 0x0103,
        FrequencySeries = 0x0104,
        TripCurve = 0x0105,
        TripConfig = 0x0106
    };


    class ZeroLiveViewModel : public QAbstractTableModel
    {
        Q_OBJECT
        public:
            ZeroLiveViewModel(std::shared_ptr<ZeroList> zList, QObject *parent = nullptr);

            int rowCount(const QModelIndex &parent = QModelIndex()) const override;
            int columnCount(const QModelIndex &parent = QModelIndex()) const override;
            QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
            QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

            bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
       
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
