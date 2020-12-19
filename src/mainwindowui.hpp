#ifndef __MAIN_WINDOW_UI_H__
#define __MAIN_WINDOW_UI_H__

#include <QObject>
#include <QMainWindow>
#include <QAction>
#include <QToolBar>
#include <QDockWidget>
#include <QTreeView>
#include <QTableWidget>

class MainWindowUI: public QObject
{

public:
    MainWindowUI();
    virtual ~MainWindowUI();

    void setupUi(QMainWindow *mainWindow);

public:
    QToolBar *toolBar;
    QAction *autoDiscoveryAction;
    QWidget *centralWidget;
    QDockWidget *devicesDock;
    QDockWidget *messagesDock;
    QTreeView *devicesTreeView;
    QTableWidget* messagesTable;

private:
    void setupToolBar(QMainWindow *mainWindow);
    void setupDocksWidgets(QMainWindow *mainWindow);

};

#endif // __MAIN_WINDOW_UI_H__