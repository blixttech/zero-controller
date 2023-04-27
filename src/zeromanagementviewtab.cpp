#include "zeromanagementviewtab.hpp"

#include <QHeaderView>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QFileDialog> 

#include "firmwareupdatefilter.hpp"

#include "mcuimage.h"

namespace zero {

ZeroManagementViewTab::ZeroManagementViewTab(QWidget* parent) : QWidget(parent),
    zeroTable(new QTableView(this)),
    mgmtWidgets(new QTabWidget(this)),
    fwUpdateTab(new QWidget(this)),
    fwInfo(new QGroupBox("Firmware", fwUpdateTab)),
    fileNameLabel(new QLabel("File:", fwInfo)),
    fileNameStr(new QLabel("N/A:", fwInfo)),
    versionLabel(new QLabel("Version:", fwInfo)),
    fwVersionStr(new QLabel("N/A", fwInfo)),
    sizeLabel(new QLabel("Size:", fwInfo)),
    fwSizeStr(new QLabel("N/A", fwInfo)),
    selectButton(new QPushButton("Select", fwInfo)),
    updateButton(new QPushButton("Update", fwInfo)),
    updateStatus(new QTableView(fwUpdateTab)),
    fwFilter(new FirmwareUpdateFilter(updateStatus)),
    firmwareFile()
{
    setupZeroTable();

    setupMgmtTabs();
    
    

    // setting up the window layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(zeroTable);
    mainLayout->addWidget(mgmtWidgets);
    setLayout(mainLayout);
}


void ZeroManagementViewTab::setupZeroTable()
{
    // Setup upper part of the ui
    zeroTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    zeroTable->verticalHeader()->setVisible(true);
    zeroTable->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    zeroTable->setEditTriggers(QTableView::AllEditTriggers);
    zeroTable->setSelectionBehavior( QAbstractItemView::SelectRows );
} 


void ZeroManagementViewTab::setupMgmtTabs()
{
    setupFwUploadTab();
    mgmtWidgets->addTab(fwUpdateTab, tr("Firmware Update"));
}


qsizetype findHeaderIndex(const QByteArray &fileContent)
{
    uint32_t magic = IMAGE_MAGIC;

    char* md = (char*)&magic;

    QByteArrayView magicPattern(md, sizeof(uint32_t)/sizeof(char));

    return fileContent.indexOf(magicPattern);
}

bool parseFirmwareHeader(const QByteArray &fileContent, image_header& imgInfo)
{
    auto idx = findHeaderIndex(fileContent);
    if (idx == -1) return false;
    imgInfo = *((image_header*)(fileContent.data()+idx));
    return true;
}

void ZeroManagementViewTab::setupFwUploadTab()
{
//    updateStatus->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    updateStatus->verticalHeader()->setVisible(true);
    updateStatus->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

    updateButton->setEnabled(false);
    auto fwBox = new QGridLayout(fwInfo);
    fwBox->addWidget(fileNameLabel,0,0);
    fwBox->addWidget(fileNameStr,0,1);
    fwBox->addWidget(versionLabel,1,0);
    fwBox->addWidget(fwVersionStr,1,1);
    fwBox->addWidget(sizeLabel,2,0);
    fwBox->addWidget(fwSizeStr,2,1);
    fwBox->addWidget(updateButton,3,0);
    fwBox->addWidget(selectButton,3,1);
    fwInfo->setLayout(fwBox);

    
    
    QHBoxLayout *mainLayout = new QHBoxLayout(fwUpdateTab);
    mainLayout->addWidget(fwInfo);
    mainLayout->addWidget(updateStatus);
    
    fwUpdateTab->setLayout(mainLayout);

    
    connect(selectButton, &QPushButton::clicked,
            this, &ZeroManagementViewTab::selectFirmware);

    connect(updateButton, &QPushButton::clicked,
            [&](bool checked)
            {
                fwFilter->initiateFwUpdate(firmwareFile);    
            }
    );
}

void ZeroManagementViewTab::selectFirmware(bool checked)
{
    auto fileContentReady = [&](const QString &fileName, const QByteArray &fileContent) 
    {
        if (fileName.isEmpty()) 
        {
            qInfo() << "No file selected";
        } 
        else 
        {
            qInfo() << "Selected " << fileName;
        }

        if (!isFileValidFw(fileContent))
       {
            fileNameStr->setText("Invalid firmware file selected");
            updateButton->setEnabled(false);
            return;
        }
            
        qInfo() << "File contains valid firmware";
        updateFirmwareFields(fileName,fileContent);
        updateButton->setEnabled(true);
    };

    QFileDialog::getOpenFileContent("Bin files (*.bin)", fileContentReady);
}

void ZeroManagementViewTab::updateFirmwareFields(const QString &fileName, const QByteArray &fileContent)
{
    fileNameStr->setText(fileName);

    image_header header;
    parseFirmwareHeader(fileContent, header);
    auto ver = &(header.ih_ver);
    QString versionStr = QString::number(ver->iv_major) + "." + QString::number(ver->iv_minor) + "." + QString::number(ver->iv_revision);
    fwVersionStr->setText(versionStr);

    fwSizeStr->setText(QString::number(header.ih_img_size));
    firmwareFile = fileContent;
}

bool ZeroManagementViewTab::isFileValidFw(const QByteArray &fileContent) const
{
    return findHeaderIndex(fileContent) != -1;
}

void ZeroManagementViewTab::setModel(ZeroManagementViewModel* model)
{
    zeroTable->setModel(model);
    fwFilter->setSourceModel(model);
    fwFilter->setSelectionModel(zeroTable->selectionModel());
    updateStatus->setModel(fwFilter);
}

} // end namespace
