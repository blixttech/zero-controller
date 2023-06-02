#pragma once

namespace smp {


/* MTU for newtmgr responses */
constexpr int MGMT_MAX_MTU = 1024;

/* Mgmt header size */
constexpr int MGMT_HDR_SIZE = 8;

/** Opcodes; encoded in first byte of header. */
enum MgmtOp 
{
    READ = 0,
    READ_RSP = 1,
    WRITE = 2,
    WRITE_RSP = 3
};

/**
 * The first 64 groups are reserved for system level mcumgr commands.
 * Per-user commands are then defined after group 64.
 */
enum MgmtGroupId 
{
    ID_OS = 0,
    ID_IMAGE = 1,
    ID_STAT = 2,
    ID_CONFIG = 3,
    ID_LOG = 4,
    ID_CRASH = 5,
    ID_SPLIT = 6,
    ID_RUN = 7,
    ID_FS = 8,
    ID_SHELL = 9,
    ID_PERUSER = 64
};

/*
 * MGMT event opcodes.
 */
enum MgmtEventOp 
{
    OP_CMD_RECV = 0x01,
    OP_CMD_STATUS = 0x02,
    OP_CMD_DONE = 0x03
};
  
/**
 * mcumgr error codes.
 */
enum MgmtErrorCode 
{
    ERR_EOK = 0,
    ERR_EUNKNOWN = 1,
    ERR_ENOMEM = 2,
    ERR_EINVAL = 3,
    ERR_ETIMEOUT = 4,
    ERR_ENOENT = 5,
    ERR_EBADSTATE = 6,       /* Current state disallows command. */
    ERR_EMSGSIZE = 7,       /* Response too large. */
    ERR_ENOTSUP = 8,       /* Command not supported. */
    ERR_ECORRUPT = 9,       /* Corrupt */
    ERR_EPERUSER = 256
};


} // end of namespace
