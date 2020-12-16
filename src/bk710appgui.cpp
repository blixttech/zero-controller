#include <bk710appgui.hpp>
#include <common.hpp>
#include <QString>
#include <QVariant>
#include <QCommandLineParser>
#include <QLoggingCategory>
#include <QFile>
#include <QDebug>

class Bk710AppGui::PrivateData : QObject
{
public:
    PrivateData(QObject *parent = nullptr) : QObject(parent)
    {
        mainWindow = NULL;
        config = new Config;
    }

    void init()
    {
        mainWindow = new MainWindow(config);      
    }

    ~PrivateData()
    {
        if (mainWindow != NULL) {
            delete mainWindow;
            mainWindow = NULL;
        }

        if (config != NULL) {
            delete config;
            config = NULL;
        }
    }

    MainWindow* mainWindow;
    Config* config;

};

Bk710AppGui::Bk710AppGui(int& argc, char **argv) : QApplication(argc, argv)
{
    setApplicationName("BK710 Tool");
    // Must create private data before processing command line arguments.
    pData_ = new PrivateData(this);
    processCmdArgs(argc, argv);

    int logLevel = GET_CONF(pData_->config, LOG_LEVEL).toInt();
    QLoggingCategory* defaultLog = QLoggingCategory::defaultCategory();
    if (logLevel >= 1) {
        defaultLog->setEnabled(QtFatalMsg, true);
    } else {
        defaultLog->setEnabled(QtFatalMsg, false);
    }
    if (logLevel >= 2) {
        defaultLog->setEnabled(QtCriticalMsg, true);
    } else {
        defaultLog->setEnabled(QtCriticalMsg, false);
    }
    if (logLevel >= 3) {
        defaultLog->setEnabled(QtWarningMsg, true);
    } else {
        defaultLog->setEnabled(QtWarningMsg, false);
    }
    if (logLevel >= 4) {
        defaultLog->setEnabled(QtInfoMsg, true);
    } else {
        defaultLog->setEnabled(QtInfoMsg, false);
    }
    if (logLevel >= 5) {
        defaultLog->setEnabled(QtDebugMsg, true);
    } else {
        defaultLog->setEnabled(QtDebugMsg, false);
    }

    // Must initialize after creating private data.
    pData_->init();
    applyStyles();
    pData_->mainWindow->show();
}

Bk710AppGui::~Bk710AppGui()
{
    if (pData_ != NULL) {
        delete pData_;
        pData_ = NULL;
    }
}

void Bk710AppGui::processCmdArgs(int& argc, char **argv)
{
    QCommandLineParser parser;

    parser.addHelpOption();

    QCommandLineOption cfgFileOpt(QStringList() << "c" << "config", 
        "Configuration file", "config-file");
    parser.addOption(cfgFileOpt);

    QCommandLineOption logLevelOpt(QStringList() << "l" << "log-level", 
        "Log level \n 1:fatal, 2:critical, 3:warning, 4:info, 5:debug", "log-level");
    parser.addOption(logLevelOpt);

    QCommandLineOption plotBufSizeOpt(QStringList() << "B" << "plot-buf-size", 
        "Buffer size for the plot", "plot-buf-size");
    parser.addOption(plotBufSizeOpt);

    QCommandLineOption addDummyPltDataOpt(QStringList() << "P" << "add-dummy-plot-data", 
        "Add dummy plot data");
    parser.addOption(addDummyPltDataOpt);

    QCommandLineOption addDummyTempDataOpt(QStringList() << "T" << "add-dummy-temp-data", 
        "Add dummy temperature data");
    parser.addOption(addDummyTempDataOpt);

    parser.process(*this);

    if (parser.isSet(cfgFileOpt)) {
        pData_->config->insert(CONF_KEY_CFG_FILE, parser.value(cfgFileOpt));
    }

    if (parser.isSet(logLevelOpt)) {
        pData_->config->insert(CONF_KEY_LOG_LEVEL, parser.value(logLevelOpt).toInt());
    }
}

void Bk710AppGui::applyStyles()
{
    QFile uiStyleFile(":/ui-style/ui-style.qss" );
    uiStyleFile.open(QFile::ReadOnly);

    QString styleSheet = uiStyleFile.readAll();
    setStyleSheet(styleSheet);
}