/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/proc/proc_data_control.c $                        */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2017                        */
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
#include "rtls_service_codes.h"
#include "proc_pstate.h"
#include "occ_util.h"

// The the GPE parameter fields for PGPE IPC calls.
extern GPE_BUFFER(ipcmsg_clip_update_t  G_clip_update_parms);
extern GPE_BUFFER(ipcmsg_set_pmcr_t     G_pmcr_set_parms);

extern GpeRequest G_clip_update_req;
extern GpeRequest G_pmcr_set_req;

extern bool G_state_transition_occuring;     // A state transition is currently going on?

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
    static bool     L_trace_logged = false;  // trace logging to avoid unnecessarily repeatig logs
    Pstate          l_pstate;
    static uint64_t L_last = 0xFFFFFFFFFFFFFFFF;

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
                    TRAC_INFO("task_core_data_control: updating clip max to pstate 0x%02X (from 0x%02X)", l_pstate, pclip);
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
    }
    else
    {
        L_trace_logged = false;

        if (G_allowPstates)
        {
            // perform Pstate/clip control if previous IPC call completed successfully
            // if not idle, ignore cycle
            // if an error was returned, log an error, and request reset
            if(G_sysConfigData.system_type.kvm) // OPAL system
            {
                // confirm that the clip update IPC from last cycle
                // has successfully completed on PGPE (with no errors)
                if( async_request_is_idle(&G_clip_update_req.request) &&  //clip_update/set_clip_ranges completed
                    (G_clip_update_parms.msg_cb.rc == PGPE_RC_SUCCESS) ) // with no errors
                {
                    //call PGPE IPC function to update the clips
                    pgpe_clip_update();
                }
                else if(G_clip_update_parms.msg_cb.rc != PGPE_RC_SUCCESS)
                {
                    // an earlier clip update IPC call has not completed, trace and log an error
                    TRAC_ERR("task_core_data_control: clip update IPC task returned an error, %d",
                             G_clip_update_parms.msg_cb.rc);

                    /*
                     * @errortype
                     * @moduleid    RTLS_TASK_CORE_DATA_CONTROL_MOD
                     * @reasoncode  PGPE_FAILURE
                     * @userdata1   rc
                     * @userdata2   clip update task idle?
                     * @userdata4   ERC_PGPE_CLIP_UNSUCCESSFULL
                     * @devdesc     pgpe clip update returned an error
                     */
                    err = createErrl(
                                     RTLS_TASK_CORE_DATA_CONTROL_MOD,                  //ModId
                                     PGPE_FAILURE,                                     //Reasoncode
                                     ERC_PGPE_CLIP_UNSUCCESSFULL,                      //Extended reason code
                                     ERRL_SEV_PREDICTIVE,                              //Severity
                                     NULL,                                             //Trace Buf
                                     DEFAULT_TRACE_SIZE,                               //Trace Size
                                     G_clip_update_parms.msg_cb.rc,                    //Userdata1
                                     async_request_is_idle(&G_clip_update_req.request) //Userdata2
                                    );
                }
            }
            else
            {
                // NON OPAL System, OCC owns PMCR:
                if( async_request_is_idle(&G_pmcr_set_req.request) &&     // PMCR IPC from last TICK completed
                    (G_pmcr_set_parms.msg_cb.rc == PGPE_RC_SUCCESS) )     // with no errors
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
                        TRAC_IMP("task_core_data_control: calling pmcr_set() w/pstates: 0x%08X%04X",
                                 WORD_HIGH(pstateList), WORD_LOW(pstateList)>>16);
                        //call PGPE IPC function to update Pstates
                        pgpe_pmcr_set();
                    }

                }
                else if(G_pmcr_set_parms.msg_cb.rc != PGPE_RC_SUCCESS)
                {
                    // an earlier clip update IPC call has not completed, trace and log an error
                    TRAC_ERR("task_core_data_control: pstate update IPC task returned an error, %d",
                             G_pmcr_set_parms.msg_cb.rc);

                    /*
                     * @errortype
                     * @moduleid    RTLS_TASK_CORE_DATA_CONTROL_MOD
                     * @reasoncode  PGPE_FAILURE
                     * @userdata1   rc
                     * @userdata2   pmcr set task idle?
                     * @userdata4   ERC_PGPE_PMCR_UNSUCCESSFULL
                     * @devdesc     pgpe PMCR set returned an error
                     */
                    err = createErrl(
                                     RTLS_TASK_CORE_DATA_CONTROL_MOD,               //ModId
                                     PGPE_FAILURE,                                  //Reasoncode
                                     ERC_PGPE_PMCR_UNSUCCESSFULL,                   //Extended reason code
                                     ERRL_SEV_PREDICTIVE,                           //Severity
                                     NULL,                                          //Trace Buf
                                     DEFAULT_TRACE_SIZE,                            //Trace Size
                                     G_pmcr_set_parms.msg_cb.rc,                    //Userdata1
                                     async_request_is_idle(&G_pmcr_set_req.request) //Userdata2
                                    );
                }
            }
        }
        // else pstates not allowed yet

        if(err)
        {
            // commit error log
            REQUEST_RESET(err);
        }
    }

    return;
}
