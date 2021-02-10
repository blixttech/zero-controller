#include "mainwindow.hpp"
#include "mainwindowui.hpp"

namespace zero {

MainWindow::MainWindow(Config& config): QMainWindow(nullptr),
    config(config), 
    ui(std::make_unique<MainWindowUI>(this)),
    interfaceList(std::make_unique<NetworkInterfaceList>()),
    zeroList(std::make_shared<ZeroList>()),
    zeroCoapScanner(std::make_unique<ZeroCoapScanner>()),
    zeroTableModel(std::make_unique<ZeroTableModel>(zeroList))
{
    ui->networkInterfaceSelector->setModel(interfaceList.get());
    ui->zeroTable->setModel(zeroTableModel.get());
    connectSignals();

    zeroCoapScanner->startScanning();


}

void MainWindow::connectSignals()
{
    connect(ui->networkInterfaceSelector, QOverload<int>::of(&QComboBox::activated),
            interfaceList.get(), &NetworkInterfaceList::selectedInterface);
    connect(interfaceList.get(), &NetworkInterfaceList::scan,
            zeroCoapScanner.get(), &ZeroCoapScanner::addScanTarget);
    
    connect(zeroCoapScanner.get(), &ZeroCoapScanner::newZeroDetected,    
        [=]( const QString& uuid, const QUrl& url, 
             const QString& hwversion, const QString& macaddress)
        {
            // if the zero is already registered, we are done
            if (zeroList->containsZero(uuid)) return;

            // create a new proxy
            auto zProxy = std::make_unique<ZeroProxy>(url, uuid, hwversion, macaddress);
            zeroList->addZeroProxy(std::move(zProxy));

        }
    );


}

} // end namespace



