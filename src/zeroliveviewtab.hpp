#pragma once
#include <QTabWidget>
#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QComboBox>
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
        

        bool isTripCurveValid(QTableWidget* zeroTable);
        void updateTripCurvePlot(QTableWidget* zeroTable);
        void addTripPoint(QTableWidget* zeroTable, double currentInA = 0.0, int timeInMs = 0);


        QWidget* createStandardTripCurveWiget(int trip_type, QwtPlotCurve* curve);
        QWidget* createCustomTripCurveWidget(QwtPlotCurve* curve);
    
        QWidget* createTripSettingsWidget();        
         
};

} // end namespace
