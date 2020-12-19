#include <mainwindow.hpp>
#include <mainwindowui.hpp>

class MainWindow::PrivateData : QObject
{
public:
    PrivateData(Config *config, QObject *parent = nullptr) : QObject(parent)
    {
        this->config = config;
        this->ui = new MainWindowUI();
    }

    ~PrivateData()
    {

    }

    Config *config;
    MainWindowUI *ui;
};

MainWindow::MainWindow(Config *config)
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