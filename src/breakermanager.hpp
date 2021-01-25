#ifndef __BREAKER_MANAGER_H__
#define __BREAKER_MANAGER_H__

#include <QObject>
#include <QHostAddress>
#include <QList>
#include <QCoapResource>
#include <QtCoap>

class Breaker;
class QCoapReply;
class BreakerManager : public QObject
{
    Q_OBJECT
public:
    explicit BreakerManager(QObject *parent=nullptr);
    virtual ~BreakerManager();
    void searchStart(uint32_t interval=1000);
    void searchStop();
    void addToSearch(QHostAddress &host, int port=5683);
    void addBreaker(QHostAddress &host, int port=5683);
    void removeBreaker(Breaker *breaker);
    QList<Breaker*> breakers() const;

signals:
    void breakerFound(Breaker *breaker);
    void breakerNotFound(QHostAddress &host, int port);

private slots:
    void onVersionReply(QCoapReply *reply);
    void onVersionError(QCoapReply *reply, QtCoap::Error error);
    void onCoapDiscovered(const QVector<QCoapResource> &resources, 
                            const QHostAddress &host, int port);

private:
    Q_DISABLE_COPY(BreakerManager);
    class PrivateData;
    PrivateData* pData_;
};

#endif // __BREAKER_MANAGER_H__