#pragma once
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <QWidget>
#include <QTableWidget>
#include <QTabWidget> 
#include <QPushButton>

#include <QwtPlot>

#include "smp/smp.hpp"
#include "zerodatastream.hpp"
#include "zeroliveviewmodel.hpp"

namespace zero {

class ZeroLiveViewTab : public QWidget
{
        Q_OBJECT
    public:
        ZeroLiveViewTab(QWidget* parent = nullptr);
        ~ZeroLiveViewTab();

        void setModel(ZeroLiveViewModel* model);

        signals:
            void sendStatusMessage( const QString & message ); 
    private: 
        QTableView*   zeroTable;

        QTabWidget*   zeroDetails;

        QWidget*      zeroPlots;
        QwtPlot*      voltagePlot;
        QwtPlotCurve* vCurve;
        
        QwtPlot*      currentPlot;
        QwtPlotCurve* cCurve;
        
        QwtPlot*      powerPlot;
        QwtPlotCurve* pCurve;
        
        QwtPlot*      frequencyPlot;
        QwtPlotCurve* fCurve;

        int selectedRowIdx;

        void replot();

        QWidget*      zeroTrip;
        QwtPlot*      tripPlot;
        QwtPlotCurve  tCurve;
        ZeroDataStream* tCurvePoints;
        ZeroDataStream tCurvePointsOriginal;
        QTableWidget* zeroTripTable;
        QPushButton*  addTripPointB;
        QPushButton*  delTripPointB;
        QPushButton*  applyTripCurveB;
        QPushButton*  resetTripCurveB;

        bool isTripCurveValid();
        void updateTripCurvePlot();
        void addTripPoint(double currentInA = 0.0, int timeInMs = 0);
        
         
};

} // end namespace
