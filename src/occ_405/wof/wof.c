/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/wof/wof.c $                                       */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2016,2018                        */
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
#include <p9_pstates_occ.h>
#include <occ_service_codes.h>
#include <wof_service_codes.h>
#include <amec_sys.h>
#include <occ_sys_config.h>
#include <wof.h>
#include <amec_freq.h>
#include <pgpe_interface.h>
#include <avsbus.h>
#include "common.h"             // For ignore_pgpe_error()
//******************************************************************************
// External Globals
//******************************************************************************
extern amec_sys_t g_amec_sys;
extern OCCPstateParmBlock G_oppb;
extern GPE_BUFFER(ipcmsg_wof_vfrt_t G_wof_vfrt_parms);
extern GPE_BUFFER(ipcmsg_wof_control_t G_wof_control_parms);
extern GpeRequest   G_wof_vfrt_req;
extern GpeRequest   G_wof_control_req;
extern uint32_t     G_nest_frequency_mhz;
extern volatile pstateStatus G_proc_pstate_status;
extern uint8_t G_occ_interrupt_type;
extern bool    G_pgpe_shared_sram_V_I_readings;
extern uint16_t G_allow_trace_flags;

//******************************************************************************
// Globals
//******************************************************************************
uint8_t G_sram_vfrt_ping_buffer[MIN_BCE_REQ_SIZE] __attribute__ ((section(".vfrt_ping_buffer")));
uint8_t G_sram_vfrt_pong_buffer[MIN_BCE_REQ_SIZE] __attribute__ ((section(".vfrt_pong_buffer")));

// BCE Request object for retrieving VFRT's from Mainstore
BceRequest G_vfrt_req;

// Buffer to hold vfrt from main memory
DMA_BUFFER(temp_bce_request_buffer_t G_vfrt_temp_buff) = {{0}};

// Wof header struct
wof_header_data_t G_wof_header __attribute__ ((section (".global_data")));

// Quad state structs to temporarily hold the data from the doublewords to
// then populate in amec structure
quad_state0_t G_quad_state_0 = {0};
quad_state1_t G_quad_state_1 = {0};

// Create a pointer to amec WOF structure
amec_wof_t * g_wof = &(g_amec_sys.wof);

// Core IDDQ voltages array (voltages in 100uV)
uint16_t G_iddq_voltages[CORE_IDDQ_MEASUREMENTS] =
{
     6000,
     7000,
     8000,
     9000,
    10000,
    11000
};

// Approximate y = full_leakage_08V^(-((T-tvpd_leak)/257.731))*1.45^((T-tvpd_leak)/10)
// full_leakage_08V is not data we have, it is a ALL core,cache,quad ON leakage measure they do at MFT.
// We can estimate by using the IQ data at 0.8v * 24/#sort cores

// Interpolate (T-tvpd_leak) for full leakage 0.8V in the table below to find m.
// y ~= (T*m) >> 10     (shift out 10 bits)
// Error in estimation is no more than 0.9%
// The first column represents the result of T-tvpd_leak where T is the
// associated temperature sensor.
// The second column represents the associated m(slope) with the delta temp (first column)
#define NUM_FULL_LEAKAGE_08V 10
#define WOF_IDDQ_MULT_TABLE_N 21
uint32_t G_wof_mft_full_leakage_08V[WOF_IDDQ_MULT_TABLE_N + 1][NUM_FULL_LEAKAGE_08V] = {
    // First row is header of voltage values in mA remaining rows are m values
    // for the full leakage @0.8V for each temperature in first column of G_wof_iddq_mult_table
    // this table is used for one time interpolation to create the final m values in
    // G_wof_iddq_mult_table (the temperatures are not repeated in this table)
    {20000, 30000, 40000, 50000, 60000, 70000, 80000, 90000, 100000, 110000}, // header in mA
    {  171,   191,   207,   220,   231,   241,   250,   258,    265,    272}, // -70 temp delta
    {  195,   216,   232,   245,   257,   267,   276,   285,    292,    299}, // -65 temp delta
    {  221,   243,   260,   274,   286,   296,   306,   314,    322,    329}, // -60 temp delta
    {  251,   274,   292,   306,   318,   328,   338,   347,    354,    362}, // -55 temp delta
    {  286,   309,   327,   341,   354,   364,   374,   382,    390,    398}, // -50 temp delta
    {  325,   348,   366,   381,   393,   404,   413,   422,    430,    437}, // -45 temp delta
    {  369,   393,   411,   425,   437,   448,   457,   466,    473,    480}, // -40 temp delta
    {  419,   443,   460,   474,   486,   497,   506,   514,    521,    528}, // -35 temp delta
    {  476,   499,   516,   530,   541,   551,   559,   567,    574,    581}, // -30 temp delta
    {  541,   563,   578,   591,   602,   611,   619,   626,    632,    638}, // -25 temp delta
    {  615,   634,   648,   660,   669,   677,   684,   691,    696,    701}, // -20 temp delta
    {  698,   715,   727,   736,   744,   751,   757,   762,    767,    771}, // -15 temp delta
    {  793,   806,   815,   822,   828,   833,   837,   841,    844,    847}, // -10 temp delta
    {  901,   908,   913,   917,   921,   923,   926,   928,    930,    932}, // -5 temp delta
    { 1024,  1024,  1024,  1024,  1024,  1024,  1024,  1024,   1024,   1024}, //  0 temp delta
    { 1163,  1154,  1148,  1143,  1139,  1136,  1133,  1130,   1128,   1126}, //  5 temp delta
    { 1322,  1301,  1287,  1276,  1267,  1259,  1253,  1247,   1242,   1237}, //  10 temp delta
    { 1502,  1467,  1442,  1424,  1409,  1396,  1385,  1376,   1368,   1360}, //  15 temp delta
    { 1706,  1654,  1617,  1589,  1567,  1548,  1532,  1518,   1506,   1495}, //  20 temp delta
    { 1939,  1864,  1813,  1774,  1743,  1717,  1695,  1676,   1659,   1643}, //  25 temp delta
    { 2203,  2101,  2032,  1980,  1938,  1904,  1874,  1849,   1826,   1806}  //  30 temp delta
};

// P9':  The 2nd column (m values) are just initial values and will be updated based on
// full leakage @0.8V (a one time calculation).  P9 will keep the same values.
int16_t G_wof_iddq_mult_table[WOF_IDDQ_MULT_TABLE_N][2] = {
    {-70, 163},
    {-65, 186},
    {-60, 212},
    {-55, 242},
    {-50, 276},
    {-45, 314},
    {-40, 359},
    {-35, 409},
    {-30, 466},
    {-25, 531},
    {-20, 606},
    {-15, 691},
    {-10, 788},
    {-5,  898},
    {0,   1024},
    {5,   1168},
    {10,  1331},
    {15,  1518},
    {20,  1731},
    {25,  1973},
    {30,  2250}
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
    // Timeout for VFRT to complete
    static uint8_t L_vfrt_last_chance = MAX_VFRT_CHANCES_EVERY_TICK;

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
        if( g_wof->vfrt_callback_error )
        {
            INTR_TRAC_ERR("Got a bad RC in wof_vfrt_callback: 0x%x",
                    g_wof->wof_vfrt_req_rc);
            set_clear_wof_disabled( SET,
                                    WOF_RC_VFRT_REQ_FAILURE,
                                    ERC_WOF_VFRT_REQ_FAILURE );

            // After official error recorded, prevent this code
            // from running from same setting of the var.
            g_wof->vfrt_callback_error = 0;
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
                        // Reset timeouts for VFRT response and WOF control
                        L_vfrt_last_chance = MAX_VFRT_CHANCES;
                        L_wof_control_last_chance = MAX_WOF_CONTROL_CHANCES;

                        // reset OC ceff adder
                        g_wof->vdd_oc_ceff_add = 0;
                        sensor_update(AMECSENSOR_PTR(OCS_ADDR), (uint16_t)g_wof->vdd_oc_ceff_add);

                        // calculate initial vfrt, send gpeRequest
                        // Initial vfrt is the last vfrt in Main memory
                        send_initial_vfrt_to_pgpe();
                        break;

                    case INITIAL_VFRT_SENT_WAITING:
                        // Check if request is still processing.
                        // Init state updated in wof_vfrt_callback
                        if( (!async_request_is_idle(&G_wof_vfrt_req.request)) ||
                             (g_wof->vfrt_state != STANDBY) )
                        {
                            if( (L_vfrt_last_chance == 0) && (!ignore_pgpe_error()) )
                            {
                                INTR_TRAC_ERR("WOF Disabled!"
                                              " Init VFRT request timeout");
                                set_clear_wof_disabled( SET,
                                                        WOF_RC_VFRT_REQ_TIMEOUT,
                                                        ERC_WOF_VFRT_REQ_TIMEOUT );
                            }
                            else if(L_vfrt_last_chance != 0)
                            {
                                if( L_vfrt_last_chance == 1 )
                                {
                                    INTR_TRAC_INFO("initial VFRT NOT idle. Last chance out of %d chances",
                                                    MAX_VFRT_CHANCES);
                                }
                                L_vfrt_last_chance--;
                            }
                            else
                            {
                                // Wait forever for PGPE to respond
                                // Put a mark on the wall so we know we hit this state
                                if(!L_current_timeout_recorded)
                                {
                                    INCREMENT_ERR_HISTORY(ERRH_VFRT_TIMEOUT_IGNORED);
                                    L_current_timeout_recorded = TRUE;
                                }
                            }
                        }
                        break;

                    case INITIAL_VFRT_SUCCESS:
                        // We made it this far. Reset Last chance
                        L_vfrt_last_chance = MAX_VFRT_CHANCES;

                        // Send wof control on gpe request
                        // If enable_success returns true, init state was set
                        enable_success = enable_wof();
                        if( !enable_success )
                        {
                            // Treat as an error only if not currently ignoring PGPE failures
                            if( (L_wof_control_last_chance == 0) && (!ignore_pgpe_error()) )
                            {
                                INTR_TRAC_ERR("WOF Disabled! Control req timeout(1)");
                                set_clear_wof_disabled(SET,
                                                       WOF_RC_CONTROL_REQ_TIMEOUT,
                                                       ERC_WOF_CONTROL_REQ_TIMEOUT);
                            }
                            else if(L_wof_control_last_chance != 0)
                            {
                                if(L_wof_control_last_chance == 1 )
                                {
                                    INTR_TRAC_ERR("Last chance for WOF control request(1) out of %d chances ",
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
                                INTR_TRAC_ERR("WOF Disabled! Control req timeout(2)");
                                set_clear_wof_disabled(SET,
                                                       WOF_RC_CONTROL_REQ_TIMEOUT,
                                                       ERC_WOF_CONTROL_REQ_TIMEOUT );
                            }
                            else if(L_wof_control_last_chance != 0)
                            {
                                if(L_wof_control_last_chance == 1 )
                                {
                                    INTR_TRAC_ERR("Last chance for WOF control request(2) out of %d chances ",
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
                if( (!async_request_is_idle(&G_wof_vfrt_req.request)) ||
                    (g_wof->vfrt_state != STANDBY) )
                {
                    if( L_vfrt_last_chance == 0 )
                    {
                        // Treat as an error only if not currently ignoring PGPE failures
                        if(!ignore_pgpe_error())
                        {
                            INTR_TRAC_ERR("WOF Disabled! VFRT req timeout");
                            set_clear_wof_disabled(SET,
                                                   WOF_RC_VFRT_REQ_TIMEOUT,
                                                   ERC_WOF_VFRT_REQ_TIMEOUT);
                        }
                        else
                        {
                            // Wait forever for PGPE to respond
                            // Put a mark on the wall so we know we hit this state
                            if(!L_current_timeout_recorded)
                            {
                                INCREMENT_ERR_HISTORY(ERRH_VFRT_TIMEOUT_IGNORED);
                                L_current_timeout_recorded = TRUE;
                            }
                        }
                    }
                    else
                    {
                        if( L_vfrt_last_chance == 1 )
                        {
                            INTR_TRAC_INFO("VFRT NOT idle. Last chance out of %d chances",
                                            MAX_VFRT_CHANCES);
                        }
                        L_vfrt_last_chance--;
                    }
                }
                else
                {
                    L_current_timeout_recorded = FALSE;

                    // Request is idle. Run wof algorithm
                    wof_main();

                    L_vfrt_last_chance = MAX_VFRT_CHANCES;
                    // Finally make sure we are in the fully enabled state
                    if( g_wof->wof_init_state == PGPE_WOF_ENABLED_NO_PREV_DATA )
                    {
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
    // Some sensors may be updated from PGPE data so we must read PGPE data before reading sensors
    // Read out PGPE data from shared SRAM
    read_shared_sram();

    // Read out the sensor data needed for calculations
    read_sensor_data();

    // Calculate the core voltage per quad
    calculate_core_voltage();

    // Calculate the core leakage across the entire Proc
    calculate_core_leakage();

    // Calculate the nest leakage for the entire system
    calculate_nest_leakage();

    // Calculate the AC currents
    calculate_AC_currents();

    // Calculate ceff_ratio_vdd and ceff_ratio_vdn
    calculate_ceff_ratio_vdd();
    calculate_ceff_ratio_vdn();

    // Calculate how many steps from the beginning for VDD, VDN, and active quads
    g_wof->vdn_step_from_start =
                             calculate_step_from_start( g_wof->ceff_ratio_vdn,
                                                        g_wof->vdn_step,
                                                        g_wof->vdn_start,
                                                        g_wof->vdn_size );

    g_wof->vdd_step_from_start =
                             calculate_step_from_start( g_wof->ceff_ratio_vdd,
                                                        g_wof->vdd_step,
                                                        g_wof->vdd_start,
                                                        g_wof->vdd_size );

    g_wof->quad_step_from_start = calc_quad_step_from_start();

    // Compute the Main Memory address of the desired VFRT table given
    // the calculated VDN, VDD, and Quad steps
    g_wof->next_vfrt_main_mem_addr = calc_vfrt_mainstore_addr();


    // Send the new vfrt to the PGPE
    send_vfrt_to_pgpe( g_wof->next_vfrt_main_mem_addr );
}

/**
 * calculate_step_from_start
 *
 * Description: Calculates the step number for the current VDN/VDD
 *
 * Param[in]: i_ceff_vdx_ratio - The current Ceff_vdd or Ceff_vdn_ratio
 *                               to calculate the step for.
 * Param[in]: i_step_size - The size of each step.
 * Param[in]: i_min_ceff - The minimum step number for this VDN/VDD
 * Param[in]: i_max_step - The maximum step number for this VDN/VDD
 *
 * Return: The calculated step for current Ceff_vdd/Ceff_vdn
 */
uint16_t calculate_step_from_start(uint16_t i_ceff_vdx_ratio,
                                   uint16_t i_step_size,
                                   uint16_t i_min_ceff,
                                   uint16_t i_max_step )
{
    uint16_t l_current_step;

    // Ensure ceff is at least the min step
    if( (i_ceff_vdx_ratio <= i_min_ceff) || (i_step_size == 0) )
    {
        l_current_step = 0;
    }
    else
    {
        // Add step size to current vdd/vdn to round up.
        //  -1 to prevent overshoot when i_ceff_vdx_ratio is equal to Ceff table value
        l_current_step = i_ceff_vdx_ratio + i_step_size - 1;

        // Subtract the starting ceff to skip the 0 table entry
        l_current_step -= i_min_ceff;

        // Divide by step size to determine how many from the 0 entry ceff is
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
 * calc_quad_step_from_start
 *
 * Description: Calculates the step number for the current number
 *              of active quads
 *
 * Return: The calculated step for current active quads
 */
uint8_t calc_quad_step_from_start( void )
{
    return (G_wof_header.active_quads_size == ACTIVE_QUAD_SZ_MIN) ? 0 :
                                           (g_wof->num_active_quads - 1);
}

/**
 * calc_vfrt_mainstore_address
 *
 * Description: Calculates the VFRT address based on the Ceff vdd/vdn and quad
 *              steps.
 *
 * Return: The desired VFRT main memory address
 */
uint32_t calc_vfrt_mainstore_addr( void )
{
    static bool L_trace_char_test = true;

    // skip calculation and return first table if WOF Char testing is enabled
    if(G_internal_flags & INT_FLAG_ENABLE_WOF_CHAR_TEST)
    {
       if(L_trace_char_test)
       {
           INTR_TRAC_IMP("Entered WOF char testing using first VRT!");
           L_trace_char_test = false;
       }

       g_wof->vfrt_mm_offset = 0;
    }
    else
    {
       if(!L_trace_char_test)
       {
           INTR_TRAC_IMP("Exited WOF char testing calculating VRT!");
           L_trace_char_test = true;
       }

       // Wof tables address calculation
       // (Base_addr +
       // (sizeof VFRT * (total active quads * ( (g_wof->vdn_step_from_start * vdd_size) + (g_wof->vdd_step_from_start) ) + (g_wof->quad_step_from_start))))
       g_wof->vfrt_mm_offset = g_wof->vfrt_block_size *
                       (( g_wof->active_quads_size *
                       ((g_wof->vdn_step_from_start * g_wof->vdd_size) +
                       g_wof->vdd_step_from_start) ) + g_wof->quad_step_from_start);
    }

    // Skip the wof header at the beginning of wof tables
    uint32_t wof_tables_base = g_wof->vfrt_tbls_main_mem_addr + WOF_HEADER_SIZE;

    return wof_tables_base + g_wof->vfrt_mm_offset;
}


/**
 * copy_vfrt_to_sram_callback
 *
 * Description: Call back function to BCE request to copy VFRT into SRAM
 *              ping/pong buffer. This call will also tell the PGPE
 *              that a new VFRT is available
 *
 * Param[in]: i_parms - pointer to a struct that will hold data necessary to
 *                      the calculation.
 *                      -Pointer to vfrt table temp buffer
 */
void copy_vfrt_to_sram_callback( void )
{
/*
 *
 * find out which ping pong buffer to use
 * copy the vfrt to said ping pong buffer
 * save current vfrt address to global
 * send IPC command to pgpe to notify of new ping/pong vfrt address
 */
    // Static variable to trac which buffer is open for use
    // 0 = PING; 1 = PONG;
    uint8_t * l_buffer_address = G_sram_vfrt_ping_buffer;
    if(g_wof->curr_ping_pong_buf == (uint32_t)G_sram_vfrt_ping_buffer)
    {
        // Switch to pong buffer
        l_buffer_address = G_sram_vfrt_pong_buffer;
    }
    else
    {
        // Switch to ping buffer
        l_buffer_address = G_sram_vfrt_ping_buffer;
    }
    // Update global "next" ping pong buffer for callback function
    g_wof->next_ping_pong_buf = (uint32_t)l_buffer_address;

    // Copy the vfrt data into the buffer
    memcpy( l_buffer_address,
            &G_vfrt_temp_buff,
            g_wof->vfrt_block_size );

    // Set the parameters for the GpeRequest
    G_wof_vfrt_parms.homer_vfrt_ptr = (HomerVFRTLayout_t*)l_buffer_address;
    G_wof_vfrt_parms.active_quads = g_wof->req_active_quad_update;

    if( g_wof->vfrt_state != STANDBY )
    {
        // Set vfrt state to let OCC know it needs to schedule the IPC command
        g_wof->vfrt_state = NEED_TO_SCHEDULE;
    }

}

/**
 * wof_vfrt_callback
 *
 * Description: Callback function for G_wof_vfrt_req GPE request to
 *              confirm the new VFRT is being used by the PGPE and
 *              record the switch on the 405. Also updates the
 *              initialization
 */
void wof_vfrt_callback( void )
{
    // Update the VFRT state to indicate a new IPC message can be
    // scheduled regardless of the RC of the previous one.
    g_wof->vfrt_state = STANDBY;

    // Confirm the WOF VFRT PGPE request has completed with no errors
    if( G_wof_vfrt_parms.msg_cb.rc == PGPE_WOF_RC_VFRT_QUAD_MISMATCH )
    {
        // Rereading OCC-SRAM to update requested active quads
        read_req_active_quads();
    }
    else if( G_wof_vfrt_parms.msg_cb.rc == PGPE_RC_SUCCESS )
    {
       // GpeRequest went through successfully. update global ping pong buffer
       g_wof->curr_ping_pong_buf = g_wof->next_ping_pong_buf;

       // Update previous active quads
       g_wof->prev_req_active_quads = g_wof->req_active_quad_update;

       // Update current vfrt_main_mem_address
       g_wof->curr_vfrt_main_mem_addr = g_wof->next_vfrt_main_mem_addr;

       // Update the wof_init_state based off the current state
       if( g_wof->wof_init_state == INITIAL_VFRT_SENT_WAITING )
       {
            g_wof->wof_init_state = INITIAL_VFRT_SUCCESS;
       }
    }
    else
    {
        // Disable WOF
        g_wof->vfrt_callback_error = 1;
        g_wof->wof_vfrt_req_rc = G_wof_vfrt_parms.msg_cb.rc;
    }
}

/**
 * send_vfrt_to_pgpe
 *
 * Description: Function to copy new VFRT from Mainstore to local SRAM buffer
 *              and calls copy_vfrt_to_sram_callback function to send new VFRT
 *              to the PGPE
 *              Note: If desired VFRT is the same as previous, skip.
 *
 * Param[in]: i_vfrt_main_mem_addr - Address of the desired vfrt table.
 */
void send_vfrt_to_pgpe( uint32_t i_vfrt_main_mem_addr )
{
    int l_ssxrc = SSX_OK;

    do
    {
        // First check if the address is 128-byte aligned. error if not.
        if( i_vfrt_main_mem_addr % 128 )
        {
            INTR_TRAC_ERR("VFRT Main Memory address NOT 128-byte aligned:"
                    " 0x%08x", i_vfrt_main_mem_addr);
            set_clear_wof_disabled(SET,
                                   WOF_RC_VFRT_ALIGNMENT_ERROR,
                                   ERC_WOF_VFRT_ALIGNMENT_ERROR);

            break;
        }

        // Check if PGPE explicitely requested a new vfrt
        ocb_occflg_t occ_flags = {0};
        occ_flags.value = in32(OCB_OCCFLG);

        if( ((i_vfrt_main_mem_addr == g_wof->curr_vfrt_main_mem_addr ) &&
            (g_wof->req_active_quad_update ==
             g_wof->prev_req_active_quads)) &&
            (!occ_flags.fields.active_quad_update) )
        {
            // VFRT and requested active quads are unchanged.
            break;
        }
        // Either the Main memory address changed or req active quads changed
        // get VFRT based on new values
        else
        {

            // Create request
            l_ssxrc = bce_request_create(
                             &G_vfrt_req,                 // block copy object
                             &G_pba_bcde_queue,           // main to sram copy engine
                             i_vfrt_main_mem_addr,     //mainstore address
                             (uint32_t) &G_vfrt_temp_buff, // SRAM start address
                             MIN_BCE_REQ_SIZE,  // size of copy
                             SSX_WAIT_FOREVER,            // no timeout
                             (AsyncRequestCallback)copy_vfrt_to_sram_callback,
                             NULL,
                             ASYNC_CALLBACK_IMMEDIATE );

            if(l_ssxrc != SSX_OK)
            {
                INTR_TRAC_ERR("send_vfrt_to_pgpe: BCDE request create failure rc=[%08X]", -l_ssxrc);
                break;
            }

            // Make sure we are in correct vfrt state
            if( g_wof->vfrt_state == STANDBY )
            {
                // Set the VFRT state to ensure asynchronous order of operations
                g_wof->vfrt_state = SEND_INIT;

                // Do the actual copy
                l_ssxrc = bce_request_schedule( &G_vfrt_req );

                if(l_ssxrc != SSX_OK)
                {
                    INTR_TRAC_ERR("send_vfrt_to_pgpe: BCE request schedule failure rc=[%08X]", -l_ssxrc);
                    break;
                }
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
 * read_shared_sram
 *
 * Description: Read out data from OCC-PGPE shared SRAM and saves the
 *              data for the current iteration of the WOF algorithm
 */
void read_shared_sram( void )
{
    // Get the actual quad states
    G_quad_state_0.value = in64(g_wof->quad_state_0_addr);
    G_quad_state_1.value = in64(g_wof->quad_state_1_addr);

    // Read f_clip, v_clip, f_ratio, and v_ratio
    pgpe_wof_state_t l_wofstate;
    l_wofstate.value = in64(g_wof->pgpe_wof_state_addr);

    g_wof->f_clip_ps  = l_wofstate.fields.fclip_ps;

    g_wof->v_clip  = l_wofstate.fields.vclip_mv;

   // Update Fclip Freq and Vratio for P9 only. P9 prime uses values updated in read_pgpe_produced_wof_values()
   if( (!G_pgpe_shared_sram_V_I_readings) ||
       (G_internal_flags & INT_FLAG_DISABLE_CEFF_TRACKING) )
   {
       g_wof->v_ratio = l_wofstate.fields.vratio;
       sensor_update(AMECSENSOR_PTR(VRATIO), (uint16_t)l_wofstate.fields.vratio);

       // convert f_clip_ps from Pstate to frequency(mHz)
       g_wof->f_clip_freq = (proc_pstate2freq(g_wof->f_clip_ps))/1000;
   }

    g_wof->f_ratio = 1;

    // the PGPE produced WOF values were already read in this tick from amec_update_avsbus_sensors()
    // required to read them in every tick regardless of WOF running so voltage and current sensors
    // are updated even when WOF isn't running

    // Get the requested active quad update
    read_req_active_quads();

    // merge the 16-bit active_cores field from quad state 0 and the 16-bit
    // active_cores field from quad state 1 and save it to amec.
    g_wof->core_pwr_on =
                   (((uint32_t)G_quad_state_0.fields.active_cores) << 16)
                  | ((uint32_t)G_quad_state_1.fields.active_cores);

    // Clear out current quad pstates
    memset(g_wof->quad_x_pstates, 0 , MAXIMUM_QUADS);

    // Add the quad states to the global quad state array for easy looping.
    g_wof->quad_x_pstates[0] = (uint8_t)G_quad_state_0.fields.quad0_pstate;
    g_wof->quad_x_pstates[1] = (uint8_t)G_quad_state_0.fields.quad1_pstate;
    g_wof->quad_x_pstates[2] = (uint8_t)G_quad_state_0.fields.quad2_pstate;
    g_wof->quad_x_pstates[3] = (uint8_t)G_quad_state_0.fields.quad3_pstate;
    g_wof->quad_x_pstates[4] = (uint8_t)G_quad_state_1.fields.quad4_pstate;
    g_wof->quad_x_pstates[5] = (uint8_t)G_quad_state_1.fields.quad5_pstate;

    // Save IVRM bit vector states to amec
    // NOTE: the ivrm_state field in both quad state 0 and quad state 1
    //       double words should contain the same data.
    g_wof->quad_ivrm_states =
           (((uint8_t)G_quad_state_0.fields.ivrm_state) << 4)
          | ((uint8_t)G_quad_state_1.fields.ivrm_state);
}

/**
 * read_pgpe_produced_wof_values
 *
 * Description: Read the PGPE Produced WOF values from OCC-PGPE shared SRAM and
 *              update sensors
 */
void read_pgpe_produced_wof_values( void )
{
    // Read in OCS bits from OCC Flag 0 register
    uint32_t occ_flags0 = 0;
    occ_flags0 = in32(OCB_OCCFLG);
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

    // Update V/I from PGPE
    uint16_t l_voltage = 0;
    uint16_t l_current = 0;
    uint32_t l_freq = 0;

    pgpe_wof_values_t l_PgpeWofValues;
    l_PgpeWofValues.dw0.value = in64(G_pgpe_header.pgpe_produced_wof_values_addr);
    l_PgpeWofValues.dw1.value = in64(G_pgpe_header.pgpe_produced_wof_values_addr + 0x08);
    l_PgpeWofValues.dw2.value = in64(G_pgpe_header.pgpe_produced_wof_values_addr + 0x10);
    l_PgpeWofValues.dw3.value = in64(G_pgpe_header.pgpe_produced_wof_values_addr + 0x18);

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

    // don't use Vdd current from PGPE if it was enabled for OCC to read from AVSbus
    if(!(G_internal_flags & INT_FLAG_ENABLE_VDD_CURRENT_READ))
    {
        // Save Vdd current to sensor
        l_current = (uint16_t)l_PgpeWofValues.dw1.fields.idd_avg_ma;
        if (l_current != 0)
        {
            // Current value stored in the sensor should be in 10mA (A scale -2)
            // Reading from SRAM is already in 10mA
            sensor_update(AMECSENSOR_PTR(CURVDD), l_current);
        }
    }

    // Save Vdn current to sensor
    l_current = (uint16_t)l_PgpeWofValues.dw1.fields.idn_avg_ma;
    if (l_current != 0)
    {
        // Current value stored in the sensor should be in 10mA (A scale -2)
        // Reading from SRAM is already in 10mA
        sensor_update(AMECSENSOR_PTR(CURVDN), l_current);
    }

    // Update the chip voltage and power sensors
    update_avsbus_power_sensors(AVSBUS_VDD);
    update_avsbus_power_sensors(AVSBUS_VDN);

    // populate values used by WOF alg

    // these are only used for Ceff frequency tracking
    if(!(G_internal_flags & INT_FLAG_DISABLE_CEFF_TRACKING))
    {
       // Average Frequency
       l_freq = proc_pstate2freq((Pstate)l_PgpeWofValues.dw0.fields.average_frequency_pstate);
       // value returned in kHz, save in MHz
       g_wof->c_ratio_vdd_freq = (l_freq / 1000);
       g_wof->f_clip_freq = g_wof->c_ratio_vdd_freq;
       g_wof->v_ratio = l_PgpeWofValues.dw0.fields.vratio_avg;
       sensor_update(AMECSENSOR_PTR(VRATIO), (uint16_t)g_wof->v_ratio);
    }

    // save the full PGPE WOF values for debug
    g_wof->pgpe_wof_values_dw0 = l_PgpeWofValues.dw0.value;
    g_wof->pgpe_wof_values_dw1 = l_PgpeWofValues.dw1.value;
    g_wof->pgpe_wof_values_dw2 = l_PgpeWofValues.dw2.value;
    g_wof->pgpe_wof_values_dw3 = l_PgpeWofValues.dw3.value;
}
/**
 * calculate_core_voltage
 *
 * Description: Calculate the core voltage based on Pstate and IVRM state.
 *              Same for all cores in the quad so only need to calculate
 *              once per quad.
 */
void calculate_core_voltage( void )
{
    uint32_t l_voltage;
    uint8_t l_quad_mask;
    int l_quad_idx;
    for(l_quad_idx = 0; l_quad_idx < MAXIMUM_QUADS; l_quad_idx++)
    {
        // Adjust current mask. (IVRM_STATE_QUAD_MASK = 0x80)
        l_quad_mask = IVRM_STATE_QUAD_MASK >> l_quad_idx;

        // Check IVRM state of quad 0.
        // 0 = BYPASS, 1 = REGULATION
        if( (g_wof->quad_ivrm_states & l_quad_mask ) == 0 )
        {
            l_voltage = g_wof->voltvddsense_sensor;
        }
        else
        {
            // Calculate the address of the pstate for the current quad.
            uint32_t pstate_addr = g_wof->pstate_tbl_sram_addr +
                    (g_wof->quad_x_pstates[l_quad_idx] * sizeof(OCCPstateTable_entry_t));

            // Get the Pstate
            OCCPstateTable_entry_t * pstate_entry_ptr;
            uint32_t current_pstate = in32(pstate_addr);
            pstate_entry_ptr = (OCCPstateTable_entry_t *)(&current_pstate);

            // Get the internal vid (ivid) from the pstate table
            uint8_t current_vid = pstate_entry_ptr->internal_vdd_vid;

            // Convert the vid to voltage and then convert units to 100uV
            // Vid-to-voltage = 512mV + ivid*4mV
            // mV to 100uV = mV*10
            l_voltage = (512 + (current_vid*4))*10;
        }
        // Save the voltage to amec_wof_t global struct
        g_wof->v_core_100uV[l_quad_idx] = l_voltage;

        // Save off the voltage index for later use
        g_wof->quad_v_idx[l_quad_idx] = get_voltage_index(l_voltage);
    }
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
    uint16_t l_quad_cache;
    uint16_t idc_vdd = 0;
    uint16_t temperature = 0;

    // Get the core leakage percent from the OPPB
    // Note: This value is named inaccurately in the OPPB so we are reassigning
    // it's value here. We are also making the value visible to amester here.
    g_wof->core_leakage_percent = G_oppb.nest_leakage_percent;

    // Loop through all Quads and their respective Cores to calculate
    // leakage.
    int quad_idx = 0;       // Quad Index (0-5)
    uint8_t core_idx = 0;   // Actual core index (0-23)
    int core_loop_idx = 0;  // On a per quad basis (0-3)

    for(quad_idx = 0; quad_idx < MAXIMUM_QUADS; quad_idx++)
    {
        // Get the voltage for the current core.
        // (Same for all cores within a single quad)
        uint8_t quad_v_idx = g_wof->quad_v_idx[quad_idx];

        // ALL_CORES_OFF_ISO
        g_wof->all_cores_off_iso =
        scale_and_interpolate( G_oppb.iddq.ivdd_all_cores_off_caches_off,
                               G_oppb.iddq.avgtemp_all_cores_off_caches_off,
                               quad_v_idx,
                               g_wof->tempnest_sensor,
                               g_wof->v_core_100uV[quad_idx] );

        g_wof->all_cores_off_before = g_wof->all_cores_off_iso;

        //Multiply by core leakage percentage
        g_wof->all_cores_off_iso = ( g_wof->all_cores_off_iso *
            g_wof->core_leakage_percent ) / 100;

        // Calculate ALL_GOOD_CACHES_ON_ISO
        g_wof->all_good_caches_on_iso =
        scale_and_interpolate( G_oppb.iddq.ivdd_all_good_cores_off_good_caches_on,
                               G_oppb.iddq.avgtemp_all_good_cores_off,
                               quad_v_idx,
                               g_wof->tempnest_sensor,
                               g_wof->v_core_100uV[quad_idx] ) -
                               g_wof->all_cores_off_iso;


        // Calculate ALL_CACHES_OFF_ISO
        g_wof->all_caches_off_iso =
        scale_and_interpolate( G_oppb.iddq.ivdd_all_cores_off_caches_off,
                               G_oppb.iddq.avgtemp_all_cores_off_caches_off,
                               quad_v_idx,
                               g_wof->tempnest_sensor,
                               g_wof->v_core_100uV[quad_idx] ) -
                               g_wof->all_cores_off_iso;

        // idc Quad uses same variables as all_cores_off_iso so just use that and
        // divide by 6 to get just one quad
        g_wof->idc_quad = g_wof->all_caches_off_iso / MAXIMUM_QUADS;

        // Calculate quad_cache for all quads
        l_quad_cache = ( g_wof->all_good_caches_on_iso - g_wof->all_caches_off_iso *
                     ( MAXIMUM_QUADS - G_oppb.iddq.good_quads_per_sort) /
                      MAXIMUM_QUADS) / G_oppb.iddq.good_quads_per_sort;

        if(g_wof->quad_x_pstates[quad_idx] == QUAD_POWERED_OFF)
        {
            // Incorporate quad off into leakage
            idc_vdd += g_wof->idc_quad;

            // Add in 4 cores worth of off leakage
            idc_vdd += g_wof->all_cores_off_iso * NUM_CORES_PER_QUAD /24;
        }
        else // Quad i is on
        {
            // Calculate the index of the first core in the quad.
            // so we reference the correct one in the inner core loop
            core_idx = quad_idx * NUM_CORES_PER_QUAD;

            // Calculate the number of cores on within the current quad.
            g_wof->cores_on_per_quad[quad_idx] =
                                        num_cores_on_in_quad(quad_idx);

            // Take a snap shot of the quad temperature for later calculations
            g_wof->tempq[quad_idx] = AMECSENSOR_ARRAY_PTR(TEMPQ0,
                                                           quad_idx)->sample;
            // If 0, use nest temperature.
            if( g_wof->tempq[quad_idx] == 0 )
            {
                g_wof->tempq[quad_idx] = g_wof->tempnest_sensor;
            }

            // Reset num_cores_off_in_quad before processing current quads cores
            uint8_t num_cores_off_in_quad = 0;

            // Loop all cores within current quad
            for(core_loop_idx = 0; core_loop_idx < NUM_CORES_PER_QUAD; core_loop_idx++)
            {
                if(core_powered_on(core_idx))
                {
                    // Get the core temperature from TEMPPROCTHRMC sensor
                    temperature = AMECSENSOR_ARRAY_PTR(TEMPPROCTHRMC0,
                                                       core_idx)->sample;

                    // If TEMPPROCTHRMCy is 0, use TEMPQx
                    if(temperature == 0)
                    {
                        // Quad temp guaranteed to be non-zero. Check was made
                        // when assigning to tempq array.
                        temperature = g_wof->tempq[quad_idx];
                    }

                    // Save selected temperature
                    g_wof->tempprocthrmc[core_idx] = temperature;

                    // Calculate QUAD_GOOD_CORES_ONLY
                    g_wof->quad_good_cores_only[quad_idx] =
                    scale_and_interpolate
                    (G_oppb.iddq.ivdd_quad_good_cores_on_good_caches_on[quad_idx],
                    G_oppb.iddq.avgtemp_quad_good_cores_on[quad_idx],
                    quad_v_idx,
                    g_wof->tempprocthrmc[core_idx],
                    g_wof->v_core_100uV[quad_idx] )
                    -
                    scale_and_interpolate
                    (G_oppb.iddq.ivdd_all_good_cores_off_good_caches_on,
                    G_oppb.iddq.avgtemp_all_good_cores_off,
                    quad_v_idx,
                    g_wof->tempprocthrmc[core_idx],
                    g_wof->v_core_100uV[quad_idx])
                    +
                    (g_wof->all_cores_off_iso * G_oppb.iddq.good_normal_cores[quad_idx])
                     / 24;

                    // Calculate quad_on_cores[quad]
                    g_wof->quad_on_cores[quad_idx] =
                        g_wof->quad_good_cores_only[quad_idx] /
                        G_oppb.iddq.good_normal_cores[quad_idx];

                    // Add to overall leakage
                    idc_vdd += g_wof->quad_on_cores[quad_idx];
                }
                else // Core is powered off
                {
                    // Increment the number of cores found to be off
                    num_cores_off_in_quad++;
                }
                // Increment the Core Index for the next iteration
                core_idx++;
            } // core loop

            // After all cores within the current quad have been processed,
            // incorporate calculations for cores that were off into leakage
            idc_vdd += g_wof->all_cores_off_iso * num_cores_off_in_quad /24;

            // Incorporate the cache into leakage calculation.
            // scale from nest to quad
            idc_vdd += scale( l_quad_cache,
                        ( g_wof->tempq[quad_idx] - g_wof->tempnest_sensor) );

        } // quad on/off conditional
    } // quad loop

    // Finally, save the calculated leakage to amec
    g_wof->idc_vdd = idc_vdd;
}

/**
 * calculate_nest_leakage
 *
 * Description: Function to calculate the nest leakage using VPD leakage data,
 *              temperature, and voltage
 */
void calculate_nest_leakage( void )
{
    // Get the VOLTVDN sensor to choose the appropriate nest voltage
    // index
    int l_nest_v_idx = get_voltage_index( g_wof->voltvdn_sensor );

    // Assign to nest leakage.
    g_wof->idc_vdn =
        scale_and_interpolate(G_oppb.iddq.ivdn,
                              G_oppb.iddq.avgtemp_vdn,
                              l_nest_v_idx,
                              g_wof->tempnest_sensor,
                              g_wof->voltvdn_sensor );
}

/**
 * calculate_effective_capacitance
 *
 * Description: Generic function to perform the effective capacitance
 *              calculations.
 *              C_eff = I / (V^1.3 * F)
 *
 *              I is the AC component of Idd in 0.01 Amps (or10 mA)
 *              V is the silicon voltage in 100 uV
 *              F is the frequency in MHz
 *
 *              Note: Caller must ensure they check for a 0 return value
 *                    and disable wof if that is the case
 *
 * Param[in]: i_iAC - the AC component
 * Param[in]: i_voltage - the voltage component in 100uV
 * Param[in]: i_frequency - the frequency component
 *
 * Return: The calculated effective capacitance
 */
uint32_t calculate_effective_capacitance( uint32_t i_iAC_10ma,
                                          uint32_t i_voltage_100uV,
                                          uint32_t i_frequency_mhz )
{
    // Prevent divide by zero
    if( (i_frequency_mhz == 0)  || (i_voltage_100uV == 0) )
    {
        // Return 0 causing caller to disable wof.
        return 0;
    }

    // Compute V^1.3 using a best-fit equation
    // (V^1.3) = (21374 * (voltage in 100uV) - 50615296)>>10
    uint32_t v_exp_1_dot_3 = (21374 * i_voltage_100uV - 50615296) >> 10;

    // Compute I / (V^1.3)
    uint32_t I = i_iAC_10ma << 14; // * 16384

    // Prevent divide by zero
    if( v_exp_1_dot_3 == 0 )
    {
        // Return 0 causing caller to disable wof.
        return 0;
    }

    uint32_t c_eff = (I / v_exp_1_dot_3);
    c_eff = c_eff << 14; // * 16384

    // Divide by frequency and return the final value.
    // (I / (V^1.3 * F)) == I / V^1.3 /F
    return c_eff / i_frequency_mhz;
}

/**
 * calculate_ceff_ratio_vdn
 *
 * Description: Function to calculate the effective capacitance ratio
 *              for the nest
 */
void calculate_ceff_ratio_vdn( void )
{
    // Get ceff_tdp_vdn from OCCPPB
    g_wof->ceff_tdp_vdn = G_oppb.ceff_tdp_vdn;

    // Calculate ceff_vdn
    // iac_vdn/ (VOLTVDN^1.3 * Fnest)
    g_wof->c_ratio_vdn_freq = G_nest_frequency_mhz;
    g_wof->ceff_vdn =
                calculate_effective_capacitance( g_wof->iac_vdn,
                                                 g_wof->voltvdn_sensor,
                                                 g_wof->c_ratio_vdn_freq );

    // Prevent divide by zero
    if( g_wof->ceff_tdp_vdn == 0 )
    {
        INTR_TRAC_ERR("WOF Disabled! Ceff VDN divide by 0");
        print_oppb();
        // Return 0
        g_wof->ceff_ratio_vdn = 0;

        set_clear_wof_disabled(SET,
                               WOF_RC_DIVIDE_BY_ZERO_VDN,
                               ERC_WOF_DIVIDE_BY_ZERO_VDN);
    }
    else
    {
        g_wof->ceff_ratio_vdn = g_wof->ceff_vdn / g_wof->ceff_tdp_vdn;
        sensor_update(AMECSENSOR_PTR(CEFFVDNRATIO), (uint16_t)g_wof->ceff_ratio_vdn);
    }
}

/**
 *  calculate_ceff_ratio_vdd
 *
 *  Description: Function to calculate the effective capacitance ratio
 *               for the core
 */
void calculate_ceff_ratio_vdd( void )
{
    uint32_t l_raw_ceff_ratio = 0;
    static bool L_trace_not_tracking = true;

    // If we get v_ratio of 0 from pgpe, force ceff_ratio_vdd to 0;
    if( g_wof->v_ratio == 0 )
    {
        g_wof->ceff_ratio_vdd = 0;
    }
    else
    {
        // If not tracking ceff to frequency use turbo
        if( (G_internal_flags & INT_FLAG_DISABLE_CEFF_TRACKING) ||
           !(G_pgpe_shared_sram_V_I_readings) )
        {
           if(L_trace_not_tracking)
           {
               INTR_TRAC_INFO("ceff_ratio_vdd: Not tracking freq: Internal Flags[0x%08X]  PGPE SRAM?[%d] F[%d]",
                              G_internal_flags,
                              G_pgpe_shared_sram_V_I_readings,
                              g_wof->c_ratio_vdd_freq);

               L_trace_not_tracking = false;
           }

           // Read iac_tdp_vdd from OCCPstateParmBlock struct
           g_wof->iac_tdp_vdd = G_oppb.lac_tdp_vdd_turbo_10ma;

           // Get Vturbo and convert to 100uV (mV -> 100uV) = mV*10
           g_wof->vdd_avg_tdp_100uv = 10 * G_oppb.operating_points[TURBO].vdd_mv;

           // Use Fturbo
           g_wof->c_ratio_vdd_freq =
               G_oppb.operating_points[TURBO].frequency_mhz * g_wof->f_ratio;
        }
        else // track Ceff ratio with currrent average frequency
        {
           // NOTE: g_wof->c_ratio_vdd_freq == g_wof->f_clip_freq is the average frequency
           //       and were updated in read_pgpe_produced_wof_values()
           // P9' we only have nominal and turbo Iac TDP Vdd from PGPE so only interpolate in that range
           if(g_wof->c_ratio_vdd_freq <= G_oppb.operating_points[NOMINAL].frequency_mhz)
           {
                // Below nominal use nominal
                if( (!L_trace_not_tracking) ||
                    (G_allow_trace_flags & ALLOW_CEFF_RATIO_VDD_TRACE) )
                {
                   INTR_TRAC_INFO("ceff_ratio_vdd: Freq[%d] below nominal[%d]",
                                   g_wof->c_ratio_vdd_freq,
                                   G_oppb.operating_points[NOMINAL].frequency_mhz);
                   L_trace_not_tracking = true;
                }

                g_wof->iac_tdp_vdd = G_oppb.lac_tdp_vdd_nominal_10ma;

                // *10 to convert mV to 100uV
                g_wof->vdd_avg_tdp_100uv = 10 * G_oppb.operating_points[NOMINAL].vdd_mv;

                g_wof->c_ratio_vdd_freq = G_oppb.operating_points[NOMINAL].frequency_mhz;

           }
           else if(g_wof->c_ratio_vdd_freq >= G_oppb.operating_points[TURBO].frequency_mhz)
           {
                // Above turbo use turbo
               if( (!L_trace_not_tracking) ||
                   (G_allow_trace_flags & ALLOW_CEFF_RATIO_VDD_TRACE) )
               {
                   INTR_TRAC_INFO("ceff_ratio_vdd: Freq[%d] above turbo[%d]",
                                  g_wof->c_ratio_vdd_freq,
                                  G_oppb.operating_points[TURBO].frequency_mhz);
                   L_trace_not_tracking = true;
               }

                g_wof->iac_tdp_vdd = G_oppb.lac_tdp_vdd_turbo_10ma;

                // *10 to convert mV to 100uV
                g_wof->vdd_avg_tdp_100uv = 10 * G_oppb.operating_points[TURBO].vdd_mv;

                g_wof->c_ratio_vdd_freq = G_oppb.operating_points[TURBO].frequency_mhz;
           }
           else
           {
              // Frequency is between nominal and turbo do linear interpolation

              // Calculate Vdd AC Current average TDP by interpolating #V current@g_wof->c_ratio_vdd_freq
              g_wof->iac_tdp_vdd = interpolate_linear( g_wof->c_ratio_vdd_freq,
                                                       G_oppb.operating_points[NOMINAL].frequency_mhz,
                                                       G_oppb.operating_points[TURBO].frequency_mhz,
                                                       G_oppb.lac_tdp_vdd_nominal_10ma,
                                                       G_oppb.lac_tdp_vdd_turbo_10ma);

              // Calculate Vdd voltage average TDP by interpolating #V voltage@g_wof->c_ratio_vdd_freq
              // *10 to convert mV to 100uV
              g_wof->vdd_avg_tdp_100uv = 10 * interpolate_linear( g_wof->c_ratio_vdd_freq,
                                                                  G_oppb.operating_points[NOMINAL].frequency_mhz,
                                                                  G_oppb.operating_points[TURBO].frequency_mhz,
                                                                  G_oppb.operating_points[NOMINAL].vdd_mv,
                                                                  G_oppb.operating_points[TURBO].vdd_mv);
              if( (!L_trace_not_tracking) ||
                  (G_allow_trace_flags & ALLOW_CEFF_RATIO_VDD_TRACE) )
              {
                  INTR_TRAC_INFO("ceff_ratio_vdd: F[%d] between nominal[%d] and turbo[%d]",
                                  g_wof->c_ratio_vdd_freq,
                                  G_oppb.operating_points[NOMINAL].frequency_mhz,
                                  G_oppb.operating_points[TURBO].frequency_mhz);
                  INTR_TRAC_INFO("ceff_ratio_vdd: interpolated iac_tdp_vdd[%d] nominal[%d] turbo[%d]",
                                  g_wof->iac_tdp_vdd,
                                  G_oppb.lac_tdp_vdd_nominal_10ma,
                                  G_oppb.lac_tdp_vdd_turbo_10ma);
                  INTR_TRAC_INFO("ceff_ratio_vdd: interpolated vdd_avg_tdp_100uv[%d] nominal[%d] turbo[%d]",
                                  g_wof->vdd_avg_tdp_100uv,
                                  G_oppb.operating_points[NOMINAL].vdd_mv * 10,
                                  G_oppb.operating_points[TURBO].vdd_mv * 10);
                  L_trace_not_tracking = true;
              }
           }
        }  // else track to frequency

        g_wof->c_ratio_vdd_volt = multiply_ratio( g_wof->vdd_avg_tdp_100uv,
                                                  g_wof->v_ratio );

        // Calculate ceff_tdp_vdd
        // iac_tdp_vdd / ((V@Freq*Vratio)^1.3 * (Freq*Fratio))
        // Freq is either Turbo (not tracking to frequency) or the average frequency from PGPE
        g_wof->ceff_tdp_vdd =
                    calculate_effective_capacitance( g_wof->iac_tdp_vdd,
                                                     g_wof->c_ratio_vdd_volt,
                                                     g_wof->c_ratio_vdd_freq );
        // Calculate ceff_vdd
        // iac_vdd / (V^1.3 * Freq)
        g_wof->ceff_vdd =
                    calculate_effective_capacitance( g_wof->iac_vdd,
                                                     g_wof->voltvddsense_sensor,
                                                     g_wof->f_clip_freq );

        // Prevent divide by zero
        if( g_wof->ceff_tdp_vdd == 0 )
        {
            // Print debug info to help isolate offending variable
            INTR_TRAC_ERR("WOF Disabled! Ceff VDD divide by 0");
            INTR_TRAC_ERR("iac_tdp_vdd = %d", g_wof->iac_tdp_vdd );
            INTR_TRAC_ERR("v_ratio     = %d", g_wof->v_ratio );
            INTR_TRAC_ERR("f_ratio     = %d", g_wof->f_ratio );
            INTR_TRAC_ERR("c ratio vdd V = %d", g_wof->c_ratio_vdd_volt);
            INTR_TRAC_ERR("c ratio vdd F    = %d", g_wof->c_ratio_vdd_freq);
            INTR_TRAC_ERR("v_clip_mv   = %d", g_wof->v_clip);
            INTR_TRAC_ERR("f_clip_freq   = 0x%x", g_wof->f_clip_freq);

            print_oppb();
            // Return 0
            g_wof->ceff_ratio_vdd = 0;

            set_clear_wof_disabled(SET,
                                   WOF_RC_DIVIDE_BY_ZERO_VDD,
                                   ERC_WOF_DIVIDE_BY_ZERO_VDD);
        }
        else
        {
            // Save raw ceff ratio vdd to a sensor, this sensor is NOT to be used by the WOF alg
            // Multiply by 10000 to convert to correct granularity.
            l_raw_ceff_ratio = (g_wof->ceff_vdd*10000) / g_wof->ceff_tdp_vdd;
            sensor_update(AMECSENSOR_PTR(CEFFVDDRATIO), (uint16_t)l_raw_ceff_ratio);

            // Now check the raw ceff ratio to prevent Over current by clipping to max of 100%
            // this is saved to the parameter used by the rest of the wof alg
            g_wof->ceff_ratio_vdd = prevent_over_current(l_raw_ceff_ratio);
        }
    }  // else v_ratio != 0
}

/**
 *  calculate_AC_currents
 *
 *  Description: Calculate the AC currents
 */
void calculate_AC_currents( void )
{
    g_wof->iac_vdd = g_wof->curvdd_sensor - g_wof->idc_vdd;
    g_wof->iac_vdn = g_wof->curvdn_sensor - g_wof->idc_vdn;
}

/**
 * core_powered_on
 *
 * Description: Helper function to determine whether the given core
 *              is on based off the most recently read data from
 *              OCC-PGPE Shared SRAM
 *
 * Param: The desired core number
 *
 * Return: Returns a non-zero value if the core is powered on, 0 otherwise
 */
uint32_t core_powered_on(uint8_t i_core_num)
{
    return ( g_wof->core_pwr_on & (0x80000000 >> i_core_num));
}

/** multiply_ratio
 *
 *  Description: Helper function to multiply V/F Ratio's by their
 *               operating point preserving the correct granularity
 *
 *  Param[in]: i_operating_point - Operating point taken from the OPPB
 *  Param[in]: i_ratio - the V/F ratio taken from shared OCC-PGPE SRAM
 *
 *  Return: Operating point * ratio
 */
uint32_t multiply_ratio( uint32_t i_operating_point,
                                uint32_t i_ratio )
{
    // We get i_ratio from the PGPE ranging from 0x0000 to 0xffff
    // These hex values conceptually translate from 0.0 to 1.0
    // Extra math is used to convert units to avoid floating point math.
    return ((((i_ratio*10000)/0xFFFF) * i_operating_point) / 10000);
}

/**
 *  num_cores_on_in_quad
 *
 *  Description: Helper function that returns the number of cores
 *               currently powered on in the given quad based off
 *               the most recently read data from OCC-PGPE Shared SRAM
 *
 *  Param: The Quad number
 *
 *  Return: Returns the number of cores powered on within the given quad.
 */
uint8_t num_cores_on_in_quad( uint8_t i_quad_num )
{
    int start_index = i_quad_num * NUM_CORES_PER_QUAD;
    int i;
    uint8_t num_powered_on_cores = 0;
    for(i = start_index; i < (start_index + NUM_CORES_PER_QUAD); i++)
    {
        if( core_powered_on(i) > 0 )
        {
            num_powered_on_cores++;
        }
    }
    return num_powered_on_cores;
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
                            int32_t i_y2 )
{
    if( (i_x2 == i_x1) || (i_y2 == i_y1) )
    {
        return i_y1;
    }
    else
    {
        return ( ((i_X - i_x1)*(i_y2 - i_y1)) / (i_x2 - i_x1) ) + i_y1;
    }
}


/**
 * calculate_temperature_scaling_08V
 *
 * Description: This function calculates the full leakage @0.8V and does interpolation
 *               to determine the m for G_wof_iddq_mult_table
 *               This is a one time calculation.
 */
void calculate_temperature_scaling_08V( void )
{
    int leakage_idx;
    int i;

    // estimate full leakage@0.8V by using IQ data all core, cache ON @0.8V * 24/#sort cores
    // 0.8V is the 3rd entry in the IQ data in 5mA unit *5 to convert to mA
    g_wof->full_leakage_08v_mA = (g_wof->allGoodCoresCachesOn[2] * 5) * 24 / g_wof->good_normal_cores_per_sort;

    // First row of G_wof_mft_full_leakage_08V has the leakage values in mA find the index
    if( g_wof->full_leakage_08v_mA < G_wof_mft_full_leakage_08V[0][0] )
    {
        leakage_idx = 0;
    }
    else if( g_wof->full_leakage_08v_mA >= G_wof_mft_full_leakage_08V[0][NUM_FULL_LEAKAGE_08V-1] )
    {
        leakage_idx = NUM_FULL_LEAKAGE_08V-2;
    }
    else
    {
        for(leakage_idx = 0 ; leakage_idx < NUM_FULL_LEAKAGE_08V-1; leakage_idx++)
        {
            if( (g_wof->full_leakage_08v_mA >= G_wof_mft_full_leakage_08V[0][leakage_idx]) &&
                (g_wof->full_leakage_08v_mA <= G_wof_mft_full_leakage_08V[0][leakage_idx+1]) )
            {
                break;
            }
        }
    }
    // Interpolate the m values in G_wof_mft_full_leakage_08V and write to G_wof_iddq_mult_table
    for(i=0; i < WOF_IDDQ_MULT_TABLE_N; i++)
    {
        G_wof_iddq_mult_table[i][1] = interpolate_linear( g_wof->full_leakage_08v_mA,
                                                          G_wof_mft_full_leakage_08V[0][leakage_idx],
                                                          G_wof_mft_full_leakage_08V[0][leakage_idx+1],
                                                          G_wof_mft_full_leakage_08V[i+1][leakage_idx],
                                                          G_wof_mft_full_leakage_08V[i+1][leakage_idx+1]);
        // only trace first and last entries of table
        if( (i == 0) ||
            (i == (WOF_IDDQ_MULT_TABLE_N - 1) ) )
        {
           TRAC_IMP("calculate_temperature_scaling_08V: G_wof_iddq_mult_table[%d] = %d , %d ",
                     i,
                     G_wof_iddq_mult_table[i][0],
                     G_wof_iddq_mult_table[i][1]);
        }
    }
}

/**
 * calculate_multiplier
 *
 * Description: This function calculates the 'm' in the formula
 * y ~= (T*m) >> 10 by choosing the appropriate row in
 * G_wof_iddq_mult_table based on the passed in temp, and interpolates
 * the values of the 'm' column in order to find the appropriate multiplier
 *
 * Param: the delta temp between tvpd_leak and a temperature sensor. Used
 *        to find the appropriate row/column index into G_wof_iddq_mult_table.
 *
 * Return: The multiplier representing the temperature factor
 */
uint32_t calculate_multiplier( int32_t i_temp )
{
    int mult_idx;  // row index (temperature delta) into G_wof_iddq_mult_table[][]

    // find the row based on the delta temperature (i_temp)
    if( i_temp < G_wof_iddq_mult_table[0][0] )
    {
        mult_idx = 0;
    }
    else if( i_temp >= G_wof_iddq_mult_table[WOF_IDDQ_MULT_TABLE_N-1][0] )
    {
        mult_idx = WOF_IDDQ_MULT_TABLE_N - 2;
    }
    else
    {
        for(mult_idx = 0 ; mult_idx < WOF_IDDQ_MULT_TABLE_N-1; mult_idx++)
        {
            if( (G_wof_iddq_mult_table[mult_idx][0] <= i_temp) &&
                (G_wof_iddq_mult_table[mult_idx+1][0] >= i_temp) )
            {
                break;
            }
        }
    }

    // mult index now has the row index into G_wof_iddq_mult_table.
    // use it to calculate the final multiplier
    return interpolate_linear( i_temp,
                      (int32_t)G_wof_iddq_mult_table[mult_idx][0],
                      (int32_t)G_wof_iddq_mult_table[mult_idx+1][0],
                      (int32_t)G_wof_iddq_mult_table[mult_idx][1],
                      (int32_t)G_wof_iddq_mult_table[mult_idx+1][1]);
}

/**
 * read_sensor_data
 *
 * Description: One time place to read out all the sensors needed
 *              for wof calculations. First thing wof_main does.
 */
void read_sensor_data( void )
{
    // Read out necessary Sensor data for WOF calculation
    g_wof->curvdd_sensor        = getSensorByGsid(CURVDD)->sample;
    g_wof->curvdn_sensor        = getSensorByGsid(CURVDN)->sample;
    g_wof->voltvddsense_sensor  = getSensorByGsid(VOLTVDDSENSE)->sample;
    g_wof->tempnest_sensor      = getSensorByGsid(TEMPNEST)->sample;
    g_wof->voltvdn_sensor       = getSensorByGsid(VOLTVDN)->sample;
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
            // Set the vfrt state back to standby
            g_wof->vfrt_state = STANDBY;

            // Set the bit
            g_wof->wof_disabled |= i_bit_mask;

            // If user is trying to force a reset even though WOF is disabled,
            // Skip straight to error log creation
            if( (g_wof->wof_disabled) &&
                (i_bit_mask == WOF_RC_RESET_DEBUG_CMD) )
            {
                INTR_TRAC_INFO("User Requested WOF reset!");
                l_logError = true;
                break;
            }

            // If OCC has not yet been enabled through TMGT/HTMGT/OPPB, skip
            // error log
            if( (g_wof->wof_disabled & WOF_RC_OCC_WOF_DISABLED) ||
                (g_wof->wof_disabled & WOF_RC_OPPB_WOF_DISABLED) )
            {
                INTR_TRAC_ERR("OCC encountered a WOF error before TMGT/HTMGT"
                              " enabled it. wof_disabled = 0x%08x",
                              g_wof->wof_disabled);
                break;
            }

            // If error has already been logged, trace and skip
            if( L_errorLogged )
            {
                INTR_TRAC_ERR("Another WOF error was encountered!"
                              " wof_disabled=0x%08x",
                              g_wof->wof_disabled);
            }
            else
            {
                INTR_TRAC_ERR("WOF encountered an error. wof_disabled ="
                              " 0x%08x", g_wof->wof_disabled );
                     // If wof is disabled in driver, skip generating all error logs
                if( g_wof->wof_disabled & WOF_RC_DRIVER_WOF_DISABLED )
                {
                    static bool trace = true;
                    if(trace)
                    {
                        INTR_TRAC_INFO("WOF is disabled in the driver. wof_disabled = "
                                       "0x%08x", g_wof->wof_disabled );
                        trace = false;
                    }
                    break;
                }
                // Make sure the reason requires an error log
                if( g_wof->wof_disabled & ERRL_RETURN_CODES )
                {
                    l_logError = true;
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
                // Set Init state
                g_wof->wof_init_state = WOF_CONTROL_ON_SENT_WAITING;
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
 * schedule_vfrt_request
 *
 * Description: Called from amec_slv_common_tasks_post every 500us. Checks
 *              to see if a new VFRT table is ready to be sent to the PGPE
 *              and if so, sends it and sets the vfrt state
 */
void schedule_vfrt_request( void )
{
    if( (g_wof->vfrt_state == NEED_TO_SCHEDULE ) &&
        (async_request_is_idle(&G_wof_vfrt_req.request)) )
    {
        g_wof->gpe_req_rc = pgpe_request_schedule(&G_wof_vfrt_req);

        // Check to make sure IPC request was scheduled correctly
        if( g_wof->gpe_req_rc != 0 )
        {
            // If we were attempting to send the initial VFRT request,
            // reset the state machine so we can start the process
            // over
            if( g_wof->wof_init_state == INITIAL_VFRT_SENT_WAITING )
            {
                g_wof->wof_init_state = WOF_DISABLED;
            }

            INTR_TRAC_ERR("schedule_vfrt_request: Error sending VFRT! gperc=%d"
                            g_wof->gpe_req_rc );

            // Formally disable wof
            set_clear_wof_disabled( SET, WOF_RC_IPC_FAILURE, ERC_WOF_IPC_FAILURE );

            // Reset the global return code after logging the error
            g_wof->gpe_req_rc = 0;
        }

        // Update vfrt state
        g_wof->vfrt_state = SCHEDULED;
    }
}



/**
 * send_initial_vfrt_to_pgpe
 *
 * Description: Function calculates the address of the final vfrt in Main Memory
 *              and subsequently sends that address to the PGPE to use via IPC
 *              command.
 */
void send_initial_vfrt_to_pgpe( void )
{
    // Set the steps for VDN, VDD, and Quads to the max value
    g_wof->vdn_step_from_start  = g_wof->vdn_size - 1;
    g_wof->vdd_step_from_start  = g_wof->vdd_size - 1;
    g_wof->quad_step_from_start = MAXIMUM_QUADS - 1;

    // Calculate the address of the final vfrt
    g_wof->next_vfrt_main_mem_addr = calc_vfrt_mainstore_addr();

    // Send the final vfrt to shared OCC-PGPE SRAM.
    send_vfrt_to_pgpe( g_wof->next_vfrt_main_mem_addr );

    // Update Init state
    if(g_wof->wof_init_state < INITIAL_VFRT_SENT_WAITING)
    {
        g_wof->wof_init_state = INITIAL_VFRT_SENT_WAITING;
    }

}

/**
 * read_req_active_quads
 *
 * Description: Reads the Requested Active Quads Update field from
 *              shared OCC-PGPE SRAM into global amec struct
 */
void read_req_active_quads( void )
{
    uint64_t l_doubleword = in64(g_wof->req_active_quads_addr);

    g_wof->req_active_quad_update = (uint8_t)(0x00000000000000ff & l_doubleword);
    // Count the number of on bits in req_active_quad_update
    int i = 0;
    uint8_t on_bits = 0;
    uint8_t bit_mask = 128; // 0b10000000
    for( i = 0; i < MAXIMUM_QUADS; i++ )
    {
        if( bit_mask & g_wof->req_active_quad_update )
        {
            on_bits++;
        }
        bit_mask >>= 1;
    }

    // Save number of on bits
    g_wof->num_active_quads = on_bits;
}

/**
 * get_voltage_index
 *
 * Description: Using the input voltage, returns the index to the lower bound
 *              of the two voltages surrounding the input voltage in
 *              G_iddq_voltages
 *
 * Param[in]: i_voltage - the input voltage to select the index for
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
 * Description: Performs i_current*full_leakage_08V^(-((T-tvpd_leak)/257.731))*1.45^((T-tvpd_leak)/10)
 *              Note: The calculation is performed by doing a lookup
 *              in G_wof_iddq_mult_table based on the passed in delta temp.
 *
 * Param[in]: i_current - The current to scale
 * Param[in]: i_delta_temp - The measured temperature - the temperature at which
 *                           the input current was measured.
 * Return: The scaled current
 */
uint32_t scale( uint16_t i_current,
                int16_t i_delta_temp )
{
    // Calculate the multipliers for the leakage current using the delta temp
    uint32_t leak_multiplier = calculate_multiplier( i_delta_temp );
    // Scale the current and return
    return (i_current * leak_multiplier) >> 10;
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
 * Param[in]: i_base_temp - The base temperature used to scale the current
 * Param[in]: i_voltage - The associated voltage.
 *
 * Return: The approximated scaled current in 10mA.
 */
uint32_t scale_and_interpolate( uint16_t * i_leak_arr,
                       uint8_t * i_avgtemp_arr,
                       int i_idx,
                       uint16_t i_base_temp,
                       uint16_t i_voltage )
{
    // Calculate the Delta temps for the upper and lower bounds
    // Note: avgtemp arrays are in 0.5C. Divide by 2 to convert
    //       to 1C
    int16_t lower_delta = i_base_temp - (i_avgtemp_arr[i_idx] >> 1);
    int16_t upper_delta = i_base_temp - (i_avgtemp_arr[i_idx+1] >> 1);

    // Scale the currents based on the delta temperature
    // Note: leakage arrays are in 5mA. Multiply by 5 to convert to mA
    uint32_t scaled_lower_leak = scale( i_leak_arr[i_idx],
                                        lower_delta )*5;
    uint32_t scaled_upper_leak = scale( i_leak_arr[i_idx+1],
                                        upper_delta )*5;

    // Approximate current between the scaled currents using linear
    // interpolation and return the result
    // Divide by 10 to get 10mA units
    return interpolate_linear( (int32_t) i_voltage,
                               (int32_t) G_iddq_voltages[i_idx],
                               (int32_t) G_iddq_voltages[i_idx+1],
                               (int32_t) scaled_lower_leak,
                               (int32_t) scaled_upper_leak ) / 10;

}

/**
 * print_data
 *
 * Description: For internal use only. Prints information on the current
 *              state of the wof algorithm.
 */
void print_data( void )
{
    INTR_TRAC_INFO("ADDRESSES:");
    INTR_TRAC_INFO("vfrt_tbls_main_mem_addr: 0x%08x", g_wof->vfrt_tbls_main_mem_addr);
    INTR_TRAC_INFO("pgpe_wof_state_addr:     0x%08x", g_wof->pgpe_wof_state_addr);
    INTR_TRAC_INFO("req_active_quads_addr:   0x%08x", g_wof->req_active_quads_addr);
    INTR_TRAC_INFO("quad_state_0_addr:       0x%08x", g_wof->quad_state_0_addr);
    INTR_TRAC_INFO("quad_state_1_addr:       0x%08x", g_wof->quad_state_1_addr);
    INTR_TRAC_INFO("pstate_tbl_sram_addr:    0x%08x", g_wof->pstate_tbl_sram_addr);
    INTR_TRAC_INFO("pong buffer address:     0x%08x", (&G_sram_vfrt_pong_buffer));
    INTR_TRAC_INFO("ping buffer address:     0x%08x", (&G_sram_vfrt_ping_buffer));
    INTR_TRAC_INFO("curr ping/pong addr:     0x%08x", g_wof->curr_ping_pong_buf);
    INTR_TRAC_INFO("next ping/pong addr:     0x%08x", g_wof->next_ping_pong_buf);
    INTR_TRAC_INFO("");
    INTR_TRAC_INFO("version:            %d", g_wof->version);
    INTR_TRAC_INFO("vfrt_block_size:    %d",g_wof->vfrt_block_size);
    INTR_TRAC_INFO("vfrt_blck_hdr_sz:   %d",g_wof->vfrt_blck_hdr_sz);
    INTR_TRAC_INFO("vfrt_data_size:     %d ",g_wof->vfrt_data_size);
    INTR_TRAC_INFO("active_quads_size:  %d",g_wof->active_quads_size);
    INTR_TRAC_INFO("core_count:         %d",g_wof->core_count);
    INTR_TRAC_INFO("vdn_start:          %d",g_wof->vdn_start);
    INTR_TRAC_INFO("vdn_step:           %d",g_wof->vdn_step);
    INTR_TRAC_INFO("vdn_size:           %d",g_wof->vdn_size);
    INTR_TRAC_INFO("vdd_start:          %d",g_wof->vdd_start);
    INTR_TRAC_INFO("vdd_step:           %d",g_wof->vdd_step);
    INTR_TRAC_INFO("vdd_size:           %d",g_wof->vdd_size);
    INTR_TRAC_INFO("vdn_step_from_start:%d",g_wof->vdn_step_from_start);
    INTR_TRAC_INFO("vdd_step_from_start:%d",g_wof->vdd_step_from_start);
    INTR_TRAC_INFO("num_active_quads:   %d",g_wof->num_active_quads);
    INTR_TRAC_INFO("quad_step_4rm_start:%d",g_wof->quad_step_from_start);
    INTR_TRAC_INFO("vratio_start:       %d",g_wof->vratio_start);
    INTR_TRAC_INFO("vratio_step:        %d",g_wof->vratio_step);
    INTR_TRAC_INFO("vratio_size:        %d",g_wof->vratio_size);
    INTR_TRAC_INFO("fratio_start:       %d",g_wof->fratio_start);
    INTR_TRAC_INFO("fratio_step:        %d",g_wof->fratio_step);
    INTR_TRAC_INFO("fratio_size:        %d",g_wof->fratio_size);
    INTR_TRAC_INFO("fratio:             %d",g_wof->f_ratio);
    INTR_TRAC_INFO("vratio:             %d",g_wof->v_ratio);
    INTR_TRAC_INFO("fclip_ps:           %x",g_wof->f_clip_ps);
    INTR_TRAC_INFO("fclip_freq:         %d",g_wof->f_clip_freq);
    INTR_TRAC_INFO("vclip:              %d",g_wof->v_clip);
    INTR_TRAC_INFO("req_active_quads:   %x", g_wof->req_active_quad_update);
    INTR_TRAC_INFO("vfrt_mm_offset:     0x%08x", g_wof->vfrt_mm_offset);
}

/**
 * print_oppb
 *
 * Description: For internal use only. Traces the contents of G_oppb that
 *              are used in this file.
 */
void print_oppb( void )
{
    CMDH_TRAC_INFO("Printing Contents of OCCPstateParmBlock");

    int i;
    for(i = 0; i < VPD_PV_POINTS; i++)
    {
        CMDH_TRAC_INFO("operating_points[%d] = Freq[%d] vdd_mv[%d] ",
                       i,
                       G_oppb.operating_points[i].frequency_mhz,
                       G_oppb.operating_points[i].vdd_mv );
    }
    CMDH_TRAC_INFO("lac_tdp_vdd_turbo_10ma = %d", G_oppb.lac_tdp_vdd_turbo_10ma);
    CMDH_TRAC_INFO("lac_tdp_vdd_nominal_10ma = %d", G_oppb.lac_tdp_vdd_nominal_10ma);
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
    static uint8_t L_method_trace = 0;  // P9 or P10 to indicate what method we are using to trace
    static uint8_t L_oc_prevention_timer = 0;
    static uint8_t L_ocs_dirty_prev = 0;
    uint32_t l_clipped_ratio = i_ceff_ratio;
    uint32_t l_new_ratio_from_measured = 0;
    uint32_t l_new_ratio_from_prev = 0;

    // determine if Ceff needs to be adjusted for overcurrent
    // different process between P9 and P10

    if( (!(G_internal_flags & INT_FLAG_ENABLE_P10_OCS)) ||
        (!G_pgpe_shared_sram_V_I_readings) )
    {
       if(L_method_trace != 9)
       {
           INTR_TRAC_IMP("Using P9 Overcurrent method");
           L_method_trace = 9;
       }
       if( i_ceff_ratio > MAX_CEFF_RATIO ) // 10000 = 1.0 ratio
       {
           INCREMENT_ERR_HISTORY( ERRH_CEFF_RATIO_VDD_EXCURSION );
           l_clipped_ratio = MAX_CEFF_RATIO;

           // If over current timer not started, start it.
           if( L_oc_prevention_timer == 0 )
           {
               L_oc_prevention_timer = 10; // 10 WOF iterations
           }
           else
           {
               L_oc_prevention_timer--;
           }
       }
       else if( L_oc_prevention_timer != 0 ) // Timer is running
       {
           l_clipped_ratio = MAX_CEFF_RATIO;
           L_oc_prevention_timer--;
       }
    }
    else  // P10 method
    {
       if(L_method_trace != 10)
       {
           INTR_TRAC_IMP("Using P10 Overcurrent method");
           L_method_trace = 10;
       }

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

       // update sensor for calculated CeffRatio addr
       sensor_update(AMECSENSOR_PTR(OCS_ADDR), (uint16_t)g_wof->vdd_oc_ceff_add);
    }  // else P10 method

    return l_clipped_ratio;
}
