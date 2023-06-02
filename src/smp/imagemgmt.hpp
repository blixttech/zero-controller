#pragma once

#include "smp.hpp"
#include "header.hpp"
#include "request.hpp"
#include "response.hpp"

namespace smp {

/**
 * Command IDs for image management group.
 */

enum ImgMgmtId
{
    ID_STATE    = 0,
    ID_UPLOAD   = 1,
    ID_FILE     = 2,
    ID_CORELIST = 3,
    ID_CORELOAD = 4,
    ID_ERASE    = 5
};


/**
 * GetStateOfImages Request / Response
 *
 */

struct ImageSlot
{
    int slot;
    QString version;
    QByteArray hash;
    bool bootable;
    bool pending;
    bool confirmed;
    bool active;
    bool permanent;
};
  
class GetStateOfImagesReq : public Request
{
public:
    GetStateOfImagesReq();

    void serialize(QByteArray& writer) override;
};


class GetStateOfImagesResp : public Response
{
public:
    GetStateOfImagesResp(const Header& header, const QByteArray& payload);

    const std::vector<ImageSlot>& images() const { return imagesVec; }

    virtual bool deserialize(const QByteArray& payload);

private:
    std::vector<ImageSlot> imagesVec;
};


class SetStateOfImagesReq : public Request
{
public:
    SetStateOfImagesReq(bool confirm, const QByteArray& hash);

    void serialize(QByteArray& writer) override;

private:
    bool confirm;
    QByteArray hash;
    
};

class SetStateOfImagesResp : public Response
{
public:
  SetStateOfImagesResp(const Header& header, const QByteArray& payload);
  
  const std::vector<ImageSlot>& images() const { return imagesVec; }

  virtual bool deserialize(const QByteArray& payload);

private:
  std::vector<ImageSlot> imagesVec;
};

/**
 * Image Impload Request / Response 
 */

class ImageUploadReq : public Request
{
public:
    ImageUploadReq(uint8_t image, uint32_t off, const QByteArray& data, 
                    const QByteArray& sha = QByteArray(), uint32_t length = 0, bool upgrade = false);

    void serialize(QByteArray& writer) override;

private:
    uint8_t image;
    uint32_t off;
    uint32_t len;
    QByteArray sha;
    QByteArray data;
    bool upgrade;
};

class ImageUploadResp : public Response
{
public:
    ImageUploadResp(const Header& header, const QByteArray& payload);  

    virtual bool deserialize(const QByteArray& payload);

    int32_t off() { return _off; }
    bool match() { return _match; }

    int32_t rc() { return _rc; }
    QString rsn() { return _rsn; }

private:
    int32_t _off;
    bool _match;

    int32_t _rc;
    QString _rsn;
        
};

} // end of namespace

