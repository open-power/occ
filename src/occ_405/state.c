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

extern GpeRequest G_clip_update_req;
extern GPE_BUFFER(ipcmsg_clip_update_t*  G_clip_update_parms_ptr);


// Maximum allowed value approx. 16.3 ms
#define PCBS_HEARBEAT_TIME_US 16320

errlHndl_t SMGR_standby_to_observation();
errlHndl_t SMGR_standby_to_active();
errlHndl_t SMGR_observation_to_standby();
errlHndl_t SMGR_observation_to_active();
errlHndl_t SMGR_active_to_observation();
errlHndl_t SMGR_active_to_standby();
errlHndl_t SMGR_all_to_safe();

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
    {OCC_STATE_STANDBY,     OCC_STATE_OBSERVATION, &SMGR_standby_to_observation},
    {OCC_STATE_STANDBY,     OCC_STATE_ACTIVE,      &SMGR_standby_to_active},
    {OCC_STATE_OBSERVATION, OCC_STATE_STANDBY,     &SMGR_observation_to_standby},
    {OCC_STATE_OBSERVATION, OCC_STATE_ACTIVE,      &SMGR_observation_to_active},
    {OCC_STATE_ACTIVE,      OCC_STATE_OBSERVATION, &SMGR_active_to_observation},
    {OCC_STATE_ACTIVE,      OCC_STATE_STANDBY,     &SMGR_active_to_standby},

    /* ----- DEFAULT STATE TRANSITIONS ----- */
    /* These are default state transitions for when it doesn't matter what
     * state we were in before the transition. */

    /* Current State        New State              Transition Function */
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
// Name:
//
// Description:
//
// End Function Specification
errlHndl_t SMGR_observation_to_standby()
{
    errlHndl_t              l_errlHndl = NULL;

    TRAC_IMP("SMGR: Observation to Standby Transition Started");

    // Set the RTL Flags to indicate which tasks can run
    //   - Clear ACTIVE b/c not in ACTIVE State
    //   - Clear OBSERVATION b/c not in OBSERVATION State
    rtl_clr_run_mask_deferred(RTL_FLAG_ACTIVE | RTL_FLAG_OBS );
    rtl_set_run_mask_deferred(RTL_FLAG_STANDBY);

    // Set the actual STATE now that we have finished everything else
    CURRENT_STATE() = OCC_STATE_STANDBY;

    TRAC_IMP("SMGR: Observation to Standby Transition Completed");

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

    // confirm that the clip update IPC call to widen clip ranges
    // has successfully completed on PGPE (with no errors)
    if( !async_request_is_idle(&G_clip_update_req.request) )  //widen_clip_ranges didn't complete
    {
        // an earlier clip update IPC call has not completed, trace and log an error
        TRAC_ERR("SMGR: clip update IPC task is not Idle");

            /*
             * @errortype
             * @moduleid    MAIN_SMGR_MID
             * @reasoncode  PGPE_FAILURE
             * @userdata4   ERC_PGPE_NOT_IDLE
             * @devdesc     pgpe clip update not idle
             */
        l_errlHndl = createErrl(
            MAIN_SMGR_MID,                   //ModId
            PGPE_FAILURE,                    //Reasoncode
            ERC_PGPE_NOT_IDLE,               //Extended reason code
            ERRL_SEV_PREDICTIVE,             //Severity
            NULL,                            //Trace Buf
            DEFAULT_TRACE_SIZE,              //Trace Size
            0,                               //Userdata1
            0                                //Userdata2
            );
        // TODO now: REQUEST_RESET?
    }
    else if ( G_clip_update_parms_ptr->msg_cb.rc != PGPE_RC_SUCCESS ) // IPC task completed with errors
    {
        // an earlier clip update IPC call has not completed, trace and log an error
        TRAC_ERR("SMGR: clip update IPC task returned an error [0x%08X]",
                 G_clip_update_parms_ptr->msg_cb.rc);

            /*
             * @errortype
             * @moduleid    MAIN_SMGR_MID
             * @reasoncode  PGPE_FAILURE
             * @userdata1   PGPE clip update's rc
             * @userdata4   ERC_PGPE_CLIP_FAILURE
             * @devdesc     pgpe clip update not idle
             */
        l_errlHndl = createErrl(
            MAIN_SMGR_MID,                        //ModId
            PGPE_FAILURE,                         //Reasoncode
            ERC_PGPE_CLIP_FAILURE,                //Extended reason code
            ERRL_SEV_PREDICTIVE,                  //Severity
            NULL,                                 //Trace Buf
            DEFAULT_TRACE_SIZE,                   //Trace Size
            G_clip_update_parms_ptr->msg_cb.rc,   //Userdata1
            0                                     //Userdata2
            );
        // TODO now: REQUEST_RESET?
    }

    else // Clips wide opened with no errors, enable Pstates on PGPE
    {

        // Pstates are enabled via an IPC call to PGPE, which will
        // set the G_proc_pstate_status flag

        l_errlHndl = pgpe_start_suspend(PGPE_ACTION_PSTATE_START);

        if(l_errlHndl)
        {
            TRAC_ERR("SMGR: Failed to switch to Active state because of a "
                     "failure to start the pstate protocol on PGPE.");
        }
        else
        {
            // Pstates enabled, update OPAL static table in main memory
            if(G_sysConfigData.system_type.kvm)
            {
                // upon succesful enablement of Pstate protocol on
                // PGPE update OPAL table with pstate information.
                proc_pstate_kvm_setup();
            }
        }
    }

    // NOTE that this is really unnecessary if you follow the TMGT OCC
    // Interface Spec, which tells you that you need to check for the "Active
    // Ready" bit in the poll response before you go to active state.
    // But since we have scenerios where TMGT isn't the one putting us in
    // active state (we are going there automatically) we needed to add this.

    // If we have all data we need to go to active state, but don't have pstates
    // enabled yet...then we will do the aforementioned wait
    if(((DATA_get_present_cnfgdata() & SMGR_VALIDATE_DATA_ACTIVE_MASK) ==
        SMGR_VALIDATE_DATA_ACTIVE_MASK))
    {
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
        if(proc_is_hwpstate_enabled() && G_sysConfigData.system_type.kvm)
        {
            TRAC_IMP("SMGR: Pstates are enabled, continuing with state trans");
        }
    }

    // Check if all conditions are met to transition to active state.  If
    // they aren't, then log an error and fail to change state.

    if( (SMGR_MASK_ACTIVE_READY ==
        (SMGR_validate_get_valid_states() & SMGR_MASK_ACTIVE_READY))
            && proc_is_hwpstate_enabled() )
    {
        l_error_logged = FALSE;
        TRAC_IMP("SMGR: Observation to Active Transition Started");

        // Set the RTL Flags to indicate which tasks can run
        //   - Clear OBSERVATION b/c not in OBSERVATION State
        //   - Set ACTIVE b/c we're in ACTIVE State
        rtl_clr_run_mask_deferred(RTL_FLAG_OBS);
        rtl_set_run_mask_deferred(RTL_FLAG_ACTIVE);

        // Set the actual STATE now that we have finished everything else
        CURRENT_STATE() = OCC_STATE_ACTIVE;
        TRAC_IMP("SMGR: Observation to Active Transition Completed");
    }
    else if(l_rc)
    {
        TRAC_ERR("SMGR: Observation to Active Transition Failed, cnfgdata=0x%08x, reqd=0x%08x",
                DATA_get_present_cnfgdata(),
                SMGR_VALIDATE_DATA_ACTIVE_MASK);
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
    errlHndl_t              l_errlHndl = NULL;

    TRAC_IMP("SMGR: Active to Observation Transition Started");

    // Set the RTL Flags to indicate which tasks can run
    //   - Set OBSERVATION b/c in OBSERVATION State
    //   - Clear ACTIVE b/c not in ACTIVE State
    rtl_clr_run_mask_deferred(RTL_FLAG_ACTIVE);
    rtl_set_run_mask_deferred(RTL_FLAG_OBS);

    // Set the actual STATE now that we have finished everything else
    CURRENT_STATE() = OCC_STATE_OBSERVATION;

    TRAC_IMP("SMGR: Active to Observation Transition Completed");

    return l_errlHndl;
}


// Function Specification
//
// Name:  SMGR_active_to_standby
//
// Description:
//
// End Function Specification
errlHndl_t SMGR_active_to_standby()
{
    errlHndl_t              l_errlHndl = NULL;

    TRAC_IMP("SMGR: Active to Standby Transition Started");

    // Transtion through both functions on this multi-state transtion
    l_errlHndl = SMGR_active_to_observation();
    if(NULL == l_errlHndl)
    {
       l_errlHndl = SMGR_observation_to_standby();
    }

    if(l_errlHndl)
    {
        TRAC_ERR("SMGR: Active to Standby Transition Failed");
    }
    else
    {
        TRAC_IMP("SMGR: Active to Standby Transition Completed");
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
            l_transResult = NULL;
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

