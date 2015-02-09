/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/cmdh/cmdh_fsp.c $                                     */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2014                        */
/* [+] Google Inc.                                                        */
/* [+] International Business Machines Corp.                              */
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
#include "special_wakeup.h"
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

// OCB Channel Setup Defines
const int      OCB_CHANNEL_FSP_LINEAR          = 0;
const int      OCB_CHANNEL_FSP_DOORBELL        = 1;
const int      ALLOW_UNTRUSTED_ACCESS          = 1;

// Do not change this without changing size of FSP Command Buffer
// (and vice versa).  The Linear window requires an alignment on
// a power of 2 boundry, and linear window registers require this
// to be expressed as a Log
const int      LOG_SIZEOF_FSP_CMD_BUFFER       = 12;   // 4096 = 2**12

// Given to OCC from HWSV team, sent in FSI2Host Mailbox1 Message
const uint32_t OCC_MSG_QUEUE_ID = 0x80000007;

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

// This determines whether or not OCC can use MBOX for attentions
eFsi2HostMboxState G_fsi2host_mbox_ready = FSI2HOST_MBOX_NOT_USEABLE;

// Temporary storage used by our SSX_PANIC macro
uint32_t __occ_panic_save_r3;
uint32_t __occ_panic_save_r4;
uint32_t __occ_panic_save_r5;
uint32_t __occ_panic_save_msr;

// Storage for commands coming from HTMGT
DMA_BUFFER( fsp_cmd_t G_htmgt_cmd_buffer ) = {{{{0}}}};
// Storage for responses sent to HTMGT
DMA_BUFFER( fsp_rsp_t G_htmgt_rsp_buffer ) = {{{{0}}}};
// Storage for responses sent to BMC
cmdh_fsp_rsp_t G_tmp_rsp_buffer = {{0}};

// This determines how OCC will send an interrupt to Host:
// 0x00 = use FSI2MBOX; 0x01 = use PSIHB complex
uint8_t G_occ_interrupt_type = FSP_SUPPORTED_OCC;

errlHndl_t cmdh_processTmgtRequest (const cmdh_fsp_cmd_t * i_cmd_ptr,
        cmdh_fsp_rsp_t * i_rsp_ptr);

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
// Description: TODO -- Add description
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
// Description: TODO -- Add description
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
// Description: TODO -- Add description
//
// End Function Specification
int cmdh_thread_wait_for_wakeup(void)
{
    int l_rc;

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
// Name: cmdh_fsp_fsi2host_mbox_wait4free
//
// Description:  Checks the OR of all of the SCRATCH7 core scoms on the
//               processor, and waits up to 90 <TBD> seconds for hostboot
//               to signal they are off the fsi2host mbox by clearing the
//               SCRATCH7 scom from 'hostboot' to all zeros.
//
// Moving the mbox check from scratch register 7 to scratch register 3.
//
// End Function Specification
int cmdh_fsp_fsi2host_mbox_wait4free(void)
{
// Scratch register 3
#define PC_SCR3 0x10013286

// Hostboot writes 'hostboot' as its magic number
#define HOSTBOOT_RUNNING 0x686f7374626f6f74ULL

// Use at least 1 min to be safe
#define FSI2HOST_TIMEOUT_IN_SECONDS 90

#define SWAKEUP_TIMEOUT_MS  25 //use 25ms timeout (same as used in gpsm code)

    uint64_t                        l_payload   = 0;
    int                             rc,rc2      = 0;
    int                             l_do_once   = 0;
    int                             l_timeout   = FSI2HOST_TIMEOUT_IN_SECONDS;
    ChipConfigCores                 l_cores, l_swup_timedout;
    pmc_core_deconfiguration_reg_t  l_pcdr;
    bool                            l_disable_swup;
    errlHndl_t                      l_errl;
    bool                            l_swakeup_failure = FALSE;


    // query configured cores for special wakeup
    l_pcdr.value = in32(PMC_CORE_DECONFIGURATION_REG);
    l_cores = ~l_pcdr.fields.core_chiplet_deconf_vector;

    while(0 != l_timeout)
    {
        // Decrement Timeout Timer
        l_timeout--;

        l_disable_swup = FALSE;
        // Enable special wakeup so following getscom doesn't
        // fail with CHIPLET_OFFLINE error on sleeping cores
        rc2 = occ_special_wakeup(TRUE,
                                l_cores,
                                SWAKEUP_TIMEOUT_MS,
                                &l_swup_timedout);
        if(rc2 || l_swup_timedout)
        {
            TRAC_ERR("cmdh_fsp_fsi2host_mbox_wait4free: enable occ_special_wakeup failed with rc=%d, timeout=0x%04x, cores=0x%04x",
                     rc2, l_swup_timedout, l_cores);

            l_swakeup_failure = TRUE;
            break;
        }

        l_disable_swup = TRUE;

        // Read from all cores & OR together so we don't have
        // to figure out which is the hostboot 'master'
        rc = getscom_ffdc(MC_ADDRESS(PC_SCR3,
                    MC_GROUP_EX_CORE, PCB_MULTICAST_OR),
                    &l_payload, NULL);                     //errors committed internally

        if (rc) {
            TRAC_INFO("PC_SCR3 getscom fail rc=%x",rc);
            break;
        }

        if (HOSTBOOT_RUNNING == l_payload)
        {
            // Hostboot is still running on the Mailbox
            if(0 == l_do_once)
            {
                l_do_once = 1;
                TRAC_INFO("HostBoot is on FSI2HOST mailbox...waiting for them to get off");
            }
        }
        else if(0 == l_payload)
        {
            // Hostboot is off the mailbox
            rc = 0;
            break;
        }
        else
        {
            // This is maybe an error, but probably not.  Especially in AVP mode.
            TRAC_INFO("Hostboot is off FSI2HOST mbox, but PC_SCR3 = 0x%08x_%08x",
                    (uint32_t) ((l_payload & 0xFFFFFFFF00000000ULL) >> 32),
                    (uint32_t) ((l_payload & 0x00000000FFFFFFFFULL) >> 0));

            // Return success
            rc = 0;
            break;
        }

        if(l_disable_swup)
        {
            l_disable_swup = FALSE;
            // clear special wakeup while we sleep
            rc2 = occ_special_wakeup(FALSE,
                                    l_cores,
                                    SWAKEUP_TIMEOUT_MS,
                                    &l_swup_timedout);
            if(rc2)
            {
                TRAC_ERR("cmdh_fsp_fsi2host_mbox_wait4free: clear occ_special_wakeup failed with rc=%d, cores=0x%04x",
                         rc2, l_cores);
                l_swakeup_failure = TRUE;
                break;
            }
        }


        // Wait for 1 second until we try again
        ssx_sleep(SSX_SECONDS(1));
    }

    //make sure we clear special wakeup before exiting this function
    if(l_disable_swup)
    {
        rc2 = occ_special_wakeup(FALSE,
                                l_cores,
                                SWAKEUP_TIMEOUT_MS,
                                &l_swup_timedout);
        if(rc2)
        {
            TRAC_ERR("cmdh_fsp_fsi2host_mbox_wait4free: clear occ_special_wakeup failed with rc=%d, cores=0x%04x",
                     rc2, l_cores);
            l_swakeup_failure = TRUE;
        }
    }

    if( 0 == l_timeout )
    {
        rc = -1;
    }

    //Since cores can be left in an unknown state after special wakeup fails
    //Log an error and request a reset to possibly clean up the special wakeup
    if(l_swakeup_failure)
    {
        /* @
         * @errortype
         * @moduleid    CMDH_FSP_FSI2HOST_MBOX_WAIT4FREE
         * @reasoncode  INTERNAL_HW_FAILURE
         * @userdata1   rc - Return code of failing function
         * @userdata2   bitmap of cores that timed out
         * @userdata4   0
         * @devdesc     Failed to enable OCC special wakeup
         */
        l_errl = createErrl(
                CMDH_FSP_FSI2HOST_MBOX_WAIT4FREE,       // modId
                INTERNAL_HW_FAILURE,                    // reasoncode
                OCC_NO_EXTENDED_RC,                     // Extended reason code
                ERRL_SEV_UNRECOVERABLE,                 // Severity
                NULL,                                   // Trace Buf
                DEFAULT_TRACE_SIZE,                     // Trace Size
                rc2,                                    // userdata1
                l_swup_timedout                         // userdata2
                );

        REQUEST_RESET(l_errl);
        if(!rc)
        {
            rc = -1;
        }
    }

    return rc;
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
    int l_rc;
    errlHndl_t l_errlHndl = NULL;
    mbox_data_area_regs_t l_mbox_msg;

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

    if(G_occ_interrupt_type == FSP_SUPPORTED_OCC)
    {
        // ----------------------------------------------------
        // Initialize FSI2Host Mailbox1 -- Attentions to FSP
        // ----------------------------------------------------
        l_rc = cmdh_fsp_fsi2host_mbox_wait4free();
        if(l_rc)
        {
            CHECKPOINT_FLAG(CF_FSI_MB_TIMEOUT);

            G_fsi2host_mbox_ready = FSI2HOST_MBOX_NOT_USEABLE;

            TRAC_ERR("Timeout waiting for HostBoot to get off FSI2HOST mailbox.  rc=%d", l_rc);

            /* @
             * @errortype
             * @moduleid    CMDH_FSI2HOST_MBOX_UNAVAIL
             * @reasoncode  EXTERNAL_INTERFACE_FAILURE
             * @userdata1   l_rc - Return code of failing function
             * @userdata2   0
             * @userdata4   ERC_CMDH_MBOX_REQST_FAILURE
             * @devdesc     Failed to get permission to use fsi2host mbox
             */
            l_errlHndl = createErrl(
                    CMDH_FSI2HOST_MBOX_UNAVAIL,             // modId
                    EXTERNAL_INTERFACE_FAILURE,             // reasoncode
                    ERC_CMDH_MBOX_REQST_FAILURE,            // Extended reason code
                    ERRL_SEV_PREDICTIVE,                    // Severity
                    NULL,                                   // Trace Buf
                    DEFAULT_TRACE_SIZE,                     // Trace Size
                    l_rc,                                   // userdata1
                    0                                       // userdata2
                    );
        }
        else
        {
            CHECKPOINT(INIT_FSI_HOST_MBOX);
            TRAC_INFO("HostBoot is off FSI2HOST mailbox.");

            /// Initialize the FSI2HOST MBOX
            memset(&l_mbox_msg.word[0],0,sizeof(l_mbox_msg));
            l_mbox_msg.fields.msg_id                          = 0;
            l_mbox_msg.fields.msg_queue_id                    = OCC_MSG_QUEUE_ID;
            l_mbox_msg.fields.msg_payload.fsp_cmd_buffer_addr = CMDH_LINEAR_WINDOW_BASE_ADDRESS;
            l_mbox_msg.fields.msg_payload.fsp_rsp_buffer_addr = CMDH_OCC_RESPONSE_BASE_ADDRESS;
            l_mbox_msg.fields.msg_payload.occ_id              = G_pob_id.chip_id;

            do
            {
                // Initialize Headers -- Hardware Provided Header Area not used
                // scom errors will be committed internally
                l_rc = putscom_ffdc(MAILBOX_1_HEADER_COMMAND_0_A_REGADDR,  0x0000000000000000ull, NULL);
                    if(l_rc)
                {
                    break;
                }
                l_rc = putscom_ffdc(MAILBOX_1_HEADER_COMMAND_1_A_REGADDR,  0x0000000000000000ull, NULL);
                if(l_rc)
                {
                    break;
                }
                l_rc = putscom_ffdc(MAILBOX_1_HEADER_COMMAND_2_A_REGADDR,  0x0000000000000000ull, NULL);
                if(l_rc)
                {
                    break;
                }

                // Initialize Data Area -- 64 bytes
                // Message ID & Message Queue ID
                l_rc = putscom_ffdc(MAILBOX_1_DATA_AREA_A_0_REGADDR,  ((uint64_t) l_mbox_msg.fields.msg_id << 32), NULL);
                if(l_rc)
                {
                    break;
                }
                // Command Type & Flags
                l_rc = putscom_ffdc(MAILBOX_1_DATA_AREA_A_1_REGADDR,  ((uint64_t) l_mbox_msg.fields.msg_queue_id << 32), NULL);
                if(l_rc)
                {
                    break;
                }
                // Data[0]
                l_rc = putscom_ffdc(MAILBOX_1_DATA_AREA_A_2_REGADDR,  ((uint64_t) l_mbox_msg.fields.msg_payload.type << 32), NULL);
                if(l_rc)
                {
                    break;
                }
                // Data[1]
                l_rc = putscom_ffdc(MAILBOX_1_DATA_AREA_A_3_REGADDR,  ((uint64_t) l_mbox_msg.fields.msg_payload.flags << 32), NULL);
                if(l_rc)
                {
                    break;
                }
                // (Void *) Extra Data
                l_rc = putscom_ffdc(MAILBOX_1_DATA_AREA_A_4_REGADDR,  ((uint64_t) l_mbox_msg.fields.msg_payload.fsp_cmd_buffer_addr << 32), NULL);
                if(l_rc)
                {
                    break;
                }
                // Unused
                l_rc = putscom_ffdc(MAILBOX_1_DATA_AREA_A_5_REGADDR,  ((uint64_t) l_mbox_msg.fields.msg_payload.fsp_rsp_buffer_addr << 32), NULL);
                if(l_rc)
                {
                    break;
                }
                l_rc = putscom_ffdc(MAILBOX_1_DATA_AREA_A_6_REGADDR,  ((uint64_t) l_mbox_msg.fields.msg_payload.occ_id << 32), NULL);
                if(l_rc)
                {
                    break;
                }
                l_rc = putscom_ffdc(MAILBOX_1_DATA_AREA_A_7_REGADDR,  0x0000000000000000ull, NULL);
                if(l_rc)
                {
                    break;
                }
                l_rc = putscom_ffdc(MAILBOX_1_DATA_AREA_A_8_REGADDR,  0x0000000000000000ull, NULL);
                if(l_rc)
                {
                    break;
                }
                l_rc = putscom_ffdc(MAILBOX_1_DATA_AREA_A_9_REGADDR,  0x0000000000000000ull, NULL);
                if(l_rc)
                {
                    break;
                }

                G_fsi2host_mbox_ready = FSI2HOST_MBOX_INITIALIZED;
                CHECKPOINT(FSI_HOST_MBOX_INITIALIZED);
            }while(0);
        }
    }
    else if(G_occ_interrupt_type == PSIHB_INTERRUPT)
    {
        // For systems like Habanero, OCC will use the PSIHB complex to
        // send an interrupt to Host. This is done via a simple SCOM
        // register, so there is nothing to initialize here.
    }
    else
    {
        // Invalid interrupt type
        TRAC_ERR("cmdh_fsp_init: Invalid OCC interrupt type was detected! interrupt_type[%d]",
                 G_occ_interrupt_type);
    }

    return l_errlHndl;
}


// Function Specification
//
// Name:  cmdh_fsp_attention
//
// Description: TODO -- Add description
//
// End Function Specification
int cmdh_fsp_attention_withRetry(uint32_t i_type, int i_timeout_in_ms)
{
#define CMDH_RETRY_ATTENTION_INTERVAL_MS   10
    int l_timeoutLimit = (i_timeout_in_ms / CMDH_RETRY_ATTENTION_INTERVAL_MS);
    int l_timeout      = 0;
    int l_alert_rc     = 0;
    int l_rc           = 0;

    // Try to send an alert to FSP every second until timeout is
    // reached, or alert is successfully sent.
    for(l_timeout=0; l_timeout < l_timeoutLimit; l_timeout++)
    {
        // Signal to TMGT that OCC has a response ready
        l_alert_rc = cmdh_fsp_attention( OCC_ALERT_FSP_RESP_READY );

        if(OCC_ALERT_SUCCESS == l_alert_rc)
        {
            // Attention successfully sent
            break;
        }
        else if(OCC_ALERT_LAST_ATTN_NOT_COMPLETE == l_alert_rc)
        {
            // If we couldn't send the alert b/c FSP hasn't processed the last
            // one, wait one second, then try again.
            ssx_sleep(SSX_MILLISECONDS(CMDH_RETRY_ATTENTION_INTERVAL_MS));
        }
        else
        {
            // Must have been a PIB Error
            TRAC_ERR("PIB Error on Attention (Type %d) to FSP. rc=0x%x",
                    i_type, l_alert_rc);
            // TODO: Create Error Log?
            break;
        }
    }
    if(l_timeoutLimit == l_timeout)
    {
        TRAC_INFO("Timeout on Attention (Type %d) to FSP. rc=0x%x",
                i_type, l_alert_rc);
        l_rc = -1;
    }

    return l_rc;
}

// Function Specification
//
// Name:  cmdh_fsp_attention
//
// Description: TODO -- Add description
//
// End Function Specification
int cmdh_fsp_attention(uint32_t i_type)
{
    doorbl_stsctrl_reg_t  l_status;
    mbox_data_area_regs_t l_mbox_msg;
    int                   l_rc        = OCC_ALERT_SUCCESS;
    errlHndl_t            l_err       = NULL;

    do
    {
        // Don't send attentions if the FSI2HOST Mailbox isn't ours
        if( FSI2HOST_MBOX_NOT_USEABLE == G_fsi2host_mbox_ready ){break;}

        // Read Status Register for checks below
        l_rc = getscom_ffdc(MAILBOX_1_DOORBELL_STS_CTRL_REGADDR, (uint64_t *) &l_status.doubleword, &l_err);
        if(l_rc)
        {
            break;
        }

        if (l_status.lbus_slaveA_pending)
        {
            // Can't send alert, FSP hasn't handled last attention yet
            l_rc = OCC_ALERT_LAST_ATTN_NOT_COMPLETE;

            // Return from function with error rc, and let caller try again.
            break;
        }
        else
        {
            // No attention in progress to FSP, we are allowed send one
            if (0 == l_status.xup)
            {
                // FSP should have set this, but didn't...
                // TODO: Ignore? Log Error? Valid Case? Abort sending another attention?
            }

            // Set Attention Command Type = OCC_ALERT_FSP_SERVICE_REQD, Flags = 0
            l_mbox_msg.fields.msg_payload.type = i_type;

            // Send Command Type & Flags to Mailbox Register
            l_rc = putscom_ffdc(MAILBOX_1_DATA_AREA_A_2_REGADDR,  ((uint64_t) l_mbox_msg.fields.msg_payload.type << 32), &l_err);
            if(l_rc){break;}

            // Set Attention Interrupt by setting "LBUS slave-A Pending (bit 28, intel notation)"
            l_status.lbus_slaveA_pending  = 1;
            l_status.lbus_slaveA_data_cnt = 0x28;

            // Clear Xup bit, it is set by FSP when it reads the OCC interrupt Mailbox
            l_status.xup = 0;

            // Write Status out to Doorbell Status/Control 1
            l_rc = putscom_ffdc(MAILBOX_1_DOORBELL_STS_CTRL_REGADDR, (uint64_t) l_status.doubleword, &l_err);
            if(l_rc){break;}

            CMDH_DBG("Sent Attention to FSP:  Type[0x%x]\n",i_type);
        }
    }while(0);

    if(l_err)
    {
        // TODO
        //can't commit errors from this function since committing an error will
        //cause another call to raise an attention and we could eventually run
        //out of stack space
        deleteErrl(&l_err);
    }
    return l_rc;
}


// Function Specification
//
// Name:  checksum16
//
// Description: TODO -- Add description
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
        if( (ERRL_RC_SUCCESS == l_errl_rsp_ptr->rc) || (NULL == *l_errlHndlPtr) )
        {
            if( (i_rc == ERRL_RC_SUCCESS) || (i_rc == ERRL_RC_CONDITIONAL_SUCCESS) )
            {
                // We can't return success in an error packet, change the
                // return code to INTERNAL FAIL, because it must have been
                // a code bug.
                l_errl_rsp_ptr->rc = ERRL_RC_INTERNAL_FAIL;
            }
            else
            {
                l_errl_rsp_ptr->rc = i_rc;
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
        l_errl_rsp_ptr->rc     = ERRL_RC_INTERNAL_FAIL;
    }

    l_errl_rsp_ptr->data_length[0] = 0;
    l_errl_rsp_ptr->data_length[1] = 1;

    return;
}


// Function Specification
//
// Name:  cmdh_fsp_cmd_hndler
//
// Description: TODO -- Add description
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
            TRAC_ERR("Attention received with invalid values: sender_id[0x%02X] attn_type[0x%02X]",
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
                                         OCC_HTMGT_CMD_OFFSET_HOMER,          // mainstore address
                                         (uint32_t) &G_htmgt_cmd_buffer,      // sram starting address
                                         (size_t) sizeof(G_htmgt_cmd_buffer), // size of copy
                                         SSX_WAIT_FOREVER,                    // no timeout
                                         NULL,                                // no call back
                                         NULL,                                // no call back arguments
                                         ASYNC_REQUEST_BLOCKING);             // blocking request

            if(l_ssxrc != SSX_OK)
            {
                TRAC_ERR("cmdh_fsp_cmd_hndler: BCE request create failure rc=[%08X]", -l_ssxrc);
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
                TRAC_ERR("cmdh_fsp_cmd_hndler: BCE request schedule failure rc=[%08X]", -l_ssxrc);
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

            // Zero out the response buffer, only update the return status of 'In Progress'
            G_htmgt_rsp_buffer.fields.seq            = 0x00;
            G_htmgt_rsp_buffer.fields.cmd_type       = 0x00;
            G_htmgt_rsp_buffer.fields.rc             = ERRL_RC_CMD_IN_PROGRESS;
            G_htmgt_rsp_buffer.fields.data_length[0] = 0;
            G_htmgt_rsp_buffer.fields.data_length[1] = 1;
            G_htmgt_rsp_buffer.fields.data[0]        = 0x00;

            // Need to write the response to HOMER. Set up a copy request
            l_ssxrc = bce_request_create(&pba_copy,                           // block copy object
                                         &G_pba_bcue_queue,                   // sram to mainstore copy engine
                                         OCC_HTMGT_RSP_OFFSET_HOMER,          // mainstore address
                                         (uint32_t) &G_htmgt_rsp_buffer,      // sram starting address
                                         (size_t) sizeof(G_htmgt_rsp_buffer), // size of copy
                                         SSX_WAIT_FOREVER,                    // no timeout
                                         NULL,                                // no call back
                                         NULL,                                // no call back arguments
                                         ASYNC_REQUEST_BLOCKING);             // blocking request

            if(l_ssxrc != SSX_OK)
            {
                TRAC_ERR("cmdh_fsp_cmd_hndler: BCE request create failure rc=[%08X]", -l_ssxrc);
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
                TRAC_ERR("cmdh_fsp_cmd_hndler: BCE request schedule failure rc=[%08X]", -l_ssxrc);
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
                TRAC_ERR("Checksum Error! Expected[0x%04X] Received[0x%02X%02X] Command Length[%u]",
                         l_cksm,
                         G_htmgt_cmd_buffer.byte[l_cmd_len],
                         G_htmgt_cmd_buffer.byte[l_cmd_len+1],
                         l_cmd_len);

                G_htmgt_rsp_buffer.fields.rc             = ERRL_RC_CHECKSUM_FAIL;
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

            // Prepare the final response
            // Copy the sequence number and command type to response
            G_htmgt_rsp_buffer.fields.seq      = G_htmgt_cmd_buffer.fields.seq;
            G_htmgt_rsp_buffer.fields.cmd_type = G_htmgt_cmd_buffer.fields.cmd_type;

            // Calculate length of response
            l_data_len = CONVERT_UINT8_ARRAY_UINT16(G_htmgt_rsp_buffer.fields.data_length[0],
                                                    G_htmgt_rsp_buffer.fields.data_length[1]);
            l_cmd_len = l_data_len + CMDH_FSP_SEQ_CMD_RC_SIZE + CMDH_FSP_DATALEN_SIZE;

            // Add checksum
            l_cksm = checksum16(&G_htmgt_rsp_buffer.byte[0], l_cmd_len);
            G_htmgt_rsp_buffer.byte[l_cmd_len] = CONVERT_UINT16_UINT8_HIGH(l_cksm);
            G_htmgt_rsp_buffer.byte[l_cmd_len+1] = CONVERT_UINT16_UINT8_LOW(l_cksm);

            // Need to write the final response to HOMER. Set up a copy request
            l_ssxrc = bce_request_create(&pba_copy,                           // block copy object
                                         &G_pba_bcue_queue,                   // sram to mainstore copy engine
                                         OCC_HTMGT_RSP_OFFSET_HOMER,          // mainstore address
                                         (uint32_t) &G_htmgt_rsp_buffer,      // sram starting address
                                         (size_t) sizeof(G_htmgt_rsp_buffer), // size of copy
                                         SSX_WAIT_FOREVER,                    // no timeout
                                         NULL,                                // no call back
                                         NULL,                                // no call back arguments
                                         ASYNC_REQUEST_BLOCKING);             // blocking request

            if(l_ssxrc != SSX_OK)
            {
                TRAC_ERR("cmdh_fsp_cmd_hndler: BCE request create failure rc=[%08X]", -l_ssxrc);
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
                TRAC_ERR("cmdh_fsp_cmd_hndler: BCE request schedule failure rc=[%08X]", -l_ssxrc);
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
            // The doorbell is coming from either TMGT or BMC. In either case,
            // prepare a response with a return status of 'In Progress'
            G_fsp_msg.rsp->fields.rc = ERRL_RC_CMD_IN_PROGRESS;

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
                TRAC_ERR("Checksum Error! Expected[0x%04X] Received[0x%02X%02X]",
                         l_cksm,
                         G_fsp_msg.cmd->byte[l_cmd_len],
                         G_fsp_msg.cmd->byte[l_cmd_len+1]);

                if(l_sender_id == ATTN_SENDER_ID_BMC)
                {
                    G_tmp_rsp_buffer.rc = ERRL_RC_CHECKSUM_FAIL;
                }
                else
                {
                    G_fsp_msg.rsp->fields.rc = ERRL_RC_CHECKSUM_FAIL;
                }
                G_fsp_msg.rsp->fields.data_length[0] = 0;
                G_fsp_msg.rsp->fields.data_length[1] = 1;
                G_fsp_msg.rsp->fields.data[0]        = 0x00; //No error log
                // You could create an errl here, but is that really necessary?
                // TMGT will know that the command didn't work...
            }
            else
            {
                if(l_sender_id == ATTN_SENDER_ID_BMC)
                {
                    // If the sender is BMC, use a temporary buffer to store the
                    // response since the return code should be updated last.
                    // Command is responsible for RC, Data Len, Data
                    l_errlHndl = cmdh_processTmgtRequest (&G_fsp_msg.cmd->fields,
                                                          &G_tmp_rsp_buffer);

                    // Write response data in SRAM except for return code
                    G_fsp_msg.rsp->fields.data_length[0] = G_tmp_rsp_buffer.data_length[0];
                    G_fsp_msg.rsp->fields.data_length[1] = G_tmp_rsp_buffer.data_length[1];
                    memcpy(&G_fsp_msg.rsp->fields.data[0], &G_tmp_rsp_buffer.data[0], sizeof(CMDH_FSP_RSP_DATA_SIZE));
                }
                else
                {
                    // Command is responsible for RC, Data Len, Data
                    l_errlHndl = cmdh_processTmgtRequest (&G_fsp_msg.cmd->fields,
                                                          &G_fsp_msg.rsp->fields);
                }
            }

            // Finish Building Response
            // Copy the sequence number and command type to the response
            G_fsp_msg.rsp->fields.seq      = G_fsp_msg.cmd->fields.seq;
            G_fsp_msg.rsp->fields.cmd_type = G_fsp_msg.cmd->fields.cmd_type;

            // Calculate Length of Command
            l_data_len = CONVERT_UINT8_ARRAY_UINT16(G_fsp_msg.rsp->fields.data_length[0],
                                                    G_fsp_msg.rsp->fields.data_length[1]);
            l_cmd_len = l_data_len + CMDH_FSP_SEQ_CMD_RC_SIZE + CMDH_FSP_DATALEN_SIZE;

            // Add checksum
            l_cksm = checksum16(&G_fsp_msg.rsp->byte[0],(l_cmd_len));
            G_fsp_msg.rsp->byte[l_cmd_len] = CONVERT_UINT16_UINT8_HIGH(l_cksm);
            G_fsp_msg.rsp->byte[l_cmd_len+1] = CONVERT_UINT16_UINT8_LOW(l_cksm);

            // TODO: Do we really need this?
            // Set an ACK on doorbell (sanity check)
            //G_fsp_msg.doorbell[0] |= 0x80;

            // Last thing is to copy the return code in SRAM if the sender is BMC
            if(l_sender_id == ATTN_SENDER_ID_BMC)
            {
                G_fsp_msg.rsp->fields.rc = G_tmp_rsp_buffer.rc;
            }
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
// Description: TODO -- Add description
//
// End Function Specification
errlHndl_t cmdh_processTmgtRequest (const cmdh_fsp_cmd_t * i_cmd_ptr,
        cmdh_fsp_rsp_t * i_rsp_ptr)
{
    errlHndl_t            l_err         = NULL;
    uint8_t               l_cmd_type    = i_cmd_ptr->cmd_type;

    // Clear the Fields that the commands are responsible for
    i_rsp_ptr->data_length[0] = 0;
    i_rsp_ptr->data_length[1] = 0;
    i_rsp_ptr->rc             = ERRL_RC_SUCCESS;

    // Run command function based on cmd_type
    switch(l_cmd_type)
    {
        case CMDH_POLL:
            l_err = cmdh_tmgt_poll (i_cmd_ptr,i_rsp_ptr);
            break;

        case CMDH_QUERYFWLEVEL:
            cmdh_tmgt_query_fw (i_cmd_ptr,i_rsp_ptr);
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

        case CMDH_CLEARERRL:
            l_err = cmdh_clear_elog(i_cmd_ptr, i_rsp_ptr);
            break;

        case CMDH_AME_PASS_THROUGH:
            l_err = cmdh_amec_pass_through(i_cmd_ptr,i_rsp_ptr);
            break;

        case CMDH_GETERRL:
            l_err = cmdh_get_elog(i_cmd_ptr,i_rsp_ptr);
            break;

        case CMDH_RESET_PREP:
            l_err = cmdh_reset_prep(i_cmd_ptr,i_rsp_ptr);
            break;

        case CMDH_GET_COOLING_REQUEST:
            l_err = cmdh_get_cooling_request(i_cmd_ptr,i_rsp_ptr);
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

        //case CMDH_PWREXECPT:
        case CMDH_SET_THERMAL_THROTTLE:
        case CMDH_READ_STATUS_REG:
        case CMDH_GET_THROTTLE_VALUE:
        case CMDH_GET_CPU_TEMPS:
        case CMDH_FOM:
        default:
            TRAC_INFO("Invalid or unsupported command 0x%02x",l_cmd_type);

            // -----------------------------------------------
            // Generate error response packet
            // -----------------------------------------------
            cmdh_build_errl_rsp(i_cmd_ptr, i_rsp_ptr, ERRL_RC_INVALID_CMD, &l_err);

            break;
    } //end switch

    return l_err;
}


