#include "zeroliveviewmodel.hpp"

#include <qglobal.h>
#include <qpoint.h>
#include <qvariant.h>

#include <QBrush>
#include <cinttypes>

#include "zeroproxy.hpp"

namespace zero {

std::vector<QString> ZeroLiveViewModel::headers(
    {QT_TR_NOOP("UUID"), QT_TR_NOOP("Status"), QT_TR_NOOP("Last Trigger"), QT_TR_NOOP("Vᵣₘₛ(V)"),
     QT_TR_NOOP("Iᵣₘₛ(A)"), QT_TR_NOOP("Uptime"), QT_TR_NOOP("Power In (C)"),
     QT_TR_NOOP("Power Out (C)"), QT_TR_NOOP("Ambient (C)"), QT_TR_NOOP("MCU (C)")});

ZeroLiveViewModel::ZeroLiveViewModel(std::shared_ptr<ZeroList> zList, QObject* parent)
    : QAbstractTableModel(parent), zList(zList)
{
    connect(zList.get(), &ZeroList::beforeAddingZero, this, &ZeroLiveViewModel::beforeAddingZero);
    connect(zList.get(), &ZeroList::zeroAdded, this, &ZeroLiveViewModel::zeroAdded);

    connect(zList.get(), &ZeroList::zeroUpdated,
            [=](int row) { zeroUpdated(row, Qt::DisplayRole); });

    connect(zList.get(), &ZeroList::ZeroParameterChanged,
            [=](int row, ZeroProxy::Parameter parameter) {
                switch (parameter) {
                    case ZeroProxy::Parameter::TripCurve:
                        zeroUpdated(row, zero::TripCurve);
                        break;
                    case ZeroProxy::Parameter::InitialSwitchState:
                        zeroUpdated(row, zero::InitialStateConfig);
                        break;
                    case ZeroProxy::Parameter::OcpHwConfig:
                        zeroUpdated(row, zero::OvercurrentHwConfig);
                        break;
                    default:
                        qWarning() << "Unknown switch parameter " << static_cast<int>(parameter);
                        break;
                }
            });

    connect(zList.get(), &ZeroList::beforeErasingZero, this, &ZeroLiveViewModel::beforeErasingZero);
    connect(zList.get(), &ZeroList::zeroErased, this, &ZeroLiveViewModel::zeroErased);

    connect(zList.get(), &ZeroList::sendStatusMessage, this, &ZeroLiveViewModel::sendStatusMessage);
}

int ZeroLiveViewModel::rowCount(const QModelIndex& parent) const { return zList->zeros().size(); }

int ZeroLiveViewModel::columnCount(const QModelIndex& parent) const { return headers.size(); }

QVariant ZeroLiveViewModel::data(const QModelIndex& index, int role) const
{
    int row = index.row();
    int col = index.column();
    // generate a log message when this method gets called
    /*qDebug() << QString("row %1, col%2, role %3")
            .arg(row).arg(col).arg(role);*/

    switch (role) {
        case Qt::ToolTipRole:
            switch (col) {
                case 0:
                    return zList->zeros()[row]->uuid();
                case 2:
                    return zList->zeros()[row]->lastTransitionReasonStr();
            }
            break;
        case Qt::DisplayRole: {
            switch (col) {
                case 0:
                    return zList->zeros()[row]->uuid();
                case 1:
                    return zList->zeros()[row]->closed();  // ? tr("Closed") : tr("Open");
                case 2:
                    return zList->zeros()[row]->lastTransitionReasonStr();
                case 3:
                    return QString::number(zList->zeros()[row]->voltageRms() / 1000.0);
                case 4:
                    return QString::number(zList->zeros()[row]->currentRms() / 1000.0);
                case 5: {
                    uint32_t up = zList->zeros()[row]->uptime();
                    uint32_t msecs = up % 1000;
                    up = up / 1000;
                    uint32_t seconds = up % 60;
                    up = up / 60;
                    uint32_t minutes = up % 60;
                    up = up / 60;
                    uint32_t hours = up % 60;
                    return QString("%1:%2:%3.%4")
                        .arg(hours, 4, 10, QChar('0'))
                        .arg(minutes, 2, 10, QChar('0'))
                        .arg(seconds, 2, 10, QChar('0'))
                        .arg(msecs, 2, 10, QChar('0'));
                }
                case 6:
                    return QString::number(zList->zeros()[row]->powerInTemp());
                case 7:
                    return QString::number(zList->zeros()[row]->powerOutTemp());
                case 8:
                    return QString::number(zList->zeros()[row]->ambientTemp());
                case 9:
                    return QString::number(zList->zeros()[row]->mcuTemp());
                default:
                    return QVariant();
            }
        }
        /*case Qt::FontRole:
            if (row == 0 && col == 0) { //change font only for cell(0,0)
                QFont boldFont;
                boldFont.setBold(true);
                return boldFont;
            }
            break;*/
        case Qt::BackgroundRole:
            if (zList->zeros()[row]->isStale()) return QBrush(Qt::gray);

            break;
            /*        case Qt::TextAlignmentRole:
                        if (row == 1 && col == 1) //change text alignment only for cell(1,1)
                            return int(Qt::AlignRight | Qt::AlignVCenter);
                        break;*/
        case Qt::ForegroundRole:
            /*            if (1 == col)
                        {
                            if (zList->zeros()[row]->closed())
                                return QBrush(Qt::red);
                            else
                                return QBrush(Qt::green);
                        }*/
            break;
        case Qt::CheckStateRole:
            /*            if (4 == col)
                            return Qt::Checked;
                        else if (5 == col)
                            return Qt::Unchecked;*/
            break;
        case Qt::UserRole:
            return zList->zeros()[row]->isStale();
        case zero::VoltageSeries:
            return QVariant::fromValue(static_cast<void*>(zList->zeros()[row]->voltageSeries()));
        case zero::CurrentSeries:
            return QVariant::fromValue(static_cast<void*>(zList->zeros()[row]->currentSeries()));
        case zero::PowerSeries:
            return QVariant::fromValue(static_cast<void*>(zList->zeros()[row]->powerSeries()));
        case zero::FrequencySeries:
            return QVariant::fromValue(static_cast<void*>(zList->zeros()[row]->frequencySeries()));
        case zero::TripCurve:
            return QVariant::fromValue(static_cast<void*>(zList->zeros()[row]->tripCurve()));
        case zero::HasTripConfig:
            return QVariant::fromValue(zList->zeros()[row]->newProtocol());
        case zero::HasInitialStateConfig:
            return QVariant::fromValue(zList->zeros()[row]->hasInitStateConfig());
        case zero::InitialStateConfig:
            return QVariant::fromValue(static_cast<int>(zList->zeros()[row]->initStateConfig()));
        case zero::OvercurrentHwConfig: {
            QList<QVariant> ocpVal;
            auto z = zList->zeros()[row];
            ocpVal.append(QVariant::fromValue(z->ocp_hw_limit()));
            ocpVal.append(QVariant::fromValue(z->jitter_filter()));

            ocpVal.append(QVariant::fromValue(z->autoreclose_enabled()));
            ocpVal.append(QVariant::fromValue(z->recover_delay()));
            ocpVal.append(QVariant::fromValue(z->recover_attempts()));
            return QVariant(ocpVal);
        }
    }
    return QVariant();
}

bool ZeroLiveViewModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    int row = index.row();
    auto zero = zList->zeros()[row];
    switch (role) {
        case Qt::EditRole: {
            int row = index.row();
            int col = index.column();

            if (1 != col) return false;

            zero->toggle();
            emit dataChanged(index, index, {role});

            return true;
        }
        case zero::TripCurve: {
            std::vector<QPointF>* curve = static_cast<std::vector<QPointF>*>(value.value<void*>());
            zero->setTripCurve(*curve);
            return true;
        }
        case zero::InitialStateConfig: {
            auto state = static_cast<ZeroProxy::InitStateConfig>(value.toInt());
            zero->setInitStateConfig(state);
            return true;
        }
        case zero::OvercurrentHwConfig: {
            auto lQ = value.toList();
            auto ocpHWLimit = lQ.at(0).toInt();
            qDebug() << "OCPHWLimit updated to " << ocpHWLimit;  // setting autoreclosing
            auto recOn = lQ.at(2).toBool();
            auto delay = lQ.at(3).toInt();
            auto attempts = lQ.at(4).toInt();
            qDebug() << "AutoReclosing Updated to "
                        "Enabled: "
                     << recOn << " Delay: " << delay << " Attempts: " << attempts;
            zero->setOcpHwConfig(ocpHWLimit, 0, delay, attempts, recOn);
            return true;
        }
    }
    return false;
}

QVariant ZeroLiveViewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) return QVariant();

    if (orientation == Qt::Horizontal) {
        if (section > (headers.size() - 1)) return QVariant();

        return headers[section];
    }
    return QVariant();
}

void ZeroLiveViewModel::beforeAddingZero(int newRow)
{
    beginInsertRows(QModelIndex(), newRow, newRow);
}

void ZeroLiveViewModel::zeroAdded(int newRow)
{
    // datasource was externally updated
    endInsertRows();
}

void ZeroLiveViewModel::zeroUpdated(int updatedRow, int role)
{
    auto topLeft = createIndex(updatedRow, 0);
    auto bottomRight = createIndex(updatedRow, columnCount() - 1);

    emit dataChanged(topLeft, bottomRight, {role});
}

void ZeroLiveViewModel::beforeErasingZero(int removedRow)
{
    beginRemoveRows(QModelIndex(), removedRow, removedRow);
}

void ZeroLiveViewModel::zeroErased(int removedRow)
{
    // datasource was externally updated
    endRemoveRows();
}

}  // namespace zero
