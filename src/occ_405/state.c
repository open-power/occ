\/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/state.c $                                             */
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

#include <occ_common.h>
#include <common_types.h>
#include <centaur_data.h>
#include "ssx_io.h"
#include "trac.h"
#include "rtls.h"
#include "state.h"
#include "dcom.h"
#include "occ_service_codes.h"
#include "proc_pstate.h"
#include "cmdh_fsp_cmds_datacnfg.h"
#include "cmdh_fsp.h"
#include "proc_data.h"
// TEMP -- Doesn't exist anymore
//#include "heartbeat.h"
#include "scom.h"
#include <fir_data_collect.h>

extern proc_gpsm_dcm_sync_occfw_t G_proc_dcm_sync_state;
extern bool G_mem_monitoring_allowed;
extern task_t G_task_table[TASK_END];  // Global task table

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
        l_error_logged = FALSE; // @at015a
        TRAC_IMP("SMGR: Standby to Observation Transition Started");

// TEMP -- NOT SUPPORTED YET IN PHASE1 
/*
        //This flag is set if tmgt sends us at least one data set in the
        //mem config data packet.  No data sets will be sent for centaur ec less than 2.0
        //due to an intermittent hw failure that can occur on those chips.

        if(G_mem_monitoring_allowed)
        {
            if(!rtl_task_is_runnable(TASK_ID_CENTAUR_DATA))
            {

                TRAC_INFO("SMGR_standby_to_observation: calling centaur_init()");
                centaur_init(); //no rc, handles errors internally

                //check if centaur_init resulted in a reset
                //since we don't have a return code from centaur_init.
                if(isSafeStateRequested())
                {
                    TRAC_ERR("SMGR_standby_to_observation: OCC is being reset, centaur_init failed");
                }
                else
                {
                    //initialization was successful.
                    //Set task flags to allow centaur control task to run and
                    //also to prevent us from doing initialization again.
                    G_task_table[TASK_ID_CENTAUR_DATA].flags = CENTAUR_DATA_RTL_FLAGS;
                    G_task_table[TASK_ID_CENTAUR_CONTROL].flags = CENTAUR_CONTROL_RTL_FLAGS;
                }
            }
        }
*/
        // Set the RTL Flags to indicate which tasks can run
        //   - Set OBSERVATION b/c we're in OBSERVATION State
        rtl_clr_run_mask_deferred(RTL_FLAG_STANDBY);
        rtl_set_run_mask_deferred(RTL_FLAG_OBS);

        // Set the actual STATE now that we have finished everything else
        CURRENT_STATE() = OCC_STATE_OBSERVATION;

        TRAC_IMP("SMGR: Standby to Observation Transition Completed");
    }
    else if(FALSE == l_error_logged)// @at015a
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

        // @wb001 -- Callout firmware
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
// Name:
//
// Description:
//
// End Function Specification
errlHndl_t SMGR_observation_to_active()
{
    errlHndl_t      l_errlHndl = NULL;
/* TEMP -- UNNECCESSARY IN PHASE1 */
#if 0
    static bool l_error_logged = FALSE;  // To prevent trace and error log happened over and over
    int                l_extRc = OCC_NO_EXTENDED_RC;
    int                l_rc = 0;

    // First check to make sure Pstates are enabled. If they aren't, then
    // wait TBD seconds in case we are going directly from Standby to Active
    // (pstate init only happens in observation state, so it might not be
    // done yet...must call it in this while loop since it is done in this
    // same thread...)
    //
    // NOTE that this is really unnecessary if you follow the TMGT OCC
    // Interface Spec, which tells you that you need to check for the "Active
    // Ready" bit in the poll response before you go to active state.
    // But since we have scenerios where TMGT isn't the one putting us in
    // active state (we are going there automatically) we needed to add this

    // If we have all data we need to go to active state, but don't have pstates
    // enabled yet...then we will do the aforementioned wait
    if(((DATA_get_present_cnfgdata() & SMGR_VALIDATE_DATA_ACTIVE_MASK) ==
            SMGR_VALIDATE_DATA_ACTIVE_MASK)
        && !proc_is_hwpstate_enabled() )
    {
        SsxTimebase start = ssx_timebase_get();
        while( ! proc_is_hwpstate_enabled() )
        {
            SsxInterval timeout =  SSX_SECONDS(5);
            if ((ssx_timebase_get() - start) > timeout)
            {
                if(FALSE == l_error_logged)
                {
                    TRAC_ERR("SMGR: Timeout waiting for Pstates to be enabled, master state=%d, slave state=%d, pmc_mode[%08x], chips_present[%02x], pmc_deconfig[%08x]", //gm034
                            G_proc_dcm_sync_state.sync_state_master,
                            G_proc_dcm_sync_state.sync_state_slave,
                            in32(PMC_MODE_REG),
                            G_sysConfigData.is_occ_present,
                            in32(PMC_CORE_DECONFIGURATION_REG));
                }
                l_extRc = ERC_GENERIC_TIMEOUT;
                break;
            }
            proc_gpsm_dcm_sync_enable_pstates_smh();
            ssx_sleep(SSX_MICROSECONDS(500));
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

        // Ensure that the dpll override (enabled when mfg biases freq) has been disabled.
        int l_core;
        uint32_t l_configured_cores;
        pcbs_pmgp1_reg_t l_pmgp1;
        l_configured_cores = ~in32(PMC_CORE_DECONFIGURATION_REG);
        for(l_core = 0; l_core < PGP_NCORES; l_core++, l_configured_cores <<= 1)
        {
            if(!(l_configured_cores & 0x80000000)) continue;
            l_pmgp1.value = 0;
            l_pmgp1.fields.dpll_freq_override_enable = 1;
            l_rc = putscom_ffdc(CORE_CHIPLET_ADDRESS(PCBS_PMGP1_REG_AND, l_core),
                           ~l_pmgp1.value, NULL); //commit errors internally -- gm033
            if(l_rc)
            {
                TRAC_ERR("Failed disabling dpll frequency override.  rc=0x%08x, core=%d", l_rc, l_core);
                break;
            }
        }
        if(!l_rc)
        {

            // Set the actual STATE now that we have finished everything else
            CURRENT_STATE() = OCC_STATE_ACTIVE;
            TRAC_IMP("SMGR: Observation to Active Transition Completed");

            // Configure and enable the PCB slave heartbeat timer
            //
            // task_core_data is running in every RTL tick and scheduling a run
            // of gpe_get_per_core_data which will do a getscom on
            // PCBS_PMSTATEHISTOCC_REG.  We will use PCBS_PMSTATEHISTOCC_REG as
            // the PCBS heartbeat trigger register.
            unsigned int l_actual_pcbs_hb_time = 0;
            ChipConfigCores l_cfgd_cores =
                    (ChipConfigCores)((uint64_t)core_configuration() >> 16);
            TRAC_IMP("Configuring PCBS heartbeat for configured cores=0x%8.8x", l_cfgd_cores);
            TRAC_IMP("OCC configuration view: G_present_hw_cores=0x%8.8x, G_present_cores=0x%8.8x",
                    G_present_hw_cores, G_present_cores);

            // Setup the pcbs heartbeat timer
            l_rc = pcbs_hb_config(1, // enable = yes
                                  l_cfgd_cores,
                                  PCBS_PMSTATEHISTOCC_REG,
                                  PCBS_HEARBEAT_TIME_US,
                                  0, // force = no
                                  &l_actual_pcbs_hb_time);

            if (l_rc)
            {
                TRAC_ERR("Failure configuring the PCBS heartbeat timer, rc=%d",
                         l_rc);
                // FIXME #state_c_002 Add appropriate callouts for
                // serviceability review
                l_extRc = ERC_STATE_HEARTBEAT_CFG_FAILURE;
            }
            else
            {
                TRAC_IMP("PCBS heartbeat enabled, requested time(us)=%d, actual time(us)=%d",
                         PCBS_HEARBEAT_TIME_US,
                         l_actual_pcbs_hb_time);
            }

            // TODO: #state_c_001 Manually configuring the PMC
            // heartbeat until pmc_hb_config is shown to be working
            // Reference SW238882 for more information on updates needed in
            // pmc_hb_config.  Note that if PMC parameter hangpulse pre-divider
            // is 0 then the PMC heartbeat counter uses nest_nclck/4 instead of
            // the hangpulse to count the heartbeat timeout.
            pmc_parameter_reg0_t ppr0;
            pmc_occ_heartbeat_reg_t pohr;
            pohr.value = in32(PMC_OCC_HEARTBEAT_REG);
            ppr0.value = in32(PMC_PARAMETER_REG0);
            pohr.fields.pmc_occ_heartbeat_en = 0;
            // This combined with the hang pulse count and pre-divider yields
            // about a 2 second timeout
            pohr.fields.pmc_occ_heartbeat_time = 0xffff;
            TRAC_IMP("Configure PMC heartbeat, heartbeat_time=0x%x",
                    pohr.fields.pmc_occ_heartbeat_time);
            ppr0.fields.hangpulse_predivider = 1;
            TRAC_IMP("Configure PMC parm reg predivider=%d",
                    ppr0.fields.hangpulse_predivider);

            // Write registers twice, known issue with heartbeat reg
            out32(PMC_OCC_HEARTBEAT_REG, pohr.value);
            out32(PMC_OCC_HEARTBEAT_REG, pohr.value);
            out32(PMC_PARAMETER_REG0, ppr0.value);
            out32(PMC_PARAMETER_REG0, ppr0.value);
            TRAC_IMP("Enable PMC heartbeat timer");
            pohr.value = in32(PMC_OCC_HEARTBEAT_REG);
            pohr.fields.pmc_occ_heartbeat_en = 1;
            out32(PMC_OCC_HEARTBEAT_REG, pohr.value);
            out32(PMC_OCC_HEARTBEAT_REG, pohr.value);
        }
    }
    else
    {
        //trace and log an error.
        l_rc = -1;
    }

    if(l_rc)
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
#endif
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
// TEMP -- IN PHASE1 WE ARE ALWAYS MASTER
//    if (OCC_MASTER == G_occ_role)
    if (TRUE)
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

    // If we hit an oversubscription, we need to set that bit
    if(FSI2HOST_MBOX_INITIALIZED == G_fsi2host_mbox_ready)
    {
        l_valid_states |=  SMGR_MASK_ATTN_ENABLED;
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

    return l_valid_states;
}

