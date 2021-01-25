#pragma once

#include "common.hpp"
#include "mainwindowui.hpp"
#include "networkinterfacescanner.hpp"
#include <QMainWindow>

namespace zero {

class MainWindow : public QMainWindow {

    Q_OBJECT
public:
    MainWindow(Config& config);

    virtual ~MainWindow();
private:
    zero::Config config;
    MainWindowUI* ui;

    NetworkInterfaceScanner* interfaceScanner;
};

}
