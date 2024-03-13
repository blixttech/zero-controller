#include "zerolist.hpp"


namespace zero {

ZeroList::ZeroList(QObject *parent) : QObject(parent),
    zeros_(), zerosVec_()
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

    connect(zero.get(), &ZeroProxy::configUpdated,
            [=]() 
            {
                notifyOfZeroConfigUpdate(zero->uuid());
            }
    );
        
    connect(zero.get(), &ZeroProxy::stopped,
            [=]() 
            {
                qDebug() << "Erasing stopped " << zero->uuid();
                erase(zero->uuid());
            }
    );
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
    
void ZeroList::notifyOfZeroConfigUpdate(const QString& uuid)
{
    if (zeros_.count(uuid) == 0) return;

    auto index = zeros_.at(uuid);
    emit zeroConfigUpdated(index);
}

void ZeroList::clear()
{
    qDebug() << "List size: " << zerosVec_.size();

    foreach (auto zero, zerosVec_) 
    {
        zero->stop();
    }
}

} //end of namespace
