/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/proc/proc_data_control.c $                        */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2019                        */
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

#include "proc_data.h"
#include "proc_data_control.h"
#include "occhw_async.h"
#include "threadSch.h"
#include "proc_data_service_codes.h"
#include "occ_service_codes.h"
#include "errl.h"
#include "trac.h"
#include "rtls.h"
#include "apss.h"
#include "state.h"
#include "occ_sys_config.h"
#include "pstates_common.H"
#include "pgpe_interface.h"
#include "pgpe_shared.h"
#include "rtls_service_codes.h"
#include "proc_pstate.h"
#include "occ_util.h"
#include "common.h"             // For ignore_pgpe_error()

// The GPE parameter fields for PGPE IPC calls.
extern GPE_BUFFER(ipcmsg_clip_update_t  G_clip_update_parms);
extern GPE_BUFFER(ipcmsg_set_pmcr_t     G_pmcr_set_parms);

extern GpeRequest G_clip_update_req;
extern GpeRequest G_pmcr_set_req;

// number of ticks to wait on clip/pmcr request to complete before checking to log an error
// this must give the PGPE at least 1ms, doubling that time to 2ms to be safe
#define SUPPRESS_PGPE_ERR_WAIT_TICKS 4  // 2ms

// A state transition is currently going on?
extern volatile bool G_state_transition_occuring;
// Need to send Pstates even if they didn't "change"?
extern volatile bool G_set_pStates;
extern uint16_t G_allow_trace_flags;

// Only allow pstates after mode change.
bool G_allowPstates = FALSE;

// Function Specification
//
// Name: task_core_data_control
//
// Description: Control quad actuation for all configured cores on every tick.
//
// End Function Specification
void task_core_data_control( task_t * i_task )
{
    errlHndl_t      err            = NULL;   //Error handler
    static bool     L_trace_logged = false;  // trace logging to avoid unnecessarily repeating logs
    static bool     L_current_timeout_recorded = FALSE;
    static uint64_t L_last = 0xFFFFFFFFFFFFFFFF;
    static uint64_t L_ignore_wait_count = 0; // number of consecutive ticks IPC task failed
    bool            l_check_failure = false;
    int             l_request_is_idle = 0;
    uint8_t         l_request_rc = 0;
    enum occExtReasonCode l_ext_rc = OCC_NO_EXTENDED_RC;

    // Once a state transition process starts, task data control
    // stops updating the PMCR/CLIPS updates, this way, the state
    // transition protocol can set the clips without colliding
    // with the task_core_data_control IPC tasks.
    if(G_state_transition_occuring)
    {
        L_ignore_wait_count = 0;
        L_current_timeout_recorded = FALSE;
        if(L_trace_logged == false)
        {
            TRAC_INFO("task_core_data_control: Pstate Control stopped because a state transition started.");
            L_trace_logged = true;
        }
    } // if in state transition
    else
    {
        L_trace_logged = false;

        if (G_allowPstates)
        {
            // perform Pstate/clip control if previous IPC call completed successfully
            if(G_sysConfigData.system_type.kvm) // OPAL system uses clip update request
            {
                l_request_is_idle = async_request_is_idle(&G_clip_update_req.request);
                l_request_rc = G_clip_update_parms.msg_cb.rc;

                // confirm that the clip update IPC from last cycle
                // has successfully completed on PGPE (with no errors)
                if( (l_request_is_idle) &&              //clip_update/set_clip_ranges completed
                    (l_request_rc == PGPE_RC_SUCCESS) ) // with no errors
                {
                    //call PGPE IPC function to update the clips
                    pgpe_clip_update();
                }
                else
                {
                    l_check_failure = true;
                    l_ext_rc = ERC_PGPE_CLIP_FAILURE;
                    /*
                     * @errortype
                     * @moduleid    RTLS_TASK_CORE_DATA_CONTROL_MOD
                     * @reasoncode  PGPE_FAILURE
                     * @userdata1   rc
                     * @userdata2   clip update task idle?
                     * @userdata4   ERC_PGPE_CLIP_FAILURE
                     * @devdesc     pgpe clip update returned an error
                     */
                }
            }
            else
            {
                // NON OPAL System, OCC owns PMCR and uses PMCR set request
                l_request_is_idle = async_request_is_idle(&G_pmcr_set_req.request);
                l_request_rc = G_pmcr_set_parms.msg_cb.rc;
                if( (l_request_is_idle) &&                  // PMCR IPC from last TICK completed
                    (l_request_rc == PGPE_RC_SUCCESS) )     // with no errors
                {
                    //The previous Non-OPAL PGPE request succeeded
                    uint64_t pstateList = 0;
                    unsigned int quad = 0;
                    for (quad = 0; quad < MAXIMUM_QUADS; quad++)
                    {
                        pstateList |= ((uint64_t) G_desired_pstate[quad] << ((7-quad)*8));
// TODO - RTC 213672 - pmcr is no longer an array
#if 0
                        // Update pmcr value (per quad) with requested pstate and version (Version 1 (P9 format))
                        G_pmcr_set_parms.pmcr[quad] = ((uint64_t) G_desired_pstate[quad] << 48) | 1;
#endif
                    }

                    // Only send pstates if they changed or first time after going active from char state
                    // in characterization state the user is writing the PMCR, need to make sure we write
                    // it back to desired Pstates
                    if( (L_last != pstateList) || (G_set_pStates) )
                    {
                        G_set_pStates = FALSE;
                        L_last = pstateList;

                        if( G_allow_trace_flags & ALLOW_PMCR_TRACE )
                        {
                            TRAC_INFO("task_core_data_control: calling pmcr_set() w/pstates: 0x%08X%04X",
                                 WORD_HIGH(pstateList), WORD_LOW(pstateList)>>16);
                        }
                        //call PGPE IPC function to update Pstates
                        pgpe_pmcr_set();
                    }

                }
                else
                {
                    l_check_failure = true;
                    l_ext_rc = ERC_PGPE_SET_PMCR_FAILURE;
                    /*
                     * @errortype
                     * @moduleid    RTLS_TASK_CORE_DATA_CONTROL_MOD
                     * @reasoncode  PGPE_FAILURE
                     * @userdata1   rc
                     * @userdata2   pmcr set task idle?
                     * @userdata4   ERC_PGPE_SET_PMCR_FAILURE
                     * @devdesc     pgpe PMCR set returned an error
                     */
                }
            }
        } // if pstates allowed

        // Common error handling for all systems
        if(l_check_failure)
        {
            // an earlier clip update IPC call has not completed
            L_ignore_wait_count++;

            // Only log the error if we are not to ignore PGPE errors and have
            // waited enough time for the PGPE to give this indication
            if(L_ignore_wait_count >= SUPPRESS_PGPE_ERR_WAIT_TICKS)
            {
                if(!ignore_pgpe_error())
                {
                    TRAC_ERR("task_core_data_control: pstate[0x%02X] update IPC task did not complete successfully, idle?[%d] rc[%08X]",
                              G_desired_pstate[0], l_request_is_idle, l_request_rc);

                    err = createPgpeErrl(RTLS_TASK_CORE_DATA_CONTROL_MOD,               //ModId
                                         PGPE_FAILURE,                                  //Reasoncode
                                         l_ext_rc,                                      //Extended reason code
                                         ERRL_SEV_PREDICTIVE,                           //Severity
                                         l_request_rc,                                  //Userdata1
                                         l_request_is_idle);                            //Userdata2

                    //Add firmware callout
                    addCalloutToErrl(err,
                                     ERRL_CALLOUT_TYPE_COMPONENT_ID,
                                     ERRL_COMPONENT_ID_FIRMWARE,
                                     ERRL_CALLOUT_PRIORITY_HIGH);

                    //Add processor callout
                    addCalloutToErrl(err,
                                     ERRL_CALLOUT_TYPE_HUID,
                                     G_sysConfigData.proc_huid,
                                     ERRL_CALLOUT_PRIORITY_MED);

                    // commit error log
                    REQUEST_RESET(err);
                }
                else
                {
                    // Wait forever for PGPE to respond
                    // Put a mark on the wall so we know we hit this state
                    if(!L_current_timeout_recorded)
                    {
                        INCREMENT_ERR_HISTORY(ERRH_PSTATE_CHANGE_IGNORED);
                        L_current_timeout_recorded = TRUE;
                    }
                }
            }
        }
        else
        {
             // no error, clear the error wait count
             L_ignore_wait_count = 0;
             L_current_timeout_recorded = FALSE;
        }
    } // else not in a state transition
    return;
}
