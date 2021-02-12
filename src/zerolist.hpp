#pragma once

#include <QObject>
#include <QHostAddress>
#include <map>

#include "zeroproxy.hpp"


namespace zero {

typedef std::map<QString, uint> ZeroMap; 
typedef std::vector<std::shared_ptr<ZeroProxy>> ZeroVec; 

class ZeroList : public QObject
{
    Q_OBJECT
public:
    explicit ZeroList(QObject *parent=nullptr);
    

    bool containsZero(const QString& uuid) const;

    void addZeroProxy(std::shared_ptr<ZeroProxy> zero);

    const ZeroVec& zeros() const;

    void unsubscribe(); 

signals:
    void newZeroAdded(int index);
    void zeroUpdated(int index);
    void allUnsubscribed();


private:
    Q_DISABLE_COPY(ZeroList);

    ZeroMap zeros_; 
    ZeroVec zerosVec_;
    uint unsubscribeCounter_;

    void notifyOfZeroUpdate(const QString& uuid);
    void notifyOfZeroUnsubscribed();
};

} // end namespace