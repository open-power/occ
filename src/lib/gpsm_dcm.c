/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/gpsm_dcm.c $                                          */
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
/// \file gpsm_dcm.h
/// \brief PgP Global PState Machine (Mechanism) in Dual Chip Model

#include "ssx.h"
#include "pmc_dcm.h"
#include "gpsm_dcm.h"
#include "gpsm.h"

/// Timeout object and methods

typedef struct {
    SsxTimebase timeout;
} Timeout;

static void
timeout_start(Timeout* t, SsxInterval timeout_period)
{
    t->timeout = ssx_timebase_get() + timeout_period;
}

static int
timeout_timed_out(Timeout* t)
{
    return ssx_timebase_get() > t->timeout;
}


/// Internal API : Send packet with timeout
///
/// \param hwPacket pointer to the packet to be sent
///
/// \param timeout_period The SSX timeout variable
///
/// This API sends \a hwPacket within \a timeout_period
/// if timed out, the function returns an error code 
///
/// \retval GPSM_DCM_SUCCESS
///
/// \retval GPSM_DCM_SEND_PACKET_TIMEOUT
///

static int
_gpsm_dcm_send(PmcDcmPacket* hwPacket,
               SsxInterval timeout_period)
{
    int rc = GPSM_DCM_SUCCESS;
    Timeout  timeout;

    //set timeout
    timeout_start(&timeout, timeout_period);

    //try to send packet within timeout
    while( (rc=pmc_dcm_send(hwPacket)) == PMC_DCM_OUTSTANDING_TRANSFER ) {
        if( timeout_timed_out(&timeout)) {
            rc = GPSM_DCM_SEND_PACKET_TIMEOUT;
            break;
        }
    }

    return rc;
}

/// Internal API : Receive packet with timeout
///
/// \param hwPacket pointer to the packet structure to receive
///
/// \param timeout_period The SSX timeout variable
///
/// This API receives a \a hwPacket within \a timeout
/// if timed out, the function returns an error code 
///
/// \retval GPSM_DCM_SUCCESS
///
/// \retval GPSM_DCM_RECV_PACKET_TIMEOUT
///

static int
_gpsm_dcm_receive(PmcDcmPacket* hwPacket,
                  SsxInterval timeout_period)
{
    int rc = GPSM_DCM_SUCCESS; 
    Timeout  timeout;
    
    //set timeout 
    timeout_start(&timeout, timeout_period);

    //try to receive packet within timeout
    while( (rc=pmc_dcm_receive(hwPacket)) == PMC_DCM_RECEIVE_NOT_DETECTED ) {
        if( timeout_timed_out(&timeout)) {
            rc = GPSM_DCM_RECV_PACKET_TIMEOUT;
            break;
        }
    }

    return rc;
}

////////////////////////////////////////////////////////////////////////////
// High-level GPSM-DCM Interfaces
////////////////////////////////////////////////////////////////////////////

/// Abstract non-blocking send over the DCM interchip link
///
/// The GPSM-DCM abstract interface guarantees that the PMC-DCM link is always
/// clear for use by the master sender.  This API (which should be called from
/// a thread context) transmits an abstract packet, then polls for the hardware
/// ACK - which must always indicate that the packet was received.  This ACK
/// will always occur very quickly.
///
/// \param fwPacket a GpsmDcmPacket structure to be sent as part of the 
/// PmcDcmPacket/hwPacket of MSG type via PMC interchip link. 
/// The fwPacket includes a firmware command and 16 bits payload. 
/// The firmware command will be filled in the cmd_ext field of the 
/// hwPacket structure, and payload 0,1 will be filled in the 
/// corresponding slots also in hwPacket structure.
/// This argument is provided by the caller and passed in as reference.
///
/// hwPacket:
///
/// cmd_code | cmd_ext | payload 0 | payload 1 | ECC
/// [0:3]    | [4:7]   | [8:15]    | [16:23]   | [24:31]
///
/// fwPacket:
///       
///          | command | payload 0 | payload 1 |
///          | [4:7]   | [8:15]    | [16:23]   |
///
/// firmware command:
///
/// GPSM_DCM_DATA                       0 //0b0000
/// GPSM_DCM_WRITE                      1 //0b0001
/// GPSM_DCM_ENABLE_PSTATES             2 //0b0010
/// GPSM_DCM_ENTER_HW_PSTATE_MODE       3 //0b0011
///
/// This API implements the lower level pmc_dcm_send API to send a firmware
/// command as part of the PMC interchip message packet. 
/// 
/// This API is working under a default time out, if send cannot be completed 
/// within the default timeout period due to possible busy interchip link,
/// the function will exit with returning an error code: 
/// \a GPSM_DCM_SEND_PACKET_TIMEOUT
/// 
/// Prerequisite: The enable_interchip_interface bit of PMC_MODE_REG
///               must be set to enable the PMC interchip transfer
///               and lower level PMC-DCM API is required to use this 
///               high level GPSM-DCM API.
///               Also, the hardware must be in DCM setup.
/// 
/// Note: This API must be called by the DCM Master, meaning the firmware 
///       command can only be given by the master to the slave. 
///       Also the API will check if the firmware command to be sent is valid.
///  
/// \retval SUCCESS
///
/// \retval GPSM_DCM_ARG_INVALID_OBJ_SND
///
/// \retval GPSM_DCM_PKT_INVALID_CMD_SND
///
/// \retval GPSM_DCM_CMD_NOT_FROM_MASTER 
///

int
gpsm_dcm_send(GpsmDcmPacket* fwPacket) {

    int rc = GPSM_DCM_SUCCESS;
    PmcDcmPacket hwPacket;

    TRACE_GPSM_B(TRACE_GPSM_DCM_SEND, fwPacket->command);

    do {

        //check if argument is NULL
        SSX_ERROR_IF_CHECK_API(
             (fwPacket == 0),
             GPSM_DCM_ARG_INVALID_OBJ_SND);

        //check if firmware command is valid             
        SSX_ERROR_IF_CHECK_API( 
             (fwPacket->command >= GPSM_DCM_NUMBER_OF_COMMANDS ||
              fwPacket->command == GPSM_IC_DATA), 
             GPSM_DCM_PKT_INVALID_CMD_SND);
     
        //check if is dcm master, note only master can send command to slave
        SSX_ERROR_IF_CHECK_API( 
             (!pmc_dcm_if_dcm_master()), 
             GPSM_DCM_CMD_NOT_FROM_MASTER);
     
        //form hardware packet from given firmware packet
        hwPacket.value = 0;
        hwPacket.fields.cmd_code   = PMC_IC_MSG_CC;
        hwPacket.fields.cmd_ext    = fwPacket->command;
        hwPacket.fields.payload[0] = fwPacket->payload.u8[0];
        hwPacket.fields.payload[1] = fwPacket->payload.u8[1];
    
        //send hardware packet 
        rc = _gpsm_dcm_send(&hwPacket, GPSM_DCM_DEFAULT_TIMEOUT);
     
    } while (0);

    TRACE_GPSM(TRACE_GPSM_DCM_SENT);

    return rc;    
}


/// Abstract blocking/non-blocking receive over the DCM interchip link
///
/// \param fwPacket A GpsmDcmPacket structure as the part of the 
/// PmcDcmPacket/hwPacket received from PMC interchip link.  
/// This argument is passed by the caller as reference. The corresponding 
/// fields of the received packet will filled in this data structure.
///
/// \param timeout_period A SsxInterval variable for time out period
///
/// This API implements the lower level pmc_dcm_receive API to receive a firmware
/// command as part of the PMC interchip message packet.
///
/// This API is working under a user given time out, if receive cannot be 
/// completed within the given timeout period,
/// the function will exit with returning an error code:
/// \a GPSM_DCM_RECV_PACKET_TIMEOUT
///
/// Prerequisite: The enable_interchip_interface bit of PMC_MODE_REG
///               must be set to enable the PMC interchip transfer
///               and lower level PMC-DCM API is required to use this
///               high level GPSM-DCM API.
///               Also, the hardware must be in DCM setup.
///
/// Note: This API must be called by the DCM Slave, meaning the firmware
///       command can only be received by the slave.
///       Also the API will check if the received firmware command is valid.
//  
/// \retval GPSM_DCM_SUCCESS
///
/// \retval GPSM_DCM_ARG_INVALID_OBJ_RCV
///
/// \retval GPSM_DCM_CMD_SHOULD_TO_SLAVE 
///
/// \retval GPSM_DCM_PKT_INVALID_CMD_RCV
///

int
gpsm_dcm_receive(GpsmDcmPacket* fwPacket,                 
                 SsxInterval timeout_period) {

    int rc = GPSM_DCM_SUCCESS;
    PmcDcmPacket hwPacket;

    TRACE_GPSM(TRACE_GPSM_DCM_RECEIVE);

    do {    
        
        //check if argument is NULL
        SSX_ERROR_IF_CHECK_API(
             (fwPacket == 0),
            GPSM_DCM_ARG_INVALID_OBJ_RCV);

        //check if is dcm slave, note only slave receives command from master
        SSX_ERROR_IF_CHECK_API(
            (pmc_dcm_if_dcm_master()), 
            GPSM_DCM_CMD_SHOULD_TO_SLAVE);
    
        //receive hardware packet
        hwPacket.value = 0;
        rc = _gpsm_dcm_receive(&hwPacket, timeout_period);
        if( rc ) break;

        //check if the received command is valid
        fwPacket->command = hwPacket.fields.cmd_ext;
        SSX_ERROR_IF_CHECK_API(
            (fwPacket->command >= GPSM_DCM_NUMBER_OF_COMMANDS ||
             fwPacket->command == 0), 
            GPSM_DCM_PKT_INVALID_CMD_RCV);
    
        //load payload from hardware packet into firmware packet
        fwPacket->payload.u8[0] = hwPacket.fields.payload[0];
        fwPacket->payload.u8[1] = hwPacket.fields.payload[1];
    
    } while (0);

    TRACE_GPSM_B(TRACE_GPSM_DCM_RECEIVED, fwPacket->command);

    return rc;     
}

////////////////////////////////////////////////////////////////////////////
// Generic communication using GPSM-DCM 'write' command
////////////////////////////////////////////////////////////////////////////
///
/// These APIs allow applications to perform generic communication over the
/// GPSM-DCM link using the GPSM-DCM 'write' command.  Like all GPSM-DCM
/// commands, communication is always controlled/initiated by the master. The
/// infrastructure assumes that the master and slave applications argee on the
/// maximum amount of data that may be transmitted generically as a single
/// abstract transaction.  It is legal for the data transmission size to be 0.
///
/// In the following APIs, the \a timeout parameter is a timeout for any
/// single send/receive transaction to complete, not a timeout covering the
/// entire send/receive.  The timeout may be specified as SSX_WAIT_FOREVER to
/// indicate no timeout.
///
/// One typical use of these APIs would be to implement a polled 'ping' of the
/// slave from the master.  This sequence might be implemented as follows:
///
/// - Master: Calls gpsm_dcm_write() with 0 size [ping]
/// - Master: Blocks on gpsm_dcm_read [Wait for ping response]
/// - Slave: GpsmSlaveControl.write_handler() is activated to handle the ping
/// - Slave: write_handler() responds by calling gpsm_dcm_write()
/// - Master: Unblocks and processes ping reqponse


/// Send an arbitrary amount of data (max 2^16 - 1 bytes) using GPSM-DCM
///
/// This API is typically used by the master to initiate generic
/// communication.  When received by the slave the application-specific
/// callback is activated on the slave to handle the write.
/// The slave will only use this API when it is known that the master is
/// expecting a communication from the slave and has blocked on a call of
/// gpsm_dcm_read().
///
/// The receiver must be prepared to accept \a size bytes of data, otherwise
/// the call will fail immediately.
///
/// \param buf The buffer contains the sending message prepared by the caller
///
/// \param size The size of a message, in number of bytes, given by the caller
///
/// \param timeout_period The SSX timeout variable
///
/// The firmware level API for sending a generic PMC interchip message
/// This API sends the message by calling lower level API: pmc_dcm_send
///
/// The API times out upon \a timeout_period.
///
/// Prerequisite: The enable_interchip_interface bit of PMC_MODE_REG
///               must be set to enable the PMC interchip transfer
///               and lower level PMC-DCM API is required to use this
///               high level GPSM-DCM API.
///               Also, the hardware must be in DCM setup.
///
/// Note: When this API is called by the DCM Master, 
///       the slave will need to provide its own write_handler to process
///       the receiving message, The counterpart gpsm_dcm_read API of this
///       gpsm_dcm_write API is not designed for slave to read the master 
///       sending message.
///
/// \retval GPSM_DCM_SUCCESS
///
/// \retval GPSM_DCM_ARG_INVALID_OBJ_WRT
///

int
gpsm_dcm_write(void* buf,
               uint16_t size,
               SsxInterval timeout_period)
{
    int rc = GPSM_DCM_SUCCESS; 
    PmcDcmPacket     hwPacket;
    GpsmDcmFastData  fastData;

    TRACE_GPSM_H(TRACE_GPSM_DCM_WRITE, size);

    //setup send irq
    ssx_irq_setup(PGP_IRQ_PMC_INTERCHIP_MSG_SEND_ONGOING,
                  SSX_IRQ_POLARITY_ACTIVE_LOW,
                  SSX_IRQ_TRIGGER_LEVEL_SENSITIVE);
    do {

        //check if buffer is allocated
        SSX_ERROR_IF_CHECK_API(
            (buf == 0 && size != 0),
            GPSM_DCM_ARG_INVALID_OBJ_WRT);
 
         //form initial hardware packet as header packet
         hwPacket.value = 0;
         hwPacket.fields.cmd_code = PMC_IC_MSG_CC;
         hwPacket.fields.cmd_ext  = GPSM_IC_WRITE;
         hwPacket.value += SET_PAYLOAD_FIELD(size);   
 
         //setup data structure for fast write handler
         fastData.buffer_pointer = buf;
         fastData.remaining_size = size;
         ssx_semaphore_create(&(fastData.fast_semaphore), 0, 1);
 
         //setup fast write handler for send irq
         ssx_irq_handler_set(PGP_IRQ_PMC_INTERCHIP_MSG_SEND_ONGOING,
                             gpsm_dcm_fast_write,
                             (void*)(&fastData), 
                             SSX_NONCRITICAL);
 
         //send header packet
         rc = _gpsm_dcm_send(&hwPacket, timeout_period);
         if( rc ) break;
         
         //enable interrupt and semaphore
         ssx_irq_enable(PGP_IRQ_PMC_INTERCHIP_MSG_SEND_ONGOING);     
         ssx_semaphore_pend(&(fastData.fast_semaphore), timeout_period);
         
     } while (0); 

    TRACE_GPSM(TRACE_GPSM_DCM_WRITE_COMPLETE);
 
     return rc;
}


/// Receive a transmission from the slave
///
/// This API is only called on the master, as part of an application-specific
/// protocol where a transmission is expected from the slave.
///
/// The call will fail immediately if the slave attempts to send more than \a
/// buf_size bytes. The actual number of bytes received is returned as \a
/// data_size.
///
/// \param buf The buffer for storing the incoming messages,
/// return to caller as reference
///
/// \param buf_size The maximum size of the buffer, given by caller
/// fail operation if data_size > buf_size
///
/// \param data_size The size of acutal receiving message,
/// return to caller as refernce
///
/// \param timeout_period The SSX timeout variable
///
/// The firmware level API for receiving a PMC interchip message
/// This API recevies the message by calling lower level API: pmc_dcm_receive
/// and then checks \a buf_size and \a data_size for unexpected message size
/// if the size overflow, the function will exit with returning an error code:
/// \a GPSM_DCM_DAT_BIGGER_THAN_BUF
/// The valid receive message will be placed into \a buf and return with
/// the actual \a data_size. The API times out upon \a timeout.
///
/// Prerequisite: The enable_interchip_interface bit of PMC_MODE_REG
///               must be set to enable the PMC interchip transfer
///               and lower level PMC-DCM API is required to use this
///               high level GPSM-DCM API.
///               Also, the hardware must be in DCM setup.
///
/// Note: This API is only designed for master to read/receive a message from
///       slave (slave will have its own write_handler). Therefore, this API
///       can only called by DCM master, and the API will check if the received
///       message is from a write command.    
///
/// \retval GPSM_DCM_SUCCESS
///
/// \retval GPSM_DCM_ARG_INVALID_OBJ_RED
///
/// \retval GPSM_DCM_READ_RECV_NOT_WRITE
///
/// \retval GPSM_DCM_DAT_BIGGER_THAN_BUF
///

int
gpsm_dcm_read(void* buf,
              uint16_t buf_size,
              uint16_t* data_size,
              SsxInterval timeout_period)
{
    int rc = GPSM_DCM_SUCCESS;
    PmcDcmPacket    hwPacket;
    GpsmDcmFastData fastData;

    TRACE_GPSM(TRACE_GPSM_DCM_READ);

    //setup receive irq
    ssx_irq_setup(PGP_IRQ_PMC_INTERCHIP_MSG_RECV,
                  SSX_IRQ_POLARITY_ACTIVE_HIGH,
                  SSX_IRQ_TRIGGER_LEVEL_SENSITIVE);

    do {

        //check if buffer is allocated
        SSX_ERROR_IF_CHECK_API(
            (buf == 0 && buf_size != 0),
            GPSM_DCM_ARG_INVALID_OBJ_RED);

        //try to receive the header packet
        hwPacket.value = 0;
        rc = _gpsm_dcm_receive(&hwPacket, timeout_period);
        if( rc ) break;

        //check if command code of header packet is valid
        SSX_ERROR_IF_CHECK_API(
            (hwPacket.fields.cmd_ext != GPSM_IC_WRITE),
            GPSM_DCM_READ_RECV_NOT_WRITE);

        //load size from header packet
        *data_size = GET_PAYLOAD_FIELD(hwPacket.value);

        //check if data size will fit in the buffer
        if( *data_size > buf_size ) {
            rc = GPSM_DCM_DAT_BIGGER_THAN_BUF;
            break;
        }

        //check if data size is 0
        if( *data_size == 0 ) 
            break;

        //setup data structure for fast read
        fastData.buffer_pointer = buf;
        fastData.remaining_size = *data_size;
        ssx_semaphore_create(&(fastData.fast_semaphore), 0, 1);

        //setup fast read handler for receive irq
        ssx_irq_handler_set(PGP_IRQ_PMC_INTERCHIP_MSG_RECV,
                            gpsm_dcm_fast_read,
                            (void*)(&fastData),
                            SSX_NONCRITICAL);
 
        //enable interrupt and semaphore
        ssx_irq_enable(PGP_IRQ_PMC_INTERCHIP_MSG_RECV);    
        ssx_semaphore_pend(&(fastData.fast_semaphore), timeout_period);
        
    } while (0);

    TRACE_GPSM_H(TRACE_GPSM_DCM_READ_COMPLETE, *data_size);

    return rc;
}

/// Method to sync between two chips using write/read API
///
/// This API can be called before a piece of application code to sync
/// between master and slave to enter the same code block together 
/// as well as after a piece of code to sync on exiting the block of code
///
/// \param state use 0 for enter and use 1 for exit
///
/// \retval GPSM_DCM_SUCCESS
///

int 
gpsm_dcm_sync(int state)
{
   int rc = GPSM_DCM_SUCCESS;
   int master = pmc_dcm_if_dcm_master();
   uint16_t buf_size = sizeof(uint16_t);
   uint16_t wbuf = 0x7962; //SYNC
   uint16_t rbuf;
   uint16_t size;
   SsxMachineContext smc;

   do {

       ssx_critical_section_enter(SSX_NONCRITICAL, &smc);

       if( master ^ state ) {
           rc = gpsm_dcm_write((void*)&wbuf,buf_size,SSX_WAIT_FOREVER);
           if(rc) break;
           rc = gpsm_dcm_read((void*)&rbuf,buf_size,&size,SSX_WAIT_FOREVER);
           if(rc) break;
       } else {
           rc = gpsm_dcm_read((void*)&rbuf,buf_size,&size,SSX_WAIT_FOREVER);
           if(rc) break;
           rc = gpsm_dcm_write((void*)&wbuf,buf_size,SSX_WAIT_FOREVER);
           if(rc) break;
       }          

       ssx_critical_section_exit(&smc);

   } while (0);
  
   return rc;    
}


/// GPSM DCM Slave function
///
/// This function must be called from a thread, as if a command is available
/// it may always be necessary for the thread to block during command
/// execution. The \a wait argument indicates whether the caller is willing to
/// wait for a command indefinitely, or prefers for gpsm_dcm_slave() to retun
/// immediately if no command is available. \a wait would normally be non-0 if
/// \a gpsm_dcm_slave() were used as a standalone thread body, and 0 if
/// gpsm_dcm_slave() were called from another thread.
///
/// \param control The callback table structure that allows the user to
/// implement callback function in addition to completion of each command.
/// 
/// \param wait The SsxInterval type timeout variable
///
/// This API is the control function running on a slave thread.  The function
/// will first try to receive a message from master, and then take the
/// corresponding action according to the command in the message. Upon
/// completion of the default action designed for each command, any user
/// provided callback function for that command in the control structure will
/// be called.
///  
/// Prerequisite: This API must be called from a DCM slave SSX thread
///               The enable_interchip_interface bit of PMC_MODE_REG
///               must be set to enable the PMC interchip transfer
///               and lower level PMC-DCM API is required to use this
///               high level GPSM-DCM API.
///               Also, the hardware must be in DCM setup.
///
/// Similar to other SSX drivers, the callback is always called regardless of
/// whether the slave command succeeds or fails.
///
/// \retval 0 Success
///
/// \retval GPSM_DCM_SLAVE_TIMEOUT The call timed out before a packet was
/// received from the master.  The application may or may not treat this as an
/// error. 
///
/// \retval GPSM_DCM_SLAVE_ERROR In the event of a this return code, the
/// application will need to query 3 return codes stored in the \a control
/// structure to understand the source of the error.  The \a
/// control->protocol_rc contains the return code for all GPSM-DCM protocol
/// actions, other than the simple timeout mentioned above.  The \a
/// control->slave_rc is the return code for the slave action commanded by the
/// master.  This is also the code sent back to the master as an
/// acknowledgement. The control->callback_rc is the return code from the
/// callback, if any.
///
///  The return value of the function is either 0,
/// GPSM_DCM_SLAVE_TIMEOUT, or 
/// GPSM_DCM_SLAVE_ERROR.  
/// 

int
gpsm_dcm_slave(GpsmSlaveControl *control, 
               SsxInterval wait)
{
    GpsmDcmPacket fwPacket;
    int rc, protocolRc, slaveRc, callbackRc;

    TRACE_GPSM(TRACE_GPSM_DCM_SLAVE);

    protocolRc = 0;
    slaveRc = 0;
    callbackRc = 0;

    do {
        protocolRc = gpsm_dcm_receive(&fwPacket, wait);
        if(protocolRc) {
            control->command = GPSM_IC_NO_COMMAND;
            break;
        }

        control->command = fwPacket.command;

        switch (fwPacket.command) {

        case GPSM_IC_ENABLE_PSTATES:

            slaveRc = gpsm_enable_pstates_slave(0, 
                                                fwPacket.payload.pstate[0],
                                                fwPacket.payload.pstate[1]);

            if(control->gpsm_dcm_callback_enable_pstates != 0) {
                callbackRc =
                    control->gpsm_dcm_callback_enable_pstates(&fwPacket);
            }

            break;

        case GPSM_IC_HW_PSTATE_MODE:

            slaveRc = gpsm_hw_mode();

            if(control->gpsm_dcm_callback_hw_pstate_mode != 0) {
                callbackRc =
                    control->gpsm_dcm_callback_hw_pstate_mode(&fwPacket);
            }

            break;

        case GPSM_IC_WRITE:
            slaveRc = control->write_handler(control->buffer, 
                                             control->buffer_size, 
                                             control->write_arg);
            break;
        default: 
            slaveRc = GPSM_DCM_PKT_INVALID_CMD_RCV;
            break;
        }
          
        if(fwPacket.command != GPSM_IC_WRITE) {
            protocolRc = gpsm_dcm_write((void*)(&slaveRc),
                                        sizeof(int),
                                        SSX_WAIT_FOREVER);
        }
    } while(0);   

    control->protocol_rc = protocolRc;
    control->slave_rc = slaveRc;
    control->callback_rc = callbackRc;

    if (protocolRc == GPSM_DCM_RECV_PACKET_TIMEOUT) {
        rc = GPSM_DCM_SLAVE_TIMEOUT;
    } else if (protocolRc || slaveRc || callbackRc) {
        rc = GPSM_DCM_SLAVE_ERROR;
    } else {
        rc = 0;
    }

    TRACE_GPSM_B(TRACE_GPSM_DCM_SLAVE_COMPLETE, fwPacket.command);

    return rc;
}


/// Master an abstract transaction with a return code response from the slave
///
/// \param fwPacket A legal, initalized GpsmDcmPacket.  This packet may be of
/// any type other than GPSM_IC_DATA or GPSM_IC_WRITE.  The GPSM_IC_WRITE does
/// not get a response from the slave.
///
/// \param slaveRc The return code from running the protocol action on the
/// slave.  This includes any return code from a callback installed on the
/// slave. If the return value of the function is non-0 then \a slaveRc is
/// considered undefined.
///
/// This API send \a fwPacket to the slave and blocks waiting for the slave to
/// respond with the \a slaveRc.  The return value of the API indicates any
/// problems with sending the packet or receiving the response. 

int
gpsm_dcm_master(GpsmDcmPacket* fwPacket, int* slaveRc)
{
   int rc;
   uint16_t size;

   do {

       rc = gpsm_dcm_send(fwPacket);
       if(rc) break;
       
       rc = gpsm_dcm_read((void*)slaveRc,
                          sizeof(int),
                          &size,
                          SSX_WAIT_FOREVER);
       if(rc) break;
       
       //check if command_reply has correct size
       SSX_ERROR_IF_CHECK_API((size != sizeof(int)),
                              GPSM_DCM_CMD_REPLY_NOT_INT);
    
   }while(0);

   return rc;
}
