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
#include "wof.h"


//******************************************************************************
// Globals
//******************************************************************************
uint32_t G_wof_active_quads_sram_addr;
uint32_t G_wof_tables_main_mem_addr;
uint32_t G_wof_tables_len;
bool     G_run_wof_main;

wof_header_data_t G_wof_header __attribute__ ((section (".global_data")));



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
