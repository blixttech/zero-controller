#ifndef __MAIN_WINDOW_H__
#define __MAIN_WINDOW_H__

#include "common.hpp"
#include <QMainWindow>

class MainWindow : public QMainWindow {

    Q_OBJECT
public:
    MainWindow(Config *config);
    ~MainWindow();

private:
    class PrivateData;
    PrivateData* pData_; 
};

#endif // __MAIN_WINDOW_H__