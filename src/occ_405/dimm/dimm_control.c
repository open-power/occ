/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/dimm/dimm_control.c $                             */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2018                        */
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
#include "dimm_control.h"
#include "dimm_structs.h"
#include "errl.h"
#include "trac.h"
#include "rtls.h"
#include "apss.h"
#include "state.h"
#include "amec_sys.h"
#include "memory.h"
#include "common.h"

//GPE IPC request and parms for the GPE job used for DIMM modules control.
//extern GpeRequest G_dimm_control_request;
extern dimm_control_args_t G_dimm_control_args;

extern memory_control_task_t G_memory_control_task;

// A bit vector that indicated that the dimms (mbas) are configured
// in a nimbus (cumulus) system.
// Bit 0 (MSB) encodes the configuration status for port 0 on MC01
// (mba01 on centaur 0).
// a 1 indicated a dimm (mba) is configured for throttling.
// most significant 8 bits are used to cover nimbus' two MC pairs of 4 ports each.
// Initialized to 0, sat by the FSP memory throttling configuration command
//   (0x21, format 0x12)
extern uint16_t G_configured_mbas;

// Bit vector that allows certain traces to run
extern uint16_t G_allow_trace_flags;
//////////////////////////
// Function Specification
//
// Name: dimm_control
//
// Description: RDIMM modules control.
//              Schedule a GPE IPC task to control the DIMMs whose
//              speed requested by the thermal controller has changed.
//
// Thread: RTL
//
// End Function Specification

bool dimm_control(uint8_t mc, uint8_t port)
{
    DIMM_DBG("dimm_control: called at tick %d", CURRENT_TICK);

    //update the min/max settings for all DIMMs according to mode
    dimm_update_nlimits(mc, port);

    // Convert speed request to N value, load N values into
    // GPE1 G_dimm_control_args.dimmNumeratorValues struct
    // and populate G_dimm_control_args.mc/port values
    // corresponding to dimm to be throttled.
    G_dimm_control_args.dimmNumeratorValues.new_n = FALSE; // Reset New N Value Flag prior to calling
    populate_dimm_control_args(g_amec->mem_speed_request, mc, port,
                               &G_dimm_control_args);

    // Check if the throttle value has been updated since the last time we
    // sent it.  If it has, then send a new value, otherwise do nothing.
    if(G_dimm_control_args.dimmNumeratorValues.new_n)
    {
        DIMM_DBG("dimm throttle control changed: MC=%d. port=%d , Throttle=%d",
                  mc, port, g_amec->mem_speed_request);
        return TRUE;
    }

    return FALSE;
}




//////////////////////////
// Function Specification
//
// Name: dimm_update_nlimits
//
// Description: Updates all memory dimms throttle settings, including:
//              1) new settings from FSP
//              2) change to/from TURBO or DPS mode
//              3) enter/exit oversubscription
//
//
// Thread: RTL
//
// End Function Specification

void dimm_update_nlimits(uint8_t mc, uint8_t port)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    static bool L_first_trace = true;
    uint16_t l_port_dimm_maxn = 0, l_slot_dimm_maxn = 0;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    do
    {
        memory_throttle_t* l_active_limits;
        mem_throt_config_data_t* l_state_limits;

        // convert N Values for configured DIMMs only
        if(NIMBUS_DIMM_THROTTLING_CONFIGURED(G_configured_mbas,mc,port))
        {
            l_active_limits = &G_memoryThrottleLimits[mc][port];
            l_state_limits  = &G_sysConfigData.mem_throt_limits[mc][port];

            //Minimum N value is not state dependent
            l_active_limits->min_n_per_mba = l_state_limits->min_n_per_mba;

            //Power Capping memory?
            if(g_amec->pcap.active_mem_level == 1)
            {
                l_port_dimm_maxn = l_state_limits->pcap_n_per_chip;
                l_slot_dimm_maxn = l_state_limits->pcap_n_per_mba;
            }
            else if(CURRENT_MODE() == OCC_MODE_NOMINAL)
            {
                l_port_dimm_maxn = l_state_limits->nom_n_per_chip;
                l_slot_dimm_maxn = l_state_limits->nom_n_per_mba;
            }
            else //all other modes will use turbo settings
            {
                l_port_dimm_maxn = l_state_limits->turbo_n_per_chip;
                l_slot_dimm_maxn = l_state_limits->turbo_n_per_mba;
            }

            l_active_limits->max_n_per_chip = l_port_dimm_maxn;

            //Trace when the dimm slot max N value changes
            if(l_slot_dimm_maxn != l_active_limits->max_n_per_mba)
            {
                l_active_limits->max_n_per_mba = l_slot_dimm_maxn;

                if( (L_first_trace) ||
                    (G_allow_trace_flags & ALLOW_MEM_TRACE) )
                {
                    TRAC_IMP("dimm_update_nlimits: New DIMM slot throttle values: "
                             "MC#|Port:[0x%04x], "
                             "Max|Min N_PER_MBA:[0x%08x], Max N_PER_CHIP:[0x%04x] ",
                             (uint16_t)((mc << 8) | port),
                             (uint32_t)( (l_active_limits->max_n_per_mba << 16) |
                                         l_active_limits->min_n_per_mba),
                             l_active_limits->max_n_per_chip);
                    L_first_trace = false;
                }
            }

        } // NIMBUS_DIMM_THROTTLING_CONFIGURED ?

    }while(0);
}


//////////////////////////
// Function Specification
//
// Name: populate_dimm_control_args
//
// Description: Converts dimm throttle percentage into 'N' value
//               that can be written to the hardware, load N values
//               into GPE1 G_dimm_control_args.dimmNumeratorValue,
//               and populate G_dimm_control_args.mc/port for the
//               corresponding dimm.
//
//
// Thread: RTL
//
// End Function Specification

#define DIMM_THROTTLE_100_PERCENT_VALUE 1000
void populate_dimm_control_args(uint16_t i_throttle, uint8_t mc, uint8_t port,
                                dimm_control_args_t * dimm_control_args)
{
    dimm_n_value_t dimm_nvalue;
    memory_throttle_t* l_dimm_throttle;

    // MC01 = 0, MC23 = 1
    l_dimm_throttle = &G_memoryThrottleLimits[mc][port];

    // a DIMM is configured?
    if(NIMBUS_DIMM_THROTTLING_CONFIGURED(G_configured_mbas,mc,port))
    {
        // Convert the dimm throttle (in units of 0.1 %) to "N" value
        dimm_nvalue.slot_n =
            convert_speed2numerator(i_throttle,
                                    l_dimm_throttle->min_n_per_mba,
                                    l_dimm_throttle->max_n_per_mba);

        dimm_nvalue.port_n = l_dimm_throttle->max_n_per_chip;

        // A change in the N value for dimm control args(mc,port)?
        if(dimm_nvalue.word32 !=
           g_amec->sys.current_dimm_n_values[mc][port].word32)
        {
            dimm_control_args->dimmNumeratorValues.word32 = dimm_nvalue.word32;
            g_amec->sys.current_dimm_n_values[mc][port].word32 =
                dimm_nvalue.word32;
            dimm_control_args->dimmNumeratorValues.new_n = 1;
            dimm_control_args->mc = mc;
            dimm_control_args->port = port;
        }
    }
}

//////////////////////////
// Function Specification
//
// Name: convertSpeed2Numerator
//
// Description: Converts dimm throttle percentages into 'N' value
//              that can be written to the hardware.
//
// Thread: RTL
//
// End Function Specification

uint16_t convert_speed2numerator(uint16_t i_throttle, uint16_t min_n_value, uint16_t max_n_value)
{
    uint16_t l_nvalue = 0;

    // Convert the throttle (in units of 0.1 %) to "N" value
    l_nvalue = (max_n_value * i_throttle) / DIMM_THROTTLE_100_PERCENT_VALUE;

    //Clip to DIMM's per-slot min and max values
    if(l_nvalue < min_n_value)
    {
        l_nvalue = min_n_value;
    }
    if(l_nvalue > max_n_value)
    {
        l_nvalue = max_n_value;
    }

    return l_nvalue;
}
