#pragma once

#include <QObject>
#include <QLabel>
#include <QMainWindow>
#include <QToolBar>
//#include <QDockWidget>
#include <QTabWidget> 

#include <QComboBox>

#include "zeroliveviewtab.hpp"
#include "zeromanagementviewtab.hpp"

namespace zero {

class MainWindowUI: public QObject
{

public:
    MainWindowUI(QMainWindow* parent);

    QToolBar *toolBar;
    QLabel *niSelect;
    QComboBox *networkInterfaceSelector;

    //QWidget *centralWidget;
    //QDockWidget *devicesDock;
    /*QDockWidget *messagesDock;

    QTableWidget* messagesTable;*/

    QTabWidget* mainTabs;
    ZeroLiveViewTab* liveView;
    ZeroManagementViewTab* mgmtView;
    
private:    
    void setupUi(QMainWindow* mainWindow);
    void setupToolBar(QMainWindow* mainWindow);
//    void setupDocksWidgets(QMainWindow* mainWindow);
    void setupMainTabs(QMainWindow* mainWindow);
};

} // end namespace zero
