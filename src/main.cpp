#include <bk710appgui.hpp>
#include <QDateTime>

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

int main(int argc, char *argv[])
{
    qInstallMessageHandler(customMessageOutput);
    Bk710AppGui app(argc, argv);

    return app.exec();
}