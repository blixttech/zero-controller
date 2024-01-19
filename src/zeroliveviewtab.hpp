#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QGroupBox>

#include <qwt_plot.h>

#include "zeroliveviewmodel.hpp"

namespace zero {

class ZeroLiveViewTab : public QWidget
{
    public:
        ZeroLiveViewTab(QWidget* parent = nullptr);
        ~ZeroLiveViewTab();

        void setModel(ZeroLiveViewModel* model); 
    private: 
        QTableView*   zeroTable;

        QGroupBox*    zeroDetails;
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
};

} // end namespace
