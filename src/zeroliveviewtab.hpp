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
#include "zerotripconfwidget.hpp"

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
        ZeroTripConfWidget*      zeroTrip;

        void replot();
         
};

} // end namespace
