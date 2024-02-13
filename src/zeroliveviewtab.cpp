#include "zeroliveviewtab.hpp"
#include "openclosebuttondelegate.hpp"
#include "zerodatastream.hpp"
#include "zeroliveviewmodel.hpp"

#include <QHeaderView>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QTime>
#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QItemSelectionModel>
#include <QWidget>
#include <QDateTimeEdit>
#include <QSignalBlocker>
#include <QTextStream>
#include <QAbstractButton>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QPoint>
#include <QPushButton>
#include <QTableWidget>


#include <qabstractbutton.h>
#include <qheaderview.h>
#include <qnamespace.h>

#include <limits>

#include <QwtPlotCurve>
#include <QwtText>
#include <QwtScaleWidget>
#include <QwtScaleDraw>
#include <QwtAxis>


namespace zero {

class TimeScaleDraw : public QwtScaleDraw
{
  public:
    TimeScaleDraw( const QTime& base, const QString& format = "hh:mm:ss")
        : baseTime( base )
    {
    }
    virtual QwtText label( double v ) const QWT_OVERRIDE
    {
        QTime upTime = baseTime.addMSecs( static_cast< int >( v ) );
        return upTime.toString();
    }
  private:
    QTime baseTime;
};

void configureTimeAxis(QwtPlot* plot)
{     
    // plot->setAxisTitle( QwtAxis::XBottom, " System Uptime [h:m:s]" );
    plot->setAxisScaleDraw( QwtAxis::XBottom, new TimeScaleDraw( QTime(0, 0)));
//    plot->setAxisScale( QwtAxis::XBottom, 0, 60 );
    plot->setAxisLabelRotation( QwtAxis::XBottom, -50.0 );
    plot->setAxisLabelAlignment( QwtAxis::XBottom, Qt::AlignLeft | Qt::AlignBottom );

    /*
       In situations, when there is a label at the most right position of the
       scale, additional space is needed to display the overlapping part
       of the label would be taken by reducing the width of scale and canvas.
       To avoid this "jumping canvas" effect, we add a permanent margin.
       We don't need to do the same for the left border, because there
       is enough space for the overlapping label below the left scale.
     */

    QwtScaleWidget* scaleWidget = plot->axisWidget( QwtAxis::XBottom );
    const int fmh = QFontMetrics( scaleWidget->font() ).height();
    scaleWidget->setMinBorderDist( 0, fmh / 2 );
}

ZeroLiveViewTab::ZeroLiveViewTab(QWidget* parent) : QWidget(parent),
    zeroTable(new QTableView(this)),
    zeroDetails(new QTabWidget(this)),
    zeroPlots(new QWidget(zeroDetails)),
    selectedRowIdx(-1),
    zeroTrip(new QWidget(zeroDetails)),
    tripPlot(nullptr),
    tCurve(),
    tCurvePoints(new ZeroDataStream()),
    tCurvePointsOriginal(),
    zeroTripTable(nullptr),
    addTripPointB(nullptr),
    delTripPointB(nullptr),
    applyTripCurveB(nullptr),
    resetTripCurveB(nullptr)
{
    zeroTripTable = new QTableWidget(0,2,zeroTrip);
    addTripPointB = new QPushButton("Add", zeroTrip);
    delTripPointB = new QPushButton("Remove", zeroTrip);
    applyTripCurveB = new QPushButton("Apply", zeroTrip);
    resetTripCurveB = new QPushButton("Reset", zeroTrip);
        
    zeroTable->setObjectName(QString::fromUtf8("zeroTable"));
    zeroTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    zeroTable->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    //zeroTable->setEditTriggers(QTableView::AllEditTriggers);
//    zeroTable->setEditTriggers(QTableView::DoubleClicked);
    zeroTable->setItemDelegateForColumn(1,new OpenCloseButtonDelegate(zeroTable));
    zeroTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    zeroTable->setMinimumSize(100,200);
    zeroTable->verticalHeader()->setEnabled(true);
    zeroTable->setSelectionBehavior( QAbstractItemView::SelectRows);
    zeroTable->setSelectionMode(QAbstractItemView::SingleSelection);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(zeroTable, 50.0);

    QwtText vo("Voltage");
    voltagePlot = new QwtPlot(vo, zeroPlots);
    voltagePlot->setAxisTitle( QwtAxis::YLeft, "Voltage [V]" );
    vCurve = nullptr;
    configureTimeAxis(voltagePlot);

    QwtText cu("Current");
    currentPlot = new QwtPlot(cu, zeroPlots);
    currentPlot->setAxisTitle( QwtAxis::YLeft, "Current [A]" );
    cCurve = nullptr;
    configureTimeAxis(currentPlot);

    QwtText po("Power");
    powerPlot = new QwtPlot(po, zeroPlots);
    powerPlot->setAxisTitle( QwtAxis::YLeft, "Power [W]" );
    pCurve = nullptr;
    configureTimeAxis(powerPlot);

    QwtText freq("Frequency");
    frequencyPlot = new QwtPlot(freq, zeroPlots);
    frequencyPlot->setAxisTitle( QwtAxis::YLeft, "Frequency [Hz]" );
    fCurve = nullptr;
    configureTimeAxis(frequencyPlot);


    auto overview = new QGridLayout(zeroPlots);
    overview->addWidget(voltagePlot, 0, 0);
    overview->addWidget(currentPlot, 0, 1);
    overview->addWidget(powerPlot, 1, 0);
    overview->addWidget(frequencyPlot, 1, 1);

    zeroPlots->setLayout(overview);
    zeroDetails->addTab(zeroPlots, tr("Overview"));


    // Setting up Trip Panel
    QwtText trip("Trip Curve");
    tripPlot = new QwtPlot(trip, zeroTrip);
    tCurve.setStyle(QwtPlotCurve::CurveStyle::Steps);
    tCurve.setData(tCurvePoints);
    tripPlot->setAxisScaleDraw( QwtAxis::YLeft, new TimeScaleDraw( QTime(0, 0),"hh:mm:ss.zzz"));
//    plot->setAxisScale( QwtAxis::XBottom, 0, 60 );
//    plot->setAxisLabelRotation( QwtAxis::XBottom, -50.0 );
//    plot->setAxisLabelAlignment( QwtAxis::XBottom, Qt::AlignLeft | Qt::AlignBottom );
    
    zeroTripTable->setHorizontalHeaderLabels(QStringList() << "Current" << "Time");
   // zeroTripTable->setDragDropMode(QAbstractItemView::InternalMove);
    zeroTripTable->setSelectionMode(QAbstractItemView::SingleSelection);
    zeroTripTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    zeroTripTable->verticalHeader()->setSectionsMovable(true);

    addTripPointB->setEnabled(false);
    delTripPointB->setEnabled(false);
    applyTripCurveB->setEnabled(false);
    resetTripCurveB->setEnabled(false);
        
    auto tLayout = new QGridLayout(zeroTrip);
    tLayout->addWidget(tripPlot, 0, 0, 4, 1);
    tLayout->addWidget(zeroTripTable, 0, 1, 4, 1);
    tLayout->addWidget(addTripPointB, 0, 2);
    tLayout->addWidget(delTripPointB, 1, 2);
    tLayout->addWidget(applyTripCurveB, 2, 2);
    tLayout->addWidget(resetTripCurveB, 3, 2);

    zeroTrip->setLayout(tLayout);
    zeroDetails->addTab(zeroTrip, tr("Trip Settings"));
    
    mainLayout->addWidget(zeroDetails, 50.0);
    
    setLayout(mainLayout);


    connect(addTripPointB, &QAbstractButton::pressed, 
            [=]()
            {
                addTripPoint();

                applyTripCurveB->setEnabled(isTripCurveValid());
                updateTripCurvePlot();
                resetTripCurveB->setEnabled(true);
            }
        );
        
    connect(delTripPointB, &QAbstractButton::pressed, 
            [=]()
            {
                auto sI = zeroTripTable->selectionModel()->selectedRows();
                for (int i = 0; i < sI.size(); ++i)
                {
                    zeroTripTable->removeRow(sI[i].row());
                }                
                applyTripCurveB->setEnabled(isTripCurveValid());
                updateTripCurvePlot();
                resetTripCurveB->setEnabled(true);
            }
        );

    connect(resetTripCurveB, &QAbstractButton::pressed,
            [=]()
            {
                tCurve.detach();
                tCurvePoints->clear();

                zeroTripTable->clearContents();
                zeroTripTable->setRowCount(0);
                
                for (int i = 0; i < tCurvePointsOriginal.size(); ++i)
                {
                    auto p = tCurvePointsOriginal.sample(i);
                    addTripPoint(p.rx(), p.ry());
                                           
                    tCurvePoints->append(p);

                }
                tCurve.attach(tripPlot);
                tCurve.show();
                
                applyTripCurveB->setEnabled(isTripCurveValid());
                updateTripCurvePlot();
                resetTripCurveB->setEnabled(false);
            }
        );

    connect(zeroTripTable, &QTableWidget::cellChanged, 
            [=](int row, int col)
            {
                applyTripCurveB->setEnabled(isTripCurveValid());
                updateTripCurvePlot();
                resetTripCurveB->setEnabled(true);
            }
        );

    connect(zeroTripTable->verticalHeader(), &QHeaderView::sectionMoved,
            [=](int logIdx, int oldIdx, int newIdx)
            {
                applyTripCurveB->setEnabled(isTripCurveValid());
                updateTripCurvePlot();

                QStringList rowLabels;
                for (int i = 0; i < zeroTripTable->rowCount(); ++i)
                {
                    
                    rowLabels << QString::number(zeroTripTable->visualRow(i)+1);
                }

                zeroTripTable->setVerticalHeaderLabels(rowLabels);
                resetTripCurveB->setEnabled(true);

            }
        );
}

ZeroLiveViewTab::~ZeroLiveViewTab()
{
    if (vCurve != nullptr)
    {
        vCurve->detach();
        vCurve = nullptr;
    }

    if (cCurve != nullptr)
    {
        cCurve->detach();
        cCurve = nullptr;
    }

    if (pCurve != nullptr)
    {
        pCurve->detach();
        pCurve = nullptr;
    }

    if (fCurve != nullptr)
    {
        fCurve->detach();
        fCurve = nullptr;
    }
}


void ZeroLiveViewTab::addTripPoint(double currentInA, int timeInMs)
{
    // atomic adding new row        
    const QSignalBlocker blocker(zeroTripTable);
    uint row = zeroTripTable->rowCount();
    zeroTripTable->setRowCount(row+1);
    QTableWidgetItem* current = new QTableWidgetItem();
    current->setData(Qt::DisplayRole, currentInA);

    // QTableWidgetItem* time = new QTableWidgetItem();
    // time->setData(Qt::DisplayRole, QTime::fromMSecsSinceStartOfDay(tVec[i].ry()));

    QTimeEdit* tEdit = new QTimeEdit( QTime::fromMSecsSinceStartOfDay(timeInMs) );
    tEdit->setDisplayFormat("hh:mm:ss.zzz");

    zeroTripTable->setItem(row, 0, current);
    zeroTripTable->setCellWidget(row, 1, tEdit);
}

bool ZeroLiveViewTab::isTripCurveValid()
{
    uint rows = zeroTripTable->rowCount();
    if (rows <= 1) return true;


    double prevCur = -1;
    int prevMs = std::numeric_limits<int>::max();
    

    for (uint i = 0; i < rows; ++i)
    {
        QTextStream status;
        QString msg;
        status.setString(&msg);
        // get the real row
        int logRow = zeroTripTable->verticalHeader()->logicalIndex(i);
        double current = zeroTripTable->item(logRow, 0)->data(Qt::DisplayRole).toDouble();
        int timeMs = static_cast<QTimeEdit*>(zeroTripTable->cellWidget(logRow, 1))->time().msecsSinceStartOfDay();

        if (prevCur > current)
        {
            status << "Current is descending but should be ascending in row " << i;
            qWarning() << msg;
            emit sendStatusMessage(msg);
            return false;        
        }
        prevCur = current;

        if (prevMs < timeMs)
        {
            status << "Time is ascending but should be descending in row " << i;
            qWarning() << msg;
            emit sendStatusMessage(msg);
            return false;        
        }
        prevMs = timeMs;
    }
    return true;
}

void ZeroLiveViewTab::updateTripCurvePlot()
{            
    uint rows = zeroTripTable->rowCount();

    tCurvePoints->clear();

    for (uint i = 0; i < rows; ++i)
    {
        // get the real row
        int logRow = zeroTripTable->verticalHeader()->logicalIndex(i);
        double current = zeroTripTable->item(logRow, 0)->data(Qt::DisplayRole).toDouble();
        int timeMs = static_cast<QTimeEdit*>(zeroTripTable->cellWidget(logRow, 1))->time().msecsSinceStartOfDay();
        tCurvePoints->append(QPointF(current, timeMs));
    }
    tripPlot->replot();
}

void ZeroLiveViewTab::setModel(ZeroLiveViewModel* model)
{
    zeroTable->setModel(model);

    connect(model, &ZeroLiveViewModel::rowsInserted, 
            [=](const QModelIndex &parent, int first, int last)
            {
                for ( int i = first; i <= last; ++i )
                {
                    zeroTable->openPersistentEditor( model->index(i,1) );
                }
            });

    connect(zeroTable->selectionModel(), &QItemSelectionModel::selectionChanged, 
            [=](const QItemSelection &selected, const QItemSelection &deselected)
            {
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

                tCurve.detach();
                tCurvePoints->clear();
                tCurvePointsOriginal.clear();

                zeroTripTable->clearContents();
                zeroTripTable->setRowCount(0);
                
                addTripPointB->setEnabled(false);
                delTripPointB->setEnabled(false);
                applyTripCurveB->setEnabled(false);
                resetTripCurveB->setEnabled(false);
            
                if (selected.size() == 1)
                {
                    QModelIndex idx = selected.indexes().first();
                    selectedRowIdx = idx.row();

                    vCurve = static_cast<QwtPlotCurve*>(idx.model()->data(idx, zero::VoltageSeries).value<void*>());
                    vCurve->attach(voltagePlot);
                    vCurve->show();

                    cCurve = static_cast<QwtPlotCurve*>(idx.model()->data(idx, zero::CurrentSeries).value<void*>());
                    cCurve->attach(currentPlot);
                    cCurve->show();
                    
                    pCurve = static_cast<QwtPlotCurve*>(idx.model()->data(idx, zero::PowerSeries).value<void*>());
                    pCurve->attach(powerPlot);
                    pCurve->show();
                    
                    fCurve = static_cast<QwtPlotCurve*>(idx.model()->data(idx, zero::FrequencySeries).value<void*>());
                    fCurve->attach(frequencyPlot);
                    fCurve->show();
                    
                    std::vector<QPointF> tVec(*(static_cast<std::vector<QPointF>*>(idx.model()->data(idx, zero::TripCurve).value<void*>())));

                    for (int i = 0; i < tVec.size(); ++i)
                    {
                        double cVal = tVec[i].rx() / 1000.0;
                        addTripPoint(cVal, tVec[i].ry());
                                               
                        tCurvePoints->append(QPointF(cVal, tVec[i].ry()));
                        tCurvePointsOriginal.append(QPointF(cVal, tVec[i].ry()));

                    }
                    tCurve.attach(tripPlot);
                    tCurve.show();

                    
                    addTripPointB->setEnabled(true);
                    delTripPointB->setEnabled(true);
                    applyTripCurveB->setEnabled(false);
                    resetTripCurveB->setEnabled(false);
                }
                replot();
                    
            });

    // updating the realtime plots
    connect(model, &QAbstractItemModel::dataChanged,
            [=](const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> &roles = QList<int>())
            {
                bool display_update = false;
                for (auto i = roles.constBegin(); i != roles.constEnd(); ++i)
                {
                    if (*i == Qt::DisplayRole)
                    {
                        display_update = true;
                        break;
                    }
                }
                
                if (!display_update) return;

                // check the selected row is part of the update
                if (!(selectedRowIdx >= topLeft.row() &&
                    selectedRowIdx <= bottomRight.row())) return;

                replot();
                
            }
        );
}

void ZeroLiveViewTab::replot()
{
    voltagePlot->replot();
    currentPlot->replot();
    powerPlot->replot();
    frequencyPlot->replot();
    tripPlot->replot();
}




} // end namespace
