#pragma once

#include <qtablewidget.h>

#include <QComboBox>
#include <QPointF>
#include <QPushButton>
#include <QTableWidget>
#include <QWidget>
#include <QwtPlot>
#include <QwtPlotCurve>

#include "zerodatastream.hpp"

namespace zero {

class ZeroTripConfWidget : public QWidget
{
    Q_OBJECT;

public:
    ZeroTripConfWidget(QWidget *parent = nullptr);

    void setCurve(std::vector<QPointF> &curve);

    void clear();

signals:
    void sendStatusMessage(const QString &message);
    void applyNewTripCurve(std::vector<QPointF> curve);

private:
    QwtPlot *tripPlot;
    QwtPlotCurve tCurve;
    ZeroDataStream *customCurvePoints;
    ZeroDataStream *tCurvePointsOriginal;

    QTableWidget *zeroTripTable;
    QComboBox *tripTypeBox;

    bool isTripCurveValid(QTableWidget *zeroTable);
    void updateTripCurvePlot(QTableWidget *zeroTable);
    void addTripPoint(QTableWidget *zeroTable, double currentInA = 0.0, int timeInMs = 0);

    QWidget *createStandardTripCurveWiget(int trip_type, QwtPlotCurve *curve);
    QWidget *createCustomTripCurveWidget(QwtPlotCurve *curve);

    void createTripSettingsWidget();
};
}  // namespace zero
