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
#include "wof.h"


//******************************************************************************
// Globals
//******************************************************************************
uint32_t G_wof_active_quads_sram_addr;
uint32_t G_wof_tables_main_mem_addr;
uint32_t G_wof_tables_len;
uint8_t  G_requested_active_quad_update;
uint8_t  G_previous_active_quads;
bool     G_run_wof_main;

uint8_t G_sram_vfrt_ping_buffer[MIN_BCE_REQ_SIZE] __attribute__ ((section(".vfrt_ping_buffer")));
uint8_t G_sram_vfrt_pong_buffer[MIN_BCE_REQ_SIZE] __attribute__ ((section(".vfrt_pong_buffer")));
uint8_t * G_current_ping_pong_buf;
wof_header_data_t G_wof_header __attribute__ ((section (".global_data")));
uint32_t G_current_vfrt_addr = 0;
quad_state0_t G_quad_state_0 = {0};
quad_state0_t G_quad_state_1 = {0};

//******************************************************************************
// External Globals
//******************************************************************************
extern OCCPstateParmBlock G_oppb;
extern GPE_BUFFER(ipcmsg_wof_vfrt_t G_wof_vfrt_parms);
extern GpeRequest G_wof_vfrt_req;
extern uint32_t   G_pgpe_shared_sram_address;


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

    // TODO Read out necessary Sensor data for WOF calculation
    //    uint16_t l_current_vdd = getSensorByGsid(CURVDD)->sample;
    //    uint16_t l_current_vdn = getSensorByGsid(CURVDN)->sample
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
    TRAC_INFO("Step from start VDN = %d, VDN = %d",
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
    uint32_t wof_tables_base = G_wof_tables_main_mem_addr + WOF_HEADER_SIZE;

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
    static uint8_t L_pingpong = 0;
    uint8_t * l_buffer_address;

    if( L_pingpong == 0 )
    {
        // Use ping buffer
        l_buffer_address = G_sram_vfrt_ping_buffer;
    }
    else
    {
        // Use pong buffer
        l_buffer_address = G_sram_vfrt_pong_buffer;
    }

    // Copy the vfrt data into the buffer
    memcpy( l_buffer_address,
            &(i_parms->vfrt_table->data[i_parms->pad]),
            G_wof_header.size_of_vfrt );

    // Set the parameters for the GpeRequest
    G_wof_vfrt_parms.vfrt_ptr = l_buffer_address;
    G_wof_vfrt_parms.active_quads = G_requested_active_quad_update;

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
    else
    {
        // Sent the IPC command successfully, update which buffer we should look
        // at next time.
        L_pingpong = ~L_pingpong;

        // Update the previous active quads
        G_previous_active_quads = G_requested_active_quad_update;

        // Update Current ping pong buffer
        G_current_ping_pong_buf = l_buffer_address;
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
 * Param[in]: i_vfrt_address - Address of the desired vfrt table.
 */
void send_vfrt_to_pgpe( uint32_t i_vfrt_address )
{
    int l_ssxrc = SSX_OK;
    int l_gperc = 0;
    uint32_t l_reasonCode = 0;
    uint32_t l_extReasonCode = 0;

    do
    {
        if( (i_vfrt_address == G_current_vfrt_addr ) &&
            (G_requested_active_quad_update == G_previous_active_quads) )
        {
            // VFRT and requested active quads are unchanged. Skip
            break;
        }
        else if( (i_vfrt_address == G_current_vfrt_addr) &&
                 (G_requested_active_quad_update != G_previous_active_quads) )
        {
            // Only requested active quads changed. No need to do a BCE request
            // for new VFRT. Just send IPC command with updated active quads
            G_wof_vfrt_parms.vfrt_ptr = G_current_ping_pong_buf;
            G_wof_vfrt_parms.active_quads = G_requested_active_quad_update;

            //Send IPC command to PGPE with new active quad update
            l_gperc = gpe_request_schedule( &G_wof_vfrt_req );

            // Confirm Successful scheduling of WOF VFRT task
            if(l_gperc != 0)
            {
                //Error in scheduling pgpe clip update task
                TRAC_ERR("copy_vfrt_to_sram: Failed to schedule WOF VFRT task rc=%x",
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
            else
            {
                // Successful Schedule. Update previous active quads
                G_previous_active_quads = G_requested_active_quad_update;
            }
        }
        else
        {
            // New VFRT needed from Mainstore, create BCE request to get it.
            BceRequest l_vfrt_req;

            // 128-byte aligned temp buffer to hold data
            temp_bce_request_buffer_t l_temp_bce_buff = {{0}};


            uint8_t l_pad = i_vfrt_address%128;
            uint32_t l_vfrt_addr_128_aligned = i_vfrt_address - l_pad;

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
                CMDH_TRAC_ERR("read_wof_header: BCDE request create failure rc=[%08X]", -l_ssxrc);
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
                CMDH_TRAC_ERR("read_wof_header: BCE request schedule failure rc=[%08X]", -l_ssxrc);
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
    memcpy(&G_requested_active_quad_update, &l_doubleword, sizeof(uint8_t));

}
