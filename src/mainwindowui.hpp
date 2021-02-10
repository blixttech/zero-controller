#pragma once

#include <QObject>
#include <QLabel>
#include <QMainWindow>
#include <QAction>
#include <QToolBar>
#include <QDockWidget>
#include <QTreeView>
#include <QTableWidget>

#include <QComboBox>
namespace zero {

class MainWindowUI: public QObject
{

public:
    MainWindowUI(QMainWindow* parent);

    QToolBar *toolBar;
    QAction *autoDiscoveryAction;
    QLabel *niSelect;
    QComboBox *networkInterfaceSelector;

    QWidget *centralWidget;
    QDockWidget *devicesDock;
    QDockWidget *messagesDock;

    //QTreeView *devicesTreeView;
    QTableWidget* messagesTable;
    QTableView*   zeroTable;
    
private:    
    void setupUi(QMainWindow* mainWindow);
    void setupToolBar(QMainWindow* mainWindow);
    void setupDocksWidgets(QMainWindow* mainWindow);
};

} // end namespace zero
