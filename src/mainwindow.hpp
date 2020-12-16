#ifndef __MAIN_WINDOW_H__
#define __MAIN_WINDOW_H__

#include <common.hpp>
#include <QWidget>
#include <QString>

namespace Ui
{
class mainWindow;
}

class MainWindow : public QWidget {

    Q_OBJECT
public:
    MainWindow(Config* config, QWidget *parent = nullptr);
    ~MainWindow();

private:
    class PrivateData;
    PrivateData* pData_; 
};

#endif // __MAIN_WINDOW_H__