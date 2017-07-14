/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/firdata/ast_mboxdd.h $                            */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2017                             */
/* [+] International Business Machines Corp.                              */
/*                                                                        */
/*                                                                        */
/* Licensed under the Apache License, Version 2.0 (the "License");        */
/* you may not use this file except in compliance with the License.       */
/* You may obtain a copy of the License at                                */
/*                                                                        */
/*     http://www.apache.org/licenses/LICENSE-2.0                         */
/*                                                                        */
/* Unless required by applicable law or agreed to in writing, software    */
/* distributed under the License is distributed on an "AS IS" BASIS,      */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or        */
/* implied. See the License for the specific language governing           */
/* permissions and limitations under the License.                         */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */

#ifndef __AST_MBOXDD_H
#define __AST_MBOXDD_H

/** @file ast_mboxdd.H
 *  @brief Provides the interfaces Aspeed MBOX hardware
 */

/**
 *  @brief  Provides the interface to exchange Mbox
 *          messages with the BMC.
 */

enum
{
    BMC_MBOX_DATA_REGS              = 14,
    BMC_MBOX_ARGS_REGS              = 11,

    /* Commands */
    MBOX_C_RESET_STATE              = 0x01,
    MBOX_C_GET_MBOX_INFO            = 0x02,
    MBOX_C_GET_FLASH_INFO           = 0x03,
    MBOX_C_CREATE_READ_WINDOW       = 0x04,
    MBOX_C_CLOSE_WINDOW             = 0x05,
    MBOX_C_CREATE_WRITE_WINDOW      = 0x06,
    MBOX_C_MARK_WRITE_DIRTY         = 0x07,
    MBOX_C_WRITE_FLUSH              = 0x08,
    MBOX_C_BMC_EVENT_ACK            = 0x09,
    MBOX_C_MARK_WRITE_ERASED        = 0x0a,

    /* Responses */
    MBOX_R_SUCCESS                  = 0x01,
    MBOX_R_PARAM_ERROR              = 0x02,
    MBOX_R_WRITE_ERROR              = 0x03,
    MBOX_R_SYSTEM_ERROR             = 0x04,
    MBOX_R_TIMEOUT                  = 0x05,
    MBOX_R_BUSY                     = 0x06,
    MBOX_R_WINDOW_ERROR             = 0x07,

};

enum{
        SIO_DEVICE_SELECT_REG   = 0x07,   /**< Regsiter to select SIO device */
        SIO_ADDR_REG_2E         = 0x2E,   /**< SuperIO address register */
        SIO_DATA_REG_2F         = 0x2F,   /**< SuperIO data register */
        SIO_PASSWORD_REG        = 0xA5,   /**< SuperIO password register (to be unlocked to access SIO) */
        SIO_MB                  = 0x0E,   /**< SIO device: Mailbox */
        SIO_ENABLE_DEVICE       = 0x01,   /**< Enable SIO device by writing 1 to reg 30 of device */
        SIO_DISABLE_DEVICE      = 0x00,   /**< Disable SIO device by writing 0 to reg 30 of device */
};


typedef struct
{
    uint8_t iv_cmd;
    uint8_t iv_seq;
    uint8_t iv_args[BMC_MBOX_ARGS_REGS];
    uint8_t iv_resp;

} mboxMessage_t;

/**
 * @brief Write a single byte into an SIO register
 *
 * @param[in] i_reg: Register to write
 * @param[in] i_data: Data to write
 *
 * @return Error from operation
 */
errorHndl_t writeRegSIO(uint8_t i_regAddr,
                        uint8_t i_data );

/**
 * @brief Read a single byte from an SIO register
 *
 * @param[in] i_reg: Register to read
 * @param[in] o_data: Data that was read
 *
 * @return Error from operation
 */
errorHndl_t readRegSIO(uint8_t i_regAddr,
                       uint8_t* o_data );


//Helper functions to create mbox messages to send to the BMC

/**
 * @brief 8-bit read accessor of the args section
 *
 * @param[in] i_msg:   mboxMessage_t pointer to access args section
 * @param[in] i_index: Index into args section to be read from
 *
 * @return data requested
 */
static inline uint8_t get8( mboxMessage_t *i_msg, uint8_t i_index)
{
    if ( i_index >= BMC_MBOX_ARGS_REGS )
    {
        return 0;
    }

    return i_msg->iv_args[i_index];
}

/**
 * @brief 8-bit write accessor of the args section
 *
 * @param[in] i_msg:   mboxMessage_t pointer to access args section
 * @param[in] i_index: Index into args section to be written to
 * @param[in] i_value: data to be written
 */
static inline void put8(mboxMessage_t *i_msg, uint8_t i_index, uint8_t i_value)
{
    if ( i_index >= BMC_MBOX_ARGS_REGS )
    {
        return;
    }

    i_msg->iv_args[i_index] = i_value;
}

/**
 * @brief 16-bit read accessor of the args section
 * @param[in] i_msg:   mboxMessage_t pointer to access args section
 * @param[in] i_index: Index into args section to be read from
 *
 * @return data requested
 */
static inline uint16_t get16( mboxMessage_t *i_msg, uint8_t i_index )
{
    if ( i_index >= (BMC_MBOX_ARGS_REGS-1) )
    {
        return 0;
    }

    return i_msg->iv_args[i_index] | (i_msg->iv_args[i_index + 1] << 8);
}

/**
 * @brief 16-bit write accessor of the args section
 *
 * @param[in] i_msg:   mboxMessage_t pointer to access args section
 * @param[in] i_index: Index into args section to be written to
 * @param[in] i_value: data to be written
 */
static inline void put16(mboxMessage_t *i_msg,
                         uint8_t i_index,
                         uint16_t i_value)
{
    if ( i_index >= (BMC_MBOX_ARGS_REGS-1) )
    {
        return;
    }

    i_msg->iv_args[i_index] = i_value & 0xff;
    i_msg->iv_args[i_index + 1] = i_value >> 8;
}

/**
 * @brief 32-bit read accessor of the args section
 *
 * @param[in] i_msg:   mboxMessage_t pointer to access args section
 * @param[in] i_index: Index into args section to be read from
 *
 * @return data requested
 */
static inline uint32_t get32( mboxMessage_t *i_msg, uint8_t i_index )
{
    if ( i_index >= (BMC_MBOX_ARGS_REGS-3) )
    {
        return 0;
    }

    return i_msg->iv_args[i_index] |
        (i_msg->iv_args[i_index + 1] << 8) |
        (i_msg->iv_args[i_index + 2] << 16) |
        (i_msg->iv_args[i_index + 3] << 24);
}

/**
 * @brief 32-bit write accessor of the args section
 *
 * @param[in] i_msg:   mboxMessage_t pointer to access args section
 * @param[in] i_index: Index into args section to be read from
 */
static inline void put32(mboxMessage_t *i_msg,
                         uint8_t i_index,
                         uint32_t i_value)
{
    if ( i_index >= (BMC_MBOX_ARGS_REGS-3) )
    {
        return;
    }

    i_msg->iv_args[i_index] = i_value & 0xff;
    i_msg->iv_args[i_index + 1] = (i_value >> 8) & 0xff;
    i_msg->iv_args[i_index + 2] = (i_value >> 16) & 0xff;
    i_msg->iv_args[i_index + 3 ] = i_value >> 24;
}

typedef struct {

    uint8_t iv_mboxMsgSeq;
    mboxMessage_t iv_msg;

} astMbox_t;

/**
 *  @brief Send a message and receive the response
 *
 *  @parm[in/out] io_mbox Mbox pointer to send message
 *  @parm[in/out] io_msg  Message to send, contains the response on exit
 *
 *  @return Error from operation
 */
errorHndl_t doMessage( astMbox_t *io_mbox, mboxMessage_t *io_msg );

enum
{
    MBOX_FLAG_REG               = 0x0f,
    MBOX_STATUS_0               = 0x10,
    MBOX_STATUS_1               = 0x11,
    MBOX_STATUS1_ATTN           = 0x80,
    MBOX_STATUS1_RESP           = 0x20,
    MBOX_BMC_CTRL               = 0x12,
    MBOX_CTRL_INT_STATUS        = 0x80,
    MBOX_CTRL_INT_MASK          = 0x02,
    MBOX_CTRL_INT_SEND          = 0x01,
    MBOX_HOST_CTRL              = 0x13,
    MBOX_BMC_INT_EN_0           = 0x14,
    MBOX_BMC_INT_EN_1           = 0x15,
    MBOX_HOST_INT_EN_0          = 0x16,
    MBOX_HOST_INT_EN_1          = 0x17,

    MBOX_IO_BASE                = 0x1000,
    MBOX_LPC_IRQ                = 0x9,

    MBOX_MAX_RESP_WAIT_US       = 10000000, /* 10s timeout */
};

/**
 * @brief Initialize/Enable the MBox in the SIO
 *
 * @return Error from operation
 */
errorHndl_t initializeMbox( void );

/**
 * @brief Write a byte to the mBox
 *
 * @parm[in] i_addr: Register offset in the mbox
 * @parm[in] i_byte: Byte to write
 *
 * @return Error from operation
*/
errorHndl_t mboxOut( uint64_t i_addr, uint8_t i_byte );

/**
 * @brief Read a byte from the mBox
 *
 * @parm[in] i_addr: Register offset in the mbox
 * @parm[out] o_byte: Byte read
 *
 * @return Error from operation
 */
errorHndl_t mboxIn( uint64_t i_addr, uint8_t *o_byte );

#endif /* __AST_MBOXDD_H */
