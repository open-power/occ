/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/amec/amec_master_smh.c $                          */
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

//*************************************************************************
// Includes
//*************************************************************************
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
#include "amec_service_codes.h" //For AMEC_MST_CHECK_PCAPS_MATCH
#include "dcom.h"

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************

//Power cap mismatch threshold set to 8 ticks (2 milliseconds)
#define PCAPS_MISMATCH_THRESHOLD 8

//Power cap failure threshold set to 32 (ticks)
#define PCAP_FAILURE_THRESHOLD 32

//*************************************************************************
// Structures
//*************************************************************************

//*************************************************************************
// Globals
//*************************************************************************
smh_state_t G_amec_mst_state = {AMEC_INITIAL_STATE,
                                AMEC_INITIAL_STATE,
                                AMEC_INITIAL_STATE};

//Array that stores active power cap values of all OCCs
slave_pcap_info_t G_slave_active_pcaps[MAX_OCCS] = {{0}};

//OCC Power cap mismatch count
uint8_t G_pcaps_mismatch_count = 0;

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

// --------------------------------------------------------
// AMEC Master State 6.1 Substate Table
// --------------------------------------------------------
// Each function inside this state table runs once every 128ms.
//
const smh_tbl_t amec_mst_state_6_1_sub_substate_table[AMEC_SMH_STATES_PER_LVL] =
{
  {amec_mst_sub_substate_6_1_0, NULL},
  {amec_mst_sub_substate_6_1_1, NULL},
  {amec_mst_sub_substate_6_1_2, NULL},
  {amec_mst_sub_substate_6_1_3, NULL},
  {amec_mst_sub_substate_6_1_4, NULL},
  {amec_mst_sub_substate_6_1_5, NULL},
  {amec_mst_sub_substate_6_1_6, NULL},
  {amec_mst_sub_substate_6_1_7, NULL},
};

// --------------------------------------------------------
// AMEC Master State 0 Substate Table
// --------------------------------------------------------
// Each function inside this state table runs once every 16ms.
//
// The 2 States are interleaved so each one runs every 4ms.
//
const smh_tbl_t amec_mst_state_0_substate_table[AMEC_SMH_STATES_PER_LVL] =
{
  {amec_mst_substate_0_0, NULL},
  {amec_mst_substate_0_1, NULL},
  {amec_mst_substate_0_0, NULL},
  {amec_mst_substate_0_1, NULL},
  {amec_mst_substate_0_0, NULL},
  {amec_mst_substate_0_1, NULL},
  {amec_mst_substate_0_0, NULL},
  {amec_mst_substate_0_1, NULL},
};

// --------------------------------------------------------
// AMEC Master State 3 Substate Table
// --------------------------------------------------------
// Each function inside this state table runs once every 16ms.
//
// The 2 States are interleaved so each one runs every 4ms.
//
const smh_tbl_t amec_mst_state_3_substate_table[AMEC_SMH_STATES_PER_LVL] =

{
  {amec_mst_substate_3_0, NULL},
  {amec_mst_substate_3_1, NULL},
  {amec_mst_substate_3_0, NULL},
  {amec_mst_substate_3_1, NULL},
  {amec_mst_substate_3_0, NULL},
  {amec_mst_substate_3_1, NULL},
  {amec_mst_substate_3_0, NULL},
  {amec_mst_substate_3_1, NULL},
};

// --------------------------------------------------------
// AMEC Master State 6 Substate Table
// --------------------------------------------------------
// Each function inside this state table runs once every 16ms.
//
// SubState1:  8 Sub-substates (128ms/sub-substate)
//
const smh_tbl_t amec_mst_state_6_substate_table[AMEC_SMH_STATES_PER_LVL] =
{
  {amec_mst_substate_6_0, NULL},
  {amec_mst_substate_6_1, amec_mst_state_6_1_sub_substate_table},
  {amec_mst_substate_6_2, NULL},
  {amec_mst_substate_6_3, NULL},
  {amec_mst_substate_6_4, NULL},
  {amec_mst_substate_6_5, NULL},
  {amec_mst_substate_6_6, NULL},
  {amec_mst_substate_6_7, NULL},
};

// --------------------------------------------------------
// Main AMEC Master State Table
// --------------------------------------------------------
// Each function inside this state table runs once every 2ms.
//
// State0:  2 Substates (4ms/substate)
// State3:  2 Substates (4ms/substate)
// State6:  8 Substates (16ms/substate)
//
const smh_tbl_t amec_mst_state_table[AMEC_SMH_STATES_PER_LVL] =
{
  {amec_mst_state_0, amec_mst_state_0_substate_table},
  {amec_mst_state_1, NULL},
  {amec_mst_state_2, NULL},
  {amec_mst_state_3, amec_mst_state_3_substate_table},
  {amec_mst_state_4, NULL},
  {amec_mst_state_5, NULL},
  {amec_mst_state_6, amec_mst_state_6_substate_table},
  {amec_mst_state_7, NULL},
};

// This sets up the function pointer that will be called to update the
// fw timings when the AMEC master State Machine finishes.
smh_state_timing_t G_amec_mst_state_timings = {amec_mst_update_smh_sensors};

//*************************************************************************
// Function Prototypes
//*************************************************************************

//*************************************************************************
// Functions
//*************************************************************************

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
    uint16_t                    l_freq = 0;
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
                g_amec->mnfg_parms.foverride = g_amec->mnfg_parms.fmax;

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
                g_amec->mnfg_parms.foverride = 0;

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

        // Now, generate a new frequency override
        l_freq = g_amec->mnfg_parms.foverride -
            (l_direction * g_amec->mnfg_parms.fstep);

        if (l_freq <= g_amec->mnfg_parms.fmin)
        {
            // We've reached an edge, increment our counter
            g_amec->mnfg_parms.slew_counter++;

            // Change direction of the auto-slew
            l_direction = -l_direction;

            // Clip frequency to fmin
            l_freq = g_amec->mnfg_parms.fmin;
        }
        else if (l_freq >= g_amec->mnfg_parms.fmax)
        {
            // We've reached an edge, increment our counter
            g_amec->mnfg_parms.slew_counter++;

            // Change direction of the auto-slew
            l_direction = -l_direction;

            // Clip frequency to fmax
            l_freq = g_amec->mnfg_parms.fmax;
        }
        g_amec->mnfg_parms.foverride = l_freq;

    }while(0);

    return;
}

// Function Specification
//
// Name: amec_mst_check_pcaps_match
//
// Description: This function checks for mismatch in power caps between
//              occs for 8 consecutive ticks.
//
// End Function Specification
void amec_mst_check_pcaps_match(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    uint8_t l_chip_id;
    bool l_prev_pcap_valid = FALSE;
    uint16_t l_prev_pcap = 0;
    bool l_pcap_mismatch = FALSE;
    errlHndl_t  l_err = NULL;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    //Loop through all occs
    for(l_chip_id = 0; l_chip_id < MAX_OCCS; l_chip_id++)
    {
        //if occ is present && its pcap data is considered valid
        if((G_sysConfigData.is_occ_present & (1<< l_chip_id)) &&
           (G_slave_active_pcaps[l_chip_id].pcap_valid != 0))
        {
            //TRAC_INFO(" occ[%d]: pcap[%d] valid(%d)",
            //    l_chip_id, G_slave_active_pcaps[l_chip_id].active_pcap, G_slave_active_pcaps[l_chip_id].pcap_valid );

            //Initialize l_prev_pcap to the first valid/present occ's power cap
            if(!l_prev_pcap_valid)
            {
                //TRAC_INFO("First present occ - power cap info[%d]=%d(%d)",
                //    l_chip_id, G_slave_active_pcaps[l_chip_id].active_pcap, G_slave_active_pcaps[l_chip_id].pcap_valid );

                l_prev_pcap = G_slave_active_pcaps[l_chip_id].active_pcap;
                l_prev_pcap_valid = TRUE;
            }
            else
            {
                //If there is mismatch between OCCs power caps, increment mismatch
                // count
                if(l_prev_pcap != G_slave_active_pcaps[l_chip_id].active_pcap)
                {
                    G_pcaps_mismatch_count++;
                    l_pcap_mismatch = TRUE;

                    TRAC_INFO("Mismatch in OCC power cap values: mismatch cnt=%d pcap=%d vs compared pcap[%d]=%d(%d)",
                        G_pcaps_mismatch_count, l_prev_pcap, l_chip_id, G_slave_active_pcaps[l_chip_id].active_pcap,
                        G_slave_active_pcaps[l_chip_id].pcap_valid);

                    //If mismatch occurs for 8 consecutive ticks
                    //i.e 8 * 250 microsecs = 2 milliseconds,then reset occ
                    if(G_pcaps_mismatch_count >= PCAPS_MISMATCH_THRESHOLD)
                    {
                        TRAC_ERR("Mismatch in OCC power cap values: pcap=%d, slave_active_pcap[%d]=%d(%d)",
                            l_prev_pcap, l_chip_id, G_slave_active_pcaps[l_chip_id].active_pcap,
                            G_slave_active_pcaps[l_chip_id].pcap_valid);

                        /* @
                         * @errortype
                         * @moduleid    AMEC_MST_CHECK_PCAPS_MATCH
                         * @reasoncode  INTERNAL_FAILURE
                         * @userdata1   First OCC Power cap
                         * @userdata2   Mismatch OCC Power cap
                         * @devdesc     Internal max power limits mismatched
                         *
                         */
                        l_err = createErrl( AMEC_MST_CHECK_PCAPS_MATCH,
                                            INTERNAL_FAILURE,
                                            ERC_AMEC_PCAPS_MISMATCH_FAILURE,
                                            ERRL_SEV_PREDICTIVE,
                                            NULL,
                                            DEFAULT_TRACE_SIZE,
                                            l_prev_pcap,
                                            G_slave_active_pcaps[l_chip_id].active_pcap);

                        //Callout to OVS
                        addCalloutToErrl(l_err,
                                         ERRL_CALLOUT_TYPE_COMPONENT_ID,
                                         ERRL_COMPONENT_ID_FIRMWARE,
                                         ERRL_CALLOUT_PRIORITY_HIGH);

                        //Callout to APSS
                        addCalloutToErrl(l_err,
                                         ERRL_CALLOUT_TYPE_HUID,
                                         G_sysConfigData.apss_huid,
                                         ERRL_CALLOUT_PRIORITY_HIGH);

                        //Callout to DPSS
                        addCalloutToErrl(l_err,
                                         ERRL_CALLOUT_TYPE_HUID,
                                         G_sysConfigData.dpss_huid,
                                         ERRL_CALLOUT_PRIORITY_HIGH);
                        //Reset OCC
                        REQUEST_RESET(l_err);
                    }
                    break;
                }
            }
        }
    }

    //If there was no power cap mismatch between occ's,then reset count to zero
    if(!l_pcap_mismatch)
    {
        G_pcaps_mismatch_count = 0;
    }
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

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    // Check if done everything possible to shed power and power still above a hard power cap
    // ppb_fmax = Fmin and PWR250US > Node power cap and
    // Node power cap >=  hard_min_pcap AND memory is throttled
    if((g_amec->proc[0].pwr_votes.ppb_fmax == g_amec->sys.fmin) &&
       (AMECSENSOR_PTR(PWR250US)->sample > g_amec->pcap.active_node_pcap) &&
       (g_amec->pcap.active_node_pcap >= G_sysConfigData.pcap.hard_min_pcap) &&
       (g_amec->pcap.active_mem_level != 0) )
    {

        G_over_cap_count++;

        //Log error and reset OCC if count >= 32 (ticks)
        if(G_over_cap_count >= PCAP_FAILURE_THRESHOLD)
        {
            TRAC_ERR("Failure to maintain power cap: Power Cap = %d ,"
                     "PWR250US = %d ,PWR250USP0 = %d ,PWR250USFAN = %d ,"
                     "PWR250USMEM0 = %d",g_amec->pcap.active_node_pcap,
                     AMECSENSOR_PTR(PWR250US)->sample,
                     AMECSENSOR_PTR(PWR250USP0)->sample,
                     AMECSENSOR_PTR(PWR250USFAN)->sample,
                     AMECSENSOR_PTR(PWR250USMEM0)->sample);

            TRAC_ERR("PWR250USIO = %d , PWR250USSTORE = %d, PWRGPU = %d",
                     AMECSENSOR_PTR(PWR250USIO)->sample,
                     AMECSENSOR_PTR(PWR250USSTORE)->sample,
                     AMECSENSOR_PTR(PWRGPU)->sample);

            /* @
             * @errortype
             * @moduleid    AMEC_MST_CHECK_UNDER_PCAP
             * @reasoncode  POWER_CAP_FAILURE
             * @userdata1   Power Cap
             * @userdata2   PWR250US (Node Power)
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
                                AMECSENSOR_PTR(PWR250US)->sample);

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
        //Decrement count if node power under power cap value
        if(G_over_cap_count > 0)
        {
            G_over_cap_count--;
        }
    }

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
            g_amec->mst_ips_parms.freq_request = g_amec->sys.fmax;

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

                TRAC_INFO("amec_mst_ips_main: We have exited IPS active state! exit_count[%u] freq_request[%u]",
                          G_ips_exit_count[i][j], g_amec->mst_ips_parms.freq_request);
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

                    g_amec->mst_ips_parms.freq_request =
                        G_sysConfigData.sys_mode_freq.table[OCC_MODE_MIN_FREQUENCY];

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
        l_fdelta = (G_sysConfigData.sys_mode_freq.table[OCC_MODE_NOMINAL] *
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
            if ((CURRENT_MODE() == OCC_MODE_DYN_POWER_SAVE) ||
                (CURRENT_MODE() == OCC_MODE_DYN_POWER_SAVE_FP))
            {
                // For DPS and DPS-FP modes, calculate the highest Fwish sent
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

      //Call OCC pcaps mismatch function
      amec_mst_check_pcaps_match();

      //Call check under power cap function
      amec_mst_check_under_pcap();
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

void amec_mst_state_5(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    static uint16_t             l_counter = 0;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    AMEC_DBG("\tAMEC Master State 5\n");

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
// Name: amec_mst_substate_0_0
//       amec_mst_substate_0_1
//
// Description: master substate amec_mst_substate_0_0
//              master substate amec_mst_substate_0_1
//
// End Function Specification
void amec_mst_substate_0_0(void){AMEC_DBG("\tAMEC Master SubState 0.0\n");}
void amec_mst_substate_0_1(void){AMEC_DBG("\tAMEC Master SubState 0.1\n");}

// Function Specification
//
// Name: amec_mst_substate_6_0
//       amec_mst_substate_6_1
//       amec_mst_substate_6_2
//       amec_mst_substate_6_3
//       amec_mst_substate_6_4
//       amec_mst_substate_6_5
//       amec_mst_substate_6_6
//       amec_mst_substate_6_7
//
// Description: master substate amec_mst_substate_6_0
//              master substate amec_mst_substate_6_1
//              master substate amec_mst_substate_6_2
//              master substate amec_mst_substate_6_3
//              master substate amec_mst_substate_6_4
//              master substate amec_mst_substate_6_5
//              master substate amec_mst_substate_6_6
//              master substate amec_mst_substate_6_7
//
// Flow:              FN= None
//
// End Function Specification
void amec_mst_substate_6_0(void){AMEC_DBG("\tAMEC Master State 6.0\n");}
void amec_mst_substate_6_1(void){AMEC_DBG("\tAMEC Master State 6.1\n");}
void amec_mst_substate_6_2(void){AMEC_DBG("\tAMEC Master State 6.2\n");}
void amec_mst_substate_6_3(void){AMEC_DBG("\tAMEC Master State 6.3\n");}
void amec_mst_substate_6_4(void){AMEC_DBG("\tAMEC Master State 6.4\n");}
void amec_mst_substate_6_5(void){AMEC_DBG("\tAMEC Master State 6.5\n");}
void amec_mst_substate_6_6(void){AMEC_DBG("\tAMEC Master State 6.6\n");}
void amec_mst_substate_6_7(void){AMEC_DBG("\tAMEC Master State 6.7\n");}

// Function Specification
//
// Name: amec_mst_substate_3_0
//       amec_mst_substate_3_1
//
// Description: master substate amec_mst_substate_3_0
//              master substate amec_mst_substate_3_1
//
// Flow:              FN= None
//
// End Function Specification
void amec_mst_substate_3_0(void){AMEC_DBG("\tAMEC Master State 3.0\n");}
void amec_mst_substate_3_1(void){AMEC_DBG("\tAMEC Master State 3.1\n");}


// Function Specification
//
// Name: amec_mst_substate_6_1_0
//       amec_mst_substate_6_1_1
//       amec_mst_substate_6_1_2
//       amec_mst_substate_6_1_3
//       amec_mst_substate_6_1_4
//       amec_mst_substate_6_1_5
//       amec_mst_substate_6_1_6
//       amec_mst_substate_6_1_7
//
// Description: master substate amec_mst_substate_6_1_0
//              master substate amec_mst_substate_6_1_1
//              master substate amec_mst_substate_6_1_2
//              master substate amec_mst_substate_6_1_3
//              master substate amec_mst_substate_6_1_4
//              master substate amec_mst_substate_6_1_5
//              master substate amec_mst_substate_6_1_6
//              master substate amec_mst_substate_6_1_7
//
// Flow:              FN= None
//
// End Function Specification
void amec_mst_sub_substate_6_1_0(void){AMEC_DBG("\tAMEC Master State 6.1.0\n");}
void amec_mst_sub_substate_6_1_1(void){AMEC_DBG("\tAMEC Master State 6.1.1\n");}
void amec_mst_sub_substate_6_1_2(void){AMEC_DBG("\tAMEC Master State 6.1.2\n");}
void amec_mst_sub_substate_6_1_3(void){AMEC_DBG("\tAMEC Master State 6.1.3\n");}
void amec_mst_sub_substate_6_1_4(void){AMEC_DBG("\tAMEC Master State 6.1.4\n");}
void amec_mst_sub_substate_6_1_5(void){AMEC_DBG("\tAMEC Master State 6.1.5\n");}
void amec_mst_sub_substate_6_1_6(void){AMEC_DBG("\tAMEC Master State 6.1.6\n");}
void amec_mst_sub_substate_6_1_7(void){AMEC_DBG("\tAMEC Master State 6.1.7\n");}

/*----------------------------------------------------------------------------*/
/* End                                                                        */
/*----------------------------------------------------------------------------*/
