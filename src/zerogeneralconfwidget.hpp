#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QPushButton>
#include <QSpinBox>
#include <QWidget>

namespace zero {

class ZeroGeneralConfWidget : public QWidget
{
    Q_OBJECT;

public:
    ZeroGeneralConfWidget(QWidget* parent = nullptr);

    void disableInitState();
    void setInitState(int state);
    int initState();

    void enableAutoReclosing(bool);
    bool isAutoReclosingEnabled();
    void setReclosingAttempts(int);
    int getReclosingAttempts();

    void setReclosingAttemptsDelay(int);
    int getReclosingAttemptsDelay();

    void setOCPHWLimit(int);
    int getOCPHWLimit();

    void clear();

signals:
    void SendStatusMessage(const QString& message);
    void RequestingReload();
    void ApplyValuesToZero();

private:
    void setupWidget();
    void enableAWRWidgets(bool);

    QGroupBox* iniStateGroup_;
    QComboBox* iniSt_;

    QCheckBox* autoReclosing_;
    QWidget* recloserAttemtps_;
    QWidget* recloserAttemtpsDelay_;

    QSpinBox* ocpHWLimit_;

    QSpinBox* recAttemptsSB_;
    QSpinBox* recDelay_;
};

}  // namespace zero
