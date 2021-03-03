#include "mainwindowui.hpp"
#include "qmainwindow.h"
#include <QToolBar>
#include <QIcon>
#include <QStatusBar>
#include <QTableWidgetItem>

namespace zero {

MainWindowUI::MainWindowUI(QMainWindow* parent)
{
    setupUi(parent);
}

void MainWindowUI::setupUi(QMainWindow* mainWindow)
{
    if (mainWindow->objectName().isEmpty())
        mainWindow->setObjectName(QString::fromUtf8("mainWindow"));

    mainWindow->resize(1024, 700);
    QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(mainWindow->sizePolicy().hasHeightForWidth());
    mainWindow->setSizePolicy(sizePolicy);
    mainWindow->setMinimumSize(QSize(1024, 700));

    //centralWidget = new QWidget(mainWindow);
   // mainWindow->setCentralWidget(this->centralWidget);

    setupToolBar(mainWindow);
    setupMainTabs(mainWindow);
//    setupDocksWidgets(mainWindow);

    mainWindow->statusBar()->showMessage(tr("Status Bar"));
}

void MainWindowUI::setupMainTabs(QMainWindow* mainWindow)
{
    mainTabs = new QTabWidget(mainWindow);
    mainWindow->setCentralWidget(mainTabs);
    
    liveView = new ZeroLiveViewTab(mainTabs);
    mgmtView = new ZeroManagementViewTab(mainTabs);
    mainTabs->addTab(liveView, tr("Live"));
    mainTabs->addTab(mgmtView, tr("Admin"));
}

void MainWindowUI::setupToolBar(QMainWindow* mainWindow)
{
    toolBar = mainWindow->addToolBar(tr("Common Functions"));
    toolBar->setObjectName(QString::fromUtf8("toolBar"));

    niSelect = new QLabel(QString::fromUtf8("Network"), toolBar);
    niSelect->setMargin(5);
    toolBar->addWidget(niSelect);

    networkInterfaceSelector = new QComboBox(toolBar);
    networkInterfaceSelector->setObjectName(QString::fromUtf8("networkInterfaceSelector"));
    networkInterfaceSelector->setStatusTip(tr("Select Network Interface"));
    toolBar->addWidget(networkInterfaceSelector);
}

#if 0
void MainWindowUI::setupDocksWidgets(QMainWindow* mainWindow)
{
    devicesDock = new QDockWidget(tr("Blixt Zeros"), mainWindow);
    devicesDock->setObjectName(QString::fromUtf8("devicesDock"));
    devicesDock->setFeatures(QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable);
    devicesDock->setAllowedAreas(Qt::TopDockWidgetArea);
    devicesDock->setMinimumSize(QSize(300, 100));


    devicesDock->setWidget(zeroTable);
    mainWindow->addDockWidget(Qt::TopDockWidgetArea, devicesDock);

/*    messagesDock = new QDockWidget(tr("Messages"), mainWindow);
    messagesDock->setObjectName(QString::fromUtf8("messagesDock"));
    messagesDock->setFeatures(QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable);
    messagesDock->setAllowedAreas(Qt::BottomDockWidgetArea);
    messagesDock->setMinimumSize(QSize(200, 150));
    
    messagesTable = new QTableWidget(messagesDock);
    messagesTable->setObjectName(QString::fromUtf8("messagesTable"));
    messagesTable->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    messagesDock->setWidget(messagesTable);
    mainWindow->addDockWidget(Qt::BottomDockWidgetArea, messagesDock);

    messagesTable->setColumnCount(3);

    QTableWidgetItem *messageTableItem = new QTableWidgetItem();
    messageTableItem->setText(tr("Timestamp"));
    messagesTable->setHorizontalHeaderItem(0, messageTableItem);
    messageTableItem = new QTableWidgetItem();
    messageTableItem->setText(tr("Type"));
    messagesTable->setHorizontalHeaderItem(1, messageTableItem);
    messageTableItem = new QTableWidgetItem();
    messageTableItem->setText(tr("Message"));
    messagesTable->setHorizontalHeaderItem(2, messageTableItem);

    messagesTable->horizontalHeader()->setStretchLastSection(true);*/
}
#endif
} // end namespace
