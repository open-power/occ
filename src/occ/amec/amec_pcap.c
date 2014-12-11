/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/amec/amec_pcap.c $                                    */
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

//*************************************************************************
// Includes
//*************************************************************************
#include "amec_pcap.h"
#include "amec_sys.h"
#include "amec_service_codes.h"
#include <occ_common.h>
#include <occ_sys_config.h>
#include <dcom.h>
#include <trac.h>

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************
#define PPB_NOM_DROP_DELAY 4    //ticks

//*************************************************************************
// Structures
//*************************************************************************

//*************************************************************************
// Globals
//*************************************************************************

//Number of ticks to wait before dropping below nominal frequency
#define PWR_SETTLED_TICKS   4
//Number of watts power must be below the node power cap before raising
//ppb_fmax
#define PDROP_THRESH        0   //TODO: need better value.
//Number of MHz to raise the proc_pcap_vote for every watt of available power
//(DCM value should be less than SCM)
#define PROC_MHZ_PER_WATT   28  //TODO: need better value.
//Number of MHz to raise ppb_fmax per watt of available power. Depends on
//number of procs in node.
#define NODE_MHZ_PER_WATT()  \
                          (G_sysConfigData.sys_num_proc_present == 0?   \
                           1:                                           \
                           ((PROC_MHZ_PER_WATT/G_sysConfigData.sys_num_proc_present) == 0? \
                            1:                                          \
                            PROC_MHZ_PER_WATT/G_sysConfigData.sys_num_proc_present))

//Frequency_step_khz (from global pstate table)/1000
uint32_t    G_mhz_per_pstate=0;   //TODO: Maybe there's a better value to initilize it to.

uint8_t     G_over_pcap_count=0;

//*************************************************************************
// Function Prototypes
//*************************************************************************

//*************************************************************************
// Functions
//*************************************************************************

//////////////////////////
// Function Specification
//
// Name: amec_pmax_clip_controller
//
// Description: Calculate the pmax_clip_freq vote.  Initialized to Turbo.
//
// Thread: Real Time Loop
//
// End Function Specification
void amec_pmax_clip_controller(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    uint16_t l_fturbo = G_sysConfigData.sys_mode_freq.table[OCC_MODE_TURBO];
    uint32_t l_pmax_clip_freq = g_amec->proc[0].pwr_votes.pmax_clip_freq;
    Pstate   l_pstate = 0;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    //Note: quickPowerDrop interrupts will not preempt the real time loop
    //      interrupt. No locking is needed between the two interrupts.
    //Note: quickPowerDropLatchAmec represents the failsafe signal on ITEs and
    //      oversubscription signal on non-ITEs.

    // See the oversub event and control oversub in AMEC
    if(AMEC_INTF_GET_OVERSUBSCRIPTION()&&
       (g_amec->oversub_status.oversubLatchAmec==FALSE) )
    {
        // ISR already did it but still need to do it again here due to
        // l_pmax_clip_freq is incorrect
        l_pmax_clip_freq = G_sysConfigData.sys_mode_freq.table[OCC_MODE_MIN_FREQUENCY];
        g_amec->oversub_status.oversubLatchAmec = TRUE;
    }
    else if( !AMEC_INTF_GET_OVERSUBSCRIPTION() )
    {
        // AMEC doesn't control it and let ISR do it
        g_amec->oversub_status.oversubLatchAmec = FALSE;
    }

    if(l_pmax_clip_freq < l_fturbo)
    {
        l_pmax_clip_freq += G_mhz_per_pstate;

        if(l_pmax_clip_freq > l_fturbo)
        {
            l_pmax_clip_freq = l_fturbo;
        }

        //call proc_freq2pstate
        l_pstate = proc_freq2pstate(l_pmax_clip_freq);

        //Set the pmax_clip register via OCI write.
        amec_oversub_pmax_clip(l_pstate);
    }

    g_amec->proc[0].pwr_votes.pmax_clip_freq = l_pmax_clip_freq;
}

//////////////////////////
// Function Specification
//
// Name: amec_pcap_calc
//
// Description: Calculate the node power cap and the processor power cap.
//
// Thread: Real Time Loop
//
// End Function Specification
void amec_pcap_calc(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    bool l_failsafe_state = AMEC_INTF_GET_FAILSAFE();
    bool l_oversub_state  = 0;
    uint16_t l_node_pwr = AMECSENSOR_PTR(PWR250US)->sample;
    uint16_t l_p0_pwr   = AMECSENSOR_PTR(PWR250USP0)->sample;
    int32_t l_avail_power = 0;
    uint32_t l_proc_fraction = 0;
    static uint32_t L_prev_node_pcap = 0;
    static bool l_apss_error_traced = FALSE;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    //TRAC_INFO("amec_pcap_calc: Calculate active_node_pcap, and nom_pcap_fmin.");
    l_oversub_state = AMEC_INTF_GET_OVERSUBSCRIPTION();

    //Set the active_node_pcap in g_amec.
    if(TRUE == l_failsafe_state)
    {
        //Set active_node_pcap to soft min pcap since failsafe is on
        g_amec->pcap.active_node_pcap = G_sysConfigData.pcap.soft_min_pcap;
    }
    else if(TRUE == l_oversub_state)
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
        if(l_apss_error_traced)
        {
            TRAC_ERR("PCAP: PWR250US sensor is no longer 0.");
            l_apss_error_traced = FALSE;
        }
    }
    else
    {
        if(!l_apss_error_traced)
        {
            TRAC_ERR("PCAP: PWR250US sensor is showing a value of 0.");
            l_apss_error_traced = TRUE;
        }
    }

    g_amec->pcap.active_proc_pcap = l_p0_pwr + ((l_proc_fraction * l_avail_power) >> 16);

    //TRAC_INFO("PCAP: calculated active proc pcap: avail_power[0x%X],proc_fraction[0x%X],"
    //         "active_proc_pcap[0x%X].",l_avail_power,l_proc_fraction,g_amec->pcap.active_proc_pcap);

    //NOTE: Power capping will not affect nominal cores unless a customer power cap is set below the
    //      max pcap or oversubscription occurs.
    //      However, nominal cores will drop below nominal if ppb_fmax drops below nominal.
    if(g_amec->pcap.active_node_pcap < G_sysConfigData.pcap.max_pcap)
    {
        g_amec->proc[0].pwr_votes.nom_pcap_fmin = G_sysConfigData.sys_mode_freq.table[OCC_MODE_MIN_FREQUENCY];
    }
    else
    {
        g_amec->proc[0].pwr_votes.nom_pcap_fmin = G_sysConfigData.sys_mode_freq.table[OCC_MODE_NOMINAL];
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
// Name: amec_conn_oc_controller
//
// Description: Handle Over Current.  Useful on ITE system only.
//              Design uses 2 step procedure when OC is detected.
//              1) Lower all cores to Fnom
//              2) If still asserted after PWR_SETTLED_TICKS, lower all cores to Fmin
//
// Thread: Real Time Loop
//
// End Function Specification
void amec_conn_oc_controller(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    errlHndl_t  l_err = NULL;
    static uint8_t  L_asserted_count = 0;
    static bool L_error_logged = FALSE;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    //If no overcurrent pins are set, then set vote to Turbo
    if(G_conn_oc_pins_bitmap == 0)
    {
        g_amec->proc[0].pwr_votes.conn_oc_vote = G_sysConfigData.sys_mode_freq.table[OCC_MODE_TURBO];
    }
    else
    {
        //Check if any of the OC gpio pins are set.
        //TODO: G_conn_oc_pins_bitmap should be populated with data sent by TMGT.
        if((G_conn_oc_pins_bitmap & ~(G_apss_pwr_meas.gpio[0])) == 0)
        {
            //Reset counter
            L_asserted_count = 0;

            g_amec->proc[0].pwr_votes.conn_oc_vote += G_mhz_per_pstate;

            //Verify that vote is not greater than turbo.
            if(g_amec->proc[0].pwr_votes.conn_oc_vote > G_sysConfigData.sys_mode_freq.table[OCC_MODE_TURBO])
            {
                g_amec->proc[0].pwr_votes.conn_oc_vote = G_sysConfigData.sys_mode_freq.table[OCC_MODE_TURBO];
            }
        }
        else if(L_asserted_count < PWR_SETTLED_TICKS)
        {
            g_amec->proc[0].pwr_votes.conn_oc_vote = G_sysConfigData.sys_mode_freq.table[OCC_MODE_NOMINAL];
            L_asserted_count++;

            //Log an informational error
            TRAC_ERR("Connector overcurrent pins still asserted. Count=%i.",L_asserted_count);

            /* @
             * @errortype
             * @moduleid    AMEC_PCAP_CONN_OC_CONTROLLER
             * @reasoncode  CONNECTOR_OC_PINS_WARNING
             * @userdata1   OC pin bitmap
             * @userdata2   APSS power measure gpio state
             * @devdesc     The connector overcurrent pins are still asserted.
             *
             */

            //TODO: ADD ACTION FLAG for manufacturing error to log it as predictive.
            if(!L_error_logged) //only log this error once
            {
                L_error_logged = TRUE;
                l_err = createErrl(
                     AMEC_PCAP_CONN_OC_CONTROLLER,       //modId
                     CONNECTOR_OC_PINS_WARNING,          //reasoncode
                     OCC_NO_EXTENDED_RC,                 //Extended reason code
                     ERRL_SEV_INFORMATIONAL,             //Severity
                     NULL,                               //Trace Buf
                     DEFAULT_TRACE_SIZE,                 //Trace Size
                     G_conn_oc_pins_bitmap,              //userdata1
                     G_apss_pwr_meas.gpio[0]             //userdata2
                     );

                commitErrl( &l_err);
            }
        }
        else //Asserted count reached or exceeded PWR_SETTLED_TICKS.
        {
            g_amec->proc[0].pwr_votes.conn_oc_vote = G_sysConfigData.sys_mode_freq.table[OCC_MODE_MIN_FREQUENCY];

            if(CURRENT_MODE() == OCC_MODE_NOMINAL)
            {
                //Log a predictive error
                TRAC_ERR("Connector overcurrent pins still asserted after max ticks. Logging error.");

                /* @
                 * @errortype
                 * @moduleid    AMEC_PCAP_CONN_OC_CONTROLLER
                 * @reasoncode  CONNECTOR_OC_PINS_FAILURE
                 * @userdata1   OC pin bitmap
                 * @userdata2   APSS power measure gpio state
                 * @devdesc     The connector overcurrent pins are asserted for too long.
                 *
                 */

                l_err = createErrl(
                        AMEC_PCAP_CONN_OC_CONTROLLER,       //modId
                        CONNECTOR_OC_PINS_FAILURE,          //reasoncode
                        OCC_NO_EXTENDED_RC,                 //Extended reason code
                        ERRL_SEV_PREDICTIVE,                //Severity
                        NULL,                               //Trace Buf
                        DEFAULT_TRACE_SIZE,                 //Trace Size
                        G_conn_oc_pins_bitmap,              //userdata1
                        G_apss_pwr_meas.gpio[0]             //userdata2
                        );

                commitErrl( &l_err);

                //Request safe mode without retries.
                //TODO: add code to request safe mode with no retries.
            }
        }
    }
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

    // Call amec pmax clip controller to control the Pmax_clip register setting
    // and voting box input.
    amec_pmax_clip_controller();

    // Calculate the power cap for the processor and the power capping limit
    // for nominal cores.
    amec_pcap_calc();

    // Calculate the voting box input frequency for staying with the current pcap
    amec_pcap_controller();

    // Calculate the performance preserving bounds voting box input frequency.
    amec_ppb_fmax_calc();

    // Check for connector overcurrent condition and calculate voting box input frequency.
    amec_conn_oc_controller();
}

/*----------------------------------------------------------------------------*/
/* End                                                                        */
/*----------------------------------------------------------------------------*/
