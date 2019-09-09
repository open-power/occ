/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe0/firdata/ipmidd.H $                               */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2018,2019                        */
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
#ifndef __IPMI_IPMIDD_H
#define __IPMI_IMPIDD_H

#include <stdint.h>

/** @file ipmidd.h
 *  @brief Provides the interfaces to the IPMI Device Driver
 */

namespace IPMI
{
    enum network_function
    {
        NETFUN_IBM  = (0x3a << 2),
        PNOR_CMD = 0x5a,
    };

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
};

enum {
    // Registers. These are fixed for LPC/BT so we can hard-wire them
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

    // Ignore CTRL_SMS_ATN
    IDLE_STATE = (CTRL_B_BUSY | CTRL_B2H_ATN | CTRL_H2B_ATN),

    // Bit in the INMASK register which signals to the BMC
    // to reset it's end of things.
    INT_BMC_HWRST      = (1 << 7),

    // How long to sychronously wait for the device to change state
    // (in micro seconds)
    WAIT_TIME_US = 100000,
    MAX_PACKET_DATA_SIZE = 16,
    RC_IPMIDD_IDLE = 10,
    RC_IPMIDD_NOT_IDLE = 11,
    RC_IPMIDD_TIMEOUT = 12,
    RC_BAD_SEQUENCE = 13,
    RC_IPMIDD_INVALID_RESP_SIZE = 14,
    RC_IPMI_EVENT = 15,
    RC_IPMI_BUSY = 16,
};

/**
 *  @brief IPMI Device Driver Class
 *     Provides read/write message capabilities.
 */
class IpmiDD
{
    public: // data

        uint8_t iv_netfun;
        uint8_t iv_seq;
        uint8_t iv_cmd;
        uint8_t iv_cc;
        uint8_t iv_data[MAX_PACKET_DATA_SIZE];
        uint8_t iv_data_len;

  public: // intefaces
    /**
     * @brief Poll the control register
     *
     * @parm void
     */
    int pollCtrl(void);

    /**
     * @brief Performs an IPMI message read operation
     *
     * @param[out] o_msg - Destination buffer for data
     *
     * @return SUCCESS | error code
     */

    int receive(void);
    /**
     * @brief Performs an IPMI message write operation
     *
     * @param[in] i_msg - Location of data to be written
     *
     * @return SUCCESS | error code
     */
    int send(void);

    /**
     * @brief Performs a reset of the BT hardware
     *
     * @param void
     *
     * @return SUCCESS | error code
     */
    int  reset(void);

    /**
     * @brief Constructor
     *
     * @parm void
     */
    IpmiDD(uint8_t i_netfun,
           uint8_t i_cmd,
           uint8_t i_seq,
           uint8_t i_cc,
           uint8_t * i_data,
           uint8_t i_data_len);

  public:

  private:

    /**
     * @brief Read an address from LPC space
     *
     * @parm i_addr  Absolute LPC Address
     * @parm o_data  Buffer to read data into
     *
     * @return Error from operation
     */
    int readLPC(const uint32_t i_addr, uint8_t& o_data);

    /**
     * @brief Write an address from LPC space
     *
     * @parm i_addr  Absolute LPC Address
     * @parm i_data  Data to write
     *
     * @return Error from operation
     */
    int writeLPC(const uint32_t i_addr, uint8_t i_data);

  private: // Variables

    // Disallow copying this class.
    IpmiDD& operator=(const IpmiDD&);
    IpmiDD(const IpmiDD&);
};

#endif
