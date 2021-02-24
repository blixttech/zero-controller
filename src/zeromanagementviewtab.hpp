#pragma once
#include <QWidget>
#include <QTableWidget>

#include "zeromanagementviewmodel.hpp"

namespace zero {

class ZeroManagementViewTab : public QWidget
{
    public:
        ZeroManagementViewTab(QWidget* parent = nullptr);

        void setModel(ZeroManagementViewModel* model); 
    private: 
        QTableView*   zeroTable;
};

} // end namespace
