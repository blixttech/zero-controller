#include "zerolist.hpp"


namespace zero {

ZeroList::ZeroList(QObject *parent) : QObject(parent),
    zeros_(), zerosVec_()
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

} //end of namespace
