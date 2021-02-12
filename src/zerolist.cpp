#include "zerolist.hpp"


namespace zero {

ZeroList::ZeroList(QObject *parent) : QObject(parent),
    zeros_(), zerosVec_(), unsubscribeCounter_(0)
{
}

bool ZeroList::contains(const QString& uuid) const
{
    return (zeros_.count(uuid) != 0);
}

std::shared_ptr<ZeroProxy> const  ZeroList::get(const QString& uuid) const
{
    if (!contains(uuid)) return std::shared_ptr<ZeroProxy>();

    return zerosVec_[zeros_.at(uuid)];
}

void ZeroList::erase(const QString& uuid)
{
    if (!contains(uuid)) return;

    auto idx = zeros_[uuid];

    emit beforeErasingZero(idx);
    zerosVec_.erase(zerosVec_.begin() + idx);
    zeros_.erase(uuid);
    emit zeroErased(idx);
}

void ZeroList::insert(std::shared_ptr<ZeroProxy> zero)
{
    if (zeros_.count(zero->uuid()) != 0)
        return;

    emit beforeAddingZero(zerosVec_.size());

    zerosVec_.push_back(zero);

    uint32_t zeroIndex = zerosVec_.size()-1;
    zeros_.insert({zero->uuid(), zeroIndex});

    emit zeroAdded(zeroIndex);

    connect(zero.get(), &ZeroProxy::statusUpdated,
            [=]() 
            {
                notifyOfZeroUpdate(zero->uuid());
            }
    );
    
    connect(zero.get(), &ZeroProxy::stale,
            [=]() 
            {
                notifyOfZeroUpdate(zero->uuid());
            }
    );

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
