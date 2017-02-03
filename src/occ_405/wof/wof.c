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
#include "wof.h"

//******************************************************************************
// External Globals
//******************************************************************************
extern amec_sys_t g_amec_sys;
extern OCCPstateParmBlock G_oppb;
extern GPE_BUFFER(ipcmsg_wof_vfrt_t G_wof_vfrt_parms);
extern GpeRequest G_wof_vfrt_req;
extern uint32_t   G_pgpe_shared_sram_address;
extern uint32_t   G_pgpe_pstate_table_address;
extern uint32_t   G_pgpe_pstate_table_sz;
extern uint32_t   G_nest_frequency_mhz;
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

    // Functions to calculate Ceff_vdd and Ceff_vdn here.
    uint16_t ceff_vdd = 0; // TODO: replace with future function call
    uint16_t ceff_vdn = 0; // TODO: replace with future function call


    // Calculate how many steps from the beginning for VDD and VDN
    uint16_t vdn_step_from_start =
                             calculate_step_from_start( ceff_vdd,
                                                        G_wof_header.vdn_step,
                                                        G_wof_header.vdn_start,
                                                        G_wof_header.vdn_size );

    uint16_t vdd_step_from_start =
                             calculate_step_from_start( ceff_vdn,
                                                        G_wof_header.vdd_step,
                                                        G_wof_header.vdd_start,
                                                        G_wof_header.vdd_size );

    // TODO: REMOVE THESE TRACES
    // NOTE to Reviewers: This trace is here just to put references to the above
    // variables such that compilation is successful. Will be removed in final
    // version
    TRAC_INFO("Step from start VDN = %d, VDD = %d",
              vdn_step_from_start,
              vdd_step_from_start );


}


/**
 *  calculate_step_from_start
 *
 *  Description: Calculates the step number for the current VDN/VDD
 *
 *  Param[in]: i_ceff_vdx - The current Ceff_vdd or Ceff_vdn to calculate the step
 *                      for.
 *  Param[in]: i_step_size - The size of each step.
 *  Param[in]: i_min_ceff - The minimum step number for this VDN/VDD
 *  Param[in]: i_max_step - The maximum step number for this VDN/VDD
 *
 *  Return: The calculated step for current Ceff_vdd/Ceff_vdn
 */
uint16_t calculate_step_from_start(uint16_t i_ceff_vdx,
                                   uint8_t i_step_size,
                                   uint8_t i_min_ceff,
                                   uint8_t i_max_step )
{
    uint16_t l_current_step;

    // Ensure ceff is at least the min step
    if( (i_ceff_vdx <= i_min_ceff) || (i_step_size == 0) )
    {
        l_current_step = 0;
    }
    else
    {
        // Add step size to current vdd/vdn to round up.
        //  -1 to prevent overshoot when i_ceff_vdx is equal to Ceff table value
        l_current_step = i_ceff_vdx + i_step_size - 1;

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
 * Param[in]: i_num_active_quads - The current number of active quads.
 *
 * Return: The calculated step for current active quads
 */
uint8_t calc_quad_step_from_start( uint8_t i_num_active_quads )
{
    return (G_wof_header.active_quads_size == ACTIVE_QUAD_SZ_MIN) ? 0 :
                                              (i_num_active_quads - 1);
}

/**
 * calc_vfrt_address
 *
 * Description: Calculates the VFRT address based on the Ceff vdd/vdn and quad
 *              steps.
 *
 * Param[in]: i_vdd_step_from_start
 * Param[in]: i_vdn_step_from_start
 * Param[in]: i_quad_step_from_start
 *
 * Return: The desired VFRT address
 */
uint32_t calc_vfrt_mainstore_addr( uint16_t i_vdd_step_from_start,
                            uint16_t i_vdn_step_from_start,
                            uint8_t i_quad_step_from_start )
{

    // Wof tables address calculation
    // (Base_addr + (sizeof VFRT * (total active quads * ( (i_vdn_step_from_start * vdd_size) + (i_vdd_step_from_start) ) + (i_quad_step_from_start))))
    uint32_t offset = G_wof_header.size_of_vfrt *
                    (( G_wof_header.active_quads_size *
                    ((i_vdn_step_from_start * G_wof_header.vdd_size) +
                    i_vdd_step_from_start) ) + i_quad_step_from_start);

    // Skip the wof header at the beginning of wof tables
    uint32_t wof_tables_base = g_wof->vfrt_tbls_main_mem_addr + WOF_HEADER_SIZE;

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
 *                      -Padding if the address needed to be 128-byte aligned
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
            &(i_parms->vfrt_table->data[i_parms->pad]),
            G_wof_header.size_of_vfrt );

    // Set the parameters for the GpeRequest
    G_wof_vfrt_parms.vfrt_ptr = l_buffer_address;
    G_wof_vfrt_parms.active_quads = g_wof->req_active_quad_update;

    // Send IPC command to PGPE with new vfrt address and active quads
    // Should not need to check if request is idle as wof_main does before
    // the WOF calculations begin.
    l_gperc = gpe_request_schedule( &G_wof_vfrt_req );

    // Confirm Successful scheduling of WOF VFRT task
    if(l_gperc != 0)
    {
        //Error in scheduling pgpe clip update task
        TRAC_ERR("copy_vfrt_to_sram: Failed to schedule WOF VFRT task rc=%x",
                 l_gperc);

        /* @
         * @errortype
         * @moduleid    COPY_VFRT_TO_SRAM
         * @reasoncode  GPE_REQUEST_SCHEDULE_FAILURE
         * @userdata1   rc - gpe_request_schedule return code
         * @userdata2   0
         * @userdata4   OCC_NO_EXTENDED_RC
         * @devdesc     OCC Failed to schedule a GPE job for clip update
         */
        errlHndl_t l_errl = createErrl(
            COPY_VFRT_TO_SRAM,                   // modId
            GPE_REQUEST_SCHEDULE_FAILURE,           // reasoncode
            OCC_NO_EXTENDED_RC,                     // Extended reason code
            ERRL_SEV_UNRECOVERABLE,                 // Severity
            NULL,                                   // Trace Buf
            DEFAULT_TRACE_SIZE,                     // Trace Size
            l_gperc,                                     // userdata1
            0                                       // userdata2
            );

        // Callout firmware
        addCalloutToErrl(l_errl,
                         ERRL_CALLOUT_TYPE_COMPONENT_ID,
                         ERRL_COMPONENT_ID_FIRMWARE,
                         ERRL_CALLOUT_PRIORITY_HIGH);

        // Commit error log
        commitErrl(&l_errl);
    }

}

/**
 * switch_ping_pong_buffer
 *
 * Description: Callback function for G_wof_vfrt_req GPE request to
 *              confirm the new VFRT is being used by the PGPE and
 *              record the switch on the 405
 */
//TODO RTC 166301 - will be renamed wof_vfrt_req_callback
void switch_ping_pong_buffer( void )
{
    // Confirm the WOF VFRT PGPE request has completed with no errors
    if( G_wof_vfrt_parms.msg_cb.rc == PGPE_WOF_RC_VFRT_QUAD_MISMATCH )
    {
        // TODO RTC 166301 - Implement this logic with read_req_active_quads
        //                   function.
        // Quad Mismatch.
        // Reread the requested active quads from Shared SRAM and set
        // parameters
        // Trace the mismatch and let the code exit. Will retry
        // next invocation of wof_main.
        // Keep retrying if this return code is seen
    }
    else if( G_wof_vfrt_parms.msg_cb.rc == PGPE_RC_SUCCESS )
    {
       // GpeRequest went through successfully. update global ping pong buffer
       g_wof->curr_ping_pong_buf = g_wof->next_ping_pong_buf;

       // Update previous active quads
       g_wof->prev_req_active_quads = g_wof->req_active_quad_update;
    }
    else
    {
        // Some other failure case. Reset PM complex.
        /* @
         * @errortype
         * @moduleid    WOF_VFRT_CALLBACK
         * @reasoncode  WOF_VFRT_REQ_FAILURE
         * @userdata1   The GpeRequest RC
         * @userdata4   OCC_NO_EXTENDED_RC
         */
        errlHndl_t l_errl = createErrl( WOF_VFRT_CALLBACK,
                                        WOF_VFRT_REQ_FAILURE,
                                        OCC_NO_EXTENDED_RC,
                                        ERRL_SEV_UNRECOVERABLE,
                                        NULL,
                                        DEFAULT_TRACE_SIZE,
                                        G_wof_vfrt_parms.msg_cb.rc,
                                        0 );

        REQUEST_RESET( l_errl );
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
    int l_gperc = 0;
    uint32_t l_reasonCode = 0;
    uint32_t l_extReasonCode = 0;

    do
    {
        if( (i_vfrt_main_mem_addr == g_wof->curr_vfrt_main_mem_addr ) &&
            (g_wof->req_active_quad_update ==
             g_wof->prev_req_active_quads) )
        {
            // VFRT and requested active quads are unchanged. Skip
            break;
        }
        else if( (i_vfrt_main_mem_addr == g_wof->curr_vfrt_main_mem_addr)&&
                 (g_wof->req_active_quad_update !=
                  g_wof->prev_req_active_quads) )
        {
            // Only requested active quads changed. No need to do a BCE request
            // for new VFRT. Just send IPC command with updated active quads
            G_wof_vfrt_parms.vfrt_ptr = (VFRT_Hcode_t*)g_wof->curr_ping_pong_buf;
            G_wof_vfrt_parms.active_quads = g_wof->req_active_quad_update;

            //Send IPC command to PGPE with new active quad update
            l_gperc = gpe_request_schedule( &G_wof_vfrt_req );

            // Confirm Successful scheduling of WOF VFRT task
            if(l_gperc != 0)
            {
                //Error in scheduling pgpe clip update task
                TRAC_ERR("send_vfrt_to_sram: Failed to schedule WOF VFRT task rc=%x",
                         l_gperc);

                /* @
                 * @errortype
                 * @moduleid    SEND_VFRT_TO_PGPE
                 * @reasoncode  GPE_REQUEST_SCHEDULE_FAILURE
                 * @userdata1   rc - gpe_request_schedule return code
                 * @userdata2   0
                 * @userdata4   OCC_NO_EXTENDED_RC
                 * @devdesc     OCC Failed to schedule a GPE job for clip update
                 */
                errlHndl_t l_errl = createErrl(
                    SEND_VFRT_TO_PGPE,                   // modId
                    GPE_REQUEST_SCHEDULE_FAILURE,           // reasoncode
                    OCC_NO_EXTENDED_RC,                     // Extended reason code
                    ERRL_SEV_UNRECOVERABLE,                 // Severity
                    NULL,                                   // Trace Buf
                    DEFAULT_TRACE_SIZE,                     // Trace Size
                    l_gperc,                                     // userdata1
                    0                                       // userdata2
                    );

                // Callout firmware
                addCalloutToErrl(l_errl,
                                 ERRL_CALLOUT_TYPE_COMPONENT_ID,
                                 ERRL_COMPONENT_ID_FIRMWARE,
                                 ERRL_CALLOUT_PRIORITY_HIGH);

                // Commit error log
                commitErrl(&l_errl);
            }
        }
        else
        {
            // New VFRT needed from Mainstore, create BCE request to get it.
            BceRequest l_vfrt_req;

            // 128-byte aligned temp buffer to hold data
            temp_bce_request_buffer_t l_temp_bce_buff = {{0}};


            uint8_t l_pad = i_vfrt_main_mem_addr%128;
            uint32_t l_vfrt_addr_128_aligned = i_vfrt_main_mem_addr - l_pad;

            // Create structure to hold parameters for callback function
            copy_vfrt_to_sram_parms_t l_callback_parms;
            l_callback_parms.vfrt_table = &l_temp_bce_buff;
            l_callback_parms.pad = l_pad;


            // Create request
            l_ssxrc = bce_request_create(
                             &l_vfrt_req,                 // block copy object
                             &G_pba_bcde_queue,           // main to sram copy engine
                             l_vfrt_addr_128_aligned,     //mainstore address
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
        errlHndl_t l_errl = createErrl(SEND_VFRT_TO_PGPE,         //modId
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
    // Skip over the first doubleword for now (magic number and pgpe beacon)
    uint32_t current_pgpe_sram_addr =
                     G_pgpe_shared_sram_address + sizeof(uint64_t);

    // Get the actual quad states
    G_quad_state_0.value = in64(current_pgpe_sram_addr);
    current_pgpe_sram_addr += sizeof(uint64_t);
    G_quad_state_1.value = in64(current_pgpe_sram_addr);
    current_pgpe_sram_addr += sizeof(uint64_t);

    // Get the requested active quad update
    uint64_t l_doubleword = in64(current_pgpe_sram_addr);
    memcpy(&g_wof->req_active_quad_update, &l_doubleword, sizeof(uint8_t));

    // merge the 16-bit power-on field from quad state 0 and the 16-bit power-on
    // field from quad state 1 and save it to amec.
    g_wof->core_pwr_on =
                   (((uint32_t)G_quad_state_0.fields.core_poweron_state) << 16)
                  | ((uint32_t)G_quad_state_1.fields.core_poweron_state);


    // Clear out current quad pstates
    memset(g_wof->quad_x_pstates, 0 , MAX_NUM_QUADS);

    // Add the quad states to the global quad state array for easy looping.
    g_wof->quad_x_pstates[0] = (uint8_t)G_quad_state_0.fields.quad0_pstate;
    g_wof->quad_x_pstates[1] = (uint8_t)G_quad_state_0.fields.quad1_pstate;
    g_wof->quad_x_pstates[2] = (uint8_t)G_quad_state_0.fields.quad2_pstate;
    g_wof->quad_x_pstates[3] = (uint8_t)G_quad_state_0.fields.quad3_pstate;
    g_wof->quad_x_pstates[4] = (uint8_t)G_quad_state_1.fields.quad4_pstate;
    g_wof->quad_x_pstates[5] = (uint8_t)G_quad_state_1.fields.quad5_pstate;


    // Save IVRM bit vector states to amec
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
    int l_quad_idx = 0;
    for(; l_quad_idx < MAX_NUM_QUADS; l_quad_idx++)
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
            uint32_t pstate_addr = G_pgpe_pstate_table_address +
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

    int l_chip_v_idx = 0;
    uint16_t l_quad_x_cache;
    uint16_t idc_vdd = 0;
    uint8_t  num_quads_off = 0;
    uint16_t temperature = 0;

    // Get the VOLTVDDSENSE sensor and choose the appropriate
    // chip voltage index
    uint32_t l_v_chip = g_wof->voltvddsense_sensor;

    if( l_v_chip <= G_iddq_voltages[0] )
    {
        // Voltage is <= to first entry. Use first two entries.
        l_chip_v_idx = 0;
    }
    else if( l_v_chip >= G_iddq_voltages[CORE_IDDQ_MEASUREMENTS-1] )
    {
        // Voltage is >= to last entry. Use last two entries.
        l_chip_v_idx = CORE_IDDQ_MEASUREMENTS - 2;
    }
    else
    {
        // Search for entries on either side of our voltage
        for(;l_chip_v_idx < CORE_IDDQ_MEASUREMENTS - 1; l_chip_v_idx++)
        {
            if( (l_v_chip >= G_iddq_voltages[l_chip_v_idx]) &&
                (l_v_chip <= G_iddq_voltages[l_chip_v_idx]) )
            {
                break;
            }
        }

    }
    // Save index used for interpolating voltages to amec
    g_wof->voltage_idx = l_chip_v_idx;


    // Calculate all variables that will be used in the core
    // loop that only need to be calculated once.

    // Look up Tvpd_leak for calculations when either the core or quad is off
    // avttemp values in 0.5C. Divide by 2 to convert to 1C
    g_wof->tvpd_leak_off =
                  G_oppb.iddq.avgtemp_all_cores_off_caches_off[l_chip_v_idx] >> 1;

    // Look up Tvpd_leak for calculations involving the cache.
    g_wof->tvpd_leak_cache =
              G_oppb.iddq.avgtemp_all_good_cores_off[l_chip_v_idx] >> 1;

    // Take the difference between the temperature and tvpd_leak_off
    // used for multiplier calculation
    g_wof->nest_delta_temp = g_wof->tempnest_sensor -
                                  g_wof->tvpd_leak_off;

    // Calculate IDDQ_TEMP_FACTOR^((TEMPNEST - tvpd_leak)/10)
    g_wof->nest_mult = calculate_multiplier(g_wof->nest_delta_temp);


    // Look up leakage current.
    // Divide by 6 to get just one quad
    g_wof->idc_quad =
                 G_oppb.iddq.ivdd_all_cores_off_caches_off[l_chip_v_idx] /
                 MAX_NUM_QUADS;




    // Calculate ALL_CORES_OFF_ISO
    // Perform linear interpolation using the neighboring entries:
    // Y = m*(X-x1) + y1, where m = (y2-y1) / (x2-x1)
    g_wof->all_cores_off_iso =
         interpolate_linear((int32_t)l_v_chip,
            (int32_t)G_iddq_voltages[l_chip_v_idx],
            (int32_t)G_iddq_voltages[l_chip_v_idx+1],
            (int32_t)G_oppb.iddq.ivdd_all_cores_off_caches_off[l_chip_v_idx],
            (int32_t)G_oppb.iddq.ivdd_all_cores_off_caches_off[l_chip_v_idx+1]);

    // Multiply by nest leakage percentage
    // TODO: This percentage(60%) will eventually be added to the OCC Pstate Parameter
    // block once it is added as a system attribute to the MRW.
    // G_oppb.iddq.nestLeakagePercentage
    g_wof->all_cores_off_iso = g_wof->all_cores_off_iso * 60 / 100;


    // Calculate ALL_CACHES_ON_ISO
    g_wof->all_caches_on_iso =
              G_oppb.iddq.ivdd_all_good_cores_off_good_caches_on[l_chip_v_idx] -
              g_wof->all_cores_off_iso;

    l_quad_x_cache = g_wof->all_caches_on_iso / MAX_NUM_QUADS;

    // Loop through all Quads and their respective Cores to calculate
    // leakage.
    int quad_idx = 0;       // Quad Index (0-5)
    uint8_t core_idx = 0;       // Actual core index (0-23)
    int core_loop_idx = 0;  // On a per quad basis (0-3)



    for(quad_idx = 0; quad_idx < MAX_NUM_QUADS; quad_idx++)
    {
        if(g_wof->quad_x_pstates[quad_idx] == QUAD_POWERED_OFF)
        {
            // Increment the number of quads found to be off
            num_quads_off++;

        }
        else // Quad i is on
        {
            // Calculate the index of the first core in the quad.
            core_idx = quad_idx * NUM_CORES_PER_QUAD;

            // Get the voltage for the current core.
            // (Same for all cores within a single quad)
            uint16_t cur_core_voltage = g_wof->v_core_100uV[quad_idx];

            // Calculate the number of cores on within the current quad.
            g_wof->cores_on_per_quad[quad_idx] =
                                        num_cores_on_in_quad(quad_idx);

            // Look up tvpd_leak_on for calculations when the core/quad is on
            // avttemp in IDDQ table is in 0.5C. Divide by 2 to convert to 1C.
            g_wof->tvpd_leak_on = G_oppb.iddq.avgtemp_quad_good_cores_on
                                               [quad_idx][cur_core_voltage] >> 1;


            // Calculate Quadx_good_cores_only
            g_wof->quad_good_cores_only[quad_idx] =
                    G_oppb.iddq.ivdd_quad_good_cores_on_good_caches_on
                                    [quad_idx][cur_core_voltage] -
                    G_oppb.iddq.ivdd_all_good_cores_off_good_caches_on
                                                        [l_chip_v_idx] +
                    g_wof->all_cores_off_iso*
                    G_oppb.iddq.good_normal_cores[quad_idx]/24;


            // Calculate quadx_ON_cores
            g_wof->quad_on_cores[quad_idx] =
                    (g_wof->quad_good_cores_only[quad_idx]*
                     g_wof->cores_on_per_quad[quad_idx]) /
                     G_oppb.iddq.good_normal_cores[quad_idx];


            // Calculate quadx_BAD_OFF_cores
            g_wof->quad_bad_off_cores[quad_idx] =
                g_wof->all_cores_off_iso*G_oppb.iddq.good_normal_cores[quad_idx]/24;

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

                    // If the TEMPPROCTHRMCy is 0, use TEMPQx
                    if(temperature == 0)
                    {
                        temperature = AMECSENSOR_ARRAY_PTR(TEMPQ0,
                                                           quad_idx)->sample;
                        // If TEMPQx is also 0, use TEMPNEST
                        if(temperature == 0)
                        {
                            temperature = g_wof->tempnest_sensor;
                        }
                    }

                    // Save the selected temperature
                    g_wof->tempprocthrmc[core_idx] = temperature;



                    // Get the difference between the temperature and tvpd_leak
                    g_wof->core_delta_temp[core_idx] =
                                          g_wof->tempprocthrmc[core_idx] -
                                          g_wof->tvpd_leak_on;

                    // Calculate the multiplier for the core
                    g_wof->core_mult[core_idx] =
                       calculate_multiplier(g_wof->core_delta_temp[core_idx]);

                    // For each core, incorporate core on calculation into
                    // leakage
                    idc_vdd += (g_wof->quad_on_cores[quad_idx]*
                                 g_wof->core_mult[core_idx]) >> 10;

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
            // incorporate calculation for cores that were off into leakage
            idc_vdd +=
               ((g_wof->quad_bad_off_cores[quad_idx]*g_wof->nest_mult)
                                                >> 10)* num_cores_off_in_quad;

            temperature = AMECSENSOR_ARRAY_PTR(TEMPQ0,
                                               quad_idx)->sample;

            // If TEMPQ0 is 0, use TEMPNEST
            if( temperature == 0 )
            {
                temperature = g_wof->tempnest_sensor;
            }

            // Save selected temperature off to amec
            g_wof->tempq[quad_idx] = temperature;


            // Get the quad delta temperature for cache calc
            g_wof->quad_delta_temp[quad_idx] =
                                 g_wof->tempq[quad_idx] -
                                 g_wof->tvpd_leak_cache;

            //Calculate the multiplier for the quad
            g_wof->quad_mult[quad_idx] =
                   calculate_multiplier(g_wof->quad_delta_temp[quad_idx]);

            // Incorporate the cache into the leakage calculation
            idc_vdd += (l_quad_x_cache*g_wof->quad_mult[quad_idx]) >> 10;


        }
    } // quad loop
    // After all Quads have been processed, incorporate calculation for quads
    // that off into leakage
    idc_vdd += ((g_wof->idc_quad*g_wof->nest_mult) >> 10)* num_quads_off;


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
    // Get the desired tvpd leak for nest calculation
    // avgtemp in IDDQ table is 0.5C units. Divide by 2 to get 1C
    g_wof->tvpd_leak_nest = G_oppb.iddq.avgtemp_vdn >> 1;

    // Subtract tvpd_leak from TEMPNEST sensor
    int16_t nest_delta_temp = g_wof->tempnest_sensor - g_wof->tvpd_leak_nest;

    // Calculate multiplier for final calculation;
    uint32_t nest_mult = calculate_multiplier( nest_delta_temp );

    // Save nest leakage to amec structure
    g_wof->idc_vdn = (G_oppb.iddq.ivdn*nest_mult) >> 10;
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
    //TODO Read iac_tdp_vdn(@turbo) from OCCPstateParmBlock struct
    g_wof->iac_tdp_vdn = 0;

    // Calculate Ceff_tdp_vdn
    // iac_tdp_vdn / (VOLTVDN^1.3 * Fnest)
    g_wof->ceff_tdp_vdn =
                calculate_effective_capacitance( g_wof->iac_tdp_vdn,
                                                 g_wof->voltvdn_sensor,
                                                 G_nest_frequency_mhz );

    // Calculate ceff_vdn
    // iac_vdn/ (VOLTVDN^1.3 * Fnest)
    g_wof->ceff_vdn =
                calculate_effective_capacitance( g_wof->iac_vdn,
                                                 g_wof->voltvdn_sensor,
                                                 G_nest_frequency_mhz );

    // Prevent divide by zero
    if( g_wof->ceff_tdp_vdn == 0 )
    {
        /*
         * @errortype
         * @moduleid        CALC_CEFF_RATIO_VDN
         * @reasoncode      DIVIDE_BY_ZERO_ERROR
         * @userdata1       0
         * @userdata4       OCC_NO_EXTENDED_RC
         * @devdesc         Divide by zero error on ceff_vdn / ceff_tdp_vdn
         */
        errlHndl_t l_errl = createErrl(
                        CALC_CEFF_RATIO_VDN,
                        DIVIDE_BY_ZERO_ERROR,
                        OCC_NO_EXTENDED_RC,
                        ERRL_SEV_PREDICTIVE,
                        NULL,
                        DEFAULT_TRACE_SIZE,
                        0,
                        0 );

        commitErrl( &l_errl );

        // Return 0
        g_wof->ceff_ratio_vdn = 0;

        //TODO: RTC 166301 - call new disable_wof function
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
    //TODO read iac_tdp_vdd from OCCPstateParmBlock struct
    g_wof->iac_tdp_vdd = 0;

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
        /*
         * @errortype
         * @moduleid        CALC_CEFF_RATIO_VDD
         * @reasoncode      DIVIDE_BY_ZERO_ERROR
         * @userdata1       0
         * @userdata4       OCC_NO_EXTENDED_RC
         * @devdesc         Divide by zero error on ceff_vdd / ceff_tdp_vdd
         */
        errlHndl_t l_errl = createErrl(
                        CALC_CEFF_RATIO_VDD,
                        DIVIDE_BY_ZERO_ERROR,
                        OCC_NO_EXTENDED_RC,
                        ERRL_SEV_PREDICTIVE,
                        NULL,
                        DEFAULT_TRACE_SIZE,
                        0,
                        0 );

        commitErrl( &l_errl );

        // Return 0
        g_wof->ceff_ratio_vdd = 0;

        //TODO: RTC 166301 - call new disable_wof function
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
int32_t calculate_multiplier( int32_t i_temp )
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
