#include "mainwindow.hpp"
#include "mainwindowui.hpp"

#include <QMessageBox>
#include <QApplication>
#include <QAbstractButton>
#include <QTimer>

namespace zero {

MainWindow::MainWindow(Config& config): QMainWindow(nullptr),
    config(config), 
    ui(std::make_unique<MainWindowUI>(this)),
    interfaceList(std::make_unique<NetworkInterfaceList>()),
    zeroList(std::make_shared<ZeroList>()),
    zeroCoapScanner(std::make_unique<ZeroCoapScanner>()),
    zeroLiveViewModel(std::make_unique<ZeroLiveViewModel>(zeroList)),
    zeroManagementViewModel(std::make_unique<ZeroManagementViewModel>(zeroList))
{
    ui->networkInterfaceSelector->setModel(interfaceList.get());
    ui->liveView->setModel(zeroLiveViewModel.get());
    ui->mgmtView->setModel(zeroManagementViewModel.get());
    connectSignals();

     
    int search_int = GET_CONF(config, SEARCH_INT).toInt();
    qInfo() << "Setting search interval to " << search_int << "sec";
    zeroCoapScanner->startScanning(search_int * 1000);
}

void MainWindow::connectSignals()
{
    connect(ui->networkInterfaceSelector, QOverload<int>::of(&QComboBox::activated),
            interfaceList.get(), &NetworkInterfaceList::selectedInterface);
    connect(interfaceList.get(), &NetworkInterfaceList::scan,
            zeroCoapScanner.get(), &ZeroCoapScanner::addScanTarget);
    
    int update_int = GET_CONF(config, UPDATE_INT).toInt();
    qInfo() << "Using update interval of " << update_int << "msec";

    connect(zeroCoapScanner.get(), &ZeroCoapScanner::newZeroDetected,    
        [=]( const QString& uuid, const QUrl& url, 
             const QString& hwversion, const QString& macaddress, bool new_protocol)
        {
            // if the zero is already registered, we are done
            if (zeroList->contains(uuid))
            {
                auto zpr = zeroList->get(uuid);

                zpr->updateUrl(url);
            }
            else 
            {
                // create a new proxy
                auto zProxy = std::make_shared<ZeroProxy>(url, uuid, hwversion, macaddress, update_int, new_protocol);
                zeroList->insert(zProxy);
            }
        }
    );


}

void MainWindow::closeEvent(QCloseEvent* event)
{
    event->ignore();

    QMessageBox box;
    box.setText("Are you sure you want to quit?");
    box.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
    box.setDefaultButton(QMessageBox::No);
    int ret = box.exec();

    switch (ret) {
      case QMessageBox::No:
        return;
        break;
      case QMessageBox::Yes:
      default:
        zeroList->clear();
        break;
    }
    qInfo() << "Shutting down";

    qApp->quit();
}


} // end namespace



