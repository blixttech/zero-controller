#ifndef __BK710_APP_GUI_H__
#define __BK710_APP_GUI_H__

#include <mainwindow.hpp>
#include <QApplication>

class ControllerAppGui: public QApplication
{

public:
    ControllerAppGui(int& argc, char **argv);
    ~ControllerAppGui();

private:
    void processCmdArgs(int& argc, char **argv);
    void applyStyles();
    class PrivateData;
    PrivateData* pData_;
};

#endif // __BK710_APP_GUI_H__