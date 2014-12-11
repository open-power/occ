/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/mode.c $                                              */
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

#include <occ_common.h>
#include <common_types.h>
#include "ssx_io.h"
#include "trac.h"
#include "rtls.h"
#include "state.h"
#include "occ_service_codes.h"
#include "amec_freq.h"
#include "amec_part.h"
#include "amec_data.h"

errlHndl_t SMGR_mode_transition_to_nominal();
errlHndl_t SMGR_mode_transition_to_powersave();
errlHndl_t SMGR_mode_transition_to_dynpowersave();
errlHndl_t SMGR_mode_transition_to_dynpowersave_fp();
errlHndl_t SMGR_mode_transition_to_turbo();
errlHndl_t SMGR_mode_transition_to_superturbo();
errlHndl_t SMGR_mode_transition_to_ffo();

// Mode that OCC is currently in
OCC_MODE           G_occ_internal_mode      = OCC_MODE_NOCHANGE;

// Mode that OCC is requesting that TMGT put OCC into
OCC_MODE           G_occ_internal_req_mode  = OCC_MODE_NOCHANGE;

// Mode that TMGT is requesting OCC go to
OCC_MODE           G_occ_external_req_mode = OCC_MODE_NOCHANGE;

// Mode that TMGT is requesting OCC go to in KVM
OCC_MODE           G_occ_external_req_mode_kvm = OCC_MODE_NOCHANGE;

// Indicates if OCC must actually change the voltage / frequency during
// a mode change.
SMGR_SMS_CMD_TYPE   G_occ_internal_sms       = SMGR_SMS_VF_INFO_ONLY;

// Indicates if we are currently in a mode transition
bool                G_mode_transition_occuring = FALSE;

// Mode that OCC Master is in
OCC_MODE            G_occ_master_mode  = OCC_MODE_NOCHANGE;

// Semaphore to allow mode change to be called from multiple threads
SsxSemaphore        G_smgrModeChangeSem;


// Table that indicates which functions should be run for a given mode
// transition.
const smgr_state_trans_t G_smgr_mode_trans[] =
{
    /* ----- SPECIFIC CASE MODE TRANSITIONS ----- */
    /* These are specific mode transitions for when it matters what
     * mode we were in before the transition.  These must come before
     * the agnostic mode transitions below, and will be run instead of
     * those catch-all transition functions. */

    /* Current Mode         New Mode                    Transition Function */
    {OCC_MODE_STURBO,       OCC_MODE_NOMINAL,           NULL},

    /* ----- DEFAULT MODE TRANSITIONS ----- */
    /* These are default mode transitions for when it doesn't matter what
     * mode we were in before the transition. */

    /* Current Mode         New Mode                    Transition Function */
    {OCC_MODE_ALL,          OCC_MODE_NOMINAL,           &SMGR_mode_transition_to_nominal},
    {OCC_MODE_ALL,          OCC_MODE_PWRSAVE,           &SMGR_mode_transition_to_powersave},
    {OCC_MODE_ALL,          OCC_MODE_DYN_POWER_SAVE,    &SMGR_mode_transition_to_dynpowersave},
    {OCC_MODE_ALL,          OCC_MODE_DYN_POWER_SAVE_FP, &SMGR_mode_transition_to_dynpowersave_fp},
    {OCC_MODE_ALL,          OCC_MODE_TURBO,             &SMGR_mode_transition_to_turbo},
    {OCC_MODE_ALL,          OCC_MODE_STURBO,            &SMGR_mode_transition_to_superturbo},
    {OCC_MODE_ALL,          OCC_MODE_FFO,               &SMGR_mode_transition_to_ffo},
};
const uint8_t G_smgr_mode_trans_count = sizeof(G_smgr_mode_trans)/sizeof(smgr_state_trans_t);


// Function Specification
//
// Name: SMGR_is_mode_transitioning
//
// Description:
//
// End Function Specification
inline bool SMGR_is_mode_transitioning(void)
{
   return G_mode_transition_occuring;
}


// Function Specification
//
// Name: SMGR_get_mode
//
// Description:
//
// End Function Specification
inline OCC_MODE SMGR_get_mode(void)
{
   return G_occ_internal_mode;
}


// Function Specification
//
// Name: SMGR_set_mode
//
// Description:
//
// End Function Specification
errlHndl_t SMGR_set_mode(const OCC_MODE i_mode,
                         const uint8_t i_sms_type)
{
     errlHndl_t  l_errlHndl = NULL;
     int         jj=0;
     OCC_MODE l_mode = i_mode;

     do
     {
         // Get lock for critical section
         if(ssx_semaphore_pend(&G_smgrModeChangeSem,SSX_WAIT_FOREVER))
         {
            /* @
             * @errortype
             * @moduleid    MAIN_MODE_TRANSITION_MID
             * @reasoncode  SSX_GENERIC_FAILURE
             * @userdata1   none
             * @userdata4   ERC_RUNNING_SEM_PENDING_FAILURE
             * @devdesc     SSX semaphore related failure
             */
             l_errlHndl = createErrl(MAIN_MODE_TRANSITION_MID,          //modId
                                        SSX_GENERIC_FAILURE,            //reasoncode
                                        ERC_RUNNING_SEM_PENDING_FAILURE,//Extended reason code
                                        ERRL_SEV_UNRECOVERABLE,         //Severity
                                        NULL,                           //Trace Buf
                                        DEFAULT_TRACE_SIZE,             //Trace Size
                                        0,                              //userdata1
                                        0);                             //userdata2

            // Callout firmware
            addCalloutToErrl(l_errlHndl,
                             ERRL_CALLOUT_TYPE_COMPONENT_ID,
                             ERRL_COMPONENT_ID_FIRMWARE,
                             ERRL_CALLOUT_PRIORITY_HIGH);
            break;
         }

         //Check to see if we need to make a change
         if(l_mode == OCC_MODE_NOCHANGE)
         {
             break;
         }

         // SAPPHIRE only accepts DPS-FE mode. In case OCC gets other modes, it should accept the request
         // and keep reporting back that it is in that mode. However, internally we should not
         // initiate any mode transition, i.e., OCC should remain internally in DPS-FE mode.
         if(G_sysConfigData.system_type.kvm)
         {
             G_occ_external_req_mode_kvm = l_mode;
             if (l_mode !=  OCC_MODE_DYN_POWER_SAVE)
             {
                 TRAC_ERR("SAPPHIRE only accepts DPS-FE mode(6) but requested mode is : %d", l_mode);
                 l_mode = OCC_MODE_DYN_POWER_SAVE;
             }
         }

         switch (l_mode)
         {
             case OCC_MODE_NOMINAL:           // FALL THROUGH
             case OCC_MODE_PWRSAVE:           // FALL THROUGH
             case OCC_MODE_DYN_POWER_SAVE:    // FALL THROUGH
             case OCC_MODE_DYN_POWER_SAVE_FP: // FALL THROUGH
             case OCC_MODE_TURBO:             // FALL THROUGH
             case OCC_MODE_STURBO:            // FALL THROUGH
             case OCC_MODE_FFO:               // FALL THROUGH
                 // Notify AMEC of mode change

                 // Change Mode via Transition Function
                 do
                 {
                     // Loop through mode transition table, and find the state
                     // transition function that matches the transition we need to do.
                     for(jj=0; jj<G_smgr_mode_trans_count; jj++)
                     {
                         if( ((G_smgr_mode_trans[jj].old_state == G_occ_internal_mode)
                                   || (G_smgr_mode_trans[jj].old_state == OCC_MODE_ALL) )
                              && (G_smgr_mode_trans[jj].new_state == l_mode) )
                         {
                             // We found the transtion that matches, now run the function
                             // that is associated with that state transition.
                             if(NULL != G_smgr_mode_trans[jj].trans_func_ptr)
                             {
                                 // Signal that we are now in a mode transition
                                 G_mode_transition_occuring = TRUE;
                                 // Run transition function
                                 l_errlHndl = (G_smgr_mode_trans[jj].trans_func_ptr)();
                                 // Signal that we are done with the transition
                                 G_mode_transition_occuring = FALSE;
                                 break;
                             }
                         }
                     }

                     // Check if we hit the end of the table without finding a valid
                     // mode transition.  If we did, log an internal error.
                     if(G_smgr_mode_trans_count == jj)
                     {
                         TRAC_ERR("No transition (or NULL) found for the mode change\n");
                         l_errlHndl = NULL;  //TODO: Create Error
                         break;
                     }

                     // Update the power mode for all core groups that are following system mode
                     AMEC_part_update_sysmode_policy(CURRENT_MODE());
                 }
                 while(0);

                 break;
             default:
                 //unsupported mode
                 break;
         }

         if(l_errlHndl)
         {
           // Punt !!! :-)
           break;
         }

         // Load correct thermal thresholds based on the current mode
         l_errlHndl = AMEC_data_write_thrm_thresholds(CURRENT_MODE());

         // Update the CPU speed in AME?
         // Register the New Mode?
         // Update Power Policy Requirements?
         // Update CPM Calibration

     }while(0);

     // If we have a mode change failure, Mode change flag needs to be set,
     // otherwise, it needs be be cleared/unset.
     if(l_errlHndl)
     {

     }

     // Unlock critical section
     ssx_semaphore_post(&G_smgrModeChangeSem);


     return l_errlHndl;
}


// Function Specification
//
// Name: SMGR_mode_transition_to_nominal
//
// Description:
//
// End Function Specification
errlHndl_t SMGR_mode_transition_to_nominal()
{
    errlHndl_t              l_errlHndl = NULL;

    TRAC_IMP("SMGR: Mode to Nominal Transition Started");

    // Set Freq Mode for AMEC to use
    l_errlHndl = amec_set_freq_range(OCC_MODE_NOMINAL);

    CURRENT_MODE() = OCC_MODE_NOMINAL;
    TRAC_IMP("SMGR: Mode to Nominal Transition Completed");

    return l_errlHndl;
}


// Function Specification
//
// Name: SMGR_mode_transition_to_powersave
//
// Description:
//
// End Function Specification
errlHndl_t SMGR_mode_transition_to_powersave()
{
    errlHndl_t              l_errlHndl = NULL;

    TRAC_IMP("SMGR: Mode to PowerSave Transition Started");

    // Set Freq Mode for AMEC to use
    l_errlHndl = amec_set_freq_range(OCC_MODE_PWRSAVE);

    CURRENT_MODE() = OCC_MODE_PWRSAVE;
    TRAC_IMP("SMGR: Mode to PowerSave Transition Completed");

    return l_errlHndl;
}


// Function Specification
//
// Name: SMGR_mode_transition_to_dynpowersave
//
// Description:
//
// End Function Specification
errlHndl_t SMGR_mode_transition_to_dynpowersave()
{
    errlHndl_t              l_errlHndl = NULL;

    TRAC_IMP("SMGR: Mode to Dynamic PowerSave-Favor Energy Transition Started");

    // Set Freq Mode for AMEC to use
    l_errlHndl = amec_set_freq_range(OCC_MODE_DYN_POWER_SAVE);

    CURRENT_MODE() = OCC_MODE_DYN_POWER_SAVE;
    TRAC_IMP("SMGR: Mode to Dynamic PowerSave-Favor Energy Transition Completed");

    return l_errlHndl;
}

// Function Specification
//
// Name: SMGR_mode_transition_to_dynpowersave_fp
//
// Description:
//
// End Function Specification
errlHndl_t SMGR_mode_transition_to_dynpowersave_fp()
{
    errlHndl_t              l_errlHndl = NULL;

    TRAC_IMP("SMGR: Mode to Dynamic PowerSave-Favor Performance Transition Started");

    // Set Freq Mode for AMEC to use
    l_errlHndl = amec_set_freq_range(OCC_MODE_DYN_POWER_SAVE_FP);

    CURRENT_MODE() = OCC_MODE_DYN_POWER_SAVE_FP;
    TRAC_IMP("SMGR: Mode to Dynamic PowerSave-Favor Performance Transition Completed");

    return l_errlHndl;
}


// Function Specification
//
// Name: SMGR_mode_transition_to_turbo
//
// Description:
//
// End Function Specification
errlHndl_t SMGR_mode_transition_to_turbo()
{
    errlHndl_t              l_errlHndl = NULL;

    TRAC_IMP("SMGR: Mode to Turbo Transition Started");

    // Set Freq Mode for AMEC to use
    l_errlHndl = amec_set_freq_range(OCC_MODE_TURBO);

    CURRENT_MODE() = OCC_MODE_TURBO;
    TRAC_IMP("SMGR: Mode to Turbo Transition Completed");

    return l_errlHndl;
}


// Function Specification
//
// Name: SMGR_mode_transition_to_superturbo
//
// Description:
//
// End Function Specification
errlHndl_t SMGR_mode_transition_to_superturbo()
{
    errlHndl_t              l_errlHndl = NULL;

    TRAC_IMP("SMGR: Mode to SuperTurbo Transition Started");

    // Set Freq Mode for AMEC to use
    l_errlHndl = amec_set_freq_range(OCC_MODE_STURBO);

    CURRENT_MODE() = OCC_MODE_STURBO;
    TRAC_IMP("SMGR: Mode to SuperTurbo Transition Completed");

    return l_errlHndl;
}


// Function Specification
//
// Name: SMGR_mode_transition_to_ffo
//
// Description:
//
// End Function Specification
errlHndl_t SMGR_mode_transition_to_ffo()
{
    errlHndl_t              l_errlHndl = NULL;

    TRAC_IMP("SMGR: Mode to FFO Transition Started");

    // Set Freq Mode for AMEC to use
    l_errlHndl = amec_set_freq_range(OCC_MODE_FFO);

    CURRENT_MODE() = OCC_MODE_FFO;
    TRAC_IMP("SMGR: Mode to FFO Transition Completed");

    return l_errlHndl;
}

