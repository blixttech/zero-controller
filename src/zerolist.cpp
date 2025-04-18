#include "zerolist.hpp"

#include <qglobal.h>
#include <qobject.h>

#include "zeroproxy.hpp"

namespace zero {

ZeroList::ZeroList(QObject* parent) : QObject(parent), zeros_(), zerosVec_() {}

bool ZeroList::contains(const QString& uuid) const { return (zeros_.count(uuid) != 0); }

std::shared_ptr<ZeroProxy> const ZeroList::get(const QString& uuid) const
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
    if (zeros_.count(zero->uuid()) != 0) return;

    emit beforeAddingZero(zerosVec_.size());

    zerosVec_.push_back(zero);

    uint32_t zeroIndex = zerosVec_.size() - 1;
    zeros_.insert({zero->uuid(), zeroIndex});

    emit zeroAdded(zeroIndex);

    connect(zero.get(), &ZeroProxy::StatusUpdated, [=]() { notifyOfZeroUpdate(zero->uuid()); });

    connect(zero.get(), &ZeroProxy::ParameterChanged,
            [=](const QString& uuid, ZeroProxy::Parameter parameter) {
                if (zeros_.count(uuid) == 0) {
                    qWarning() << "Received unknown uuid " << uuid << " in ParameterChange signal";
                    return;
                }

                auto index = zeros_.at(uuid);
                qDebug() << "UUID " << uuid << " at row " << index << " with paramater "
                         << static_cast<int>(parameter);
                emit ZeroParameterChanged(index, parameter);
            });

    connect(zero.get(), &ZeroProxy::Stop, [=]() {
        qDebug() << "Erasing stopped " << zero->uuid();
        erase(zero->uuid());
    });

    connect(zero.get(), &ZeroProxy::SendStatusMessage, this, &ZeroList::sendStatusMessage);
}

const ZeroVec& ZeroList::zeros() const { return zerosVec_; }

void ZeroList::notifyOfZeroUpdate(const QString& uuid)
{
    if (zeros_.count(uuid) == 0) return;

    auto index = zeros_.at(uuid);
    emit zeroUpdated(index);
}

void ZeroList::clear()
{
    qDebug() << "List size: " << zerosVec_.size();

    foreach (auto zero, zerosVec_) {
        zero->stop();
    }
}

}  // namespace zero
