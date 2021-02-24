#include "zeroliveviewtab.hpp"
#include "openclosebuttondelegate.hpp"

#include <QHeaderView>
#include <QVBoxLayout>

namespace zero {

ZeroLiveViewTab::ZeroLiveViewTab(QWidget* parent) : QWidget(parent),
    zeroTable(new QTableView(this))
{
    zeroTable->setObjectName(QString::fromUtf8("zeroTable"));
    zeroTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    zeroTable->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    zeroTable->setEditTriggers(QTableView::AllEditTriggers);
    zeroTable->setItemDelegateForColumn(1,new OpenCloseButtonDelegate(zeroTable));
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(zeroTable);
    setLayout(mainLayout);
}

void ZeroLiveViewTab::setModel(ZeroLiveViewModel* model)
{
    zeroTable->setModel(model);

    connect(model, &ZeroLiveViewModel::rowsInserted, 
            [=](const QModelIndex &parent, int first, int last)
            {
                qDebug() << "In here";
                for ( int i = first; i <= last; ++i )
                {
                    qDebug() << "In here loop";
                    zeroTable->openPersistentEditor( model->index(i,1) );
                }
                qDebug() << "In here out";
            });
}

} // end namespace
