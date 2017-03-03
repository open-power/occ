/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/amec/amec_pcap.c $                                */
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

//*************************************************************************/
// Includes
//*************************************************************************/
#include "amec_pcap.h"
#include "amec_sys.h"
#include "amec_service_codes.h"
#include <occ_common.h>
#include <occ_sys_config.h>
#include <dcom.h>
#include <trac.h>

//*************************************************************************/
// Externs
//*************************************************************************/

//*************************************************************************/
// Defines/Enums
//*************************************************************************/
#define PPB_NOM_DROP_DELAY 4    //ticks

//*************************************************************************/
// Structures
//*************************************************************************/

//*************************************************************************/
// Globals
//*************************************************************************/

//Number of ticks to wait before dropping below nominal frequency
#define PWR_SETTLED_TICKS   4
//Number of watts power must be below the node power cap before raising
//ppb_fmax
#define PDROP_THRESH        0
//Number of MHz to raise the proc_pcap_vote for every watt of available power
//(DCM value should be less than SCM)
#define PROC_MHZ_PER_WATT   28
//Number of MHz to raise ppb_fmax per watt of available power. Depends on
//number of procs in node.
#define NODE_MHZ_PER_WATT()  \
                          (G_sysConfigData.sys_num_proc_present == 0?   \
                           1:                                           \
                           ((PROC_MHZ_PER_WATT/G_sysConfigData.sys_num_proc_present) == 0? \
                            1:                                          \
                            PROC_MHZ_PER_WATT/G_sysConfigData.sys_num_proc_present))

//Frequency_step_khz (from global pstate table)/1000
uint32_t    G_mhz_per_pstate=0;

uint8_t     G_over_pcap_count=0;

//*************************************************************************/
// Function Prototypes
//*************************************************************************/

//*************************************************************************/
// Functions
//*************************************************************************/


//////////////////////////
// Function Specification
//
// Name: amec_pcap_calc
//
// Description: Calculate the node, memory and processor power caps.
//
// Thread: Real Time Loop
//
// End Function Specification
void amec_pcap_calc(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    bool l_oversub_state  = 0;
    uint16_t l_node_pwr = AMECSENSOR_PTR(PWR250US)->sample;
    uint16_t l_p0_pwr   = AMECSENSOR_PTR(PWR250USP0)->sample;
    int32_t l_avail_power = 0;
    uint16_t mem_pwr_diff = 0;
    uint32_t l_proc_fraction = 0;
    static uint32_t L_prev_node_pcap = 0;
    static bool L_apss_error_traced = FALSE;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    l_oversub_state = AMEC_INTF_GET_OVERSUBSCRIPTION();

    // Determine the active power cap.  norm_node_pcap is set as lowest
    // between sys and user in amec_data_write_pcap()
    // when in oversub only use oversub pcap if lower than norm_node_pcap
    // to handle user set power cap lower than the oversub power cap
    if( (TRUE == l_oversub_state) &&
        (g_amec->pcap.ovs_node_pcap < g_amec->pcap.norm_node_pcap) )
    {
        g_amec->pcap.active_node_pcap = g_amec->pcap.ovs_node_pcap;
    }
    else
    {
        g_amec->pcap.active_node_pcap = g_amec->pcap.norm_node_pcap;
    }

    //Trace whenever the node pcap changes
    if(L_prev_node_pcap != g_amec->pcap.active_node_pcap)
    {
        TRAC_IMP("amec_pcap_calc: Node pcap set to %d watts.",
                  g_amec->pcap.active_node_pcap);
        L_prev_node_pcap = g_amec->pcap.active_node_pcap;

        // set this pcap as valid (needed by master for comparison)
        g_amec->pcap_valid = 1;
    }

    l_avail_power = g_amec->pcap.active_node_pcap - l_node_pwr;
    if(l_node_pwr != 0)
    {
        l_proc_fraction = ((uint32_t)(l_p0_pwr) << 16)/l_node_pwr;
        if(L_apss_error_traced)
        {
            TRAC_ERR("PCAP: PWR250US sensor is no longer 0.");
            L_apss_error_traced = FALSE;
        }

        // check if allowed to increase power AND memory throttled due to pcap
        if((l_avail_power > 0) && (g_amec->pcap.active_mem_level != 0))
        {
            // un-throttle memory if there is enough available power between
            // current and new throttles
            if (CURRENT_MODE() == OCC_MODE_NOMINAL)
            {
                mem_pwr_diff = g_amec->pcap.nominal_mem_pwr;
            }
            else
            {
                mem_pwr_diff = g_amec->pcap.turbo_mem_pwr;
            }

            // currently there's only 1 mem pcap throt level so must be pcap1
            mem_pwr_diff -= g_amec->pcap.pcap1_mem_pwr;

            if(l_avail_power >= mem_pwr_diff)
            {
                TRAC_IMP("PCAP: Un-Throttling memory");
                g_amec->pcap.active_mem_level = 0;
                // don't let the proc have any available power this tick
                l_avail_power = 0;
            }
        }
        // check if need to reduce power and frequency is already at the min
        else if((l_avail_power < 0) && (g_amec->proc[0].pwr_votes.ppb_fmax == g_amec->sys.fmin))
        {
            // frequency at min now shed additional power by throttling
            // memory if memory is currently un-throttled due to power
            if (g_amec->pcap.active_mem_level == 0)
            {
                TRAC_IMP("PCAP: Throttling memory");
                g_amec->pcap.active_mem_level = 1;
            }
        }
        else
        {
            // no changes to memory throttles due to power
        }
    }
    else
    {
        if(!L_apss_error_traced)
        {
            TRAC_ERR("PCAP: PWR250US sensor is showing a value of 0.");
            L_apss_error_traced = TRUE;
        }
    }

    // skip processor changes until memory is un-capped
    if(!g_amec->pcap.active_mem_level)
    {
        g_amec->pcap.active_proc_pcap = l_p0_pwr + ((l_proc_fraction * l_avail_power) >> 16);

        //NOTE: Power capping will not affect nominal cores unless a customer pcap
        //      is set below the max pcap or oversubscription occurs.  However,
        //      nominal cores will drop below nominal if ppb_fmax drops below nominal
        if(g_amec->pcap.active_node_pcap < G_sysConfigData.pcap.max_pcap)
        {
           g_amec->proc[0].pwr_votes.nom_pcap_fmin = G_sysConfigData.sys_mode_freq.table[OCC_MODE_MIN_FREQUENCY];
        }
        else
        {
           g_amec->proc[0].pwr_votes.nom_pcap_fmin = G_sysConfigData.sys_mode_freq.table[OCC_MODE_NOMINAL];
        }
    }
}

//////////////////////////
// Function Specification
//
// Name: amec_pcap_controller
//
// Description: Execute the processor Pcap control loop.
//
// Thread: Real Time Loop
//
// End Function Specification
void amec_pcap_controller(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    int32_t     l_power_avail = 0;
    int32_t     l_proc_pcap_vote = g_amec->proc[0].pwr_votes.proc_pcap_vote;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    l_power_avail = g_amec->pcap.active_proc_pcap - AMECSENSOR_PTR(PWR250USP0)->sample;

    if(l_proc_pcap_vote > g_amec->proc[0].pwr_votes.nom_pcap_fmin)
    {
        l_proc_pcap_vote = g_amec->proc[0].core_max_freq +
                                                  (PROC_MHZ_PER_WATT * l_power_avail);
    }
    else
    {
        l_proc_pcap_vote += (PROC_MHZ_PER_WATT * l_power_avail);
    }

    if(l_proc_pcap_vote > G_sysConfigData.sys_mode_freq.table[OCC_MODE_TURBO])
    {
        l_proc_pcap_vote = G_sysConfigData.sys_mode_freq.table[OCC_MODE_TURBO];
    }

    if(l_proc_pcap_vote < G_sysConfigData.sys_mode_freq.table[OCC_MODE_MIN_FREQUENCY])
    {
        l_proc_pcap_vote = G_sysConfigData.sys_mode_freq.table[OCC_MODE_MIN_FREQUENCY];
    }

    //Power capping for nominal cores is not allowed to drop frequency below nom_pcap_fmin
    if(l_proc_pcap_vote <  g_amec->proc[0].pwr_votes.nom_pcap_fmin)
    {
        g_amec->proc[0].pwr_votes.proc_pcap_nom_vote = g_amec->proc[0].pwr_votes.nom_pcap_fmin;
    }
    else
    {
        g_amec->proc[0].pwr_votes.proc_pcap_nom_vote = l_proc_pcap_vote;
    }

    g_amec->proc[0].pwr_votes.proc_pcap_vote = l_proc_pcap_vote;
}

//////////////////////////
// Function Specification
//
// Name: amec_ppb_fmax_calc
//
// Description: Calculate the Performance Preserving Bounds (PPB) vote.
//
// Thread: Real Time Loop
//
// End Function Specification
void amec_ppb_fmax_calc(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    int32_t     l_power_avail = 0;
    bool        l_continue = TRUE;    //Used to break from code if needed.

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    //Set the slaves local copy of ppb_fmax to that received from Master OCC.
    g_amec->proc[0].pwr_votes.ppb_fmax = G_dcom_slv_inbox_doorbell_rx.ppb_fmax;
    // For debug
    sensor_update( AMECSENSOR_PTR(PROBE250US0), g_amec->proc[0].pwr_votes.ppb_fmax);

    //CALCULATION done by MASTER OCC only.
    if(OCC_MASTER == G_occ_role)
    {
        //Power available is the ActiveNodePower - PowerDropThreshold - ActualPwr
        l_power_avail = g_amec->pcap.active_node_pcap - PDROP_THRESH - AMECSENSOR_PTR(PWR250US)->sample;

        //Note: The PWR250US value is read over the SPI bus, which has no error
        //detection. In order to prevent a single bad SPI transfer from causing
        //OCC to lower nominal core frequencies, we require the power to be over
        //the pcap for PPB_NOM_DROP_DELAY ticks before lowering PPB Fmax below
        //Fnom.
        if((g_amec->proc[0].pwr_votes.ppb_fmax == G_sysConfigData.sys_mode_freq.table[OCC_MODE_NOMINAL])
                && (l_power_avail <=0))
        {
            if(G_over_pcap_count < PPB_NOM_DROP_DELAY)
            {
                G_over_pcap_count++;
                l_continue = FALSE;
            }
        }
        else
        {
            G_over_pcap_count = 0;
        }

        //Only run once every 4 ticks (1ms) to allow time for power hogging
        //chips to drop power and power starved chips to raise power.
        if(l_continue && (0 == (G_current_tick & 0x3)))
        {
            if(l_power_avail <= 0)
            {
                G_sysConfigData.master_ppb_fmax -= G_mhz_per_pstate;
            }
            else
            {
                G_sysConfigData.master_ppb_fmax += NODE_MHZ_PER_WATT() * l_power_avail;
            }

            if(G_sysConfigData.master_ppb_fmax > G_sysConfigData.sys_mode_freq.table[OCC_MODE_TURBO])
            {
                G_sysConfigData.master_ppb_fmax = G_sysConfigData.sys_mode_freq.table[OCC_MODE_TURBO];
            }

            if(G_sysConfigData.master_ppb_fmax < G_sysConfigData.sys_mode_freq.table[OCC_MODE_MIN_FREQUENCY])
            {
                G_sysConfigData.master_ppb_fmax = G_sysConfigData.sys_mode_freq.table[OCC_MODE_MIN_FREQUENCY];
            }
        }
    }//End of Master code
}

//////////////////////////
// Function Specification
//
// Name: amec_power_control
//
// Description: Main function for power control loop.
//
// Thread: Real Time Loop
//
// End Function Specification
void amec_power_control(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    // Calculate the pcap for the proc, memory and the power capping limit
    // for nominal cores.
    amec_pcap_calc();

    // skip processor changes until memory is un-capped
    if(!g_amec->pcap.active_mem_level)
    {
       // Calculate voting box input freq for staying with the current pcap
       amec_pcap_controller();

       // Calculate the performance preserving bounds voting box input freq
       amec_ppb_fmax_calc();
    }
}

/*----------------------------------------------------------------------------*/
/* End                                                                        */
/*----------------------------------------------------------------------------*/
