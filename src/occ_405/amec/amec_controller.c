/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/amec/amec_controller.c $                          */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2025                        */
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

//*************************************************************************
// Includes
//*************************************************************************
#include <occ_common.h>
#include <sensor.h>
#include <amec_sys.h>
#include <memory.h>
#include <proc_data.h>

//*************************************************************************
// Externs
//*************************************************************************
extern dimm_sensor_flags_t G_dimm_temp_expired_bitmap;
extern uint16_t G_membuf_temp_expired_bitmap;
extern uint8_t G_ocmb_dts_type_expired_bitmap;

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************

//*************************************************************************
// Structures
//*************************************************************************

//*************************************************************************
// Globals
//*************************************************************************

//*************************************************************************
// Function Prototypes
//*************************************************************************

//*************************************************************************
// Functions
//*************************************************************************
// Function Specification
//
// Name: amec_controller_proc_thermal
//
// Description: This function implements the Proportional Controller for the
//              processor thermal control. Although it doesn't return any
//              results, it populates the thermal vote in the field
//              g_amec->thermalproc.speed_request.
//
// Task Flags:
//
// End Function Specification
void amec_controller_proc_thermal()
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    uint16_t                      l_thermal_winner = 0;
    uint16_t                      l_residue = 0;
    uint16_t                      l_old_residue = 0;
    uint16_t                      l_freq_request = 0;
    int16_t                       l_error = 0;
    int16_t                       l_cpu_speed = 0;
    int16_t                       l_throttle_chg = 0;
    int32_t                       l_throttle = 0;
    sensor_t                    * l_sensor = NULL;
    static uint16_t               L_last_thermal_winner = 0;
    static uint16_t               L_last_freq_request = 0;
    static uint16_t               L_num_ticks_passed = MAX_NUM_TICKS; // num ticks since last freq change
    static bool                   L_freq_increase = TRUE; // last freq request was a freq increase

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    // average temp is not used, just save for debug
    l_sensor = getSensorByGsid(TEMPPROCAVG);
    g_amec->thermalproc.avg_temp = l_sensor->sample;

    // Get TEMPPROCTHRM sensor, which is hottest core temperature in OCC
    // processor
    l_sensor = getSensorByGsid(TEMPPROCTHRM);
    // save the temperature used by control loop for debug
    g_amec->thermalproc.current_temp = l_sensor->sample;

    // Use the highest temperature of all processors in 0.1 degrees C
    l_thermal_winner = l_sensor->sample * 10;

    // Check if there is an error
    if (g_amec->thermalproc.setpoint == l_thermal_winner)
    {
        if((G_internal_flags & INT_FLAG_ENABLE_EVERY_TICK_TEMP_DVFS) &&
           (L_num_ticks_passed < MAX_NUM_TICKS))
        {
            L_num_ticks_passed++;
        }
        return;
    }

    // Calculate the thermal control error
    l_error = g_amec->thermalproc.setpoint - l_thermal_winner;

    // Proportional Controller for the thermal control loop
    l_throttle = (int32_t) l_error * g_amec->thermalproc.Pgain;
    l_residue = (uint16_t) l_throttle;
    l_throttle_chg = (int16_t) (l_throttle >> 16);

    if ((int16_t) l_throttle_chg > (int16_t) g_amec->sys.speed_step_limit)
    {
        l_throttle_chg = g_amec->sys.speed_step_limit;
    }
    else
    {
        if ((int16_t) l_throttle_chg < ((int16_t) (-g_amec->sys.speed_step_limit)))
        {
            l_throttle_chg = (int16_t)(-g_amec->sys.speed_step_limit);
        }
    }

    // Calculate the new thermal CPU speed request
    l_cpu_speed = g_amec->thermalproc.speed_request +
        (int16_t) l_throttle_chg * g_amec->sys.speed_step;

    // Proceed with residue summation to correctly follow set-point
    l_old_residue = g_amec->thermalproc.total_res;
    g_amec->thermalproc.total_res += l_residue;
    if (g_amec->thermalproc.total_res < l_old_residue)
    {
        l_cpu_speed += g_amec->sys.speed_step;
    }

    // Enforce actuator saturation limits
    if (l_cpu_speed > g_amec->sys.max_speed)
        l_cpu_speed = g_amec->sys.max_speed;
    if (l_cpu_speed < g_amec->sys.min_speed)
        l_cpu_speed = g_amec->sys.min_speed;

    // Calculate frequency request based on thermal speed request
    l_freq_request = amec_controller_speed2freq(
            l_cpu_speed,
            g_amec->sys.fmax);

    if(G_internal_flags & INT_FLAG_ENABLE_EVERY_TICK_TEMP_DVFS)
    {
        // wait MAX_NUM_TICKS for additional changes for moving frequency same direction
        if( (l_thermal_winner != L_last_thermal_winner) ||
            ((L_num_ticks_passed >= MAX_NUM_TICKS) && (L_last_freq_request != l_freq_request)) ||
            (L_freq_increase && (l_freq_request < L_last_freq_request)) ||
            ((L_freq_increase == FALSE) && (l_freq_request > L_last_freq_request)))
        {
            if(l_freq_request < L_last_freq_request)
                L_freq_increase = FALSE; // frequency is being lowered
            else
                L_freq_increase = TRUE;

            L_num_ticks_passed = 0;
            g_amec->thermalproc.speed_request = l_cpu_speed;
            g_amec->thermalproc.freq_request = l_freq_request;
            L_last_freq_request = g_amec->thermalproc.freq_request;
        }
       else if(L_num_ticks_passed < MAX_NUM_TICKS)
            L_num_ticks_passed++;
    }
    else
    {
        g_amec->thermalproc.speed_request = l_cpu_speed;
        g_amec->thermalproc.freq_request = l_freq_request;
        L_last_freq_request = g_amec->thermalproc.freq_request;
        L_num_ticks_passed = MAX_NUM_TICKS;
    }
    // Update the Processor OT Throttle Sensor
    if(g_amec->thermalproc.freq_request < g_amec->sys.fmax)
    {
       // frequency is less than max indicate throttle due to OT
       sensor_update(AMECSENSOR_PTR(PROCOTTHROT), 1);
    }
    else  // not currently throttled due to OT
    {
       sensor_update(AMECSENSOR_PTR(PROCOTTHROT), 0);
    }

    L_last_thermal_winner = l_thermal_winner;
}

// Function Specification
//
// Name: amec_controller_vrm_vdd_thermal
//
// Description: This function implements the Proportional Controller for the
//              VRM Vdd thermal control. Although it doesn't return any
//              results, it populates the thermal vote in the field
//              g_amec->thermalvdd.speed_request.
//
// Task Flags:
//
// End Function Specification
void amec_controller_vrm_vdd_thermal()
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    uint16_t                      l_vdd_temp_tenthsC = 0;
    uint16_t                      l_residue = 0;
    uint16_t                      l_old_residue = 0;
    int16_t                       l_thermal_diff = 0;
    int16_t                       l_cpu_speed = 0;
    int16_t                       l_throttle_chg = 0;
    int32_t                       l_throttle = 0;
    sensor_t                    * l_sensor = NULL;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    // Get VRM Vdd temperature sensor
    l_sensor = getSensorByGsid(TEMPVDD);

    // Convert current Vdd temperature to 0.1 degrees C
    l_vdd_temp_tenthsC = l_sensor->sample * 10;

    // Calculate the temperature difference from the DVFS setpoint
    l_thermal_diff = g_amec->thermalvdd.setpoint - l_vdd_temp_tenthsC;

    // Proportional Controller for the thermal control loop
    l_throttle = (int32_t) l_thermal_diff * g_amec->thermalvdd.Pgain;
    l_residue = (uint16_t) l_throttle;
    l_throttle_chg = (int16_t) (l_throttle >> 16);

    // don't allow a throttle change more than step limit
    if ((int16_t) l_throttle_chg > (int16_t) g_amec->sys.speed_step_limit)
    {
        l_throttle_chg = g_amec->sys.speed_step_limit;
    }
    else
    {
        if ((int16_t) l_throttle_chg < ((int16_t) (-g_amec->sys.speed_step_limit)))
        {
            l_throttle_chg = (int16_t)(-g_amec->sys.speed_step_limit);
        }
    }

    // Calculate the new thermal CPU speed request
    l_cpu_speed = g_amec->thermalvdd.speed_request +
        (int16_t)(l_throttle_chg * g_amec->sys.speed_step);

    // Proceed with residue summation to correctly follow set-point
    l_old_residue = g_amec->thermalvdd.total_res;
    g_amec->thermalvdd.total_res += l_residue;
    if (g_amec->thermalvdd.total_res < l_old_residue)
    {
        l_cpu_speed += g_amec->sys.speed_step;
    }

    // Enforce actuator saturation limits
    if (l_cpu_speed > g_amec->sys.max_speed)
        l_cpu_speed = g_amec->sys.max_speed;
    if (l_cpu_speed < g_amec->sys.min_speed)
        l_cpu_speed = g_amec->sys.min_speed;

    // Generate the new thermal speed request
    g_amec->thermalvdd.speed_request = l_cpu_speed;
    // Calculate frequency request based on thermal speed request
    g_amec->thermalvdd.freq_request = amec_controller_speed2freq(
            g_amec->thermalvdd.speed_request,
            g_amec->sys.fmax);
}

//*************************************************************************
// Function Specification
//
// Name: amec_controller_dimm_thermal
//
// Description: This function implements the Proportional Controller for the
//              DIMM thermal control. Although it doesn't return any
//              results, it populates the thermal vote in the field
//              g_amec->thermaldimm.speed_request, g_amec->thermalmcdimm.speed_request,
//              g_amec->thermalpmic.speed_request and g_amec->thermalmcext.speed_request,
// Task Flags:
//
// End Function Specification
void amec_controller_dimm_thermal()
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    uint8_t                       i = 0;
    uint8_t                       l_max_dimm_types = 4;
    const uint16_t                l_dimm_types[4] = {DATA_FRU_DIMM,
                                                     DATA_FRU_MEMCTRL_DRAM,
                                                     DATA_FRU_PMIC,
                                                     DATA_FRU_MEMCTRL_EXT};
    uint16_t                      l_thermal_winner = 0;
    uint16_t                      l_residue = 0;
    uint16_t                      l_old_residue = 0;
    uint16_t                      l_throttle_temp = 0;
    uint16_t                      l_Pgain = 0;
    uint16_t                    * l_speed_request = NULL;
    uint16_t                    * l_total_res = NULL;
    int16_t                       l_error = 0;
    int16_t                       l_mem_speed = 0;
    int16_t                       l_throttle_chg = 0;
    int32_t                       l_throttle = 0;
    sensor_t                    * l_sensor = NULL;
    bool                          l_timeout = false;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    // loop for the number of different fru types the "dimm" sensors can be
    // to determine memory throttle based on each type
    for(i= 0; i < l_max_dimm_types; i++)
    {
        l_timeout = false;  // default this type did not timeout

        // setup vars specific for type being processed
        if(l_dimm_types[i] == DATA_FRU_DIMM)
        {
            // use control values for DATA_FRU_DIMM type
            l_throttle_temp = g_amec->thermaldimm.setpoint;
            l_Pgain = g_amec->thermaldimm.Pgain;
            l_speed_request = &g_amec->thermaldimm.speed_request;
            l_total_res = &g_amec->thermaldimm.total_res;

            // Get the highest DIMM temperature in 0.1 degrees C
            l_sensor = getSensorByGsid(TEMPDIMMTHRM);
            l_thermal_winner = l_sensor->sample * 10;

            // check for time out
            if( (G_dimm_temp_expired_bitmap.dw[0] || G_dimm_temp_expired_bitmap.dw[1]) &&
                (G_ocmb_dts_type_expired_bitmap & OCM_DTS_TYPE_DIMM_MASK) )
            {
                l_timeout = true;
            }
        } // end if DATA_FRU_DIMM
        else if(l_dimm_types[i] == DATA_FRU_MEMCTRL_DRAM)
        {
            // use control values for DATA_FRU_MEMCTRL_DRAM type
            l_throttle_temp = g_amec->thermalmcdimm.setpoint;
            l_Pgain = g_amec->thermalmcdimm.Pgain;
            l_speed_request = &g_amec->thermalmcdimm.speed_request;
            l_total_res = &g_amec->thermalmcdimm.total_res;

            // Get the highest Memctrl+DRAM temperature in 0.1 degrees C
            l_sensor = getSensorByGsid(TEMPMCDIMMTHRM);
            l_thermal_winner = l_sensor->sample * 10;

            // check if this type timed out
            if(G_ocmb_dts_type_expired_bitmap & OCM_DTS_TYPE_MEMCTRL_DRAM_MASK)
            {
                l_timeout = true;
            }
        }
        else if(l_dimm_types[i] == DATA_FRU_PMIC)
        {
            // use control values for DATA_FRU_PMIC type
            l_throttle_temp = g_amec->thermalpmic.setpoint;
            l_Pgain = g_amec->thermalpmic.Pgain;
            l_speed_request = &g_amec->thermalpmic.speed_request;
            l_total_res = &g_amec->thermalpmic.total_res;

            // Get the highest PMIC temperature in 0.1 degrees C
            l_sensor = getSensorByGsid(TEMPPMICTHRM);
            l_thermal_winner = l_sensor->sample * 10;

            // check if this type timed out
            if(G_ocmb_dts_type_expired_bitmap & OCM_DTS_TYPE_PMIC_MASK)
            {
                l_timeout = true;
            }
        }
        else if(l_dimm_types[i] == DATA_FRU_MEMCTRL_EXT)
        {
            // use control values for DATA_FRU_MEMCTRL_EXT type
            l_throttle_temp = g_amec->thermalmcext.setpoint;
            l_Pgain = g_amec->thermalmcext.Pgain;
            l_speed_request = &g_amec->thermalmcext.speed_request;
            l_total_res = &g_amec->thermalmcext.total_res;

            // Get the highest external mem controller temperature in 0.1 degrees C
            l_sensor = getSensorByGsid(TEMPMCEXTTHRM);
            l_thermal_winner = l_sensor->sample * 10;

            // check if this type timed out
            if(G_ocmb_dts_type_expired_bitmap & OCM_DTS_TYPE_MEMCTRL_EXT_MASK)
            {
                l_timeout = true;
            }
        }
        else
        {
            // should never happen -- code bug
            TRAC_ERR("amec_controller_dimm_thermal: Invalid DIMM sensor type[0x%02X] at idx[%d]",
                     l_dimm_types[i],
                     i);
            continue;
        }

        // start common code for all types to determine throttle level
        // Adjust the temperature if there was a time out reading this sensor fru type
        if(l_timeout)
        {
            //Assume temperature is at the throttle temp plus 1 degree C.
            l_thermal_winner = l_throttle_temp + 10;
        }

        // Check if this type is being used and the temp differs from the throttle point
        if( (!l_thermal_winner) || (l_throttle_temp == l_thermal_winner) )
            continue;

        // Calculate the thermal control error
        l_error = l_throttle_temp - l_thermal_winner;

        // Proportional Controller for the thermal control loop based on memory temperatures
        l_throttle = (int32_t) l_error * l_Pgain;
        l_residue = (uint16_t) l_throttle;
        l_throttle_chg = (int16_t) (l_throttle >> 16);

        if ((int16_t) l_throttle_chg > AMEC_MEMORY_SPEED_CHANGE_LIMIT)
        {
            l_throttle_chg = AMEC_MEMORY_SPEED_CHANGE_LIMIT;
        }
        else
        {
            if ((int16_t) l_throttle_chg < (-AMEC_MEMORY_SPEED_CHANGE_LIMIT))
            {
                l_throttle_chg = -AMEC_MEMORY_SPEED_CHANGE_LIMIT;
            }
        }

        // Calculate the new thermal speed request
        l_mem_speed = *l_speed_request +
           (int16_t) l_throttle_chg * AMEC_MEMORY_STEP_SIZE;

        // Proceed with residue summation to correctly follow set-point
        l_old_residue = *l_total_res;
        *l_total_res += l_residue;
        if (*l_total_res < l_old_residue)
        {
            l_mem_speed += AMEC_MEMORY_STEP_SIZE;
        }

        // Enforce actuator saturation limits
        if (l_mem_speed > AMEC_MEMORY_MAX_STEP)
                l_mem_speed = AMEC_MEMORY_MAX_STEP;
        if (l_mem_speed < AMEC_MEMORY_MIN_STEP)
                l_mem_speed = AMEC_MEMORY_MIN_STEP;

        // Save the new thermal speed request for this memory sensor type
        *l_speed_request = (uint16_t) l_mem_speed;

    } // end for loop processing each memory sensor type
}


//*************************************************************************
// Function Specification
//
// Name: amec_controller_membuf_thermal
//
// Description: This function implements the Proportional Controller for the
//              membuf thermal control. Although it doesn't return any
//              results, it populates the thermal vote in the field
//              g_amec->thermalmembuf.speed_request.
//
// Task Flags:
//
// End Function Specification
void amec_controller_membuf_thermal()
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    uint16_t                      l_thermal_winner = 0;
    uint16_t                      l_residue = 0;
    uint16_t                      l_old_residue = 0;
    int16_t                       l_error = 0;
    int16_t                       l_mem_speed = 0;
    int16_t                       l_throttle_chg = 0;
    int32_t                       l_throttle = 0;
    sensor_t                    * l_sensor = NULL;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    // Get hottest membuf sensor
    l_sensor = getSensorByGsid(TEMPMEMBUFTHRM);

    if(G_membuf_temp_expired_bitmap)
    {
        //we were not able to get a valid temperature.  Assume it is 1 degree
        //over the setpoint.
        l_thermal_winner = g_amec->thermalmembuf.setpoint + 10;
    }
    else
    {
        // Use the highest temperature of all membuf in 0.1 degrees C
        l_thermal_winner = l_sensor->sample * 10;
    }

    // Check if there is an error
    if (g_amec->thermalmembuf.setpoint == l_thermal_winner)
        return;

    // Calculate the thermal control error
    l_error = g_amec->thermalmembuf.setpoint - l_thermal_winner;

    // Proportional Controller for the thermal control loop based on membuf
    // temperatures
    l_throttle = (int32_t) l_error * g_amec->thermalmembuf.Pgain;
    l_residue = (uint16_t) l_throttle;
    l_throttle_chg = (int16_t) (l_throttle >> 16);

    if ((int16_t) l_throttle_chg > AMEC_MEMORY_SPEED_CHANGE_LIMIT)
    {
        l_throttle_chg = AMEC_MEMORY_SPEED_CHANGE_LIMIT;
    }
    else
    {
        if ((int16_t) l_throttle_chg < (-AMEC_MEMORY_SPEED_CHANGE_LIMIT))
        {
            l_throttle_chg = -AMEC_MEMORY_SPEED_CHANGE_LIMIT;
        }
    }

    // Calculate the new thermal speed request for membufs
    l_mem_speed = g_amec->thermalmembuf.speed_request +
        (int16_t) l_throttle_chg * AMEC_MEMORY_STEP_SIZE;

    // Proceed with residue summation to correctly follow set-point
    l_old_residue = g_amec->thermalmembuf.total_res;
    g_amec->thermalmembuf.total_res += l_residue;
    if (g_amec->thermalmembuf.total_res < l_old_residue)
    {
        l_mem_speed += AMEC_MEMORY_STEP_SIZE;
    }

    // Enforce actuator saturation limits
    if (l_mem_speed > AMEC_MEMORY_MAX_STEP)
        l_mem_speed = AMEC_MEMORY_MAX_STEP;
    if (l_mem_speed < AMEC_MEMORY_MIN_STEP)
        l_mem_speed = AMEC_MEMORY_MIN_STEP;

    // Generate the new thermal speed request
    g_amec->thermalmembuf.speed_request = (uint16_t) l_mem_speed;
}

//*************************************************************************
// Function Specification
//
// Name: amec_controller_speed2freq
//
// Description: Helper function to convert speed to MHz.
//
// End Function Specification
uint16_t amec_controller_speed2freq (const uint16_t i_speed, const uint16_t i_fmax)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    uint16_t                    l_tempreg = 0;
    uint16_t                    l_temp16 = 0;
    uint32_t                    l_temp32 = 0;
    uint16_t                    l_freq = 0;
    uint32_t                    l_divide32[2] = {0, 0};

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    // to handle max freq changing (i.e. mode change) between now and running amec_slv_proc_voting_box
    // if speed is unconstrained set freq to unconstrained so voting box will use
    // the most recent maximum frequency
    if(i_speed >= g_amec->sys.max_speed)
    {
        l_freq = 0xFFFF;
    }
    else
    {
        l_temp16 = i_fmax;
        l_tempreg = (uint16_t)i_speed;
        l_temp32 = ((uint32_t)l_tempreg)*((uint32_t)l_temp16);
        l_temp16 = (uint16_t)1000;
        l_divide32[1] = (uint32_t)l_temp16;
        l_divide32[0] = (uint32_t)l_temp32;
        l_divide32[0] /= l_divide32[1];
        l_temp32 = l_divide32[0];
        l_freq = (uint16_t)l_temp32; /* freq will always fit in 16 bits */
    }
    return l_freq;
}

// Function Specification
//
// Name: amec_idle_chip_freq_control
//
// Description: This function determines the frequency vote for idle chip control
//              Frequency vote is g_amec->eff_mode_parms.idle_chip_freq_request
//
// End Function Specification
void amec_idle_chip_freq_control()
{
    uint16_t    l_core_util = 0;
    uint16_t    l_core_util_min = 0xffff;
    uint16_t    l_core_util_max = 0;
    bool        l_all_cores_below_enter_util = TRUE;
    bool        l_core_above_exit_util = FALSE;
    uint8_t     l_core_num = 0;
    static uint16_t L_32ms_ticks_below_enter_util = 0;
    static uint16_t L_32ms_ticks_above_exit_util = 0;

    // Check if Idle Chip Frequency control is enabled and we are in an efficiency mode
    if( (g_amec->eff_mode_parms.enable.fields.utilization_enable) &&
        (g_amec->eff_mode_parms.enable.fields.mode_support) )
    {
        // check all core's utilization for all below enter or at least one above exit
        for(l_core_num = 0; l_core_num < MAX_NUM_CORES; l_core_num++)
        {
           if(CORE_PRESENT(l_core_num))
           {
              l_core_util = G_amec_sensor_list[UTILC0 + l_core_num]->sample;
              if(l_core_util > l_core_util_max)
                  l_core_util_max = l_core_util;
              if(l_core_util < l_core_util_min)
                  l_core_util_min = l_core_util;

              if(l_core_util >= g_amec->eff_mode_parms.entry_threshold)
              {
                 // found a core above entry threshold
                 l_all_cores_below_enter_util = FALSE;
                 // check if this is also above exit threshold
                 if(l_core_util > g_amec->eff_mode_parms.exit_threshold)
                 {
                     l_core_above_exit_util = TRUE;
                 }
              }
           }
        } // for all cores

        // Update min/max util for debug
        g_amec->eff_mode_parms.core_util_min = l_core_util_min;
        g_amec->eff_mode_parms.core_util_max = l_core_util_max;
        // Update timers
        if(l_all_cores_below_enter_util)
        {
           if(L_32ms_ticks_below_enter_util != 0xFFFF) // prevent wrapping
              L_32ms_ticks_below_enter_util++;
           L_32ms_ticks_above_exit_util = 0;
        }
        else if(l_core_above_exit_util)
        {
           L_32ms_ticks_below_enter_util = 0;
           if(L_32ms_ticks_above_exit_util != 0xFFFF) // prevent wrapping
              L_32ms_ticks_above_exit_util++;
        }
        else // in hystresis window
        {
           L_32ms_ticks_below_enter_util = 0;
           L_32ms_ticks_above_exit_util = 0;
        }

        // check if met enter or exit criteria
        if(L_32ms_ticks_below_enter_util >= g_amec->eff_mode_parms.entry_delay)
        {
           if(g_amec->eff_mode_parms.idle_chip_freq_request != G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MIN_FREQ])
           {
              // Chip met idle frequency entry criteria drop to min freq
              g_amec->eff_mode_parms.idle_chip_freq_request = G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MIN_FREQ];
           }
        }
        else if(L_32ms_ticks_above_exit_util >= g_amec->eff_mode_parms.exit_delay)
        {
           if(g_amec->eff_mode_parms.idle_chip_freq_request != 0xFFFF)
           {
              // Chip met idle frequency exit criteria set frequency to unrestricted
              g_amec->eff_mode_parms.idle_chip_freq_request = 0xFFFF;
           }
        }

    } // if idle chip freq control enabled
    else if(g_amec->eff_mode_parms.idle_chip_freq_request != 0xFFFF)
    {
        TRAC_INFO("amec_idle_chip_freq_control disabled previous freq vote %d",
                   g_amec->eff_mode_parms.idle_chip_freq_request);
        g_amec->eff_mode_parms.idle_chip_freq_request = 0xFFFF;
    }
}

/*----------------------------------------------------------------------------*/
/* End                                                                        */
/*----------------------------------------------------------------------------*/
