/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/gpsm_dcm.h $                                          */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2014,2016                        */
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
#ifndef __GPSM_DCM_H__
#define __GPSM_DCM_H__

/// \file gpsm_dcm.h
/// \brief PgP Global PState Machine (Mechanism) in Dual Chip Model

#include "pmc_dcm.h"

/// GPSM-DCM Return Code
#define GPSM_DCM_SUCCESS             0
#define GPSM_DCM_ARG_INVALID_OBJ_SND 0x00326401 //ssx panic
#define GPSM_DCM_ARG_INVALID_OBJ_RCV 0x00326402 //ssx panic
#define GPSM_DCM_ARG_INVALID_OBJ_WRT 0x00326403 //ssx panic
#define GPSM_DCM_ARG_INVALID_OBJ_RED 0x00326404 //ssx panic
#define GPSM_DCM_PKT_INVALID_CMD_SND 0x00326405 //ssx panic
#define GPSM_DCM_PKT_INVALID_CMD_RCV 0x00326406 //ssx panic
#define GPSM_DCM_CMD_NOT_FROM_MASTER 0x00326407 //ssx panic
#define GPSM_DCM_CMD_SHOULD_TO_SLAVE 0x00326408 //ssx panic
#define GPSM_DCM_SEND_PACKET_TIMEOUT 0x00326409 //user handle
#define GPSM_DCM_RECV_PACKET_TIMEOUT 0x0032640a //user handle
#define GPSM_DCM_READ_RECV_NOT_WRITE 0x0032640b //ssx panic
#define GPSM_DCM_READ_NOT_WRITE_DATA 0x0032640c //ssx panic
#define GPSM_DCM_DAT_BIGGER_THAN_BUF 0x0032640d //user handle
#define GPSM_DCM_CMD_REPLY_NOT_INT   0x0032640e //ssx panic
#define GPSM_DCM_SLAVE_TIMEOUT       0x0032640f //user handle
#define GPSM_DCM_SLAVE_ERROR         0x00326410 //user handle

/// GPSM Interchip Command Code
#define GPSM_IC_DATA           0 //0b0000
#define GPSM_IC_WRITE          1 //0b0001
#define GPSM_IC_ENABLE_PSTATES 2 //0b0010
#define GPSM_IC_HW_PSTATE_MODE 3 //0b0011

#define GPSM_DCM_NUMBER_OF_COMMANDS  4

/// This is a special command return code returned by gpsm_dcm_slave() when it
/// times out.
#define GPSM_IC_NO_COMMAND GPSM_DCM_NUMBER_OF_COMMANDS


/// Timeout Parameter
#define GPSM_DCM_DEFAULT_TIMEOUT SSX_MICROSECONDS(15)

#ifndef __ASSEMBLER__

/// GPSM-DCM abstract packet

typedef struct {
    /// Firmware command
    uint8_t command : 4;
    union {
        /// Used for Pstate-based protocols
        Pstate pstate[2];
        /// Generic byte data
        uint8_t u8[2];
    } payload;
} GpsmDcmPacket;

/// Data Structure for Fast Write/Read Handlers

typedef struct {
  void*        buffer_pointer;
  uint32_t     remaining_size;
  SsxSemaphore fast_semaphore;
} GpsmDcmFastData;

/// Data Structure for Sync Mehotds


/// Abstract type of gpsm_dcm_slave() callbacks
///
/// The callback receives the (first) master packet of the exchange.  The
/// return code is passed back to the master.

typedef int (*GpsmDcmSlaveCallback)(GpsmDcmPacket* fwPacket);


/// Control structure for gpsm_dcm_slave()

typedef struct {
    /// Slave timeout when waiting for next packet to arrive in long commands.
    SsxInterval timeout;

    /// Callback called after "Enable Pstates" command
    GpsmDcmSlaveCallback gpsm_dcm_callback_enable_pstates;

    /// Callback called after "Enter HW Pstate Mode" command
    GpsmDcmSlaveCallback gpsm_dcm_callback_hw_pstate_mode;

    /// Callback for GPSM-DCM write command
    ///
    /// Will be called with the application-supplied buffer and the actual
    /// size of the data transmission.
    int (*write_handler)(void* buffer, uint16_t size, void* arg);

    /// Application-supplied buffer for GPSM-DCM write commands
    void* buffer;

    /// Size of the application-supplied write buffer
    uint16_t buffer_size;

    /// Application-supplied generic argument to the write handler
    void* write_arg;

    /// Callback when slave detects timeout from master
    GpsmDcmSlaveCallback timeout_handler;

    /// The last command recieved by the slave.
    int command;

    /// Return code from gpsm_dcm protocol actions
    int protocol_rc;

    /// Return code from slave action in response to master command
    int slave_rc;

    /// Return code from the application specific callback, if any.
    int callback_rc;

} GpsmSlaveControl;

////////////////////////////////////////////////////////////////////////////
// High-level GPSM-DCM Interchip Communication Methods
////////////////////////////////////////////////////////////////////////////

int
gpsm_dcm_send(GpsmDcmPacket* fwPacket);



int
gpsm_dcm_receive(GpsmDcmPacket* fwPacket,
                 SsxInterval timeout_period);


////////////////////////////////////////////////////////////////////////////
// Generic Data Communication using GPSM-DCM 'write' command
////////////////////////////////////////////////////////////////////////////

int
gpsm_dcm_write(void* buf,
               uint16_t size,
               SsxInterval timeout_period);


int
gpsm_dcm_read(void* buf,
              uint16_t buf_size,
              uint16_t* data_size,
              SsxInterval timeout_period);

////////////////////////////////////////////////////////////////////////////
// Fast Interrrupt Handlers for Data Transfer of GPSM-DCM 'write' command
////////////////////////////////////////////////////////////////////////////

SSX_IRQ_HANDLER(gpsm_dcm_fast_write);
SSX_IRQ_HANDLER(gpsm_dcm_fast_read);

////////////////////////////////////////////////////////////////////////////
// Generic Synchronization Mechanism using GPSM-DCM 'write' command
////////////////////////////////////////////////////////////////////////////

int
gpsm_dcm_sync(int state);

////////////////////////////////////////////////////////////////////////////
// The Control Method for Slave Thread
////////////////////////////////////////////////////////////////////////////

int
gpsm_dcm_slave(GpsmSlaveControl *control,
               SsxInterval wait);


////////////////////////////////////////////////////////////////////////////
// The Command Method for Master Thread
////////////////////////////////////////////////////////////////////////////

int
gpsm_dcm_master(GpsmDcmPacket* fwPacket, int* slaveRc);

#endif  /* __ASSEMBLER__ */

#endif  /* __GPSM_DCM_H__ */
