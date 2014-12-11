// $Id: pmc_dcm.c,v 1.2 2014/02/03 01:30:25 daviddu Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/lib/pmc_dcm.c,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file pmc_dcm.c
/// \brief Genertic PMC Interhchip Communication Mechanism 

#include "ssx.h"
#include "pmc_dcm.h"

////////////////////////////////////////////////////////////////////////////
// Low-level PMC-DCM Interfaces
////////////////////////////////////////////////////////////////////////////

/// Non-blocking transmission of a packet over the PMC-DCM interface
///
/// Note: Locking/synchronization of the PMC-DCM interface is the
/// responsibility of the application.
///
/// \param hwPacket A PmcDcmPacket structure to be sent via PMC interchip link
/// This argument is provided by the caller and passed in as reference.
/// 
/// \code
/// hwPacket: 
/// 
/// cmd_code | cmd_ext | payload 0 | payload 1 | ECC
/// [0:3]    | [4:7]   | [8:15]    | [16:23]   | [24:31]
/// 
/// cmd_code:
/// PMC_IC_GPA_CC      | 1  | 0b0001 | Global PState Actual  | Master to Slave
/// PMC_IC_GPA_ACK_CC  | 2  | 0b0010 | Global PState Ack     | Slave to Master
/// PMC_IC_GAR_CC      | 3  | 0b0011 | Global Actual Request | Slave to Master
/// PMC_IC_PING_CC     | 4  | 0b0100 | Ping                  | Master to Slave
/// PMC_IC_PING_ACK_CC | 6  | 0b0110 | Ping Acknowledge      | Slave to Master
/// PMC_IC_MSG_CC      | 8  | 0b1000 | Message               | Bidirectional
/// PMC_IC_MSG_NACK_CC | 10 | 0b1010 | Message NACK          | Bidirectional
/// PMC_IC_MSG_ACK_CC  | 11 | 0b1011 | Message ACK           | Bidirectional
/// PMC_IC_ERROR_CC    | 12 | 0b1111 | Error                 | Slave to Master
/// \endcode
/// 
/// This API sends command to another chip through the PMC interchip wire 
/// in DCM setup. The message will be sent out by writing the packet value 
/// to the regisiter: PMC_INTCHP_MSG_WDATA
///
/// It also checks the interchip_ga_ongoing and interchip_msg_send_ongoing
/// bits of PMC_INTCHP_STATUS_REG to detect if the channel is free
/// If the channel is busy, the function will exit with returning 
/// an error code \a PMC_DCM_OUTSTANDING_TRANSFER
///
/// Prerequisite: The enable_interchip_interface bit of PMC_MODE_REG
///               must be set to enable the PMC interchip transfer
///               Also, the hardware must be in DCM setup
///
/// Note: This API can be used to send any valid command over the link
///       however, both command code and corresponding direction of transfer
///       will be checked to ensure correctness of the protocol. Upon 
///       attempt to send an invalid command or an unexpected direction 
///       of transfering certain command will cause this API to abort 
///       which indicates a HW/FW bug 
///
/// \retval PMC_DCM_SUCCESS
///
/// \retval PMC_DCM_ARG_NULL_OBJECT_SEND
///
/// \retval PMC_DCM_INTCHP_DISABLED_SEND
///
/// \retval PMC_DCM_OUTSTANDING_TRANSFER
/// 
/// \retval PMC_DCM_INVALID_COMMAND_CODE
///

int
pmc_dcm_send(PmcDcmPacket* hwPacket)
{
    int rc = PMC_DCM_SUCCESS;
 
    do {

        // check if reference packet is null
        SSX_ERROR_IF_CHECK_API(
            (hwPacket == 0), 
            PMC_DCM_ARG_NULL_OBJECT_SEND);

        // check if interchip transfer is enabled on this chip
        SSX_ERROR_IF_CHECK_API(
            (pmc_dcm_if_interchip_interface_enabled() == 0),
            PMC_DCM_INTCHP_DISABLED_SEND);

        //check if command code is valid and direction of transfer is valid
        rc = pmc_dcm_check_ic_command((int)hwPacket->fields.cmd_code);
        if( rc ) break; 

        // check if the interchip channel is busy
        if( pmc_dcm_if_channel_busy() ) {
            rc = PMC_DCM_OUTSTANDING_TRANSFER;
            break;
        }

        // send out the command
        _pmc_dcm_send(&hwPacket->value);

    } while (0);

    return rc;
}

/// Non-blocking reception of a packet from the PMC-DCM interface
///
/// Note: Locking/synchronization of the PMC-DCM interface is the
/// responsibility of the application.
///
/// \param hwPacket A PmcDcmPacket structure passed by the caller 
/// as reference to receive the message sent from PMC interchip link
/// 
/// This API receives the message from the PMC interchip wire 
/// by reading the register: PMC_INTCHP_MSG_RDATA
///
/// It checks the interchip_msg_recv_detected bit of 
/// PMC_INTCHP_STATUS_REG to know if there is a new message
/// If no new message is detected, the receive function will 
/// exit with returning an error code \a PMC_DCM_RECEIVE_NOT_DETECTRD
///
/// Prerequisite: The enable_interchip_interface bit of PMC_MODE_REG 
///               must be set to enable the PMC interchip transfer
///               Also, the hardware must be in DCM setup
///
/// Note: only MSG type message or cmd code should be received
///       otherwise function aborts and indicates a hardware bug
///
/// \retval PMC_DCM_SUCCESS
///
/// \retval PMC_DCM_ARG_NULL_OBJECT_RECV
///
/// \retval PMC_DCM_INTCHP_DISABLED_RECV
///
/// \retval PMC_DCM_RECEIVE_NOT_DETECTRD
///
/// \retval PMC_DCM_RECEIVE_NOT_MSG_TYPE
///

int
pmc_dcm_receive(PmcDcmPacket* hwPacket)
{
    int rc = PMC_DCM_SUCCESS;

    do {

        //check if reference packet is null
        SSX_ERROR_IF_CHECK_API(
            (hwPacket == 0),
            PMC_DCM_ARG_NULL_OBJECT_RECV);

        // check if interchip transfer is enabled on this chip
        SSX_ERROR_IF_CHECK_API(
            (!pmc_dcm_if_interchip_interface_enabled()),
            PMC_DCM_INTCHP_DISABLED_RECV);   
 
        // check if there is a new incoming message
        if( !pmc_dcm_if_new_message() ) {
            rc = PMC_DCM_RECEIVE_NOT_DETECTED;
            break; 
        }
    
        // receive the new message
        _pmc_dcm_receive(&hwPacket->value);
    
        // check if the command is MSG type
        SSX_ERROR_IF_CHECK_API(
            (hwPacket->fields.cmd_code != PMC_IC_MSG_CC), 
            PMC_DCM_RECEIVE_NOT_MSG_TYPE);

    } while (0);

    return rc;
}



/// Internal API : Send data without error checking
///
/// \param value 32 bits data to be sent
///
/// This API send the interchip data by writing register:
/// PMC_INTCHP_MSG_WDATA
///
/// \retval NONE
///

void
_pmc_dcm_send(uint32_t *value)
{
    out32(PMC_INTCHP_MSG_WDATA, *value);
}


/// Internal API : Receive data without error checking
///
/// \param value 32 bits data to be received
///
/// This API receive the interchip data by reading register: 
/// PMC_INTCHP_MSG_RDATA
/// 
/// \retval NONE
///

void
_pmc_dcm_receive(uint32_t *value)
{
    *value = in32(PMC_INTCHP_MSG_RDATA);
}

/// This API tells if the given command is a valid pmc interchip command
/// and if the command is given by the designated source
///
/// \param cmd_code the command code
///
/// \param rc the return code back to caller
/// 
/// \code
/// cmd_code:
///
/// PMC_IC_GPA_CC      | 1  | 0b0001 | Global PState Actual  | Master to Slave
/// PMC_IC_GPA_ACK_CC  | 2  | 0b0010 | Global PState Ack     | Slave to Master
/// PMC_IC_GAR_CC      | 3  | 0b0011 | Global Actual Request | Slave to Master
/// PMC_IC_PING_CC     | 4  | 0b0100 | Ping                  | Master to Slave
/// PMC_IC_PING_ACK_CC | 6  | 0b0110 | Ping Acknowledge      | Slave to Master
/// PMC_IC_MSG_CC      | 8  | 0b1000 | Message               | Bidirectional
/// PMC_IC_MSG_NACK_CC | 10 | 0b1010 | Message NACK          | Bidirectional
/// PMC_IC_MSG_ACK_CC  | 11 | 0b1011 | Message ACK           | Bidirectional
/// PMC_IC_ERROR_CC    | 12 | 0b1111 | Error                 | Slave to Master
/// \endcode
///
/// \retval PMC_DCM_INTCHP_CMD_ONLY_MTOS
///
/// \retval PMC_DCM_INTCHP_CMD_ONLY_STOM
///
/// \retval PMC_DCM_INVALID_COMMAND_CODE
///

int
pmc_dcm_check_ic_command(int cmd_code)
{
    //note:certain command can only be transferred from master to slave
    //     or from slave to master or bidirectional.
    if( cmd_code == PMC_IC_GPA_CC ||
        cmd_code == PMC_IC_PING_CC ) {
        //those commands can only be sent from master to slave
        SSX_ERROR_IF_CHECK_API(
            (!pmc_dcm_if_dcm_master()),
            PMC_DCM_INTCHP_CMD_ONLY_MTOS);            
    } else if( cmd_code == PMC_IC_GPA_ACK_CC  ||
               cmd_code == PMC_IC_GAR_CC      ||
               cmd_code == PMC_IC_PING_ACK_CC ||
               cmd_code == PMC_IC_ERROR_CC ) {
        //those commands can only be sent from slave to master
        SSX_ERROR_IF_CHECK_API(
            (pmc_dcm_if_dcm_master()),
            PMC_DCM_INTCHP_CMD_ONLY_STOM);
    } else if( !(cmd_code == PMC_IC_MSG_CC      ||
                 cmd_code == PMC_IC_MSG_NACK_CC ||
                 cmd_code == PMC_IC_MSG_ACK_CC) ) {
        //those commands are bidirectional
        //none of above, invalid command
        SSX_ERROR_IF_CHECK_API(
            0,
            PMC_DCM_INVALID_COMMAND_CODE);
    } 
    return PMC_DCM_SUCCESS;
}



/// This API tells if the current chip is the DCM master or slave
///
/// \param NONE
///
/// The DCM master/slave is configured as the interchip_mode bit in register:
/// PMC_MODE_REG
/// 
/// \retval 1 Master
/// 
/// \retval 0 Slave
///

int
pmc_dcm_if_dcm_master()
{
    pmc_mode_reg_t pmc_mode_reg;
    pmc_mode_reg.value = in32(PMC_MODE_REG);
    return pmc_mode_reg.fields.enable_interchip_interface && 
           pmc_mode_reg.fields.interchip_mode;
}

/// This API sets the current chip to be the DCM master or slave
///
/// \param master if 1 then set to master otherwise slave
///
/// The DCM master/slave is configured as the interchip_mode bit in register:
/// PMC_MODE_REG
///
/// \retval NONE
///

void
pmc_dcm_set_interchip_mode(int master)
{
    pmc_mode_reg_t pmc_mode_reg;
    pmc_mode_reg.value = in32(PMC_MODE_REG);
    pmc_mode_reg.fields.enable_interchip_interface = 1;
    if( master == 0 )
        pmc_mode_reg.fields.interchip_mode = 0;
    else
        pmc_mode_reg.fields.interchip_mode = 1;
    out32(PMC_MODE_REG, pmc_mode_reg.value);
}

/// This API tells if the current chip is enabled with interchip interface 
///
/// \param NONE
///
/// The DCM master/slave is configured as the enable_interchip_interface bit 
/// in register: PMC_MODE_REG
///
/// Note: set this bit is required for any interchip communication
///
/// \retval 1 Enabled
///
/// \retval 0 Disabled
///

int
pmc_dcm_if_interchip_interface_enabled()
{
    pmc_mode_reg_t pmc_mode_reg;
    pmc_mode_reg.value = in32(PMC_MODE_REG);
    return pmc_mode_reg.fields.enable_interchip_interface;
}

/// This API sets the current chip to be enabled with interchip interface
///
/// \param enable if 1 then interface enabled otherwise disabled
///
/// The DCM master/slave is configured as the enable_interchip_interface bit 
/// in register: PMC_MODE_REG
///
/// \retval NONE
///

void
pmc_dcm_set_interchip_interface(int enable)
{
    pmc_mode_reg_t pmc_mode_reg;
    pmc_mode_reg.value = in32(PMC_MODE_REG);
    if( enable == 0 )
        pmc_mode_reg.fields.enable_interchip_interface = 0;
    else
        pmc_mode_reg.fields.enable_interchip_interface = 1;
    out32(PMC_MODE_REG, pmc_mode_reg.value);
}


/// This API tells if the interchip channel is busy for outgoing communication
///
/// \param NONE
///
/// depends on bits: interchip_ga_ongoing and interchip_msg_send_ongoing
/// in register: PMC_INTCHP_STATUS_REG
///
/// \retval 1 Busy
///
/// \retval 0 Free
///

int
pmc_dcm_if_channel_busy()
{
    pmc_intchp_status_reg_t pmc_intchp_status_reg;
    pmc_intchp_status_reg.value = in32(PMC_INTCHP_STATUS_REG);
    return pmc_intchp_status_reg.fields.interchip_msg_send_ongoing;
    //return (pmc_intchp_status_reg.fields.interchip_ga_ongoing ||
    //        pmc_intchp_status_reg.fields.interchip_msg_send_ongoing); 
}

/// This API tells if there is a new message arrived from the interchip wire
///
/// \param NONE
///
/// if bit interchip_msg_recv_detected in register: PMC_INTCHP_STATUS_REG
/// is set then new message otherwise none
///
/// \retval 1 New Message
///
/// \retval 0 NO New Message
///

int
pmc_dcm_if_new_message()
{
    pmc_intchp_status_reg_t pmc_intchp_status_reg;
    pmc_intchp_status_reg.value = in32(PMC_INTCHP_STATUS_REG);
    return pmc_intchp_status_reg.fields.interchip_msg_recv_detected;
}

/// This API initializes the DCM setup 
///
/// \param master_or_slave configure the current chip to be master or slave
///
/// for current chip
/// 1) enable interchip interface
/// 2) configure to be DCM master or slave
///
/// Note: one chip has to be master and one chip has to be slave
///
/// \retval NONE
///

void
pmc_dcm_init(int master_or_slave)
{     
    pmc_dcm_set_interchip_mode(master_or_slave);
}
