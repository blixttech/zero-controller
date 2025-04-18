#include "zeroliveviewtab.hpp"

#include <qglobal.h>
#include <qnamespace.h>
#include <qvariant.h>

#include <QAbstractButton>
#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QBoxLayout>
#include <QColor>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QLabel>
#include <QPoint>
#include <QPushButton>
#include <QTextStream>
#include <QTime>
#include <QVBoxLayout>
#include <QWidget>
#include <QwtAxis>
#include <QwtPlotCurve>
#include <QwtScaleDraw>
#include <QwtScaleWidget>
#include <QwtText>

#include "openclosebuttondelegate.hpp"
#include "timescaledraw.hpp"
#include "zerogeneralconfwidget.hpp"
#include "zeroliveviewmodel.hpp"
#include "zerotripconfwidget.hpp"

namespace zero {

void configureTimeAxis(QwtPlot* plot)
{
    // plot->setAxisTitle( QwtAxis::XBottom, " System Uptime [h:m:s]" );
    plot->setAxisScaleDraw(QwtAxis::XBottom, new TimeScaleDraw(QTime(0, 0)));
    //    plot->setAxisScale( QwtAxis::XBottom, 0, 60 );
    plot->setAxisLabelRotation(QwtAxis::XBottom, -50.0);
    plot->setAxisLabelAlignment(QwtAxis::XBottom, Qt::AlignLeft | Qt::AlignBottom);

    /*
       In situations, when there is a label at the most right position of the
       scale, additional space is needed to display the overlapping part
       of the label would be taken by reducing the width of scale and canvas.
       To avoid this "jumping canvas" effect, we add a permanent margin.
       We don't need to do the same for the left border, because there
       is enough space for the overlapping label below the left scale.
     */

    QwtScaleWidget* scaleWidget = plot->axisWidget(QwtAxis::XBottom);
    const int fmh = QFontMetrics(scaleWidget->font()).height();
    scaleWidget->setMinBorderDist(0, fmh / 2);
}

ZeroLiveViewTab::ZeroLiveViewTab(QWidget* parent)
    : QWidget(parent),
      zeroTable(new QTableView(this)),
      zeroDetails(new QTabWidget(this)),
      zeroPlots(new QWidget(zeroDetails)),
      selectedRowIdx(-1),
      zeroTrip(new ZeroTripConfWidget),
      zeroGConf(new ZeroGeneralConfWidget)
{
    zeroTable->setObjectName(QString::fromUtf8("zeroTable"));
    zeroTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    zeroTable->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    // zeroTable->setEditTriggers(QTableView::AllEditTriggers);
    //    zeroTable->setEditTriggers(QTableView::DoubleClicked);
    zeroTable->setItemDelegateForColumn(1, new OpenCloseButtonDelegate(zeroTable));
    zeroTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    zeroTable->setMinimumSize(100, 200);
    zeroTable->verticalHeader()->setEnabled(true);
    zeroTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    zeroTable->setSelectionMode(QAbstractItemView::SingleSelection);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(zeroTable, 50.0);

    QwtText vo("Voltage");
    voltagePlot = new QwtPlot(vo, zeroPlots);
    voltagePlot->setAxisTitle(QwtAxis::YLeft, "Voltage [V]");
    vCurve = nullptr;
    configureTimeAxis(voltagePlot);

    QwtText cu("Current");
    currentPlot = new QwtPlot(cu, zeroPlots);
    currentPlot->setAxisTitle(QwtAxis::YLeft, "Current [A]");
    cCurve = nullptr;
    configureTimeAxis(currentPlot);

    QwtText po("Power");
    powerPlot = new QwtPlot(po, zeroPlots);
    powerPlot->setAxisTitle(QwtAxis::YLeft, "Power [W]");
    pCurve = nullptr;
    configureTimeAxis(powerPlot);

    QwtText freq("Frequency");
    frequencyPlot = new QwtPlot(freq, zeroPlots);
    frequencyPlot->setAxisTitle(QwtAxis::YLeft, "Frequency [Hz]");
    fCurve = nullptr;
    configureTimeAxis(frequencyPlot);

    auto overview = new QGridLayout(zeroPlots);
    overview->addWidget(voltagePlot, 0, 0);
    overview->addWidget(currentPlot, 0, 1);
    overview->addWidget(powerPlot, 1, 0);
    overview->addWidget(frequencyPlot, 1, 1);

    zeroPlots->setLayout(overview);
    zeroDetails->addTab(zeroPlots, tr("Overview"));

    zeroTrip->setEnabled(false);
    connect(zeroTrip, &ZeroTripConfWidget::sendStatusMessage, this,
            &ZeroLiveViewTab::sendStatusMessage);

    connect(
        zeroTrip, &ZeroTripConfWidget::applyNewTripCurve, [=](std::vector<QPointF> newTripCurve) {
            auto tZeroIdx = zeroTable->model()->index(selectedRowIdx, 0);
            zeroTable->model()->setData(
                tZeroIdx, QVariant::fromValue(static_cast<void*>(&newTripCurve)), zero::TripCurve);
        });

    connect(zeroGConf, &ZeroGeneralConfWidget::ApplyValuesToZero, [=]() {
        auto tZeroIdx = zeroTable->model()->index(selectedRowIdx, 0);

        if (tZeroIdx.model()->data(tZeroIdx, zero::HasInitialStateConfig).toBool()) {
            QVariant iniSwi;

            zeroTable->model()->setData(tZeroIdx, QVariant::fromValue(zeroGConf->initState()),
                                        zero::InitialStateConfig);
        }

        QList<QVariant> ocpVal;
        ocpVal.append(QVariant::fromValue(zeroGConf->getOCPHWLimit()));
        ocpVal.append(QVariant::fromValue(0));

        ocpVal.append(QVariant::fromValue(zeroGConf->isAutoReclosingEnabled()));
        ocpVal.append(QVariant::fromValue(zeroGConf->getReclosingAttemptsDelay()));
        ocpVal.append(QVariant::fromValue(zeroGConf->getReclosingAttempts()));

        QVariant ql(ocpVal);
        zeroTable->model()->setData(tZeroIdx, ql, zero::OvercurrentHwConfig);
    });

    connect(zeroGConf, &ZeroGeneralConfWidget::RequestingReload, [=]() {
        auto tZeroIdx = zeroTable->model()->index(selectedRowIdx, 0);
        loadGConfFromModel(tZeroIdx);
    });

    zeroDetails->addTab(zeroTrip, tr("Trip Settings"));

    connect(zeroGConf, &ZeroGeneralConfWidget::SendStatusMessage, this,
            &ZeroLiveViewTab::sendStatusMessage);

    zeroDetails->addTab(zeroGConf, tr("General Settings"));
    zeroGConf->setEnabled(false);

    mainLayout->addWidget(zeroDetails, 50.0);

    setLayout(mainLayout);
}

ZeroLiveViewTab::~ZeroLiveViewTab()
{
    if (vCurve != nullptr) {
        vCurve->detach();
        vCurve = nullptr;
    }

    if (cCurve != nullptr) {
        cCurve->detach();
        cCurve = nullptr;
    }

    if (pCurve != nullptr) {
        pCurve->detach();
        pCurve = nullptr;
    }

    if (fCurve != nullptr) {
        fCurve->detach();
        fCurve = nullptr;
    }
}

void zero::ZeroLiveViewTab::loadGConfFromModel(const QModelIndex& idx)
{
    zeroGConf->setEnabled(true);
    bool has_init_state = idx.model()->data(idx, zero::HasInitialStateConfig).toBool();

    if (has_init_state) {
        int state = idx.model()->data(idx, zero::InitialStateConfig).toInt();

        // automatically enables init State
        zeroGConf->setInitState(state);
    } else
        zeroGConf->disableInitState();

    // setting autoreclosing and ocpHW limit
    auto lQ = idx.model()->data(idx, zero::OvercurrentHwConfig).toList();
    auto ocpHWLimit = lQ.at(0).toInt();
    qDebug() << "OCPHWLimit setting to " << ocpHWLimit;
    zeroGConf->setOCPHWLimit(ocpHWLimit);
    // skipping index 1 as jitter not in use
    auto recOn = lQ.at(2).toBool();
    auto delay = lQ.at(3).toInt();
    auto attempts = lQ.at(4).toInt();
    qDebug() << "AutoReclosing Settings Enabled: " << recOn << " Delay: " << delay
             << " Attempts: " << attempts;
    zeroGConf->enableAutoReclosing(recOn);
    zeroGConf->setReclosingAttemptsDelay(delay);
    zeroGConf->setReclosingAttempts(attempts);
}

void ZeroLiveViewTab::setModel(ZeroLiveViewModel* model)
{
    zeroTable->setModel(model);

    connect(model, &ZeroLiveViewModel::sendStatusMessage, this,
            &ZeroLiveViewTab::sendStatusMessage);

    connect(model, &ZeroLiveViewModel::rowsInserted,
            [=](const QModelIndex& parent, int first, int last) {
                for (int i = first; i <= last; ++i) {
                    zeroTable->openPersistentEditor(model->index(i, 1));
                }
            });

    connect(zeroTable->selectionModel(), &QItemSelectionModel::selectionChanged,
            [=](const QItemSelection& selected, const QItemSelection& deselected) {
                // unselecting a row or selecting new
                selectedRowIdx = -1;
                if (vCurve) vCurve->detach();
                vCurve = nullptr;

                if (cCurve) cCurve->detach();
                cCurve = nullptr;

                if (pCurve) pCurve->detach();
                pCurve = nullptr;

                if (fCurve) fCurve->detach();
                fCurve = nullptr;

                zeroTrip->clear();
                zeroTrip->setEnabled(false);

                zeroGConf->clear();
                zeroGConf->setEnabled(false);

                if (selected.size() == 1) {
                    QModelIndex idx = selected.indexes().first();
                    selectedRowIdx = idx.row();

                    vCurve = static_cast<QwtPlotCurve*>(
                        idx.model()->data(idx, zero::VoltageSeries).value<void*>());
                    vCurve->attach(voltagePlot);
                    vCurve->show();

                    cCurve = static_cast<QwtPlotCurve*>(
                        idx.model()->data(idx, zero::CurrentSeries).value<void*>());
                    cCurve->attach(currentPlot);
                    cCurve->show();

                    pCurve = static_cast<QwtPlotCurve*>(
                        idx.model()->data(idx, zero::PowerSeries).value<void*>());
                    pCurve->attach(powerPlot);
                    pCurve->show();

                    fCurve = static_cast<QwtPlotCurve*>(
                        idx.model()->data(idx, zero::FrequencySeries).value<void*>());
                    fCurve->attach(frequencyPlot);
                    fCurve->show();

                    std::vector<QPointF> tVec(*(static_cast<std::vector<QPointF>*>(
                        idx.model()->data(idx, zero::TripCurve).value<void*>())));

                    zeroTrip->setCurve(tVec);

                    bool has_trip = idx.model()->data(idx, zero::HasTripConfig).toBool();

                    zeroTrip->setEnabled(has_trip);

                    loadGConfFromModel(idx);
                }
                replot();
            });

    // updating the realtime plots
    connect(
        model, &QAbstractItemModel::dataChanged,
        [=](const QModelIndex& topLeft, const QModelIndex& bottomRight,
            const QList<int>& roles = QList<int>()) {
            // check the selected row is part of the update
            if (!(selectedRowIdx >= topLeft.row() && selectedRowIdx <= bottomRight.row())) return;

            bool display_update = false;
            bool trip_update = false;
            for (auto i = roles.constBegin(); i != roles.constEnd(); ++i) {
                switch (*i) {
                    case Qt::DisplayRole:
                        replot();
                        break;
                    case zero::TripCurve: {
                        std::vector<QPointF> tVec(*(static_cast<std::vector<QPointF>*>(
                            topLeft.model()->data(topLeft, zero::TripCurve).value<void*>())));
                        zeroTrip->setCurve(tVec);
                        break;
                    }
                    case zero::InitialStateConfig: {
                        bool has_init_state =
                            topLeft.model()->data(topLeft, zero::HasInitialStateConfig).toBool();

                        if (has_init_state) {
                            int state =
                                topLeft.model()->data(topLeft, zero::InitialStateConfig).toInt();

                            // automatically enables init State
                            zeroGConf->setInitState(state);
                        }
                        break;
                    }
                    case zero::OvercurrentHwConfig: {
                        loadGConfFromModel(topLeft);
                        break;
                    }
                }
            }
        });
}

void ZeroLiveViewTab::replot()
{
    voltagePlot->replot();
    currentPlot->replot();
    powerPlot->replot();
    frequencyPlot->replot();
}

}  // namespace zero
