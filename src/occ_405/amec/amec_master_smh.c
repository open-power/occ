/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/amec/amec_master_smh.c $                          */
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

//*************************************************************************/
// Includes
//*************************************************************************/
#include <occ_common.h>
#include <ssx.h>
#include <errl.h>               // Error logging
#include "rtls.h"
#include "occ_service_codes.h"  // for SSX_GENERIC_FAILURE
#include "sensor.h"
#include "amec_smh.h"
#include "amec_master_smh.h"
#include <trac.h>               // For traces
#include "amec_sys.h"
#include "amec_service_codes.h"
#include "dcom.h"
#include <amec_sensors_power.h>
#include <cmdh_fsp_cmds.h>      // For G_apss_ch_to_function
#include "common.h"             // For ignore_pgpe_error()

//*************************************************************************/
// Externs
//*************************************************************************/

//*************************************************************************/
// Macros
//*************************************************************************/

//*************************************************************************/
// Defines/Enums
//*************************************************************************/

//Power cap failure threshold with no GPUs set to 32 ticks
#define PCAP_FAILURE_THRESHOLD 32

//Power cap failure threshold with GPUs set to number of ticks for 2s
#define PCAP_GPU_FAILURE_THRESHOLD (2000000 / MICS_PER_TICK)

//*************************************************************************/
// Structures
//*************************************************************************/

//*************************************************************************/
// Globals
//*************************************************************************/
smh_state_t G_amec_mst_state = {AMEC_INITIAL_STATE,
                                AMEC_INITIAL_STATE,
                                AMEC_INITIAL_STATE};

//Array that stores active power cap values of all OCCs
slave_pcap_info_t G_slave_active_pcaps[MAX_OCCS] = {{0}};

//OCC over power cap count
uint8_t G_over_cap_count = 0;

//Array that stores the exit counts for the IPS algorithm
uint32_t G_ips_exit_count[MAX_OCCS][MAX_CORES] = {{0}};
//Array that stores the entry counts for the IPS algorithm
uint32_t G_ips_entry_count = 0;

//Soft Fmin to be sent to Slave OCCs
uint16_t G_mst_soft_fmin = 0;
//Soft Fmax to be sent to Slave OCCs
uint16_t G_mst_soft_fmax = 0xFFFF;
//Counter of committed violations by the Slave OCCs
uint8_t  G_mst_violation_cnt[MAX_OCCS] = {0};

extern uint32_t G_first_num_gpus_sys;

// --------------------------------------------------------
// AMEC Master State 5 Substate Table
// --------------------------------------------------------
// Each substate runs every 64th tick time = 64 * MICS_PER_TICK
//
const smh_tbl_t amec_mst_state_5_substate_table[AMEC_SMH_STATES_PER_LVL] =
{
  {amec_mst_substate_5_even, NULL}, // Substate 5.0
  {NULL,                     NULL}, // Substate 5.1 (not used)
  {amec_mst_substate_5_even, NULL}, // Substate 5.2
  {NULL,                     NULL}, // Substate 5.3 (not used)
  {amec_mst_substate_5_even, NULL}, // Substate 5.4
  {NULL,                     NULL}, // Substate 5.5 (not used)
  {amec_mst_substate_5_even, NULL}, // Substate 5.6
  {NULL,                     NULL}, // Substate 5.7 (not used)
};

// --------------------------------------------------------
// Main AMEC Master State Table
// --------------------------------------------------------
// Each function inside this state table runs once every MICS_PER_TICK * 8
//
const smh_tbl_t amec_mst_state_table[AMEC_SMH_STATES_PER_LVL] =
{
  {amec_mst_state_0, NULL},
  {amec_mst_state_1, NULL},
  {amec_mst_state_2, NULL},
  {amec_mst_state_3, NULL},
  {amec_mst_state_4, NULL},
  {amec_mst_state_5, amec_mst_state_5_substate_table},
  {amec_mst_state_6, NULL},
  {amec_mst_state_7, NULL},
};

// This sets up the function pointer that will be called to update the
// fw timings when the AMEC master State Machine finishes.
smh_state_timing_t G_amec_mst_state_timings = {amec_mst_update_smh_sensors};

//*************************************************************************/
// Function Prototypes
//*************************************************************************/

//*************************************************************************/
// Functions
//*************************************************************************/

// Function Specification
//
// Name: amec_mst_update_smh_sensors
//
// Description:  Update the sensor
//
// End Function Specification
void amec_mst_update_smh_sensors(int i_smh_state, uint32_t i_duration)
{
    // Update the duration in the fw timing table
    if(G_fw_timing.amess_state != i_smh_state)
    {
      AMEC_DBG("Mismatch between Master and Slave AMEC states\n");
    }

    G_fw_timing.amess_dur   += i_duration;
}

// Function Specification
//
// Name: amec_master_auto_slew
//
// Description: This function executes the auto-slewing of frequency based on
//              manufacturing parameters.
//
// Task Flags:
//
// End Function Specification
void amec_master_auto_slew(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    static int8_t               l_direction = 1;
    int16_t                     l_pstate = 0;
    static uint16_t             l_step_delay = 0;
    static bool                 l_first_time_enable = TRUE;
    static bool                 l_first_time_disable = FALSE;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    do
    {
        // Check if auto-slewing has been enabled
        if (g_amec->mnfg_parms.auto_slew)
        {
            // Is this the first time we are enabling auto-slewing?
            if (l_first_time_enable)
            {
                // Need to start the auto-slew from fmax
                g_amec->mnfg_parms.poverride = g_amec->mnfg_parms.pmax;

                // Reset the delay counter and reset the direction
                l_step_delay = g_amec->mnfg_parms.delay;
                l_direction = 1;

                l_first_time_enable = FALSE;
                l_first_time_disable = TRUE;

                // Break out now to give it some time to get to fmax
                break;
            }
        }
        else
        {
            // Is this the first time we are disabling auto-slewing?
            if (l_first_time_disable)
            {
                // Clear the frequency override parameter
                g_amec->mnfg_parms.poverride = 0xff;

                l_first_time_enable = TRUE;
                l_first_time_disable = FALSE;
            }

            // We are done, break
            break;
        }

        // If we've made it this far, then we are running auto-slew. First,
        // check if our delay counter has expired.
        if (l_step_delay != 0)
        {
            // Decrement our delay counter and exit
            l_step_delay--;
            break;
        }

        // Our delay counter has expired, reset it to its original value
        l_step_delay = g_amec->mnfg_parms.delay;

        // Now, generate a new frequency override in terms of a pState
        l_pstate = g_amec->mnfg_parms.poverride +
            (l_direction * g_amec->mnfg_parms.pstep);

        if (l_pstate >= g_amec->mnfg_parms.pmin)
        {
            // We've reached an edge, increment our counter
            g_amec->mnfg_parms.slew_counter++;

            // Change direction of the auto-slew
            l_direction = -l_direction;

            // Clip frequency to pmin
            l_pstate = g_amec->mnfg_parms.pmin;
        }
        else if (l_pstate <= g_amec->mnfg_parms.pmax)
        {
            // We've reached an edge, increment our counter
            g_amec->mnfg_parms.slew_counter++;

            // Change direction of the auto-slew
            l_direction = -l_direction;

            // Clip frequency to pmax
            l_pstate = g_amec->mnfg_parms.pmax;
        }
        g_amec->mnfg_parms.poverride = l_pstate;

    }while(0);

    return;
}

// Function Specification
//
// Name: amec_mst_check_under_pcap
//
// Description: This function checks if there is failure in maintaining power
//              cap value.
//
// End Function Specification

void amec_mst_check_under_pcap(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    errlHndl_t l_err = NULL;
    uint8_t i = 0;
    uint8_t l_apss_func_id = 0;
    uint32_t l_trace[MAX_APSS_ADC_CHANNELS] = {0};  // used to trace per channel data
    uint8_t l_trace_idx = 0;


    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    do
    {
        // Check if done everything possible to shed power and power still above a hard power cap
        // ppb_fmax = Fmin@MaxThrottle and PWRSYS > Node power cap and
        // Node power cap >=  hard_min_pcap
        if((g_amec->proc[0].pwr_votes.ppb_fmax == g_amec->sys.fmin_max_throttled) &&
           (AMECSENSOR_PTR(PWRSYS)->sample > g_amec->pcap.active_node_pcap) &&
           (g_amec->pcap.active_node_pcap >= G_sysConfigData.pcap.hard_min_pcap) )
        {
            // Check if we are to ignore pgpe errors meaning the PGPE cannot set frequency which could
            // cause this over power event.  This will not cover if a different OCC is not able to shed
            // power due to PGPE which would require to add this status to occ-occ communication
            if(ignore_pgpe_error())
            {
                // make sure count is cleared to give time for frequency to be set once PGPE can set it
                G_over_cap_count = 0;
                INCREMENT_ERR_HISTORY(ERRH_OVER_PCAP_IGNORED);
                break;
            }

            G_over_cap_count++;

            // GPUs take longer for power limit to take effect if GPUs are present need to use
            // a longer wait time before logging an error and resetting
            if( ( (!G_first_num_gpus_sys) && (G_over_cap_count >= PCAP_FAILURE_THRESHOLD) ) ||
                ( (G_first_num_gpus_sys) && (G_over_cap_count >= PCAP_GPU_FAILURE_THRESHOLD) ) )
            {
                TRAC_ERR("Failure to maintain power cap: Power Cap = %d ,"
                         "PWRSYS = %d",g_amec->pcap.active_node_pcap,
                         AMECSENSOR_PTR(PWRSYS)->sample);

                // Trace power per APSS channel to have the best breakdown for debug
                // compress traces to 4 max to save space on OP systems
                for (i = 0; i < MAX_APSS_ADC_CHANNELS; i++)
                {
                    l_apss_func_id = G_apss_ch_to_function[i];

                    if((l_apss_func_id != ADC_RESERVED) &&
                       (l_apss_func_id != ADC_12V_SENSE) &&
                       (l_apss_func_id != ADC_GND_REMOTE_SENSE) &&
                       (l_apss_func_id != ADC_12V_STANDBY_CURRENT) &&
                       (l_apss_func_id != ADC_VOLT_SENSE_2))
                    {
                        l_trace[l_trace_idx] = (i << 24) | (l_apss_func_id << 16) | (AMECSENSOR_PTR(PWRAPSSCH00 + i)->sample);
                        l_trace_idx++;
                    }
                }
                while(l_trace_idx != 0)
                {
                   if(l_trace_idx >=4)
                   {
                      TRAC_ERR("APSS channel/FuncID/Power: [%08X], [%08X], [%08X], [%08X]",
                                l_trace[l_trace_idx-1], l_trace[l_trace_idx-2], l_trace[l_trace_idx-3], l_trace[l_trace_idx-4]);
                      l_trace_idx -= 4;
                   }
                   else if(l_trace_idx == 3)
                   {
                      TRAC_ERR("APSS channel/FuncID/Power: [%08X], [%08X], [%08X]",
                                l_trace[l_trace_idx-1], l_trace[l_trace_idx-2], l_trace[l_trace_idx-3]);
                      l_trace_idx = 0;
                   }
                   else if(l_trace_idx == 2)
                   {
                      TRAC_ERR("APSS channel/FuncID/Power: [%08X], [%08X]",
                                 l_trace[l_trace_idx-1], l_trace[l_trace_idx-2]);
                      l_trace_idx = 0;
                   }
                   else // l_trace_idx == 1
                   {
                      TRAC_ERR("APSS channel/FuncID/Power: [%08X]",
                                l_trace[l_trace_idx-1]);
                      l_trace_idx = 0;
                   }
                }

                /* @
                 * @errortype
                 * @moduleid    AMEC_MST_CHECK_UNDER_PCAP
                 * @reasoncode  POWER_CAP_FAILURE
                 * @userdata1   Power Cap
                 * @userdata2   PWRSYS (Node Power)
                 * @devdesc     Failure to maintain max power limits
                 *
                 */
                l_err = createErrl( AMEC_MST_CHECK_UNDER_PCAP,
                                    POWER_CAP_FAILURE,
                                    ERC_AMEC_UNDER_PCAP_FAILURE,
                                    ERRL_SEV_PREDICTIVE,
                                    NULL,
                                    DEFAULT_TRACE_SIZE,
                                    g_amec->pcap.active_node_pcap,
                                    AMECSENSOR_PTR(PWRSYS)->sample);

                //Callout to firmware
                addCalloutToErrl(l_err,
                                 ERRL_CALLOUT_TYPE_COMPONENT_ID,
                                 ERRL_COMPONENT_ID_FIRMWARE,
                                 ERRL_CALLOUT_PRIORITY_HIGH);

                //Callout to APSS
                addCalloutToErrl(l_err,
                                 ERRL_CALLOUT_TYPE_HUID,
                                 G_sysConfigData.apss_huid,
                                 ERRL_CALLOUT_PRIORITY_HIGH);

                //Reset OCC
                REQUEST_RESET(l_err);
            }
        }
        else
        {
            // Clear counter
            G_over_cap_count = 0;
        }
    }while(0);
    return;
}

// Function Specification
//
// Name: amec_mst_ips_main
//
// Description: This function executes the Idle Power Saver (IPS)
//              algorithm.
//
// End Function Specification
void amec_mst_ips_main(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    uint16_t                    i = 0;
    uint16_t                    j = 0;
    uint16_t                    l_time_interval = 0;
    uint16_t                    l_core_count = 0;
    uint16_t                    l_entry_core_count = 0;
    BOOLEAN                     l_exit = FALSE;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    do
    {
        // If IPS is disabled, no need to execute the algorithm
        if (g_amec->mst_ips_parms.enable == 0)
        {
            // Reset the following parameters
            g_amec->mst_ips_parms.active = 0;
            G_ips_entry_count = 0;
            g_amec->mst_ips_parms.freq_request = 0; // No IPS Frequency request when IPS is disabled

            break;
        }

        // We made it here, so that means IPS is enabled.
        l_time_interval = AMEC_DPS_SAMPLING_RATE * AMEC_IPS_AVRG_INTERVAL;

        // If IPS is active, check if the exit criteria has been met
        if (g_amec->mst_ips_parms.active == 1)
        {
            // Useful trace for debugging
            //TRAC_INFO("DBG_IPS: IPS is active! exit_countp0c5[%u]",
            //          G_ips_exit_count[0][5]);

            for (i=0; i<MAX_OCCS; i++)
            {
                if (!(G_sysConfigData.is_occ_present & (1<<i)))
                {
                    // This OCC is not present, just move along
                    continue;
                }

                for (j=0; j<MAX_CORES; j++)
                {
                    // Check if this core's normalized utilization has exceeded
                    // the exit threshold
                    if (G_dcom_slv_outbox_rx[i].nutil3sp0cy[j] >=
                        g_amec->mst_ips_parms.exit_threshold)
                    {
                        // Increment our exit count by the number of samples
                        // used to average utilization (default is 3 seconds)
                        G_ips_exit_count[i][j] += l_time_interval;

                        if (G_ips_exit_count[i][j] >=
                            g_amec->mst_ips_parms.exit_delay)
                        {
                            // Found one core that meets the exit criteria!
                            l_exit = TRUE;
                            break;
                        }
                    }
                    else
                    {
                        // If the normalized utilization of this core is below
                        // the exit threshold, reset its exit count
                        G_ips_exit_count[i][j] = 0;
                    }
                }

                if (l_exit)
                {
                    break;
                }
            }

            // If we found a core that meets the exit criteria, then exit IPS
            // active state
            if (l_exit)
            {
                g_amec->mst_ips_parms.active = 0;
                G_ips_entry_count = 0;
                // A special request of 0 frequency will inform the slaves to
                // ignore the IPS request from the master
                g_amec->mst_ips_parms.freq_request = 0;

                TRAC_INFO("amec_mst_ips_main: We have exited IPS active state! exit_count[%u] freq_request[0]",
                          G_ips_exit_count[i][j]);
            }
        }
        else //IPS is inactive, check if entry criteria has been met
        {
            // Useful trace for debugging
            //TRAC_INFO("DBG_IPS: IPS is inactive! entry_count[%u]",
            //          G_ips_entry_count);

            for (i=0; i<MAX_OCCS; i++)
            {
                if (!(G_sysConfigData.is_occ_present & (1<<i)))
                {
                    // This OCC is not present, just move along
                    continue;
                }

                for (j=0; j<MAX_CORES; j++)
                {
                    l_core_count++;

                    // Note: cores that are not present will return a zero
                    // utilization, thus fulfilling the check below
                    if (G_dcom_slv_outbox_rx[i].nutil3sp0cy[j] <
                        g_amec->mst_ips_parms.entry_threshold)
                    {
                        // Count how many cores have a normalized utilization
                        // below the entry threshold
                        l_entry_core_count++;
                    }
                    else
                    {
                        // Reset the entry count since we found a core that
                        // has a utilization >= entry threshold
                        G_ips_entry_count = 0;
                    }
                }
            }

            // All cores have a utilization lower than the entry threshold
            if (l_entry_core_count == l_core_count)
            {
                // Increment the entry count by the number of samples used to
                // average utilization (default is 3 seconds)
                G_ips_entry_count += l_time_interval;

                if (G_ips_entry_count >= g_amec->mst_ips_parms.entry_delay)
                {
                    // We have met the entry criteria, move IPS state to active
                    g_amec->mst_ips_parms.active = 1;

                    for (i=0; i<MAX_OCCS; i++)
                    {
                        if (!(G_sysConfigData.is_occ_present & (1<<i)))
                        {
                            continue;
                        }
                        for (j=0; j<MAX_CORES; j++)
                        {
                            G_ips_exit_count[i][j] = 0;
                        }
                    }

                    // Allow IPS to go into the throttle space
                    g_amec->mst_ips_parms.freq_request = g_amec->sys.fmin_max_throttled;

                    TRAC_INFO("amec_mst_ips_main: We have entered IPS active state! entry_count[%u] freq_request[%u]",
                              G_ips_entry_count, g_amec->mst_ips_parms.freq_request);
                }
            }
        }
    }while(0);
}

uint8_t AMEC_mst_get_ips_active_status()
{
    return g_amec->mst_ips_parms.active;
}

// Function Specification
//
// Name: amec_mst_gen_soft_freq
//
// Description: This function executes the algorithm that generates the soft
// frequency boundaries to be sent to the Slave OCCs. This is the strategy
// to be used for P8 GA1.
//
// End Function Specification
void amec_mst_gen_soft_freq(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    uint8_t                     i = 0;
    uint16_t                    l_ftarget = 0;
    uint16_t                    l_fdelta = 0;
    uint16_t                    l_lowest_fwish = 0;
    uint16_t                    l_highest_fwish = 0;
    BOOLEAN                     l_need_reset = FALSE;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    do
    {
        // First, check if frequency delta has been enabled
        if (G_mst_tunable_parameter_table_ext[7].adj_value == 0)
        {
            // If it hasn't been enabled, then use the full frequency range and
            // break
            G_mst_soft_fmin = 0x0000;
            G_mst_soft_fmax = 0xFFFF;

            break;
        }

        // Use the frequency delta sent by the customer
        l_fdelta = (G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MODE_DISABLED] *
                    G_mst_tunable_parameter_table_ext[8].adj_value/2) / 100;

        // Check if all OCCs are operating inside the soft frequency boundaries
        for (i=0; i<MAX_OCCS; i++)
        {
            // Ignore OCCs that are sending zeros for factual or fwish
            if ((G_dcom_slv_outbox_rx[i].factual == 0) ||
                (G_dcom_slv_outbox_rx[i].fwish == 0))
            {
                continue;
            }

            // Compare factual against the soft frequency boundaries
            if ((G_dcom_slv_outbox_rx[i].factual < G_mst_soft_fmin) ||
                (G_dcom_slv_outbox_rx[i].factual > G_mst_soft_fmax))
            {
                G_mst_violation_cnt[i]++;
            }
            else
            {
                G_mst_violation_cnt[i] = 0;
            }

            // Now check if the violation count has exceeded the threshold
            if (G_mst_violation_cnt[i] > 3)
            {
                // The new target frequency is the violator's factual
                l_need_reset = TRUE;
                l_ftarget = G_dcom_slv_outbox_rx[i].factual;
            }
        }

        // If all OCCs are inside the soft frequency boundaries, select the
        // target frequeny based on the power mode
        if (!l_need_reset)
        {
            if(CURRENT_MODE() == OCC_MODE_DYN_PERF)
            {
                // For DPS modes, calculate the highest Fwish sent
                // by all slave OCCs
                l_highest_fwish = 0;
                for (i=0; i<MAX_OCCS; i++)
                {
                    // Ignore OCCs that are sending zeros for factual or fwish
                    if ((G_dcom_slv_outbox_rx[i].factual == 0) ||
                        (G_dcom_slv_outbox_rx[i].fwish == 0))
                    {
                        continue;
                    }
                    if (G_dcom_slv_outbox_rx[i].fwish > l_highest_fwish)
                    {
                        l_highest_fwish = G_dcom_slv_outbox_rx[i].fwish;
                    }
                }

                // If the highest Fwish is in range, go for it. Otherwise, step
                // towards it in small steps
                if ((l_highest_fwish >= G_mst_soft_fmin) &&
                    (l_highest_fwish <= G_mst_soft_fmax))
                {
                    l_ftarget = l_highest_fwish;
                }
                else if (l_highest_fwish > G_mst_soft_fmax)
                {
                    // Fwish is too high for range, step up (new target is the
                    // old soft Fmax)
                    l_ftarget = G_mst_soft_fmax;
                }
                else
                {
                    // Fwish is too low for range, step down (new target is the
                    // old soft Fmin)
                    l_ftarget = G_mst_soft_fmin;
                }
            }
            else
            {
                // For other system power modes, calculate the lowest Fwish sent
                // by all slave OCCs
                l_lowest_fwish = 0xFFFF;
                for (i=0; i<MAX_OCCS; i++)
                {
                    // Ignore OCCs that are sending zeros for factual or fwish
                    if ((G_dcom_slv_outbox_rx[i].factual == 0) ||
                        (G_dcom_slv_outbox_rx[i].fwish == 0))
                    {
                        continue;
                    }
                    if ((G_dcom_slv_outbox_rx[i].fwish < l_lowest_fwish) &&
                        (G_dcom_slv_outbox_rx[i].fwish != 0))
                    {
                        l_lowest_fwish = G_dcom_slv_outbox_rx[i].fwish;
                    }
                }

                // If the lowest Fwish is in range, go for it. Otherwise, step
                // towards it in small steps
                if ((l_lowest_fwish >= G_mst_soft_fmin) &&
                    (l_lowest_fwish <= G_mst_soft_fmax))
                {
                    l_ftarget = l_lowest_fwish;
                }
                else if (l_lowest_fwish > G_mst_soft_fmax)
                {
                    // Fwish is too high for range, step up (new target is the
                    // old soft Fmax)
                    l_ftarget = G_mst_soft_fmax;
                }
                else
                {
                    // Fwish is too low for range, step down (new target is the
                    // old soft Fmin)
                    l_ftarget = G_mst_soft_fmin;
                }
            }
        }

        // Calculate the new soft frequency boundaries
        G_mst_soft_fmin = l_ftarget - l_fdelta;
        G_mst_soft_fmax = l_ftarget + l_fdelta;

    } while(0);
}


// Function Specification
//
// Name: amec_mst_cmmon_tasks_pre
//
// Description: master common tasks pre function
//
// Task Flags:
//
// End Function Specification
void amec_mst_common_tasks_pre(void)
{
  AMEC_DBG("\tAMEC Master Pre-State Common\n");

  // ------------------------------------------------------
  //
  // ------------------------------------------------------


  // ------------------------------------------------------
  //
  // ------------------------------------------------------
}


// Function Specification
//
// Name: amec_mst_cmmon_tasks_post
//
// Description: master common tasks post function
//
// End Function Specification
void amec_mst_common_tasks_post(void)
{
  AMEC_DBG("\tAMEC Master Post-State Common\n");

  // Only execute if OCC is in the active state
  if ( IS_OCC_STATE_ACTIVE() )
  {
      // Call the OCC auto-slew function
      amec_master_auto_slew();

      if(0 == G_dcom_slv_inbox_doorbell_rx.apss_recovery_in_progress)
      {
        //Call check under power cap function
        amec_mst_check_under_pcap();
      }
  }
}


// Function Specification
//
// Name: amec_mst_state_0 ~ amec_mst_state_7
//
// Description: master state 0 ~ 7 functions
//
// End Function Specification
void amec_mst_state_0(void){AMEC_DBG("\tAMEC Master State 0\n");}
void amec_mst_state_1(void){AMEC_DBG("\tAMEC Master State 1\n");}
void amec_mst_state_2(void){AMEC_DBG("\tAMEC Master State 2\n");}
void amec_mst_state_3(void){AMEC_DBG("\tAMEC Master State 3\n");}
void amec_mst_state_4(void){AMEC_DBG("\tAMEC Master State 4\n");}
void amec_mst_state_5(void){AMEC_DBG("\tAMEC Master State 5\n");}

void amec_mst_state_6(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    AMEC_DBG("\tAMEC Master State 6\n");

    if ( IS_OCC_STATE_ACTIVE() )
    {
        amec_mst_gen_soft_freq();
    }
}

void amec_mst_state_7(void){AMEC_DBG("\tAMEC Master State 7\n");}

// Function Specification
//
// Name: amec_mst_substate_5_even
//
// Description: state 5 even numbered master substates for IPS
//              gives state 5 substate function to be called every 16th tick
//              This must be called at the same frequency as CORE_DATA_COLLECTION_US
//              for IPS enter/exit timings to work correctly
//              Time = MICS_PER_TICK * MAX_NUM_TICKS
//              odd substates of state 5 are not currently used
//
// End Function Specification
void amec_mst_substate_5_even(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    static uint16_t             l_counter = 0;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    AMEC_DBG("\tAMEC Master State 5 even substate\n");

    // Only execute if OCC is in the active state
    if ( IS_OCC_STATE_ACTIVE() )
    {
        // Increment our local counter
        l_counter++;

        // Is it time to run the Idle Power Saver algorithm? (default is 3sec)
        if (l_counter == AMEC_DPS_SAMPLING_RATE * AMEC_IPS_AVRG_INTERVAL)
        {
            l_counter = 0;
            amec_mst_ips_main();
        }
    }
}


/*----------------------------------------------------------------------------*/
/* End                                                                        */
/*----------------------------------------------------------------------------*/
