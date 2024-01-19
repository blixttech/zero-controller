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

#include <qnamespace.h>

#include <QwtPlotCurve>
#include <QwtText>
#include <QwtScaleWidget>
#include <QwtScaleDraw>

namespace zero {

class TimeScaleDraw : public QwtScaleDraw
{
  public:
    TimeScaleDraw( const QTime& base )
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
    plot->setAxisTitle( QwtAxis::XBottom, " System Uptime [h:m:s]" );
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
    zeroDetails(new QGroupBox("Overview", this)),
    selectedRowIdx(-1)
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
    voltagePlot = new QwtPlot(vo, zeroDetails);
    voltagePlot->setAxisTitle( QwtAxis::YLeft, "Voltage [V]" );
    vCurve = nullptr;
    configureTimeAxis(voltagePlot);

    QwtText cu("Current");
    currentPlot = new QwtPlot(cu, zeroDetails);
    currentPlot->setAxisTitle( QwtAxis::YLeft, "Current [A]" );
    cCurve = nullptr;
    configureTimeAxis(currentPlot);

    QwtText po("Power");
    powerPlot = new QwtPlot(po, zeroDetails);
    powerPlot->setAxisTitle( QwtAxis::YLeft, "Power [W]" );
    pCurve = nullptr;
    configureTimeAxis(powerPlot);

    QwtText freq("Frequency");
    frequencyPlot = new QwtPlot(freq, zeroDetails);
    frequencyPlot->setAxisTitle( QwtAxis::YLeft, "Frequency [Hz]" );
    fCurve = nullptr;
    configureTimeAxis(frequencyPlot);


    auto overview = new QGridLayout(zeroDetails);
    overview->addWidget(voltagePlot, 0, 0);
    overview->addWidget(currentPlot, 0, 1);
    overview->addWidget(powerPlot, 1, 0);
    overview->addWidget(frequencyPlot, 1, 1);

    zeroDetails->setLayout(overview);

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
                if (selected.size() == 1)
                {
                    qDebug() << "=============================================================";
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
                    
                    replot();
                }
                else { 
                    // unselecting a row
                    selectedRowIdx = -1;
                    vCurve->detach();
                    vCurve = nullptr;
                    
                    cCurve->detach();
                    cCurve = nullptr;
                    
                    pCurve->detach();
                    pCurve = nullptr;
                    
                    fCurve->detach();
                    fCurve = nullptr;

                    replot();
                }
                    
            });

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
}

} // end namespace
