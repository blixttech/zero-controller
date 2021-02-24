#pragma once
#include <QWidget>
#include <QTableWidget>

#include "zeroliveviewmodel.hpp"

namespace zero {

class ZeroLiveViewTab : public QWidget
{
    public:
        ZeroLiveViewTab(QWidget* parent = nullptr);

        void setModel(ZeroLiveViewModel* model); 
    private: 
        QTableView*   zeroTable;
};

} // end namespace
