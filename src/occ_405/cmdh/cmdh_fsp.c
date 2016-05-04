/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/cmdh/cmdh_fsp.c $                                     */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2015                        */
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

#include "ssx.h"
//#include "special_wakeup.h" // lib/special_wakeup.h doesn't exist
#include "cmdh_service_codes.h"
#include "errl.h"
#include "trac.h"
#include "rtls.h"
#include "dcom.h"
#include "occ_common.h"
#include "state.h"
#include "cmdh_fsp.h"
#include "cmdh_fsp_cmds.h"
#include "cmdh_mnfg_intf.h"
#include "cmdh_tunable_parms.h"
#include "cmdh_snapshot.h"
#include "scom.h"
#include "homer.h"

// OCB Channel Setup Defines
const int      OCB_CHANNEL_FSP_LINEAR          = 0;
const int      OCB_CHANNEL_FSP_DOORBELL        = 1;
const int      ALLOW_UNTRUSTED_ACCESS          = 1;

// Do not change this without changing size of FSP Command Buffer
// (and vice versa).  The Linear window requires an alignment on
// a power of 2 boundry, and linear window registers require this
// to be expressed as a Log
const int      LOG_SIZEOF_FSP_CMD_BUFFER       = 12;   // 4096 = 2**12

// The OCB Device Driver request that OCC uses to receive a doorbell from FSP
OcbRequest G_fsp_doorbell_ocb_request;

// Semaphore used to alert CMDH thread that a message was received from TMGT
SsxSemaphore G_cmdh_fsp_wakeup_thread;

// Buffers for the command being sent between FSP & OCC.  This needs to be in
// noncacheable space, or needs cache invalidate/flush to be run appropriately
// This also needs to have the alignment sync'd up with the size of FSP
// Command Buffer, and LOG_SIZEOF_FSP_CMD_BUFFER
LINEAR_WINDOW_WR_BUFFER(fsp_cmd_t G_fsp_cmd);
LINEAR_WINDOW_RD_BUFFER(fsp_rsp_t G_fsp_rsp);

// This holds the pointers to the command & response buffers, and to the
// doorbell placeholder data.
fsp_msg_t G_fsp_msg = {
    .cmd = (fsp_cmd_t *) CMDH_LINEAR_WINDOW_BASE_ADDRESS,
    .rsp = (fsp_rsp_t *) CMDH_OCC_RESPONSE_BASE_ADDRESS,
};

// Temporary storage used by our SSX_PANIC macro
uint32_t __occ_panic_save_r3;
uint32_t __occ_panic_save_r4;
uint32_t __occ_panic_save_r5;
uint32_t __occ_panic_save_msr;

// Storage for commands coming from HTMGT
DMA_BUFFER( fsp_cmd_t G_htmgt_cmd_buffer ) = {{{{0}}}};
// Storage for responses sent to HTMGT
DMA_BUFFER( fsp_rsp_t G_htmgt_rsp_buffer ) = {{{{0}}}};

// This determines how OCC will send an interrupt to Host:
// 0x00 = use FSI2MBOX; 0x01 = use PSIHB complex
uint8_t G_occ_interrupt_type = FSP_SUPPORTED_OCC;

errlHndl_t cmdh_processTmgtRequest (const cmdh_fsp_cmd_t * i_cmd_ptr,
                                          cmdh_fsp_rsp_t * i_rsp_ptr);
// G_rsp_status is used to store the response status value for the command being
// processed.  The code will not write this status to the response buffer until the
// entire buffer is ready to be sent.  At that point, G_rsp_status will be written
// which will notify/trigger the consumer that the response is ready.
uint8_t  G_rsp_status = 0;

// Function Specification
//
// Name:  notifyCmdhWakeupCondition
//
// Description: Wake up command processing thread
//
// End Function Specification
void notifyCmdhWakeupCondition(eCmdhWakeupThreadMask i_cond)
{
    G_cmdh_thread_wakeup_mask |= i_cond;
    ssx_semaphore_post(&G_cmdh_fsp_wakeup_thread);
}

// Function Specification
//
// Name:  clearCmdhWakeupCondition
//
// Description: Removes condition from the thread wakeup mask
//
// End Function Specification
void clearCmdhWakeupCondition(eCmdhWakeupThreadMask i_cond)
{
    G_cmdh_thread_wakeup_mask &= ~i_cond;
}

// Function Specification
//
// Name:  notifyFspDoorbellReceived
//
// Description: Notifies the command handler thread that a command
//              from the FSP has been received by releasing the
//              G_cmdh_fsp_wakeup_thread semaphore.
//
// End Function Specification
void notifyFspDoorbellReceived(void * i_arg)
{
    notifyCmdhWakeupCondition(CMDH_WAKEUP_FSP_COMMAND);
}


// Function Specification
//
// Name:  cmdh_thread_wait_for_wakeup
//
// Description: Blocks command handler thread until
//              a doorbell from the FSP is received
//
// End Function Specification
int cmdh_thread_wait_for_wakeup(void)
{
    int l_rc = 0;

    // Check if we already have a pending wait for a doorbell,
    // if we do, don't schedule another one, that would result
    // in either undefined behavior or a race condition
    if(async_request_is_idle(&G_fsp_doorbell_ocb_request.request))
    {
        CMDH_DBG("Waiting for FSP Doorbell...\n");
        ocb_request_schedule(&G_fsp_doorbell_ocb_request);
    }

    // Wait for someone to wakeup this thread
    l_rc = ssx_semaphore_pend(&G_cmdh_fsp_wakeup_thread, SSX_WAIT_FOREVER);

    return l_rc;
}

// Function Specification
//
// Name: cmdh_fsp_init
//
// Description: TODO -- Add description
//
// End Function Specification
errlHndl_t cmdh_fsp_init(void)
{
    errlHndl_t            l_errlHndl = NULL;

    CHECKPOINT(INIT_OCB);

    // ----------------------------------------------------
    // Initialize OCB Channels
    // ----------------------------------------------------
    // The channel initialization is mostly done by SSX
    // or by HostBoot, but there are a couple things OCC
    // needs to do

    // Create OCB Request for doorbell
    ssx_semaphore_create(&G_cmdh_fsp_wakeup_thread,0,0);

    ocb_request_create(&G_fsp_doorbell_ocb_request,
            &G_ocb_read_queue[1],
            (uint64_t *) &G_fsp_msg.doorbell[0],
            8,
            SSX_WAIT_FOREVER,
            (AsyncRequestCallback)notifyFspDoorbellReceived,
            (void *)NULL,
            ASYNC_CALLBACK_NONCRITICAL);

    ocb_request_schedule(&G_fsp_doorbell_ocb_request);

    CHECKPOINT(OCB_INITIALIZED);

    // ----------------------------------------------------
    // Open Up Linear Window for FSP Communication
    // ----------------------------------------------------

    if(G_occ_interrupt_type != PSIHB_INTERRUPT &&
       G_occ_interrupt_type != FSP_SUPPORTED_OCC)
    {
        // Invalid interrupt type
        CMDH_TRAC_ERR("cmdh_fsp_init: Invalid OCC interrupt type was detected! interrupt_type[%d]",
                 G_occ_interrupt_type);

        /* @
         * @errortype
         * @moduleid    CMDH_OCC_INTERRUPT_TYPE
         * @reasoncode  EXTERNAL_INTERFACE_FAILURE
         * @userdata1   OCC Interrupt Type
         * @userdata2   0
         * @userdata4   OCC_NO_EXTENDED_RC
         * @devdesc     Invalid OCC interrupt type was detected
         */
        l_errlHndl = createErrl(
                CMDH_OCC_INTERRUPT_TYPE,           // modId
                EXTERNAL_INTERFACE_FAILURE,        // reasoncode
                OCC_NO_EXTENDED_RC,                // Extended reason code
                ERRL_SEV_PREDICTIVE,               // Severity
                NULL,                              // Trace Buf
                DEFAULT_TRACE_SIZE,                // Trace Size
                G_occ_interrupt_type,              // userdata1
                0                                  // userdata2
                );
    }

    return l_errlHndl;
}

// Function Specification
//
// Name:  checksum16
//
// Description: Generates the checksum of the message (sum of preceding bytes)
//
// End Function Specification
uint16_t checksum16(uint8_t * i_data, const uint16_t i_len)
{
    uint8_t * l_data  = i_data;
    uint16_t  l_len   = 0;
    uint16_t  l_cksm  = 0;

    for(l_len=0; l_len<i_len; l_len++)
    {
        l_cksm += *l_data;
        l_data++;
    }

    return l_cksm;
}


// Function Specification
//
// Name:  cmdh_build_errl_rsp
//
// Description: This function is to be used to handle as many as possible
//              error log scenerios that come up during a TMGT->OCC
//              command execution, resulting in the correct "Error Response
//              Data Packet"
//
//              It is to be placed in the error path, and pass in the bad RC
//              that should be sent to TMGT, and/or the errlHndl to go along
//              with it.
//
//              If a errlHndl containing an error log is passed in, that log
//              will be used, and no generic structure will be generated.
//
//              If the errlHndl that is passed in to this function is
//              INVALID_ERR_HNDL, then this function will use that as the
//              error log.
//
//              ...However...
//
//              It is valid (and encouraged, when appropriate) to pass in an
//              empty/NULL errlHndl, which will then be filled in with the
//              generic error log, containing:
//                 - The passed in Return Code to TMGT (modified if SUCCESS)
//                 - The first 32 bytes of the passed in command as data in
//                   the user details area of the error log
//
//              This function then generates the "Error Response Data Packet"
//              response to TMGT, including the "error log id" in the response
//              automatically.
//
//              This function will only update the response RC if it is
//              currently set to success, or if the errl passed in is
//              NULL.  If the response rc is set, and there is an error
//              log, it is assumed this function has already been called,
//              and we won't overwrite the previous error response.
//
//              Interface Architecture: TMGT to OCC -- contains further
//              details on the "Error Response Data Packet."
//
// End Function Specification
void cmdh_build_errl_rsp(const cmdh_fsp_cmd_t * i_cmd_ptr,
                         cmdh_fsp_rsp_t       * o_rsp_ptr,
                         ERRL_RC                i_rc,
                         errlHndl_t           * io_errlHndl)
{
    errl_generic_resp_t * l_errl_rsp_ptr = (errl_generic_resp_t *) o_rsp_ptr;
    errlHndl_t * l_errlHndlPtr           = io_errlHndl;
    uint8_t l_reason;

    // Make sure there is an error log pointer passed in so we don't
    // dereference a NULL pointer
    if(NULL != l_errlHndlPtr)
    {
        // Check if we need to update the return code, we only need to update this
        // if no one has previously set it to an error rc due to a error log
        if( (ERRL_RC_SUCCESS == G_rsp_status) || (NULL == *l_errlHndlPtr) )
        {
            if( (i_rc == ERRL_RC_SUCCESS) || (i_rc == ERRL_RC_CONDITIONAL_SUCCESS) )
            {
                // We can't return success in an error packet, change the
                // return code to INTERNAL FAIL, because it must have been
                // a code bug.
                G_rsp_status = ERRL_RC_INTERNAL_FAIL;
            }
            else
            {
                G_rsp_status = i_rc;
            }
        }

        // If no error log was passed in to this function, create one
        if(NULL == *l_errlHndlPtr)
        {
            /* @
             * @errortype
             * @moduleid    CMDH_GENERIC_CMD_FAILURE
             * @reasoncode  INTERNAL_FAILURE
             * @userdata1   1st 4 bytes of command that had the failure
             * @userdata2   Return Code of command that failed
             * @userdata4   ERC_CMDH_INTERNAL_FAILURE
             * @devdesc     A Valid FSP to OCC command failed
             */
            /* @
             * @errortype
             * @moduleid    CMDH_GENERIC_CMD_FAILURE
             * @reasoncode  INVALID_INPUT_DATA
             * @userdata1   1st 4 bytes of command that had the failure
             * @userdata2   Return Code of command that failed
             * @userdata4   ERC_CMDH_INTERNAL_FAILURE
             * @devdesc     FSP to OCC packet is invalid
             */
            //For invalid data, use INVALID_INPUT_DATA reason code.  Use INTERNAL_FAILURE otherwise.
            l_reason = (i_rc == ERRL_RC_INVALID_DATA)? INVALID_INPUT_DATA: INTERNAL_FAILURE;
            *l_errlHndlPtr = createErrl(
                    CMDH_GENERIC_CMD_FAILURE,           //modId
                    l_reason,                           //reasoncode
                    ERC_CMDH_INTERNAL_FAILURE,          //Extended reason code
                    ERRL_SEV_UNRECOVERABLE,             //Severity
                    NULL,    //TODO: create trace       //Trace Buf
                    DEFAULT_TRACE_SIZE,                 //Trace Size
                    CONVERT_UINT8_ARRAY_UINT32(i_cmd_ptr->cmd_type,
                                               i_cmd_ptr->data[0],
                                               i_cmd_ptr->data[1],
                                               i_cmd_ptr->data[2]), //userdata1
                    i_rc                                //userdata2
                    );

            // Callout firmware
            addCalloutToErrl(*l_errlHndlPtr,
                             ERRL_CALLOUT_TYPE_COMPONENT_ID,
                             ERRL_COMPONENT_ID_FIRMWARE,
                             ERRL_CALLOUT_PRIORITY_HIGH);

            // Copy in the first 32 bytes of the command that was sent
            addUsrDtlsToErrl(*l_errlHndlPtr,                //io_err
                    (uint8_t *) i_cmd_ptr,         //i_dataPtr,
                    32,                            //i_size
                    ERRL_USR_DTL_STRUCT_VERSION_1, //version
                    ERRL_USR_DTL_BINARY_DATA);     //type
        }

        // Fill out the response lod_id with the error id
        l_errl_rsp_ptr->log_id = getErrlLogId(*l_errlHndlPtr);
    }
    else
    {
        // This is a firmware bug, TMGT will see it is a bad RC,
        // with no error log, pick up on this, and log its own error
        l_errl_rsp_ptr->log_id = 0;  //ERRL_SLOT_INVALID
        G_rsp_status = ERRL_RC_INTERNAL_FAIL;
    }

    l_errl_rsp_ptr->data_length[0] = 0;
    l_errl_rsp_ptr->data_length[1] = 1;

    return;
}


// Function Specification
//
// Name:  cmdh_fsp_cmd_hndler
//
// Description: Start processing command from HTMGT/TMGT/BMC
//
// End Function Specification
errlHndl_t cmdh_fsp_cmd_hndler(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    errlHndl_t l_errlHndl = NULL;
    uint16_t l_cksm       = 0;
    uint16_t l_data_len   = 0;
    uint16_t l_cmd_len    = 0;
    uint8_t  l_sender_id  = 0;
    uint8_t  l_attn_type  = 0;
    int      l_ssxrc      = SSX_OK;
    uint32_t l_reasonCode = 0;
    uint32_t l_extReasonCode = 0;
    uint32_t l_userdata2  = 0;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    do
    {
        // Extract the sender ID and the attention type from doorbell
        l_sender_id = G_fsp_msg.doorbell[0];
        l_attn_type = G_fsp_msg.doorbell[1];

        // Validate the sender ID and the attention type
        if(((l_sender_id != ATTN_SENDER_ID_FSP)   &&
            (l_sender_id != ATTN_SENDER_ID_HTMGT) &&
            (l_sender_id != ATTN_SENDER_ID_BMC))  ||
           (l_attn_type != ATTN_TYPE_CMD_WRITE))
        {
            CMDH_TRAC_ERR("Attention received with invalid values: sender_id[0x%02X] attn_type[0x%02X]",
                     l_sender_id,
                     l_attn_type);

            /* @
             * @errortype
             * @moduleid    CMDH_GENERIC_CMD_FAILURE
             * @reasoncode  EXTERNAL_INTERFACE_FAILURE
             * @userdata1   Sender id
             * @userdata2   Attention type
             * @userdata4   ERC_CMDH_INVALID_ATTN_DATA
             * @devdesc     Received an attention with invalid values
             */
            l_errlHndl = createErrl(
                CMDH_GENERIC_CMD_FAILURE,           //modId
                EXTERNAL_INTERFACE_FAILURE,         //reasoncode
                ERC_CMDH_INVALID_ATTN_DATA,         //Extended reason code
                ERRL_SEV_PREDICTIVE,                //Severity
                NULL,                               //Trace Buf
                DEFAULT_TRACE_SIZE,                 //Trace Size
                l_sender_id,                        //userdata1
                l_attn_type);                       //userdata2

            break;
        }

        // Determine which buffer to read based on the sender ID
        if(l_sender_id == ATTN_SENDER_ID_HTMGT)
        {
            // Need to read command data from HOMER
            BceRequest pba_copy;

            // Set up a copy request
            l_ssxrc = bce_request_create(&pba_copy,                           // block copy object
                                         &G_pba_bcde_queue,                   // mainstore to sram copy engine
                                         OCC_HTMGT_CMD_ADDRESS_HOMER,         // mainstore address
                                         (uint32_t) &G_htmgt_cmd_buffer,      // sram starting address
                                         (size_t) sizeof(G_htmgt_cmd_buffer), // size of copy
                                         SSX_WAIT_FOREVER,                    // no timeout
                                         NULL,                                // no call back
                                         NULL,                                // no call back arguments
                                         ASYNC_REQUEST_BLOCKING);             // blocking request

            if(l_ssxrc != SSX_OK)
            {
                CMDH_TRAC_ERR("cmdh_fsp_cmd_hndler: BCE request create failure rc=[%08X]", -l_ssxrc);
                /*
                 * @errortype
                 * @moduleid    CMDH_GENERIC_CMD_FAILURE
                 * @reasoncode  SSX_GENERIC_FAILURE
                 * @userdata1   RC for BCE block-copy engine
                 * @userdata2   Internal function check-point
                 * @userdata4   ERC_BCE_REQ_CREATE_READ_FAILURE
                 * @devdesc     SSX BCE related failure
                 */
                l_reasonCode = SSX_GENERIC_FAILURE;
                l_extReasonCode = ERC_BCE_REQ_CREATE_READ_FAILURE;
                l_userdata2 = 0x01;
                break;
            }

            // Do actual copying
            l_ssxrc = bce_request_schedule(&pba_copy);

            if(l_ssxrc != SSX_OK)
            {
                CMDH_TRAC_ERR("cmdh_fsp_cmd_hndler: BCE request schedule failure rc=[%08X]", -l_ssxrc);
                /*
                 * @errortype
                 * @moduleid    CMDH_GENERIC_CMD_FAILURE
                 * @reasoncode  SSX_GENERIC_FAILURE
                 * @userdata1   RC for BCE block-copy engine
                 * @userdata2   Internal function check-point
                 * @userdata4   ERC_BCE_REQ_SCHED_READ_FAILURE
                 * @devdesc     Failed to copy data by using DMA
                 */
                l_reasonCode = SSX_GENERIC_FAILURE;
                l_extReasonCode = ERC_BCE_REQ_SCHED_READ_FAILURE;
                l_userdata2 = 0x02;
                break;
            }

            // Zero out the response buffer and then fill in seq, cmd_type,
            // and update the return status to IN_PROGRESS to signal that
            // OCC has started processing the command
            memset(&G_htmgt_rsp_buffer, 0, sizeof(G_htmgt_rsp_buffer));
            G_htmgt_rsp_buffer.fields.seq      = G_htmgt_cmd_buffer.fields.seq;
            G_htmgt_rsp_buffer.fields.cmd_type = G_htmgt_cmd_buffer.fields.cmd_type;
            G_htmgt_rsp_buffer.fields.rc             = ERRL_RC_CMD_IN_PROGRESS;
            G_htmgt_rsp_buffer.fields.data_length[0] = 0;
            G_htmgt_rsp_buffer.fields.data_length[1] = 1;
            G_htmgt_rsp_buffer.fields.data[0]        = 0x00;

            // Need to write the response to HOMER. Set up a copy request
            l_ssxrc = bce_request_create(&pba_copy,                           // block copy object
                                         &G_pba_bcue_queue,                   // sram to mainstore copy engine
                                         OCC_HTMGT_RSP_ADDRESS_HOMER,         // mainstore address
                                         (uint32_t) &G_htmgt_rsp_buffer,      // sram starting address
                                         (size_t) sizeof(G_htmgt_rsp_buffer), // size of copy
                                         SSX_WAIT_FOREVER,                    // no timeout
                                         NULL,                                // no call back
                                         NULL,                                // no call back arguments
                                         ASYNC_REQUEST_BLOCKING);             // blocking request

            if(l_ssxrc != SSX_OK)
            {
                CMDH_TRAC_ERR("cmdh_fsp_cmd_hndler: BCE request create failure rc=[%08X]", -l_ssxrc);
                /*
                 * @errortype
                 * @moduleid    CMDH_GENERIC_CMD_FAILURE
                 * @reasoncode  SSX_GENERIC_FAILURE
                 * @userdata1   RC for BCE block-copy engine
                 * @userdata2   Internal function check-point
                 * @userdata4   ERC_BCE_REQ_CREATE_INPROG_FAILURE
                 * @devdesc     SSX BCE related failure
                 */
                l_reasonCode = SSX_GENERIC_FAILURE;
                l_extReasonCode = ERC_BCE_REQ_CREATE_INPROG_FAILURE;
                l_userdata2 = 0x03;
                break;
            }

            // Do actual copying
            l_ssxrc = bce_request_schedule(&pba_copy);

            if(l_ssxrc != SSX_OK)
            {
                CMDH_TRAC_ERR("cmdh_fsp_cmd_hndler: BCE request schedule failure rc=[%08X]", -l_ssxrc);
                /*
                 * @errortype
                 * @moduleid    CMDH_GENERIC_CMD_FAILURE
                 * @reasoncode  SSX_GENERIC_FAILURE
                 * @userdata1   RC for BCE block-copy engine
                 * @userdata2   Internal function check-point
                 * @userdata4   ERC_BCE_REQ_SCHED_INPROG_FAILURE
                 * @devdesc     Failed to copy data by using DMA
                 */
                l_reasonCode = SSX_GENERIC_FAILURE;
                l_extReasonCode = ERC_BCE_REQ_SCHED_INPROG_FAILURE;
                l_userdata2 = 0x04;
                break;
            }

            // Determine the command length
            l_data_len = CONVERT_UINT8_ARRAY_UINT16(G_htmgt_cmd_buffer.fields.data_length[0],
                                                    G_htmgt_cmd_buffer.fields.data_length[1]);
            l_cmd_len = l_data_len + CMDH_FSP_SEQ_CMD_SIZE + CMDH_FSP_DATALEN_SIZE;

            // Make sure that the command length isn't > max command length
            // before we calculate the checksum
            if(l_cmd_len > CMDH_FSP_CMD_SIZE)
            {
                l_cmd_len = CMDH_FSP_CMD_SIZE;
            }

            // Verify the command checksum
            l_cksm = checksum16(&G_htmgt_cmd_buffer.byte[0], l_cmd_len);
            if(l_cksm != CONVERT_UINT8_ARRAY_UINT16(G_htmgt_cmd_buffer.byte[l_cmd_len],
                                                    G_htmgt_cmd_buffer.byte[l_cmd_len+1]))
            {
                CMDH_TRAC_ERR("HTMGT Checksum Error! Expected[0x%04X] Received[0x%04X] Command[0x%02X] Command Length[%u]",
                         l_cksm,
                         CONVERT_UINT8_ARRAY_UINT16(G_htmgt_cmd_buffer.byte[l_cmd_len],
                                                    G_htmgt_cmd_buffer.byte[l_cmd_len+1]),
                         G_htmgt_cmd_buffer.fields.cmd_type,
                         l_cmd_len);

                G_rsp_status = ERRL_RC_CHECKSUM_FAIL;
                G_htmgt_rsp_buffer.fields.data_length[0] = 0;
                G_htmgt_rsp_buffer.fields.data_length[1] = 1;
                G_htmgt_rsp_buffer.fields.data[0]        = 0x00; //No error log
            }
            else
            {
                // Command is responsible for RC, Data Len, Data
                l_errlHndl = cmdh_processTmgtRequest(&G_htmgt_cmd_buffer.fields,
                                                     &G_htmgt_rsp_buffer.fields);
            }

            // Calculate length of response
            l_data_len = CONVERT_UINT8_ARRAY_UINT16(G_htmgt_rsp_buffer.fields.data_length[0],
                                                    G_htmgt_rsp_buffer.fields.data_length[1]);
            l_cmd_len = l_data_len + CMDH_FSP_SEQ_CMD_RC_SIZE + CMDH_FSP_DATALEN_SIZE;

            // Add checksum
            l_cksm = checksum16(&G_htmgt_rsp_buffer.byte[0], l_cmd_len);
            // The IN_PROGRESS return status must be removed from checksum, and
            // the final return status (which must be written last) must be added
            l_cksm += G_rsp_status - ERRL_RC_CMD_IN_PROGRESS;
            G_htmgt_rsp_buffer.byte[l_cmd_len]   = CONVERT_UINT16_UINT8_HIGH(l_cksm);
            G_htmgt_rsp_buffer.byte[l_cmd_len+1] = CONVERT_UINT16_UINT8_LOW(l_cksm);
            G_htmgt_rsp_buffer.fields.rc         = G_rsp_status;

            // Need to write the final response to HOMER. Set up a copy request
            l_ssxrc = bce_request_create(&pba_copy,                           // block copy object
                                         &G_pba_bcue_queue,                   // sram to mainstore copy engine
                                         OCC_HTMGT_RSP_ADDRESS_HOMER,         // mainstore address
                                         (uint32_t) &G_htmgt_rsp_buffer,      // sram starting address
                                         (size_t) sizeof(G_htmgt_rsp_buffer), // size of copy
                                         SSX_WAIT_FOREVER,                    // no timeout
                                         NULL,                                // no call back
                                         NULL,                                // no call back arguments
                                         ASYNC_REQUEST_BLOCKING);             // blocking request

            if(l_ssxrc != SSX_OK)
            {
                CMDH_TRAC_ERR("cmdh_fsp_cmd_hndler: BCE request create failure rc=[%08X]", -l_ssxrc);
                /*
                 * @errortype
                 * @moduleid    CMDH_GENERIC_CMD_FAILURE
                 * @reasoncode  SSX_GENERIC_FAILURE
                 * @userdata1   RC for BCE block-copy engine
                 * @userdata2   Internal function check-point
                 * @userdata4   ERC_BCE_REQ_CREATE_WRITE_FAILURE
                 * @devdesc     SSX BCE related failure
                 */
                l_reasonCode = SSX_GENERIC_FAILURE;
                l_extReasonCode = ERC_BCE_REQ_CREATE_WRITE_FAILURE;
                l_userdata2 = 0x05;
                break;
            }

            // Do actual copying
            l_ssxrc = bce_request_schedule(&pba_copy);

            if(l_ssxrc != SSX_OK)
            {
                CMDH_TRAC_ERR("cmdh_fsp_cmd_hndler: BCE request schedule failure rc=[%08X]", -l_ssxrc);
                /*
                 * @errortype
                 * @moduleid    CMDH_GENERIC_CMD_FAILURE
                 * @reasoncode  SSX_GENERIC_FAILURE
                 * @userdata1   RC for BCE block-copy engine
                 * @userdata2   Internal function check-point
                 * @userdata4   ERC_BCE_REQ_SCHED_WRITE_FAILURE
                 * @devdesc     Failed to copy data by using DMA
                 */
                l_reasonCode = SSX_GENERIC_FAILURE;
                l_extReasonCode = ERC_BCE_REQ_SCHED_WRITE_FAILURE;
                l_userdata2 = 0x06;
                break;
            }
        }
        else
        {
            // The doorbell is coming from either TMGT or BMC.

            // Zero out the response buffer and then fill in seq, cmd_type,
            // and update the return status to IN_PROGRESS to signal that
            // OCC has started processing the command
            memset(G_fsp_msg.rsp, 0, (size_t)sizeof(fsp_rsp_t));
            G_fsp_msg.rsp->fields.seq            = G_fsp_msg.cmd->fields.seq;
            G_fsp_msg.rsp->fields.cmd_type       = G_fsp_msg.cmd->fields.cmd_type;
            G_fsp_msg.rsp->fields.rc             = ERRL_RC_CMD_IN_PROGRESS;
            G_fsp_msg.rsp->fields.data_length[0] = 0;
            G_fsp_msg.rsp->fields.data_length[1] = 1;

            // Get Command Data Field Length
            l_data_len = CONVERT_UINT8_ARRAY_UINT16(G_fsp_msg.cmd->fields.data_length[0],
                                                    G_fsp_msg.cmd->fields.data_length[1]);
            l_cmd_len = l_data_len + CMDH_FSP_SEQ_CMD_SIZE + CMDH_FSP_DATALEN_SIZE;

            // Make sure that the message isn't > max message size before we do checksum
            if(l_cmd_len > CMDH_FSP_CMD_SIZE)
            {
                l_cmd_len = CMDH_FSP_CMD_SIZE;
            }

            // Verify Command Checksum
            l_cksm = checksum16(&G_fsp_msg.cmd->byte[0],(l_cmd_len));
            if(l_cksm != CONVERT_UINT8_ARRAY_UINT16(G_fsp_msg.cmd->byte[l_cmd_len],
                                                    G_fsp_msg.cmd->byte[l_cmd_len+1]))
            {
                CMDH_TRAC_ERR("Checksum Error! Expected[0x%04X] Received[0x%04X] Command[0x%02X] Command Length[%u]",
                         l_cksm,
                         CONVERT_UINT8_ARRAY_UINT16(G_fsp_msg.cmd->byte[l_cmd_len],
                                                    G_fsp_msg.cmd->byte[l_cmd_len+1]),
                         G_fsp_msg.cmd->fields.cmd_type,
                         l_cmd_len);

                G_rsp_status = ERRL_RC_CHECKSUM_FAIL;

                G_fsp_msg.rsp->fields.data_length[0] = 0;
                G_fsp_msg.rsp->fields.data_length[1] = 1;
                G_fsp_msg.rsp->fields.data[0]        = 0x00; //No error log
            }
            else
            {
                // Command is responsible for RC, Data Len, Data
                l_errlHndl = cmdh_processTmgtRequest(&G_fsp_msg.cmd->fields,
                                                     &G_fsp_msg.rsp->fields);
            }

            // Finish Building Response

            // Calculate Length of Command
            l_data_len = CONVERT_UINT8_ARRAY_UINT16(G_fsp_msg.rsp->fields.data_length[0],
                                                    G_fsp_msg.rsp->fields.data_length[1]);
            l_cmd_len = l_data_len + CMDH_FSP_SEQ_CMD_RC_SIZE + CMDH_FSP_DATALEN_SIZE;

            // Calculate and add checksum
            l_cksm = checksum16(&G_fsp_msg.rsp->byte[0],(l_cmd_len));
            // The IN_PROGRESS return status must be removed from checksum, and
            // the final return status (which must be written last) must be added
            l_cksm += G_rsp_status - ERRL_RC_CMD_IN_PROGRESS;
            G_fsp_msg.rsp->byte[l_cmd_len] = CONVERT_UINT16_UINT8_HIGH(l_cksm);
            G_fsp_msg.rsp->byte[l_cmd_len+1] = CONVERT_UINT16_UINT8_LOW(l_cksm);

            // Copy the return status last (to indicate command completion)
            G_fsp_msg.rsp->fields.rc = G_rsp_status;
        }

    }while(0);

    if(l_ssxrc != SSX_OK)
    {
        l_errlHndl = createErrl(
            CMDH_GENERIC_CMD_FAILURE,     //modId
            l_reasonCode,                 //reasoncode
            l_extReasonCode,              //Extended reason code
            ERRL_SEV_UNRECOVERABLE,       //Severity
            NULL,                         //Trace Buf
            0,                            //Trace Size
            -l_ssxrc,                     //userdata1
            l_userdata2);                 //userdata2

        // Callout firmware
        addCalloutToErrl(l_errlHndl,
                         ERRL_CALLOUT_TYPE_COMPONENT_ID,
                         ERRL_COMPONENT_ID_FIRMWARE,
                         ERRL_CALLOUT_PRIORITY_HIGH);
    }

    return l_errlHndl;
}

// Function Specification
//
// Name:  cmdh_processTmgtRequest
//
// Description: Call the required function to process the command
//
// End Function Specification
errlHndl_t cmdh_processTmgtRequest (const cmdh_fsp_cmd_t * i_cmd_ptr,
                                    cmdh_fsp_rsp_t       * i_rsp_ptr)
{
    errlHndl_t            l_err         = NULL;
    uint8_t               l_cmd_type    = i_cmd_ptr->cmd_type;

    // Clear the Fields that the commands are responsible for
    i_rsp_ptr->data_length[0] = 0;
    i_rsp_ptr->data_length[1] = 0;
    // Do not write return status to buffer until entire buffer has been written (include cksum)
    G_rsp_status = ERRL_RC_SUCCESS;

    // Run command function based on cmd_type
    switch(l_cmd_type)
    {
        case CMDH_POLL:
            l_err = cmdh_tmgt_poll (i_cmd_ptr,i_rsp_ptr);
            break;

        case CMDH_DEBUGPT:
            cmdh_dbug_cmd (i_cmd_ptr,i_rsp_ptr);
            break;

        case CMDH_SETMODESTATE:
            l_err = cmdh_tmgt_setmodestate(i_cmd_ptr,i_rsp_ptr);
            break;

        case CMDH_SETCONFIGDATA:
            l_err = DATA_store_cnfgdata (i_cmd_ptr,i_rsp_ptr);
            break;
/*
        case CMDH_CLEARERRL:
            l_err = cmdh_clear_elog(i_cmd_ptr, i_rsp_ptr);
            break;

        case CMDH_AME_PASS_THROUGH:
            l_err = cmdh_amec_pass_through(i_cmd_ptr,i_rsp_ptr);
            break;

        case CMDH_RESET_PREP:
            l_err = cmdh_reset_prep(i_cmd_ptr,i_rsp_ptr);
            break;

        case CMDH_MFG_TEST_CMD:
            cmdh_mnfg_test_parse(i_cmd_ptr,i_rsp_ptr);
            break;

        case CMDH_GET_FIELD_DEBUG_DATA:
            l_err = cmdh_tmgt_get_field_debug_data(i_cmd_ptr,i_rsp_ptr);
            break;

        case CMDH_TUNABLE_PARMS:
            l_err = cmdh_tunable_parms(i_cmd_ptr,i_rsp_ptr);
            break;
        case CMDH_SNAPSHOT_SYNC:
            l_err = cmdh_snapshot_sync(i_cmd_ptr,i_rsp_ptr);
            break;
        case CMDH_GET_SNAPSHOT_BUFFER:
            l_err = cmdh_get_snapshot_buffer(i_cmd_ptr,i_rsp_ptr);
            break;
        case CMDH_SET_USER_PCAP:
            l_err = cmdh_set_user_pcap(i_cmd_ptr, i_rsp_ptr);
            break;

        //case CMDH_GET_CPU_TEMPS:
*/
        default:
            CMDH_TRAC_INFO("Invalid or unsupported command 0x%02x",l_cmd_type);

            // -----------------------------------------------
            // Generate error response packet
            // -----------------------------------------------
            cmdh_build_errl_rsp(i_cmd_ptr, i_rsp_ptr, ERRL_RC_INVALID_CMD, &l_err);

            break;
    } //end switch
    return l_err;
}


