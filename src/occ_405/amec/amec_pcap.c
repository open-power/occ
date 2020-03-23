/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/amec/amec_pcap.c $                                */
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
#include "amec_pcap.h"
#include "amec_sys.h"
#include "amec_service_codes.h"
#include "common.h"
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

//Number of consecutive ticks with power available to wait before un-throttling memory
#define UNTHROTTLE_MEMORY_DELAY   2   // ticks

//*************************************************************************/
// Structures
//*************************************************************************/

//*************************************************************************/
// Globals
//*************************************************************************/
extern PWR_READING_TYPE  G_pwr_reading_type;
extern uint16_t G_allow_trace_flags;
//Number of ticks to wait before dropping below nominal frequency
#define PWR_SETTLED_TICKS   4
//Number of watts power must be below the node power cap before raising
//ppb_fmax
#define PDROP_THRESH        0
//Number of MHz to raise the proc_pcap_vote for every watt of available power
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

extern uint16_t G_proc_fmax_mhz;   // max(turbo,uturbo) frequencies
extern uint32_t G_first_proc_gpu_config;
extern uint32_t G_first_num_gpus_sys;

//*************************************************************************/
// Function Prototypes
//*************************************************************************/

//*************************************************************************/
// Functions
//*************************************************************************/

//////////////////////////
// Function Specification
//
// Name: amec_gpu_pcap
//
// Description: Determine power cap for GPUs
//
// Thread: Real Time Loop
//
// End Function Specification
void amec_gpu_pcap(bool i_oversubscription, bool i_active_pcap_changed, int32_t i_avail_power)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    uint8_t  i = 0;
    uint32_t l_gpu_cap_mw = 0;
    uint16_t l_system_gpu_total_pcap = 0;  // total GPU pcap required by system based on if currently in oversub or not
    static uint16_t L_total_gpu_pcap = 0;  // Current total GPU pcap in effect
    static uint16_t L_n_plus_1_mode_gpu_total_pcap = 0;  // Total GPU pcap required for N+1 (not in oversubscription)
    static uint16_t L_n_mode_gpu_total_pcap = 0;  // Total GPU pcap required for oversubscription
    static uint16_t L_active_psr_gpu_total_pcap = 0; // Total GPU pcap for the currently set pcap and PSR
    static uint16_t L_per_gpu_pcap = 0;  // Amount of L_total_gpu_pcap for each GPU
    static uint8_t L_psr = 100;   // PSR value used in L_active_psr_gpu_total_pcap calculation
    static bool L_first_run = TRUE;  // for calculations done only 1 time

    static uint32_t L_last_pcap_traced[MAX_NUM_GPU_PER_DOMAIN] = {0};

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    // If this is the first time running calculate the total GPU power cap for system power caps (N and N+1)
    if(L_first_run)
    {
       // calculate total GPU power cap for oversubscription
       if(g_amec->pcap.ovs_node_pcap > G_sysConfigData.total_non_gpu_max_pwr_watts)
       {
           // Take all non-GPU power away from the oversubscription power cap
           L_n_mode_gpu_total_pcap = g_amec->pcap.ovs_node_pcap - G_sysConfigData.total_non_gpu_max_pwr_watts;
           // Add back in the power that will be dropped by processor DVFS and memory throttling and give to GPUs
           L_n_mode_gpu_total_pcap += G_sysConfigData.total_proc_mem_pwr_drop_watts;
       }
       else
       {
           // This should not happen, the total non GPU power should never be higher than the N mode cap
           // Log error and set GPUs to minimum power cap
           L_n_mode_gpu_total_pcap = 0; // this will set minimum GPU power cap

           TRAC_ERR("amec_gpu_pcap: non GPU max power %dW is more than N mode pwr limit %dW",
                     G_sysConfigData.total_non_gpu_max_pwr_watts, g_amec->pcap.ovs_node_pcap);

           /* @
            * @errortype
            * @moduleid    AMEC_GPU_PCAP_MID
            * @reasoncode  GPU_FAILURE
            * @userdata1   N mode Power Cap watts
            * @userdata2   Total non-GPU power watts
            * @userdata4   ERC_GPU_N_MODE_PCAP_CALC_FAILURE
            * @devdesc     Total non-GPU power more than N mode power cap
            *
            */
           errlHndl_t l_err = createErrl(AMEC_GPU_PCAP_MID,
                                         GPU_FAILURE,
                                         ERC_GPU_N_MODE_PCAP_CALC_FAILURE,
                                         ERRL_SEV_PREDICTIVE,
                                         NULL,
                                         DEFAULT_TRACE_SIZE,
                                         g_amec->pcap.ovs_node_pcap,
                                         G_sysConfigData.total_non_gpu_max_pwr_watts);

           //Callout firmware
           addCalloutToErrl(l_err,
                            ERRL_CALLOUT_TYPE_COMPONENT_ID,
                            ERRL_COMPONENT_ID_FIRMWARE,
                            ERRL_CALLOUT_PRIORITY_HIGH);
           commitErrl(&l_err);
       }

       // calculate total GPU power cap for N+1 (not in oversubscription)
       if(G_sysConfigData.pcap.system_pcap > G_sysConfigData.total_non_gpu_max_pwr_watts)
       {
           // Take all non-GPU power away from the N+1 power cap
           L_n_plus_1_mode_gpu_total_pcap = G_sysConfigData.pcap.system_pcap - G_sysConfigData.total_non_gpu_max_pwr_watts;
           // Add back in the power that will be dropped by processor DVFS and memory throttling and give to GPUs
           L_n_plus_1_mode_gpu_total_pcap += G_sysConfigData.total_proc_mem_pwr_drop_watts;
       }
       else
       {
           // This should not happen, the total non GPU power should never be higher than the N+1 mode cap
           // Log error and set GPUs to minimum power cap
           L_n_plus_1_mode_gpu_total_pcap = 0; // this will set minimum GPU power cap

           TRAC_ERR("amec_gpu_pcap: non GPU max power %dW is more than N+1 mode pwr limit %dW",
                     G_sysConfigData.total_non_gpu_max_pwr_watts, G_sysConfigData.pcap.system_pcap);

           /* @
            * @errortype
            * @moduleid    AMEC_GPU_PCAP_MID
            * @reasoncode  GPU_FAILURE
            * @userdata1   N+1 mode Power Cap watts
            * @userdata2   Total non-GPU power watts
            * @userdata4   ERC_GPU_N_PLUS_1_MODE_PCAP_CALC_FAILURE
            * @devdesc     Total non-GPU power more than N+1 mode power cap
            *
            */
           errlHndl_t l_err = createErrl(AMEC_GPU_PCAP_MID,
                                         GPU_FAILURE,
                                         ERC_GPU_N_PLUS_1_MODE_PCAP_CALC_FAILURE,
                                         ERRL_SEV_PREDICTIVE,
                                         NULL,
                                         DEFAULT_TRACE_SIZE,
                                         G_sysConfigData.pcap.system_pcap,
                                         G_sysConfigData.total_non_gpu_max_pwr_watts);

           //Callout firmware
           addCalloutToErrl(l_err,
                            ERRL_CALLOUT_TYPE_COMPONENT_ID,
                            ERRL_COMPONENT_ID_FIRMWARE,
                            ERRL_CALLOUT_PRIORITY_HIGH);
           commitErrl(&l_err);
       }
    }  // if first run

    // Calculate the total GPU power cap for the current active limit and PSR
    // this only needs to be calculated if either the active limit or PSR changed
    if( (L_first_run) || (i_active_pcap_changed) || (L_psr != G_sysConfigData.psr) )
    {
       L_psr = G_sysConfigData.psr;
       if(g_amec->pcap.active_node_pcap > G_sysConfigData.total_non_gpu_max_pwr_watts)
       {
           // Take all non-GPU power away from the active power cap
           L_active_psr_gpu_total_pcap = g_amec->pcap.active_node_pcap - G_sysConfigData.total_non_gpu_max_pwr_watts;
           // Add back in the power that will be dropped by processor DVFS and memory throttling based on the PSR
           // to give to GPUs
           L_active_psr_gpu_total_pcap += ( (L_psr / 100) * G_sysConfigData.total_proc_mem_pwr_drop_watts );
       }
       else
       {
           // Set GPUs to minimum power cap
           L_active_psr_gpu_total_pcap = 0;
           TRAC_IMP("amec_gpu_pcap: non GPU max power %dW is more than active pwr limit %dW",
                     G_sysConfigData.total_non_gpu_max_pwr_watts, g_amec->pcap.active_node_pcap);
       }

       // Total GPU power cap is the lower of system (N+1 or oversubscription depending on if in oversub)
       // and the active power limit.  We do not need to always account for oversubscription since
       // the automatic hw power brake will assert to the GPUs if there is a problem when oversub is
       // entered from the time OCC can set and GPUs react to a new power limit
       if(i_oversubscription)
       {
          // system in oversubscription use N mode cap
          l_system_gpu_total_pcap = L_n_mode_gpu_total_pcap;
       }
       else
       {
          // system is not in oversubscription use N+1 mode cap
          l_system_gpu_total_pcap = L_n_plus_1_mode_gpu_total_pcap;
       }

       L_total_gpu_pcap = (l_system_gpu_total_pcap < L_active_psr_gpu_total_pcap) ?
                           l_system_gpu_total_pcap : L_active_psr_gpu_total_pcap;

       // Divide the total equally across all GPUs in the system
       if(G_first_num_gpus_sys)
       {
          L_per_gpu_pcap = L_total_gpu_pcap / G_first_num_gpus_sys;
       }
       else
       {
           L_per_gpu_pcap = 0;
           TRAC_ERR("amec_gpu_pcap: Called with no GPUs present!");
       }
    }

    // Setup to send new power limit to GPUs. The actual sending of GPU power limit will be handled by task_gpu_sm()
    for (i=0; i<MAX_NUM_GPU_PER_DOMAIN; i++)
    {
        // Before sending a GPU a power limit the power limits must be read from the GPU to know min/max GPU allows
        if( GPU_PRESENT(i) && g_amec->gpu[i].pcap.pwr_limits_read )
        {
           l_gpu_cap_mw = L_per_gpu_pcap * 1000;  // convert W to mW

           // GPU is present and have min/max power limits from GPU
           // clip the GPU power limit to min/max GPU limit if needed
           if(l_gpu_cap_mw < g_amec->gpu[i].pcap.gpu_min_pcap_mw)  // clip to min?
           {
              l_gpu_cap_mw = g_amec->gpu[i].pcap.gpu_min_pcap_mw;
           }
           else if(l_gpu_cap_mw > g_amec->gpu[i].pcap.gpu_max_pcap_mw)  // clip to max?
           {
              l_gpu_cap_mw = g_amec->gpu[i].pcap.gpu_max_pcap_mw;
           }

           // check if this is a new power limit
           if(g_amec->gpu[i].pcap.gpu_desired_pcap_mw != l_gpu_cap_mw)
           {
              if( (g_amec->gpu[i].pcap.gpu_desired_pcap_mw != 0) ||
                  (L_last_pcap_traced[i] != l_gpu_cap_mw) )
              {
                 L_last_pcap_traced[i] = l_gpu_cap_mw;
                 TRAC_IMP("amec_gpu_pcap: Updating GPU%d desired pcap %dmW to %dmW", i,
                          g_amec->gpu[i].pcap.gpu_desired_pcap_mw, l_gpu_cap_mw);

              }

              g_amec->gpu[i].pcap.gpu_desired_pcap_mw = l_gpu_cap_mw;
           }
        }
    }  // for each GPU

    L_first_run = FALSE;
}


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
void amec_pcap_calc(const bool i_oversub_state)
{
    bool l_active_pcap_changed = FALSE;
    uint16_t l_node_pwr = AMECSENSOR_PTR(PWRSYS)->sample;
    uint16_t l_p0_pwr   = AMECSENSOR_PTR(PWRPROC)->sample;
    int32_t l_avail_power = 0;
    uint16_t mem_pwr_diff = 0;
    uint32_t l_proc_fraction = 0;
    static uint32_t L_prev_node_pcap = 0;
    static bool L_apss_error_traced = FALSE;
    static uint32_t L_ticks_mem_pwr_available = 0;
    static bool L_trace_pcap_throttle = true;
    static bool L_trace_pcap_unthrottle = true;

    // Determine the active power cap.
    // when in oversub (N mode) only use oversub pcap if lower than user set pcap
    // OCC should allow N mode to be higher than N+1 (don't compare against norm_node_pcap)
    // N mode may be higher on some systems due to ps issue reporting higher power in N mode
    if( (TRUE == i_oversub_state) &&
        (g_amec->pcap.ovs_node_pcap < G_sysConfigData.pcap.current_pcap) )
    {
        g_amec->pcap.active_node_pcap = g_amec->pcap.ovs_node_pcap;
    }
    // norm_node_pcap is set as lowest between sys (N+1 mode) and
    // user in amec_data_write_pcap()
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
        l_active_pcap_changed = TRUE;
    }

    l_avail_power = g_amec->pcap.active_node_pcap - l_node_pwr;

    // Determine GPU power cap if there are GPUs present
    if(G_first_proc_gpu_config)
    {
       amec_gpu_pcap(i_oversub_state, l_active_pcap_changed, l_avail_power);
    }

    if(l_node_pwr != 0)
    {
        l_proc_fraction = ((uint32_t)(l_p0_pwr) << 16)/l_node_pwr;
        if(L_apss_error_traced)
        {
            TRAC_ERR("PCAP: PWRSYS sensor is no longer 0.");
            L_apss_error_traced = FALSE;
        }

        // check if allowed to increase power AND memory throttled due to pcap
        if((l_avail_power > 0) && (g_amec->pcap.active_mem_level != 0))
        {
            // un-throttle memory if there is enough available power between
            // current and new throttles
            if (CURRENT_MODE() == OCC_MODE_DISABLED)
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
                L_ticks_mem_pwr_available++;

                if( L_ticks_mem_pwr_available == UNTHROTTLE_MEMORY_DELAY )
                {
                    if( L_trace_pcap_unthrottle ||
                       (G_allow_trace_flags & ALLOW_MEM_TRACE) )
                    {
                        TRAC_IMP("PCAP: Un-Throttling memory");
                        L_trace_pcap_unthrottle = false;
                    }
                    g_amec->pcap.active_mem_level = 0;
                    L_ticks_mem_pwr_available = 0;
                    // don't let the proc have any available power this tick
                    l_avail_power = 0;
                }
            }
        }
        // check if need to reduce power and frequency is already at the min
        else if(l_avail_power < 0)
        {
            L_ticks_mem_pwr_available = 0;

            // if memory is not throttled and frequency is at min shed additional power
            // by throttling memory
            if( (g_amec->pcap.active_mem_level == 0) &&
                (g_amec->proc[0].pwr_votes.ppb_fmax == g_amec->sys.fmin_max_throttled) )
            {
                if( L_trace_pcap_throttle ||
                   (G_allow_trace_flags & ALLOW_MEM_TRACE) )
                {
                    TRAC_IMP("PCAP: Throttling memory");
                    L_trace_pcap_throttle = false;
                }
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
            TRAC_ERR("PCAP: PWRSYS sensor is showing a value of 0.");
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
           g_amec->proc[0].pwr_votes.nom_pcap_fmin = g_amec->sys.fmin_max_throttled;
        }
        else
        {
           g_amec->proc[0].pwr_votes.nom_pcap_fmin = G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MODE_DISABLED];
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

    l_power_avail = g_amec->pcap.active_proc_pcap - AMECSENSOR_PTR(PWRPROC)->sample;

    if(l_proc_pcap_vote > g_amec->proc[0].pwr_votes.nom_pcap_fmin)
    {
        l_proc_pcap_vote = g_amec->proc[0].core_max_freq +
                                                  (PROC_MHZ_PER_WATT * l_power_avail);
    }
    else
    {
        l_proc_pcap_vote += (PROC_MHZ_PER_WATT * l_power_avail);
    }

    if(l_proc_pcap_vote > G_proc_fmax_mhz)
    {
        l_proc_pcap_vote = G_proc_fmax_mhz;
    }

    if(l_proc_pcap_vote < g_amec->sys.fmin_max_throttled)
    {
        l_proc_pcap_vote = g_amec->sys.fmin_max_throttled;
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

    //CALCULATION done by MASTER OCC only.
    if(OCC_MASTER == G_occ_role)
    {
        //Power available is the ActiveNodePower - PowerDropThreshold - ActualPwr
        l_power_avail = g_amec->pcap.active_node_pcap - PDROP_THRESH - AMECSENSOR_PTR(PWRSYS)->sample;

        //Note: The PWRSYS value is read over the SPI bus, which has no error
        //detection. In order to prevent a single bad SPI transfer from causing
        //OCC to lower nominal core frequencies, we require the power to be over
        //the pcap for PPB_NOM_DROP_DELAY ticks before lowering PPB Fmax below
        //Fnom.
        if((g_amec->proc[0].pwr_votes.ppb_fmax == G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MODE_DISABLED])
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

            if(G_sysConfigData.master_ppb_fmax > G_proc_fmax_mhz)
            {
                G_sysConfigData.master_ppb_fmax = G_proc_fmax_mhz;
            }

            if(G_sysConfigData.master_ppb_fmax < g_amec->sys.fmin_max_throttled)
            {
                G_sysConfigData.master_ppb_fmax = g_amec->sys.fmin_max_throttled;
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

    if(G_pwr_reading_type == PWR_READING_TYPE_APSS)
    {
        const bool l_oversub_state = AMEC_INTF_GET_OVERSUBSCRIPTION();

        // if (power supply policy == redundant) or (not in oversubscription)
        if ((G_sysConfigData.system_type.non_redund_ps == FALSE) ||
            (l_oversub_state == FALSE))
        {
            // Calculate the pcap for the proc, memory and the power capping limit
            // for nominal cores.
            amec_pcap_calc(l_oversub_state);

            // skip processor changes until memory is un-capped
            if(!g_amec->pcap.active_mem_level)
            {
                // Calculate voting box input freq for staying with the current pcap
                amec_pcap_controller();

                // Calculate the performance preserving bounds voting box input freq
                amec_ppb_fmax_calc();
            }

            // Update the Processor and Memory Throttle due to power sensors
            if(g_amec->proc[0].pwr_votes.proc_pcap_vote < G_proc_fmax_mhz)
            {
                // Frequency is being throttled due to power cap
                sensor_update(AMECSENSOR_PTR(PROCPWRTHROT), 1);
            }
            else  // not currently throttled due to power
            {
                sensor_update(AMECSENSOR_PTR(PROCPWRTHROT), 0);
            }
            if(g_amec->pcap.active_mem_level != 0)
            {
                // Memory is being throttled due to power cap
                sensor_update(AMECSENSOR_PTR(MEMPWRTHROT), 1);
            }
            else  // not currently throttled due to power
            {
                sensor_update(AMECSENSOR_PTR(MEMPWRTHROT), 0);
            }
        }
        // else, dont run pcap algorithm while: oversubscription AND non-redundant ps
    }
    else
    {
        // No system power reading for power capping set pcap frequency votes to max
        g_amec->proc[0].pwr_votes.proc_pcap_nom_vote = G_proc_fmax_mhz;
        g_amec->proc[0].pwr_votes.proc_pcap_vote = G_proc_fmax_mhz;
        g_amec->proc[0].pwr_votes.ppb_fmax = G_proc_fmax_mhz;
    }
}

/*----------------------------------------------------------------------------*/
/* End                                                                        */
/*----------------------------------------------------------------------------*/
