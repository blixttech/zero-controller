#include "mainwindow.hpp"
#include "mainwindowui.hpp"
#include "breakermanager.hpp"
#include "breaker.hpp"
#include <QDebug>

class MainWindow::PrivateData : QObject
{
public:
    PrivateData(Config *config, QObject *parent = nullptr) : QObject(parent)
    {
        config = config;
        ui = new MainWindowUI();
        breakerManager = new BreakerManager(parent);
    }

    ~PrivateData()
    {
        if (ui) {
            delete ui;
            ui = nullptr;
        }

        if (breakerManager) {
            delete breakerManager;
            breakerManager = nullptr;
        }

    }

    Config *config;
    MainWindowUI *ui;
    BreakerManager *breakerManager;
};

MainWindow::MainWindow(Config *config)
{
    pData_ = new PrivateData(config, this);
    pData_->ui->setupUi(this);
    QObject::connect(pData_->breakerManager, &BreakerManager::breakerFound, 
                    this, &MainWindow::onBreakerFound);
    QObject::connect(pData_->breakerManager, &BreakerManager::breakerNotFound, 
                    this, &MainWindow::onBreakerNotFound);
}

MainWindow::~MainWindow()
{
    if (pData_ != NULL) {
        delete pData_;
        pData_ = NULL;
    }
}

void MainWindow::showEvent(QShowEvent* event)
{
    QHostAddress host;

    // Add address for searching
    host.setAddress("10.42.0.255");
    pData_->breakerManager->addToSearch(host);

    // Add address of a known breaker
    host.setAddress("10.42.0.45");
    pData_->breakerManager->addBreaker(host);

    pData_->breakerManager->searchStart();
}

void MainWindow::onBreakerFound(Breaker *breaker)
{
    qInfo() << "Breaker found. UUID:" << breaker->uuid() 
            << "IP:" << breaker->host().toString()
            << "MAC:" << breaker->hardwareAddress();
}

void MainWindow::onBreakerNotFound(QHostAddress &host, int port)
{
    qCritical() << "Breaker not found at address" << host.toString() << "and port" << port;
}