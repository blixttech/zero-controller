#include "mainwindow.hpp"
#include "mainwindowui.hpp"

namespace zero {

MainWindow::MainWindow(Config& config): QMainWindow(nullptr),
    config(config), 
    ui(new MainWindowUI(this)),
    interfaceScanner(new NetworkInterfaceScanner(this))
{
    ui->networkInterfaceSelector->setModel(interfaceScanner);
}

MainWindow::~MainWindow()
{
    delete ui;
}

} // end namespace



