#include <qabstractitemmodel.h>
#include <qwt_plot_curve.h>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QSpinBox>
#include <QSignalBlocker>
#include <QStackedWidget>
#include <QTableWidget>
#include <QTime>
#include <QTimeEdit>
#include <QVBoxLayout>

// QWT Items
#include <QwtLegend>
#include <QwtText>

#include "zerodatastream.hpp"
#include "zerotripconfwidget.hpp"
#include "timescaledraw.hpp"

namespace zero {

ZeroTripConfWidget::ZeroTripConfWidget(QWidget* parent) :
	QWidget(parent),	
  tripPlot(nullptr),
  tCurve(),
  customCurvePoints(new ZeroDataStream()),
  tCurvePointsOriginal(new ZeroDataStream()),
  zeroTripTable( new QTableWidget(0,2)),
  tripTypeBox(new QComboBox)
{
  tripTypeBox->addItem(tr("Select..."));
  tCurve.setTitle("Current");
  createTripSettingsWidget();		
}

QWidget* ZeroTripConfWidget::createStandardTripCurveWiget(int trip_type, QwtPlotCurve* curve)
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
    tTable->setHorizontalHeaderLabels(QStringList() << tr("xIₙ") << tr("I(A)") << tr("Time"));

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
                       
    connect(apply, &QAbstractButton::pressed, 
        [=]()
        {
            // 1. create std::vector for trip curve
            std::vector<QPointF> newTripCurve;
            uint rows = tTable->rowCount();

            for (uint i = 0; i < rows; ++i)
            {
                // get the real row
                int logRow = tTable->verticalHeader()->logicalIndex(i);
                double current = tTable->item(logRow, 1)->data(Qt::DisplayRole).toDouble() * 1000.0;
                int timeMs = tTable->item(logRow, 2)->data(Qt::DisplayRole).toTime().msecsSinceStartOfDay();
                newTripCurve.push_back(QPointF(current, timeMs));
            }

            emit applyNewTripCurve(newTripCurve);
        }
    );

    
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

QWidget* ZeroTripConfWidget::createCustomTripCurveWidget(QwtPlotCurve* curve)
{
    curve->setData(customCurvePoints);
    QWidget* customTrip = new QWidget;
        

        
    auto addTripPointB = new QPushButton("Add");
    auto delTripPointB = new QPushButton("Remove");
    auto applyTripCurveB = new QPushButton("Apply");
    auto resetTripCurveB = new QPushButton("Reset");
    
    zeroTripTable->setHorizontalHeaderLabels(QStringList() << "Current (A)" << "Time");
   // zeroTripTable->setDragDropMode(QAbstractItemView::InternalMove);
    zeroTripTable->setSelectionMode(QAbstractItemView::SingleSelection);
    zeroTripTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    zeroTripTable->verticalHeader()->setSectionsMovable(true);

    addTripPointB->setEnabled(true);
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

                addTripPointB->setEnabled(zeroTripTable->rowCount() < 16);
                delTripPointB->setEnabled(zeroTripTable->rowCount() != 0);
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

                delTripPointB->setEnabled(zeroTripTable->rowCount() != 0);
                addTripPointB->setEnabled(zeroTripTable->rowCount() < 16);
            }
        );

    connect(resetTripCurveB, &QAbstractButton::pressed,
            [=]()
            {
                curve->detach();
                customCurvePoints->clear();

                zeroTripTable->clearContents();
                zeroTripTable->setRowCount(0);
                
                for (int i = 0; i < tCurvePointsOriginal->size(); ++i)
                {
                    auto p = tCurvePointsOriginal->sample(i);
                    addTripPoint(zeroTripTable, p.rx(), p.ry());
                                           
                    customCurvePoints->append(p);

                }
                curve->attach(tripPlot);
                curve->show();
                
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

                emit applyNewTripCurve(newTripCurve);
                // 2. send trip curve to Zero
                // auto tZeroIdx = zeroTable->model()->index(selectedRowIdx, 0);
                // zeroTable->model()->setData(tZeroIdx,
                //                             QVariant::fromValue(static_cast<void*>(&newTripCurve)),
                //                             zero::TripCurve);

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

    // need for the init loading of the table
    connect(zeroTripTable->model(), &QAbstractItemModel::rowsInserted, 
      [=]()
      {
        addTripPointB->setEnabled(zeroTripTable->rowCount() < 16);
        delTripPointB->setEnabled(zeroTripTable->rowCount() != 0);
      }
    );
        
    return customTrip;
}
    
void ZeroTripConfWidget::createTripSettingsWidget()
{
    // QWidget* zeroTrip = this;
       
    // Setting up Trip Panel
    QwtText trip("Trip Curve");
    tripPlot = new QwtPlot(trip);
    tCurve.setStyle(QwtPlotCurve::CurveStyle::Steps);
    tCurve.setData(tCurvePointsOriginal);
    tripPlot->setAxisScaleDraw( QwtAxis::YLeft, new TimeScaleDraw( QTime(0, 0),"hh:mm:ss.zzz"));
//    plot->setAxisScale( QwtAxis::XBottom, 0, 60 );
//    plot->setAxisLabelRotation( QwtAxis::XBottom, -50.0 );
//    plot->setAxisLabelAlignment( QwtAxis::XBottom, Qt::AlignLeft | Qt::AlignBottom );


    QwtLegend *legend = new QwtLegend;
    //legend->setItemMode( QwtLegend::CheckableItem );
    tripPlot->insertLegend( legend, QwtPlot::RightLegend );
    
    QStackedWidget *stackedWidget = new QStackedWidget;
    std::vector<QwtPlotCurve*> trip_curves;
    trip_curves.push_back(nullptr);
    stackedWidget->addWidget(new QWidget);
    for (int i = 0; i < 3; ++i)
    {        
        QwtPlotCurve* tr_curve = new QwtPlotCurve;
        tr_curve->setPen( QColorConstants::Svg::darkorange);
        tr_curve->setStyle(QwtPlotCurve::CurveStyle::Steps);
        tr_curve->setTitle("New");
        trip_curves.push_back(tr_curve);
        QWidget *bCurveWidget = createStandardTripCurveWiget(i, tr_curve);
        stackedWidget->addWidget(bCurveWidget);
    }
        
    QwtPlotCurve* tr_curve = new QwtPlotCurve;
    tr_curve->setPen( QColorConstants::Svg::darkorange);
    tr_curve->setStyle(QwtPlotCurve::CurveStyle::Steps);
    tr_curve->setTitle("New");
    trip_curves.push_back(tr_curve); 
    QWidget *customCurveWidget = createCustomTripCurveWidget(tr_curve);

    stackedWidget->addWidget(customCurveWidget);

    tripTypeBox->addItem(tr("B"));
    tripTypeBox->addItem(tr("C"));
    tripTypeBox->addItem(tr("D"));
    tripTypeBox->addItem(tr("Custom"));

    connect(tripTypeBox, &QComboBox::currentIndexChanged,
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

    setLayout(tLayout);
}

void ZeroTripConfWidget::addTripPoint(QTableWidget* zeroTripTable, double currentInA, int timeInMs)
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

bool ZeroTripConfWidget::isTripCurveValid(QTableWidget* zeroTripTable)
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

void ZeroTripConfWidget::updateTripCurvePlot(QTableWidget* zeroTripTable)
{            
    uint rows = zeroTripTable->rowCount();

    customCurvePoints->clear();

    for (uint i = 0; i < rows; ++i)
    {
        // get the real row
        int logRow = zeroTripTable->verticalHeader()->logicalIndex(i);
        double current = zeroTripTable->item(logRow, 0)->data(Qt::DisplayRole).toDouble();
        int timeMs = static_cast<QTimeEdit*>(zeroTripTable->cellWidget(logRow, 1))->time().msecsSinceStartOfDay();
        customCurvePoints->append(QPointF(current, timeMs));
    }
    tripPlot->replot();
}

void ZeroTripConfWidget::setCurve(std::vector<QPointF>& curve)
{
    tCurve.detach();
    customCurvePoints->clear();
    tCurvePointsOriginal->clear();

    for (int i = 0; i < curve.size(); ++i)
    {
        double cVal = curve[i].rx() / 1000.0;
        addTripPoint(zeroTripTable, cVal, curve[i].ry());
                           
        customCurvePoints->append(QPointF(cVal, curve[i].ry()));
        tCurvePointsOriginal->append(QPointF(cVal,curve[i].ry()));

    }
    tCurve.attach(tripPlot);
    tCurve.show();
    tripPlot->replot();
}

void ZeroTripConfWidget::clear()
{
  zeroTripTable->clearContents();
  zeroTripTable->setRowCount(0);
  tripTypeBox->setCurrentIndex(0);
  tCurve.detach();
  tripPlot->replot();
}
  
}
