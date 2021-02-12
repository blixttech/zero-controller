#include "zerolist.hpp"


namespace zero {

ZeroList::ZeroList(QObject *parent) : QObject(parent),
    zeros_(), zerosVec_(), unsubscribeCounter_(0)
{
}

bool ZeroList::containsZero(const QString& uuid) const
{
    return (zeros_.count(uuid) != 0);
}

void ZeroList::addZeroProxy(std::shared_ptr<ZeroProxy> zero)
{
    if (zeros_.count(zero->uuid()) != 0)
        return;

    zerosVec_.push_back(zero);

    uint32_t zeroIndex = zerosVec_.size()-1;
    zeros_.insert({zero->uuid(), zeroIndex});

    emit newZeroAdded(zeroIndex);

    connect(zero.get(), &ZeroProxy::statusUpdated,
            this, &ZeroList::notifyOfZeroUpdate);
    
    connect(zero.get(), &ZeroProxy::unsubscribed,
            this, &ZeroList::notifyOfZeroUnsubscribed);
}

const ZeroVec& ZeroList::zeros() const
{
    return zerosVec_;
}

void ZeroList::notifyOfZeroUpdate(const QString& uuid)
{
    if (zeros_.count(uuid) == 0) return;

    auto index = zeros_.at(uuid);
    emit zeroUpdated(index);
}

void ZeroList::unsubscribe()
{
    unsubscribeCounter_ = zerosVec_.size();
    if (0 == unsubscribeCounter_)
        emit allUnsubscribed();

    foreach (auto zero, zerosVec_) 
    {
        zero->unsubscribe();
    }
}

void ZeroList::notifyOfZeroUnsubscribed()
{
    if (unsubscribeCounter_ > 1)
    {
        unsubscribeCounter_--;
    }
    else
    {
        unsubscribeCounter_ = 0;
        emit allUnsubscribed();
    }
}

} //end of namespace
