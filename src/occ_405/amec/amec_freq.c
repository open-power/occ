/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/amec/amec_freq.c $                                */
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

//*************************************************************************
// Includes
//*************************************************************************
#include <occ_common.h>
#include <ssx.h>
#include <errl.h>
#include "sensor.h"
#include "rtls.h"
#include "occ_sys_config.h"
#include "occ_service_codes.h"  // for SSX_GENERIC_FAILURE
#include "dcom.h"
#include "proc_data.h"
#include "proc_data_control.h"
#include "amec_smh.h"
#include "amec_slave_smh.h"
#include <trac.h>
#include "amec_sys.h"
#include "sensor_enum.h"
#include "amec_service_codes.h"
#include <amec_sensors_core.h>
#include <amec_sensors_power.h>
#include <amec_sensors_fw.h>
#include <amec_data.h>
#include <amec_freq.h>
#include "pss_constants.h"
#include <memory.h>
#include "common.h"
#include <pstates_occ.H>

//*************************************************************************
// Externs
//*************************************************************************
extern uint8_t G_membuf_temp_expired_bitmap;
extern dimm_sensor_flags_t G_dimm_temp_expired_bitmap;

extern uint16_t G_proc_fmax_mhz;

extern uint16_t G_allow_trace_flags;
extern bool G_allowPstates;
extern OCCPstateParmBlock_t G_oppb;

//*************************************************************************
// Defines/Enums
//*************************************************************************

//*************************************************************************
// Structures
//*************************************************************************

//*************************************************************************
// Globals
//*************************************************************************
BOOLEAN                   G_non_dps_power_limited       = FALSE;

opal_proc_voting_reason_t G_amec_opal_proc_throt_reason = NO_THROTTLE;
opal_mem_voting_reason_t  G_amec_opal_mem_throt_reason  = NO_MEM_THROTTLE;


//*************************************************************************
// Function Prototypes
//*************************************************************************

//*************************************************************************
// Functions
//*************************************************************************


// Function Specification
//
// Name: amec_set_freq_range
//
// Description: Set the frequency range for AMEC based on mode only
//              NOTE:  Any other clipping of frequency should be done in
//                     amec_slv_proc_voting_box() (called every tick)
//                     so the CLIP history in poll response will accurately
//                     show all clipping for the given mode the system is in
//              This function will run on mode changes and cnfg_data changes
//
// Thread: RealTime Loop
//
// Task Flags:
//
// End Function Specification
errlHndl_t amec_set_freq_range(const OCC_MODE i_mode)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    errlHndl_t                  l_err = NULL;
    uint16_t                    l_freq_min  = 0;
    uint32_t                    l_freq_min_at_max_throttle  = 0;
    uint16_t                    l_freq_max  = 0;
    uint32_t                    l_steps = 0;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    // freq_min is the lowest frequency the processers actually run at
    l_freq_min = G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MIN_FREQ];
    // power capping will allow frequencies into throttle space (even through
    // the processor frequency will never really drop below fmin)
    l_freq_min_at_max_throttle = proc_pstate2freq(G_oppb.pstate_max_throttle, &l_steps);
    if (l_steps)
    {
        // adjust min frequency by convert l_steps into frequency in kHz
        l_freq_min_at_max_throttle -= l_steps * G_oppb.frequency_step_khz;
        // convert to Mhz
        l_freq_min_at_max_throttle /= 1000;
    }

    // Determine Max Freq for this mode
    // if no mode set yet or running with OPAL set to the full range
    if( (i_mode == OCC_MODE_NOCHANGE) || (G_sysConfigData.system_type.kvm) )
    {
        // Set Max frequency to WOF base if wof and Fmax is off, otherwise max possible
        if( (g_amec->wof.wof_disabled || (g_amec->wof.wof_init_state != WOF_ENABLED)) &&
            (!G_sysConfigData.fmax_mode_enable) )
        {
            l_freq_max = G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_WOF_BASE];
        }
        else
        {
            l_freq_max = G_proc_fmax_mhz;
        }
    }
    else  // Set max based on specific mode
    {

      switch(i_mode)
      {
          case OCC_MODE_DISABLED:
              l_freq_max = G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MODE_DISABLED];
              break;

          case OCC_MODE_STATIC_FREQ_POINT:
          case OCC_MODE_FFO:
              // frequency was set by user in the mode parameter and verified when mode was set
              l_freq_max = G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MODE_USER];
              break;

          case OCC_MODE_PWRSAVE:
              l_freq_max = G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MODE_PWR_SAVE];
              break;

          case OCC_MODE_DYN_PERF:
              l_freq_max = G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MODE_DYN_PERF];
              break;

          case OCC_MODE_MAX_PERF:
              l_freq_max = G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MODE_MAX_PERF];
              break;

          case OCC_MODE_FMAX:
              l_freq_max = G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MODE_FMAX];
              break;

          default: // should never happen
              TRAC_ERR("amec_set_freq_range unknown mode[%d]", i_mode);
              break;
      } // switch i_mode

      // Verify that WOF is running for performance modes
      if( ( (i_mode == OCC_MODE_DYN_PERF) || (i_mode == OCC_MODE_MAX_PERF) ) &&
          ( g_amec->wof.wof_disabled || (g_amec->wof.wof_init_state != WOF_ENABLED) ) )
      {
          // clip to WOF base since WOF is disabled
          l_freq_max = G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_WOF_BASE];
      }
    }

    if( (l_freq_min == 0) || (l_freq_max == 0) || (l_freq_min_at_max_throttle == 0))
    {
      // Do not update amec vars with a 0 frequency.
      // The frequency limit for each mode should have been set prior
      // to calling or the mode passed was invalid
      TRAC_ERR("amec_set_freq_range: Freq of 0 found! mode[0x%02x] Fmin[%u] Fmax[%u] Fmin@maxThrot[%u]",
                  i_mode,
                  l_freq_min,
                  l_freq_max,
                  l_freq_min_at_max_throttle);

      // Log an error if this is PowerVM as this should never happen when OCC
      // supports modes
      if(!G_sysConfigData.system_type.kvm)
      {
        /* @
         * @errortype
         * @moduleid    AMEC_SET_FREQ_RANGE
         * @reasoncode  INTERNAL_FW_FAILURE
         * @userdata1   Mode
         * @userdata2   0
         * @userdata4   ERC_FW_ZERO_FREQ_LIMIT
         * @devdesc     Fmin or Fmax of 0 found for mode
         */
         errlHndl_t l_err = createErrl(AMEC_SET_FREQ_RANGE,           //modId
                                       INTERNAL_FW_FAILURE,           //reasoncode
                                       ERC_FW_ZERO_FREQ_LIMIT,        //Extended reason code
                                       ERRL_SEV_PREDICTIVE,           //Severity
                                       NULL,                          //Trace Buf
                                       DEFAULT_TRACE_SIZE,            //Trace Size
                                       i_mode,                        //userdata1
                                       0);                            //userdata2

         // Callout Firmware
         addCalloutToErrl(l_err,
                          ERRL_CALLOUT_TYPE_COMPONENT_ID,
                          ERRL_COMPONENT_ID_FIRMWARE,
                          ERRL_CALLOUT_PRIORITY_LOW
                          );
      }
    }
    else
    {
        uint32_t l_steps = 0;
        uint8_t l_pstate = proc_freq2pstate(l_freq_max, &l_steps);
        // add the steps to the pstate to reflect the actual Fmax Pstate
        l_pstate += l_steps;

        g_amec->sys.fmin_max_throttled = l_freq_min_at_max_throttle;
        g_amec->sys.fmax = l_freq_max;

        TRAC_INFO("amec_set_freq_range: Mode[0x%02x] Fmin[%u/0x%02X] Fmax[%u/0x%02X]",
                  i_mode,
                  l_freq_min,
                  G_oppb.pstate_min,
                  l_freq_max,
                  l_pstate);
        TRAC_INFO("amec_set_freq_range:   Fmin@MaxThrot[%u/0x%02X]",
                  l_freq_min_at_max_throttle,
                  G_oppb.pstate_max_throttle);
    }
    return l_err;
}

// Function Specification
//
// Name: amec_slv_proc_voting_box
//
// Description: Slave OCC's voting box that decides the frequency request.
//              This function will run every tick.
//
// Thread: RealTime Loop
//
// Task Flags:
//
// End Function Specification
void amec_slv_proc_voting_box(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    uint16_t                        k = 0;
    uint16_t                        l_chip_fmax = g_amec->sys.fmax;
    uint16_t                        l_core_freq = 0;
    uint16_t                        l_core_freq_max = 0;  // max freq across all cores
    uint16_t                        l_core_freq_min = g_amec->sys.fmax;  // min freq across all cores
    uint32_t                        l_current_reason = 0;  // used for debug purposes
    static uint32_t                 L_last_reason = 0;     // used for debug purposes
    uint32_t                        l_chip_reason = 0;
    uint32_t                        l_core_reason = 0;
    amec_proc_voting_reason_t       l_kvm_throt_reason = NO_THROTTLE;
    amec_part_t                     *l_part = NULL;
    uint32_t                        l_steps = 0;
    uint32_t                        l_core_freq_kHz = 0;
    uint32_t                        l_steps_in_freq = 0;

    // frequency threshold for reporting throttling
    OCC_FREQ_POINT l_freq_point = OCC_FREQ_PT_WOF_BASE;
    if ( CURRENT_MODE() && G_sysConfigData.system_type.kvm )
    {
        if (CURRENT_MODE() == OCC_MODE_DISABLED)
            l_freq_point = OCC_FREQ_PT_MODE_DISABLED;
        else if (CURRENT_MODE() == OCC_MODE_PWRSAVE)
            l_freq_point = OCC_FREQ_PT_MODE_PWR_SAVE;
        else if ((CURRENT_MODE() == OCC_MODE_STATIC_FREQ_POINT) || (CURRENT_MODE() == OCC_MODE_FFO))
            l_freq_point = OCC_FREQ_PT_MODE_USER;
    }
    const uint16_t l_report_throttle_freq = G_sysConfigData.sys_mode_freq.table[l_freq_point];

    if (!G_allowPstates)
    {
        // Don't allow pstates to be sent until after initial mode has been set
        if ( (CURRENT_MODE()) || (G_sysConfigData.system_type.kvm) )
        {
            G_allowPstates = TRUE;
        }
    }

    // Voting Box for CPU speed.
    // This function implements the voting box to decide which input gets the right
    // to actuate the system.

    // PPB_FMAX
    if(g_amec->proc[0].pwr_votes.ppb_fmax < l_chip_fmax)
    {
        l_chip_fmax = g_amec->proc[0].pwr_votes.ppb_fmax;
        l_chip_reason = AMEC_VOTING_REASON_PPB;

        if(l_report_throttle_freq <= l_chip_fmax)
        {
            l_kvm_throt_reason = PCAP_EXCEED_REPORT;
        }
        else
        {
            l_kvm_throt_reason = POWERCAP;
        }
    }

    // PMAX_CLIP_FREQ
    if(g_amec->proc[0].pwr_votes.pmax_clip_freq < l_chip_fmax)
    {
        l_chip_fmax = g_amec->proc[0].pwr_votes.pmax_clip_freq;
        l_chip_reason = AMEC_VOTING_REASON_PMAX;
        l_kvm_throt_reason = POWER_SUPPLY_FAILURE;
    }

    // Pmax_clip frequency request if there is an APSS failure
    if(g_amec->proc[0].pwr_votes.apss_pmax_clip_freq < l_chip_fmax)
    {
        l_chip_fmax = g_amec->proc[0].pwr_votes.apss_pmax_clip_freq;
        l_chip_reason = AMEC_VOTING_REASON_APSS_PMAX;
        l_kvm_throt_reason = POWER_SUPPLY_FAILURE;
    }

    //THERMALPROC.FREQ_REQUEST
    //Thermal controller input based on processor temperature
    if(g_amec->thermalproc.freq_request < l_chip_fmax)
    {
        l_chip_fmax = g_amec->thermalproc.freq_request;
        l_chip_reason = AMEC_VOTING_REASON_PROC_THRM;

        if( l_report_throttle_freq <= l_chip_fmax)
        {
            l_kvm_throt_reason = PROC_OVERTEMP_EXCEED_REPORT;
        }
        else
        {
            // report DVFS due to proc OT in poll response
            l_kvm_throt_reason = CPU_OVERTEMP;
        }
    }

    //Thermal controller input based on VRM Vdd temperature
    if(g_amec->thermalvdd.freq_request < l_chip_fmax)
    {
        l_chip_fmax = g_amec->thermalvdd.freq_request;
        l_chip_reason = AMEC_VOTING_REASON_VDD_THRM;

        if( l_report_throttle_freq <= l_chip_fmax)
        {
            l_kvm_throt_reason = VDD_OVERTEMP_EXCEED_REPORT;
        }
        else
        {
            l_kvm_throt_reason = VDD_OVERTEMP;
        }
    }

    // Overcurrent protection when WOF is disabled
    if(g_amec->oc_wof_off.freq_request < l_chip_fmax)
    {
        l_chip_fmax = g_amec->oc_wof_off.freq_request;
        l_chip_reason = AMEC_VOTING_REASON_OVER_CURRENT;
        l_kvm_throt_reason = OVERCURRENT;
    }

    for (k=0; k<MAX_NUM_CORES; k++)
    {
        if( CORE_PRESENT(k) && !CORE_OFFLINE(k) )
        {
            l_core_freq = l_chip_fmax;
            l_core_reason = l_chip_reason;

            // Disable DPS in KVM
            if(!G_sysConfigData.system_type.kvm)
            {
                l_part = amec_part_find_by_core(&g_amec->part_config, k);

                // Check frequency request generated by DPS algorithms
                if(g_amec->proc[0].core[k].core_perf.dps_freq_request < l_core_freq)
                {
                    l_core_freq = g_amec->proc[0].core[k].core_perf.dps_freq_request;
                    l_core_reason = AMEC_VOTING_REASON_UTIL;
                }

                // Adjust frequency based on soft frequency boundaries
                if(l_part != NULL)
                {
                    if(l_core_freq < l_part->soft_fmin)
                    {
                        // Before enforcing a soft Fmin, make sure we don't
                        // have a thermal or power emergency
                        if(!(l_chip_reason & (AMEC_VOTING_REASON_PROC_THRM |
                                              AMEC_VOTING_REASON_VDD_THRM |
                                              AMEC_VOTING_REASON_PPB |
                                              AMEC_VOTING_REASON_PMAX |
                                              AMEC_VOTING_REASON_CONN_OC)))
                        {
                            l_core_freq = l_part->soft_fmin;
                            l_core_reason = AMEC_VOTING_REASON_SOFT_MIN;
                        }
                    }
                    else if(l_core_freq > l_part->soft_fmax)
                    {
                        l_core_freq = l_part->soft_fmax;
                        l_core_reason = AMEC_VOTING_REASON_SOFT_MAX;
                    }
                }
            }

// Ignore proc pcap vote in order to keep frequency same across all processors in a
// node power cap will be handled via the ppb_fmax that is calculated by master OCC
#ifdef POWER_CAP_VOTE
            if(CURRENT_MODE() == OCC_MODE_DISABLED)
            {
                // PROC_PCAP_NOM_VOTE
                if(g_amec->proc[0].pwr_votes.proc_pcap_nom_vote < l_core_freq)
                {
                    l_core_freq = g_amec->proc[0].pwr_votes.proc_pcap_nom_vote;
                    l_core_reason = AMEC_VOTING_REASON_PWR;
                    l_kvm_throt_reason = POWERCAP;
                }
            }
            else
            {
                // PROC_PCAP_VOTE
                if(g_amec->proc[0].pwr_votes.proc_pcap_vote < l_core_freq)
                {
                    l_core_freq = g_amec->proc[0].pwr_votes.proc_pcap_vote;
                    l_core_reason = AMEC_VOTING_REASON_PWR;

                    if(l_report_throttle_freq <= l_core_freq)
                    {
                        l_kvm_throt_reason = PCAP_EXCEED_REPORT;
                    }
                    else
                    {
                        l_kvm_throt_reason = POWERCAP;
                    }
                }
            }
#endif

            // Check IPS frequency request sent by Master OCC
            if(g_amec->slv_ips_freq_request != 0)
            {
                if(g_amec->slv_ips_freq_request < l_core_freq)
                {
                    l_core_freq = g_amec->slv_ips_freq_request;
                    l_core_reason = AMEC_VOTING_REASON_IPS;
                }
            }

            // Override frequency with request from Master OCC from mfg auto slew
            if(g_amec->poverride_enable)
            {
                if(g_amec->poverride != 0xff)
                {
                    // Override the Pstate on all cores if Master OCC sends
                    // a non-0xff Pstate request
                    l_core_freq_kHz = proc_pstate2freq(g_amec->poverride, &l_steps);
                    // check if entered into throttle space
                    if(l_steps)
                    {
                        // convert l_steps into frequency in kHz
                        l_steps_in_freq = l_steps * G_oppb.frequency_step_khz;
                        // subtract off the additional frequency to get into throttle space
                        if(l_steps_in_freq <= l_core_freq_kHz)  // prevent going negative
                            l_core_freq_kHz -= l_steps_in_freq;
                        else
                            l_core_freq_kHz = 1000;  // can't use 0, that indicates core not online
                    }
                    // proc_pstate2_freq returns frequency in kHz, need to convert to mHz for l_core_freq
                    l_core_freq = l_core_freq_kHz / 1000;
                    l_core_reason = AMEC_VOTING_REASON_OVERRIDE;
                }
                l_kvm_throt_reason = MANUFACTURING_OVERRIDE;
            }

            // Override frequency via Amester parameter interface
            if (g_amec->proc[0].parm_f_override_enable &&
                g_amec->proc[0].parm_f_override[k] > 0)
            {
                l_core_freq = g_amec->proc[0].parm_f_override[k];
                l_core_reason = AMEC_VOTING_REASON_OVERRIDE_CORE;
            }

            //STORE core frequency and reason
            g_amec->proc[0].core[k].f_request = l_core_freq;
            g_amec->proc[0].core[k].f_reason = l_core_reason;
            if(l_core_freq < l_core_freq_min)
            {
                // store the new lowest frequency and reason to be used after all cores checked
                l_core_freq_min = l_core_freq;
                l_current_reason = l_core_reason;
            }

            // Update the Amester parameter telling us the reason. Needed for
            // parameter array.
            g_amec->proc[0].parm_f_reason[k] = l_core_reason;

            //CURRENT_MODE() may be OCC_MODE_NOCHANGE because STATE change is processed
            //before MODE change
            if ((CURRENT_MODE() != OCC_MODE_DYN_PERF)   &&
                (CURRENT_MODE() != OCC_MODE_MAX_PERF)   &&
                (CURRENT_MODE() != OCC_MODE_FFO)               &&
                (CURRENT_MODE() != OCC_MODE_NOCHANGE)          &&
                (l_core_reason & NON_DPS_POWER_LIMITED))
            {
                G_non_dps_power_limited = TRUE;
            }
            else
            {
                G_non_dps_power_limited = FALSE;
            }

#if DEBUG_PROC_VOTING_BOX
            /// This trace that can be used to debug the voting
            /// box and control loops.  It will trace the reason why a
            /// controller is lowering the freq, but will only do it once in a
            /// row for the specific freq it wants to control to.  It assumes
            /// that all cores will be controlled to same freq.
            if(l_chip_fmax != g_amec->sys.fmax){
                static uint16_t L_trace = 0;
                if(l_chip_fmax != L_trace){
                    L_trace = l_chip_fmax;
                    TRAC_INFO("Core: %d, Freq: %d, Reason: %d",k,l_core_freq,l_core_reason);
                }
            }
#endif

            if(l_core_freq > l_core_freq_max)
            {
                l_core_freq_max = l_core_freq;
            }
        } // if core present and not offline
        else
        {
            //Set f_request to 0 so this core is ignored in amec_slv_freq_smh()
            g_amec->proc[0].core[k].f_request = 0;
            g_amec->proc[0].core[k].f_reason = 0;
        }
    }//End of for loop

    // update max core frequency if not 0 i.e. all cores offline (stop 2 or greater)
    // this is used by power capping alg, updating to 0 will cause power throttling when not needed
    if(l_core_freq_max)
    {
        g_amec->proc[0].core_max_freq = l_core_freq_max;
        // update the overall reason driving frequency across all cores
        g_amec->proc[0].f_reason = l_current_reason;
    }

    //check if there was a throttle reason change
    if(l_kvm_throt_reason != G_amec_opal_proc_throt_reason)
    {
        //Always update G_amec_opal_proc_throt_reason, this is used to set poll rsp bits for all system types
        G_amec_opal_proc_throt_reason = l_kvm_throt_reason;

        // Only if running OPAL need to notify dcom thread to update the table in HOMER for OPAL
        if(G_sysConfigData.system_type.kvm)
        {
            ssx_semaphore_post(&G_dcomThreadWakeupSem);
        }
    }

    // Update the Processor Throttle due to power sensor
    if( (l_kvm_throt_reason == PCAP_EXCEED_REPORT) ||
        (l_kvm_throt_reason == POWERCAP) )
    {
         // Frequency is being throttled due to power cap
         sensor_update(AMECSENSOR_PTR(PROCPWRTHROT), 1);
    }
    else  // not currently throttled due to power
    {
         sensor_update(AMECSENSOR_PTR(PROCPWRTHROT), 0);
    }

    // For debug... if lower than max update vars returned in poll response to give clipping reason
    g_amec->proc[0].core_min_freq = l_core_freq_min;
    if(l_core_freq_min < g_amec->sys.fmax)
    {
        if(l_current_reason == L_last_reason)
        {
            // same reason INC counter
            if(g_amec->proc[0].current_clip_count != 0xFF)
            {
                 g_amec->proc[0].current_clip_count++;
            }
        }
        else
        {
            // new reason update history and set counter to 1
            L_last_reason = l_current_reason;
            g_amec->proc[0].current_clip_count = 1;
            if( (g_amec->proc[0].chip_f_reason_history & l_current_reason) == 0)
            {
                g_amec->proc[0].chip_f_reason_history |= l_current_reason;
                TRAC_IMP("First time throttling for reason[0x%08X] History[0x%08X] freq = %d",
                          l_current_reason, g_amec->proc[0].chip_f_reason_history, l_core_freq_min);
            }
        }
    }
    else // no active clipping
    {
        L_last_reason = 0;
        g_amec->proc[0].current_clip_count = 0;
    }
}

// Function Specification
//
// Name: amec_slv_freq_smh
//
// Description: Slave OCC's frequency state machine.
//              This function will run every tick.
//
// Thread: RealTime Loop
//
// Task Flags:
//
// End Function Specification
void amec_slv_freq_smh(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    uint8_t     core_num = 0;   // loop through cores
    uint8_t     core_pmax = 0;
    Pstate_t    pmax_chip = 0;  // highest Pstate (lowest frequency) across all cores

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    // loop through all cores, get f_requests, translate to pstates and determine pmax across chip
    for (core_num=0; core_num<MAX_CORES; core_num++)  // loop thru all cores
    {
        // ignore core if freq request is 0 (core not present when amec_slv_proc_voting_box ran)
        if(g_amec->proc[0].core[core_num].f_request != 0)
        {
           uint32_t l_steps = 0;
           core_pmax = proc_freq2pstate(g_amec->proc[0].core[core_num].f_request, &l_steps);
           // add the throttle steps to the pstate
           core_pmax += l_steps;

           // The higher the pstate number, the lower the frequency and increasing throttle
           if(pmax_chip < core_pmax)
           {
               pmax_chip = core_pmax;
           }
        }
    }
    if( (G_desired_pstate != pmax_chip) &&
        (G_allow_trace_flags & ALLOW_AMEC_PSTATE_TRACE) )
    {
        TRAC_IMP("Updating chip Pstate from %d to %d",
                 G_desired_pstate, pmax_chip);
    }
    // update pstate request
    G_desired_pstate = pmax_chip;
}


// Function Specification
//
// Name: amec_slv_mem_voting_box
//
// Description: Slave OCC's voting box that decides the memory speed request.
//              This function will run every tick.
//
// Thread: RealTime Loop
//
// Task Flags:
//
// End Function Specification
void amec_slv_mem_voting_box(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    UINT16                   l_vote;
    amec_mem_voting_reason_t l_reason;
    opal_mem_voting_reason_t  kvm_reason;
    static INT16             l_slew_step = AMEC_MEMORY_STEP_SIZE;
    static bool              L_throttle_traced = FALSE;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    // Start with max allowed speed
    l_vote     = AMEC_MEMORY_MAX_STEP;
    l_reason   = AMEC_MEM_VOTING_REASON_INIT;
    kvm_reason = NO_THROTTLE;

    // Memory throttled due to power cap.  if also throttled due to
    // over temp, report over-temp as the reason to OPAL.
    if (g_amec->pcap.active_mem_level != 0)
    {
        kvm_reason = POWER_CAP;
    }

    // Check vote from membuf thermal control loop
    if (l_vote > g_amec->thermalmembuf.speed_request)
    {
        l_vote = g_amec->thermalmembuf.speed_request;
        l_reason = AMEC_MEM_VOTING_REASON_MEMBUF;
        kvm_reason = MEMORY_OVER_TEMP;
    }

    // Check vote from DIMM thermal control loop
    if (l_vote > g_amec->thermaldimm.speed_request)
    {
        l_vote = g_amec->thermaldimm.speed_request;
        l_reason = AMEC_MEM_VOTING_REASON_DIMM;
        kvm_reason = MEMORY_OVER_TEMP;
    }

    // Check vote from Mem ctrl+DIMM thermal control loop
    if (l_vote > g_amec->thermalmcdimm.speed_request)
    {
        l_vote = g_amec->thermalmcdimm.speed_request;
        l_reason = AMEC_MEM_VOTING_REASON_MCDIMM;
        kvm_reason = MEMORY_OVER_TEMP;
    }

    // Check vote from Pmic thermal control loop
    if (l_vote > g_amec->thermalpmic.speed_request)
    {
        l_vote = g_amec->thermalpmic.speed_request;
        l_reason = AMEC_MEM_VOTING_REASON_PMIC;
        kvm_reason = MEMORY_OVER_TEMP;
    }

    // Check vote from external mem controller thermal control loop
    if (l_vote > g_amec->thermalmcext.speed_request)
    {
        l_vote = g_amec->thermalmcext.speed_request;
        l_reason = AMEC_MEM_VOTING_REASON_MC_EXT;
        kvm_reason = MEMORY_OVER_TEMP;
    }

    // Check if memory autoslewing is enabled
    if (g_amec->mnfg_parms.mem_autoslew)
    {
        //check if we've reached the max setting and need to start going down
        if(g_amec->mem_speed_request >= AMEC_MEMORY_MAX_STEP)
        {
            g_amec->mnfg_parms.mem_slew_counter++;
            l_slew_step = -AMEC_MEMORY_STEP_SIZE;
        }

        //check if we've reached the min setting and need to start going up
        else if(g_amec->mem_speed_request <= AMEC_MEMORY_MIN_STEP)
        {
            g_amec->mnfg_parms.mem_slew_counter++;
            l_slew_step = AMEC_MEMORY_STEP_SIZE;
        }

        l_vote = g_amec->mem_speed_request + l_slew_step;
        l_reason = AMEC_MEM_VOTING_REASON_SLEW;
    }

    // Store final vote and vote reason in g_amec
    g_amec->mem_throttle_reason = l_reason;
    g_amec->mem_speed_request = l_vote;

    //trace changes in memory throttling
    if(l_reason != AMEC_MEM_VOTING_REASON_INIT)
    {
        if(!L_throttle_traced)
        {
            L_throttle_traced = TRUE;
            TRAC_INFO("Memory is being throttled. reason[%d] vote[%d] "
                      "membuf_expired[0x%02x] dimm_expired[0x%08x%08x]",
                       l_reason,
                       l_vote,
                       G_membuf_temp_expired_bitmap,
                       (uint32_t)(G_dimm_temp_expired_bitmap.dw[0] >> 32),
                       (uint32_t)G_dimm_temp_expired_bitmap.dw[0]);
        }
    }
    else
    {
        if(L_throttle_traced)
        {
            L_throttle_traced = FALSE;
            TRAC_INFO("Memory is no longer being throttled");
        }
    }

    //check if we need to update the throttle reason in OPAL table
    if(G_sysConfigData.system_type.kvm &&
       (kvm_reason != G_amec_opal_mem_throt_reason))
    {
        //Notify dcom thread to update the table
        G_amec_opal_mem_throt_reason = kvm_reason;
        ssx_semaphore_post(&G_dcomThreadWakeupSem);
    }

    return;
}

// Function Specification
//
// Name: amec_slv_check_perf
//
// Description: Slave OCC's Detect and log degraded performance errors
//              This function will run every tick.
//
// Thread: RealTime Loop
//
// Task Flags:
//
// End Function Specification
void amec_slv_check_perf(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    static BOOLEAN          l_prev_ovs_state = FALSE;
    static BOOLEAN          l_prev_pcap_state = FALSE;
    static ERRL_SEVERITY    l_pcap_sev =  ERRL_SEV_PREDICTIVE;
    static BOOLEAN          l_throttle_traced = FALSE;
    static uint64_t         l_time = 0;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    do
    {
        // was frequency limited by power ?
        if ( G_non_dps_power_limited != TRUE )
        {
            if(l_throttle_traced)
            {
                TRAC_INFO("Frequency not limited by power algorithms anymore");
                l_throttle_traced = FALSE;
            }
            // we are done break and return
            break;
        }

        // frequency limited due to oversubscription condition ?
        if ( AMEC_INTF_GET_OVERSUBSCRIPTION() == TRUE )
        {
            if ( l_prev_ovs_state == TRUE)
            {
                // we are done break and return
                break;
            }
            else
            {
                // log this error ONLY ONCE per IPL
                l_prev_ovs_state = TRUE;

                TRAC_ERR("Frequency limited due to oversubscription condition(mode:%d, state:%d)",
                          CURRENT_MODE(), CURRENT_STATE());
                l_throttle_traced = TRUE;
                l_time = ssx_timebase_get();

                // log error that calls out OVS procedure
                // set error severity to RRL_SEV_PREDICTIVE

                // Updated the RC to match the actual RC passed to createErrl()
                /* @
                 * @errortype
                 * @moduleid    AMEC_SLAVE_CHECK_PERFORMANCE
                 * @reasoncode  OVERSUB_LIMIT_ALERT
                 * @userdata1   Previous OVS State
                 * @userdata4   ERC_AMEC_SLAVE_OVS_STATE
                 * @devdesc     Frequency limited due to oversubscription condition
                 */
                errlHndl_t l_errl = createErrl(AMEC_SLAVE_CHECK_PERFORMANCE, //modId
                                              OVERSUB_LIMIT_ALERT,           //reasoncode
                                              ERC_AMEC_SLAVE_OVS_STATE,      //Extended reason code
                                              ERRL_SEV_PREDICTIVE,           //Severity
                                              NULL,                          //Trace Buf
                                              DEFAULT_TRACE_SIZE,            //Trace Size
                                              l_prev_ovs_state,              //userdata1
                                              0);                            //userdata2

                // Callout to Oversubscription
                addCalloutToErrl(   l_errl,
                                    ERRL_CALLOUT_TYPE_COMPONENT_ID,
                                    ERRL_COMPONENT_ID_OVERSUBSCRIPTION,
                                    ERRL_CALLOUT_PRIORITY_HIGH
                                );

                // Callout to APSS
                addCalloutToErrl(   l_errl,
                                    ERRL_CALLOUT_TYPE_HUID,
                                    G_sysConfigData.apss_huid,
                                    ERRL_CALLOUT_PRIORITY_MED
                                );

                // Callout to Firmware
                addCalloutToErrl(   l_errl,
                                    ERRL_CALLOUT_TYPE_COMPONENT_ID,
                                    ERRL_COMPONENT_ID_FIRMWARE,
                                    ERRL_CALLOUT_PRIORITY_LOW
                                );

                // Commit Error
                commitErrl(&l_errl);

                // we are done lets break
                break;
            }
        }

        uint16_t l_snrBulkPwr = AMECSENSOR_PTR(PWRSYS)->sample;

        // frequency limited due to system power cap condition ?
        if (( l_snrBulkPwr > (G_sysConfigData.pcap.system_pcap - PDROP_THRESH) )
            &&
            ( G_sysConfigData.pcap.current_pcap == 0 ))
        {
            if ( l_prev_pcap_state == TRUE)
            {
                // we are done break and return
                break;
            }
            else
            {
                //log this error ONLY ONCE per IPL
                l_prev_pcap_state = TRUE;

                TRAC_ERR("Frequency limited due to power cap condition(mode:%d, state:%d)",
                         CURRENT_MODE(), CURRENT_STATE());

                TRAC_ERR("SnrBulkPwr %d > Sys Pcap %d ",l_snrBulkPwr,
                         G_sysConfigData.pcap.system_pcap );

                TRAC_ERR("SnrGpuPrw %d ",
                        AMECSENSOR_PTR(PWRGPU)->sample );

                TRAC_ERR("SnrProcPwr 0 %d, SnrProcPwr 1 %d, SnrProcPwr 2 %d, SnrProcPwr 3 %d",
                        g_amec->proc_snr_pwr[0],
                        g_amec->proc_snr_pwr[1],
                        g_amec->proc_snr_pwr[2],
                        g_amec->proc_snr_pwr[3] );

                TRAC_ERR("SnrMemPwr 0 %d, SnrMemPwr 1 %d, SnrMemPwr 2 %d, SnrMemPwr 3 %d",
                        g_amec->mem_snr_pwr[0],
                        g_amec->mem_snr_pwr[1],
                        g_amec->mem_snr_pwr[2],
                        g_amec->mem_snr_pwr[3] );


                l_throttle_traced = TRUE;
                l_time = ssx_timebase_get();

                // log error that calls out firmware and APSS procedure
                // set error severity to l_pcap_sev

                /* @
                 * @errortype
                 * @moduleid    AMEC_SLAVE_CHECK_PERFORMANCE
                 * @reasoncode  PCAP_THROTTLE_POWER_LIMIT
                 * @userdata1   Current Sensor Bulk Power
                 * @userdata2   System PCAP
                 * @userdata4   ERC_AMEC_SLAVE_POWERCAP
                 * @devdesc     Frequency limited due to PowerCap  condition
                 */
                errlHndl_t l_errl = createErrl(AMEC_SLAVE_CHECK_PERFORMANCE, //modId
                                              PCAP_THROTTLE_POWER_LIMIT,     //reasoncode
                                              ERC_AMEC_SLAVE_POWERCAP,       //Extended reason code
                                              l_pcap_sev,                    //Severity
                                              NULL,                          //Trace Buf
                                              DEFAULT_TRACE_SIZE,            //Trace Size
                                              l_snrBulkPwr,                  //userdata1
                                              G_sysConfigData.pcap.system_pcap);//userdata2

                addCalloutToErrl(   l_errl,
                                    ERRL_CALLOUT_TYPE_COMPONENT_ID,
                                    ERRL_COMPONENT_ID_FIRMWARE,
                                    ERRL_CALLOUT_PRIORITY_HIGH
                                );

                addCalloutToErrl(   l_errl,
                                    ERRL_CALLOUT_TYPE_HUID,
                                    G_sysConfigData.apss_huid,
                                    ERRL_CALLOUT_PRIORITY_HIGH
                                );

                // then l_pcap_sev to informational
                l_pcap_sev = ERRL_SEV_INFORMATIONAL;

                // Commit Error
                commitErrl(&l_errl);

                // we are done lets break
                break;
            }
        }

        // trottle trace to every 3600 seconds (1hr = 3600000)
        if(!l_throttle_traced && ( DURATION_IN_MS_UNTIL_NOW_FROM(l_time) > 3600000 ) )
        {
            TRAC_INFO("Frequency power limited due to transient condition: PowerLimited=%x, OverSubScription=%x CurrentBulkPwr=%x",
            G_non_dps_power_limited, AMEC_INTF_GET_OVERSUBSCRIPTION(), l_snrBulkPwr );
            l_throttle_traced = TRUE;

            l_time = ssx_timebase_get();
        }
    }
    while( 0 );

    return;
}


/*----------------------------------------------------------------------------*/
/* End                                                                        */
/*----------------------------------------------------------------------------*/
