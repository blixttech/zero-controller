#include <mainwindow.hpp>
#include <ui_mainwindow.h>

class MainWindow::PrivateData : QObject
{
public:
    PrivateData(Config* config, QObject* parent = nullptr) : QObject(parent)
    {
        this->config = config;
        ui = new Ui::mainWindow();
    }

    ~PrivateData()
    {
        if (ui != NULL) {
            delete ui;
            ui = NULL;
        }
    }

    Ui::mainWindow* ui;
    Config* config;
};

MainWindow::MainWindow(Config* config, QWidget *parent) : QWidget(parent)
{
    pData_ = new PrivateData(config, this);
    pData_->ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    if (pData_ != NULL) {
        delete pData_;
        pData_ = NULL;
    }
}