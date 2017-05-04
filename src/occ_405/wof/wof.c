/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/wof/wof.c $                                       */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2016,2017                        */
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
extern pstateStatus G_proc_pstate_status;
//******************************************************************************
// Globals
//******************************************************************************
uint8_t G_sram_vfrt_ping_buffer[MIN_BCE_REQ_SIZE] __attribute__ ((section(".vfrt_ping_buffer")));
uint8_t G_sram_vfrt_pong_buffer[MIN_BCE_REQ_SIZE] __attribute__ ((section(".vfrt_pong_buffer")));

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

// Approximate y = 1.3^((T-tvpd_leak)/10)
// Interpolate (T-tvpd_leak) in the table below to find m.
// y ~= (T*m) >> 10     (shift out 10 bits)
// Error in estimation is no more than 0.9%
// The first column represents the result of T-tvpd_leak where T is the
// associated temperature sensor. The second column represents the associated
// m(slope) when the delta temp the first value.
int16_t G_wof_iddq_mult_table[][2] = {
    //Delta Temperature in C, m
    {-50, 276},
    {-40, 359},
    {-30, 466},
    {-20, 606},
    {-10, 788},
    {0,   1024},
    {10,  1331},
    {20,  1731},
    {30,  2250},
    {40,  2925},
    {50,  3802}
};
#define WOF_IDDQ_MULT_TABLE_N 11

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
 *              entirely. Called from amec_slave_smh.c::amec_slv_state_4. Called
 *              every 2ms, but calls the wof_main algorithm every other invocation
 *              resulting in 4ms intervals
 * Param: None
 *
 * Return: None
 */
void call_wof_main( void )
{
    // Variable to control toggling between invocations
    // Init to false to ensure init asynchronous operations have
    // the extra 2ms to finish.
    static bool L_run_wof = false;

    // Variable to ensure we do not keep trying to send vfrt GpeRequest
    // more than 1 extra time.
    static bool L_vfrt_last_chance = false;

    // Variable to ensure we do not keep trying to send the wof control
    static bool L_wof_control_last_chance = false;

    // GpeRequest more than 1 extra time.
    bool enable_success = false;
    do
    {
        // If the init state says we just turned WOF on in pgpe, clear
        // PGPE wof disabled bit
        if(g_wof->wof_init_state == PGPE_WOF_ENABLED_NO_PREV_DATA)
        {
            set_clear_wof_disabled( CLEAR, WOF_RC_PGPE_WOF_DISABLED );
        }


        // Make sure wof has not been disabled
        if( g_wof->wof_disabled )
        {
            // If error loggedin callback, record now
            if( g_wof->vfrt_callback_error )
            {
                set_clear_wof_disabled( SET, WOF_RC_VFRT_REQ_FAILURE );
                // After official error recorded, prevent this code
                // from running from same setting of the var.
                g_wof->vfrt_callback_error = 0;
            }

            if( g_wof->pgpe_wof_off )
            {
                set_clear_wof_disabled( SET, WOF_RC_PGPE_WOF_DISABLED );
                g_wof->pgpe_wof_off = 0;
            }
            // WOF has been flagged as disabled. Skip algorithm
            break;
        }

        // Make sure Pstate Protocol is on
        if(G_proc_pstate_status != PSTATES_ENABLED)
        {
            // No need to call disable wof as the PGPE would
            // already have disabled wof. Just flag reason.
            CMDH_TRAC_ERR("WOF Disabled! Pstate Protocol off");
            set_clear_wof_disabled( SET, WOF_RC_PSTATE_PROTOCOL_OFF );
            break;
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
                        // calculate initial vfrt, send gpeRequest
                        // Initial vfrt is the last vfrt in Main memory
                        send_initial_vfrt_to_pgpe();
                        break;

                    case INITIAL_VFRT_SENT_WAITING:
                        // Check if request is still processing.
                        // Init state updated in wof_vfrt_callback
                        if( !async_request_is_idle(&G_wof_vfrt_req.request) )
                        {
                            if( L_vfrt_last_chance )
                            {
                                CMDH_TRAC_ERR("WOF Disabled! Initial VFRT request timeout");
                                set_clear_wof_disabled( SET, WOF_RC_VFRT_REQ_TIMEOUT);
                            }
                            else
                            {
                                L_vfrt_last_chance = true;
                            }
                        }
                        else
                        {
                            // If we got here, init state was set in
                            // wof_vfrt_callback
                            L_vfrt_last_chance = false;
                        }
                        break;

                    case INITIAL_VFRT_SUCCESS:
                        // Send wof control on gpe request
                        // If enable_success returns true, init state was set
                        enable_success = enable_wof();
                        if( !enable_success )
                        {
                            if( L_wof_control_last_chance )
                            {
                                CMDH_TRAC_ERR("WOF Disabled! Control req timeout(1)");
                                set_clear_wof_disabled(SET, WOF_RC_CONTROL_REQ_TIMEOUT);
                            }
                            else
                            {
                                L_wof_control_last_chance = true;
                            }
                        }
                        else
                        {
                            // Reset the last chance variable
                            // Init state updated in enable_wof
                            L_wof_control_last_chance = false;
                        }
                        break;

                    case WOF_CONTROL_ON_SENT_WAITING:
                        // check if request is still processing.
                        if( !async_request_is_idle(&G_wof_control_req.request) )
                        {
                            if( L_wof_control_last_chance )
                            {
                                CMDH_TRAC_ERR("WOF Disabled! Control req timeout(2)");
                                set_clear_wof_disabled(SET, WOF_RC_CONTROL_REQ_TIMEOUT);
                            }
                            else
                            {
                                L_wof_control_last_chance = true;
                            }
                        }
                        // Init state updated in wof_control_callback
                        break;

                    default:
                        break;
                }
            }
            // If we have made it to at least WOF enabled no previous data
            // state run wof routine normally ensuring wof was not disabled
            // in previous 5 states
            if( (g_wof->wof_init_state >= PGPE_WOF_ENABLED_NO_PREV_DATA) &&
                !g_wof->wof_disabled )
            {
                // Make sure we run algo on appropriate tick
                if( !L_run_wof )
                {
                    // Only check if req active quads changed if we are
                    // in the fully enabled wof state
                    if(g_wof->wof_init_state == WOF_ENABLED)
                    {
                        // Read active quads from sram
                        read_req_active_quads();

                        // If the requested active quads changed last loop
                        // Send vfrt with new req active quads
                        if( g_wof->req_active_quad_update !=
                            g_wof->prev_req_active_quads )
                        {
                            // Calculate new quad step
                            g_wof->quad_step_from_start =
                                                    calc_quad_step_from_start();
                            // Compute new VFRT Main Memory address using new quads
                            g_wof->next_vfrt_main_mem_addr =
                                                     calc_vfrt_mainstore_addr();
                            // Send new VFRT
                            send_vfrt_to_pgpe( g_wof->next_vfrt_main_mem_addr );
                        }
                    }
                    L_run_wof = true;
                }
                else
                {
                    // Normal execution of wof algorithm
                    if( !async_request_is_idle(&G_wof_vfrt_req.request) )
                    {
                        if( L_vfrt_last_chance )
                        {
                            CMDH_TRAC_ERR("WOF Disabled! VFRT req timeout");
                            set_clear_wof_disabled(SET,WOF_RC_VFRT_REQ_TIMEOUT);
                        }
                        else
                        {
                            L_vfrt_last_chance = true;
                        }
                    }
                    else
                    {
                        // Request is idle. Run wof algorithm
                        wof_main();
                        L_run_wof = false;
                        L_vfrt_last_chance = false;
                        // Finally make sure we are in the fully enabled state
                        if( g_wof->wof_init_state == PGPE_WOF_ENABLED_NO_PREV_DATA )
                        {
                            g_wof->wof_init_state = WOF_ENABLED;
                        }
                    }
                } // L_run_wof
            } // >= PGPE_WOF_ENABLED_NO_PREV_DATA
        } // IS_OCC_STATE_ACTIVE
    } while( 0 );
}

/**
 *  wof_main
 *
 *  Description: Main Wof algorithm
 *
 *  Param: None
 *
 *  Return: None
 */
void wof_main(void)
{
    // Read out the sensor data needed for calculations
    read_sensor_data();

    // Read out PGPE data from shared SRAM
    read_shared_sram();

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
 *  calculate_step_from_start
 *
 *  Description: Calculates the step number for the current VDN/VDD
 *
 *  Param[in]: i_ceff_vdx_ratio - The current Ceff_vdd or Ceff_vdn_ratio
 *                                to calculate the step for.
 *  Param[in]: i_step_size - The size of each step.
 *  Param[in]: i_min_ceff - The minimum step number for this VDN/VDD
 *  Param[in]: i_max_step - The maximum step number for this VDN/VDD
 *
 *  Return: The calculated step for current Ceff_vdd/Ceff_vdn
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
 * Return: The desired VFRT address
 */
uint32_t calc_vfrt_mainstore_addr( void )
{
    // Wof tables address calculation
    // (Base_addr + (sizeof VFRT * (total active quads * ( (g_wof->vdn_step_from_start * vdd_size) + (g_wof->vdd_step_from_start) ) + (g_wof->quad_step_from_start))))
    uint32_t offset = G_wof_header.vfrt_block_size *
                    (( G_wof_header.active_quads_size *
                    ((g_wof->vdn_step_from_start * G_wof_header.vdd_size) +
                    g_wof->vdd_step_from_start) ) + g_wof->quad_step_from_start);

    // Skip the wof header at the beginning of wof tables
    uint32_t wof_tables_base = G_pgpe_header.wof_tables_addr + WOF_HEADER_SIZE;

    return wof_tables_base + offset;
}


/**
 * copy_vfrt_to_sram
 *
 * Description: Call back function to copy VFRT into SRAM ping/pong buffer
 *              This call will also tell the PGPE that a new VFRT is available
 *
 * Param[in]: i_parms - pointer to a struct that will hold data necessary to
 *                      the calculation.
 *                      -Pointer to vfrt table temp buffer
 */
void copy_vfrt_to_sram( copy_vfrt_to_sram_parms_t * i_parms)
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
    int l_gperc; // gpe schedule return code
    uint8_t * l_buffer_address;


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
            i_parms->vfrt_table->data,
            G_wof_header.vfrt_block_size );

    // Set the parameters for the GpeRequest
    G_wof_vfrt_parms.homer_vfrt_ptr = (HomerVFRTLayout_t*)l_buffer_address;
    G_wof_vfrt_parms.active_quads = g_wof->req_active_quad_update;

    // Send IPC command to PGPE with new vfrt address and active quads
    // Should not need to check if request is idle as wof_main does before
    // the WOF calculations begin.
    l_gperc = gpe_request_schedule( &G_wof_vfrt_req );

    // Confirm Successful scheduling of WOF VFRT task
    if(l_gperc != 0)
    {
        // TODO: Cannot create an error log within a callback function. Need to handle.
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
    }
}

/**
 * send_vfrt_to_pgpe
 *
 * Description: Function to copy new VFRT from Mainstore to local SRAM buffer
 *              and calls copy_vfrt_to_sram callback function to send new VFRT
 *              to the PGPE
 *              Note: If desired VFRT is the same as previous, skip.
 *
 * Param[in]: i_vfrt_main_mem_addr - Address of the desired vfrt table.
 */
void send_vfrt_to_pgpe( uint32_t i_vfrt_main_mem_addr )
{
    int l_ssxrc = SSX_OK;
    uint32_t l_reasonCode = 0;
    uint32_t l_extReasonCode = 0;

    do
    {
        // First check if the address is 128-byte aligned. error if not.
        if( i_vfrt_main_mem_addr % 128 )
        {
            CMDH_TRAC_ERR("VFRT Main Memory address NOT 128-byte aligned:"
                    " 0x%08x", i_vfrt_main_mem_addr);
            set_clear_wof_disabled(SET, WOF_RC_VFRT_ALIGNMENT_ERROR);

            break;
        }

        if( (i_vfrt_main_mem_addr == g_wof->curr_vfrt_main_mem_addr ) &&
            (g_wof->req_active_quad_update ==
             g_wof->prev_req_active_quads) )
        {
            // VFRT and requested active quads are unchanged. Skip
            break;
        }
        // Either the Main memory address changed or req active quads changed
        // get VFRT based on new values
        else
        {
            // New VFRT needed from Mainstore, create BCE request to get it.
            BceRequest l_vfrt_req;

            // 128-byte aligned temp buffer to hold data
            temp_bce_request_buffer_t l_temp_bce_buff = {{0}};

            // Create structure to hold parameters for callback function
            copy_vfrt_to_sram_parms_t l_callback_parms;
            l_callback_parms.vfrt_table = &l_temp_bce_buff;

            // Create request
            l_ssxrc = bce_request_create(
                             &l_vfrt_req,                 // block copy object
                             &G_pba_bcde_queue,           // main to sram copy engine
                             i_vfrt_main_mem_addr,     //mainstore address
                             (uint32_t) &l_temp_bce_buff, // SRAM start address
                             MIN_BCE_REQ_SIZE,            // size of copy
                             SSX_WAIT_FOREVER,            // no timeout
                             (AsyncRequestCallback)copy_vfrt_to_sram,
                             (void *)&l_callback_parms,
                             ASYNC_CALLBACK_IMMEDIATE );

            if(l_ssxrc != SSX_OK)
            {
                CMDH_TRAC_ERR("send_vfrt_to_pgpe: BCDE request create failure rc=[%08X]", -l_ssxrc);
                /*
                 * @errortype
                 * @moduleid    SEND_VFRT_TO_PGPE
                 * @reasoncode  SSX_GENERIC_FAILURE
                 * @userdata1   RC for BCE block-copy engine
                 * @userdata2   Internal function checkpoint
                 * @userdata4   ERC_BCE_REQUEST_CREATE_FAILURE
                 * @devdesc     Failed to create BCDE request
                 */
                l_reasonCode = SSX_GENERIC_FAILURE;
                l_extReasonCode = ERC_BCE_REQUEST_CREATE_FAILURE;
                break;
            }

            // Do the actual copy
            l_ssxrc = bce_request_schedule( &l_vfrt_req );

            if(l_ssxrc != SSX_OK)
            {
                CMDH_TRAC_ERR("send_vfrt_to_pgpe: BCE request schedule failure rc=[%08X]", -l_ssxrc);
                /*
                 * @errortype
                 * @moduleid    SEND_VFRT_TO_PGPE
                 * @reasoncode  SSX_GENERIC_FAILURE
                 * @userdata1   RC for BCE block-copy engine
                 * @userdata4   ERC_BCE_REQUEST_SCHEDULE_FAILURE
                 * @devdesc     Failed to read PPMR data by using BCDE
                 */
                l_reasonCode = SSX_GENERIC_FAILURE;
                l_extReasonCode = ERC_BCE_REQUEST_SCHEDULE_FAILURE;
                break;
            }
        }
    }while( 0 );

    // Check for errors and log, if any
    if( l_ssxrc != SSX_OK )
    {
        errlHndl_t l_errl = createErrl(SEND_VFRT_TO_PGPE,        //modId
                                       l_reasonCode,             //reasoncode
                                       l_extReasonCode,          //Extended reason code
                                       ERRL_SEV_UNRECOVERABLE,   //Severity
                                       NULL,                     //Trace Buf
                                       0,                        //Trace Size
                                       -l_ssxrc,                 //userdata1
                                       0);                       //userdata2

        // Callout firmware
        addCalloutToErrl(l_errl,
                         ERRL_CALLOUT_TYPE_COMPONENT_ID,
                         ERRL_COMPONENT_ID_FIRMWARE,
                         ERRL_CALLOUT_PRIORITY_HIGH);

        // Commit error log
        commitErrl(&l_errl);

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
    g_wof->f_clip  = l_wofstate.fields.fclip_ps;
    g_wof->v_clip  = l_wofstate.fields.vclip_mv;
    g_wof->f_ratio = l_wofstate.fields.fratio;
    g_wof->v_ratio = l_wofstate.fields.vratio;

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
            l_voltage = g_wof->voltvddsense_sensor;;
        }
        else
        {
            // Calculate the address of the pstate for the current quad.
            uint32_t pstate_addr = G_pgpe_header.occ_pstate_table_sram_addr +
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
    int l_chip_v_idx;
    uint16_t l_quad_cache;
    uint16_t idc_vdd = 0;
    uint8_t  num_quads_off = 0;
    uint16_t temperature = 0;

    // Get the VOLTVDDSENSE sensor and choose the appropriate
    // chip voltage index
    uint32_t l_v_chip = g_wof->voltvddsense_sensor;

    // Choose the lower bound index of G_iddq_voltages
    l_chip_v_idx = get_voltage_index( l_v_chip );

    // Save index used for interpolating voltages to amec
    g_wof->chip_volt_idx = l_chip_v_idx;

    // Calculate all variables that will be used in the core
    // loop that only need to be calculated once.

    // ALL_CORES_OFF_ISO
    g_wof->all_cores_off_iso =
            scale_and_interpolate( G_oppb.iddq.ivdd_all_cores_off_caches_off,
                                   G_oppb.iddq.avgtemp_all_cores_off_caches_off,
                                   l_chip_v_idx,
                                   g_wof->tempnest_sensor,
                                   l_v_chip );

    // Get the core leakage percent from the OPPB
    // Note: This value is named inaccurately in the OPPB so we are reassigning
    // it's value here. We are also making the value visible to amester here.
    g_wof->core_leakage_percent = G_oppb.nest_leakage_percent;

    // Multiply by core leakage percentage
    g_wof->all_cores_off_iso = g_wof->all_cores_off_iso *
         g_wof->core_leakage_percent / 100;

    // Calculate ALL_GOOD_CACHES_ON_ISO
    g_wof->all_good_caches_on_iso =
     scale_and_interpolate( G_oppb.iddq.ivdd_all_good_cores_off_good_caches_on,
                            G_oppb.iddq.avgtemp_all_good_cores_off,
                            l_chip_v_idx,
                            g_wof->tempnest_sensor,
                            l_v_chip ) - g_wof->all_cores_off_iso;

    // Calculate ALL_CACHES_OFF_ISO
    g_wof->all_caches_off_iso =
        scale_and_interpolate( G_oppb.iddq.ivdd_all_cores_off_caches_off,
                               G_oppb.iddq.avgtemp_all_cores_off_caches_off,
                               l_chip_v_idx,
                               g_wof->tempnest_sensor,
                               l_v_chip ) - g_wof->all_cores_off_iso;

    // idc Quad uses same variables as all_cores_off_iso so just use that and
    // divide by 6 to get just one quad
    g_wof->idc_quad = g_wof->all_cores_off_iso / MAXIMUM_QUADS;

    // Calculate quad_cache for all quads
    l_quad_cache = ( g_wof->all_good_caches_on_iso - g_wof->all_caches_off_iso *
                     ( MAXIMUM_QUADS - G_oppb.iddq.good_quads_per_sort) /
                      MAXIMUM_QUADS ) / G_oppb.iddq.good_quads_per_sort;

    // Loop through all Quads and their respective Cores to calculate
    // leakage.
    int quad_idx = 0;       // Quad Index (0-5)
    uint8_t core_idx = 0;       // Actual core index (0-23)
    int core_loop_idx = 0;  // On a per quad basis (0-3)

    for(quad_idx = 0; quad_idx < MAXIMUM_QUADS; quad_idx++)
    {
        if(g_wof->quad_x_pstates[quad_idx] == QUAD_POWERED_OFF)
        {
            // Increment the number of quads found to be off
            num_quads_off++;

        }
        else // Quad i is on
        {
            // Calculate the index of the first core in the quad.
            // so we reference the correct one in the inner core loop
            core_idx = quad_idx * NUM_CORES_PER_QUAD;

            // Get the voltage for the current core.
            // (Same for all cores within a single quad)
            uint8_t quad_v_idx = g_wof->quad_v_idx[quad_idx];

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
                    g_wof->all_cores_off_iso *
                    G_oppb.iddq.good_normal_cores[quad_idx] / 24;


                    // Calculate quad_on_cores[quad]
                    g_wof->quad_on_cores[quad_idx] =
                        (g_wof->quad_good_cores_only[quad_idx]*
                         g_wof->cores_on_per_quad[quad_idx]) /
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
            // Calculate quadx_BAD_OFF_cores
            g_wof->quad_bad_off_cores[quad_idx] =
                g_wof->all_cores_off_iso *
                G_oppb.iddq.good_normal_cores[quad_idx]/24;

            // Scale to nest temp and add to overall leakage
            idc_vdd +=
            (scale(g_wof->quad_bad_off_cores[quad_idx],
                 (g_wof->tempnest_sensor -
                 (G_oppb.iddq.avgtemp_all_cores_off_caches_off[quad_v_idx]>>1))))
            * num_cores_off_in_quad;
            // After all cores have been processed in current quad, multiply
            // the scaled value by the numer of cores that were off.

            // Incorporate the cache into leakage calculation.
            // scale from nest to quad
            idc_vdd += scale( l_quad_cache,
                        ( g_wof->tempq[quad_idx] - g_wof->tempnest_sensor) );

        } // quad on/off conditional
    } // quad loop

    // After all Quads have been processed, incorporate calculation for quads
    // that off into leakage
    idc_vdd += g_wof->idc_quad * num_quads_off;

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
uint32_t calculate_effective_capacitance( uint32_t i_iAC,
                                          uint32_t i_voltage,
                                          uint32_t i_frequency )
{
    // Prevent divide by zero
    if( i_frequency == 0 )
    {
        // Return 0 causing caller to disable wof.
        return 0;
    }

    // Compute V^1.3 using a best-fit equation
    // (V^1.3) = (21374 * (voltage in 100uV) - 50615296)>>10
    uint32_t v_exp_1_dot_3 = (21374 * i_voltage - 50615296)>>10;

    // Compute I / (V^1.3)
    uint32_t I = i_iAC << 14; // * 16384

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
    return c_eff / i_frequency;

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
    g_wof->ceff_vdn =
                calculate_effective_capacitance( g_wof->iac_vdn,
                                                 g_wof->voltvdn_sensor,
                                                 G_nest_frequency_mhz );

    // Prevent divide by zero
    if( g_wof->ceff_tdp_vdn == 0 )
    {
        CMDH_TRAC_ERR("WOF Disabled! Ceff VDN divide by 0");

        // Return 0
        g_wof->ceff_ratio_vdn = 0;

        set_clear_wof_disabled(SET, WOF_RC_DIVIDE_BY_ZERO);
    }
    else
    {
        g_wof->ceff_ratio_vdn = g_wof->ceff_vdn / g_wof->ceff_tdp_vdn;
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
    // Read iac_tdp_vdd from OCCPstateParmBlock struct
    g_wof->iac_tdp_vdd = G_oppb.lac_tdp_vdd_turbo_10ma;

    // Get Vturbo and convert to 100uV (mV -> 100uV) = mV*10
    // Multiply by Vratio
    uint32_t V = (G_oppb.operating_points[TURBO].vdd_mv*10) * g_wof->v_ratio;

    // Get Fturbo and multiply by Fratio
    uint32_t F = G_oppb.operating_points[TURBO].frequency_mhz * g_wof->f_ratio;

    // Calculate ceff_tdp_vdd
    // iac_tdp_vdd / ((Vturbo*Vratio)^1.3 * (Fturbo*Fratio))
    g_wof->ceff_tdp_vdd =
                calculate_effective_capacitance( g_wof->iac_tdp_vdd,
                                                 V,
                                                 F );

    // Calculate ceff_vdd
    // iac_vdd / (Vclip^1.3 * Fclip)
    g_wof->ceff_vdd =
                calculate_effective_capacitance( g_wof->iac_vdd,
                                                 g_wof->v_clip,
                                                 g_wof->f_clip );

    // Prevent divide by zero
    if( g_wof->ceff_tdp_vdd == 0 )
    {
        CMDH_TRAC_ERR("WOF Disabled! Ceff VDD divide by 0");

        // Return 0
        g_wof->ceff_ratio_vdd = 0;

        // Disable wof
        set_clear_wof_disabled(SET, WOF_RC_DIVIDE_BY_ZERO);
    }
    else
    {
        g_wof->ceff_ratio_vdd = g_wof->ceff_vdd / g_wof->ceff_tdp_vdd;
    }
}

/**
 *  calculate_AC_currents
 *
 *  Description: Calculate the AC currents
 */
inline void calculate_AC_currents( void )
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
 * Return: Returns TRUE if the core is powered on, FALSE otherwise
 */
inline bool core_powered_on(uint8_t i_core_num)
{
    return ( g_wof->core_pwr_on & (0x80000000 >> i_core_num));
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
        if( core_powered_on(i) )
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
inline int32_t interpolate_linear( int32_t i_X,
                                   int32_t i_x1,
                                   int32_t i_x2,
                                   int32_t i_y1,
                                   int32_t i_y2 )
{
    return (i_X - i_x1)*((i_y2 - i_y1) / (i_x2 - i_x1)) + i_y1;
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
 *        to find the appropriate row index into G_wof_iddq_mult_table.
 *
 * Return: The multiplier representing the temperature factor
 */
uint32_t calculate_multiplier( int32_t i_temp )
{
    int mult_idx;

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
 */
void set_clear_wof_disabled( uint8_t i_action,
                             uint32_t i_bit_mask )
{
    // Keep track of whether an error has already been logged
    static bool L_errorLogged = false;

    errlHndl_t l_errl = NULL;

    uint32_t prev_wof_disabled = g_wof->wof_disabled;

    if( i_action == SET )
    {
        // Set the bit
        g_wof->wof_disabled |= i_bit_mask;
        // If error has already been logged, trace and skip
        if( L_errorLogged )
        {
            CMDH_TRAC_ERR("Another WOF error was encountered!"
                          " wof_disabled=0x%08x",
                          g_wof->wof_disabled);
        }
        else
        {
            CMDH_TRAC_ERR("WOF encountered an error. wof_disabled ="
                          " 0x%08x", g_wof->wof_disabled );
            // Make sure the reason requires an error log
            if( g_wof->wof_disabled & ERRL_RETURN_CODES )
            {
                // Create error log
                /** @errortype
                 *  @moduleid   SET_CLEAR_WOF_DISABLED
                 *  @reasoncode WOF_DISABLED_RC
                 *  @userdata1  current wof_disabled
                 *  @userdata2  Bit requested to be set
                 *  @userdata4  OCC_NO_EXTENDED_RC
                 *  @devdesc    WOF has been disabled due to an error
                 */
                l_errl = createErrl(
                            SET_CLEAR_WOF_DISABLED,
                            WOF_DISABLED_RC,
                            OCC_NO_EXTENDED_RC,
                            ERRL_SEV_UNRECOVERABLE,
                            NULL,
                            DEFAULT_TRACE_SIZE,
                            g_wof->wof_disabled,
                            i_bit_mask );

                // commit the error log
                commitErrl( &l_errl );
                L_errorLogged = true;
            }
            // if the previous wof_disabled was all zeros,
            // send IPC command to PGPE to disable wof
            if( !prev_wof_disabled )
            {
                // Disable WOF
                disable_wof();
            }
        }
    }
    else if ( i_action == CLEAR )
    {
        // Clear the bit
        g_wof->wof_disabled &= ~i_bit_mask;
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
        CMDH_TRAC_ERR("Invalid action given. Ignoring for now...");
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
    // Disable wof on 405
    g_wof->wof_init_state = WOF_DISABLED;

    CMDH_TRAC_ERR("WOF is being disabled. Reasoncode: %x",
                  g_wof->wof_disabled );
    uint32_t reasonCode = 0;
    int user_data_rc = 0;
    do
    {
        // Make sure IPC command is idle
        if(async_request_is_idle(&G_wof_control_req.request))
        {

            // Check to see if a previous wof control IPC message observed an error
            if( g_wof->control_ipc_rc != 0 )
            {
                CMDH_TRAC_ERR("Unknown error from wof control IPC message");
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
                user_data_rc = gpe_request_schedule( &G_wof_control_req );

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
                    CMDH_TRAC_ERR("disable_wof() - Error when sending WOF Control"
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
    CMDH_TRAC_ERR("WOF is being enabled...");
    uint32_t reasonCode = 0;
    bool result = true;
    uint32_t bit_to_set = 0;
    int rc = 0;
    // Make sure IPC command is idle.
    if(!async_request_is_idle( &G_wof_control_req.request ) )
    {
        result = false;;
    }
    else
    {
        // Check to see if a previous wof control IPC message observed an error
        if( g_wof->control_ipc_rc != 0 )
        {
            CMDH_TRAC_ERR("Unknown error from wof control IPC message");
            rc = g_wof->control_ipc_rc;
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

            rc = gpe_request_schedule( &G_wof_control_req );

            if( rc != 0 )
            {
                CMDH_TRAC_ERR("enable_wof() - Error when sending WOF Control"
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
                reasonCode = GPE_REQUEST_SCHEDULE_FAILURE;
            }
            else
            {
                // Set Init state
                CMDH_TRAC_INFO("wof control on sent waiting!");
                g_wof->wof_init_state = WOF_CONTROL_ON_SENT_WAITING;
                result = true;
            }
        }
        if( rc != 0 )
        {
            errlHndl_t l_errl = createErrl(
                    ENABLE_WOF,
                    reasonCode,
                    OCC_NO_EXTENDED_RC,
                    ERRL_SEV_PREDICTIVE,
                    NULL,
                    DEFAULT_TRACE_SIZE,
                    rc,
                    0);

            result = false;
            set_clear_wof_disabled( SET, bit_to_set );

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
        g_wof->pgpe_wof_off = 1;
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
            g_wof->pgpe_wof_off = 1;
        }
    }
    else
    {
        g_wof->control_ipc_rc = G_wof_control_parms.msg_cb.rc;
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
    g_wof->quad_step_from_start = g_wof->num_active_quads - 1;

    // Calculate the address of the final vfrt
    g_wof->next_vfrt_main_mem_addr = calc_vfrt_mainstore_addr();

    // Send the final vfrt to shared OCC-PGPE SRAM.
    send_vfrt_to_pgpe( g_wof->next_vfrt_main_mem_addr );

    // Update Init state
    g_wof->wof_init_state = INITIAL_VFRT_SENT_WAITING;
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
    memcpy(&g_wof->req_active_quad_update, &l_doubleword, sizeof(uint8_t));

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
 * Desctiption: Using the input voltage, returns the index to the lower bound
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
 * Description: Performs i_current*IDDQ_TEMP_FACTOR^(i_delta_temp)/10
 *              where IDDQ_TEMP_FACTOR == 1.3
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
 * Return: The approximated scaled current.
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
    uint32_t scaled_lower_leak = scale( i_leak_arr[i_idx],
                                        lower_delta );
    uint32_t scaled_upper_leak = scale( i_leak_arr[i_idx],
                                        upper_delta );

    // Approximate current between the scaled currents using linear
    // interpolation and return the result
    return interpolate_linear( (int32_t) i_voltage,
                               (int32_t) G_iddq_voltages[i_idx],
                               (int32_t) G_iddq_voltages[i_idx+1],
                               (int32_t) scaled_lower_leak,
                               (int32_t) scaled_upper_leak );

}
