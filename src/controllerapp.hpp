#ifndef __BK710_APP_GUI_H__
#define __BK710_APP_GUI_H__

#include "mainwindow.hpp"
#include <QApplication>

class ControllerApp: public QApplication
{

public:
    ControllerApp(int& argc, char **argv);
    ~ControllerApp();

private:
    void processCmdArgs(int& argc, char **argv);
    void applyStyles();
    class PrivateData;
    PrivateData* pData_;
};

#endif // __BK710_APP_GUI_H__