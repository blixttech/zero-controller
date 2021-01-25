#ifndef __MAIN_WINDOW_H__
#define __MAIN_WINDOW_H__

#include "common.hpp"
#include <QMainWindow>
#include <QVector>
#include <QHostAddress>

class Breaker;
class MainWindow : public QMainWindow {

    Q_OBJECT
public:
    MainWindow(Config *config);
    ~MainWindow();

private slots:
    void onBreakerFound(Breaker *breaker);
    void onBreakerNotFound(QHostAddress &host, int port);

private:
    void showEvent(QShowEvent* event);
    class PrivateData;
    PrivateData* pData_; 
};

#endif // __MAIN_WINDOW_H__