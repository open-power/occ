/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/amec/amec_controller.c $                          */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2019                        */
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

//*************************************************************************
// Externs
//*************************************************************************
extern dimm_sensor_flags_t G_dimm_temp_expired_bitmap;
extern uint16_t G_cent_temp_expired_bitmap;
extern uint8_t G_ocm_dts_type_expired_bitmap;

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
    int16_t                       l_error = 0;
    int16_t                       l_cpu_speed = 0;
    int16_t                       l_throttle_chg = 0;
    int32_t                       l_throttle = 0;
    sensor_t                    * l_sensor = NULL;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    // Get TEMPPROCTHRM sensor, which is hottest core temperature in OCC
    // processor
    l_sensor = getSensorByGsid(TEMPPROCTHRM);

    // Use the highest temperature of all processors in 0.1 degrees C
    l_thermal_winner = l_sensor->sample * 10;

    // Check if there is an error
    if (g_amec->thermalproc.setpoint == l_thermal_winner)
        return;

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

    // Generate the new thermal speed request
    g_amec->thermalproc.speed_request = l_cpu_speed;
    // Calculate frequency request based on thermal speed request
    g_amec->thermalproc.freq_request = amec_controller_speed2freq(
            g_amec->thermalproc.speed_request,
            g_amec->sys.fmax);

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
    uint8_t                       l_max_dimm_types = 0;
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
    if(MEM_TYPE_OCM == G_sysConfigData.mem_type)
    {
       // all 4 types are possible:
       l_max_dimm_types = 4;
    }
    else
    {
       // can only be the one DATA_FRU_DIMM type which must be listed first in l_dimm_types
       l_max_dimm_types = 1;
    }

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
           if(G_dimm_temp_expired_bitmap.dw[0] || G_dimm_temp_expired_bitmap.dw[1])
           {
              if(MEM_TYPE_OCM != G_sysConfigData.mem_type)
              {
                 // non-OCM can only have DIMM type so timeout must be for DIMM
                 l_timeout = true;
              }
              else if(G_ocm_dts_type_expired_bitmap & OCM_DTS_TYPE_DIMM_MASK) // MEM_TYPE_OCM
              {
                 l_timeout = true;
              }
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
           if(G_ocm_dts_type_expired_bitmap & OCM_DTS_TYPE_MEMCTRL_DRAM_MASK)
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
           if(G_ocm_dts_type_expired_bitmap & OCM_DTS_TYPE_PMIC_MASK)
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
           if(G_ocm_dts_type_expired_bitmap & OCM_DTS_TYPE_MEMCTRL_EXT_MASK)
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

    // Done processing all types now determine if any of them are driving throttling
    // and update the Memory OT Throttle Sensor
    if( (g_amec->thermaldimm.speed_request < AMEC_MEMORY_MAX_STEP) ||
        (g_amec->thermalmcdimm.speed_request < AMEC_MEMORY_MAX_STEP) ||
        (g_amec->thermalpmic.speed_request < AMEC_MEMORY_MAX_STEP) ||
        (g_amec->thermalmcext.speed_request < AMEC_MEMORY_MAX_STEP) )
    {
       // Memory speed is less than max indicate throttle due to OT
       sensor_update(AMECSENSOR_PTR(MEMOTTHROT), 1);
    }
    else  // not currently throttled due to OT
    {
       sensor_update(AMECSENSOR_PTR(MEMOTTHROT), 0);
    }
}


//*************************************************************************
// Function Specification
//
// Name: amec_controller_membuf_thermal
//
// Description: This function implements the Proportional Controller for the
//              membuf thermal control. Although it doesn't return any
//              results, it populates the thermal vote in the field
//              g_amec->thermalcent.speed_request.
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
    // Get TEMPCENT sensor value
    l_sensor = getSensorByGsid(TEMPCENT);

    if(G_cent_temp_expired_bitmap)
    {
        //we were not able to get a valid temperature.  Assume it is 1 degree
        //over the setpoint.
        l_thermal_winner = g_amec->thermalcent.setpoint + 10;
    }
    else
    {
        // Use the highest temperature of all membuf in 0.1 degrees C
        l_thermal_winner = l_sensor->sample * 10;
    }

    // Check if there is an error
    if (g_amec->thermalcent.setpoint == l_thermal_winner)
        return;

    // Calculate the thermal control error
    l_error = g_amec->thermalcent.setpoint - l_thermal_winner;

    // Proportional Controller for the thermal control loop based on membuf
    // temperatures
    l_throttle = (int32_t) l_error * g_amec->thermalcent.Pgain;
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
    l_mem_speed = g_amec->thermalcent.speed_request +
        (int16_t) l_throttle_chg * AMEC_MEMORY_STEP_SIZE;

    // Proceed with residue summation to correctly follow set-point
    l_old_residue = g_amec->thermalcent.total_res;
    g_amec->thermalcent.total_res += l_residue;
    if (g_amec->thermalcent.total_res < l_old_residue)
    {
        l_mem_speed += AMEC_MEMORY_STEP_SIZE;
    }

    // Enforce actuator saturation limits
    if (l_mem_speed > AMEC_MEMORY_MAX_STEP)
        l_mem_speed = AMEC_MEMORY_MAX_STEP;
    if (l_mem_speed < AMEC_MEMORY_MIN_STEP)
        l_mem_speed = AMEC_MEMORY_MIN_STEP;

    // Generate the new thermal speed request
    g_amec->thermalcent.speed_request = (uint16_t) l_mem_speed;
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

/*----------------------------------------------------------------------------*/
/* End                                                                        */
/*----------------------------------------------------------------------------*/
