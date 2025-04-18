#include "zerogeneralconfwidget.hpp"

#include <qabstractitemmodel.h>
#include <qboxlayout.h>
#include <qcheckbox.h>
#include <qdatetimeedit.h>
#include <qnamespace.h>
#include <qspinbox.h>
#include <qwidget.h>

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QWidget>

namespace zero {

ZeroGeneralConfWidget::ZeroGeneralConfWidget(QWidget* parent) : QWidget(parent) { setupWidget(); }

void ZeroGeneralConfWidget::setupWidget()
{
    QVBoxLayout* vLay = new QVBoxLayout;

    // the settings boxes
    QWidget* cW = new QWidget;
    QHBoxLayout* cLay = new QHBoxLayout;

    QWidget* lC = new QWidget;
    QVBoxLayout* lCL = new QVBoxLayout;

    lC->setLayout(lCL);
    cLay->addWidget(lC);

    // initial state group
    iniStateGroup_ = new QGroupBox(tr("Initial State"));
    QHBoxLayout* iniL = new QHBoxLayout;
    QLabel* iniT = new QLabel(tr("Initial Switch State:"));
    iniSt_ = new QComboBox;
    iniSt_->addItem(tr("Open"));
    iniSt_->addItem(tr("Close"));
    iniSt_->addItem(tr("Previous"));

    iniL->addWidget(iniT);
    iniL->addWidget(iniSt_);
    iniStateGroup_->setLayout(iniL);

    // hardware limit group
    QGroupBox* hwLimit = new QGroupBox(tr("Hardware Limit"));
    QHBoxLayout* hwL = new QHBoxLayout;
    QLabel* cLimit = new QLabel(tr("H/W Current Limit (A)"));
    ocpHWLimit_ = new QSpinBox;
    ocpHWLimit_->setRange(0, 60);
    ocpHWLimit_->setValue(30);
    ocpHWLimit_->setSingleStep(1);

    hwL->addWidget(cLimit);
    hwL->addWidget(ocpHWLimit_);
    hwLimit->setLayout(hwL);

    lCL->addWidget(iniStateGroup_);
    lCL->addWidget(hwLimit);

    // auto recloser group
    QGroupBox* reClos = new QGroupBox(tr("Auto recloser"));
    QVBoxLayout* vreL = new QVBoxLayout;
    reClos->setLayout(vreL);

    autoReclosing_ = new QCheckBox(tr("Enable Autoreclosing"));
    vreL->addWidget(autoReclosing_);

    // recloser attempts
    recloserAttemtps_ = new QWidget;
    QHBoxLayout* recAL = new QHBoxLayout;
    recAttemptsSB_ = new QSpinBox;
    recAttemptsSB_->setRange(1, 1000);
    recAttemptsSB_->setValue(20);
    recAttemptsSB_->setSingleStep(1);
    recAL->addWidget(new QLabel(tr("Attempts")));
    recAL->addWidget(recAttemptsSB_);
    recloserAttemtps_->setLayout(recAL);
    vreL->addWidget(recloserAttemtps_);

    // recloser attempts delay
    recloserAttemtpsDelay_ = new QWidget;
    QHBoxLayout* recDWL = new QHBoxLayout;
    recDelay_ = new QSpinBox;
    recDelay_->setRange(10, 1000);
    recDelay_->setValue(20);
    recDelay_->setSingleStep(1);
    recDWL->addWidget(new QLabel(tr("Delay")));
    recDWL->addWidget(recDelay_);
    recloserAttemtpsDelay_->setLayout(recDWL);
    vreL->addWidget(recloserAttemtpsDelay_);

    connect(autoReclosing_, &QCheckBox::stateChanged, [=](int state) {
        bool enable_rec = false;
        switch (state) {
            case Qt::Checked:
                enable_rec = true;
        }
        enableAWRWidgets(enable_rec);
    });

    reClos->setLayout(vreL);

    // create final window
    cLay->addWidget(reClos);
    cLay->addStretch();
    cW->setLayout(cLay);

    // the buttons
    QWidget* bW = new QWidget;
    QHBoxLayout* bLay = new QHBoxLayout;
    QPushButton* oB = new QPushButton(tr("Apply"));
    QPushButton* rB = new QPushButton(tr("Reset"));
    bLay->addWidget(oB);
    bLay->addWidget(rB);
    bLay->addStretch();
    bW->setLayout(bLay);

    vLay->addWidget(cW);
    vLay->addWidget(bW);
    setLayout(vLay);

    connect(oB, &QAbstractButton::pressed, [=]() { emit ApplyValuesToZero(); });
    connect(rB, &QAbstractButton::pressed, [=]() { emit RequestingReload(); });
}

void ZeroGeneralConfWidget::clear() {}

void ZeroGeneralConfWidget::enableAutoReclosing(bool enable)
{
    autoReclosing_->setChecked(enable);
    enableAWRWidgets(enable);
}

void ZeroGeneralConfWidget::disableInitState() { iniStateGroup_->setEnabled(false); }

void ZeroGeneralConfWidget::setInitState(int state)
{
    iniStateGroup_->setEnabled(true);
    iniSt_->setCurrentIndex(state);
}

void ZeroGeneralConfWidget::enableAWRWidgets(bool enabled)
{
    recloserAttemtps_->setEnabled(enabled);
    recloserAttemtpsDelay_->setEnabled(enabled);
}

void ZeroGeneralConfWidget::setReclosingAttempts(int attemtps)
{
    recAttemptsSB_->setValue(attemtps);
}
int ZeroGeneralConfWidget::getReclosingAttempts() { return recAttemptsSB_->value(); }

void ZeroGeneralConfWidget::setReclosingAttemptsDelay(int delay) { recDelay_->setValue(delay); }
int ZeroGeneralConfWidget::getReclosingAttemptsDelay() { return recDelay_->value(); }

void ZeroGeneralConfWidget::setOCPHWLimit(int value) { ocpHWLimit_->setValue(value); }

int ZeroGeneralConfWidget::getOCPHWLimit() { return ocpHWLimit_->value(); }

int ZeroGeneralConfWidget::initState() { return iniSt_->currentIndex(); }

bool ZeroGeneralConfWidget::isAutoReclosingEnabled() { return autoReclosing_->isChecked(); }

}  // namespace zero
