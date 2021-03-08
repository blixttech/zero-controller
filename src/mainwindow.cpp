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

    zeroCoapScanner->startScanning(10000);
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
            if (zeroList->contains(uuid))
            {
                auto zpr = zeroList->get(uuid);

                zpr->updateUrl(url);
            }
            else 
            {
                // create a new proxy
                auto zProxy = std::make_shared<ZeroProxy>(url, uuid, hwversion, macaddress);
                zeroList->insert(zProxy);
            }
        }
    );


}

void MainWindow::closeEvent(QCloseEvent* event)
{
    event->ignore();

    QMessageBox* box = new QMessageBox(QMessageBox::Question, 
                                       qApp->applicationName(), 
                                       "Are you sure you want to quit?",
                                       QMessageBox::Yes | QMessageBox::No, 
                                       this);

    QObject::connect(box->button(QMessageBox::Yes), &QAbstractButton::clicked, zeroList.get(), &ZeroList::clear);
    QObject::connect(box->button(QMessageBox::No), &QAbstractButton::clicked, box, &QObject::deleteLater);

    QObject::connect(zeroList.get(), &ZeroList::listClear, qApp, &QApplication::quit);

    box->show();
}


} // end namespace



