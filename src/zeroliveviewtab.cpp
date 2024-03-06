#include "zeroliveviewtab.hpp"
#include "openclosebuttondelegate.hpp"
#include "zerodatastream.hpp"
#include "zeroliveviewmodel.hpp"

#include <QAbstractButton>
#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QBoxLayout>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QItemSelectionModel>
#include <QLabel>
#include <QPoint>
#include <QPushButton>
#include <QSpinBox>
#include <QSignalBlocker>
#include <QStackedWidget>
#include <QTableWidget>
#include <QTextStream>
#include <QTime>
#include <QVBoxLayout>
#include <QWidget>


#include <qboxlayout.h>
#include <qcolor.h>
#include <qlabel.h>
#include <qnamespace.h>
#include <qtablewidget.h>
#include <qwidget.h>
#include <qwt_plot_curve.h>

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
    zeroTrip(nullptr),
    tripPlot(nullptr),
    tCurve(),
    tCurvePoints(new ZeroDataStream()),
    tCurvePointsOriginal()
{
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


    zeroTrip = createTripSettingsWidget();
    zeroTrip->setEnabled(false);
    zeroDetails->addTab(zeroTrip, tr("Trip Settings"));
    
    mainLayout->addWidget(zeroDetails, 50.0);
    
    setLayout(mainLayout);


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

QWidget* ZeroLiveViewTab::createStandardTripCurveWiget(int trip_type, QwtPlotCurve* curve)
{   
    std::vector<int> max_nominal = { 16, 10, 5 };


    QWidget *curveWidget = new QWidget;
        

    // the control buttons
    QPushButton* apply = new QPushButton(tr("Apply Curve"));
        
    // nominal current input
    QSpinBox* inE = new QSpinBox;
    inE->setRange(1,max_nominal[trip_type]);
    inE->setValue(max_nominal[trip_type]);
    inE->setSingleStep(1);
        
    // short trip time
    QSpinBox* sTr = new QSpinBox;
    sTr->setRange(2,59);
    sTr->setValue(1);
    sTr->setSingleStep(1);
        
    ZeroDataStream* curve_data = new ZeroDataStream;
    QTableWidget* tTable = new QTableWidget(0,3);
    tTable->setHorizontalHeaderLabels(QStringList() << tr("xI <sub>n</sub>") << tr("I(A)") << tr("Time"));

    auto calc_trip = [=]()
    {
        int nominal_cur = inE->value();
        std::vector< std::pair<double, int>> trip_curve_params =
        {
            {1.13, 3601000 }, //1 hour and 1 sec
            {1.24, 1800500 },
            {1.35,  900250 },
            {1.45,  450125 }
        };


        double c_step_size = 0.11;
        double c_s = 1.45;

        int l_b = sTr->value()*1000;
        
        int t_s = 450125;
        int t_step_size = (t_s - l_b) / 10;
        // qDebug() << "Step Size is :" << t_step_size;

        for (int i = 0; i < 9; ++i)
        {
            c_s += c_step_size;
            t_s -= t_step_size;

            trip_curve_params.push_back(std::make_pair(c_s, t_s));

        }

        trip_curve_params.push_back(std::make_pair(2.55, l_b));

        //         {
        //     {1.56,  225062 },
        //     {1.67,  112532 },
        //     {1.78,   56266 },
        //     {1.89,   28133 },
        //     {2.00,   14067 },
        //     {2.11,    7034 },
        //     {2.22,    3517 },
        //     {2.33,    3000 },
        //     {2.44,    2500 },
        //     {2.55,    2000 }   
        // };

        std::vector< std::vector< std::pair<double, int>>> instant_trip_points =
        {
            // B
            {
                {3.0,    200 },  //200 mill         
                {3.1,      0 }
            },
            // c
            {
                {5.0,    200 },  //200 mill         
                {5.1,      0 }
            },
            //D
            {
                {10.0,    200 },  //200 mill         
                {10.1,      0 }
            },
        };

        auto inst_curve = instant_trip_points[trip_type];
        std::copy (inst_curve.begin(), inst_curve.end(), std::back_inserter(trip_curve_params));
                
        curve_data->clear();
        tTable->clearContents();
        tTable->setRowCount(trip_curve_params.size());
        
        for (int i = 0; i < trip_curve_params.size(); ++i)
        {
            auto e = trip_curve_params[i];
            double cur = e.first * nominal_cur;

            curve_data->append(QPointF(cur, e.second));        

            QTableWidgetItem* nFac = new QTableWidgetItem;
            nFac->setData(Qt::DisplayRole, e.first);
            nFac->setFlags( nFac->flags() & ~Qt::ItemIsEditable);
            
            QTableWidgetItem* current = new QTableWidgetItem();
            current->setData(Qt::DisplayRole, cur);
            current->setFlags( current->flags() & ~Qt::ItemIsEditable);

            QTableWidgetItem* ttrip = new QTableWidgetItem;
            auto tt = QTime::fromMSecsSinceStartOfDay(e.second);
            ttrip->setData(Qt::DisplayRole,tt.toString("hh:mm:ss.zzz"));
            ttrip->setFlags( ttrip->flags() & ~Qt::ItemIsEditable);

            tTable->setItem(i, 0, nFac);
            tTable->setItem(i, 1, current);
            tTable->setItem(i, 2, ttrip);
        }

        curve->setData(curve_data);
    };
    calc_trip();

    connect(inE, &QSpinBox::valueChanged, calc_trip);
    connect(sTr, &QSpinBox::valueChanged, calc_trip);
                       
    QHBoxLayout* inL = new QHBoxLayout;
    inL->addWidget(new QLabel(tr("I<sub>n</sub>")));
    inL->addWidget(inE);
    inL->addWidget(new QLabel(tr("Short Time Trip(s)")));
    inL->addWidget(sTr);
    inL->insertStretch(2);
    inL->addWidget(apply);
            
    QWidget* InW = new QWidget;
    InW->setLayout(inL);
    
    // Putting it all together
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(tTable);
    layout->addWidget(InW);

    curveWidget->setLayout(layout);

    return curveWidget;
}

QWidget* ZeroLiveViewTab::createCustomTripCurveWidget(QwtPlotCurve* curve)
{
    QWidget* customTrip = new QWidget;
        
    QTableWidget* zeroTripTable = new QTableWidget(0,2);

        
    auto addTripPointB = new QPushButton("Add");
    auto delTripPointB = new QPushButton("Remove");
    auto applyTripCurveB = new QPushButton("Apply");
    auto resetTripCurveB = new QPushButton("Reset");
    
    zeroTripTable->setHorizontalHeaderLabels(QStringList() << "Current (A)" << "Time");
   // zeroTripTable->setDragDropMode(QAbstractItemView::InternalMove);
    zeroTripTable->setSelectionMode(QAbstractItemView::SingleSelection);
    zeroTripTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    zeroTripTable->verticalHeader()->setSectionsMovable(true);

    addTripPointB->setEnabled(false);
    delTripPointB->setEnabled(false);
    applyTripCurveB->setEnabled(false);
    resetTripCurveB->setEnabled(false);


    
    QVBoxLayout* bLayout = new QVBoxLayout;
    bLayout->addWidget(addTripPointB);
    bLayout->addWidget(delTripPointB);
    bLayout->addWidget(applyTripCurveB);
    bLayout->addWidget(resetTripCurveB);

    QWidget* bGroup = new QWidget;
    bGroup->setLayout(bLayout);

    QHBoxLayout* mLayout = new QHBoxLayout;
    mLayout->addWidget(zeroTripTable);
    mLayout->addWidget(bGroup);

    customTrip->setLayout(mLayout);
        

    connect(addTripPointB, &QAbstractButton::pressed, 
            [=]()
            {
                addTripPoint(zeroTripTable);

                applyTripCurveB->setEnabled(isTripCurveValid(zeroTripTable));
                updateTripCurvePlot(zeroTripTable);
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
                applyTripCurveB->setEnabled(isTripCurveValid(zeroTripTable));
                updateTripCurvePlot(zeroTripTable);
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
                    addTripPoint(zeroTripTable, p.rx(), p.ry());
                                           
                    tCurvePoints->append(p);

                }
                tCurve.attach(tripPlot);
                tCurve.show();
                
                applyTripCurveB->setEnabled(isTripCurveValid(zeroTripTable));
                updateTripCurvePlot(zeroTripTable);
                resetTripCurveB->setEnabled(false);
            }
        );

    connect(applyTripCurveB, &QAbstractButton::pressed, 
            [=]()
            {
                // 1. create std::vector for trip curve
                std::vector<QPointF> newTripCurve;
                uint rows = zeroTripTable->rowCount();

                for (uint i = 0; i < rows; ++i)
                {
                    // get the real row
                    int logRow = zeroTripTable->verticalHeader()->logicalIndex(i);
                    double current = zeroTripTable->item(logRow, 0)->data(Qt::DisplayRole).toDouble() * 1000.0;
                    int timeMs = static_cast<QTimeEdit*>(zeroTripTable->cellWidget(logRow, 1))->time().msecsSinceStartOfDay();
                    newTripCurve.push_back(QPointF(current, timeMs));
                }
                // 2. send trip curve to Zero
                auto tZeroIdx = zeroTable->model()->index(selectedRowIdx, 0);
                zeroTable->model()->setData(tZeroIdx,
                                            QVariant::fromValue(static_cast<void*>(&newTripCurve)),
                                            zero::TripCurve);

                // 3. reset UI
            }
        );

    connect(zeroTripTable, &QTableWidget::cellChanged, 
            [=](int row, int col)
            {
                applyTripCurveB->setEnabled(isTripCurveValid(zeroTripTable));
                updateTripCurvePlot(zeroTripTable);
                resetTripCurveB->setEnabled(true);
            }
        );

    connect(zeroTripTable->verticalHeader(), &QHeaderView::sectionMoved,
            [=](int logIdx, int oldIdx, int newIdx)
            {
                applyTripCurveB->setEnabled(isTripCurveValid(zeroTripTable));
                updateTripCurvePlot(zeroTripTable);

                QStringList rowLabels;
                for (int i = 0; i < zeroTripTable->rowCount(); ++i)
                {
                    
                    rowLabels << QString::number(zeroTripTable->visualRow(i)+1);
                }

                zeroTripTable->setVerticalHeaderLabels(rowLabels);
                resetTripCurveB->setEnabled(true);

            }
        );
        
    return customTrip;
}
    
QWidget* ZeroLiveViewTab::createTripSettingsWidget()
{
    QWidget* zeroTrip = new QWidget;
       
    // Setting up Trip Panel
    QwtText trip("Trip Curve");
    tripPlot = new QwtPlot(trip, zeroTrip);
    tCurve.setStyle(QwtPlotCurve::CurveStyle::Steps);
    tCurve.setData(tCurvePoints);
    tripPlot->setAxisScaleDraw( QwtAxis::YLeft, new TimeScaleDraw( QTime(0, 0),"hh:mm:ss.zzz"));
//    plot->setAxisScale( QwtAxis::XBottom, 0, 60 );
//    plot->setAxisLabelRotation( QwtAxis::XBottom, -50.0 );
//    plot->setAxisLabelAlignment( QwtAxis::XBottom, Qt::AlignLeft | Qt::AlignBottom );
    
    QStackedWidget *stackedWidget = new QStackedWidget;
    std::vector<QwtPlotCurve*> trip_curves;
    trip_curves.push_back(nullptr);
    stackedWidget->addWidget(new QWidget);
    for (int i = 0; i < 3; ++i)
    {        
        QwtPlotCurve* tr_curve = new QwtPlotCurve;
        tr_curve->setPen( QColorConstants::Svg::darkorange);
        tr_curve->setStyle(QwtPlotCurve::CurveStyle::Steps);
        trip_curves.push_back(tr_curve);
        QWidget *bCurveWidget = createStandardTripCurveWiget(i, tr_curve);
        stackedWidget->addWidget(bCurveWidget);
    }
        
    QwtPlotCurve* tr_curve = new QwtPlotCurve;
    tr_curve->setPen( QColorConstants::Svg::darkorange);
    tr_curve->setStyle(QwtPlotCurve::CurveStyle::Steps);
    trip_curves.push_back(tr_curve); 
    QWidget *customCurveWidget = createCustomTripCurveWidget(tr_curve);

    stackedWidget->addWidget(customCurveWidget);

    QComboBox *tripTypeBox = new QComboBox;
    tripTypeBox->addItem(tr("Select..."));
    tripTypeBox->addItem(tr("B"));
    tripTypeBox->addItem(tr("C"));
    tripTypeBox->addItem(tr("D"));
    tripTypeBox->addItem(tr("Custom"));

    connect(tripTypeBox, &QComboBox::activated,
            [=](int idx)
            {
                stackedWidget->setCurrentIndex(idx);
                for (int i = 0; i < trip_curves.size(); ++i)
                {
                    if (trip_curves[i] == nullptr) continue;
                    
                    if (i == idx) trip_curves[i]->attach(tripPlot);
                    else trip_curves[i]->detach();
                }
                tripPlot->replot(); 
            }
        );

    QWidget* tripTypeW = new QWidget;
    QHBoxLayout* tTl = new QHBoxLayout;
    tTl->addWidget(new QLabel(tr("Type")));
    tTl->addWidget(tripTypeBox);
    tripTypeW->setLayout(tTl);

    QVBoxLayout *tGlayout = new QVBoxLayout;
    tGlayout->addWidget(tripTypeW);
    tGlayout->addWidget(stackedWidget);

        
    QGroupBox* tripConf = new QGroupBox(tr("Trip Config"));
    tripConf->setFlat(false);
    tripConf->setLayout(tGlayout);

    auto tLayout = new QHBoxLayout;
    tLayout->addWidget(tripPlot);
    tLayout->addWidget(tripConf);

    zeroTrip->setLayout(tLayout);

    return zeroTrip;
}

void ZeroLiveViewTab::addTripPoint(QTableWidget* zeroTripTable, double currentInA, int timeInMs)
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

bool ZeroLiveViewTab::isTripCurveValid(QTableWidget* zeroTripTable)
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

void ZeroLiveViewTab::updateTripCurvePlot(QTableWidget* zeroTripTable)
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
                zeroTrip->setEnabled(false);

                // zeroTripTable->clearContents();
                // zeroTripTable->setRowCount(0);
                
                // addTripPointB->setEnabled(false);
                // delTripPointB->setEnabled(false);
                // applyTripCurveB->setEnabled(false);
                // resetTripCurveB->setEnabled(false);
            
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
//                        addTripPoint(cVal, tVec[i].ry());
                                               
                        tCurvePoints->append(QPointF(cVal, tVec[i].ry()));
                        tCurvePointsOriginal.append(QPointF(cVal, tVec[i].ry()));

                    }
                    tCurve.attach(tripPlot);
                    tCurve.show();

                    
                    // addTripPointB->setEnabled(true);
                    // delTripPointB->setEnabled(true);
                    // applyTripCurveB->setEnabled(false);
                    // resetTripCurveB->setEnabled(false);
                    zeroTrip->setEnabled(true);
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
