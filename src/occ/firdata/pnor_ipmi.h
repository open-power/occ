#if !defined(__PNOR_IPMI_H)
#define __PNOR_IPMI_H

#include <native.h>

enum network_function
{
    NETFUN_IBM = (0x3a << 2),
    PNOR_CMD = 0x5a,
};

// IPMI interface completion codes
enum completion_code
{
    CC_OK        = 0x00,
    CC_CMDSPC1   = 0x80, // command specific completion code
    CC_CMDSPC2   = 0x81, // command specific completion code
    CC_BUSY      = 0xc0,
    CC_INVALID   = 0xc1,
    CC_CMDLUN    = 0xc2,
    CC_TIMEOUT   = 0xc3,
    CC_NOSPACE   = 0xc4,
    CC_BADRESV   = 0xc5,
    CC_TRUNC     = 0xc6,
    CC_BADLEN    = 0xc7,
    CC_TOOLONG   = 0xc8,
    CC_OORANGE   = 0xc9,
    CC_LONGREPLY = 0xca,
    CC_BADSENSOR = 0xcb,
    CC_REQINVAL  = 0xcc,
    CC_CMDSENSOR = 0xcd,
    CC_CANTREPLY = 0xce,
    CC_DUPREQ    = 0xcf,
    CC_SDRUPDATE = 0xd0,
    CC_FMWUPDATE = 0xd1,
    CC_BMCINIT   = 0xd2,
    CC_BADDEST   = 0xd3,
    CC_NOPERM    = 0xd4,
    CC_BADSTATE  = 0xd5,
    CC_ILLPARAM  = 0xd6,
    CC_UNKBAD    = 0xff
};

enum
{
    HIOMAP_C_RESET_STATE            = 0x01,
    HIOMAP_C_GET_INFO               = 0x02,
    HIOMAP_C_GET_FLASH_INFO         = 0x03,
    HIOMAP_C_CREATE_READ_WINDOW     = 0x04,
    HIOMAP_C_CLOSE_WINDOW           = 0x05,
    HIOMAP_C_CREATE_WRITE_WINDOW    = 0x06,
    HIOMAP_C_MARK_WRITE_DIRTY       = 0x07,
    HIOMAP_C_WRITE_FLUSH            = 0x08,
    HIOMAP_C_BMC_EVENT_ACK          = 0x09,
    HIOMAP_C_MARK_WRITE_ERASED      = 0x0a,

    // Responses
    HIOMAP_R_SUCCESS                = 0x01,
    HIOMAP_R_PARAM_ERROR            = 0x02,
    HIOMAP_R_WRITE_ERROR            = 0x03,
    HIOMAP_R_SYSTEM_ERROR           = 0x04,
    HIOMAP_R_TIMEOUT                = 0x05,
    HIOMAP_R_BUSY                   = 0x06,
    HIOMAP_R_WINDOW_ERROR           = 0x07,




    HIOMAP_MAX_ARG_SIZE = 11,
    MAX_PACKET_DATA_SIZE = HIOMAP_MAX_ARG_SIZE+2,
    IPMI_MAX_TRIES = 10000,

    // return codes. Reserve 1-9 for SCOM errors
    RC_IPMIDD_IDLE = 10,
    RC_IPMIDD_NOT_IDLE = 11,
    RC_IPMIDD_TIMEOUT = 12,
    RC_BAD_SEQUENCE = 13,
    RC_IPMIDD_INVALID_RESP_SIZE = 14,
    RC_IPMI_EVENT = 15,
    RC_IPMI_BUSY = 16,

    // Registers. These are fixed for LPC/BT
    REG_CONTROL = 0xE4,
    REG_HOSTBMC = 0xE5,
    REG_INTMASK = 0xE6,

    // Control register bits. The control register is interesting in that
    // writing 0's never does anything; all registers are either set to 1
    // when written with a 1 or toggled (1/0) when written with a one. So,
    // we don't ever need to read-modify-write, we can just write an or'd
    // mask of bits.
    CTRL_B_BUSY        = (1 << 7),
    CTRL_H_BUSY        = (1 << 6),
    CTRL_OEM0          = (1 << 5),
    CTRL_SMS_ATN       = (1 << 4),
    CTRL_B2H_ATN       = (1 << 3),
    CTRL_H2B_ATN       = (1 << 2),
    CTRL_CLR_RD_PTR    = (1 << 1),
    CTRL_CLR_WR_PTR    = (1 << 0),
    IDLE_STATE = (CTRL_B_BUSY | CTRL_B2H_ATN |
                  CTRL_SMS_ATN | CTRL_H2B_ATN),
    // Bit in the INMASK register which signals to the BMC
    // to reset it's end of things.
    INT_BMC_HWRST      = (1 << 7),

};

typedef struct
{
    bool     iv_useIPMI;
    uint32_t iv_protocolVersion;
    uint32_t iv_blockShift;
    uint32_t iv_flashSize;
    uint32_t iv_flashEraseSize;
    bool     iv_curWindowOpen;      //Currently opn
    bool     iv_curWindowWrite;     // Write vs Read window
    uint32_t iv_curWindowOffset;    // Offset into flash
    uint32_t iv_curWindowSize;      // Size
    uint32_t iv_curWindowLpcOffset; // Offset into LPC FW space
    // Legacy V1 protocol
    uint32_t iv_readWindowSize;
    uint32_t iv_writeWindowSize;
} hioPnorInfo_t;


typedef struct
{
    uint8_t iv_cmd;
    uint8_t iv_seq;
    uint8_t iv_args[HIOMAP_MAX_ARG_SIZE];
    uint8_t iv_resp;
} hio_msg_t;

typedef struct
{
    uint8_t iv_netfun;
    uint8_t iv_seq;
    uint8_t iv_cmd;
    uint8_t iv_cc;
    hio_msg_t iv_hio_msg;
    uint8_t iv_hio_msg_len; // len of valid part of hio_msg
} ipmi_msg_t;


static inline void put8(hio_msg_t * i_msg, int i_index, uint8_t i_value)
{
    if(i_index >= HIOMAP_MAX_ARG_SIZE) return;
    i_msg->iv_args[i_index] = i_value;
}

static inline uint8_t get8(hio_msg_t * i_msg, int i_index)
{
    if(i_index >= HIOMAP_MAX_ARG_SIZE) return 0;
    return i_msg->iv_args[i_index];
}


static inline void put16(hio_msg_t * i_msg, int i_index, uint32_t i_value)
{
    if(i_index >= (HIOMAP_MAX_ARG_SIZE-1)) return;
    i_msg->iv_args[i_index] = (uint8_t)(i_value & 0xff);
    i_msg->iv_args[i_index + 1] = (uint8_t)(i_value >> 8);
}

static inline uint16_t get16(hio_msg_t * i_msg, int i_index)
{
    if(i_index >= (HIOMAP_MAX_ARG_SIZE-1)) return 0;
    return i_msg->iv_args[i_index] | (i_msg->iv_args[i_index + 1] << 8);
}

static inline void put32(hio_msg_t * i_msg, int i_index, uint32_t i_value)
{
    if(i_index >= (HIOMAP_MAX_ARG_SIZE-3)) return;
    i_msg->iv_args[i_index] = i_value & 0xff;
    i_msg->iv_args[i_index + 1] = (i_value >> 8) & 0xff;
    i_msg->iv_args[i_index + 2] = (i_value >> 16) & 0xff;
    i_msg->iv_args[i_index + 3 ] = i_value >> 24;
}

static inline uint32_t get32(hio_msg_t * i_msg, int i_index)
{
    if(i_index >= (HIOMAP_MAX_ARG_SIZE-3)) return 0;
    return i_msg->iv_args[i_index] |
        (i_msg->iv_args[i_index + 1] << 8) |
        (i_msg->iv_args[i_index + 2] << 16) |
        (i_msg->iv_args[i_index + 3] << 24);
}

errorHndl_t getInfo(hioPnorInfo_t * o_pnorInfo);

errorHndl_t ipmiWriteFlash(hioPnorInfo_t * i_hioInfo,
                       uint32_t i_addr,
                       uint32_t i_size,
                       void* i_data);

#endif
