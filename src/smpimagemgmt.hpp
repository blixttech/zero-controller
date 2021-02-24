#pragma once

#include "smp.hpp"
#include "smprequest.hpp"
#include "smpreply.hpp"
#include "smppayload.hpp"

namespace smp {


#define IMG_MGMT_HASH_STR         48
#define IMG_MGMT_HASH_LEN         32
#define IMG_MGMT_DATA_SHA_LEN     32 /* SHA256 */

#define IMG_MGMT_VER_MAX_STR_LEN    25  /* 255.255.65535.4294967295\0 */

/*
 * Swap Types for image management state machine
 */
#define IMG_MGMT_SWAP_TYPE_NONE     0
#define IMG_MGMT_SWAP_TYPE_TEST     1
#define IMG_MGMT_SWAP_TYPE_PERM     2
#define IMG_MGMT_SWAP_TYPE_REVERT   3

/**
 * Command IDs for image management group.
 */

enum ImgMgmtId
{
    ID_STATE = 0,
    ID_UPLOAD = 1,
    ID_FILE = 2,
    ID_CORELIST = 3,
    ID_CORELOAD = 4,
    ID_ERASE = 5
};

/*
 * IMG_MGMT_ID_UPLOAD statuses.
 */
#define IMG_MGMT_ID_UPLOAD_STATUS_START         0
#define IMG_MGMT_ID_UPLOAD_STATUS_ONGOING       1
#define IMG_MGMT_ID_UPLOAD_STATUS_COMPLETE      2

class ImageMgmtStateReq : public SmpRequest
{
    public:
    ImageMgmtStateReq();

    void serialize(QByteArray& writer) override;
};

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

struct ImageMgmtStatePayload : public SmpPayload
{
    bool deserialize(QByteArray& reader) override;
    std::vector<ImageSlot> fwSlots;
};



} // end of namespace

