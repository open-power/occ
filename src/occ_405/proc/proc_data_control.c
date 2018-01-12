/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/proc/proc_data_control.c $                        */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2018                        */
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
#include "p9_pstates_common.h"
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

extern bool G_state_transition_occuring;     // A state transition is currently going on?

extern uint8_t G_allow_trace_flags;
// a global flag used by task_core_data_control() to indicate
// that the OCC is ready to transition to observation state
// (after initiatibg a clip update IPC task if needed)
bool G_active_to_observation_ready = false;

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
    Pstate          l_pstate;
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
        if(L_trace_logged == false)
        {
            TRAC_INFO("task_core_data_control: Pstate Control stopped because a state transition started.");
            L_trace_logged = true;
        }

        // Only Transitioning to Observation state necessitates clip update
        // (if the last clip update was higher than legacy turbo).
        if ((G_occ_master_state == OCC_STATE_OBSERVATION) &&
            !G_active_to_observation_ready)
        {
            // confirm that the last clip update IPC successfully completed on PGPE (with no errors)
            if( async_request_is_idle(&G_clip_update_req.request) &&  //clip_update/set_clip_ranges completed
                (G_clip_update_parms.msg_cb.rc == PGPE_RC_SUCCESS) ) // with no errors
            {
                uint8_t quad = 0;
                Pstate pclip = 0xff;    // Initialize pclip to 0xff (lowest possible frequency)

                // Only if last clip update sent to PGPE is larger than legacy turbo,
                // send new clips with legacy turbo values, otherwise, no action needed.
                for (quad = 0; quad < MAXIMUM_QUADS; quad++)
                {
                    if(G_clip_update_parms.ps_val_clip_max[quad] < pclip)
                    {
                        // minimum pclip value corresponds to pstate of maximum frequency
                        pclip = G_clip_update_parms.ps_val_clip_max[quad];
                    }
                }
                l_pstate = proc_freq2pstate(G_sysConfigData.sys_mode_freq.table[OCC_MODE_TURBO]);
                // pclip of highest quad frequency corresponds to a frequency higher than legacy turbo
                if(pclip < l_pstate)
                {
                    if( G_allow_trace_flags & PGPE_ALLOW_CLIP_TRACE )
                    {
                        TRAC_INFO("task_core_data_control: updating clip max to pstate 0x%02X (from 0x%02X)", l_pstate, pclip);
                    }
                    // set the all quads to same pstate
                    memset(G_desired_pstate, l_pstate, MAXIMUM_QUADS);
                    //call PGPE IPC function to update the clips
                    pgpe_clip_update();
                }

                //Whether clips have been lowered from frequencies higher than legacy turbo
                //frequency, or already lower than turbo frequency, OCC is now ready to
                //transition to Observation state.
                G_active_to_observation_ready = true;
            }
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
                        // Update pmcr value (per quad) with requested pstate and version (Version 1 (P9 format))
                        G_pmcr_set_parms.pmcr[quad] = ((uint64_t) G_desired_pstate[quad] << 48) | 1;
                    }

                    // Only send pstates if they changed
                    if (L_last != pstateList)
                    {
                        L_last = pstateList;

                        if( G_allow_trace_flags & PGPE_ALLOW_PMCR_TRACE )
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
                    TRAC_ERR("task_core_data_control: pstate update IPC task did not complete successfully, idle?[%d] rc[%08X]",
                              l_request_is_idle, l_request_rc);

                    err = createErrl(
                                     RTLS_TASK_CORE_DATA_CONTROL_MOD,               //ModId
                                     PGPE_FAILURE,                                  //Reasoncode
                                     l_ext_rc,                                      //Extended reason code
                                     ERRL_SEV_PREDICTIVE,                           //Severity
                                     NULL,                                          //Trace Buf
                                     DEFAULT_TRACE_SIZE,                            //Trace Size
                                     l_request_rc,                                  //Userdata1
                                     l_request_is_idle                              //Userdata2
                                    );

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
