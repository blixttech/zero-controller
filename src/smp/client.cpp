#include "client.hpp"

#include <QByteArray> 
#include <QByteArrayView>
#include <QNetworkDatagram>
#include <QCryptographicHash>
#include <QFinalState>


#include "smp/smp.hpp"


namespace smp {

#define MSGTYPE(OP, GROUP, CMD) ((OP << 24) | (GROUP << 8) | CMD) 

enum MessageTypes
{
    GetStateOfImagesRespType = MSGTYPE(MgmtOp::READ_RSP, MgmtGroupId::ID_IMAGE, ImgMgmtId::ID_STATE),
    SetStateOfImagesRespType = MSGTYPE(MgmtOp::WRITE_RSP, MgmtGroupId::ID_IMAGE, ImgMgmtId::ID_STATE),
    ImageUploadRespType = MSGTYPE(MgmtOp::WRITE_RSP, MgmtGroupId::ID_IMAGE, ImgMgmtId::ID_UPLOAD),
    ResetRespType = MSGTYPE(MgmtOp::WRITE_RSP, MgmtGroupId::ID_OS, OsMgmtId::ID_RESET) 
};

Client::Client(const QString& smpServer, uint16_t port,
            QObject* parent) : QObject(parent),
            host(smpServer), port(port),
            seqCounter(1), socket(),
            uploadFw(),
            image(0), chunkSize(512),
            totalSize(0), offset(0),
            chunkPercentage(0), chunksSent(0),
            totalProgress(0),
            firmwareUpdateSuccess(std::nullopt),
            awaitRespTimer(),
            firmware(),
            imageHash()
{
        setupFirmwareUploadFSM();
}

bool Client::isConnected() const
{
    return (nullptr != socket);
}

void Client::disconnectFromHost()
{
    socket.reset();
}

std::pair<bool, uint8_t> Client::send(Request& msg)
{
    if (!isConnected()) 
    {
        qDebug() << "Smpclient not connected";
        return std::make_pair(false, 0);
    }

    msg.setSeq(seqCounter);
    seqCounter++;

    QByteArray data;
    msg.serialize(data);

    socket->write(data);
    return std::make_pair(true, seqCounter);
}

bool Client::isFirmwareUpdateOngoing()
{
    return uploadFw.isRunning();    
}

std::optional<bool> Client::didFirmwareUpdateSucceed()
{
    return firmwareUpdateSuccess;        
}
    
bool Client::sendFirmwareUpdate(const QByteArray& fw)
{
    if (uploadFw.isRunning())
    {
        qWarning() << "Firmware upload is in progress. Wait until it is completed.";
        return false;        
    }
        
    firmware = fw;

    totalSize = firmware.length();
    offset = 0;
    

    uploadFw.start();
    return true;
}

uint16_t Client::firmwareUpdateProgress()
{
    return totalProgress;
}

void Client::setupFirmwareUploadFSM()
{   
    auto initial_req = new QState();
    auto await_resp = new QState();
    auto send_chunk = new QState();
    auto upload_completed = new QState();
    auto mark_upld_permanent = new QState();
    auto rebooting_zero = new QState();
    auto fw_update_completed = new QFinalState();
    auto upload_error = new QFinalState();

    // local function to handle firmwareUploadResp
    connect(this, &Client::receivedImageUploadResp,
            [&](std::shared_ptr<ImageUploadResp> reply)
            {
                if (reply->rc() != 0)
                {
                    qWarning() << "ImageUpload failed with rc " << reply->rc();
                    emit imageUploadFailed();
                }
                else
                {
                    qDebug() << "Last written offset: " << reply->off();
                    if (reply->off() == totalSize)
                    {
                        qDebug() << "Last byte of image was written";
                        emit lastImageChunkWritten();
                        return;
                    }
                    else if (reply->off() > offset)
                    {
                        // Got an old session, resume
                        offset = reply->off();
                        qInfo() << "Resumming aborted upload";
                    }
                    emit imageChunkWritten();                    
                }
            }
    );

    connect(this, &Client::receivedGetStateOfImagesResp,
            [&](std::shared_ptr<GetStateOfImagesResp> reply)
            {
                qDebug() << "Got StateOfImages";
                if (reply->images().size() != 2)
                    return;
                imageHash = reply->images()[1].hash;
                emit receivedImageHash();                
            }
    );

    // initial_req state
    initial_req->addTransition(this, &Client::imageChunkSent, await_resp);
    initial_req->addTransition(this, &Client::imageUploadFailed, upload_error);
    connect(initial_req, &QState::entered,
            [&]()
            {
                qDebug() << "Entered Initial Upload State";
                QByteArrayView fwView(firmware);
                QByteArray sha = QCryptographicHash::hash(fwView, QCryptographicHash::Sha256);
                smp::ImageUploadReq req(image, 0, firmware.first(chunkSize), sha, firmware.length());
                offset = chunkSize;
                chunkPercentage = (100.0/firmware.length())*chunkSize;
                                
                auto confirm = send(req);

                if (!confirm.first)
                {
                    qWarning() << "Error when initiating firmware upload";
                    emit imageUploadFailed();
                }
                else
                {
                    emit imageChunkSent();
                }
                chunksSent = 1;
                qDebug() << "Percentage uploaded " << QString::number(chunkPercentage*chunksSent, 'f', 1);
                totalProgress = chunkPercentage * chunksSent;
                emit firmwareUpdateProgressing();
            }      
    );
    uploadFw.addState(initial_req);

    
    // await_resp state
    awaitRespTimer.setInterval(500);
    
    await_resp->addTransition(this, &Client::imageChunkWritten, send_chunk);
    await_resp->addTransition(this, &Client::lastImageChunkWritten, upload_completed);
    await_resp->addTransition(this, &Client::imageUploadFailed, upload_error);
    await_resp->addTransition(&awaitRespTimer, &QTimer::timeout, upload_error);

    connect(await_resp, &QState::entered,
            [&]()
            {
                qDebug() << "Entering await_resp";
                awaitRespTimer.start(); 
            }
    );    
    
    connect(await_resp, &QState::exited,
            [&]()
            {
                qDebug() << "Leaving await_resp";
                awaitRespTimer.stop(); 
            }
    );
    uploadFw.addState(await_resp);

    // send_chunck
    send_chunk->addTransition(this, &Client::imageChunkSent, await_resp);
    send_chunk->addTransition(this, &Client::imageUploadFailed, upload_error);
    connect(send_chunk, &QState::entered,
            [&]()
            {
                qDebug() << "Entered Send Chunck State";
                QByteArray chunk;
                uint32_t stepSize = chunkSize;
                if (chunkSize > (firmware.length()-offset))
                {
                    qDebug() << "Reached the last chunck";
                    stepSize = firmware.length()-offset;
                }
                chunk = firmware.sliced(offset,stepSize);
                
                smp::ImageUploadReq req(image, offset, chunk);
                offset += stepSize;
                
                auto confirm = send(req);

                if (!confirm.first)
                {
                    qWarning() << "Error when sending firmware upload";
                    emit imageUploadFailed();
                }
                else
                {
                    qDebug() << "Chunk was sent";
                    emit imageChunkSent();
                }
                
                chunksSent++;
                qDebug() << "Percentage uploaded " << QString::number(chunkPercentage*chunksSent, 'f', 1);
                totalProgress = chunkPercentage * chunksSent;
                emit firmwareUpdateProgressing();
            }      
    );
    uploadFw.addState(send_chunk);

    // upload error
    connect(upload_error, &QState::entered,
            [&]()
            {
                qWarning() << "Firmware upload to " << host << " could not be completed due to error";
                firmwareUpdateSuccess = false;
                emit firmwareUpdateFailed();
            }
    );    
    
    uploadFw.addState(upload_error);


    // Upload completed
    connect(upload_completed, &QState::entered,
            [&]()
            {
                qInfo() << "Firmware upload to " << host << " completed";
                smp::GetStateOfImagesReq req;
                auto confirm = send(req);

                if (!confirm.first)
                {
                    qWarning() << "Error when sending GetStateOfImagesReq";
                    emit imageUploadFailed();
                }
                awaitRespTimer.start(); 
            }
    );

    connect(upload_completed, &QState::exited,
            [&]()
            {
                qDebug() << "Leaving upload_completed";
                awaitRespTimer.stop(); 
            }
    );
        
    upload_completed->addTransition(this, &Client::imageUploadFailed, upload_error);
    upload_completed->addTransition(this, &Client::receivedImageHash, mark_upld_permanent);
    upload_completed->addTransition(&awaitRespTimer, &QTimer::timeout, upload_error);
    uploadFw.addState(upload_completed);


    // mark upload permanent
    connect(mark_upld_permanent, &QState::entered,
            [&]()
            {
                qInfo() << "Marking firmware as permanent";
                smp::SetStateOfImagesReq req(true, imageHash);
                auto confirm = send(req);

                if (!confirm.first)
                {
                    qWarning() << "Error when sending SetStateOfImagesReq";
                    emit imageUploadFailed();
                }
                awaitRespTimer.start(); 
            }
    );

    connect(mark_upld_permanent, &QState::exited,
            [&]()
            {
                qDebug() << "Leaving mark_upld_permanent";
                awaitRespTimer.stop(); 
            }
    );
    mark_upld_permanent->addTransition(this, &Client::receivedSetStateOfImagesResp, rebooting_zero);
    mark_upld_permanent->addTransition(this, &Client::imageUploadFailed, upload_error);
    mark_upld_permanent->addTransition(&awaitRespTimer, &QTimer::timeout, upload_error);
    uploadFw.addState(mark_upld_permanent);


    // Rebooting Zero
    connect(this, &Client::receivedResetResp,
            [&](std::shared_ptr<ResetResp> reply)
            {
                if (reply->rc() != 0)
                    emit imageUploadFailed();
                else
                    emit zeroRebooting();                
            }
    );

    connect(rebooting_zero, &QState::entered,
            [&]()
            {
                qInfo() << "Sending reboot command";
                smp::ResetReq req;
                auto confirm = send(req);

                if (!confirm.first)
                {
                    qWarning() << "Error when sending ResetReq";
                    emit imageUploadFailed();
                }
                awaitRespTimer.start(); 
            }
    );
    connect(rebooting_zero, &QState::exited,
            [&]()
            {
                qDebug() << "Leaving rebooting_zero";
                awaitRespTimer.stop(); 
            }
    );
    rebooting_zero->addTransition(this, &Client::zeroRebooting, fw_update_completed);
    rebooting_zero->addTransition(this, &Client::imageUploadFailed, upload_error);
    rebooting_zero->addTransition(&awaitRespTimer, &QTimer::timeout, upload_error);
    uploadFw.addState(rebooting_zero);


    // Firmware update completed
    connect(fw_update_completed, &QState::entered,
            [&]()
            {
                qInfo() << "Firmware update complete";
                firmwareUpdateSuccess = true;
                emit firmwareUpdateCompleted();
            }
    );
    uploadFw.addState(fw_update_completed);
    
    uploadFw.setInitialState(initial_req);      
}

bool Client::connectToHost()
{
    if (isConnected()) disconnectFromHost();

    socket = std::make_unique<QUdpSocket>(this);
    QObject::connect(socket.get(), &QUdpSocket::readyRead, this, &Client::onReadyRead);
    QObject::connect(socket.get(), &QUdpSocket::connected, this, &Client::connectionEstablished);

    if (!socket->bind()) {
        qWarning() << "Client: Cannot bind to port";
        return false;
    }

    socket->connectToHost(host, port);
    return true;
}

void Client::onReadyRead()
{
    auto datagram = socket->receiveDatagram();
    auto data = datagram.data();

    Header hdr(data);

    qDebug() << "ID: " << Qt::bin << MessageTypes::ImageUploadRespType;
    switch (hdr.msgType())
    {
        case MessageTypes::GetStateOfImagesRespType:
        {
            auto ptr = std::make_shared<smp::GetStateOfImagesResp>(hdr, data);
            emit receivedGetStateOfImagesResp(ptr);
            break;
        }
        case MessageTypes::SetStateOfImagesRespType:
        {
            auto ptr = std::make_shared<smp::SetStateOfImagesResp>(hdr, data);
            emit receivedSetStateOfImagesResp(ptr);
            break;
        }
        case MessageTypes::ImageUploadRespType:
        {
            auto ptr = std::make_shared<smp::ImageUploadResp>(hdr, data);
            emit receivedImageUploadResp(ptr);
            break;            
        }
        case MessageTypes::ResetRespType:
        {
            auto ptr = std::make_shared<smp::ResetResp>(hdr, data);
            emit receivedResetResp(ptr);
            break;            
        }
        default:
        {
            qWarning() << "Received an unknown Smp response of id " << Qt::bin << hdr.msgType();            
        }
    }
} 

} // end of namespace
