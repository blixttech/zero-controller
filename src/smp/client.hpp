#pragma once

#include "smp/smp.hpp"
#include "request.hpp"
#include "smp/imagemgmt.hpp"
#include "smp/osmgmt.hpp"

#include <QObject>
#include <QHostAddress>
#include <QUdpSocket>
#include <QStateMachine>
#include <QTimer>

namespace smp {

class Client : public QObject
{
    Q_OBJECT

public:
    Client(const QString& smpServer, uint16_t port = 1337,
                QObject* parent = nullptr);

    // sends the request
    // returns a pair of <send successful, seq numb>
    // if send successful, the seq number is the valid and will be in the matching reply
    std::pair<bool,uint8_t> send(Request& msg);

    bool sendFirmwareUpdate(const QByteArray& firmware);
    uint16_t firmwareUpdateProgress();
    
    bool isFirmwareUpdateOngoing();
    /*
        Returns true if successful,
        Otherwise false,
        std::nullopt if never ran
    */
    std::optional<bool> didFirmwareUpdateSucceed();
    

    bool connectToHost();
    void disconnectFromHost();
    bool isConnected() const;

signals:
    void receivedGetStateOfImagesResp(std::shared_ptr<GetStateOfImagesResp> reply);
    void receivedSetStateOfImagesResp(std::shared_ptr<SetStateOfImagesResp> reply);
    void receivedImageUploadResp(std::shared_ptr<ImageUploadResp> reply);
    void receivedResetResp(std::shared_ptr<ResetResp> reply);
    void connectionEstablished();

    void firmwareUpdateProgressing();
    void firmwareUpdateCompleted();
    void firmwareUpdateFailed();
    
    void imageChunkSent();
    void imageChunkWritten();
    void lastImageChunkWritten();
    void imageUploadFailed();
    void imageUploadCompleted();
    void receivedImageHash();
    void zeroRebooting();

private:
    void onReadyRead();

    QHostAddress host;
    uint16_t port;

    uint8_t seqCounter;
    
    std::unique_ptr<QUdpSocket> socket;
    
    QStateMachine uploadFw;
    uint8_t image;
    uint32_t chunkSize;
    uint32_t totalSize;
    uint32_t offset;
    double chunkPercentage;
    uint32_t chunksSent;
    uint16_t totalProgress;
    std::optional<bool> firmwareUpdateSuccess;
    
    QTimer awaitRespTimer;
    QByteArray firmware;
    // needed for the firmware
    QByteArray imageHash;

    void setupFirmwareUploadFSM();
    
};

} // end of namespace
