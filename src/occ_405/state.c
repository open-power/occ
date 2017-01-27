/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/state.c $                                         */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2016                        */
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

#include <occ_common.h>
#include <common_types.h>
#include <centaur_data.h>
#include "ssx_io.h"
#include "trac.h"
#include "rtls.h"
#include "state.h"
#include "dcom.h"
#include "occ_service_codes.h"
#include "cmdh_fsp_cmds_datacnfg.h"
#include "cmdh_fsp.h"
#include "proc_data.h"
#include "scom.h"
#include <fir_data_collect.h>
#include <dimm.h>
#include "pgpe_interface.h"
#include "pstate_pgpe_occ_api.h"

extern bool G_mem_monitoring_allowed;
extern task_t G_task_table[TASK_END];  // Global task table
extern bool G_simics_environment;

extern pstateStatus G_proc_pstate_status;

extern GpeRequest G_clip_update_req;
extern GPE_BUFFER(ipcmsg_clip_update_t*  G_clip_update_parms_ptr);

// OCC is ready to transition to observation state?
extern bool G_active_to_observation_ready;

extern PMCR_OWNER G_proc_pmcr_owner;

// State that OCC is currently in
OCC_STATE          G_occ_internal_state     = OCC_STATE_STANDBY;

// State that OCC is requesting that TMGT put OCC into
OCC_STATE          G_occ_internal_req_state = OCC_STATE_NOCHANGE;

// State that TMGT wants to put OCC into
OCC_STATE          G_occ_external_req_state = OCC_STATE_NOCHANGE;

// Indicates if we are currently in a state transition
bool               G_state_transition_occuring = FALSE;

// State that OCC Master is requesting
OCC_STATE          G_occ_master_state       = OCC_STATE_NOCHANGE;

// Semaphore to allow state change to be called from multiple threads
SsxSemaphore G_smgrStateChangeSem;

// Table that indicates which functions should be run for a given mode
// transition.
const smgr_state_trans_t G_smgr_state_trans[] =
{
    /* ----- SPECIFIC CASE STATE TRANSITIONS ----- */
    /* These are specific state transitions for when it matters what
     * state we were in before the transition.  These must come before
     * the agnostic state transitions below, and will be run instead of
     * those catch-all transition functions. */

    /* Current State        New State              Transition Function */
    {OCC_STATE_STANDBY,          OCC_STATE_OBSERVATION,      &SMGR_standby_to_observation},
    {OCC_STATE_STANDBY,          OCC_STATE_CHARACTERIZATION, &SMGR_standby_to_characterization},
    {OCC_STATE_STANDBY,          OCC_STATE_ACTIVE,           &SMGR_standby_to_active},
    {OCC_STATE_OBSERVATION,      OCC_STATE_CHARACTERIZATION, &SMGR_observation_to_characterization},
    {OCC_STATE_OBSERVATION,      OCC_STATE_ACTIVE,           &SMGR_observation_to_active},
    {OCC_STATE_CHARACTERIZATION, OCC_STATE_OBSERVATION,      &SMGR_characterization_to_observation},
    {OCC_STATE_CHARACTERIZATION, OCC_STATE_ACTIVE,           &SMGR_characterization_to_active},
    {OCC_STATE_ACTIVE,           OCC_STATE_OBSERVATION,      &SMGR_active_to_observation},
    {OCC_STATE_ACTIVE,           OCC_STATE_CHARACTERIZATION, &SMGR_active_to_characterization},

    /* ----- DEFAULT STATE TRANSITIONS ----- */
    /* These are default state transitions for when it doesn't matter what
     * state we were in before the transition. */

    /* Current State        New State              Transition Function */
    {OCC_STATE_ALL,         OCC_STATE_STANDBY,     &SMGR_all_to_standby},
    {OCC_STATE_ALL,         OCC_STATE_SAFE,        &SMGR_all_to_safe},
};

const uint8_t G_smgr_state_trans_count = sizeof(G_smgr_state_trans)/sizeof(smgr_state_trans_t);


uint32_t G_smgr_validate_data_active_mask = SMGR_VALIDATE_DATA_ACTIVE_MASK_HARDCODES;
uint32_t G_smgr_validate_data_observation_mask = SMGR_VALIDATE_DATA_OBSERVATION_MASK_HARDCODES;

// Function Specification
//
// Name: SMGR_is_mode_transitioning
//
// Description:
//
// End Function Specification
inline bool SMGR_is_state_transitioning(void)
{
   return G_state_transition_occuring;
}


// Function Specification
//
// Name:
//
// Description:
//
// End Function Specification
errlHndl_t SMGR_standby_to_observation()
{
    errlHndl_t              l_errlHndl = NULL;
    static bool l_error_logged = FALSE;  // To prevent trace and error log happened over and over

    if( SMGR_MASK_OBSERVATION_READY ==
        (SMGR_validate_get_valid_states() & SMGR_MASK_OBSERVATION_READY))
    {
        l_error_logged = FALSE;
        TRAC_IMP("SMGR: Standby to Observation Transition Started");

        memory_init();

        // Set the RTL Flags to indicate which tasks can run
        //   - Set OBSERVATION b/c we're in OBSERVATION State
        rtl_clr_run_mask_deferred(RTL_FLAG_STANDBY);
        rtl_set_run_mask_deferred(RTL_FLAG_OBS);

        // Set the actual STATE now that we have finished everything else
        CURRENT_STATE() = OCC_STATE_OBSERVATION;

        TRAC_IMP("SMGR: Standby to Observation Transition Completed");

    }
    else if(FALSE == l_error_logged)
    {
        l_error_logged = TRUE;
        TRAC_ERR("SMGR: Standby to Observation Transition Failed");
        /* @
         * @errortype
         * @moduleid    MAIN_STATE_TRANSITION_MID
         * @reasoncode  INTERNAL_FAILURE
         * @userdata1   none
         * @userdata4   ERC_STATE_FROM_STB_TO_OBS_FAILURE
         * @devdesc     Failed changing from standby to observation
         */
        l_errlHndl = createErrl(MAIN_STATE_TRANSITION_MID,        //modId
                                INTERNAL_FAILURE,                 //reasoncode
                                ERC_STATE_FROM_STB_TO_OBS_FAILURE,//Extended reason code
                                ERRL_SEV_UNRECOVERABLE,           //Severity
                                NULL,                             //Trace Buf
                                DEFAULT_TRACE_SIZE,               //Trace Size
                                0,                                //userdata1
                                0);                               //userdata2

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
// Name: SMGR_standby_to_characterization
//
// Description: switch from standby state to characterization state
//
// End Function Specification
errlHndl_t SMGR_standby_to_characterization()
{
    errlHndl_t  l_errlHndl = NULL;
    int         rc = 0;
    static bool l_error_logged = FALSE;  // To prevent trace and error log happened over and over

    do
    {

        if( SMGR_MASK_ACTIVE_READY ==
            (SMGR_validate_get_valid_states() & SMGR_MASK_ACTIVE_READY))
        {
            l_error_logged = FALSE;
            TRAC_IMP("SMGR: Standby to Characterization State Transition Started");

            // wide open pstate clips
            rc = pgpe_widen_clip_blocking(OCC_STATE_CHARACTERIZATION);

            if(rc)
            {
                TRAC_ERR("SMGR: failed to widen pstate clips.");
                break;
            }
            else // successfully wide opened clips; enable pstates, then start transition
            {

                // update OPAL static table in main memory
                if(G_sysConfigData.system_type.kvm)
                {
                    // upon succesful enablement of Pstate protocol on
                    // PGPE update OPAL table with pstate information.
                    proc_pstate_kvm_setup();
                }

                // Start pstates on PGPE and set Characterization as owner
                rc = pgpe_start_suspend(PGPE_ACTION_PSTATE_START, PMCR_OWNER_CHAR);

                if(rc)
                {
                    TRAC_ERR("SMGR: failed to start the pstate protocol on PGPE.");
                    break;
                }
                else // Clips wide opened and pstates started successfully, start transition
                {
                    memory_init();

                    // Set the RTL Flags to indicate which tasks can run
                    //   - Set OBSERVATION flags in Characterization State
                    rtl_clr_run_mask_deferred(RTL_FLAG_STANDBY);
                    rtl_set_run_mask_deferred(RTL_FLAG_OBS);

                    // Set the actual STATE now that we have finished everything else
                    CURRENT_STATE() = OCC_STATE_CHARACTERIZATION;

                    TRAC_IMP("SMGR: Standby to Characterization Transition Completed");
                }
            }
        }
    } while (0);

    if(l_errlHndl && (false == l_error_logged))
    {
        l_error_logged = TRUE;
        TRAC_ERR("SMGR: Standby to Characterization Transition Failed");
        /* @
         * @errortype
         * @moduleid    MAIN_STATE_TRANSITION_MID
         * @reasoncode  INTERNAL_FAILURE
         * @userdata1   none
         * @userdata4   ERC_STATE_FROM_STB_TO_CHR_FAILURE
         * @devdesc     Failed changing from standby to characterization
         */
        l_errlHndl = createErrl(MAIN_STATE_TRANSITION_MID,          //modId
                                INTERNAL_FAILURE,                   //reasoncode
                                ERC_STATE_FROM_STB_TO_CHR_FAILURE,  //Extended reason code
                                ERRL_SEV_UNRECOVERABLE,             //Severity
                                NULL,                               //Trace Buf
                                DEFAULT_TRACE_SIZE,                 //Trace Size
                                0,                                  //userdata1
                                0);                                 //userdata2

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
// Name: SMGR_all_to_standby
//
// Description: Switch from any state to standby state
//
// End Function Specification

#define WAIT_PGPE_TASK_TIMEOUT 200         // maximum time to wait for a PGPE task before timeout
errlHndl_t SMGR_all_to_standby()
{
    errlHndl_t  l_errlHndl     = NULL;
    static bool l_error_logged = FALSE;  // To prevent trace and error logging over and over
    uint8_t     wait_time = 0;
    int         rc;

    TRAC_IMP("SMGR: Transition from State (%d) to Standby Started",
             CURRENT_STATE());

    do
    {

        // if Psates in transition (a pgpe_start_suspend IPC call still running),
        // wait until it is settled up to WAIT_PGPE_TASK_TIMEOUT usec
        while(G_proc_pstate_status == PSTATES_IN_TRANSITION &&
              wait_time < WAIT_PGPE_TASK_TIMEOUT)
        {
            // wait until pgpe_start_suspend call is completed. Sleep enables context switching.
            ssx_sleep(SSX_MICROSECONDS(10));
            wait_time += 10;
        }

        // check for timeout while waiting for pgpe_start_suspend() IPC completion
        if(wait_time >= WAIT_PGPE_TASK_TIMEOUT)
        {
            TRAC_ERR("SMGR: Timeout waiting for Pstates start/suspend IPC task");

            /* @
             * @errortype
             * @moduleid    MAIN_STATE_TRANSITION_MID
             * @reasoncode  GPE_REQUEST_TASK_TIMEOUT
             * @userdata1   wait_time
             * @userdata4   OCC_NO_EXTENDED_RC
             * @devdesc     timeout waiting for pstates start/suspend task
             */
            l_errlHndl = createErrl(MAIN_STATE_TRANSITION_MID,          //modId
                                    GPE_REQUEST_TASK_TIMEOUT,           //reasoncode
                                    OCC_NO_EXTENDED_RC,                 //Extended reason code
                                    ERRL_SEV_UNRECOVERABLE,             //Severity
                                    NULL,                               //Trace Buf
                                    DEFAULT_TRACE_SIZE,                 //Trace Size
                                    wait_time,                          //userdata1
                                    0);                                 //userdata2

            // Callout firmware
            addCalloutToErrl(l_errlHndl,
                             ERRL_CALLOUT_TYPE_COMPONENT_ID,
                             ERRL_COMPONENT_ID_FIRMWARE,
                             ERRL_CALLOUT_PRIORITY_HIGH);
            break;
        }

        // Stop Pstates if enabled
        if(G_proc_pstate_status == PSTATES_ENABLED)
        {
            rc = pgpe_start_suspend(PGPE_ACTION_PSTATE_STOP, G_proc_pmcr_owner);
            if(rc)
            {
                TRAC_ERR("SMGR: Failed to stop the pstate protocol on PGPE.");
                break;
            }
        }

        // Pstates Disabled, ready to safely transition to standby

        // Set the RTL Flags to indicate which tasks can run
        //   - Clear ACTIVE b/c not in ACTIVE State
        //   - Clear OBSERVATION b/c not in CHARACTERIZATION State
        rtl_clr_run_mask_deferred(RTL_FLAG_ACTIVE | RTL_FLAG_OBS );
        rtl_set_run_mask_deferred(RTL_FLAG_STANDBY);

        // Set the actual STATE now that we have finished everything else
        CURRENT_STATE() = OCC_STATE_STANDBY;

        TRAC_IMP("SMGR: Transition to Standby Completed");

    } while (0);

    if(l_errlHndl)
    {
        l_error_logged = TRUE;
        TRAC_ERR("SMGR: Transition to Standby Failed");

        /* @
         * @errortype
         * @moduleid    MAIN_STATE_TRANSITION_MID
         * @reasoncode  INTERNAL_FAILURE
         * @userdata1   starting state
         * @userdata4   ERC_STATE_FROM_ALL_TO_STB_FAILURE
         * @devdesc     Failed changing from observation to characterization
         */
        l_errlHndl = createErrl(MAIN_STATE_TRANSITION_MID,          //modId
                                INTERNAL_FAILURE,                   //reasoncode
                                ERC_STATE_FROM_ALL_TO_STB_FAILURE,  //Extended reason code
                                ERRL_SEV_UNRECOVERABLE,             //Severity
                                NULL,                               //Trace Buf
                                DEFAULT_TRACE_SIZE,                 //Trace Size
                                CURRENT_STATE(),                    //userdata1
                                0);                                 //userdata2

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
// Name: SMGR_characterization_to_observation
//
// Description: Switch from characterization to observation state
//
// End Function Specification
errlHndl_t SMGR_characterization_to_observation()
{
    errlHndl_t  l_errlHndl     = NULL;
    int         rc             = 0;
    static bool l_error_logged = FALSE;  // To prevent trace and error logging over and over

    TRAC_IMP("SMGR: Characterization to Observation Transition Started");

    do
    {
        // widen clips to legacy turbo
        rc = pgpe_widen_clip_blocking(OCC_STATE_OBSERVATION);

        if(rc)
        {
            TRAC_ERR("SMGR: failed to tighten pstate clips.");
            break;
        }
        else // clips set to legacy turbo; stop pstate protocol
        {
            rc = pgpe_start_suspend(PGPE_ACTION_PSTATE_STOP, G_proc_pmcr_owner);
            if(rc)
            {
                TRAC_ERR("SMGR: Failed to stop the pstate protocol on PGPE.");
                break;
            }
            else // Clips tightened successfully, and pstates disabled: perform transition
            {
                // No RTL Flag changes; only state change
                CURRENT_STATE() = OCC_STATE_OBSERVATION;

                TRAC_IMP("SMGR: Characterization to Observation Transition Completed");
            }
        }
    } while (0);

    if(rc)
    {
        l_error_logged = TRUE;
        TRAC_ERR("SMGR: Characterization to Observation Transition Failed");

        /* @
         * @errortype
         * @moduleid    MAIN_STATE_TRANSITION_MID
         * @reasoncode  INTERNAL_FAILURE
         * @userdata1   none
         * @userdata4   ERC_STATE_FROM_CHR_TO_OBS_FAILURE
         * @devdesc     Failed changing from observation to characterization
         */
        l_errlHndl = createErrl(MAIN_STATE_TRANSITION_MID,          //modId
                                INTERNAL_FAILURE,                   //reasoncode
                                ERC_STATE_FROM_CHR_TO_OBS_FAILURE,  //Extended reason code
                                ERRL_SEV_UNRECOVERABLE,             //Severity
                                NULL,                               //Trace Buf
                                DEFAULT_TRACE_SIZE,                 //Trace Size
                                0,                                  //userdata1
                                0);                                 //userdata2

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
// Name: SMGR_observation_to_characterization
//
// Description: Switch from Observation to Characterization state
//
// End Function Specification
errlHndl_t SMGR_observation_to_characterization()
{
    int         rc             = 0;
    errlHndl_t  l_errlHndl     = NULL;
    static bool l_error_logged = FALSE;  // To prevent trace and error logging over and over

    TRAC_IMP("SMGR: Observation to Characterization Transition Started");

    // no change in RTL flags, just turn-on pstate protocol, and wide open clips

    do
    {
        // Must be active ready if transitioning to characterization state
        if( (SMGR_MASK_ACTIVE_READY !=
             (SMGR_validate_get_valid_states() & SMGR_MASK_ACTIVE_READY)) )
        {
            TRAC_ERR("SMGR: failed to transition to characterization state "
                     "since OCC is not active ready.");
            break;
        }
        // wide open pstate clips
        rc = pgpe_widen_clip_blocking(OCC_STATE_CHARACTERIZATION);

        if(rc)
        {
            TRAC_ERR("SMGR: failed to widen pstate clips.");
            break;
        }
        else // successfully wide opened clips; enable pstates, then start transition
        {
            // update OPAL static table in main memory
            if(G_sysConfigData.system_type.kvm)
            {
                // upon succesful enablement of Pstate protocol on
                // PGPE update OPAL table with pstate information.
                proc_pstate_kvm_setup();
            }

            // Start pstates on PGPE and set Characterization as owner
            rc = pgpe_start_suspend(PGPE_ACTION_PSTATE_START, PMCR_OWNER_CHAR);

            if(rc)
            {
                TRAC_ERR("SMGR: failed to start the pstate protocol on PGPE.");
                break;
            }
            else // Clips wide opened successfully and pstates enabled; complete transition
            {
                // Set the actual STATE now that we have finished everything else
                CURRENT_STATE() = OCC_STATE_CHARACTERIZATION;

                TRAC_IMP("SMGR: Observation to Characterization Transition Completed");
            }
        }
    } while (0);

    if(rc && (false == l_error_logged))
    {
        l_error_logged = TRUE;
        TRAC_ERR("SMGR: Observation to Characterization Transition Failed");
        /* @
         * @errortype
         * @moduleid    MAIN_STATE_TRANSITION_MID
         * @reasoncode  INTERNAL_FAILURE
         * @userdata1   none
         * @userdata4   ERC_STATE_FROM_OBS_TO_CHR_FAILURE
         * @devdesc     Failed changing from observation to characterization
         */
        l_errlHndl = createErrl(MAIN_STATE_TRANSITION_MID,          //modId
                                INTERNAL_FAILURE,                   //reasoncode
                                ERC_STATE_FROM_OBS_TO_CHR_FAILURE,  //Extended reason code
                                ERRL_SEV_UNRECOVERABLE,             //Severity
                                NULL,                               //Trace Buf
                                DEFAULT_TRACE_SIZE,                 //Trace Size
                                0,                                  //userdata1
                                0);                                 //userdata2

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
// Name: SMGR_observation_to_active
//
// Description: Transition from Observation state to Active state
//
// End Function Specification
errlHndl_t SMGR_observation_to_active()
{
    errlHndl_t      l_errlHndl = NULL;
    static bool l_error_logged = FALSE;  // To prevent trace and error log happened over and over
    int                l_extRc = OCC_NO_EXTENDED_RC;
    int                l_rc = 0;
    do
    {
        // NOTE that this is really unnecessary if you follow the TMGT OCC
        // Interface Spec, which tells you that you need to check for the "Active
        // Ready" bit in the poll response before you go to active state.
        // But since we have scenerios where TMGT isn't the one putting us in
        // active state (we are going there automatically) we needed to add this.

        // If we have all data we need to go to active state, but don't have pstates
        // enabled yet...then we will do the aforementioned wait
        if(SMGR_MASK_ACTIVE_READY ==
           (SMGR_validate_get_valid_states() & SMGR_MASK_ACTIVE_READY))
        {
            l_rc = pgpe_widen_clip_blocking(OCC_STATE_ACTIVE);

            if(l_rc)
            {
                TRAC_ERR("SMGR: Failed to switch to Active state because of a "
                         "failure to widen clip pstates");
                break;
            }

            else // Clips wide opened with no errors, enable Pstates on PGPE
            {

                // Pstates are enabled via an IPC call to PGPE, which will set the
                // G_proc_pstate_status flag. PMCR owner is set based on system type.
                if(G_sysConfigData.system_type.kvm)
                {
                    l_rc = pgpe_start_suspend(PGPE_ACTION_PSTATE_START, PMCR_OWNER_HOST);
                }
                else
                {
                    l_rc = pgpe_start_suspend(PGPE_ACTION_PSTATE_START, PMCR_OWNER_OCC);
                }
                if(l_rc)
                {
                    TRAC_ERR("SMGR: Failed to switch to Active state because of a "
                             "failure to start the pstate protocol on PGPE.");
                    break;
                }
            }

            // Wait for pstates enablement completition.
            SsxTimebase start = ssx_timebase_get();
            while( ! proc_is_hwpstate_enabled() )
            {
                SsxInterval timeout =  SSX_SECONDS(5);
                if ((ssx_timebase_get() - start) > timeout)
                {
                    l_rc = 1;
                    if(FALSE == l_error_logged)
                    {
                        TRAC_ERR("SMGR: Timeout waiting for Pstates to be enabled, "
                                 "pmc_mode[%08x], chips_present[%02x], Cores Present [%08x]",
                                 in32(PMC_MODE_REG),
                                 G_sysConfigData.is_occ_present,
                                 (uint32_t) ((in64(OCB_CCSR)) >> 32));
                    }
                    l_extRc = ERC_GENERIC_TIMEOUT;
                    break;
                }
            }

            // if pstates are now enabled, all conditions are already met
            // to transition to active state.
            if(proc_is_hwpstate_enabled() )
            {
                l_error_logged = FALSE;
                TRAC_IMP("SMGR: Observation to Active Transition Started");

                // Set the RTL Flags to indicate which tasks can run
                //   - Clear OBSERVATION b/c not in OBSERVATION State
                //   - Set ACTIVE b/c we're in ACTIVE State
                rtl_clr_run_mask_deferred(RTL_FLAG_OBS);
                rtl_set_run_mask_deferred(RTL_FLAG_ACTIVE);

                // Pstates enabled, update OPAL static table in main memory
                if(G_sysConfigData.system_type.kvm)
                {
                    TRAC_IMP("SMGR: Pstates are enabled, continuing with state trans");

                    // upon succesful enablement of Pstate protocol on
                    // PGPE update OPAL table with pstate information.
                    proc_pstate_kvm_setup();
                }

                // Set the actual STATE now that we have finished everything else
                CURRENT_STATE() = OCC_STATE_ACTIVE;
                TRAC_IMP("SMGR: Observation to Active Transition Completed");
            }
            else
            {
                TRAC_ERR("SMGR: Observation to Active Transition Failed, because pstates are not enabled");
            }

            if(l_rc && FALSE == l_error_logged)
            {
                l_error_logged = TRUE;
                /* @
                 * @errortype
                 * @moduleid    MAIN_STATE_TRANSITION_MID
                 * @reasoncode  INTERNAL_FAILURE
                 * @userdata1   SMGR_MASK_ACTIVE_READY
                 * @userdata2   valid states
                 * @userdata4   ERC_STATE_FROM_OBS_TO_STB_FAILURE
                 * @devdesc     Failed changing from observation to standby
                 */
                l_errlHndl = createErrl(MAIN_STATE_TRANSITION_MID,        //modId
                                        INTERNAL_FAILURE,                 //reasoncode
                                        l_extRc,                          //Extended reason code
                                        ERRL_SEV_UNRECOVERABLE,           //Severity
                                        NULL,                             //Trace Buf
                                        DEFAULT_TRACE_SIZE,               //Trace Size
                                        SMGR_MASK_ACTIVE_READY,           //userdata1
                                        SMGR_validate_get_valid_states());//userdata2

                // Callout firmware
                addCalloutToErrl(l_errlHndl,
                                 ERRL_CALLOUT_TYPE_COMPONENT_ID,
                                 ERRL_COMPONENT_ID_FIRMWARE,
                                 ERRL_CALLOUT_PRIORITY_HIGH);
            }
        }   // Active Ready
        else
        {
            TRAC_ERR("SMGR: Observation to Active Transition Failed, "
                     "OCC is not Active Ready cnfgdata=0x%08x, reqd=0x%08x",
                     DATA_get_present_cnfgdata(),
                     SMGR_VALIDATE_DATA_ACTIVE_MASK);
        }
    } while (0);


    return l_errlHndl;
}

// Function Specification
//
// Name: SMGR_characterization_to_active
//
// Description: Transition from Characterization state to Active state
//
// End Function Specification
errlHndl_t SMGR_characterization_to_active()
{
    int         rc         = 0;
    errlHndl_t  l_errlHndl = NULL;
    static bool l_error_logged = FALSE;  // To prevent trace and error log happened over and over

    do
    {
        // change PMCR ownership via an IPC call to PGPE based on system type.
        if(G_sysConfigData.system_type.kvm)
        {
            rc = pgpe_start_suspend(PGPE_ACTION_PSTATE_START, PMCR_OWNER_HOST);
        }
        else
        {
            rc = pgpe_start_suspend(PGPE_ACTION_PSTATE_START, PMCR_OWNER_OCC);
        }
        if(rc)
        {
            TRAC_ERR("SMGR: Failed to change PMCR ownership on PGPE.");
            break;
        }

        // Have all data we need to go to active state
        if(((DATA_get_present_cnfgdata() & SMGR_VALIDATE_DATA_ACTIVE_MASK) ==
            SMGR_VALIDATE_DATA_ACTIVE_MASK)   &&
           // and Psate protocol is enabled
           proc_is_hwpstate_enabled() && G_sysConfigData.system_type.kvm)
        {
            TRAC_IMP("SMGR: Pstates are enabled, continuing with state trans");
        }

        // Check if all conditions are met to transition to active state.  If
        // they aren't, then log an error and fail to change state.
        if( (SMGR_MASK_ACTIVE_READY ==
             (SMGR_validate_get_valid_states() & SMGR_MASK_ACTIVE_READY))
            && proc_is_hwpstate_enabled() )
        {
            l_error_logged = FALSE;
            TRAC_IMP("SMGR: Characterization to Active Transition Started");

            // Set the RTL Flags to indicate which tasks can run
            //   - Clear OBSERVATION b/c not in CHARACTERIZATION State
            //   - Set ACTIVE b/c we're in ACTIVE State
            rtl_clr_run_mask_deferred(RTL_FLAG_OBS);
            rtl_set_run_mask_deferred(RTL_FLAG_ACTIVE);

            // Set the actual STATE now that we have finished everything else
            CURRENT_STATE() = OCC_STATE_ACTIVE;
            TRAC_IMP("SMGR: Characterization to Active Transition Completed");
        }
    } while (0);

    if(rc && (false == l_error_logged))
    {
        l_error_logged = TRUE;
        /* @
         * @errortype
         * @moduleid    MAIN_STATE_TRANSITION_MID
         * @reasoncode  INTERNAL_FAILURE
         * @userdata1   SMGR_MASK_ACTIVE_READY
         * @userdata2   valid states
         * @userdata4   ERC_STATE_FROM_CHR_TO_ACT_FAILURE
         * @devdesc     Failed changing from characterization to active
         */
        l_errlHndl = createErrl(MAIN_STATE_TRANSITION_MID,           //modId
                                INTERNAL_FAILURE,                    //reasoncode
                                ERC_STATE_FROM_CHR_TO_ACT_FAILURE,   //Extended reason code
                                ERRL_SEV_UNRECOVERABLE,              //Severity
                                NULL,                                //Trace Buf
                                DEFAULT_TRACE_SIZE,                  //Trace Size
                                SMGR_MASK_ACTIVE_READY,              //userdata1
                                SMGR_validate_get_valid_states());   //userdata2

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
// Name:  SMGR_active_to_observation
//
// Description:
//
// End Function Specification

errlHndl_t SMGR_active_to_observation()
{
    int              rc         = 0;
    errlHndl_t       l_errlHndl = NULL;
    uint8_t          wait_time  = 0;

    TRAC_IMP("SMGR: Active to Observation Transition Started");

    do
    {
        // wait until task_core_data_control() declares that
        // OCC is ready to transition to Observation state
        while(!G_active_to_observation_ready)
        {
            if(wait_time > WAIT_PGPE_TASK_TIMEOUT)
            {
                break;
            }

            // Sleep enables context switching.
            ssx_sleep(SSX_MICROSECONDS(10));
            wait_time += 10;
        }

        // check for timeout while waiting for pgpe_start_suspend() IPC completion
        if(wait_time > WAIT_PGPE_TASK_TIMEOUT)
        {
            TRAC_ERR("SMGR: Timeout waiting for G_active_to_observation_ready flag.");

            /* @
             * @errortype
             * @moduleid    MAIN_STATE_TRANSITION_MID
             * @reasoncode  PGPE_FAILURE
             * @userdata1   wait_time
             * @userdata4   ERC_PGPE_ACTIVE_TO_OBSERVATION_TIMEOUT
             * @devdesc     timeout waiting for pstates start/suspend task
             */
            l_errlHndl = createErrl(MAIN_STATE_TRANSITION_MID,              //modId
                                    PGPE_FAILURE,                           //reasoncode
                                    ERC_PGPE_ACTIVE_TO_OBSERVATION_TIMEOUT, //Extended reason code
                                    ERRL_SEV_UNRECOVERABLE,                 //Severity
                                    NULL,                                   //Trace Buf
                                    DEFAULT_TRACE_SIZE,                     //Trace Size
                                    wait_time,                              //userdata1
                                    0);                                     //userdata2

            // Callout firmware
            addCalloutToErrl(l_errlHndl,
                             ERRL_CALLOUT_TYPE_COMPONENT_ID,
                             ERRL_COMPONENT_ID_FIRMWARE,
                             ERRL_CALLOUT_PRIORITY_HIGH);

            rc = PGPE_FAILURE;
            break;
        }

        wait_time = 0;

        // wait until clip update task is done
        while(!async_request_is_idle(&G_clip_update_req.request))
        {
            if(wait_time > WAIT_PGPE_TASK_TIMEOUT)
            {
                break;
            }

            // wait until pgpe_start_suspend call is completed.
            // Sleep enables context switching.
            ssx_sleep(SSX_MICROSECONDS(10));
            wait_time += 10;
        }

        // check for timeout while waiting for pgpe_start_suspend() IPC completion
        if(wait_time > WAIT_PGPE_TASK_TIMEOUT)
        {
            TRAC_ERR("SMGR: Timeout waiting for clip update IPC task");

            /* @
             * @errortype
             * @moduleid    MAIN_STATE_TRANSITION_MID
             * @reasoncode  PGPE_FAILURE
             * @userdata1   wait_time
             * @userdata4   ERC_PGPE_TASK_TIMEOUT
             * @devdesc     timeout waiting for pstates start/suspend task
             */
            l_errlHndl = createErrl(MAIN_STATE_TRANSITION_MID,          //modId
                                    PGPE_FAILURE,                       //reasoncode
                                    ERC_PGPE_TASK_TIMEOUT,              //Extended reason code
                                    ERRL_SEV_UNRECOVERABLE,             //Severity
                                    NULL,                               //Trace Buf
                                    DEFAULT_TRACE_SIZE,                 //Trace Size
                                    wait_time,                          //userdata1
                                    0);                                 //userdata2

            // Callout firmware
            addCalloutToErrl(l_errlHndl,
                             ERRL_CALLOUT_TYPE_COMPONENT_ID,
                             ERRL_COMPONENT_ID_FIRMWARE,
                             ERRL_CALLOUT_PRIORITY_HIGH);

            rc = PGPE_FAILURE;
            break;
        }

        rc = pgpe_start_suspend(PGPE_ACTION_PSTATE_STOP, G_proc_pmcr_owner);
        if(rc)
        {
            TRAC_ERR("SMGR: failed to stop the pstate protocol on PGPE.");
            break;
        }
        else // Pstates Disabled and clips wide opened successfully, perform state transition
        {
            // Set the RTL Flags to indicate which tasks can run
            //   - Set OBSERVATION b/c in OBSERVATION State
            //   - Clear ACTIVE b/c not in ACTIVE State
            rtl_clr_run_mask_deferred(RTL_FLAG_ACTIVE);
            rtl_set_run_mask_deferred(RTL_FLAG_OBS);

            // Set the actual STATE now that we have finished everything else
            CURRENT_STATE() = OCC_STATE_OBSERVATION;

            // clear G_active_to_observation_ready flag again
            // (for next active_to_observation transition)
            G_active_to_observation_ready = false;
        }

    } while (0);

    if(rc)
    {
        TRAC_ERR("SMGR: Failed to switch to Observation state");
    }
    else
    {
        // Pstates disabled, update OPAL static table in main memory
        if(G_sysConfigData.system_type.kvm)
        {
            // upon succesful enablement of Pstate protocol on
            // PGPE update OPAL table with pstate information.
            proc_pstate_kvm_setup();
        }

        TRAC_IMP("SMGR: Active to Observation Transition Completed");
    }

    return l_errlHndl;
}


// Function Specification
//
// Name:  SMGR_active_to_characterization
//
// Description: Switch from Active to characterization state
//
// End Function Specification
errlHndl_t SMGR_active_to_characterization()
{
    int          rc         = 0;
    errlHndl_t   l_errlHndl = NULL;

    TRAC_IMP("SMGR: Active to Characterization Transition Started");

    do
    {
        // open wide pstate clips
        rc = pgpe_widen_clip_blocking(OCC_STATE_CHARACTERIZATION);

        if(rc)
        {
            TRAC_ERR("SMGR: failed to widen pstate clips.");
            break;
        }
        else // clips widened successfully, keep pstates enabled, but change ownership
        {
            rc = pgpe_start_suspend(PGPE_ACTION_PSTATE_START, PMCR_OWNER_CHAR);

            if(rc)
            {
                TRAC_ERR("SMGR: failed to change PMCR ownership.");
                break;
            }
            else // Pstates Disabled and clips wide opened successfully, perform state transition
            {
                // Set the RTL Flags to indicate which tasks can run
                //   - Set OBSERVATION RTL flags for Characterization State
                //   - Clear ACTIVE b/c not in ACTIVE State
                rtl_clr_run_mask_deferred(RTL_FLAG_ACTIVE);
                rtl_set_run_mask_deferred(RTL_FLAG_OBS);

                // Set the actual STATE now that we have finished everything else
                CURRENT_STATE() = OCC_STATE_CHARACTERIZATION;
            }
        }
    } while (0);

    if(rc)
    {
        TRAC_ERR("SMGR: Failed to switch to Characterization state");
        /* @
         * @errortype
         * @moduleid    MAIN_STATE_TRANSITION_MID
         * @reasoncode  INTERNAL_FAILURE
         * @userdata1   rc
         * @userdata4   ERC_STATE_FROM_ACT_TO_CHR_FAILURE
         * @devdesc     Failed changing from standby to observation
         */
        l_errlHndl = createErrl(MAIN_STATE_TRANSITION_MID,        //modId
                                INTERNAL_FAILURE,                 //reasoncode
                                ERC_STATE_FROM_ACT_TO_CHR_FAILURE,//Extended reason code
                                ERRL_SEV_UNRECOVERABLE,           //Severity
                                NULL,                             //Trace Buf
                                DEFAULT_TRACE_SIZE,               //Trace Size
                                rc,                                //userdata1
                                0);                               //userdata2

        // Callout firmware
        addCalloutToErrl(l_errlHndl,
                         ERRL_CALLOUT_TYPE_COMPONENT_ID,
                         ERRL_COMPONENT_ID_FIRMWARE,
                         ERRL_CALLOUT_PRIORITY_HIGH);
    }
    else
    {
        // Pstates disabled, update OPAL static table in main memory
        if(G_sysConfigData.system_type.kvm)
        {
            // upon succesful enablement of Pstate protocol on
            // PGPE update OPAL table with pstate information.
            proc_pstate_kvm_setup();
        }

        TRAC_IMP("SMGR: Active to Characterization Transition Completed");
    }

    return l_errlHndl;
}


// Function Specification
//
// Name:  SMGR_standby_to_active
//
// Description: Makes the appropriate calls when transitioning from
//              standby to active.
//
// End Function Specification
errlHndl_t SMGR_standby_to_active()
{
    errlHndl_t              l_errlHndl = NULL;

    TRAC_IMP("SMGR: Standby to Active Transition Started");

    // Transtion through both functions on this multi-state transtion
    l_errlHndl = SMGR_standby_to_observation();
    if(NULL == l_errlHndl)
    {
       l_errlHndl = SMGR_observation_to_active();
    }

    if(l_errlHndl)
    {
        TRAC_ERR("SMGR: Standby to Active Transition Failed");
    }
    else
    {
        TRAC_IMP("SMGR: Standby to Active Transition Completed");
    }

    return l_errlHndl;
}


// Function Specification
//
// Name:  SMGR_all_to_safe
//
// Description:
//
// End Function Specification
errlHndl_t SMGR_all_to_safe()
{
    errlHndl_t              l_errlHndl = NULL;

    TRAC_IMP("SMGR: All States to Safe Transition Started");

    // If we are master, make sure we are broadcasting that the requested
    // state is "safe state"
    if (OCC_MASTER == G_occ_role)
    {
       G_occ_external_req_state = OCC_STATE_SAFE;
    }

    // If we are master, we will wait 15ms to go to full on safe mode
    // This is to give the slaves time to see that we are broadcasting
    // Safe State, and react to it.  This must be > 10ms.  It doesn't hurt
    // to do this on the slaves also.
    ssx_sleep(SSX_MILLISECONDS(15));

    // Set the RTL Flags to indicate which tasks can run
    //   - Clear ACTIVE b/c not in ACTIVE State
    //   - Clear OBSERVATION b/c not in OBSERVATION State
    //   - Clear STANDBY b/c not in STANDBY State
    rtl_clr_run_mask_deferred(RTL_FLAG_ACTIVE | RTL_FLAG_OBS | RTL_FLAG_STANDBY);

    // Only reset task & wdt task will run from now on
    // !!! There is no recovery to this except a reset !!!
    rtl_set_run_mask_deferred(RTL_FLAG_RST_REQ);

    // Notes:
    //  - We can still talk to FSP
    //  - We will still be able to go out on PIB
    //  - We will still attempt to load an applet if told to do so
    //  - The master will no longer broadcast to slaves, and vice versa

    // Set the actual STATE now that we have finished everything else
    CURRENT_STATE() = OCC_STATE_SAFE;

    TRAC_IMP("SMGR: All States to Safe Transition Completed");

    return l_errlHndl;
}


// Function Specification
//
// Name:  SMGR_set_state
//
// Description:
//
// End Function Specification
errlHndl_t SMGR_set_state(OCC_STATE i_new_state)
{
    errlHndl_t l_transResult = NULL; //Was the transition successful?
    int jj=0;

    do
    {
        // Get lock for critical section
        if(ssx_semaphore_pend(&G_smgrStateChangeSem,SSX_WAIT_FOREVER))
        {
            /* @
             * @errortype
             * @moduleid    MAIN_STATE_TRANSITION_MID
             * @reasoncode  SSX_GENERIC_FAILURE
             * @userdata1   none
             * @userdata4   ERC_RUNNING_SEM_PENDING_FAILURE
             * @devdesc     SSX semaphore related failure
             */
            l_transResult = createErrl(MAIN_STATE_TRANSITION_MID,        //modId
                                       SSX_GENERIC_FAILURE,              //reasoncode
                                       ERC_RUNNING_SEM_PENDING_FAILURE,  //Extended reason code
                                       ERRL_SEV_UNRECOVERABLE,           //Severity
                                       NULL,                             //Trace Buf
                                       DEFAULT_TRACE_SIZE,               //Trace Size
                                       0,                                //userdata1
                                       0);                               //userdata2

            addCalloutToErrl(l_transResult,
                             ERRL_CALLOUT_TYPE_COMPONENT_ID,
                             ERRL_COMPONENT_ID_FIRMWARE,
                             ERRL_CALLOUT_PRIORITY_HIGH);

            break;
        }

        //If there is no change to the state, just exit
        if(i_new_state == OCC_STATE_NOCHANGE)
        {
            break;
        }

        // Loop through state transition table, and find the state
        // transition function that matches the transition we need to do.
        for(jj=0; jj<G_smgr_state_trans_count; jj++)
        {
            if(((G_smgr_state_trans[jj].old_state == G_occ_internal_state)
                ||
                (G_smgr_state_trans[jj].old_state == OCC_STATE_ALL))
               &&
               (G_smgr_state_trans[jj].new_state == i_new_state))
            {
                // We found the transtion that matches, now run the function
                // that is associated with that state transition.
                if(NULL != G_smgr_state_trans[jj].trans_func_ptr)
                {
                    // Signal that we are now in a state transition
                    G_state_transition_occuring = TRUE;
                    // Run transition function

                    l_transResult = (G_smgr_state_trans[jj].trans_func_ptr)();


                    // Signal that we are done with the transition
                    G_state_transition_occuring = FALSE;

                    //State transition done: If KVM, update the OPAL dynamic data
                    if(G_sysConfigData.system_type.kvm)
                    {
                        ssx_semaphore_post(&G_dcomThreadWakeupSem);
                    }

                    break;
                }
            }
        }

        // Check if we hit the end of the table without finding a valid
        // state transition.  If we did, log an internal error.
        if(G_smgr_state_trans_count == jj)
        {
            TRAC_ERR("No transition (or NULL) found for the state change");

            /* @
             * @errortype
             * @moduleid    MAIN_STATE_TRANSITION_MID
             * @reasoncode  INTERNAL_FAILURE
             * @userdata1   G_occ_internal_state
             * @userdata2   i_new_state
             * @userdata4   ERC_SMGR_NO_VALID_STATE_TRANSITION_CALL
             * @devdesc     no valid state transition routine found
             */
            l_transResult = createErrl(MAIN_STATE_TRANSITION_MID,                //modId
                                       INTERNAL_FAILURE,                         //reasoncode
                                       ERC_SMGR_NO_VALID_STATE_TRANSITION_CALL,  //Extended reason code
                                       ERRL_SEV_UNRECOVERABLE,                   //Severity
                                       NULL,                                     //Trace Buf
                                       DEFAULT_TRACE_SIZE,                       //Trace Size
                                       G_occ_internal_state,                     //userdata1
                                       i_new_state);                             //userdata2

            addCalloutToErrl(l_transResult,
                             ERRL_CALLOUT_TYPE_COMPONENT_ID,
                             ERRL_COMPONENT_ID_FIRMWARE,
                             ERRL_CALLOUT_PRIORITY_HIGH);

            break;
        }

        // If the state OCC requested from TMGT is the state we are now in,
        // then change the requested state to NO CHANGE.
        if(CURRENT_STATE() == REQUESTED_STATE())
        {
            REQUESTED_STATE() = OCC_STATE_NOCHANGE;
        }
    } while ( 0 );

    // Get lock for critical section
    ssx_semaphore_post(&G_smgrStateChangeSem);

    return l_transResult;
}


// Function Specification
//
// Name:  SMGR_validate_get_valid_states
//
// Description: Return a byte of status masks that correspond to the v10 poll
//              response definition status byte.
//
// End Function Specification
uint8_t SMGR_validate_get_valid_states(void)
{
    uint8_t         l_valid_states = 0;
    uint32_t        l_datamask = DATA_get_present_cnfgdata();
    static BOOLEAN  l_throttle_traced = FALSE;

    // If we have everything we need to go to observation state
    if((l_datamask & SMGR_VALIDATE_DATA_OBSERVATION_MASK) ==
            SMGR_VALIDATE_DATA_OBSERVATION_MASK)
    {
        l_valid_states |= SMGR_MASK_OBSERVATION_READY;
    }

    // If we have everything we need to go to active state
    if(((l_datamask & SMGR_VALIDATE_DATA_ACTIVE_MASK) ==
            SMGR_VALIDATE_DATA_ACTIVE_MASK) )
    {
        l_valid_states |= SMGR_MASK_ACTIVE_READY;

        if(!l_throttle_traced)
        {
            TRAC_IMP("Setting Active Bit for this OCC");
            l_throttle_traced = TRUE;
        }
    }

    // If we are master OCC, set this bit
    if(OCC_MASTER == G_occ_role)
    {
       l_valid_states |= SMGR_MASK_MASTER_OCC;
    }

    // Indicate if this OCC is the FIR master.
    if (OCC_IS_FIR_MASTER())
    {
        l_valid_states |= OCC_ROLE_FIR_MASTER_MASK;
    }

    if(G_simics_environment)
    {
        l_valid_states |= OCC_SIMICS_ENVIRONMENT;
    }

    return l_valid_states;
}

