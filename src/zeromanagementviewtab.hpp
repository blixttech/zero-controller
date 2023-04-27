#pragma once

#include <QWidget>
#include <QTableView>
#include <QTabWidget> 
#include <QPushButton>
#include <QGroupBox>
#include <QLabel> 

#include "zeromanagementviewmodel.hpp"
#include "firmwareupdatefilter.hpp"

namespace zero {

class ZeroManagementViewTab : public QWidget
{
    public:
        ZeroManagementViewTab(QWidget* parent = nullptr);

        void setModel(ZeroManagementViewModel* model); 
    private: 
        QTableView*   zeroTable;

        QTabWidget*    mgmtWidgets;
    
        // Firmware update tab
        QWidget*              fwUpdateTab;
        QGroupBox*            fwInfo;
        QLabel*               fileNameLabel;
        QLabel*               fileNameStr;
        QLabel*               versionLabel;
        QLabel*               fwVersionStr;
        QLabel*               sizeLabel;
        QLabel*               fwSizeStr;
        QPushButton*          selectButton;
        QPushButton*          updateButton;
        QTableView*           updateStatus;
        FirmwareUpdateFilter* fwFilter;        
        QByteArray firmwareFile;

        

        void setupZeroTable();
        void setupMgmtTabs();
        void setupFwUploadTab();


        bool isFileValidFw(const QByteArray &fileContent) const;
        void updateFirmwareFields(const QString &fileName, const QByteArray &fileContent);

        void selectFirmware(bool checked);
};

} // end namespace
