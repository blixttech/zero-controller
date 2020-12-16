#ifndef __BK710_APP_GUI_H__
#define __BK710_APP_GUI_H__

#include <mainwindow.hpp>
#include <QApplication>

class Bk710AppGui: public QApplication
{

public:
    Bk710AppGui(int& argc, char **argv);
    ~Bk710AppGui();

private:
    void processCmdArgs(int& argc, char **argv);
    void applyStyles();
    class PrivateData;
    PrivateData* pData_;
};

#endif // __BK710_APP_GUI_H__