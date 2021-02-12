#pragma once

#include "common.hpp"
#include "mainwindowui.hpp"
#include "networkinterfacelist.hpp"
#include "zerolist.hpp"
#include "zerocoapscanner.hpp"
#include "zerotablemodel.hpp"

#include <QMainWindow>
#include <QCloseEvent> 

namespace zero {

class MainWindow : public QMainWindow {

    Q_OBJECT
public:
    MainWindow(Config& config);

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    zero::Config config;
    std::unique_ptr<MainWindowUI> ui;

    std::unique_ptr<NetworkInterfaceList> interfaceList;
    std::shared_ptr<ZeroList> zeroList;

    std::unique_ptr<ZeroCoapScanner> zeroCoapScanner;
    std::unique_ptr<ZeroTableModel> zeroTableModel;

    

    void connectSignals();
};

}
