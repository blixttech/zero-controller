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
    

    void insert(std::shared_ptr<ZeroProxy> zero);
    bool contains(const QString& uuid) const;
    std::shared_ptr<ZeroProxy> const get(const QString& uuid) const;
    void erase(const QString& uuid);

    const ZeroVec& zeros() const;

    void unsubscribe(); 

signals:
    void beforeAddingZero(int index);
    void zeroAdded(int index);
    
    void zeroUpdated(int index);

    void beforeErasingZero(int index);
    void zeroErased(int index);
   
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
