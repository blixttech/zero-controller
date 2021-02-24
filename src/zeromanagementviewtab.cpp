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

/*    connect(model, &ZeroManagementViewModel::rowsInserted, 
            [=](const QModelIndex &parent, int first, int last)
            {
                for ( int i = first; i <= last; ++i )
                {
                    zeroTable->openPersistentEditor( model->index(i,1) );
                }
            });*/
}

} // end namespace
