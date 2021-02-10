#include <iostream>

#include <QDateTime>

#include <QApplication>
#include <QVariant>
#include <QCommandLineParser>
#include <QLoggingCategory>
#include <QFile>
#include <QDebug>

#include "common.hpp"
#include "mainwindow.hpp"

namespace zero {

void customMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    const char* file = context.file ? context.file : "";
    const char* function = context.function ? context.function : "";
    QString dateTimeStr = QDateTime::currentDateTime().toString("yyyyMMdd-hh:mm:ss.zzz");
    switch (type) {
        case QtDebugMsg:
            fprintf(stdout, "%s|DEBUG| %s\n", dateTimeStr.toStdString().c_str(), msg.toStdString().c_str());
            break;
        case QtInfoMsg:
            fprintf(stdout, "%s|INFO | %s\n", dateTimeStr.toStdString().c_str(), msg.toStdString().c_str());
            break;
        case QtWarningMsg:
            fprintf(stderr, "%s|WARN | %s\n", dateTimeStr.toStdString().c_str(), msg.toStdString().c_str());
            break;
        case QtCriticalMsg:
            fprintf(stderr, "%s|CRTCL| %s\n", dateTimeStr.toStdString().c_str(), msg.toStdString().c_str());
            break;
        case QtFatalMsg:
            fprintf(stderr, "%s|FATAL| %s\n", dateTimeStr.toStdString().c_str(), msg.toStdString().c_str());
            break;
    }
}

void processCmdArgs(zero::Config& config)
{
    QCommandLineParser parser;

    parser.addHelpOption();

    QCommandLineOption cfgFileOpt(QStringList() << "c" << "config", 
        "Configuration file", "config-file");
    parser.addOption(cfgFileOpt);

    QCommandLineOption logLevelOpt(QStringList() << "l" << "log-level", 
        "Log level \n 1:fatal, 2:critical, 3:warning, 4:info, 5:debug", "log-level");
    parser.addOption(logLevelOpt);

    parser.parse(QCoreApplication::arguments());

    qDebug() << "Starting Application";
    if (parser.isSet(cfgFileOpt)) {
        config.insert(CONF_KEY_CFG_FILE, parser.value(cfgFileOpt));
    }

    if (parser.isSet(logLevelOpt)) {
        config.insert(CONF_KEY_LOG_LEVEL, parser.value(logLevelOpt).toInt());
    }
}

void configureLogging(const zero::Config& config)
{
    int logLevel = GET_CONF(config, LOG_LEVEL).toInt();

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
}

void applyStyles(QApplication& app)
{
    QFile uiStyleFile(":/ui-style/ui-style.qss" );
    uiStyleFile.open(QFile::ReadOnly);

    QString styleSheet = uiStyleFile.readAll();
    app.setStyleSheet(styleSheet);
}

} //end namespace


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    qInstallMessageHandler(zero::customMessageOutput);
    app.setApplicationName("Blixt Zero Controller");
    app.setWindowIcon(QIcon(":/icons/BLIXT_Logo_50x25.png"));

    zero::Config config;
    zero::processCmdArgs(config);
    zero::configureLogging(config);

    zero::MainWindow main(config);
    zero::applyStyles(app);
    main.show();

    std::cout << "Starting" << std::endl;
    return app.exec();
}
