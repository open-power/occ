/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/cmdh/cmdh_thread.c $                                  */
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
#include "ssx_io.h"
#include "simics_stdio.h"
#include "errl.h"
#include "trac.h"
#include <thread.h>
#include <threadSch.h>
#include "state.h"
#include <cmdh_fsp.h>

eCmdhWakeupThreadMask G_cmdh_thread_wakeup_mask;

extern uint8_t G_occ_interrupt_type;

// Function Specification
//
// Name: Cmd_Hndl_thread_routine
//
// Description: This needs to be moved to separate file after we add cmd handler
//              thread support
//
// End Function Specification
void Cmd_Hndl_thread_routine(void *arg)
{
#define OCC_RESP_READY_ALERT_TIMEOUT_ATTEMPTS 5
    int l_rc = 0;
    errlHndl_t l_errlHndl = NULL;

    CHECKPOINT(CMDH_THREAD_STARTED);
    TRAC_INFO("Command Handler Thread Started ... " );

    // ------------------------------------------------
    // Initialize HW for FSP Comm
    // ------------------------------------------------
    l_errlHndl = cmdh_fsp_init();
    if(l_errlHndl)
    {
        // Mark Errl as committed, so FSP knows right away we are having
        // problems with Attention, if that is the cause of the error.
        commitErrl(&l_errlHndl);
    }

    CHECKPOINT(FSP_COMM_INITIALIZED);

    // Only send this first attention if FSP is present
    if(G_occ_interrupt_type == FSP_SUPPORTED_OCC)
    {
        // ------------------------------------------------
        // Send 'Service' Attention to signal that we are
        // ready to accept commands from FSP.
        // ------------------------------------------------
        cmdh_fsp_attention( OCC_ALERT_FSP_SERVICE_REQD );

        CHECKPOINT(FIRST_FSP_ATTN_SENT);
    }

    // ------------------------------------------------
    // Loop forever, handling FSP commands
    // ------------------------------------------------
    while(1)
    {
        // ------------------------------------------------
        // Block, Waiting on sem for a doorbell from FSP
        // ------------------------------------------------
        l_rc = cmdh_thread_wait_for_wakeup(); // Blocking Call

        // ------------------------------------------------
        // Handle the command
        // ------------------------------------------------
        if(SSX_OK == l_rc)
        {
            if( CMDH_WAKEUP_FSP_COMMAND & G_cmdh_thread_wakeup_mask )
            {
                clearCmdhWakeupCondition(CMDH_WAKEUP_FSP_COMMAND);

                // Handle the command that TMGT just sent to OCC
                l_errlHndl = cmdh_fsp_cmd_hndler();

                // Commit an error if we get one passed back, do it before
                // we tell the FSP we have a response ready
                if(NULL != l_errlHndl)
                {
                    commitErrl(&l_errlHndl);
                }

                // Check is the sender is FSP
                if(G_fsp_msg.doorbell[0] == ATTN_SENDER_ID_FSP)
                {
                    // Tell the FSP we have a response ready for them.
                    // Try to send this for 500ms before giving up
                    cmdh_fsp_attention_withRetry(OCC_ALERT_FSP_RESP_READY, 500);
                }
            }

            if (CMDH_WAKEUP_FSP_ATTENTION_ALERT & G_cmdh_thread_wakeup_mask)
            {
                clearCmdhWakeupCondition(CMDH_WAKEUP_FSP_ATTENTION_ALERT);

                // Tell the FSP we have something they need to come check on.
                // Try to send this for 500ms, before giving up
                cmdh_fsp_attention_withRetry(OCC_ALERT_FSP_SERVICE_REQD, 500);
            }

            if (CMDH_WAKEUP_FSP_CHECKSTOP_ALERT & G_cmdh_thread_wakeup_mask)
            {
                clearCmdhWakeupCondition(CMDH_WAKEUP_FSP_CHECKSTOP_ALERT);

                // Tell the FSP we have something they need to come check on.
                // Try to send this for 500ms, before giving up
                cmdh_fsp_attention_withRetry(OCC_ALERT_SYS_CHECKSTOP, 500);
            }

        }
    }

}



