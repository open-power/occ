/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/mode.c $                                          */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2024                        */
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
#include "amec_data.h"
#include "amec_sys.h"
#include <pstates_occ.H>

errlHndl_t SMGR_mode_transition_to_disabled();
errlHndl_t SMGR_mode_transition_to_powersave();
errlHndl_t SMGR_mode_transition_to_static_freq_point();
errlHndl_t SMGR_mode_transition_to_ffo();
errlHndl_t SMGR_mode_transition_to_fmax();
errlHndl_t SMGR_mode_transition_to_non_deterministic();
errlHndl_t SMGR_mode_transition_to_efficiency_power();
errlHndl_t SMGR_mode_transition_to_efficiency_perf();
errlHndl_t SMGR_mode_transition_to_balanced();
errlHndl_t SMGR_mode_transition_to_max_perf();

extern uint32_t G_present_cores;
extern OCCPstateParmBlock_t G_oppb;   // OCC Pstate Parameters Block Structure

// Mode that OCC is currently in
OCC_MODE           G_occ_internal_mode      = OCC_MODE_NOCHANGE;
OCC_FREQ_PT_PARM   G_occ_internal_mode_parm = OCC_FREQ_PT_PARM_NONE;

// Mode that OCC is requesting that TMGT put OCC into
OCC_MODE           G_occ_internal_req_mode  = OCC_MODE_NOCHANGE;

// Mode that TMGT is requesting OCC go to
OCC_MODE           G_occ_external_req_mode      = OCC_MODE_NOCHANGE;
// In FFO or Static Frequency Point, this extra parameter is used to define the frequency
OCC_FREQ_PT_PARM   G_occ_external_req_mode_parm = OCC_FREQ_PT_PARM_NONE;

// Mode that TMGT is requesting OCC go to in KVM
OCC_MODE           G_occ_external_req_mode_kvm = OCC_MODE_NOCHANGE;

// Indicates if we are currently in a mode transition
bool                G_mode_transition_occuring = FALSE;

// Mode that OCC Master is in
OCC_MODE            G_occ_master_mode      = OCC_MODE_NOCHANGE;
OCC_FREQ_PT_PARM    G_occ_master_mode_parm = OCC_FREQ_PT_PARM_NONE;

// Semaphore to allow mode change to be called from multiple threads
SsxSemaphore        G_smgrModeChangeSem;


// Table that indicates which functions should be run for a given mode
// transition.
const smgr_state_trans_t G_smgr_mode_trans[] =
{
    /* Current Mode         New Mode                    Transition Function */
    {OCC_MODE_ALL,          OCC_MODE_DISABLED,          &SMGR_mode_transition_to_disabled},
    {OCC_MODE_ALL,          OCC_MODE_NON_DETERMINISTIC, &SMGR_mode_transition_to_non_deterministic},
    {OCC_MODE_ALL,          OCC_MODE_STATIC_FREQ_POINT, &SMGR_mode_transition_to_static_freq_point},
    {OCC_MODE_ALL,          OCC_MODE_PWRSAVE,           &SMGR_mode_transition_to_powersave},
    {OCC_MODE_ALL,          OCC_MODE_EFFICIENCY_POWER,  &SMGR_mode_transition_to_efficiency_power},
    {OCC_MODE_ALL,          OCC_MODE_EFFICIENCY_PERF,   &SMGR_mode_transition_to_efficiency_perf},
    {OCC_MODE_ALL,          OCC_MODE_FMAX,              &SMGR_mode_transition_to_fmax},
    {OCC_MODE_ALL,          OCC_MODE_BALANCED,          &SMGR_mode_transition_to_balanced},
    {OCC_MODE_ALL,          OCC_MODE_FFO,               &SMGR_mode_transition_to_ffo},
    {OCC_MODE_ALL,          OCC_MODE_MAX_PERF,          &SMGR_mode_transition_to_max_perf},
};
const uint8_t G_smgr_mode_trans_count = sizeof(G_smgr_mode_trans)/sizeof(smgr_state_trans_t);

// Table that translates the static freq point mode parameter to an actual frequency point
const smgr_sfp_parm_trans_t G_smgr_sfp_mode_parm_trans[] =
{
    // Static Freq Pt Mode Parameter         Freq point
    {OCC_FREQ_PT_PARM_VPD_CF0_PT,          OCC_FREQ_PT_VPD_CF0},
    {OCC_FREQ_PT_PARM_VPD_CF1_PT,          OCC_FREQ_PT_VPD_CF1},
    {OCC_FREQ_PT_PARM_VPD_CF2_PT,          OCC_FREQ_PT_VPD_CF2},
    {OCC_FREQ_PT_PARM_VPD_CF3_PT,          OCC_FREQ_PT_VPD_CF3},
    {OCC_FREQ_PT_PARM_VPD_CF4_PT,          OCC_FREQ_PT_VPD_CF4},
    {OCC_FREQ_PT_PARM_VPD_CF5_PT,          OCC_FREQ_PT_VPD_CF5},
    {OCC_FREQ_PT_PARM_VPD_CF6_PT,          OCC_FREQ_PT_VPD_CF6},
    {OCC_FREQ_PT_PARM_VPD_CF7_PT,          OCC_FREQ_PT_VPD_LAST_CF},
    {OCC_FREQ_PT_PARM_MIN_FREQ_PT,         OCC_FREQ_PT_MIN_FREQ},
    {OCC_FREQ_PT_PARM_WOF_BASE_FREQ_PT,    OCC_FREQ_PT_WOF_BASE},
    {OCC_FREQ_PT_PARM_UT_FREQ_PT,          OCC_FREQ_PT_VPD_UT},
    {OCC_FREQ_PT_PARM_FMAX_FREQ_PT,        OCC_FREQ_PT_MAX_FREQ},
    {OCC_FREQ_PT_PARM_MODE_OFF_FREQ_PT,    OCC_FREQ_PT_MODE_DISABLED},
    {OCC_FREQ_PT_PARM_BOTTOM_THROTTLE,     OCC_FREQ_PT_BOTTOM_THROTTLE}
};
const uint8_t G_smgr_sfp_parm_trans_count = sizeof(G_smgr_sfp_mode_parm_trans)/sizeof(smgr_sfp_parm_trans_t);

extern OCCPstateParmBlock_t G_oppb;

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
         if( (l_mode == OCC_MODE_NOCHANGE) &&
             (G_occ_master_mode_parm == OCC_FREQ_PT_PARM_NONE) )
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
                         if(G_present_cores > 0)
                         {
                             // Signal that we are now in a mode transition
                             G_mode_transition_occuring = TRUE;
                             // Run transition function
                             l_errlHndl = (G_smgr_mode_trans[jj].trans_func_ptr)();
                             // Signal that we are done with the transition
                             G_mode_transition_occuring = FALSE;
                         }
                         else
                         {
                             TRAC_IMP("SMGR: Skipping mode %d change due to no cores present", l_mode);
                             CURRENT_MODE() = l_mode;
                             G_occ_internal_mode_parm = G_occ_master_mode_parm;
                         }
                         break;
                     }
                 }
             }

             // Check if we hit the end of the table without finding a valid
             // mode transition.  If we did, log an internal error.
             if(G_smgr_mode_trans_count == jj)
             {
                 TRAC_ERR("No transition (or NULL) found for mode[0x%2X] change", l_mode);

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
// Name: set_efficiency_mode_parms
//
// Description: Set the max frequency clip and ceff addr from the WOF table for given mode
//
// End Function Specification
void set_efficiency_mode_parms( const OCC_MODE i_mode )
{
    uint16_t                l_freq_mhz = 0;
    int16_t                 l_temp_ceff_add = 0;
    uint8_t                 l_table_ceff_add = 0;

    switch(i_mode)
    {
        case OCC_MODE_NON_DETERMINISTIC:
            l_freq_mhz = G_oppb.non_det_freq_limit_mhz;
            l_table_ceff_add = G_oppb.non_det_ceff_pct;
            break;

        case OCC_MODE_EFFICIENCY_POWER:
            l_freq_mhz = G_oppb.fav_pow_freq_limit_mhz;
            l_table_ceff_add = G_oppb.fav_pow_ceff_pct;
            break;

        case OCC_MODE_EFFICIENCY_PERF:
            l_freq_mhz = G_oppb.fav_perf_freq_limit_mhz;
            l_table_ceff_add = G_oppb.fav_perf_ceff_pct;
            break;

        case OCC_MODE_BALANCED:
            l_freq_mhz = G_oppb.bal_perf_freq_limit_mhz;
            l_table_ceff_add = G_oppb.bal_perf_ceff_pct;
            break;

        default:
            TRAC_ERR("set_efficiency_mode_parms: Unsupported mode 0x%02X", i_mode);
            break;
    }

    // verify the frequency
    if( (l_freq_mhz >= G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MIN_FREQ]) &&
        (l_freq_mhz <= G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_VPD_UT]) )
    {
        g_amec->wof.eco_mode_freq_mhz = l_freq_mhz;
    }
    else
    {
        TRAC_ERR("set_efficiency_mode_parms: WOF table Frequency[%dMHz] out of range[%dMHz - %dMHz] for mode 0x%02X",
                 l_freq_mhz,
                 G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MIN_FREQ],
                 G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_VPD_UT],
                 i_mode);
        g_amec->wof.eco_mode_freq_mhz = G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_VPD_UT];
    }

    // Shouldn't be getting mode with OPAL but don't set ceff if we did
    if(!G_sysConfigData.system_type.kvm)
    {
       // msb is sign bit remaining bits in 1% unit convert to 0.01%
       l_temp_ceff_add = (l_table_ceff_add & 0x7F) * 100;

       if(l_table_ceff_add & 0x80)
           l_temp_ceff_add = -l_temp_ceff_add;

       g_amec->wof.eco_mode_ceff_add = l_temp_ceff_add;
    }

    return;
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

    // Shouldn't be getting mode with OPAL but ignore changes to
    // WOF_RC_MODE_NO_SUPPORT_MASK since it is controlled by inband WOF command
    if(!G_sysConfigData.system_type.kvm)
    {
       // WOF is disabled in disabled mode
       set_clear_wof_disabled( SET,
                               WOF_RC_MODE_NO_SUPPORT_MASK,
                               ERC_WOF_MODE_NO_SUPPORT_MASK );
    }
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

    // Shouldn't be getting mode with OPAL but ignore changes to
    // WOF_RC_MODE_NO_SUPPORT_MASK since it is controlled by inband WOF command
    if(!G_sysConfigData.system_type.kvm)
    {
       // WOF is disabled in SPS mode
       set_clear_wof_disabled( SET,
                               WOF_RC_MODE_NO_SUPPORT_MASK,
                               ERC_WOF_MODE_NO_SUPPORT_MASK );
    }

    TRAC_IMP("SMGR: Mode to PowerSave Transition Completed");

    return l_errlHndl;
}

// Function Specification
//
// Name: SMGR_mode_transition_to_non_deterministic
//
// Description: Transition to non deterministic mode
//
// End Function Specification
errlHndl_t SMGR_mode_transition_to_non_deterministic()
{
    errlHndl_t              l_errlHndl = NULL;

    TRAC_IMP("SMGR: Mode to Non-Deterministic Transition Started");

    // set freq limit and ceff add
    set_efficiency_mode_parms(OCC_MODE_NON_DETERMINISTIC);

    // Set Freq Mode for AMEC to use
    l_errlHndl = amec_set_freq_range(OCC_MODE_NON_DETERMINISTIC);

    CURRENT_MODE() = OCC_MODE_NON_DETERMINISTIC;

    // Shouldn't be getting mode with OPAL but ignore changes to
    // WOF_RC_MODE_NO_SUPPORT_MASK since it is controlled by inband WOF command
    if(!G_sysConfigData.system_type.kvm)
    {
       // WOF is enabled in balanced mode, clear the mode bit
       set_clear_wof_disabled( CLEAR,
                               WOF_RC_MODE_NO_SUPPORT_MASK,
                               ERC_WOF_MODE_NO_SUPPORT_MASK );
    }

    TRAC_IMP("SMGR: Mode to non-determinitic Transition Completed ceff add[%d] Freq limit[%dMhz]", g_amec->wof.eco_mode_ceff_add,
              g_amec->wof.eco_mode_freq_mhz);

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
    OCC_FREQ_POINT          l_freq_pt = 0;
    uint32_t                l_steps = 0;
    uint32_t                l_freq  = 0;
    uint8_t                 l_pstate = 0;
    bool                    l_fail  = FALSE;
    errlHndl_t              l_errlHndl = NULL;
    int                     i=0;

    TRAC_IMP("SMGR: Mode to Static Frequency Point with parm[0x%04X] Transition Started",
             G_occ_master_mode_parm);

    if( (G_occ_master_mode_parm & OCC_FREQ_PT_PARM_PSTATE) == OCC_FREQ_PT_PARM_PSTATE )
    {
        // the LSB of the mode parm is the Pstate to set
        l_pstate = (uint8_t)(G_occ_master_mode_parm & 0x00FF);

        // translate Pstate to freq and set the user defined frequency to be used by AMEC
        l_freq = proc_pstate2freq(l_pstate, &l_steps);
        // adjust frequency for l_steps to allow Pstates in throttle space
        l_freq -= (l_steps * G_oppb.frequency_step_khz);
        // l_freq is in kHz, save in MHz
        G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MODE_USER] = (l_freq / 1000);

        // update this OCC internal mode parm that it now matches what was sent from master
        G_occ_internal_mode_parm = G_occ_master_mode_parm;
    }
    else
    {
        // Loop through SFP mode parameter translation table to convert parameter
        // to an actual frequency
        for(i=0; i<G_smgr_sfp_parm_trans_count; i++)
        {
            if(G_smgr_sfp_mode_parm_trans[i].freq_pt_parm == G_occ_master_mode_parm)
            {
                // We found the mode parameter, verify there is a valid frequency
                l_freq_pt = G_smgr_sfp_mode_parm_trans[i].freq_point;
                if(l_freq_pt < OCC_FREQ_PT_COUNT)
                {
                    l_freq = G_sysConfigData.sys_mode_freq.table[l_freq_pt];
                    if(l_freq)
                    {
                        // Set the user defined frequency to be used by AMEC
                        if(G_smgr_sfp_mode_parm_trans[i].freq_pt_parm == OCC_FREQ_PT_PARM_BOTTOM_THROTTLE)
                        {
                            // ok for freq to be below min don't clip
                        }
                        else if(l_freq < G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MIN_FREQ])
                        {
                            TRAC_IMP("SMGR_mode_transition_to_static_freq_point: freq pt freq %dMHz is below min freq %dMHz",
                                     l_freq,
                                     G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MIN_FREQ]);
                            l_freq = G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MIN_FREQ];
                        }
                        else if(l_freq > G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MAX_FREQ])
                        {
                            TRAC_IMP("SMGR_mode_transition_to_static_freq_point: freq pt freq %dMHz is above max freq %dMHz",
                                     l_freq,
                                     G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MAX_FREQ]);
                            l_freq = G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MAX_FREQ];
                        }
                        G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MODE_USER] = l_freq;

                        // update this OCCs internal mode parm that it now matches what was sent
                        // from master
                        G_occ_internal_mode_parm = G_occ_master_mode_parm;
                    }
                    else
                    {
                        TRAC_ERR("SMGR_mode_transition_to_static_freq_point: Zero frequency found for mode parameter[0x%04X]",
                                 G_occ_master_mode_parm);
                        l_fail = TRUE;
                    }
                }
                else
                {
                    TRAC_ERR("SMGR_mode_transition_to_static_freq_point: Invalid freq point[0x%02X] found for mode parameter[0x%04X]",
                             l_freq_pt, G_occ_master_mode_parm);
                    l_fail = TRUE;
                }
                break;
            } // if mode parm matches
        }  // for loop
    } // else parm is not OCC_FREQ_PT_PARM_PSTATE

    // Check if we had a failure or hit the end of the table without finding a valid
    // frequency.  If we did, log an internal error.
    if( (G_smgr_sfp_parm_trans_count == i) || (l_fail) )
    {
        if(G_smgr_sfp_parm_trans_count == i)
            TRAC_ERR("SMGR_mode_transition_to_static_freq_point: Mode parameter[0x%04X] not found",
                     G_occ_master_mode_parm);

        /* @
         * @errortype
         * @moduleid    MAIN_MODE_TRANSITION_MID
         * @reasoncode  INTERNAL_FAILURE
         * @userdata1   G_occ_master_mode_parm
         * @userdata2   0
         * @userdata4   ERC_SMGR_NO_VALID_FREQ_PT
         * @devdesc     no valid frequency point found
         */
        l_errlHndl = createErrl(MAIN_MODE_TRANSITION_MID,    //modId
                                INTERNAL_FAILURE,            //reasoncode
                                ERC_SMGR_NO_VALID_FREQ_PT,   //Extended reason code
                                ERRL_SEV_UNRECOVERABLE,      //Severity
                                NULL,                        //Trace Buf
                                DEFAULT_TRACE_SIZE,          //Trace Size
                                G_occ_master_mode_parm,      //userdata1
                                0);                          //userdata2

        addCalloutToErrl(l_errlHndl,
                         ERRL_CALLOUT_TYPE_COMPONENT_ID,
                         ERRL_COMPONENT_ID_FIRMWARE,
                         ERRL_CALLOUT_PRIORITY_HIGH);

    }  // if failure found
    else // so far so good, finish the mode change
    {
        // Set Freq Mode for AMEC to use
        l_errlHndl = amec_set_freq_range(OCC_MODE_STATIC_FREQ_POINT);

        CURRENT_MODE() = OCC_MODE_STATIC_FREQ_POINT;

        // Shouldn't be getting mode with OPAL but ignore changes to
        // WOF_RC_MODE_NO_SUPPORT_MASK since it is controlled by inband WOF command
        if(!G_sysConfigData.system_type.kvm)
        {
            // WOF is disabled in this mode
            set_clear_wof_disabled( SET,
                                    WOF_RC_MODE_NO_SUPPORT_MASK,
                                    ERC_WOF_MODE_NO_SUPPORT_MASK );
        }
        TRAC_IMP("SMGR: Mode to Static Frequency Point frequency %dMHz Transition Completed", l_freq);
    }
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

    // for FFO G_occ_master_mode_parm contains the FFO frequency
    // frequency should have already been verified to be within range by master
    // in cmdh_tmgt_setmodestate() adding another check here just in case there is ever
    // a chip that has a different range settable via FFO (FFO does NOT allow Fmax which we
    // know will be different per chip)
    if( (G_occ_master_mode_parm >= G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MIN_FREQ]) &&
        (G_occ_master_mode_parm <= G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_VPD_UT]) )
    {
        // Set the user defined frequency to be used by AMEC
        G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MODE_USER] = G_occ_master_mode_parm;

        // update this OCCs internal mode parm that it now matches what was sent from master
        G_occ_internal_mode_parm = G_occ_master_mode_parm;

        // Set Freq Mode for AMEC to use
        l_errlHndl = amec_set_freq_range(OCC_MODE_FFO);

        CURRENT_MODE() = OCC_MODE_FFO;

        // Shouldn't be getting mode with OPAL but ignore changes to
        // WOF_RC_MODE_NO_SUPPORT_MASK since it is controlled by inband WOF command
        if(!G_sysConfigData.system_type.kvm)
        {
            // WOF is disabled in FFO
            set_clear_wof_disabled( SET,
                                    WOF_RC_MODE_NO_SUPPORT_MASK,
                                    ERC_WOF_MODE_NO_SUPPORT_MASK );
        }
        TRAC_IMP("SMGR: Mode to FFO frequency %dMHz Transition Completed", G_occ_master_mode_parm);
    }
    else
    {
        TRAC_ERR("SMGR_mode_transition_to_ffo: Requested Frequency[%dMHz] out of range[%dMHz - %dMHz]",
                 G_occ_master_mode_parm,
                 G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MIN_FREQ],
                 G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_VPD_UT]);

        /* @
         * @errortype
         * @moduleid    MAIN_MODE_TRANSITION_MID
         * @reasoncode  INTERNAL_FAILURE
         * @userdata1   G_occ_master_mode_parm
         * @userdata2   0
         * @userdata4   ERC_SMGR_FREQ_OUT_OF_RANGE
         * @devdesc     Requested FFO frequency not within range
         */
        l_errlHndl = createErrl(MAIN_MODE_TRANSITION_MID,    //modId
                                INTERNAL_FAILURE,            //reasoncode
                                ERC_SMGR_FREQ_OUT_OF_RANGE,  //Extended reason code
                                ERRL_SEV_UNRECOVERABLE,      //Severity
                                NULL,                        //Trace Buf
                                DEFAULT_TRACE_SIZE,          //Trace Size
                                G_occ_master_mode_parm,      //userdata1
                                0);                          //userdata2

        addCalloutToErrl(l_errlHndl,
                         ERRL_CALLOUT_TYPE_COMPONENT_ID,
                         ERRL_COMPONENT_ID_FIRMWARE,
                         ERRL_CALLOUT_PRIORITY_HIGH);
    }
    return l_errlHndl;
}

// Function Specification
//
// Name: SMGR_mode_transition_to_efficiency_power
//
// Description: Transition to efficiency favor power mode
//
// End Function Specification
errlHndl_t SMGR_mode_transition_to_efficiency_power()
{
    errlHndl_t              l_errlHndl = NULL;

    TRAC_IMP("SMGR: Mode to Efficiency Power Transition Started");

    // set freq limit and ceff add
    set_efficiency_mode_parms(OCC_MODE_EFFICIENCY_POWER);

    // Set Freq Mode for AMEC to use
    l_errlHndl = amec_set_freq_range(OCC_MODE_EFFICIENCY_POWER);

    CURRENT_MODE() = OCC_MODE_EFFICIENCY_POWER;

    // Shouldn't be getting mode with OPAL but ignore changes to
    // WOF_RC_MODE_NO_SUPPORT_MASK since it is controlled by inband WOF command
    if(!G_sysConfigData.system_type.kvm)
    {
       // WOF is enabled in balanced mode, clear the mode bit
       set_clear_wof_disabled( CLEAR,
                               WOF_RC_MODE_NO_SUPPORT_MASK,
                               ERC_WOF_MODE_NO_SUPPORT_MASK );
    }

    TRAC_IMP("SMGR: Mode to Efficiency Power Transition Completed ceff add[%d] Freq limit[%dMhz]", g_amec->wof.eco_mode_ceff_add,
              g_amec->wof.eco_mode_freq_mhz);

    return l_errlHndl;
}

// Function Specification
//
// Name: SMGR_mode_transition_to_efficiency_perf
//
// Description: Transition to efficiency favor performance mode
//
// End Function Specification
errlHndl_t SMGR_mode_transition_to_efficiency_perf()
{
    errlHndl_t              l_errlHndl = NULL;

    TRAC_IMP("SMGR: Mode to Efficiency Performance Transition Started");

    // set freq limit and ceff add
    set_efficiency_mode_parms(OCC_MODE_EFFICIENCY_PERF);

    // Set Freq Mode for AMEC to use
    l_errlHndl = amec_set_freq_range(OCC_MODE_EFFICIENCY_PERF);

    CURRENT_MODE() = OCC_MODE_EFFICIENCY_PERF;

    // Shouldn't be getting mode with OPAL but ignore changes to
    // WOF_RC_MODE_NO_SUPPORT_MASK since it is controlled by inband WOF command
    if(!G_sysConfigData.system_type.kvm)
    {
       // WOF is enabled in balanced mode, clear the mode bit
       set_clear_wof_disabled( CLEAR,
                               WOF_RC_MODE_NO_SUPPORT_MASK,
                               ERC_WOF_MODE_NO_SUPPORT_MASK );
    }

    TRAC_IMP("SMGR: Mode to Efficiency Performance Transition Completed ceff add[%d] Freq limit[%dMhz]", g_amec->wof.eco_mode_ceff_add,
              g_amec->wof.eco_mode_freq_mhz);

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

    // Shouldn't be getting mode with OPAL but ignore changes to
    // WOF_RC_MODE_NO_SUPPORT_MASK since it is controlled by inband WOF command
    if(!G_sysConfigData.system_type.kvm)
    {
       // WOF is disabled in Fmax
       set_clear_wof_disabled( SET,
                               WOF_RC_MODE_NO_SUPPORT_MASK,
                               ERC_WOF_MODE_NO_SUPPORT_MASK );
    }

    TRAC_IMP("SMGR: Mode to Fmax Transition Completed");

    return l_errlHndl;
}

// Function Specification
//
// Name: SMGR_mode_transition_to_balanced
//
// Description: Transition to balanced mode
//
// End Function Specification
errlHndl_t SMGR_mode_transition_to_balanced()
{
    errlHndl_t              l_errlHndl = NULL;

    TRAC_IMP("SMGR: Mode to Balanced Transition Started");

    // set freq limit and ceff add
    set_efficiency_mode_parms(OCC_MODE_BALANCED);

    // Set Freq Mode for AMEC to use
    l_errlHndl = amec_set_freq_range(OCC_MODE_BALANCED);

    CURRENT_MODE() = OCC_MODE_BALANCED;

    // Shouldn't be getting mode with OPAL but ignore changes to
    // WOF_RC_MODE_NO_SUPPORT_MASK since it is controlled by inband WOF command
    if(!G_sysConfigData.system_type.kvm)
    {
       // WOF is enabled in balanced mode, clear the mode bit
       set_clear_wof_disabled( CLEAR,
                               WOF_RC_MODE_NO_SUPPORT_MASK,
                               ERC_WOF_MODE_NO_SUPPORT_MASK );
    }

    TRAC_IMP("SMGR: Mode to Balanced Transition Completed ceff add[%d] Freq limit[%dMhz]", g_amec->wof.eco_mode_ceff_add,
              g_amec->wof.eco_mode_freq_mhz);

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

    // max perf isn't an "efficiency" mode, clear the efficiency parms
    // Note the debug command may be used AFTER setting max perf mode to
    // set an efficency mode addr and freq limit
    g_amec->wof.eco_mode_ceff_add = 0;
    g_amec->wof.eco_mode_freq_mhz = 0;

    // Set Freq Mode for AMEC to use
    l_errlHndl = amec_set_freq_range(OCC_MODE_MAX_PERF);

    CURRENT_MODE() = OCC_MODE_MAX_PERF;
    // Shouldn't be getting mode with OPAL but ignore changes to
    // WOF_RC_MODE_NO_SUPPORT_MASK since it is controlled by inband WOF command
    if(!G_sysConfigData.system_type.kvm)
    {
       // WOF is enabled in max performance mode, clear the mode bit
       set_clear_wof_disabled( CLEAR,
                               WOF_RC_MODE_NO_SUPPORT_MASK,
                               ERC_WOF_MODE_NO_SUPPORT_MASK );
    }

    TRAC_IMP("SMGR: Mode to Maximum Performance Transition Completed");

    return l_errlHndl;
}
