#include "mainwindowui.hpp"
#include "qmainwindow.h"
#include <QToolBar>
#include <QIcon>
#include <QStatusBar>
#include <QTableWidgetItem>
#include <QHeaderView>


MainWindowUI::MainWindowUI()
{

}

MainWindowUI::~MainWindowUI()
{

}

void MainWindowUI::setupUi(QMainWindow *mainWindow)
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

    this->centralWidget = new QWidget(mainWindow);
    mainWindow->setCentralWidget(this->centralWidget);

    setupToolBar(mainWindow);
    setupDocksWidgets(mainWindow);

    mainWindow->statusBar()->showMessage(tr("Status Bar"));
}

void MainWindowUI::setupToolBar(QMainWindow *mainWindow)
{
    this->toolBar = mainWindow->addToolBar(tr("Common Functions"));
    this->toolBar->setObjectName(QString::fromUtf8("toolBar"));

    this->autoDiscoveryAction = new QAction(QIcon(":/icons/discovery.png"), tr("&Auto Discover Devices"), mainWindow);
    this->autoDiscoveryAction->setObjectName(QString::fromUtf8("autoDiscoveryAction"));
    this->autoDiscoveryAction->setStatusTip(tr("Auto Discover Devices"));
    this->autoDiscoveryAction->setCheckable(true);
    this->autoDiscoveryAction->setChecked(true);
    toolBar->addAction(this->autoDiscoveryAction);
}

void MainWindowUI::setupDocksWidgets(QMainWindow *mainWindow)
{
    this->devicesDock = new QDockWidget(tr("Devices"), mainWindow);
    this->devicesDock->setObjectName(QString::fromUtf8("devicesDock"));
    this->devicesDock->setFeatures(QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable);
    this->devicesDock->setAllowedAreas(Qt::LeftDockWidgetArea);
    this->devicesDock->setMinimumSize(QSize(200, 300));
    this->devicesTreeView = new QTreeView(this->devicesDock);
    this->devicesTreeView->setObjectName(QString::fromUtf8("devicesTreeView"));
    this->devicesTreeView->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    this->devicesDock->setWidget(this->devicesTreeView);
    mainWindow->addDockWidget(Qt::LeftDockWidgetArea, this->devicesDock);

    this->messagesDock = new QDockWidget(tr("Messages"), mainWindow);
    this->messagesDock->setObjectName(QString::fromUtf8("messagesDock"));
    this->messagesDock->setFeatures(QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable);
    this->messagesDock->setAllowedAreas(Qt::BottomDockWidgetArea);
    this->messagesDock->setMinimumSize(QSize(200, 150));
    this->messagesTable = new QTableWidget(this->messagesDock);
    this->messagesTable->setObjectName(QString::fromUtf8("messagesTable"));
    this->messagesTable->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    this->messagesDock->setWidget(this->messagesTable);
    mainWindow->addDockWidget(Qt::BottomDockWidgetArea, this->messagesDock);

    this->messagesTable->setColumnCount(3);

    QTableWidgetItem *messageTableItem = new QTableWidgetItem();
    messageTableItem->setText(tr("Timestamp"));
    this->messagesTable->setHorizontalHeaderItem(0, messageTableItem);
    messageTableItem = new QTableWidgetItem();
    messageTableItem->setText(tr("Type"));
    this->messagesTable->setHorizontalHeaderItem(1, messageTableItem);
    messageTableItem = new QTableWidgetItem();
    messageTableItem->setText(tr("Message"));
    this->messagesTable->setHorizontalHeaderItem(2, messageTableItem);

    this->messagesTable->horizontalHeader()->setStretchLastSection(true);

}