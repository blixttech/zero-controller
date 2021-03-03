#include "zeromanagementviewtab.hpp"

#include <QHeaderView>
#include <QVBoxLayout>

namespace zero {

ZeroManagementViewTab::ZeroManagementViewTab(QWidget* parent) : QWidget(parent),
    zeroTable(new QTableView(this))
{
    zeroTable->setObjectName(QString::fromUtf8("zeroTable"));
    zeroTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    zeroTable->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    zeroTable->setEditTriggers(QTableView::AllEditTriggers);
//    zeroTable->setItemDelegateForColumn(1,new OpenCloseButtonDelegate(zeroTable));
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(zeroTable);
    setLayout(mainLayout);
}

void ZeroManagementViewTab::setModel(ZeroManagementViewModel* model)
{
    zeroTable->setModel(model);
}

} // end namespace
