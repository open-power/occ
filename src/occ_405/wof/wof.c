/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/wof/wof.c $                                       */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2016,2023                        */
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
#include <errl.h>
#include <trac.h>
#include <sensor.h>
#include <occhw_async.h>
#include <pgpe_shared.h>
#include <pstate_pgpe_occ_api.h>
#include <pstates_occ.H>
#include <occ_service_codes.h>
#include <wof_service_codes.h>
#include <amec_sys.h>
#include <occ_sys_config.h>
#include <wof.h>
#include <amec_freq.h>
#include <pgpe_interface.h>
#include <avsbus.h>
#include "common.h"             // For ignore_pgpe_error()
#include "memory.h"
//******************************************************************************
// External Globals
//******************************************************************************
extern amec_sys_t g_amec_sys;
extern OCCPstateParmBlock_t G_oppb;
extern GPE_BUFFER(ipcmsg_wof_vrt_t G_wof_vrt_parms);
extern GPE_BUFFER(ipcmsg_wof_control_t G_wof_control_parms);
extern GpeRequest   G_wof_vrt_req;
extern GpeRequest   G_wof_control_req;
extern uint32_t     G_occ_frequency_mhz;
extern volatile pstateStatus G_proc_pstate_status;
extern uint8_t G_occ_interrupt_type;
extern uint16_t G_allow_trace_flags;
//******************************************************************************
// Globals
//******************************************************************************
uint8_t G_sram_vrt_ping_buffer[MIN_BCE_REQ_SIZE] __attribute__ ((section(".vrt_ping_buffer")));
uint8_t G_sram_vrt_pong_buffer[MIN_BCE_REQ_SIZE] __attribute__ ((section(".vrt_pong_buffer")));

// BCE Request object for retrieving VRT's from Mainstore
BceRequest G_vrt_req;

// Buffer to hold vrt from main memory
DMA_BUFFER(temp_bce_request_buffer_t G_vrt_temp_buff) = {{0}};

// Create a pointer to amec WOF structure
amec_wof_t * g_wof = &(g_amec_sys.wof);

// Core IDDQ voltages array (voltages in 100uV)
uint16_t G_iddq_voltages[CORE_IDDQ_MEASUREMENTS] =
{
     5000,
     6000,
     7000,
     8000,
     9000,
    10000
};

//******************************************************************************
// Function Definitions
//******************************************************************************

/**
 * call_wof_main
 *
 * Description: Performs the Initialization of the WOF infrastructure
 *              such that the WOF algorithm can run. This includes making
 *              sure the PGPE is ready to perform WOF calculations and enforcing
 *              when WOF should wait a tick to perform a calc or disable wof
 *              entirely. Called from amec_slave_smh.c::amec_slv_common_tasks_post.
 * Param: None
 *
 * Return: None
 */
void call_wof_main( void )
{
    // Timeout for VRT to complete
    static uint8_t L_vrt_last_chance = MAX_VRT_CHANCES_EVERY_TICK;

    // Timeout for WOF Control to complete
    static uint8_t L_wof_control_last_chance = MAX_WOF_CONTROL_CHANCES_EVERY_TICK;

    // Variable to keep track of logging timeouts being ignored
    // WOF runs every 500us all timeouts must be at least 1ms for PGPE
    // to have time to set the bit to give ignore indication
    static bool L_current_timeout_recorded = false;

    // Variable to keep track of PState enablement to prevent setting/clearing
    // wof_disabled bit every iteration.
    static uint8_t L_pstate_protocol_off = 0;
    // GpeRequest more than 1 extra time.
    bool enable_success = false;
    bool l_idle = false;
    do
    {
        // If the init state says we just turned WOF on in pgpe, clear
        // PGPE wof disabled bit
        if(g_wof->wof_init_state == PGPE_WOF_ENABLED_NO_PREV_DATA)
        {
            set_clear_wof_disabled( CLEAR,
                                    WOF_RC_PGPE_WOF_DISABLED,
                                    ERC_WOF_PGPE_WOF_DISABLED );
        }

        // If error logged in callback, record now
        if( g_wof->vrt_callback_error )
        {
            INTR_TRAC_ERR("Got a bad RC in wof_vrt_callback: 0x%x",
                    g_wof->wof_vrt_req_rc);
            if(G_allow_trace_flags & ALLOW_VRT_TRACE)
            {
                INTR_TRAC_INFO("call WOF Main VRT failed. OTBR[0x%08X]",
                                in32(OCB_OTBR));
            }

            set_clear_wof_disabled( SET,
                                    WOF_RC_VRT_REQ_FAILURE,
                                    ERC_WOF_VRT_REQ_FAILURE );

            // After official error recorded, prevent this code
            // from running from same setting of the var.
            g_wof->vrt_callback_error = 0;
        }

        // If the 405 turned WOF off on pgpe and it is the only bit set
        // clear the bit so we can re-enable WOF
        if( g_wof->pgpe_wof_off &&
           (g_wof->wof_disabled == WOF_RC_PGPE_WOF_DISABLED) )
        {
            g_wof->pgpe_wof_off = 0;
            set_clear_wof_disabled( CLEAR,
                                    WOF_RC_PGPE_WOF_DISABLED,
                                    ERC_WOF_PGPE_WOF_DISABLED );

        }

        // Make sure wof has not been disabled
        if( g_wof->wof_disabled )
        {
            if( g_wof->pgpe_wof_disabled )
            {
                set_clear_wof_disabled( SET,
                                        WOF_RC_PGPE_WOF_DISABLED,
                                        ERC_WOF_PGPE_WOF_DISABLED );
                g_wof->pgpe_wof_disabled = 0;
            }

            // if the only reason WOF isn't running is due to Static Freq Point mode
            // allow WOF to run thru calculations anyway for lab debug.  WOF will not
            // be enabled on the PGPE to avoid actual WOF clipping
            if( ((g_wof->wof_disabled & (~WOF_RC_MODE_NO_SUPPORT_MASK)) == 0) &&
                (CURRENT_MODE() == OCC_MODE_STATIC_FREQ_POINT) &&
                (IS_OCC_STATE_ACTIVE()) )
            {
                wof_main();
            }

            break;
        }

        // Make sure Pstate Protocol is on
        if(G_proc_pstate_status != PSTATES_ENABLED)
        {
            if( L_pstate_protocol_off == 0 )
            {
                INTR_TRAC_ERR("WOF Disabled! Pstate Protocol off");
                set_clear_wof_disabled( SET,
                                        WOF_RC_PSTATE_PROTOCOL_OFF,
                                        ERC_WOF_PSTATE_PROTOCOL_OFF );
                L_pstate_protocol_off = 1;
            }
            // Since Pstates are off, break out
            break;
        }
        else if(G_proc_pstate_status == PSTATES_ENABLED)
        {
            if( L_pstate_protocol_off == 1 )
            {
                INTR_TRAC_INFO("Pstate Protocol on! Clearing PSTATE_PROTOCOL_OFF");
                set_clear_wof_disabled( CLEAR,
                                        WOF_RC_PSTATE_PROTOCOL_OFF,
                                        ERC_WOF_PSTATE_PROTOCOL_OFF );
                L_pstate_protocol_off = 0;
            }
        }

        // Ensure the OCC is active
        if( IS_OCC_STATE_ACTIVE() )
        {
            // Make sure we are not disabled
            if( !g_wof->wof_disabled &&
                 g_wof->wof_init_state < PGPE_WOF_ENABLED_NO_PREV_DATA )
            {
                switch( g_wof->wof_init_state )
                {
                    // For each possible initialization state,
                    // do the appropriate action
                    case WOF_DISABLED:
                        // Reset timeouts for VRT response and WOF control
                        L_vrt_last_chance = MAX_VRT_CHANCES;
                        L_wof_control_last_chance = MAX_WOF_CONTROL_CHANCES;

                        // reset OC ceff adder
                        g_wof->vdd_oc_ceff_add = 0;
                        sensor_update(AMECSENSOR_PTR(OCS_ADDR), (uint16_t)g_wof->vdd_oc_ceff_add);

                        // reset throttle ceff adder
                        g_wof->vdd_throt_ceff_add = 0;
                        sensor_update(AMECSENSOR_PTR(THROTTLE_ADDR), (uint16_t)g_wof->vdd_throt_ceff_add);

                        // calculate initial vrt, send gpeRequest
                        // Initial vrt is the last vrt in Main memory
                        send_initial_vrt_to_pgpe();
                        break;

                    case INITIAL_VRT_SENT_WAITING:
                        // Check if request is still processing.
                        // Init state updated in wof_vrt_callback
                        l_idle = async_request_is_idle(&G_wof_vrt_req.request);
                        if( (!l_idle) || (g_wof->vrt_state != STANDBY) )
                        {
                            if( (L_vrt_last_chance == 0) && (!ignore_pgpe_error()) )
                            {
                                INTR_TRAC_ERR("WOF Disabled! Init VRT req timeout! state[%d] IPC idle?[%d]", g_wof->vrt_state, l_idle);
                                // if state is SCHEDULED that means the BCE completed, trace time for BCE
                                if( (!l_idle) && (g_wof->vrt_state == SCHEDULED) )
                                {
                                    sensor_t *l_bce_sensor = getSensorByGsid(VRT_BCEdur);
                                    INTR_TRAC_ERR("Initial VRT timeout! BCE completed in %dus",
                                                   l_bce_sensor->sample);
                                }
                                set_clear_wof_disabled( SET,
                                                        WOF_RC_VRT_REQ_TIMEOUT,
                                                        ERC_WOF_VRT_REQ_TIMEOUT );
                            }
                            else if(L_vrt_last_chance != 0)
                            {
                                if( L_vrt_last_chance == 1 )
                                {
                                    INTR_TRAC_INFO("Initial VRT NOT complete state[%d] idle?[%d]. Last chance out of %d chances",
                                                    g_wof->vrt_state, l_idle, MAX_VRT_CHANCES);
                                }
                                L_vrt_last_chance--;
                            }
                            else
                            {
                                // Wait forever for PGPE to respond
                                // Put a mark on the wall so we know we hit this state
                                if(!L_current_timeout_recorded)
                                {
                                    INCREMENT_ERR_HISTORY(ERRH_VRT_TIMEOUT_IGNORED);
                                    L_current_timeout_recorded = TRUE;
                                }
                            }
                        }
                        break;

                    case INITIAL_VRT_SUCCESS:
                        // We made it this far. Reset Last chance
                        L_vrt_last_chance = MAX_VRT_CHANCES;

                        // Send wof control on gpe request
                        // If enable_success returns true, init state was set
                        enable_success = enable_wof();
                        if( !enable_success )
                        {
                            // Treat as an error only if not currently ignoring PGPE failures
                            if( (L_wof_control_last_chance == 0) && (!ignore_pgpe_error()) )
                            {
                                sensor_t *l_ipc_sensor = getSensorByGsid(WOFC_IPCdur);
                                INTR_TRAC_ERR("INITIAL_VRT_SUCCESS: WOF Control Time Out! IPC max %dus",
                                               l_ipc_sensor->sample_max);
                                set_clear_wof_disabled(SET,
                                                       WOF_RC_CONTROL_REQ_TIMEOUT,
                                                       ERC_WOF_CONTROL_REQ_TIMEOUT);
                            }
                            else if(L_wof_control_last_chance != 0)
                            {
                                if(L_wof_control_last_chance == 1 )
                                {
                                    INTR_TRAC_ERR("INITIAL_VRT_SUCCESS: Last chance for WOF control request out of %d chances ",
                                                   MAX_WOF_CONTROL_CHANCES);
                                }
                                L_wof_control_last_chance--;
                            }
                            else
                            {
                                // Wait forever for PGPE to respond
                                // Put a mark on the wall so we know we hit this state
                                if(!L_current_timeout_recorded)
                                {
                                    INCREMENT_ERR_HISTORY(ERRH_WOF_CONTROL_TIMEOUT_IGNORED);
                                    L_current_timeout_recorded = TRUE;
                                }
                            }
                        }
                        else
                        {
                            // Reset the last chance variable
                            // Init state updated in enable_wof
                            L_wof_control_last_chance = MAX_WOF_CONTROL_CHANCES;

                            L_current_timeout_recorded = FALSE;
                        }
                        break;

                    case WOF_CONTROL_ON_SENT_WAITING:
                        // check if request is still processing.
                        if( !async_request_is_idle(&G_wof_control_req.request) )
                        {
                            // Treat as an error only if not currently ignoring PGPE failures
                            if( (L_wof_control_last_chance == 0) && (!ignore_pgpe_error()) )
                            {
                                sensor_t *l_ipc_sensor = getSensorByGsid(WOFC_IPCdur);
                                INTR_TRAC_ERR("WOF Control Time Out! IPC max %dus",
                                               l_ipc_sensor->sample_max);
                                set_clear_wof_disabled(SET,
                                                       WOF_RC_CONTROL_REQ_TIMEOUT,
                                                       ERC_WOF_CONTROL_REQ_TIMEOUT );
                            }
                            else if(L_wof_control_last_chance != 0)
                            {
                                if(L_wof_control_last_chance == 1 )
                                {
                                    INTR_TRAC_ERR("Last chance for WOF control request out of %d chances ",
                                                   MAX_WOF_CONTROL_CHANCES);
                                }
                                L_wof_control_last_chance--;
                            }
                            else
                            {
                                // Wait forever for PGPE to respond
                                // Put a mark on the wall so we know we hit this state
                                if(!L_current_timeout_recorded)
                                {
                                    INCREMENT_ERR_HISTORY(ERRH_WOF_CONTROL_TIMEOUT_IGNORED);
                                    L_current_timeout_recorded = TRUE;
                                }
                            }
                        }
                        else
                        {
                            L_current_timeout_recorded = FALSE;
                        }
                        // Init state updated in wof_control_callback
                        break;

                    default:
                        break;
                } // Switch statement
            }// initial state machine

            // If we have made it to at least WOF enabled no previous data
            // state run wof routine normally ensuring wof was not disabled
            // in previous 5 states
            if( (g_wof->wof_init_state >= PGPE_WOF_ENABLED_NO_PREV_DATA) &&
                !g_wof->wof_disabled )
            {
                // Normal execution of wof algorithm
                l_idle = async_request_is_idle(&G_wof_vrt_req.request);
                if( (!l_idle) || (g_wof->vrt_state != STANDBY) )
                {
                    if( L_vrt_last_chance == 0 )
                    {
                        // Treat as an error only if not currently ignoring PGPE failures
                        if(!ignore_pgpe_error())
                        {
                            INTR_TRAC_ERR("WOF Disabled! VRT req timeout! state[%d] IPC idle?[%d]", g_wof->vrt_state, l_idle);

                            sensor_t *l_bce_sensor = getSensorByGsid(VRT_BCEdur);
                            sensor_t *l_ipc_sensor = getSensorByGsid(VRT_IPCdur);
                            INTR_TRAC_ERR("VRT timeout! BCE max %dus  IPC max %dus",
                                           l_bce_sensor->sample_max, l_ipc_sensor->sample_max);

                            set_clear_wof_disabled(SET,
                                                   WOF_RC_VRT_REQ_TIMEOUT,
                                                   ERC_WOF_VRT_REQ_TIMEOUT);

                            if(G_allow_trace_flags & ALLOW_VRT_TRACE)
                            {
                                INTR_TRAC_INFO("VRT TIMEOUT OTBR[0x%08X]",
                                in32(OCB_OTBR));
                            }
                        }
                        else
                        {
                            // Wait forever for PGPE to respond
                            // Put a mark on the wall so we know we hit this state
                            if(!L_current_timeout_recorded)
                            {
                                INCREMENT_ERR_HISTORY(ERRH_VRT_TIMEOUT_IGNORED);
                                L_current_timeout_recorded = TRUE;
                            }
                        }
                    }
                    else
                    {
                        if( L_vrt_last_chance == 1 )
                        {
                            INTR_TRAC_INFO("VRT NOT complete state[%d] idle?[%d]. Last chance out of %d chances",
                                            g_wof->vrt_state, l_idle, MAX_VRT_CHANCES);
                        }
                        L_vrt_last_chance--;
                    }
                }
                else
                {
                    L_current_timeout_recorded = FALSE;
                    if(G_allow_trace_flags & ALLOW_VRT_TRACE)
                    {
                        INTR_TRAC_INFO("wof main VRT IDLE OTBR[0x%08X]",
                                        in32(OCB_OTBR));
                    }

                    // Request is idle. Run wof algorithm
                    wof_main();

                    L_vrt_last_chance = MAX_VRT_CHANCES;
                    // Finally make sure we are in the fully enabled state
                    if( g_wof->wof_init_state == PGPE_WOF_ENABLED_NO_PREV_DATA )
                    {
                        // remove any OC clip that was set while WOF was off now that OC
                        // will be handled by WOF vdd ceff ratio adjustment
                        if(g_amec->oc_wof_off.pstate_request)
                        {
                            INTR_TRAC_INFO("call_wof_main: WOF now enabled removing OC wof off pstate clip[0x%02X]",
                                            g_amec->oc_wof_off.pstate_request);
                            g_amec->oc_wof_off.pstate_request = 0;
                            g_amec->oc_wof_off.freq_request = 0xFFFF;
                        }

                        g_wof->wof_init_state = WOF_ENABLED;
                        // Set the the frequency ranges
                        errlHndl_t l_errl = amec_set_freq_range(CURRENT_MODE());
                        if(l_errl)
                        {
                            INTR_TRAC_ERR("call_wof_main: amec_set_freq_range reported an error");
                            commitErrl( &l_errl);
                        }
                    }
                }
            } // >= PGPE_WOF_ENABLED_NO_PREV_DATA
        } // IS_OCC_STATE_ACTIVE
    } while( 0 );
}

/**
 * wof_main
 *
 * Description: Main Wof algorithm
 *
 * Param: None
 *
 * Return: None
 */
void wof_main( void )
{
    // Read out the sensor data needed for calculations
    read_sensor_data();

    // Calculate the core leakage across the entire Proc
    calculate_core_leakage();

    // Calculate the AC currents
    calculate_AC_currents();

    // Calculate ceff_ratio_vdd and ceff_ratio_vcs
    // If we get v_ratio of 0 from pgpe, force ceff_ratio to 0;
    if( g_wof->v_ratio_vdd == 0 )
    {
        g_wof->ceff_ratio_vdd = 0;
    }
    else
    {
        calculate_ceff_ratio_vdd();
    }

    // Due to an issue with some Vcs VRMs we could get an invalid 0 current reading
    // resulting in an error trying to calculate ceff.  If vcs dimension is disabled
    // just skip the calculation since we don't need it anyway
    if( (g_wof->v_ratio_vcs == 0) ||
        (g_wof->vcs_override_index != WOF_VRT_IDX_NO_OVERRIDE) )
    {
        g_wof->ceff_ratio_vcs = 0;
    }
    else
    {
        calculate_ceff_ratio_vcs();
    }

    // Calculate how many steps from the beginning for each VRT parm that is not currently
    // disabled (checked once in read_wof_header) or being overwritten by mfg select VRT command
    if(g_wof->vcs_override_index == WOF_VRT_IDX_NO_OVERRIDE)
    {
        g_wof->vcs_step_from_start =
                             calculate_step_from_start( g_wof->ceff_ratio_vcs,
                                                        g_amec_sys.static_wof_data.wof_header.vcs_step,
                                                        g_amec_sys.static_wof_data.wof_header.vcs_start,
                                                        g_amec_sys.static_wof_data.wof_header.vcs_size );
    }
    else
    {
        // Use override value for VCS
        g_wof->vcs_step_from_start = g_wof->vcs_override_index;
    }

    if(g_wof->vdd_override_index == WOF_VRT_IDX_NO_OVERRIDE)
    {
        g_wof->vdd_step_from_start =
                             calculate_step_from_start( g_wof->ceff_ratio_vdd,
                                                        g_amec_sys.static_wof_data.wof_header.vdd_step,
                                                        g_amec_sys.static_wof_data.wof_header.vdd_start,
                                                        g_amec_sys.static_wof_data.wof_header.vdd_size );
    }
    else
    {
        // Use override value for VDD
        g_wof->vdd_step_from_start = g_wof->vdd_override_index;
    }

    // IO Power index was set in read_xgpe_values() only need to check for override here
    if(g_wof->io_pwr_override_index != WOF_VRT_IDX_NO_OVERRIDE)
    {
        // Use override value for IO Power
        g_wof->io_pwr_step_from_start = g_wof->io_pwr_override_index;
    }

    if(g_wof->ambient_override_index == WOF_VRT_IDX_NO_OVERRIDE)
    {
        g_wof->ambient_step_from_start =
                             calculate_step_from_start( g_wof->ambient_condition,
                                                        g_amec_sys.static_wof_data.wof_header.amb_cond_step,
                                                        g_amec_sys.static_wof_data.wof_header.amb_cond_start,
                                                        g_amec_sys.static_wof_data.wof_header.amb_cond_size );
    }
    else
    {
        // Use override value for ambient
        g_wof->ambient_step_from_start = g_wof->ambient_override_index;
    }

    // Compute the Main Memory address of the desired VRT table given
    // the calculated steps from start
    g_wof->vrt_main_mem_addr = calc_vrt_mainstore_addr();

    // skip sending VRT to PGPE if only went thru calculation due to SFP mode
    if(CURRENT_MODE() != OCC_MODE_STATIC_FREQ_POINT)
    {
        // Copy the new vrt from main memory to sram
        copy_vrt_to_sram( g_wof->vrt_main_mem_addr );
    }
}

/**
 * calculate_step_from_start
 *
 * Description: Calculates the step number for the given vrt parm
 *
 * Param[in]: i_vrt_parm - The current VRT parm to calculate the step for.
 * Param[in]: i_step_size - The size of each step for this VRT parm
 * Param[in]: i_min_step - The minimum step number for this VRT parm
 * Param[in]: i_max_step - The maximum step number for this VRT parm
 *
 * Return: The calculated number of steps for given VRT parm
 */
uint16_t calculate_step_from_start(uint16_t i_vrt_parm,
                                   uint16_t i_step_size,
                                   uint16_t i_min_step,
                                   uint16_t i_max_step )
{
    uint16_t l_current_step;

    // Check if VRT parm is for the first entry
    if( (i_vrt_parm <= i_min_step) || (i_step_size == 0) )
    {
        l_current_step = 0;
    }
    else
    {
        // Add step size to parm to round up.
        //  -1 to prevent overshoot when i_vrt_parm is equal to table value
        l_current_step = i_vrt_parm + i_step_size - 1;

        // Subtract the starting value to skip the 0 table entry
        l_current_step -= i_min_step;

        // Divide by step size to determine how many from the 0 entry VRT parm is
        l_current_step /= i_step_size;

        // If the calculated step is greater than the max step, use max step
        if( l_current_step >= i_max_step )
        {
            // Since function returns number of steps from start
            // (first entry is 0 from start) subtract 1.
            l_current_step = i_max_step-1;
        }
    }

    return l_current_step;
}

/**
 * calc_vrt_mainstore_address
 *
 * Description: Calculates the VRT address based on all VRT parm steps
 *
 * Return: The desired VRT main memory address
 */
uint32_t calc_vrt_mainstore_addr( void )
{
    static bool L_trace_char_test = true;
           uint32_t l_vrt_mm_offset = 0;

    // skip calculation and return first table if WOF Char testing is enabled
    if(G_internal_flags & INT_FLAG_ENABLE_WOF_CHAR_TEST)
    {
       if(L_trace_char_test)
       {
           INTR_TRAC_IMP("Entered WOF char testing using first VRT!");
           L_trace_char_test = false;
       }

       g_wof->vrt_mm_offset = 0;
    }
    else
    {
       if(!L_trace_char_test)
       {
           INTR_TRAC_IMP("Exited WOF char testing calculating VRT!");
           L_trace_char_test = true;
       }

       // Calculate the VRT offset in main memory
       // tables are sorted by Vcs, then Vdd, then IO Pwr, then ambient
       // first determine number of VRT tables need to be skipped due to Vcs
       l_vrt_mm_offset = g_wof->vcs_step_from_start * g_amec_sys.static_wof_data.wof_header.vdd_size * g_amec_sys.static_wof_data.wof_header.io_size * g_amec_sys.static_wof_data.wof_header.amb_cond_size;
       // next add number of tables skipped within this Vcs based on Vdd
       l_vrt_mm_offset += (g_wof->vdd_step_from_start * g_amec_sys.static_wof_data.wof_header.io_size * g_amec_sys.static_wof_data.wof_header.amb_cond_size);
       // now add number of tables skipped within this Vcs/Vdd based on IO power
       l_vrt_mm_offset += (g_wof->io_pwr_step_from_start * g_amec_sys.static_wof_data.wof_header.amb_cond_size);

       // now add number of tables skipped within this Vcs/Vdd/IO power based on ambient
       // if interpolation based on ambient is enabled need to check if need to copy 1 table prior in order to do
       // the interpolation
       if(G_internal_flags & INT_FLAG_ENABLE_WOF_AMBIENT_INTERP)
       {
            // check if the VRT will need to be interpolated based on ambient
            // no interpolation if already using lowest table or
            // if ambient condition is greater than last table or
            // if there is an ambient condition override set
            if( (g_wof->ambient_step_from_start == 0) ||
                (g_wof->ambient_condition >= g_amec_sys.static_wof_data.last_ambient_condition) ||
                (g_wof->ambient_override_index != WOF_VRT_IDX_NO_OVERRIDE) )
            {
                g_wof->interpolate_ambient_vrt = 0;
            }
            else
            {
                g_wof->interpolate_ambient_vrt = 1;
                g_wof->ambient_step_from_start--;
            }
       }
       else // interpolation disabled
       {
           g_wof->interpolate_ambient_vrt = 0;
       }
       l_vrt_mm_offset += g_wof->ambient_step_from_start;

       // We now have the total number of tables to skip multiply by the sizeof one VRT to get final offset
       l_vrt_mm_offset *= g_amec_sys.static_wof_data.wof_header.vrt_block_size;
       g_wof->vrt_mm_offset = l_vrt_mm_offset;
    }

    // Skip over the WOF header to find the base address where the tables actually start
    uint32_t wof_tables_mm_addr = g_amec_sys.static_wof_data.vrt_tbls_main_mem_addr + WOF_HEADER_SIZE + g_wof->vrt_mm_offset;

    // now we know the final address of the table we want, but need to make sure it is 128B aligned for BCE
    if(wof_tables_mm_addr%128 == 0)
    {
       // address is 128 byte aligned use it and set bce table offset to 0
       g_wof->vrt_bce_table_offset = 0;
    }
    else
    {
       // go down to nearest 128B alignment and store how many tables in the table we really want is
       // by design we have a guarantee that a VRT size is a factor of 128
       uint32_t overage = wof_tables_mm_addr%128;
       wof_tables_mm_addr -= overage;
       g_wof->vrt_bce_table_offset = overage / g_amec_sys.static_wof_data.wof_header.vrt_block_size;
    }
    return wof_tables_mm_addr;
}


/**
 * copy_vrt_to_sram_callback
 *
 * Description: Call back function to BCE request to copy VRT into SRAM
 *              ping/pong buffer. This call will also tell the PGPE
 *              that a new VRT is available
 *
 * Param[in]: i_parms - pointer to a struct that will hold data necessary to
 *                      the calculation.
 *                      -Pointer to vrt table temp buffer
 */
void copy_vrt_to_sram_callback( void )
{
    // save time BCE took in us
    uint32_t l_time = (uint32_t)((G_vrt_req.request.end_time - G_vrt_req.request.start_time) / ( SSX_TIMEBASE_FREQUENCY_HZ / 1000000 ));
    sensor_update( AMECSENSOR_PTR(VRT_BCEdur), (uint16_t)l_time);
/*
 *
 * find out which ping pong buffer to use
 * copy the vrt to said ping pong buffer
 * save current vrt address to global
 * setup VRT IPC command parameters to be sent to pgpe
 */

    // default use ping buffer
    uint8_t * l_buffer_address = G_sram_vrt_ping_buffer;

    // offset needed for 128B alignment to the VRT we really want from main memory
    uint32_t  l_vrt_offset = g_wof->vrt_bce_table_offset * g_amec_sys.static_wof_data.wof_header.vrt_block_size;
    uint8_t * l_vrt_address = (uint8_t *)(&G_vrt_temp_buff.data[0] + l_vrt_offset);

    // If ping buffer is currently in use then use the pong buffer
    if(g_wof->curr_ping_pong_buf == (uint32_t)G_sram_vrt_ping_buffer)
    {
        l_buffer_address = G_sram_vrt_pong_buffer;
    }

    // Update global "next" ping pong buffer for callback function
    g_wof->next_ping_pong_buf = (uint32_t)l_buffer_address;

    if(g_wof->interpolate_ambient_vrt)
    {
        // this will do the memcpy of newly interpolated VRT into the ping/pong buffer
        interpolate_ambient_vrt(l_buffer_address,
                                (uint8_t *)l_vrt_address);
    }
    else // no interpolation needed use VRT as is
    {
        // Copy the vrt data into the buffer
        memcpy( l_buffer_address,
                l_vrt_address,
                g_amec_sys.static_wof_data.wof_header.vrt_block_size );
    }

    // save 16B VRT contents for debug
    memcpy( &g_wof->VRT,
            l_buffer_address,
            16 );

    // Set the parameters for the GpeRequest
    G_wof_vrt_parms.idd_vrt_ptr = (VRT_t*)l_buffer_address;

    // check for Vratio override
    if(g_wof->v_ratio_override_index == WOF_VRT_IDX_NO_OVERRIDE)
    {
        // no override
        G_wof_vrt_parms.vratio_mode = 0;
    }
    else
    {
        // there is an override
        G_wof_vrt_parms.vratio_mode = 1;
    }
    G_wof_vrt_parms.fixed_vratio_index = g_wof->v_ratio_override_index;

    if( (g_wof->wof_init_state < INITIAL_VRT_SENT_WAITING) ||
        (g_wof->vdd_override_index != WOF_VRT_IDX_NO_OVERRIDE) )
    {
        // We didn't calculate this VRT using a real Vdd set ceff ratio to 0xFF's
        G_wof_vrt_parms.vdd_ceff_ratio = 0xFFFFFFFF;
    }
    else // set Vdd ratio used to determine VRT
    {
        G_wof_vrt_parms.vdd_ceff_ratio = g_wof->ceff_ratio_vdd;
    }
    if( (g_wof->wof_init_state < INITIAL_VRT_SENT_WAITING) ||
        (g_wof->vcs_override_index != WOF_VRT_IDX_NO_OVERRIDE) )
    {
        // We didn't calculate this VRT using a real Vcs set ceff ratio to 0xFF's
        G_wof_vrt_parms.vcs_ceff_ratio = 0xFFFFFFFF;
    }
    else // send Vcs ratio used to determine VRT
    {
        G_wof_vrt_parms.vcs_ceff_ratio = g_wof->ceff_ratio_vcs;
    }

    if( g_wof->vrt_state != STANDBY )
    {
        // Set vrt state to let OCC know it needs to schedule the IPC command
        g_wof->vrt_state = NEED_TO_SCHEDULE;
    }
}

/**
 * wof_vrt_callback
 *
 * Description: Callback function for G_wof_vrt_req GPE request to
 *              confirm the new VRT is being used by the PGPE and
 *              record the switch on the 405. Also updates the
 *              initialization
 */
void wof_vrt_callback( void )
{
    // save time IPC took in us
    uint32_t l_time = (uint32_t)((G_wof_vrt_req.request.end_time - G_wof_vrt_req.request.start_time) / ( SSX_TIMEBASE_FREQUENCY_HZ / 1000000 ));
    sensor_update( AMECSENSOR_PTR(VRT_IPCdur), (uint16_t)l_time);

    // Update the VRT state to indicate a new IPC message can be
    // scheduled regardless of the RC of the previous one.
    g_wof->vrt_state = STANDBY;

    if( G_wof_vrt_parms.msg_cb.rc == PGPE_RC_SUCCESS )
    {
       // GpeRequest went through successfully. update global ping pong buffer
       g_wof->curr_ping_pong_buf = g_wof->next_ping_pong_buf;

       // Update the wof_init_state based off the current state
       if( g_wof->wof_init_state == INITIAL_VRT_SENT_WAITING )
       {
            g_wof->wof_init_state = INITIAL_VRT_SUCCESS;
       }
    }
    else
    {
        // Disable WOF
        g_wof->vrt_callback_error = 1;
        g_wof->wof_vrt_req_rc = G_wof_vrt_parms.msg_cb.rc;
    }
}

/**
 * copy_vrt_to_sram
 *
 * Description: Function to copy new VRT from Mainstore to local SRAM buffer
 *              and calls copy_vrt_to_sram_callback function to setup new VRT
 *              to be sent to the PGPE
 *
 * Param[in]: i_vrt_main_mem_addr - Address of the desired vrt table.
 */
void copy_vrt_to_sram( uint32_t i_vrt_main_mem_addr )
{
    int l_ssxrc = SSX_OK;

    do
    {
        // First check if the address is 128-byte aligned. error if not.
        if( i_vrt_main_mem_addr % 128 )
        {
            INTR_TRAC_ERR("VRT Main Memory address NOT 128-byte aligned:"
                    " 0x%08x", i_vrt_main_mem_addr);
            set_clear_wof_disabled(SET,
                                   WOF_RC_VRT_ALIGNMENT_ERROR,
                                   ERC_WOF_VRT_ALIGNMENT_ERROR);

            break;
        }

        // Create request
        l_ssxrc = bce_request_create(
                                     &G_vrt_req,                 // block copy object
                                     &G_pba_bcde_queue,           // main to sram copy engine
                                     i_vrt_main_mem_addr,     //mainstore address
                                     (uint32_t) &G_vrt_temp_buff, // SRAM start address
                                     MIN_BCE_REQ_SIZE,  // size of copy
                                     SSX_WAIT_FOREVER,            // no timeout
                                     (AsyncRequestCallback)copy_vrt_to_sram_callback,
                                     NULL,
                                     ASYNC_CALLBACK_IMMEDIATE );

        if(l_ssxrc != SSX_OK)
        {
            INTR_TRAC_ERR("copy_vrt_to_sram: BCDE request create failure rc=[%08X]", -l_ssxrc);
            break;
        }

        // Make sure we are in correct vrt state
        if( g_wof->vrt_state == STANDBY )
        {
            // Set the VRT state to ensure asynchronous order of operations
            g_wof->vrt_state = SEND_INIT;

            // Do the actual copy
            l_ssxrc = bce_request_schedule( &G_vrt_req );

            if(l_ssxrc != SSX_OK)
            {
                INTR_TRAC_ERR("copy_vrt_to_sram: BCE request schedule failure rc=[%08X]", -l_ssxrc);
                break;
            }
        }
    }while( 0 );

    // Check for errors and log, if any
    if( l_ssxrc != SSX_OK )
    {
        // Formally disable WOF
        set_clear_wof_disabled( SET,
                                WOF_RC_IPC_FAILURE,
                                ERC_WOF_IPC_FAILURE );

        return;
    }
}

/**
 * read_pgpe_produced_wof_values
 *
 * Description: Read the PGPE Produced WOF values from OCC-PGPE shared SRAM and
 *              update sensors
 *              This is called every tick from amec_update_avsbus_sensors()
 *              regardless of WOF being enabled or not
 */
void read_pgpe_produced_wof_values( void )
{
    uint8_t  l_update_pwr_sensors = 0;
    uint16_t l_uv_avg_0p1pct = 0;
    uint16_t l_ov_avg_0p1pct = 0;
    static bool L_traced_no_readings       = FALSE;
    static bool L_traced_received_readings = FALSE;

    // Read in OCS bits from OCC Flag 0 register
    uint32_t occ_flags0 = 0;
    occ_flags0 = in32(OCB_OCCFLG0);
    g_wof->ocs_dirty = (uint8_t)(occ_flags0 & (OCS_PGPE_DIRTY_MASK | OCS_PGPE_DIRTY_TYPE_MASK));

    // INC counter for value of ocs_dirty
    if(g_wof->ocs_dirty == 0) // not dirty
        g_wof->ocs_not_dirty_count++;
    else if(g_wof->ocs_dirty == OCS_PGPE_DIRTY_TYPE_MASK) // not dirty, type 1. PGPE shouldn't be setting this
        g_wof->ocs_not_dirty_type1_count++;
    else if(g_wof->ocs_dirty == OCS_PGPE_DIRTY_MASK) // dirty type 0 (hold)
        g_wof->ocs_dirty_type0_count++;
    else if(g_wof->ocs_dirty == (OCS_PGPE_DIRTY_MASK | OCS_PGPE_DIRTY_TYPE_MASK)) // dirty type 1 (act)
        g_wof->ocs_dirty_type1_count++;
    else
        INTR_TRAC_ERR("???????? Invalid ocs_dirty[%d]", g_wof->ocs_dirty);

    // if WOF is disabled determine the Pstate clip based on dirty bits for OC protection
    if( (g_wof->wof_init_state < PGPE_WOF_ENABLED_NO_PREV_DATA) || g_wof->wof_disabled )
    {
        prevent_oc_wof_off();
    }

    // Update V/I from PGPE
    uint16_t l_voltage = 0;
    uint16_t l_current = 0;
    uint32_t l_freq = 0;

    pgpe_wof_values_t l_PgpeWofValues;
    l_PgpeWofValues.dw0.value = in64(g_amec_sys.static_wof_data.pgpe_values_sram_addr);
    l_PgpeWofValues.dw1.value = in64(g_amec_sys.static_wof_data.pgpe_values_sram_addr + 0x08);
    l_PgpeWofValues.dw2.value = in64(g_amec_sys.static_wof_data.pgpe_values_sram_addr + 0x10);
    l_PgpeWofValues.dw3.value = in64(g_amec_sys.static_wof_data.pgpe_values_sram_addr + 0x18);

    // save Vdd voltage to sensor
    l_voltage = (uint16_t)l_PgpeWofValues.dw2.fields.vdd_avg_mv;
    if (l_voltage != 0)
    {
        // Voltage value stored in the sensor should be in 100uV (mV scale -1)
        l_voltage *= 10;
        sensor_update(AMECSENSOR_PTR(VOLTVDD), l_voltage);
    }

    // save Vdn voltage to sensor
    l_voltage = (uint16_t)l_PgpeWofValues.dw2.fields.vdn_avg_mv;
    if (l_voltage != 0)
    {
        // Voltage value stored in the sensor should be in 100uV (mV scale -1)
        l_voltage *= 10;
        sensor_update(AMECSENSOR_PTR(VOLTVDN), l_voltage);
    }

    // save Vcs voltage to sensor
    l_voltage = (uint16_t)l_PgpeWofValues.dw2.fields.vcs_avg_mv;
    if (l_voltage != 0)
    {
        // Voltage value stored in the sensor should be in 100uV (mV scale -1)
        l_voltage *= 10;
        sensor_update(AMECSENSOR_PTR(VOLTVCS), l_voltage);
    }

    // Save Vdd current to sensor
    l_current = (uint16_t)l_PgpeWofValues.dw1.fields.idd_avg_10ma;
    if (l_current != 0)
    {
        // Current value stored in the sensor should be in 10mA (A scale -2)
        // Reading from SRAM is already in 10mA
        sensor_update(AMECSENSOR_PTR(CURVDD), l_current);
        l_update_pwr_sensors |= AVSBUS_PGPE_VDD;
    }

    // Save Vdn current to sensor
    l_current = (uint16_t)l_PgpeWofValues.dw1.fields.idn_avg_10ma;
    if (l_current != 0)
    {
        // Current value stored in the sensor should be in 10mA (A scale -2)
        // Reading from SRAM is already in 10mA
        sensor_update(AMECSENSOR_PTR(CURVDN), l_current);
        l_update_pwr_sensors |= AVSBUS_PGPE_VDN;
    }

    // Save Vcs current to sensor
    l_current = (uint16_t)l_PgpeWofValues.dw1.fields.ics_avg_10ma;
    if (l_current != 0)
    {
        // Current value stored in the sensor should be in 10mA (A scale -2)
        // Reading from SRAM is already in 10mA
        sensor_update(AMECSENSOR_PTR(CURVCS), l_current);
        l_update_pwr_sensors |= AVSBUS_PGPE_VCS;
    }

    // Update the chip voltage and power sensors
    if(l_update_pwr_sensors)
    {
        update_avsbus_power_sensors(l_update_pwr_sensors);
        if( (L_traced_no_readings) && (!L_traced_received_readings) )
        {
           INTR_TRAC_IMP("read_pgpe_produced_wof_values: Received current readings from PGPE dw1[0x%08X%08X]",
                          WORD_HIGH(l_PgpeWofValues.dw1.value), WORD_LOW(l_PgpeWofValues.dw1.value));
           INTR_TRAC_IMP("read_pgpe_produced_wof_values: Received voltage readings from PGPE dw2[0x%08X%08X]",
                          WORD_HIGH(l_PgpeWofValues.dw2.value), WORD_LOW(l_PgpeWofValues.dw2.value));
           L_traced_received_readings = TRUE;
        }
    }
    else if(!L_traced_no_readings)
    {
        INTR_TRAC_IMP("read_pgpe_produced_wof_values: No current readings from PGPE dw1[0x%08X%08X]",
                       WORD_HIGH(l_PgpeWofValues.dw1.value), WORD_LOW(l_PgpeWofValues.dw1.value));
        INTR_TRAC_IMP("read_pgpe_produced_wof_values: No voltage readings from PGPE dw2[0x%08X%08X]",
                       WORD_HIGH(l_PgpeWofValues.dw2.value), WORD_LOW(l_PgpeWofValues.dw2.value));
        L_traced_no_readings = TRUE;
    }

    // populate values used by WOF alg

    // Average Frequency
    uint32_t l_steps = 0;
    l_freq = proc_pstate2freq((Pstate_t)l_PgpeWofValues.dw0.fields.average_frequency_pstate, &l_steps);
    // value returned in kHz, save in MHz
    g_wof->avg_freq_mhz = (l_freq / 1000);
    // Need to do some linear interpolation with #V data to track Ceff ratio with current
    // average frequency, find the two #V points that the average frequency falls between
    get_poundV_points(g_wof->avg_freq_mhz,
                      &g_wof->vpd_index1,
                      &g_wof->vpd_index2);

    // save pstates and freq in sensors
    sensor_update(AMECSENSOR_PTR(FREQ_PSTATE), (uint16_t)l_PgpeWofValues.dw0.fields.average_frequency_pstate);
    sensor_update(AMECSENSOR_PTR(PSTATE), (uint16_t)l_PgpeWofValues.dw0.fields.average_pstate);
    sensor_update(AMECSENSOR_PTR(FREQA), (uint16_t)g_wof->avg_freq_mhz);

    g_wof->v_ratio_vcs = l_PgpeWofValues.dw3.fields.vratio_vcs_roundup_avg;

    g_wof->v_ratio_vdd = l_PgpeWofValues.dw3.fields.vratio_vdd_roundup_avg;
    sensor_update(AMECSENSOR_PTR(VRATIO_VDD), (uint16_t)g_wof->v_ratio_vdd);

    // clip Pstate is the last value read from VRT and used for debug only
    g_wof->f_clip_ps = l_PgpeWofValues.dw0.fields.wof_clip_pstate;

    // save over/under volting percentages into sensors
    l_uv_avg_0p1pct = (uint16_t)l_PgpeWofValues.dw3.fields.uv_avg_0p1pct;
    l_ov_avg_0p1pct = (uint16_t)l_PgpeWofValues.dw3.fields.ov_avg_0p1pct;
    sensor_update(AMECSENSOR_PTR(UV_AVG), l_uv_avg_0p1pct);
    sensor_update(AMECSENSOR_PTR(OV_AVG), l_ov_avg_0p1pct);
    // Save the Over volting difference to adjust ceff later
    g_wof->ov_uv_diff_0p01pct = 10*(l_ov_avg_0p1pct - l_uv_avg_0p1pct);

    // save the full PGPE WOF values for debug
    g_wof->pgpe_wof_values_dw0 = l_PgpeWofValues.dw0.value;
    g_wof->pgpe_wof_values_dw1 = l_PgpeWofValues.dw1.value;
    g_wof->pgpe_wof_values_dw2 = l_PgpeWofValues.dw2.value;
    g_wof->pgpe_wof_values_dw3 = l_PgpeWofValues.dw3.value;
}

/**
 * read_xgpe_values
 *
 * Description: Read XGPE Produced WOF and IDDQ activity values from shared SRAM
 *              and update g_wof parms needed for WOF calculations
 *              This is only called when WOF is running
 */
void read_xgpe_values( void )
{
    xgpe_wof_values_t l_XgpeWofValues;
    // maximum valid IO Power index
    uint16_t l_max_io_index = g_amec_sys.static_wof_data.wof_header.io_size - 1;
    int l_core = 0;
    uint16_t l_sram_addr_offset = 0;
    // sum of core cache off and core off samples
    uint16_t l_core_total_off_samples = 0;
    // l_core_total_off_samples converted to 0.1%
    uint16_t l_core_total_off_p1pct = 0;
    // used to trace only once getting an invalid IO power index from XGPE
    static bool L_trace_io_index_invalid = TRUE;

    // Read and process XGPE Produced WOF values
    l_XgpeWofValues.value = in64(g_amec_sys.static_wof_data.xgpe_values_sram_addr);

    // update IO Power Proxy sensor with last computed power
    sensor_update(AMECSENSOR_PTR(IO_PWR_PROXY), (uint16_t)l_XgpeWofValues.fields.compute_pwr_10mw);

    // Set IO Power index bits 1:3 of io_index
    g_wof->io_pwr_step_from_start = (uint16_t)((l_XgpeWofValues.fields.io_index & 0x70) >> 4);

    // make sure we didn't get something out of range
    if(g_wof->io_pwr_step_from_start > l_max_io_index)
    {
         if(L_trace_io_index_invalid)
         {
             L_trace_io_index_invalid = FALSE;
             TRAC_ERR("Invalid IO power index[%d] > max index[%d]",
                       g_wof->io_pwr_step_from_start,
                       l_max_io_index);
         }
         g_wof->io_pwr_step_from_start = l_max_io_index;
    }

    // save the full XGPE WOF value for debug
    g_wof->xgpe_wof_values_dw0 = l_XgpeWofValues.value;


    // Read and process XGPE Produced WOF IDDQ Activity values
    for(l_core = 0; l_core < MAX_NUM_CORES; l_core++)
    {
       l_sram_addr_offset = l_core * ACT_CNT_IDX_MAX;

       // single read to get the core data in one shot
       g_wof->xgpe_activity_values.act_val_core[l_core] = in32(g_amec_sys.static_wof_data.xgpe_iddq_activity_sram_addr + l_sram_addr_offset);

       // convert values to percentage and store into g_wof in 0.1% unit
       g_wof->p1pct_off[l_core] = (uint16_t)( (g_wof->xgpe_activity_values.act_val[l_core][ACT_CNT_IDX_CORECACHE_OFF] * 1000) >>
                                                 g_amec_sys.static_wof_data.iddq_activity_divide_bit_shift );

       g_wof->p1pct_vmin[l_core] = (uint16_t)( (g_wof->xgpe_activity_values.act_val[l_core][ACT_CNT_IDX_CORE_VMIN] * 1000) >>
                                                 g_amec_sys.static_wof_data.iddq_activity_divide_bit_shift );

       g_wof->p1pct_mma_off[l_core] = (uint16_t)( (g_wof->xgpe_activity_values.act_val[l_core][ACT_CNT_IDX_MMA_OFF] * 1000) >>
                                                g_amec_sys.static_wof_data.iddq_activity_divide_bit_shift );

       // calculate percentage on by subtracting off the total core off
       l_core_total_off_samples = g_wof->xgpe_activity_values.act_val[l_core][ACT_CNT_IDX_CORECLK_OFF] + g_wof->xgpe_activity_values.act_val[l_core][ACT_CNT_IDX_CORECACHE_OFF];
       l_core_total_off_p1pct = (1000 * l_core_total_off_samples) >> g_amec_sys.static_wof_data.iddq_activity_divide_bit_shift;
       // prevent overflow
       if(l_core_total_off_p1pct <= 1000)
       {
           g_wof->p1pct_on[l_core] = (uint16_t)(1000 - l_core_total_off_p1pct);
       }
       else
       {
           g_wof->p1pct_on[l_core] = 0;
       }

    } // for each core
}

/**
 * calculate_core_leakage
 *
 * Description: Calculate core-level leakage
 *
 * Return: the calculated core leakage
 */
void calculate_core_leakage( void )
{
    // Initialize leakage to racetrack component (Vcs racetrack component is 0)
    // Calculations are done in unit of 0.000001ua : 1000 to account for 0.1% IDDQ activity * 1000 for l_t_p001_scale_
    // Final divide to convert to 100ua unit will happen at very end
    uint64_t  l_iddq_p000001ua = g_wof->racetrack_only_vdd_chip_ua * 1000000;
    uint64_t  l_icsq_p000001ua = g_wof->racetrack_only_vcs_chip_ua * 1000000;

    uint64_t  l_t_p001_scale_c = 0;  // unit 0.001
    uint64_t  l_t_p001_scale_nc = 0; // unit 0.001
    uint16_t  l_temperature = 0;
    uint64_t  l_temp64 = 0;
    uint32_t  l_eco_cores = 0;
    uint32_t  l_core_eco_bit_mask = 0;
    uint32_t  l_temp32 = 0;
    int l_oct_idx = 0;  // octant index
    int l_core_idx = 0; // core index
    bool l_non_core_scaling_line = FALSE;  // default to core scaling line
    static bool L_trace_eco = TRUE;

    // read in ECO cores
    l_eco_cores = in32(OCB_OCCFLG6);
    if(L_trace_eco)
    {
        TRAC_IMP("calculate_core_leakage: ECO cores[0x%08X]", l_eco_cores);
        L_trace_eco = FALSE;  // only trace once
    }

    // Start leakage summation loop for each octant (EQ01, EQ23, EQ45, EQ67)
    for (l_oct_idx=0; l_oct_idx<MAXIMUM_EQ_SETS; l_oct_idx++)
    {
        g_wof->scaled_good_eqs_on_on_vdd_chip_ua = scale_and_interpolate(G_oppb.iddq.iddq_eqs_good_cores_on_good_caches_on_5ma[l_oct_idx],
                                                                         G_oppb.iddq.avgtemp_all_cores_on_good_caches_on_p5c,
                                                                         g_wof->Vdd_chip_index,
                                                                         g_wof->T_racetrack,
                                                                         g_wof->Vdd_chip_p1mv,
                                                                         l_non_core_scaling_line);
        // avoid divide by 0
        if(G_oppb.iddq.good_normal_cores_per_EQs[l_oct_idx] == 0)
        {
            g_wof->single_core_on_vdd_chip_eqs_ua = 0;
        }
        else
        {
            // avoid negative
            if(g_wof->scaled_good_eqs_on_on_vdd_chip_ua < g_wof->scaled_all_off_on_vdd_chip_ua_c)
            {
                g_wof->single_core_on_vdd_chip_eqs_ua = g_wof->single_core_off_vdd_chip_ua_c;
            }
            else
            {
                g_wof->single_core_on_vdd_chip_eqs_ua = ( (g_wof->scaled_good_eqs_on_on_vdd_chip_ua - g_wof->scaled_all_off_on_vdd_chip_ua_c)
                                                     / G_oppb.iddq.good_normal_cores_per_EQs[l_oct_idx] ) + g_wof->single_core_off_vdd_chip_ua_c;
            }
        }

        g_wof->scaled_good_eqs_on_on_vdd_vmin_ua = scale_and_interpolate(G_oppb.iddq.iddq_eqs_good_cores_on_good_caches_on_5ma[l_oct_idx],
                                                                         G_oppb.iddq.avgtemp_all_cores_on_good_caches_on_p5c,
                                                                         g_amec_sys.static_wof_data.Vdd_vret_index,
                                                                         g_wof->T_racetrack,
                                                                         g_amec_sys.static_wof_data.Vdd_vret_p1mv,
                                                                         l_non_core_scaling_line);

        // avoid divide by 0
        if(G_oppb.iddq.good_normal_cores_per_EQs[l_oct_idx] == 0)
        {
             l_temp32 = 0;
        }
        else
        {
            // avoid negative
            if(g_wof->scaled_good_eqs_on_on_vdd_vmin_ua < g_wof->scaled_all_off_on_vdd_vmin_ua_c)
            {
                l_temp32 = g_wof->single_core_off_vdd_vmin_ua_c;
            }
            else
            {
                l_temp32 = ( (g_wof->scaled_good_eqs_on_on_vdd_vmin_ua - g_wof->scaled_all_off_on_vdd_vmin_ua_c)
                                                 / G_oppb.iddq.good_normal_cores_per_EQs[l_oct_idx] ) + g_wof->single_core_off_vdd_vmin_ua_c;
            }
        }

        // need to take away the MMA portion of the single core on at vdd vmin if the Iddq includes the MMA, Retention mode guarantees MMA OFF
        if (G_oppb.iddq.mma_not_active == 0)  // MMA on
        {
            l_temp32 *= (100 - G_oppb.iddq.mma_off_leakage_pct);
            // divide by 100 to account for leakage percentage to keep unit of ua
            l_temp32 /= 100;
        }
        g_wof->single_core_on_vdd_vmin_eqs_ua = l_temp32;

        g_wof->scaled_good_eqs_on_on_vcs_chip_ua = scale_and_interpolate(G_oppb.iddq.icsq_eqs_good_cores_on_good_caches_on_5ma[l_oct_idx],
                                                                         G_oppb.iddq.avgtemp_all_cores_on_good_caches_on_p5c,
                                                                         g_wof->Vcs_chip_index,
                                                                         g_wof->T_racetrack,
                                                                         g_wof->Vcs_chip_p1mv,
                                                                         l_non_core_scaling_line);

        // avoid divide by 0
        if(G_oppb.iddq.good_normal_cores_per_EQs[l_oct_idx] == 0)
        {
            g_wof->single_core_on_vcs_chip_eqs_ua = 0;
        }
        else
        {
            // avoid negative
            if(g_wof->scaled_good_eqs_on_on_vcs_chip_ua < g_wof->scaled_all_off_on_vcs_chip_ua_c)
            {
                g_wof->single_core_on_vcs_chip_eqs_ua = g_wof->single_core_off_vcs_chip_ua_c;
            }
            else
            {
                g_wof->single_core_on_vcs_chip_eqs_ua = ( (g_wof->scaled_good_eqs_on_on_vcs_chip_ua - g_wof->scaled_all_off_on_vcs_chip_ua_c)
                                                 / G_oppb.iddq.good_normal_cores_per_EQs[l_oct_idx] ) + g_wof->single_core_off_vcs_chip_ua_c;
            }
        }

        // Begin Loop over ALL cores per octant (l_oct_idx)
        for (l_core_idx = (l_oct_idx * 8); l_core_idx < ((l_oct_idx*8) + 8); l_core_idx++)
        {
            // Get the core temperature from TEMPPROCTHRMC sensor
            l_temperature = AMECSENSOR_ARRAY_PTR(TEMPPROCTHRMC0, l_core_idx)->sample;

            // If core temperature is 0 use quad i.e. Racetrack
            if(l_temperature == 0)
            {
                 l_temperature = AMECSENSOR_ARRAY_PTR(TEMPQ0, l_core_idx/4)->sample;

                 if(l_temperature == 0)
                 {
                     // this core's racetrack DTS is 0 so use average Racetrack which is guaranteed to be non-0
                     l_temperature = g_wof->T_racetrack;
                 }
            }

            // Save core temperature used
            g_wof->tempprocthrmc[l_core_idx] = l_temperature;

            // core scaling line scale() returns unit of 0.001
            l_t_p001_scale_c = scale(g_wof->tempprocthrmc[l_core_idx], g_wof->T_racetrack, FALSE);
            // non-core scaling line scale() returns unit of 0.001
            l_t_p001_scale_nc = scale(g_wof->tempprocthrmc[l_core_idx], g_wof->T_racetrack, TRUE);

            // each core adds leakage of being ON, OFF, and/or @VddMIN  (no MIN for Vcs)
            l_iddq_p000001ua += g_wof->single_core_on_vdd_chip_eqs_ua * l_t_p001_scale_c * g_wof->p1pct_on[l_core_idx];

            l_iddq_p000001ua += g_wof->single_core_off_vdd_chip_ua_nc * g_wof->p1pct_off[l_core_idx] * l_t_p001_scale_nc;

            if(G_oppb.iddq.mma_not_active == 0)  // MMA on
            {
                l_temp64 = g_wof->single_core_on_vdd_chip_eqs_ua *
                           G_oppb.iddq.mma_off_leakage_pct *
                           g_wof->p1pct_mma_off[l_core_idx] *
                           g_wof->p1pct_on[l_core_idx] *
                           l_t_p001_scale_c;
                // to get unit of 0.000001ua divide by 100,000: 100 to account for iddq.mma_off_leakage_pct * 1000 for p1pct_on
                // note the p1pct_mma_off is accounted for in the final divide at the very end
                l_temp64 = (uint32_t)(l_temp64 / 100000);

                // prevent going negative.  This should never be negative
                if(l_temp64 <= l_iddq_p000001ua)
                    l_iddq_p000001ua -= l_temp64;
                else
                {
                    INTR_TRAC_ERR("calculate_core_leakage: core[%d] MMA[%d]>l_iddq_p000001ua[%d]",
                                   l_core_idx, l_temp64, l_iddq_p000001ua);
                    set_clear_wof_disabled(SET,
                                           WOF_RC_NEGATIVE_MMA_LEAKAGE,
                                           ERC_WOF_NEGATIVE_MMA_LEAKAGE);
                }
            }

            l_iddq_p000001ua += g_wof->single_core_on_vdd_vmin_eqs_ua *
                                g_wof->p1pct_vmin[l_core_idx] *
                                l_t_p001_scale_c;

            l_icsq_p000001ua += g_wof->single_core_on_vcs_chip_eqs_ua *
                               (g_wof->p1pct_on[l_core_idx] + g_wof->p1pct_vmin[l_core_idx]) *
                                l_t_p001_scale_c;

            l_icsq_p000001ua += g_wof->single_core_off_vcs_chip_ua_nc *
                                g_wof->p1pct_off[l_core_idx] *
                                l_t_p001_scale_nc;

            // each cache adds leakage of being ON, and/or OFF -- including ECO cores that are always ON
            l_core_eco_bit_mask = (0x80000000 >> l_core_idx);

            if((l_eco_cores & l_core_eco_bit_mask) == 0)
            {
               l_iddq_p000001ua += g_wof->single_cache_on_vdd_chip_ua_nc *
                                  (g_wof->p1pct_on[l_core_idx] + g_wof->p1pct_vmin[l_core_idx]) *
                                   l_t_p001_scale_nc;

               l_iddq_p000001ua += g_wof->single_cache_off_vdd_chip_ua_nc *
                                   g_wof->p1pct_off[l_core_idx] *
                                   l_t_p001_scale_nc;
            }
            else // Core is in ECO mode
            {
               // add in the ECO contributor (always on)
               l_iddq_p000001ua += (g_wof->single_cache_on_vdd_chip_ua_nc * 1000 * l_t_p001_scale_nc);
            }

            l_icsq_p000001ua += g_wof->single_cache_on_vcs_chip_ua_nc *
                               (g_wof->p1pct_on[l_core_idx] + g_wof->p1pct_vmin[l_core_idx]) *
                                l_t_p001_scale_nc;

            l_icsq_p000001ua += g_wof->single_cache_off_vcs_chip_ua_nc *
                                g_wof->p1pct_off[l_core_idx] *
                                l_t_p001_scale_nc;
        }  // for each core
    } // for each octant

    // Finally, save the calculated leakage to amec in 100ua
    // divide by 100,000,000 = 1000 for 0.1% IDDQ activity * 1000 for l_t_p001_scale_ (to get unit uA) * 100 convert uA to 100uA
    g_wof->iddq_100ua = (uint32_t) (l_iddq_p000001ua / 100000000);
    g_wof->icsq_100ua = (uint32_t) (l_icsq_p000001ua / 100000000);
}

/**
 * calculate_exp_1p3
 *
 * Description: Generic function to perform x^1.3
 *
 * Param[in]: i_x - Value to raise to 1.3
 *                  NOTE the best fit equation is assuming value passed in is in 0.1mv unit
 *                        and was calculated for a range of 0.5V - 1.0V (error -0.020% - 0.010%)
 *
 * Return: i_x^1.3
 */
uint32_t calculate_exp_1p3(uint32_t i_x)
{
    uint64_t temp64 = 0;

    // Compute i_x^1.3 using best-fit equation
    // y = 18.839*x - 31142
    //   = (18839*x - 31142000) / 1000
    //   = (19291*x - 31889408) / 1024
    temp64 = 19291 * i_x;
    if(temp64 >= 31889408)
        return (uint32_t)( (temp64 - 31889408) >> 10 );
    else
        return 0;
}

/**
 * calculate_wov_fudge_factor
 *
 * Description: Function to calculate WOF ceff fudge factor for over/under volting credit
 *
 * Param[in]: i_uv_0p01pct - this is the value of (1 - UVcreditKnob*UVpct) in 0.01% unit
 *                           fudge factor = i_uv_0p01pct^1.3
 *
 * NOTE the best fit equation is assuming value passed in is in 0.01% unit
 *      and was calculated for a range of 0.909 - 0.997 (i.e. UV% range with knob 9.1% - 0.3%)
 *
 * Return: ceff fudge factor in 0.001 unit
 */
uint32_t calculate_wov_fudge_factor(uint32_t i_uv_0p01pct)
{
    uint64_t l_fudge_factor_0p001 = 0;
    uint64_t l_uvpct_to1p3 = 0;

    // Compute i_uv_0p01pct^1.3 using best-fit equation
    // y = 20.306*x - 44615
    //   = (20306*x - 44615000) / 1000
    //   = (20793*x - 45685760) / 1024
    l_uvpct_to1p3 = 20793 * i_uv_0p01pct;
    if(l_uvpct_to1p3 >= 45685760)
    {
        l_uvpct_to1p3 = (l_uvpct_to1p3 - 45685760) >> 10;
        //  Convert fudge factor to 0.001 unit
        //   (i_uv_0p01pct/10000)^1.3 * 1000
        // = (i_uv_0p01pct^1.3/10000^1.3) * 1000
        // = (i_uv_0p01pct^1.3 * 1000)/10000^1.3
        // = (i_uv_0p01pct^1.3 * 1000)/158489
        l_fudge_factor_0p001 = (uint32_t)((l_uvpct_to1p3 * 1000)/158489);
    }
    return l_fudge_factor_0p001;
}

/**
 * calculate_ceff_ratio_vcs
 *
 * Description: Function to calculate the effective capacitance ratio
 *              for the cache
 * Ceff_ratio_vcs derivation to get less divides
 * Ceff_vcs_avg_tdp= (ics_ac_avg_tdp_ma * vratio_vcs_avg) / (vcs_avg_tdp_mv^1.3 * Favg_pstate)
 * Ceff_vcs_avg_present= ics_ac_avg_ma / (vcs_avg_mv^1.3 * Favg_pstate)
 * Ceff_ratio_vcs_avg= Ceff_vcs_avg_present / Ceff_vcs_avg_tdp
 * = ics_ac_avg_ma / (vcs_avg_mv^1.3 * Favg_pstate)/(ics_ac_avg_tdp_ma * vratio_vcs_avg))/ (vcs_avg_tdp_mv^1.3 * Favg_pstate)
 * = ics_ac_avg_ma / (vcs_avg_mv^1.3)/(ics_ac_avg_tdp_ma * vratio_vcs_avg) / (vcs_avg_tdp_mv^1.3)
 * = ics_ac_avg_ma / (vcs_avg_mv^1.3)/(ics_ac_avg_tdp_ma * vratio_vcs_avg) / (vcs_tdp_mv^1.3)
 * = roundup(ics_ac_avg_ma / (ics_ac_tdp_ma * vratio_vcs_avg)) * roundup((vcs_tdp_mv^1.3)/(vcs_avg_mv^1.3))
 * IMPLEMENT >>> roundup((ics_ac_avg_ma * vcs_tdp_mv^1.3) / (ics_ac_avg_tdp_ma * vratio_vcs_avg * vcs_avg_mv^1.3))
 * CODE NAMES >>> (g_wof->iac_vcs_100ua * g_wof->vcs_avg_tdp_100uv^1.3) / (g_wof->iac_tdp_vcs_100ua * g_wof->v_ratio_vcs * g_wof->Vcs_chip_p1mv^1.3)
 */


void calculate_ceff_ratio_vcs( void )
{
    uint32_t l_ceff_ratio = 0;
    uint64_t l_temp64 = 0;

    do
    {
        // track Ceff ratio with currrent average frequency from PGPE
        // The two #V points (g_wof->vpd_index1 and g_wof->vpd_index2) that
        // the average frequency falls between were updated in read_pgpe_produced_wof_values()

        // Calculate Vcs AC Current average TDP by interpolating #V current@g_wof->avg_freq_mhz
        // *100 to convert 10ma to 100ua
        g_wof->iac_tdp_vcs_100ua = 100 * interpolate_linear( g_wof->avg_freq_mhz,
                                                      G_oppb.operating_points[g_wof->vpd_index1].frequency_mhz,
                                                      G_oppb.operating_points[g_wof->vpd_index2].frequency_mhz,
                                                      G_oppb.operating_points[g_wof->vpd_index1].ics_tdp_ac_10ma,
                                                      G_oppb.operating_points[g_wof->vpd_index2].ics_tdp_ac_10ma,
                                                      FALSE); // round down

        // Calculate Vcs voltage average TDP by interpolating #V voltage@g_wof->avg_freq_mhz
        // *10 to convert mV to 100uV
        g_wof->vcs_avg_tdp_100uv = 10 * interpolate_linear( g_wof->avg_freq_mhz,
                                                            G_oppb.operating_points[g_wof->vpd_index1].frequency_mhz,
                                                            G_oppb.operating_points[g_wof->vpd_index2].frequency_mhz,
                                                            G_oppb.operating_points[g_wof->vpd_index1].vcs_mv,
                                                            G_oppb.operating_points[g_wof->vpd_index2].vcs_mv,
                                                            FALSE); // round down);

        if(G_allow_trace_flags & ALLOW_CEFF_RATIO_VCS_TRACE)
        {
            INTR_TRAC_INFO("ceff_ratio_vcs: F[%d] between #V OP index[%d] freq[%d] and index[%d] freq[%d]",
                            g_wof->avg_freq_mhz,
                            g_wof->vpd_index1, G_oppb.operating_points[g_wof->vpd_index1].frequency_mhz,
                            g_wof->vpd_index2, G_oppb.operating_points[g_wof->vpd_index2].frequency_mhz);
            INTR_TRAC_INFO("ceff_ratio_vcs: interpolated iac_tdp_vcs_100ua[%d] point1[%d] point2[%d]",
                            g_wof->iac_tdp_vcs_100ua,
                            G_oppb.operating_points[g_wof->vpd_index1].ics_tdp_ac_10ma * 100,
                            G_oppb.operating_points[g_wof->vpd_index2].ics_tdp_ac_10ma * 100);
            INTR_TRAC_INFO("ceff_ratio_vcs: interpolated vcs_avg_tdp_100uv[%d] point1[%d] point2[%d]",
                            g_wof->vcs_avg_tdp_100uv,
                            G_oppb.operating_points[g_wof->vpd_index1].vcs_mv * 10,
                            G_oppb.operating_points[g_wof->vpd_index2].vcs_mv * 10);
        }

        // Calculate ceff ratio numerator = (g_wof->iac_vcs_100ua * g_wof->vcs_avg_tdp_100uv^1.3)
        // calculate_exp_1p3() requires input to be in 0.1mv (100uv) unit
        g_wof->tdpvcsp1mv_exp1p3 = calculate_exp_1p3(g_wof->vcs_avg_tdp_100uv);
        l_temp64 = (uint64_t)((uint64_t)g_wof->iac_vcs_100ua * (uint64_t)g_wof->tdpvcsp1mv_exp1p3);

        // *10000 to get unit 0.01% (100->1.0% x 100->0.01%)
        g_wof->ceff_ratio_vcs_numerator = (uint64_t)(l_temp64 * 10000);

       // Scale TDP AC current to the number of active cores represented by v_ratio_vcs
       // 2nd parm TRUE to indicate use v_ratio_vcs
        g_wof->c_ratio_iac_tdp_vcsp1ma = multiply_v_ratio(g_wof->iac_tdp_vcs_100ua, TRUE);
        // Calculate g_wof->vcsp1mv_exp1p3 = g_wof->Vcs_chip_p1mv^1.3
        // calculate_exp_1p3() requires input to be in 0.1mv unit
        g_wof->vcsp1mv_exp1p3 = calculate_exp_1p3(g_wof->Vcs_chip_p1mv);

        // Prevent divide by zero
        if((g_wof->vcsp1mv_exp1p3 == 0) || (g_wof->c_ratio_iac_tdp_vcsp1ma == 0))
        {
            INTR_TRAC_ERR("WOF Ceff VCS 0 divide (Vcs_chip_p1mv^1.3[%d] * c_ratio_iac_tdp_vcsp1ma[%d])",
                          g_wof->vcsp1mv_exp1p3,
                          g_wof->c_ratio_iac_tdp_vcsp1ma);
            print_oppb();
            // Return 0
            g_wof->ceff_ratio_vcs = 0;

            set_clear_wof_disabled(SET,
                                   WOF_RC_DIVIDE_BY_ZERO_VDD,
                                   ERC_WOF_DIVIDE_BY_ZERO_VDD);
        }
        else
        {
            // Finally put pieces together and calculate Vcs ceff ratio
            g_wof->ceff_ratio_vcs_denominator = (uint64_t)((uint64_t)g_wof->c_ratio_iac_tdp_vcsp1ma * (uint64_t)g_wof->vcsp1mv_exp1p3);
            l_temp64 = (uint64_t)(g_wof->ceff_ratio_vcs_numerator / g_wof->ceff_ratio_vcs_denominator);
            l_ceff_ratio = (uint32_t)l_temp64;

            // roundup to 1.0%
            if(l_ceff_ratio % 100)
                l_ceff_ratio += (100 - (l_ceff_ratio % 100));

            g_wof->ceff_ratio_vcs = l_ceff_ratio;

            // Save Vcs ceff ratio to a sensor
            sensor_update(AMECSENSOR_PTR(CEFFVCSRATIO), (uint16_t)g_wof->ceff_ratio_vcs);
        }

    }while( 0 );
}

/**
 *  calculate_ceff_ratio_vdd
 *
 *  Description: Function to calculate the core effective capacitance ratio
 * Ceff_ratio_vdd derivation to get less divides
 * Ceff_vdd_avg_tdp= (((idd_ac_avg_tdp_ma - idd_rt_ac_avg_tdp_ma)*vratio_vdd_avg)+idd_rt_ac_avg_tdp_ma) / ((vdd_avg_tdp_mv)^1.3 * Favg_pstate)
 * Ceff_vdd_avg_present= idd_ac_avg_ma / (vdd_avg_mv)^1.3 * Favg_pstate)
 * Ceff_ratio_vdd_avg= Ceff_vdd_avg_present / Ceff_vdd_avg_tdp
 * = idd_ac_avg_ma / (vdd_avg_mv^1.3 * Favg_pstate)/(((idd_ac_avg_tdp_ma - idd_rt_ac_avg_tdp_ma)* vratio_vdd_avg) + idd_rt_ac_avg_tdp_ma)/ ((vdd_avg_tdp_mv)^1.3 * Favg_pstate)
 * = idd_ac_avg_ma / (vdd_avg_mv^1.3)/(((idd_ac_avg_tdp_ma - idd_rt_ac_avg_tdp_ma)* vratio_vdd_avg) + idd_rt_ac_avg_tdp_ma) / ((vdd_avg_tdp_mv)^1.3)
 * = idd_ac_avg_ma / (vdd_avg_mv^1.3)/(((idd_ac_avg_tdp_ma - idd_rt_ac_avg_tdp_ma)* vratio_vdd_avg) + idd_rt_ac_avg_tdp_ma) / ((vdd_tdp_mv)^1.3)
 * = roundup(idd_ac_avg_ma / ((idd_ac_tdp_ma - idd_rt_ac_avg_tdp_ma) * vratio_vdd_avg) + idd_rt_ac_avg_tdp_ma) * roundup(((vdd_tdp_mv)^1.3)/(vdd_avg_mv^1.3))
 * IMPLEMENT >>> roundup((idd_ac_avg_ma* vdd_tdp_mv^1.3) / ((((idd_ac_avg_tdp_ma - idd_rt_ac_avg_tdp_ma) * vratio_vdd_avg) + idd_rt_ac_avg_tdp_ma) * vdd_avg_mv^1.3))
 * CODE NAMES >>>> (g_wof->iac_vdd_100ua * g_wof->vdd_avg_tdp_100uv^1.3) / ((((g_wof->iac_tdp_vdd_100ua - g_wof->tdp_idd_rt_ac_100ua) * g_wof->v_ratio_vdd) + g_wof->tdp_idd_rt_ac_100ua) * g_wof->Vdd_chip_p1mv^1.3)
*/
void calculate_ceff_ratio_vdd( void )
{
    uint32_t l_raw_ceff_ratio = 0;
    uint32_t l_ceff_ratio_oc_adjusted = 0;
    uint32_t l_avg_pstate = 0;
    uint32_t l_temp32 = 0;
    uint32_t l_scratch_reg3 = 0;
    uint32_t l_throttle_index_percent = 0;
    uint32_t l_temp_ceff = 0;
    uint32_t l_fudge = 0;
    int32_t  l_signed_temp32 = 0;
    uint64_t l_temp64 = 0;
    static bool L_trace_error = TRUE;
    static uint8_t L_trace_count = 2;

    do
    {
        // track Ceff ratio with currrent average frequency from PGPE
        // The two #V points (g_wof->vpd_index1 and g_wof->vpd_index2) that
        // the average frequency falls between were updated in read_pgpe_produced_wof_values()

        // Calculate Vdd AC Current average TDP by interpolating #V current@g_wof->avg_freq_mhz
        // *100 to convert 10ma to 100ua
        g_wof->iac_tdp_vdd_100ua = 100 * interpolate_linear( g_wof->avg_freq_mhz,
                                                           G_oppb.operating_points[g_wof->vpd_index1].frequency_mhz,
                                                           G_oppb.operating_points[g_wof->vpd_index2].frequency_mhz,
                                                           G_oppb.operating_points[g_wof->vpd_index1].idd_tdp_ac_10ma,
                                                           G_oppb.operating_points[g_wof->vpd_index2].idd_tdp_ac_10ma,
                                                           FALSE); // round down);

        // Calculate Vdd voltage average TDP by interpolating #V voltage@g_wof->avg_freq_mhz
        // *10 to convert mV to 100uV
        g_wof->vdd_avg_tdp_100uv = 10 * interpolate_linear( g_wof->avg_freq_mhz,
                                                            G_oppb.operating_points[g_wof->vpd_index1].frequency_mhz,
                                                            G_oppb.operating_points[g_wof->vpd_index2].frequency_mhz,
                                                            G_oppb.operating_points[g_wof->vpd_index1].vdd_mv,
                                                            G_oppb.operating_points[g_wof->vpd_index2].vdd_mv,
                                                            FALSE); // round down);

        // Calculate Racetrack AC Current average TDP by interpolating #V current@g_wof->avg_freq_mhz
        // *100 to convert 10ma to 100ua
        g_wof->tdp_idd_rt_ac_100ua = 100 * interpolate_linear( g_wof->avg_freq_mhz,
                                                          G_oppb.operating_points[g_wof->vpd_index1].frequency_mhz,
                                                          G_oppb.operating_points[g_wof->vpd_index2].frequency_mhz,
                                                          G_oppb.operating_points[g_wof->vpd_index1].rt_tdp_ac_10ma,
                                                          G_oppb.operating_points[g_wof->vpd_index2].rt_tdp_ac_10ma,
                                                          FALSE); // round down);


        if(G_allow_trace_flags & ALLOW_CEFF_RATIO_VDD_TRACE)
        {
            INTR_TRAC_INFO("ceff_ratio_vdd: F[%d] between #V OP index[%d] freq[%d] and index[%d] freq[%d]",
                            g_wof->avg_freq_mhz,
                            g_wof->vpd_index1, G_oppb.operating_points[g_wof->vpd_index1].frequency_mhz,
                            g_wof->vpd_index2, G_oppb.operating_points[g_wof->vpd_index2].frequency_mhz);
            INTR_TRAC_INFO("ceff_ratio_vdd: interpolated iac_tdp_vdd_100ua[%d] point1[%d] point2[%d]",
                            g_wof->iac_tdp_vdd_100ua,
                            G_oppb.operating_points[g_wof->vpd_index1].idd_tdp_ac_10ma * 100,
                            G_oppb.operating_points[g_wof->vpd_index2].idd_tdp_ac_10ma * 100);
            INTR_TRAC_INFO("ceff_ratio_vdd: interpolated vdd_avg_tdp_100uv[%d] point1[%d] point2[%d]",
                            g_wof->vdd_avg_tdp_100uv,
                            G_oppb.operating_points[g_wof->vpd_index1].vdd_mv * 10,
                            G_oppb.operating_points[g_wof->vpd_index2].vdd_mv * 10);
            INTR_TRAC_INFO("ceff_ratio_vdd: interpolated tdp_idd_rt_ac_100ua[%d] point1[%d] point2[%d]",
                            g_wof->tdp_idd_rt_ac_100ua,
                            G_oppb.operating_points[g_wof->vpd_index1].rt_tdp_ac_10ma * 100,
                            G_oppb.operating_points[g_wof->vpd_index2].rt_tdp_ac_10ma * 100);

        }

        // Calculate ceff ratio numerator = (g_wof->iac_vdd_100ua * g_wof->vdd_avg_tdp_100uv^1.3)
        // calculate_exp_1p3() requires input to be in 0.1mv (100uv) unit
        g_wof->tdpvddp1mv_exp1p3 = calculate_exp_1p3(g_wof->vdd_avg_tdp_100uv);
        l_temp64 = (uint64_t)((uint64_t)g_wof->iac_vdd_100ua * (uint64_t)g_wof->tdpvddp1mv_exp1p3);

        // *10000 to get unit 0.01% (100->1.0% x 100->0.01%)
        g_wof->ceff_ratio_vdd_numerator = (uint64_t)(l_temp64 * 10000);

        // remove racetrack component
        if(g_wof->iac_tdp_vdd_100ua > g_wof->tdp_idd_rt_ac_100ua)
            l_temp32 = g_wof->iac_tdp_vdd_100ua - g_wof->tdp_idd_rt_ac_100ua;
        else
        {
            // trace once and set ceff ratio to 0
            if(L_trace_error)
            {
               INTR_TRAC_ERR("WOF tdp Racetrack[%d 100ua] >= tdp core[%d 100ua])",
                             g_wof->tdp_idd_rt_ac_100ua,
                             g_wof->iac_tdp_vdd_100ua);
               L_trace_error = FALSE;
            }
            g_wof->ceff_ratio_vdd  = 0;
            break;
        }
       // scale core only (no RT) TDP AC current to the number of active cores represented by v_ratio_vdd
       // 2nd parm FALSE to indicate use v_ratio_vdd
       // and add back in racetrack AC component measured at MFT
        g_wof->c_ratio_iac_tdp_vddp1ma = multiply_v_ratio(l_temp32, FALSE) + g_wof->tdp_idd_rt_ac_100ua;

        // Calculate g_wof->vddp1mv_exp1p3 = g_wof->Vdd_chip_p1mv^1.3
        // calculate_exp_1p3() requires input to be in 0.1mv unit
        g_wof->vddp1mv_exp1p3 = calculate_exp_1p3(g_wof->Vdd_chip_p1mv);

        // Prevent divide by zero
        if((g_wof->vddp1mv_exp1p3 == 0) || (g_wof->c_ratio_iac_tdp_vddp1ma == 0))
        {
            INTR_TRAC_ERR("WOF Ceff VDD 0 divide (Vdd_chip_p1mv^1.3[%d] * c_ratio_iac_tdp_vddp1ma[%d])",
                          g_wof->vddp1mv_exp1p3,
                          g_wof->c_ratio_iac_tdp_vddp1ma);
            print_oppb();
            // Return 0
            g_wof->ceff_ratio_vdd = 0;

            set_clear_wof_disabled(SET,
                                   WOF_RC_DIVIDE_BY_ZERO_VDD,
                                   ERC_WOF_DIVIDE_BY_ZERO_VDD);
        }
        else
        {
            // Finally put pieces together to calculate raw ceff ratio
            g_wof->ceff_ratio_vdd_denominator = (uint64_t)((uint64_t)g_wof->c_ratio_iac_tdp_vddp1ma * (uint64_t)g_wof->vddp1mv_exp1p3);
            l_temp64 = (uint64_t)(g_wof->ceff_ratio_vdd_numerator / g_wof->ceff_ratio_vdd_denominator);
            l_raw_ceff_ratio = (uint32_t)l_temp64;

            // roundup to 1.0%
            if(l_raw_ceff_ratio % 100)
                l_raw_ceff_ratio += (100 - (l_raw_ceff_ratio % 100));

            // Save raw ceff ratio vdd to a sensor, this sensor is NOT to be used by the WOF alg
            sensor_update(AMECSENSOR_PTR(CEFFVDDRATIO), (uint16_t)l_raw_ceff_ratio);

            // Now check the raw ceff ratio to prevent Over current by clipping to max of 100%
            l_ceff_ratio_oc_adjusted = prevent_over_current(l_raw_ceff_ratio);

            // Check if raw ceff needs to be adjusted to account for throttling that was put in place
            pgpe_wof_values_t l_PgpeWofValues;
            l_PgpeWofValues.dw0.value = g_wof->pgpe_wof_values_dw0;
            l_avg_pstate = (uint32_t)l_PgpeWofValues.dw0.fields.average_pstate;

            if(l_avg_pstate > G_oppb.pstate_min)
            {
                // In throttle range, read adder values from OCC scratch reg 3
                l_scratch_reg3 = in32(OCB_OCCS3);
                // bits 0:15 is throttle index ceff percentage in 0.0001% unit if 0 use 1.5625%
                l_throttle_index_percent = (l_scratch_reg3 >> 16);
                if(l_throttle_index_percent == 0)
                    g_wof->throt_idx_percent = 1562500;
                else // x100 to convert to 0.000001% unit
                    g_wof->throt_idx_percent = l_throttle_index_percent * 100;

                // bits 16:31 is throttle index offset
                g_wof->throt_idx_offset = (l_scratch_reg3 & 0x0000FFFF);

                // total throttle ceff adjustment is (number of pstates in throttle range + offset) x percentage
                l_temp32 = (uint32_t)( (l_avg_pstate - G_oppb.pstate_min) + g_wof->throt_idx_offset );
                l_temp32 = l_temp32 * g_wof->throt_idx_percent;
                // /10000 to convert to final 0.01% unit
                g_wof->vdd_throt_ceff_add = (uint32_t)(l_temp32/10000);
            }
            else
            {
                // not in throttle range, no throttle adjustment
                g_wof->vdd_throt_ceff_add = 0;
            }

            sensor_update(AMECSENSOR_PTR(THROTTLE_ADDR), (uint16_t)g_wof->vdd_throt_ceff_add);
            g_wof->vdd_ceff_ratio_throt_adj = l_raw_ceff_ratio + g_wof->vdd_throt_ceff_add;

            // the final ceff is the higher of the 2 adjusted
            if(l_ceff_ratio_oc_adjusted > g_wof->vdd_ceff_ratio_throt_adj)
                l_temp_ceff = l_ceff_ratio_oc_adjusted;
            else
                l_temp_ceff = g_wof->vdd_ceff_ratio_throt_adj;

            // Now adjust ceff to account for over/under volting
            if(g_wof->wov_credit_knob)
            {
                // apply the credit knob to the over/under volt difference
                l_signed_temp32 = (int32_t)((int32_t)g_wof->wov_credit_knob * (int32_t)g_wof->ov_uv_diff_0p01pct);
                l_signed_temp32 = (int32_t)(l_signed_temp32/100); // knob is factor of 100
                if(l_signed_temp32 > -10000)
                {
                    l_temp32 = (uint32_t)(10000 + l_signed_temp32);
                    l_fudge = calculate_wov_fudge_factor(l_temp32);
                }
                // don't mutltiply by 0
                if(l_fudge)
                {
                    l_temp64 = (uint64_t)l_temp_ceff * (uint64_t)l_fudge;
                    // fudge factor is returned in 0.001 unit
                    l_temp_ceff = (uint32_t)(l_temp64/1000);
                }
                if( (G_allow_trace_flags & ALLOW_WOF_OV_TRACE) && (L_trace_count) )
                {
                    L_trace_count--;
                    INTR_TRAC_INFO("WOV Debug: credit knob[%d] * ov_uv_diff[%d] = %d",
                                   g_wof->wov_credit_knob, g_wof->ov_uv_diff_0p01pct, l_signed_temp32);

                    INTR_TRAC_INFO("WOV Debug: (%d/10000)^1.3 = fudge factor[%d]",
                                    l_temp32, l_fudge);

                    INTR_TRAC_INFO("WOV Debug: ceff * fudge = %d final WOV adjusted ceff[%d]", l_temp64, l_temp_ceff);
                }
                else if(!(G_allow_trace_flags & ALLOW_WOF_OV_TRACE))
                {
                    // reset trace count to allow ALLOW_WOF_OV_TRACE to be re-set
                    L_trace_count = 2;
                }
            }

            g_wof->ov_uv_fudge = l_fudge;
            g_wof->ceff_ratio_vdd = l_temp_ceff;

            //  if eco mode addr is set adjust ceff some more
            if(g_wof->eco_mode_ceff_add > 0)
            {
               // positive addr is added on top of any previous adjustments
               g_wof->ceff_ratio_vdd += g_wof->eco_mode_ceff_add;
            }
            else if(g_wof->eco_mode_ceff_add < 0)
            {
               // negative addr is only applied if there were no previous adjustments
               if(l_raw_ceff_ratio == g_wof->ceff_ratio_vdd)
               {
                  // prevent overflow
                  if(g_wof->ceff_ratio_vdd > -g_wof->eco_mode_ceff_add)
                     g_wof->ceff_ratio_vdd += g_wof->eco_mode_ceff_add;
                  else
                     g_wof->ceff_ratio_vdd = 0;
               }
            }
            // save the final adjusted Ceff ratio to a sensor this parameter is used by the rest of wof alg
            sensor_update(AMECSENSOR_PTR(CEFFVDDRATIOADJ), (uint16_t)g_wof->ceff_ratio_vdd);
        }

    }while( 0 );
}

/** multiply_v_ratio
 *
 *  Description: Helper function to multiply by V ratio
 *
 *  Param[in]: i_value - value to multiply v_ratio by
 *  Param[in]: i_vcs   - TRUE indicates multiply by v_ratio_vcs else multiply by v_ratio_vdd
 *
 *  Return: i_value * v_ratio
 */
uint32_t multiply_v_ratio( uint32_t i_value, bool i_vcs )
{
    // We get v_ratio from the PGPE ranging from 0x0000 to 0xffff
    // These hex values conceptually translate from 0.0 to 1.0
    // (v_ratio / 0xFFFF) * i_value
    // *16384 to avoid floating point math 16384 chosen so can /16384 with >>14
    // (v_ratio*16384/0xffff) * i_value / 16384
    uint64_t l_temp64 = 0;

    if(i_vcs)
    {
        l_temp64 = (uint64_t)g_wof->v_ratio_vcs * 16384;
    }
    else
    {
        l_temp64 = (uint64_t)g_wof->v_ratio_vdd * 16384;
    }
    l_temp64 = l_temp64 / 0xFFFF;
    l_temp64 = l_temp64 * i_value;
    l_temp64 = l_temp64 >> 14;
    return ((uint32_t)l_temp64);

}

/**
 *  calculate_AC_currents
 *
 *  Description: Calculate the AC component of the workload
 */
void calculate_AC_currents( void )
{
    // sensor readings are in 0.01A (10mA) unit *100 to get 100uA
    uint32_t l_idd_avg_100ua = g_wof->curvdd_sensor * 100;
    uint32_t l_ics_avg_100ua = g_wof->curvcs_sensor * 100;

    // avoid negative AC currents
    if(l_idd_avg_100ua > g_wof->iddq_100ua)
    {
       g_wof->iac_vdd_100ua = l_idd_avg_100ua - g_wof->iddq_100ua;
    }
    else
    {
       g_wof->iac_vdd_100ua = 0;
    }

    if(l_ics_avg_100ua > g_wof->icsq_100ua)
    {
       g_wof->iac_vcs_100ua = l_ics_avg_100ua - g_wof->icsq_100ua;
    }
    else
    {
       g_wof->iac_vcs_100ua = 0;
    }
}

/**
 * interpolate_linear
 *
 * Description: Helper function that takes in the necessary input for
 *              a linear interpolation and returns the result of the
 *              calculation
 *
 *              Y = m*(X-x1) + y1, where m = (y2-y1) / (x2-x1)
 *
 * Return: The result Y of the formula above
 */
int32_t interpolate_linear( int32_t i_X,
                            int32_t i_x1,
                            int32_t i_x2,
                            int32_t i_y1,
                            int32_t i_y2,
                            bool    i_roundup )
{
    if( (i_x2 == i_x1) || (i_y2 == i_y1) || (i_X == i_x1) )
    {
        return i_y1;
    }
    else if(i_X == i_x2)
    {
        return i_y2;
    }
    else if(i_roundup)
    {
        int32_t l_temp_num = (i_X - i_x1)*(i_y2 - i_y1)*10;
        int32_t l_temp_denom = (i_x2 - i_x1);
        int32_t l_temp = l_temp_num / l_temp_denom;
        if(l_temp % 10) // round up
           l_temp += 10;

        return (l_temp / 10) + i_y1;
    }
    else
    {
        return ( ((i_X - i_x1)*(i_y2 - i_y1)) / (i_x2 - i_x1) ) + i_y1;
    }
}

/**
 * get_poundV_points
 *
 * Description: Helper function that takes in a frequency (MHz) and returns the indicies
 *              for the 2 points in #V that the frequency falls between
 */
void get_poundV_points( uint32_t i_freq_mhz,
                        uint8_t* o_point1_index,
                        uint8_t* o_point2_index)
{
    int i;
    static bool L_trace_max_freq = TRUE;

    // this should never happen that we get a frequency > the highest VPD point
    // for now just trace we may decide later that this should cause a WOF reset
    if(i_freq_mhz > G_oppb.operating_points[NUM_PV_POINTS-1].frequency_mhz)
    {
       *o_point1_index = NUM_PV_POINTS-1;
       *o_point2_index = NUM_PV_POINTS-1;

       if(L_trace_max_freq)
       {
           L_trace_max_freq = FALSE;
           INTR_TRAC_ERR("get_poundV_points: Frequency[%d] is higher than max VPD[%d]",
                          i_freq_mhz, G_oppb.operating_points[NUM_PV_POINTS-1].frequency_mhz);

           INTR_TRAC_ERR("get_poundV_points: Pstate 0 frequency is %dkHz", G_oppb.frequency_max_khz);

           pgpe_wof_values_t l_PgpeWofValues;
           l_PgpeWofValues.dw0.value = in64(g_amec_sys.static_wof_data.pgpe_values_sram_addr);
           INTR_TRAC_ERR("PGPE dw0[0x%08X%08X]",
                          l_PgpeWofValues.dw0.words.high_order, l_PgpeWofValues.dw0.words.low_order);
       }
    }
    else
    {
        *o_point1_index = 0;
        *o_point2_index = 0;

        for(i = 0; i < NUM_PV_POINTS; i++)
        {
            if(i_freq_mhz <= G_oppb.operating_points[i].frequency_mhz)
            {
                *o_point1_index = i;
                *o_point2_index = i;

                // set point 1 to be the previous VPD point, leave equal to 2nd point
                // if freq was found to be below first VPD point or equal to a VPD point
                if( (i != 0) && (i_freq_mhz != G_oppb.operating_points[i].frequency_mhz) )
                {
                    *o_point1_index = i-1;
                }

                break;
            }
        }
    }
    return;
}

/**
 * read_sensor_data
 *
 * Description: One time place to read out all the sensors needed
 *              for wof calculations. First thing wof_main does.
 */
void read_sensor_data( void )
{
    uint8_t l_ambient = 0;

    // Read out necessary Sensor data for WOF calculation

    l_ambient = (uint8_t)getSensorByGsid(TEMPAMBIENT)->sample;
    if(l_ambient == 0)
    {
       // have not received ambient temperature
       // set condition to ff so highest ambient table is used
       g_wof->ambient_condition = 0xFF;
    }
    else
    {
       // add on adjustment to account for altitude
       if(g_wof->ambient_adj_for_altitude >= 0)
           g_wof->ambient_condition = l_ambient + g_wof->ambient_adj_for_altitude;
       else  // negative adjust prevent overflow
       {
           if(l_ambient >= (-g_wof->ambient_adj_for_altitude))
               g_wof->ambient_condition = l_ambient + g_wof->ambient_adj_for_altitude;
           else
               g_wof->ambient_condition = 0;
       }
       // If DIMM power credit is enabled determine adjustment to account for DIMM power
       if(g_wof->mem_thermal_credit_constant && g_wof->max_dimm_pwr_total_cW)
       {
           // determine g_wof->ambient_adj_for_dimm
           calc_wof_dimm_adjustment();

           if(g_wof->ambient_condition >= (-g_wof->ambient_adj_for_dimm))
               g_wof->ambient_condition += g_wof->ambient_adj_for_dimm;
           else
               g_wof->ambient_condition = 0;
       }
       else
           g_wof->ambient_adj_for_dimm = 0;
    }

    g_wof->curvdd_sensor  = getSensorByGsid(CURVDD)->sample;
    g_wof->curvcs_sensor  = getSensorByGsid(CURVCS)->sample;

    g_wof->Vdd_chip_p1mv  = getSensorByGsid(VOLTVDDSENSE)->sample;
    g_wof->Vdd_chip_index = get_voltage_index(g_wof->Vdd_chip_p1mv);

    g_wof->Vcs_chip_p1mv  = getSensorByGsid(VOLTVCSSENSE)->sample;
    g_wof->Vcs_chip_index = get_voltage_index(g_wof->Vcs_chip_p1mv);

    g_wof->T_racetrack    = getSensorByGsid(TEMPRTAVG)->sample;

    read_xgpe_values();

    setup_vdd();
    setup_vcs();
    setup_racetrack();
}

/**
 * calc_wof_dimm_adjustment
 *
 * Description: Calculate adjustment to ambient condition to account for DIMMs
 */
void calc_wof_dimm_adjustment( void )
{
    uint8_t  l_index1 = 0;
    uint8_t  l_index2 = 0;
    uint8_t  l_num_interp_pts = 0;
    int8_t   l_signed = 1;
    uint16_t l_ocmb_util = 0;
    uint16_t l_ocmb_util_p1 = 0;
    uint16_t l_util1 = 0;
    uint16_t l_util2 = 0;
    uint32_t l_ocmb_total_pwr_cW = 0;
    uint32_t l_pwr1_cW = 0;
    uint32_t l_pwr2_cW = 0;
    uint32_t l_processed_ocmbs = 0;
    uint32_t l_power_delta = 0;
    uint64_t l_temp64 = 0;
    uint64_t l_adjustment = 0;
    int l_ocmb_num, i;
    static bool L_traced_over_max = FALSE;
    static uint8_t L_trace_over_max_count = 2;
    static uint8_t L_trace_count = 2;

    // verify present OCMBs from WOF pwr data and memory config match
    if(g_wof->ocmbs_present != G_present_membufs)
    {
       // Mismatch in OCMB data disable WOF memory credit
       INTR_TRAC_ERR("calc_wof_dimm_adjustment: Mismatch OCMB Present[0x%08X] and OCMB WOF Pwr data[0x%08X]",
                      G_present_membufs, g_wof->ocmbs_present);
       g_wof->mem_thermal_credit_constant = 0;
    }
    else
    {
       for(l_ocmb_num = 0; l_ocmb_num < MAX_NUM_OCMBS; l_ocmb_num++)
       {
           if(MEMBUF_PRESENT(l_ocmb_num))
           {
              l_ocmb_util = getSensorByGsid((uint16_t)(MEMUTILM0 + l_ocmb_num))->sample;
              // Save OCMB util used
              g_wof->memutil[l_ocmb_num] = l_ocmb_util;
              if(IS_OCM_DDR5_MEM_TYPE(G_sysConfigData.mem_type))
              {
                  // add port 1 utilization
                  l_ocmb_util_p1 = getSensorByGsid((uint16_t)(MEMUTILP1M0 + l_ocmb_num))->sample;
                  l_ocmb_util += l_ocmb_util_p1;
                  g_wof->memutilp1[l_ocmb_num] = l_ocmb_util_p1;
              }

              // find the interpolation points for l_ocmb_util
              l_index1 = 0;
              l_index2 = 0;
              l_num_interp_pts = g_amec->proc[0].memctl[l_ocmb_num].membuf.num_interp_pts;
              if(l_num_interp_pts == 0)
              {
                 // no interpolation points, disable memory power credit
                 INTR_TRAC_ERR("calc_wof_dimm_adjustment: OCMB[%d] has no interpolation points",
                                l_ocmb_num);
                 g_wof->mem_thermal_credit_constant = 0;
                 break;
              }
              else if(l_ocmb_util >= g_amec->proc[0].memctl[l_ocmb_num].membuf.util_pwr_pt[l_num_interp_pts-1].util_cPercent)
              {
                 // set to last index
                 l_index1 = l_num_interp_pts - 1;
                 l_index2 = l_num_interp_pts - 1;
              }
              else
              {
                 for(i = 0; i < l_num_interp_pts; i++)
                 {
                     if(l_ocmb_util <= g_amec->proc[0].memctl[l_ocmb_num].membuf.util_pwr_pt[i].util_cPercent)
                     {
                        l_index1 = i;
                        l_index2 = i;
                        // set point 1 to be the previous point, leave equal to 2nd point
                        // if util was found to be below first point or equal to point
                        if( (i != 0) &&
                            (l_ocmb_util != g_amec->proc[0].memctl[l_ocmb_num].membuf.util_pwr_pt[i].util_cPercent) )
                        {
                            l_index1 = i-1;
                        }
                        break;
                     }
                 }
              }

              // interpolate power for l_ocmb_util
              l_util1 = g_amec->proc[0].memctl[l_ocmb_num].membuf.util_pwr_pt[l_index1].util_cPercent;
              l_util2 = g_amec->proc[0].memctl[l_ocmb_num].membuf.util_pwr_pt[l_index2].util_cPercent;
              l_pwr1_cW = g_amec->proc[0].memctl[l_ocmb_num].membuf.util_pwr_pt[l_index1].power_cW;
              l_pwr2_cW = g_amec->proc[0].memctl[l_ocmb_num].membuf.util_pwr_pt[l_index2].power_cW;
              l_ocmb_total_pwr_cW += interpolate_linear(l_ocmb_util,
                                                        l_util1,
                                                        l_util2,
                                                        l_pwr1_cW,
                                                        l_pwr2_cW,
                                                        TRUE);  // round up
              l_processed_ocmbs |= (MEMBUF0_PRESENT_MASK >> l_ocmb_num);
           } // if OCMB present
       }  // for each OCMB

       if(G_present_membufs != l_processed_ocmbs)
       {
           // Mismatch in present OCMBs and OCMBs processed
           INTR_TRAC_ERR("calc_wof_dimm_adjustment: Mismatch OCMB Present[0x%08X] and processed OCMBs[0x%08X]",
                          G_present_membufs, l_processed_ocmbs);
           // disable WOF memory credit
           g_wof->mem_thermal_credit_constant = 0;
       }
    }  // else present OCMBs from WOF pwr data and memory config match

    // make sure didn't hit an error during processing causing disable
    if(g_wof->mem_thermal_credit_constant)
    {
       g_wof->total_dimm_preheat_pwr_cW = l_ocmb_total_pwr_cW;
       // check if total power is over the max
       if(g_wof->total_dimm_preheat_pwr_cW > g_wof->max_dimm_pwr_total_cW)
       {
          l_signed = 1;
          l_power_delta = (uint32_t)(g_wof->total_dimm_preheat_pwr_cW - g_wof->max_dimm_pwr_total_cW);
          if( (L_traced_over_max == FALSE) ||
              ( (G_allow_trace_flags & ALLOW_MEM_TRACE) && L_trace_over_max_count ))
          {
              // trace once if we are over max
              L_traced_over_max = TRUE;
              if(L_trace_over_max_count)
                  L_trace_over_max_count--;
              INTR_TRAC_ERR("calc_wof_dimm_adjustment: Total dimm preheat power[%dcW] > max[%dcW]",
                             g_wof->total_dimm_preheat_pwr_cW, g_wof->max_dimm_pwr_total_cW);
          }
       }
       else
       {
          l_signed = -1;  // reduce ambient for being under max
          l_power_delta = (uint32_t)(g_wof->max_dimm_pwr_total_cW - g_wof->total_dimm_preheat_pwr_cW);
       }
       l_adjustment = (uint64_t)((uint64_t)l_power_delta * (uint64_t)g_wof->mem_thermal_credit_constant);

       if((G_allow_trace_flags & ALLOW_MEM_TRACE) && L_trace_count)
       {
            INTR_TRAC_INFO("MEM PWR DBUG1: ocmb_total_pwr[%dcW] - max_dimm_pwr_total[%dcW] =",
                           g_wof->total_dimm_preheat_pwr_cW, g_wof->max_dimm_pwr_total_cW);
            INTR_TRAC_INFO("MEM PWR DBUG2: power_delta[%d] * mem_thermal_credit_constant[%d] =",
                           l_power_delta, g_wof->mem_thermal_credit_constant);
            INTR_TRAC_INFO("MEM PWR DBUG3: l_adjustment[0x%08X%08X]",
                           (uint32_t)(l_adjustment>>32), (uint32_t)l_adjustment);
       }
       // adjustment / 1,000,000 -->
       //                           10,000 for mem_thermal_credit_constant sent to OCC as x10,000
       //                           100 for cW-->W
       l_temp64 = l_adjustment / (uint64_t)1000000;
       if((G_allow_trace_flags & ALLOW_MEM_TRACE) && L_trace_count)
       {
            INTR_TRAC_INFO("MEM PWR DBUG4: l_adjustment/1,000,000 = 0x%04X",
                           (uint16_t)l_temp64);
       }

       // round up if we are over max power
       if(l_signed == 1)
       {
           // make sure at least 1 degree is added to ambient
           if((l_temp64 == 0) || (l_adjustment % 1000000))
               l_temp64++;
       }

       g_wof->ambient_adj_for_dimm = (int8_t)(l_temp64 * l_signed);


       if(G_allow_trace_flags & ALLOW_MEM_TRACE)
       {
            if(L_trace_count)
            {
                L_trace_count--;
                INTR_TRAC_INFO("MEM PWR DBUG5: signed[%d] final dimm adj[%d]",
                                l_signed, g_wof->ambient_adj_for_dimm);
            }
       }
       else
       {
           // reset trace counts for ALLOW_MEM_TRACE to be set
           L_trace_count = 2;
           L_trace_over_max_count = 2;
       }
    } // if no error
    else
       g_wof->ambient_adj_for_dimm = 0;
}

/**
 * setup_vdd
 *
 * Description: One time place to get Vdd data setup for wof calculations
 */
void setup_vdd( void )
{
    // Scaled by "core" temperature scaling line
    bool l_non_core_scaling_line = FALSE;

    g_wof->scaled_all_off_off_vdd_chip_ua_c = scale_and_interpolate(G_oppb.iddq.iddq_all_good_cores_off_good_caches_off_5ma,
                                                                    G_oppb.iddq.avgtemp_all_cores_off_caches_off_p5c,
                                                                    g_wof->Vdd_chip_index,
                                                                    g_wof->T_racetrack,
                                                                    g_wof->Vdd_chip_p1mv,
                                                                    l_non_core_scaling_line);

    g_wof->scaled_all_off_off_vdd_vmin_ua_c = scale_and_interpolate(G_oppb.iddq.iddq_all_good_cores_off_good_caches_off_5ma,
                                                                    G_oppb.iddq.avgtemp_all_cores_off_caches_off_p5c,
                                                                    g_amec_sys.static_wof_data.Vdd_vret_index,
                                                                    g_wof->T_racetrack,
                                                                    g_amec_sys.static_wof_data.Vdd_vret_p1mv,
                                                                    l_non_core_scaling_line);

    g_wof->scaled_all_off_on_vdd_chip_ua_c = scale_and_interpolate(G_oppb.iddq.iddq_all_good_cores_off_good_caches_on_5ma,
                                                                   G_oppb.iddq.avgtemp_all_good_cores_off_good_caches_on_p5c,
                                                                   g_wof->Vdd_chip_index,
                                                                   g_wof->T_racetrack,
                                                                   g_wof->Vdd_chip_p1mv,
                                                                   l_non_core_scaling_line);

    g_wof->scaled_all_off_on_vdd_vmin_ua_c = scale_and_interpolate(G_oppb.iddq.iddq_all_good_cores_off_good_caches_on_5ma,
                                                                   G_oppb.iddq.avgtemp_all_good_cores_off_good_caches_on_p5c,
                                                                   g_amec_sys.static_wof_data.Vdd_vret_index,
                                                                   g_wof->T_racetrack,
                                                                   g_amec_sys.static_wof_data.Vdd_vret_p1mv,
                                                                   l_non_core_scaling_line);

    // divide by 32 to get single core
    g_wof->single_core_off_vdd_chip_ua_c = ( (g_wof->scaled_all_off_off_vdd_chip_ua_c *
                                              G_oppb.iddq.iddq_all_cores_off_all_caches_off_core_pct) >> 5) / 100;

    g_wof->single_cache_off_vdd_chip_ua_c = ( (g_wof->scaled_all_off_off_vdd_chip_ua_c *
                                               G_oppb.iddq.iddq_all_cores_off_all_caches_off_cache_pct) >> 5) / 100;

    g_wof->single_core_off_vdd_vmin_ua_c = ( (g_wof->scaled_all_off_off_vdd_vmin_ua_c *
                                              G_oppb.iddq.iddq_all_cores_off_all_caches_off_core_pct) >> 5) / 100;

    g_wof->single_cache_on_vdd_chip_ua_c = ( (g_wof->scaled_all_off_on_vdd_chip_ua_c - g_wof->scaled_all_off_off_vdd_chip_ua_c) /
                                               G_oppb.iddq.good_normal_cores_per_sort ) + g_wof->single_cache_off_vdd_chip_ua_c;

    // Scaled by "non-core" temperature scaling line
    l_non_core_scaling_line = TRUE;

    g_wof->scaled_all_off_off_vdd_chip_ua_nc = scale_and_interpolate(G_oppb.iddq.iddq_all_good_cores_off_good_caches_off_5ma,
                                                                     G_oppb.iddq.avgtemp_all_cores_off_caches_off_p5c,
                                                                     g_wof->Vdd_chip_index,
                                                                     g_wof->T_racetrack,
                                                                     g_wof->Vdd_chip_p1mv,
                                                                     l_non_core_scaling_line);

    g_wof->scaled_all_off_off_vdd_vmin_ua_nc = scale_and_interpolate(G_oppb.iddq.iddq_all_good_cores_off_good_caches_off_5ma,
                                                                     G_oppb.iddq.avgtemp_all_cores_off_caches_off_p5c,
                                                                     g_amec_sys.static_wof_data.Vdd_vret_index,
                                                                     g_wof->T_racetrack,
                                                                     g_amec_sys.static_wof_data.Vdd_vret_p1mv,
                                                                     l_non_core_scaling_line);

    g_wof->scaled_all_off_on_vdd_chip_ua_nc = scale_and_interpolate(G_oppb.iddq.iddq_all_good_cores_off_good_caches_on_5ma,
                                                                    G_oppb.iddq.avgtemp_all_good_cores_off_good_caches_on_p5c,
                                                                    g_wof->Vdd_chip_index,
                                                                    g_wof->T_racetrack,
                                                                    g_wof->Vdd_chip_p1mv,
                                                                    l_non_core_scaling_line);

    g_wof->scaled_all_off_on_vdd_vmin_ua_nc = scale_and_interpolate(G_oppb.iddq.iddq_all_good_cores_off_good_caches_on_5ma,
                                                                    G_oppb.iddq.avgtemp_all_good_cores_off_good_caches_on_p5c,
                                                                    g_amec_sys.static_wof_data.Vdd_vret_index,
                                                                    g_wof->T_racetrack,
                                                                    g_amec_sys.static_wof_data.Vdd_vret_p1mv,
                                                                    l_non_core_scaling_line);

    g_wof->single_core_off_vdd_chip_ua_nc = ( (g_wof->scaled_all_off_off_vdd_chip_ua_nc *
                                               G_oppb.iddq.iddq_all_cores_off_all_caches_off_core_pct) >> 5) / 100;

    g_wof->single_cache_off_vdd_chip_ua_nc = ( (g_wof->scaled_all_off_off_vdd_chip_ua_nc *
                                                G_oppb.iddq.iddq_all_cores_off_all_caches_off_cache_pct) >> 5) / 100;

    g_wof->single_core_off_vdd_vmin_ua_nc = ( (g_wof->scaled_all_off_off_vdd_vmin_ua_nc *
                                               G_oppb.iddq.iddq_all_cores_off_all_caches_off_core_pct) >> 5) / 100;

    g_wof->single_cache_on_vdd_chip_ua_nc = ( (g_wof->scaled_all_off_on_vdd_chip_ua_nc - g_wof->scaled_all_off_off_vdd_chip_ua_nc) /
                                               G_oppb.iddq.good_normal_cores_per_sort ) + g_wof->single_cache_off_vdd_chip_ua_nc;
}

/**
 * setup_vcs
 *
 * Description: One time place to get Vcs data setup for wof calculations
 */
void setup_vcs( void )
{
    // Scaled by "core" temperature scaling line
    bool l_non_core_scaling_line = FALSE;

    g_wof->scaled_all_off_off_vcs_chip_ua_c = scale_and_interpolate(G_oppb.iddq.icsq_all_good_cores_off_good_caches_off_5ma,
                                                                    G_oppb.iddq.avgtemp_all_cores_off_caches_off_p5c,
                                                                    g_wof->Vcs_chip_index,
                                                                    g_wof->T_racetrack,
                                                                    g_wof->Vcs_chip_p1mv,
                                                                    l_non_core_scaling_line);

    g_wof->scaled_all_off_on_vcs_chip_ua_c = scale_and_interpolate(G_oppb.iddq.icsq_all_good_cores_off_good_caches_on_5ma,
                                                                   G_oppb.iddq.avgtemp_all_good_cores_off_good_caches_on_p5c,
                                                                   g_wof->Vcs_chip_index,
                                                                   g_wof->T_racetrack,
                                                                   g_wof->Vcs_chip_p1mv,
                                                                   l_non_core_scaling_line);

    // divide by 32 to get single core
    g_wof->single_core_off_vcs_chip_ua_c = ( (g_wof->scaled_all_off_off_vcs_chip_ua_c *
                                              G_oppb.iddq.icsq_all_cores_off_all_caches_off_core_pct) >> 5) / 100;

    g_wof->single_cache_off_vcs_chip_ua_c = ( (g_wof->scaled_all_off_off_vcs_chip_ua_c *
                                               G_oppb.iddq.icsq_all_cores_off_all_caches_off_cache_pct) >> 5) / 100;

    g_wof->single_cache_on_vcs_chip_ua_c = ( (g_wof->scaled_all_off_on_vcs_chip_ua_c - g_wof->scaled_all_off_off_vcs_chip_ua_c) /
                                               G_oppb.iddq.good_normal_cores_per_sort ) + g_wof->single_cache_off_vcs_chip_ua_c;

    // Scaled by "non-core" temperature scaling line
    l_non_core_scaling_line = TRUE;

    g_wof->scaled_all_off_off_vcs_chip_ua_nc = scale_and_interpolate(G_oppb.iddq.icsq_all_good_cores_off_good_caches_off_5ma,
                                                                     G_oppb.iddq.avgtemp_all_cores_off_caches_off_p5c,
                                                                     g_wof->Vcs_chip_index,
                                                                     g_wof->T_racetrack,
                                                                     g_wof->Vcs_chip_p1mv,
                                                                     l_non_core_scaling_line);

    g_wof->scaled_all_off_on_vcs_chip_ua_nc = scale_and_interpolate(G_oppb.iddq.icsq_all_good_cores_off_good_caches_on_5ma,
                                                                    G_oppb.iddq.avgtemp_all_cores_off_caches_off_p5c,
                                                                    g_wof->Vcs_chip_index,
                                                                    g_wof->T_racetrack,
                                                                    g_wof->Vcs_chip_p1mv,
                                                                    l_non_core_scaling_line);

    g_wof->single_core_off_vcs_chip_ua_nc = ( (g_wof->scaled_all_off_off_vcs_chip_ua_nc *
                                               G_oppb.iddq.icsq_all_cores_off_all_caches_off_core_pct) >> 5) / 100;

    g_wof->single_cache_off_vcs_chip_ua_nc = ( (g_wof->scaled_all_off_off_vcs_chip_ua_nc *
                                                G_oppb.iddq.icsq_all_cores_off_all_caches_off_cache_pct) >> 5) / 100;

    g_wof->single_cache_on_vcs_chip_ua_nc = ( (g_wof->scaled_all_off_on_vcs_chip_ua_nc - g_wof->scaled_all_off_off_vcs_chip_ua_nc) /
                                               G_oppb.iddq.good_normal_cores_per_sort ) + g_wof->single_cache_off_vcs_chip_ua_nc;
}

/**
 * setup_racetrack
 *
 * Description: One time place to get racetrack data setup for wof calculations
 */
void setup_racetrack( void )
{
    // Racetrack values use "non-core" temperature scaling line

    g_wof->racetrack_only_vdd_chip_ua = (g_wof->scaled_all_off_off_vdd_chip_ua_nc *
                                         G_oppb.iddq.iddq_all_cores_off_all_caches_off_racetrack_pct) / 100;

    g_wof->racetrack_only_vcs_chip_ua = (g_wof->scaled_all_off_off_vcs_chip_ua_nc *
                                         G_oppb.iddq.icsq_all_cores_off_all_caches_off_racetrack_pct) / 100;
}

/**
 * set_clear_wof_disabled
 *
 * Description: Sets or clears the bit specified by i_bit_mask and
 *              logs an error if an error hasnt already been logged.
 *
 * Param[in]: i_action - Either CLEAR(0) or SET(1).
 * Param[in]: i_bit_mask - The bit to set or clear. If setting a bit,
 *                         this will be added to the errorlog created
 *                         as userdata1
 * Param[in]: i_ext_rc - The extended reason code to be added to
 *                       error log
 */
void set_clear_wof_disabled( uint8_t i_action,
                             uint32_t i_bit_mask,
                             uint16_t i_ext_rc )
{
    // Keep track of whether an error has already been logged
    static bool L_errorLogged = false;
    bool l_logError = false;
    errlHndl_t l_errl = NULL;
    uint32_t prev_wof_disabled = g_wof->wof_disabled;

    do
    {
        if( i_action == SET )
        {
            // Set the vrt state back to standby
            g_wof->vrt_state = STANDBY;

            // If user is trying to force a reset even though WOF is disabled,
            // Skip straight to error log creation
            if( (g_wof->wof_disabled) &&
                (i_bit_mask == WOF_RC_RESET_DEBUG_CMD) )
            {
                INTR_TRAC_INFO("User Requested WOF reset!");
                l_logError = true;
                break;
            }

            // Set the bit
            g_wof->wof_disabled |= i_bit_mask;

            // If OCC has not yet been enabled through TMGT/HTMGT/OPPB, skip
            // error log
            if( (g_wof->wof_disabled & WOF_RC_OCC_WOF_DISABLED) ||
                (g_wof->wof_disabled & WOF_RC_OPPB_WOF_DISABLED) )
            {
                // only trace if this was a real WOF error
                if( i_bit_mask & ERRL_RETURN_CODES )
                {
                    INTR_TRAC_ERR("OCC encountered a WOF error before (H)TMGT"
                                  " enabled it. wof_disabled = 0x%08x",
                                  g_wof->wof_disabled);
                }
                break;
            }

            // If error has already been logged, trace and skip
            if( L_errorLogged )
            {
                if( i_bit_mask & ERRL_RETURN_CODES )
                {
                    INTR_TRAC_ERR("Another WOF error was encountered!"
                                  " wof_disabled=0x%08x",
                                  g_wof->wof_disabled);
                }
                else
                {
                    INTR_TRAC_INFO("Non-error WOF disable reason was encountered!"
                                  " wof_disabled=0x%08x",
                                  g_wof->wof_disabled);
                }
            }
            else
            {
                // Make sure the reason requires an error log
                if( i_bit_mask & ERRL_RETURN_CODES )
                {
                    l_logError = true;
                    INTR_TRAC_ERR("WOF encountered an error. wof_disabled ="
                                  " 0x%08x", g_wof->wof_disabled );
                }
                else
                {
                    INTR_TRAC_INFO("WOF disabled due to non-error condition. wof_disabled ="
                                   " 0x%08x", g_wof->wof_disabled );
                }

                // if the previous wof_disabled was all zeros,
                // send IPC command to PGPE to disable wof
                if( !prev_wof_disabled )
                {
                    // Disable WOF
                    disable_wof();

                    INTR_TRAC_INFO("WOF disabled, setting frequency ranges");
                    // Set the the frequency ranges
                    l_errl = amec_set_freq_range(CURRENT_MODE());
                    if(l_errl)
                    {
                        INTR_TRAC_ERR("WOF: amec_set_freq_range reported an error");
                        commitErrl( &l_errl);
                    }
                }
            }
        }
        else if ( i_action == CLEAR )
        {
            // Clear the bit
            g_wof->wof_disabled &= ~i_bit_mask;

            // If TMGT/HTMGT is enabling WOF, check for any previous
            // errors and log if they exist and if they
            // should log an error.
            if( (i_bit_mask == WOF_RC_OCC_WOF_DISABLED) ||
                (i_bit_mask == WOF_RC_OPPB_WOF_DISABLED) )
            {

                uint32_t disabled_mask = WOF_RC_OCC_WOF_DISABLED |
                                         WOF_RC_OPPB_WOF_DISABLED;

                // If OPPB or (H)TMGT still say wof is disabled, don't log
                // any errors if the other is still set.
                if( g_wof->wof_disabled & disabled_mask )
                {
                    break;
                }
                // Log any lingering errors.
                else if( g_wof->wof_disabled & ERRL_RETURN_CODES )
                {
                    l_logError = true;
                }
                break;
            }

            // If clearing the bit put wof_disabled at all 0's AND
            // wof_disabled was not already all 0's, wof is being
            // re-enabled. Log informational error.
            if( prev_wof_disabled && !g_wof->wof_disabled )
            {
                /** @
                 *  @errortype
                 *  @moduleid   SET_CLEAR_WOF_DISABLED
                 *  @reasoncode WOF_RE_ENABLED
                 *  @userdata1  Last Bit cleared
                 *  @userdata2  0
                 *  @userdata4  OCC_NO_EXTENDED_RC
                 *  @devdesc    WOF is being re-enabled
                 */
                l_errl = createErrl(
                        SET_CLEAR_WOF_DISABLED,
                        WOF_RE_ENABLED,
                        OCC_NO_EXTENDED_RC,
                        ERRL_SEV_INFORMATIONAL,
                        NULL,
                        DEFAULT_TRACE_SIZE,
                        i_bit_mask,
                        0 );

                // commit the error log
                commitErrl( &l_errl );
            }
        }
        else
        {
            INTR_TRAC_ERR("Invalid action given. Ignoring for now...");
        }
    }while( 0 );

    // Check for error
    if( l_logError )
    {
        if( (g_wof->wof_disabled & (~(ERRL_RETURN_CODES))) &&
            (i_bit_mask != WOF_RC_RESET_DEBUG_CMD)  )
        {
            INTR_TRAC_ERR("Encountered an error, but WOF is off. RC: 0x%08x",
                    i_bit_mask);
        }
        else
        {
            // Create error log
            /** @errortype
             *  @moduleid   SET_CLEAR_WOF_DISABLED
             *  @reasoncode WOF_DISABLED_RC
             *  @userdata1  current wof_disabled
             *  @userdata2  Bit requested to be set
             *  @userdata4  Unique extended RC given by caller
             *  @devdesc    WOF has been disabled due to an error
             */
            l_errl = createPgpeErrl(SET_CLEAR_WOF_DISABLED,
                                    WOF_DISABLED_RC,
                                    i_ext_rc,
                                    ERRL_SEV_UNRECOVERABLE,
                                    g_wof->wof_disabled,
                                    i_bit_mask );

            // Reset if on Reason Code requires it.
            if(i_bit_mask & ~(IGNORE_WOF_RESET) )
            {
                //Callout firmware
                addCalloutToErrl(l_errl,
                                 ERRL_CALLOUT_TYPE_COMPONENT_ID,
                                 ERRL_COMPONENT_ID_FIRMWARE,
                                 ERRL_CALLOUT_PRIORITY_HIGH);

                //Callout processor
                addCalloutToErrl(l_errl,
                                 ERRL_CALLOUT_TYPE_HUID,
                                 G_sysConfigData.proc_huid,
                                 ERRL_CALLOUT_PRIORITY_MED);

                REQUEST_WOF_RESET( l_errl );
            }
            else
            {
                // Just commit the error log
                commitErrl( &l_errl );
            }

            L_errorLogged = true;
        }
    }
}

/**
 * disable_wof
 *
 * Description: Sends IPC command to PGPE to turn WOF off.
 *              This function DOES NOT set the specific reason
 *              bit in the amec structure.
 */
void disable_wof( void )
{
    errlHndl_t l_errl = NULL;
    uint8_t l_prev_state = g_wof->wof_init_state;

    // Disable wof on 405
    g_wof->wof_init_state = WOF_DISABLED;

    INTR_TRAC_ERR("WOF is being disabled. Reasoncode: 0x%08x",
                  g_wof->wof_disabled );
    uint32_t reasonCode = 0;
    int user_data_rc = 0;
    do
    {
        if(l_prev_state >= WOF_CONTROL_ON_SENT_WAITING)
        {
            // Make sure IPC command is idle
            if(async_request_is_idle(&G_wof_control_req.request))
            {

                // Check to see if a previous wof control IPC message observed an error
                if( g_wof->control_ipc_rc != 0 )
                {
                    INTR_TRAC_ERR("Unknown error from wof control IPC message(disable)");
                    INTR_TRAC_ERR("Return Code = 0x%x", g_wof->control_ipc_rc);
                    /** @
                     *  @errortype
                     *  @moduleid   DISABLE_WOF
                     *  @reasoncode GPE_REQUEST_RC_FAILURE
                     *  @userdata1  rc - wof_control rc
                     *  @userdata2  0
                     *  @userdata4  OCC_NO_EXTENDED_RC
                     *  @devdesc    OCC Failure from sending wof control
                     */
                    user_data_rc = g_wof->control_ipc_rc;
                    g_wof->control_ipc_rc = 0;
                    reasonCode = GPE_REQUEST_RC_FAILURE;
                }
                else
                {
                    // Set parameters for the GpeRequest
                    G_wof_control_parms.action = PGPE_ACTION_WOF_OFF;
                    user_data_rc = pgpe_request_schedule( &G_wof_control_req );

                    if( user_data_rc != 0 )
                    {
                        /** @
                         *  @errortype
                         *  @moduleid   DISABLE_WOF
                         *  @reasoncode GPE_REQUEST_SCHEDULE_FAILURE
                         *  @userdata1  rc - gpe_request_schedule return code
                         *  @userdata2  0
                         *  @userdata4  OCC_NO_EXTENDED_RC
                         *  @devdesc    OCC Failed to schedule a GPE job for enabling wof
                         */
                        reasonCode = GPE_REQUEST_SCHEDULE_FAILURE;
                        INTR_TRAC_ERR("disable_wof() - Error when sending WOF Control"
                                     " OFF IPC command! RC = %x", user_data_rc );
                    }
                }

                if( user_data_rc != 0 )
                {
                    l_errl = createErrl(
                            DISABLE_WOF,
                            reasonCode,
                            OCC_NO_EXTENDED_RC,
                            ERRL_SEV_PREDICTIVE,
                            NULL,
                            DEFAULT_TRACE_SIZE,
                            user_data_rc,
                            0);

                    // commit the error log
                    commitErrl( &l_errl );
                }
            }
        }
        else
        {
            INTR_TRAC_IMP("WOF has not been enabled so no need to disable");
        }
    }while( 0 );
}

/**
 * enable_wof
 *
 * Description: Sends IPC command to PGPE to turn WOF on
 *
 * Return: True if we were able to successfully schedule the IPC command
 *         False if the IPC request is still idle.
 */
bool enable_wof( void )
{
    INTR_TRAC_IMP("WOF is being enabled...");
    uint32_t reasonCode = 0;
    bool     result     = true;
    uint32_t bit_to_set = 0;
    int      rc         = 0;
    uint16_t erc        = 0;
    // Make sure IPC command is idle.
    if(!async_request_is_idle( &G_wof_control_req.request ) )
    {
        result = false;
    }
    else
    {
        // Check to see if a previous wof control IPC message observed an error
        if( g_wof->control_ipc_rc != 0 )
        {
            INTR_TRAC_ERR("Unknown error from wof control IPC message(enable)");
            INTR_TRAC_ERR("Return Code = 0x%X", g_wof->control_ipc_rc);
            rc = g_wof->control_ipc_rc;
            erc = ERC_WOF_CONTROL_REQ_FAILURE;
            bit_to_set = WOF_RC_CONTROL_REQ_FAILURE;
            g_wof->control_ipc_rc = 0;
            /** @
             *  @errortype
             *  @moduleid   ENABLE_WOF
             *  @reasoncode GPE_REQUEST_RC_FAILURE
             *  @userdata1  rc - wof_control RC
             *  @userdata2  0
             *  @userdata4  OCC_NO_EXTENDED_RC
             *  @devdesc    OCC Failure from sending wof command
             */
            reasonCode = GPE_REQUEST_RC_FAILURE;
        }
        else
        {

            // Set parameters for the GpeRequest
            G_wof_control_parms.action = PGPE_ACTION_WOF_ON;

            // Set Init state
            g_wof->wof_init_state = WOF_CONTROL_ON_SENT_WAITING;

            rc = pgpe_request_schedule( &G_wof_control_req );

            if( rc != 0 )
            {
                INTR_TRAC_ERR("enable_wof() - Error when sending WOF Control"
                        " ON IPC command! RC = %x", rc);
                /** @
                 *  @errortype
                 *  @moduleid   ENABLE_WOF
                 *  @reasoncode GPE_REQUEST_SCHEDULE_FAILURE
                 *  @userdata1  rc - gpe_request_schedule return code
                 *  @userdata2  0
                 *  @userdata4  OCC_NO_EXTENDED_RC
                 *  @devdesc    OCC Failed to schedule a GPE job for enabling wof
                 */
                bit_to_set = WOF_RC_PGPE_WOF_DISABLED;
                erc = ERC_WOF_PGPE_WOF_DISABLED;
                reasonCode = GPE_REQUEST_SCHEDULE_FAILURE;
            }
            else
            {
                result = true;
            }
        }
        if( rc != 0 )
        {
            errlHndl_t l_errl = createErrl(
                    ENABLE_WOF,
                    reasonCode,
                    erc,
                    ERRL_SEV_PREDICTIVE,
                    NULL,
                    DEFAULT_TRACE_SIZE,
                    rc,
                    0);

            result = false;
            set_clear_wof_disabled( SET, bit_to_set, erc );

            // Commit the error
            commitErrl( &l_errl );
        }
    }
    return result;
}

/**
 * wof_control_callback
 *
 * Description: Callback function for wof_control GpeRequest. Upon successful
 *              return of the asynchronous request, if successful, set the
 *              appropriate wof_init state based on whether request wanted
 *              to turn WOF on or off
 */
void wof_control_callback( void )
{
    // save time IPC took in us
    uint32_t l_time = (uint32_t)((G_wof_control_req.request.end_time - G_wof_control_req.request.start_time) / ( SSX_TIMEBASE_FREQUENCY_HZ / 1000000 ));
    sensor_update( AMECSENSOR_PTR(WOFC_IPCdur), (uint16_t)l_time);

    // Check to see if GpeRequest was successful
    if( G_wof_control_parms.msg_cb.rc == PGPE_WOF_RC_NOT_ENABLED )
    {
        // PGPE cannot enable wof
        g_wof->pgpe_wof_disabled = 1;
    }
    else if( G_wof_control_parms.msg_cb.rc == PGPE_RC_SUCCESS)
    {
        // GpeRequest Success. Set Globals based on action
        if( G_wof_control_parms.action == PGPE_ACTION_WOF_ON )
        {
            g_wof->wof_init_state = PGPE_WOF_ENABLED_NO_PREV_DATA;
        }
        else
        {
            // Record that 405 turned PGPE WOF off
            g_wof->pgpe_wof_off = 1;
        }
    }
    else
    {
        // Record the error return code we saw in this callback to be
        // picked up on next tick
        g_wof->control_ipc_rc = G_wof_control_parms.msg_cb.rc;
    }
}


/**
 * task_send_vrt_to_pgpe
 *
 * Description: Schedule the VRT IPC command to send VRT to the PGPE
 *              Called from the tick table at the beginning of every tick.
 *              This also gives the PGPE a required timing for HW power
 *              proxy throttle control loop to write to QME register every 500us
 */
void task_send_vrt_to_pgpe(task_t* i_task)
{
    if( (g_wof->vrt_state == NEED_TO_SCHEDULE ) &&
        (async_request_is_idle(&G_wof_vrt_req.request)) )
    {
        if(G_allow_trace_flags & ALLOW_VRT_TRACE)
        {
            INTR_TRAC_INFO("scheduling VRT OTBR[0x%08X]", in32(OCB_OTBR));
        }
        // must set vrt state prior to calling schedule to prevent re-setting
        // it to scheduled after VRT callback function processed
        g_wof->vrt_state = SCHEDULED;

        g_wof->gpe_req_rc = pgpe_request_schedule(&G_wof_vrt_req);

        // Check to make sure IPC request was scheduled correctly
        if( g_wof->gpe_req_rc != 0 )
        {
            // failed set VRT state to standby
            g_wof->vrt_state = STANDBY;
            // If we were attempting to send the initial VRT request,
            // reset the state machine so we can start the process
            // over
            if( g_wof->wof_init_state == INITIAL_VRT_SENT_WAITING )
            {
                g_wof->wof_init_state = WOF_DISABLED;
            }

            INTR_TRAC_ERR("schedule_vrt_request: Error sending VRT! gperc=%d"
                            g_wof->gpe_req_rc );

            // Formally disable wof
            set_clear_wof_disabled( SET, WOF_RC_IPC_FAILURE, ERC_WOF_IPC_FAILURE );

            // Reset the global return code after logging the error
            g_wof->gpe_req_rc = 0;
        }
    } // if VRT request idle and need to scheudle
}



/**
 * send_initial_vrt_to_pgpe
 *
 * Description: Function calculates the address of the final vrt in Main Memory
 *              and subsequently sends that address to the PGPE to use via IPC
 *              command.
 */
void send_initial_vrt_to_pgpe( void )
{
    // Set the steps for all VRT parms to the max value
    g_wof->vcs_step_from_start  = g_amec_sys.static_wof_data.wof_header.vcs_size - 1;
    g_wof->vdd_step_from_start  = g_amec_sys.static_wof_data.wof_header.vdd_size - 1;
    g_wof->io_pwr_step_from_start  = g_amec_sys.static_wof_data.wof_header.io_size - 1;
    g_wof->ambient_step_from_start  = g_amec_sys.static_wof_data.wof_header.amb_cond_size - 1;

    // Calculate the address of the final vrt
    g_wof->vrt_main_mem_addr = calc_vrt_mainstore_addr();

    // Copy the final vrt to shared OCC-PGPE SRAM.
    copy_vrt_to_sram( g_wof->vrt_main_mem_addr );

    // Update Init state
    if(g_wof->wof_init_state < INITIAL_VRT_SENT_WAITING)
    {
        g_wof->wof_init_state = INITIAL_VRT_SENT_WAITING;
    }

}


/**
 * get_voltage_index
 *
 * Description: Using the input voltage, returns the index to the lower bound
 *              of the two voltages surrounding the input voltage in
 *              G_iddq_voltages
 *
 * Param[in]: i_voltage - the input voltage to select the index for (100uV units)
 *
 * Return: The index to the lower bound voltage in G_iddq_voltages
 */
int get_voltage_index( uint32_t i_voltage )
{

    int l_volt_idx;
    if( i_voltage <= G_iddq_voltages[0] )
    {
        // Voltage is <= to first entry. Use first two entries.
        l_volt_idx = 0;
    }
    else if( i_voltage >= G_iddq_voltages[CORE_IDDQ_MEASUREMENTS-1] )
    {
        // Voltage is >= to last entry. Use last two entries.
        l_volt_idx = CORE_IDDQ_MEASUREMENTS - 2;
    }
    else
    {
        // Search for entries on either side of our voltage
        for(l_volt_idx = 0; l_volt_idx < CORE_IDDQ_MEASUREMENTS - 1; l_volt_idx++)
        {
            if( (i_voltage >= G_iddq_voltages[l_volt_idx]) &&
                (i_voltage <= G_iddq_voltages[l_volt_idx+1]) )
            {
                break;
            }
        }
    }
    return l_volt_idx;
}

/**
 * scale
 *
 * Description: Leakage temperature scaling function is split into "core" vs "non-core" curves
 *              based on the distribution of VT devices used in those areas
 *              new current = present current * (1 + (t2c(T1) - t2c(T2)) / t2c(T2))
 *              using a 3rd degree polynomial for either core or non-core temperature scaling line
 *              t2c(T) = aT^3 + bT^2 + cT + d
 *              result is returned in 0.001 unit
 * Param[in]: i_target_temp
 * Param[in]: i_reference_temp
 * Param[in]: i_non_core_scaling_line - TRUE use non-core temperature scaling line,
 *                                      FALSE use core temperature scaling line
 */
uint32_t scale( uint16_t i_target_temp,
                uint16_t i_reference_temp,
                bool i_non_core_scaling_line )
{
    static bool L_traced_zero = FALSE;
    static bool L_traced_negative = FALSE;
    static uint8_t L_trace_count = 2;
    int32_t  l_iddqt1 = 0;
    int32_t  l_iddqt2 = 0;
    int32_t  l_result = 0;
    int32_t  l_temp32 = 0;

    // Temperature scaling lines are 3rd degree polynomials: aT^3 + bT^2 + cT + d
    // default coefficients a, b, c, d for core scaling line
    // coefficients are in 0.00001 unit
    int32_t l_coeff_a = 31;      //   0.000306435182
    int32_t l_coeff_b = -869;    //  -0.00869469445
    int32_t l_coeff_c = 97938;   //   0.979379065
    int32_t l_coeff_d = 1429459; //  14.2945863

/*
    // currently we will use the same coefficients for non-core
    if(i_non_core_scaling_line)
    {
       // change coefficients for non-core temperature scaling line
    }
*/

    l_iddqt1 = (l_coeff_a * (i_target_temp * i_target_temp * i_target_temp));
    l_iddqt1 += (l_coeff_b * (i_target_temp * i_target_temp));
    l_iddqt1 += (l_coeff_c * i_target_temp);
    l_iddqt1 += l_coeff_d;

    l_iddqt2 = l_coeff_a * (i_reference_temp * i_reference_temp * i_reference_temp);
    l_iddqt2 += l_coeff_b * (i_reference_temp * i_reference_temp);
    l_iddqt2 += l_coeff_c * i_reference_temp;
    l_iddqt2 += l_coeff_d;

    // avoid divide by 0
    if(l_iddqt2 != 0)
    {
        // l_iddqt's are in 0.00001 unit we want result in 0.001 unit so divide by only
        // 1000 (not 100,000) to account for the /100 needed to convert 0.00001-->0.001
        l_temp32 = l_iddqt2 / 1000;
        l_result = (l_iddqt1 - l_iddqt2) / l_temp32;


        if( (G_allow_trace_flags & ALLOW_WOF_SCALE_TRACE) && (L_trace_count) )
        {
            INTR_TRAC_INFO("scale: target temp(t1)[%d] ref temp(t2)[%d]",
                           i_target_temp, i_reference_temp);

            INTR_TRAC_INFO("scale: iddqt1[%d] iddqt2[%d]",
                            l_iddqt1, l_iddqt2);

            INTR_TRAC_INFO("scale: l_temp32[%d] result[%d]", l_temp32, l_result);
        }

        // add one
        l_result += 1000;
        if(G_allow_trace_flags & ALLOW_WOF_SCALE_TRACE)
        {
            if(L_trace_count)
            {
               L_trace_count--;
               INTR_TRAC_INFO("scale: FINAL RESULT += 1000 = %d (0.001 unit)", l_result);
            }
        }
        else
           L_trace_count = 2; // reset for ALLOW_WOF_SCALE_TRACE to be set
    }
    else
    {
        l_result = 0;
        if(!L_traced_zero)
        {
            INTR_TRAC_ERR("scale: leakage temp scaling iddqt2 is 0! ref temp[%d] iddqt1[%d] target temp[%d]",
                            i_reference_temp, l_iddqt1, i_target_temp);
            INTR_TRAC_ERR("scale: coefficients a[%d] b[%d] c[%d] d[%d]",
                            l_coeff_a, l_coeff_b, l_coeff_c, l_coeff_d);
            L_traced_zero = TRUE;
        }
    }

    // check for negative
    if(l_result < 0)
    {
        if(!L_traced_negative)
        {
            INTR_TRAC_ERR("scale: leakage temp scaling negative result[%d]! iddqt1[%d] iddqt2[%d]",
                            l_result, l_iddqt1, l_iddqt2);
            INTR_TRAC_ERR("scale: coefficients a[%d] b[%d] c[%d] d[%d]",
                            l_coeff_a, l_coeff_b, l_coeff_c, l_coeff_d);
            INTR_TRAC_ERR("scale: leakage temp scaling ref temp[%d] target temp[%d]",
                            i_reference_temp, i_target_temp);
            L_traced_negative = TRUE;
        }
        l_result = 0;
    }

    return l_result;
}

/**
 * scale_and_interpolate
 *
 * Desctiption: This function combines the scale and interpolate_linear
 *              functions in order to approximate a current based off the
 *              voltage scaled to the measured temperature (base temp)
 *
 * Param[in]: i_leak_arr - Array of IQ Currents taken from vpd (OCC Pstate
 *                         parameter block.
 * Param[in]: i_avgtemp_arr - Array of temperatures in which the currents
 *                            in i_leak_arr were measured. Also taken from vpd.
 * Param[in]: i_idx - The index into the two arrays calculated from voltage
 * Param[in]: i_target_temp - The target temperature used to scale the current
 * Param[in]: i_voltage - The associated voltage. (100uV units)
 * Param[in]: i_non_core_scaling_line - TRUE use non-core temperature scaling line,
 *                                      FALSE use core temperature scaling line
 *
 * Return: The approximated scaled current in uA.
 */
uint32_t scale_and_interpolate( uint16_t * i_leak_arr,
                                uint8_t * i_avgtemp_arr,
                                int i_idx,
                                uint16_t i_target_temp,
                                uint16_t i_voltage,
                                bool i_non_core_scaling_line )
{
    // Note: leakage arrays are in 5mA. Multiply by 5 to convert to mA
    uint32_t scaled_lower_leak = i_leak_arr[i_idx] * 5;
    uint32_t scaled_upper_leak = i_leak_arr[i_idx+1] * 5;

    // Note: avgtemp arrays are in 0.5C. Divide by 2 to convert to 1C
    // scale() returns value in 0.001 unit meaning mA just got converted to uA
    scaled_lower_leak *= scale( i_target_temp,
                                i_avgtemp_arr[i_idx] >> 1,
                                i_non_core_scaling_line );
    scaled_upper_leak *= scale( i_target_temp,
                                i_avgtemp_arr[i_idx+1] >> 1,
                                i_non_core_scaling_line );

    // Approximate current between the scaled currents using linear
    // interpolation and return the result
    return interpolate_linear( (int32_t) i_voltage,
                               (int32_t) G_iddq_voltages[i_idx],
                               (int32_t) G_iddq_voltages[i_idx+1],
                               (int32_t) scaled_lower_leak,
                               (int32_t) scaled_upper_leak,
                                         FALSE); // round down

}

/**
 * print_oppb
 *
 * Description: For internal use only. Traces contents of G_oppb
 */
void print_oppb( void )
{
    CMDH_TRAC_INFO("Printing Contents of OCCPstateParmBlock magic num[0x%08X%08X]",
                    WORD_HIGH(G_oppb.magic.value), WORD_LOW(G_oppb.magic.value));
    CMDH_TRAC_INFO("OPPB Attributes: pstates_enabled=0x%02X, wof_enabled=0x%02X, dds_enabled=0x%02X, ocs_enabled=0x%02X",
                    G_oppb.attr.fields.pstates_enabled, G_oppb.attr.fields.wof_enabled,
                    G_oppb.attr.fields.dds_enabled, G_oppb.attr.fields.ocs_enabled);

    int i;
    for(i = 0; i < NUM_PV_POINTS; i++)
    {
        CMDH_TRAC_INFO("operating_points[%d]: Freq[%d] vdd_mv[%d] idd_tdp_ac_10ma[%d] ",
                       i,
                       G_oppb.operating_points[i].frequency_mhz,
                       G_oppb.operating_points[i].vdd_mv,
                       G_oppb.operating_points[i].idd_tdp_ac_10ma );
    }

    CMDH_TRAC_INFO("Pstate 0[%dMHz] Min Pstate %d[%dMHz]",
                    G_oppb.frequency_max_khz / 1000,
                    G_oppb.pstate_min,
                    G_oppb.frequency_min_khz / 1000);

    CMDH_TRAC_INFO("Ceiling[%dMHz] Ultra Turbo[%dMHz] Fmax[%d]",
                    G_oppb.frequency_ceiling_khz / 1000,
                    G_oppb.ultraturbo_freq_mhz,
                    G_oppb.fmax_freq_mhz);
}

/**
 * prevent_over_current
 *
 * Description: Determines whether ceff_ratio_vdd will cause an over-current
 *              and clips it to a new ceff ratio if necessary.
 *
 * Param: Calculated ceff_ratio before any clipping
 *
 * Return: Clipped ceff_ratio
 */
uint32_t prevent_over_current( uint32_t i_ceff_ratio )
{
    static uint8_t L_ocs_dirty_prev = 0;
    uint32_t l_clipped_ratio = i_ceff_ratio;
    uint32_t l_new_ratio_from_measured = 0;
    uint32_t l_new_ratio_from_prev = 0;

    // determine if Ceff needs to be adjusted for overcurrent
    if(l_clipped_ratio > G_max_ceff_ratio)
         l_clipped_ratio = G_max_ceff_ratio;

    if((g_wof->ocs_dirty & OCS_PGPE_DIRTY_MASK) == 0)
    {
        // no over current condition detected on previous PGPE tick time
        // decrease OC CeffRatio addr, stop at 0
        if(g_wof->vdd_oc_ceff_add > g_wof->ocs_decrease_ceff)
        {
            g_wof->vdd_oc_ceff_add -= g_wof->ocs_decrease_ceff;

            // now determine if any adjustment is needed
            if(i_ceff_ratio < g_wof->vdd_ceff_ratio_adj_prev)
            {
                 // new measured ratio is lower than previous add the adder but max out at previous
                 l_clipped_ratio += g_wof->vdd_oc_ceff_add;
                 if(l_clipped_ratio > g_wof->vdd_ceff_ratio_adj_prev)
                     l_clipped_ratio = g_wof->vdd_ceff_ratio_adj_prev;
            }
        }
        else // no adjustment to ceff ratio
        {
            g_wof->vdd_oc_ceff_add = 0;
        }
        if( (L_ocs_dirty_prev != g_wof->ocs_dirty) &&
            (G_allow_trace_flags & ALLOW_WOF_OCS_TRACE) )
        {
           INTR_TRAC_IMP("OCS NOW CLEAN: Adder[%d]  Measured[%d] Previous[%d] Adjusted[%d]",
                           g_wof->vdd_oc_ceff_add,
                           i_ceff_ratio,
                           g_wof->vdd_ceff_ratio_adj_prev,
                           l_clipped_ratio);
        }

    }
    else if(g_wof->ocs_dirty == (OCS_PGPE_DIRTY_MASK | OCS_PGPE_DIRTY_TYPE_MASK)) // dirty type 1 (act)
    {
        INCREMENT_ERR_HISTORY( ERRH_CEFF_RATIO_VDD_EXCURSION );

        // over current condition detected on previous PGPE tick time
        // increase OC CeffRatio addr stop at max
        g_wof->vdd_oc_ceff_add += g_wof->ocs_increase_ceff;

        if(g_wof->vdd_oc_ceff_add > G_max_ceff_ratio)
        {
            g_wof->vdd_oc_ceff_add = G_max_ceff_ratio;
        }

        // Calculate adjusted Ceff Ratio for new and previous
        // Add the full accumulated adder to the new measured ceff
        l_new_ratio_from_measured = i_ceff_ratio + g_wof->vdd_oc_ceff_add;

        // only add one ceff adder to the previous
        l_new_ratio_from_prev = g_wof->vdd_ceff_ratio_adj_prev + g_wof->ocs_increase_ceff;

        // use the max for the new Ceff Ratio
        if(l_new_ratio_from_measured > l_new_ratio_from_prev)
        {
            l_clipped_ratio = l_new_ratio_from_measured;
        }
        else
        {
            l_clipped_ratio = l_new_ratio_from_prev;
        }

        if(l_clipped_ratio > G_max_ceff_ratio)
           l_clipped_ratio = G_max_ceff_ratio;

        if( (L_ocs_dirty_prev != g_wof->ocs_dirty) &&
            (G_allow_trace_flags & ALLOW_WOF_OCS_TRACE) )
        {
           INTR_TRAC_IMP("OCS DIRTY ACT TYPE: Measured[%d] Previous[%d] Adjusted[%d]",
                           i_ceff_ratio,
                           g_wof->vdd_ceff_ratio_adj_prev,
                           l_clipped_ratio);
        }

    }
    else // OCS Dirty but type is 0 (block action)
    {
        INCREMENT_ERR_HISTORY(ERRH_OCS_DIRTY_BLOCK);

        // over current condition detected on previous PGPE tick time
        // but type is block action so no adjustment to the new measured or addr
        // default to use max of new and previous but command allows switching to
        // test alg always using new
        if(g_wof->vdd_ceff_ratio_adj_prev > l_clipped_ratio)
        {
            l_clipped_ratio = g_wof->vdd_ceff_ratio_adj_prev;

            if(l_clipped_ratio > G_max_ceff_ratio)
               l_clipped_ratio = G_max_ceff_ratio;
        }
        if(G_internal_flags & INT_FLAG_ENABLE_OCS_HOLD_NEW)  // debug enabled to use new?
            l_clipped_ratio = i_ceff_ratio;

        if( (L_ocs_dirty_prev != g_wof->ocs_dirty) &&
            (G_allow_trace_flags & ALLOW_WOF_OCS_TRACE) )
        {
           INTR_TRAC_IMP("OCS DIRTY HOLD TYPE: Measured[%d] Previous[%d] Using[%d]",
                           i_ceff_ratio,
                           g_wof->vdd_ceff_ratio_adj_prev,
                           l_clipped_ratio);
        }
    }

    // save to previous
    g_wof->vdd_ceff_ratio_adj_prev = l_clipped_ratio;
    L_ocs_dirty_prev = g_wof->ocs_dirty;

    // update sensor for calculated CeffRatio addr due to dirty
    sensor_update(AMECSENSOR_PTR(OCS_ADDR), (uint16_t)g_wof->vdd_oc_ceff_add);

    return l_clipped_ratio;
}

/**
 * prevent_oc_wof_off
 *
 * Description: Determines Pstate clip to prevent over-current when WOF is off
 *
 */
void prevent_oc_wof_off( void )
{
    static uint8_t  L_ocs_dirty_prev = 0;
    static uint8_t  L_trace_first_dirty = 5; // number of times to trace a transition to dirty act
           uint16_t l_pstate = 0;
           uint32_t l_steps = 0;
           uint32_t l_steps_in_kHz = 0;
           uint32_t l_freq_kHz = 0;
           bool     l_trace_final_vote = FALSE;

    // check if OC protection when WOF is off was disabled
    if(G_internal_flags & INT_FLAG_DISABLE_OC_WOF_OFF)
        return;

    if((g_wof->ocs_dirty & OCS_PGPE_DIRTY_MASK) == 0)
    {
        // no over current condition detected on previous PGPE tick time
        // decrease Pstate clip (i.e. increase frequency)
        if(g_amec->oc_wof_off.pstate_request > g_amec->oc_wof_off.decrease_pstate)
        {
             g_amec->oc_wof_off.pstate_request -= g_amec->oc_wof_off.decrease_pstate;
        }
        else  // no clip needed
        {
             g_amec->oc_wof_off.pstate_request = 0;
        }

        if( (L_ocs_dirty_prev != g_wof->ocs_dirty) &&
            (G_allow_trace_flags & ALLOW_WOF_OCS_TRACE) )
        {
           INTR_TRAC_IMP("OCS NOW CLEAN: Pstate Request[0x%02X]",
                          g_amec->oc_wof_off.pstate_request);
        }
    }
    else if(g_wof->ocs_dirty == (OCS_PGPE_DIRTY_MASK | OCS_PGPE_DIRTY_TYPE_MASK)) // dirty type 1 (act)
    {
        INCREMENT_ERR_HISTORY( ERRH_CEFF_RATIO_VDD_EXCURSION );

        // over current condition detected on previous PGPE tick time
        // if there was previously no clip, set Pstate to frequency when dirty occurred
        if(g_amec->oc_wof_off.pstate_request == 0)
        {
             pgpe_wof_values_t l_PgpeWofValues;
             l_PgpeWofValues.dw0.value = in64(g_amec_sys.static_wof_data.pgpe_values_sram_addr);
             l_PgpeWofValues.dw2.value = in64(g_amec_sys.static_wof_data.pgpe_values_sram_addr + 0x10);

             g_amec->oc_wof_off.pstate_request = l_PgpeWofValues.dw2.fields.dirty_pstate_inst;

             if( (L_trace_first_dirty) || (G_allow_trace_flags & ALLOW_WOF_OCS_TRACE) )
             {
                L_trace_first_dirty--;
                INTR_TRAC_IMP("OCS DIRTY ACT DETECTED: Dirty Pstate Inst[0x%02X] Avg Pstate[0x%02X]",
                               l_PgpeWofValues.dw2.fields.dirty_pstate_inst,
                               l_PgpeWofValues.dw0.fields.average_frequency_pstate);
		l_trace_final_vote = TRUE;
             }
        }
        // increase Pstate clip (i.e. decrease frequency) stop at max allowed
        l_pstate = g_amec->oc_wof_off.pstate_request + g_amec->oc_wof_off.increase_pstate;

        if(l_pstate > g_amec->oc_wof_off.pstate_max)
        {
            g_amec->oc_wof_off.pstate_request = g_amec->oc_wof_off.pstate_max;
        }
        else
        {
            g_amec->oc_wof_off.pstate_request = (Pstate_t)l_pstate;
        }

        if( (l_trace_final_vote) ||
            ( (L_ocs_dirty_prev != g_wof->ocs_dirty) &&
              (G_allow_trace_flags & ALLOW_WOF_OCS_TRACE) ) )
        {
           INTR_TRAC_IMP("OCS DIRTY ACT TYPE: Pstate vote[0x%02X]",
                          g_amec->oc_wof_off.pstate_request);
        }

    }
    else // OCS Dirty but type is 0 (block action)
    {
        INCREMENT_ERR_HISTORY(ERRH_OCS_DIRTY_BLOCK);

        // over current condition detected on previous PGPE tick time
        // but type is block action so no adjustment
        if( (L_ocs_dirty_prev != g_wof->ocs_dirty) &&
            (G_allow_trace_flags & ALLOW_WOF_OCS_TRACE) )
        {
           INTR_TRAC_IMP("OCS DIRTY HOLD TYPE: Pstate vote[0x%02X]",
                          g_amec->oc_wof_off.pstate_request);
        }
    }

    // update frequency vote
    l_freq_kHz = proc_pstate2freq(g_amec->oc_wof_off.pstate_request, &l_steps);
    // convert l_steps into frequency in kHz
    l_steps_in_kHz = l_steps * G_oppb.frequency_step_khz;
    // subtract off the additional frequency to get into throttle space
    if(l_steps_in_kHz <= l_freq_kHz)  // prevent going negative
        l_freq_kHz -= l_steps_in_kHz;
    else
        l_freq_kHz = 1000;  // vote 1mHz

    // save in MHz
    g_amec->oc_wof_off.freq_request = (l_freq_kHz / 1000);

    // save dirty to previous
    L_ocs_dirty_prev = g_wof->ocs_dirty;

    return;
}

/**
 * interpolate_ambient_vrt
 *
 * Description: Helper function that takes 2 VRTs and interpolates a new VRT
 *              based on ambient condition and writes new VRT to given ping/pong buffer
 *              NOTE: The VRT address must be the start address of the first VRT
 *                    and the 2nd VRT must be right after.  If we selected the last
 *                    VRT based on ambient condition we should NOT be calling this as
 *                    no interpolation is needed.
 *              NO TRACES!  This is called in interrupt
 */
void interpolate_ambient_vrt(uint8_t * i_ping_pong_buffer_address,
                             uint8_t * i_vrt_address)
{
    VRT_t l_new_vrt = {{{0}}};
    VRT_t *l_vrt1_ptr = (VRT_t *)i_vrt_address;
    VRT_t *l_vrt2_ptr = (VRT_t *)(i_vrt_address + g_amec_sys.static_wof_data.wof_header.vrt_block_size);
    int i;
    uint16_t l_ac1 = 0;
    uint16_t l_ac2 = 0;

    // set VRT header to header of first VRT
    l_new_vrt.vrtHeader.value = l_vrt1_ptr->vrtHeader.value;
    // set msb in ambient condition index (5b field) to indicate interpolated VRT
    l_new_vrt.vrtHeader.fields.ac_id |= 0x10;

    // calculate ambient condition for 1st VRT to interpolate
    l_ac1 = g_amec_sys.static_wof_data.wof_header.amb_cond_start +
           (g_wof->ambient_step_from_start * g_amec_sys.static_wof_data.wof_header.amb_cond_step);

    // second VRT ambient condition is one step size more
    l_ac2 = l_ac1 + g_amec_sys.static_wof_data.wof_header.amb_cond_step;

    // interpolate each VRT entry Pstate and round up (i.e. lower freq) to be safe
    for(i = 0; i < WOF_VRT_SIZE; i++)
    {
        l_new_vrt.data[i] = interpolate_linear(g_wof->ambient_condition,
                                               l_ac1,
                                               l_ac2,
                                               l_vrt1_ptr->data[i],
                                               l_vrt2_ptr->data[i],
                                               TRUE);  // round up
    }

    // Copy the new VRT into the given ping/pong buffer
    memcpy( i_ping_pong_buffer_address,
            &l_new_vrt,
            g_amec_sys.static_wof_data.wof_header.vrt_block_size );

    return;
}


