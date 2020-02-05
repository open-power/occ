/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/mode.c $                                          */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2020                        */
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
#include "ssx_io.h"
#include "trac.h"
#include "rtls.h"
#include "state.h"
#include "occ_service_codes.h"
#include "amec_freq.h"
#include "amec_part.h"
#include "amec_data.h"
#include "amec_sys.h"

errlHndl_t SMGR_mode_transition_to_disabled();
errlHndl_t SMGR_mode_transition_to_powersave();
errlHndl_t SMGR_mode_transition_to_static_freq_point();
errlHndl_t SMGR_mode_transition_to_ffo();
errlHndl_t SMGR_mode_transition_to_fmax();
errlHndl_t SMGR_mode_transition_to_dyn_perf();
errlHndl_t SMGR_mode_transition_to_max_perf();


// Mode that OCC is currently in
OCC_MODE           G_occ_internal_mode      = OCC_MODE_NOCHANGE;

// Mode that OCC is requesting that TMGT put OCC into
OCC_MODE           G_occ_internal_req_mode  = OCC_MODE_NOCHANGE;

// Mode that TMGT is requesting OCC go to
OCC_MODE           G_occ_external_req_mode = OCC_MODE_NOCHANGE;

// Mode that TMGT is requesting OCC go to in KVM
OCC_MODE           G_occ_external_req_mode_kvm = OCC_MODE_NOCHANGE;

// In FFO or Static Frequency Point modes, this extra parameter is used to define the frequency
uint16_t           G_occ_external_req_mode_parm = OCC_MODE_PARM_NONE;
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
    /* Current Mode         New Mode                    Transition Function */
    {OCC_MODE_ALL,          OCC_MODE_DISABLED,          &SMGR_mode_transition_to_disabled},
    {OCC_MODE_ALL,          OCC_MODE_PWRSAVE,           &SMGR_mode_transition_to_powersave},
    {OCC_MODE_ALL,          OCC_MODE_STATIC_FREQ_POINT, &SMGR_mode_transition_to_static_freq_point},
    {OCC_MODE_ALL,          OCC_MODE_FFO,               &SMGR_mode_transition_to_ffo},
    {OCC_MODE_ALL,          OCC_MODE_FMAX,              &SMGR_mode_transition_to_fmax},
    {OCC_MODE_ALL,          OCC_MODE_DYN_PERF,          &SMGR_mode_transition_to_dyn_perf},
    {OCC_MODE_ALL,          OCC_MODE_MAX_PERF,          &SMGR_mode_transition_to_max_perf},
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
errlHndl_t SMGR_set_mode( const OCC_MODE i_mode )
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

         // Running with OPAL, just accept whatever mode and keep reporting back that we are
         // in that mode. However, internally we should not initiate any mode transition
         if(G_sysConfigData.system_type.kvm)
         {
             G_occ_external_req_mode_kvm = l_mode;
         }

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

                 /* @
                  * @errortype
                  * @moduleid    MAIN_MODE_TRANSITION_MID
                  * @reasoncode  INTERNAL_FAILURE
                  * @userdata1   G_occ_internal_mode
                  * @userdata2   l_mode
                  * @userdata4   ERC_SMGR_NO_VALID_MODE_TRANSITION_CALL
                  * @devdesc     no valid state transition routine found
                  */
                  l_errlHndl = createErrl(MAIN_MODE_TRANSITION_MID,                 //modId
                                          INTERNAL_FAILURE,                         //reasoncode
                                          ERC_SMGR_NO_VALID_MODE_TRANSITION_CALL,   //Extended reason code
                                          ERRL_SEV_UNRECOVERABLE,                   //Severity
                                          NULL,                                     //Trace Buf
                                          DEFAULT_TRACE_SIZE,                       //Trace Size
                                          G_occ_internal_mode,                      //userdata1
                                          l_mode);                                  //userdata2

                  addCalloutToErrl(l_errlHndl,
                                   ERRL_CALLOUT_TYPE_COMPONENT_ID,
                                   ERRL_COMPONENT_ID_FIRMWARE,
                                   ERRL_CALLOUT_PRIORITY_HIGH);

                 break;
             }

             // there is only one core group possible
             g_amec->part_config.part_list[0].es_policy = CURRENT_MODE();
/* TODO RTC 246412
             // Update the DPS parameters based on the power policy
             if (g_amec->part_config.part_list[0].es_policy == OCC_MODE_DYN_PERF)
             {
                amec_part_update_dps_parameters(&(g_amec->part_config.part_list[0]));
             }

             // Update internal performance settings for this partition
             amec_part_update_perf_settings(&(g_amec->part_config.part_list[0]));
*/

         }
         while(0);

         if(l_errlHndl)
         {
           // Punt !!! :-)
           break;
         }

         // Load correct thermal thresholds based on the current mode
         l_errlHndl = AMEC_data_write_thrm_thresholds(CURRENT_MODE());

     }while(0);

     // Unlock critical section
     ssx_semaphore_post(&G_smgrModeChangeSem);

     return l_errlHndl;
}


// Function Specification
//
// Name: SMGR_mode_transition_to_disabled
//
// Description:
//
// End Function Specification
errlHndl_t SMGR_mode_transition_to_disabled()
{
    errlHndl_t              l_errlHndl = NULL;

    TRAC_IMP("SMGR: Mode to Disabled Transition Started");

    // Set Freq Mode for AMEC to use
    l_errlHndl = amec_set_freq_range(OCC_MODE_DISABLED);

    CURRENT_MODE() = OCC_MODE_DISABLED;

    // WOF is disabled in disabled mode
    set_clear_wof_disabled( SET,
                            WOF_RC_MODE_NO_SUPPORT_MASK,
                            ERC_WOF_MODE_NO_SUPPORT_MASK );
    TRAC_IMP("SMGR: Mode to Disabled Transition Completed");

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

    // WOF is disabled in SPS mode
    set_clear_wof_disabled( SET,
                            WOF_RC_MODE_NO_SUPPORT_MASK,
                            ERC_WOF_MODE_NO_SUPPORT_MASK );

    TRAC_IMP("SMGR: Mode to PowerSave Transition Completed");

    return l_errlHndl;
}

// Function Specification
//
// Name: SMGR_mode_transition_to_static_freq_point
//
// Description:
//
// End Function Specification
errlHndl_t SMGR_mode_transition_to_static_freq_point()
{
    errlHndl_t              l_errlHndl = NULL;

    TRAC_IMP("SMGR: Mode to Static Frequency Point Transition Started");

    // Set Freq Mode for AMEC to use
    l_errlHndl = amec_set_freq_range(OCC_MODE_STATIC_FREQ_POINT);

    CURRENT_MODE() = OCC_MODE_STATIC_FREQ_POINT;

    // WOF is disabled in this mode
    set_clear_wof_disabled( SET,
                            WOF_RC_MODE_NO_SUPPORT_MASK,
                            ERC_WOF_MODE_NO_SUPPORT_MASK );

    TRAC_IMP("SMGR: Mode to Static Frequency Point Transition Completed");

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

    // WOF is disabled in FFO
    set_clear_wof_disabled( SET,
                            WOF_RC_MODE_NO_SUPPORT_MASK,
                            ERC_WOF_MODE_NO_SUPPORT_MASK );

    TRAC_IMP("SMGR: Mode to FFO Transition Completed");

    return l_errlHndl;
}

// Function Specification
//
// Name: SMGR_mode_transition_to_fmax
//
// Description: Transition to Maximum Frequency mode
//
// End Function Specification
errlHndl_t SMGR_mode_transition_to_fmax()
{
    errlHndl_t              l_errlHndl = NULL;

    TRAC_IMP("SMGR: Mode to Fmax Transition Started");

    // Set Freq Mode for AMEC to use
    l_errlHndl = amec_set_freq_range(OCC_MODE_FMAX);

    CURRENT_MODE() = OCC_MODE_FMAX;
    // WOF is disabled in Fmax
    set_clear_wof_disabled( SET,
                            WOF_RC_MODE_NO_SUPPORT_MASK,
                            ERC_WOF_MODE_NO_SUPPORT_MASK );
    TRAC_IMP("SMGR: Mode to Fmax Transition Completed");

    return l_errlHndl;
}

// Function Specification
//
// Name: SMGR_mode_transition_to_dyn_perf
//
// Description: Transition to dynamic performance mode
//
// End Function Specification
errlHndl_t SMGR_mode_transition_to_dyn_perf()
{
    errlHndl_t              l_errlHndl = NULL;

    TRAC_IMP("SMGR: Mode to Dynamic Performance Transition Started");

    // Set Freq Mode for AMEC to use
    l_errlHndl = amec_set_freq_range(OCC_MODE_DYN_PERF);

    CURRENT_MODE() = OCC_MODE_DYN_PERF;
    // WOF is enabled in dynamic performance mode, clear the mode bit
    set_clear_wof_disabled( CLEAR,
                            WOF_RC_MODE_NO_SUPPORT_MASK,
                            ERC_WOF_MODE_NO_SUPPORT_MASK );

    TRAC_IMP("SMGR: Mode to Dynamic Performance Transition Completed");

    return l_errlHndl;
}

// Function Specification
//
// Name: SMGR_mode_transition_to_max_perf
//
// Description: Transition to Maximum Performance mode
//
// End Function Specification
errlHndl_t SMGR_mode_transition_to_max_perf()
{
    errlHndl_t              l_errlHndl = NULL;

    TRAC_IMP("SMGR: Mode to Maximum Performance Transition Started");

    // Set Freq Mode for AMEC to use
    l_errlHndl = amec_set_freq_range(OCC_MODE_MAX_PERF);

    CURRENT_MODE() = OCC_MODE_MAX_PERF;
    // WOF is enabled in max performance mode, clear the mode bit
    set_clear_wof_disabled( CLEAR,
                            WOF_RC_MODE_NO_SUPPORT_MASK,
                            ERC_WOF_MODE_NO_SUPPORT_MASK );

    TRAC_IMP("SMGR: Mode to Maximum Performance Transition Completed");

    return l_errlHndl;
}
