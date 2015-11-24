/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/amec/amec_sensors_core.c $                            */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2015                        */
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

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
//#include <occ_common.h>
#include <ssx.h>
#include <errl.h>               // Error logging
#include "sensor.h"
#include "rtls.h"
#include "occ_sys_config.h"
#include "occ_service_codes.h"  // for SSX_GENERIC_FAILURE
#include "dcom.h"
#include "proc_data.h"
#include "amec_smh.h"
#include "amec_slave_smh.h"
#include <trac.h>
#include "amec_sys.h"
#include "sensor_enum.h"
#include "amec_service_codes.h"
#include <amec_sensors_core.h>
#include "amec_perfcount.h"
#include "proc_shared.h"

/******************************************************************************/
/* Globals                                                                    */
/******************************************************************************/

/******************************************************************************/
/* Forward Declarations                                                       */
/******************************************************************************/
void amec_calc_dts_sensors(gpe_bulk_core_data_t * i_core_data_ptr, uint8_t i_core);
void amec_calc_freq_and_util_sensors(gpe_bulk_core_data_t * i_core_data_ptr, uint8_t i_core);
void amec_calc_ips_sensors(gpe_bulk_core_data_t * i_core_data_ptr, uint8_t i_core);
void amec_calc_spurr(uint8_t i_core);

//*************************************************************************
// Code
//*************************************************************************

// Function Specification
//
// Name: amec_update_proc_core_sensors
//
// Description: Update all the sensors for a given proc
//
// Thread: RealTime Loop
//
// End Function Specification
void amec_update_proc_core_sensors(uint8_t i_core)
{
  gpe_bulk_core_data_t *l_core_data_ptr;
  uint16_t              l_temp16 = 0;
  uint32_t              l_temp32 = 0;

  // Make sure the core is present, and that it has updated data.
  if(CORE_PRESENT(i_core) && CORE_UPDATED(i_core))
  {
    // Clear flag indicating core was updated by proc task
    CLEAR_CORE_UPDATED(i_core);

    // Get pointer to core data
    l_core_data_ptr = proc_get_bulk_core_data_ptr(i_core);

    //-------------------------------------------------------
    // Thermal Sensors & Calc
    //-------------------------------------------------------
    amec_calc_dts_sensors(l_core_data_ptr, i_core);

/*
    //-------------------------------------------------------
    // Util / Freq
    //-------------------------------------------------------
    // Skip this update if there was an empath collection error
    if (!CORE_EMPATH_ERROR(i_core))
    {
        amec_calc_freq_and_util_sensors(l_core_data_ptr,i_core);
    }
    
    //-------------------------------------------------------
    // Performance counter - This function should be called
    // after amec_calc_freq_and_util_sensors().
    //-------------------------------------------------------
    //amec_calc_dps_util_counters(i_core);

    //-------------------------------------------------------
    // IPS
    //-------------------------------------------------------
    // Skip this update if there was an empath collection error
    if (!CORE_EMPATH_ERROR(i_core))
    {
        amec_calc_ips_sensors(l_core_data_ptr,i_core);
    }

    //-------------------------------------------------------
    // SPURR
    //-------------------------------------------------------
    //amec_calc_spurr(i_core);

    // ------------------------------------------------------
    // Update PREVIOUS values for next time
    // ------------------------------------------------------
    g_amec->proc[0].core[i_core].prev_PC_RAW_Th_CYCLES = l_core_data_ptr->per_thread[0].raw_cycles;

    // Skip empath updates if there was an empath collection error on this core
    if (!CORE_EMPATH_ERROR(i_core))
    {
        g_amec->proc[0].core[i_core].prev_PC_RAW_CYCLES    = l_core_data_ptr->empath.raw_cycles;
        g_amec->proc[0].core[i_core].prev_PC_RUN_CYCLES    = l_core_data_ptr->empath.run_cycles;
        g_amec->proc[0].core[i_core].prev_PC_COMPLETED     = l_core_data_ptr->empath.completion;
        g_amec->proc[0].core[i_core].prev_PC_DISPATCH      = l_core_data_ptr->empath.dispatch;
        g_amec->proc[0].core[i_core].prev_tod_2mhz         = l_core_data_ptr->empath.tod_2mhz;
        g_amec->proc[0].core[i_core].prev_FREQ_SENS_BUSY   = l_core_data_ptr->empath.freq_sens_busy;
        g_amec->proc[0].core[i_core].prev_FREQ_SENS_FINISH = l_core_data_ptr->empath.freq_sens_finish;
    }

    for(i=0; i<MAX_THREADS_PER_CORE; i++)
    {
      g_amec->proc[0].core[i_core].thread[i].prev_PC_RUN_Th_CYCLES = l_core_data_ptr->per_thread[i].run_cycles;
    }
*/
    // Final step is to update TOD sensors
    // Extract 32 bits with 16usec resolution
    l_temp32 = (uint32_t)(G_dcom_slv_inbox_doorbell_rx.tod>>13);
    l_temp16 = (uint16_t)(l_temp32);
    // low 16 bits is 16usec resolution with 512MHz TOD clock
    sensor_update( AMECSENSOR_PTR(TODclock0), l_temp16);
    l_temp16 = (uint16_t)(l_temp32>>16);
    // mid 16 bits is 1.05sec resolution with 512MHz TOD clock
    sensor_update( AMECSENSOR_PTR(TODclock1), l_temp16);
    l_temp16 = (uint16_t)(G_dcom_slv_inbox_doorbell_rx.tod>>45);
    // hi 3 bits in 0.796 day resolution with 512MHz TOD clock
    sensor_update( AMECSENSOR_PTR(TODclock2), l_temp16);
  }
}


// Core Weight - Weight factor for core DTS used to calculate a core temp
// TODO - fmk - to be changed once we have TMGT config data with weights.
// Suggest adding core weight to g_amec struct
// TEMP - Using the same weight for all cores.
uint8_t     G_coreWeight = 2;
uint8_t     G_quadWeight = 1;

// Function Specification
//
// Name: amec_calc_dts_sensors
//
// Description: Compute core temperature. This function is called every
// 2ms/core.
//
// PreCondition: The core is present.
//
// Thread: RealTime Loop
//
// End Function Specification
void amec_calc_dts_sensors(gpe_bulk_core_data_t * i_core_data_ptr, uint8_t i_core)
{
#define DTS_PER_CORE     2
#define DTS_INVALID_MASK 0x0C00


  uint32_t      l_coreTemp = 0;
  uint8_t       k = 0;
  uint16_t      l_dts[DTS_PER_CORE];
  uint16_t      l_quadDts = 0;
  BOOLEAN       l_update_sensor = FALSE;
  uint16_t      l_core_hot = 0;
  uint8_t       l_dtsCnt = 0;   //Number of valid Core DTSs

  //Clear DTS array.
  memset((void *)&(l_dts[0]), 0, sizeof(l_dts));

  if (i_core_data_ptr != NULL)
  {
    //the Core DTS temperatures are considered in the calculation only if:
    //  - They are valid.
    //  - Non-zero
    //  - Non-negative
    for (k = 0; k < DTS_PER_CORE; k++)
    {
      //Check validity
      if (i_core_data_ptr->dts.core[k].fields.valid)
      {
        l_dts[k] = i_core_data_ptr->dts.core[k].fields.reading;
        l_dtsCnt++;

        //Hardware bug workaround:  Temperatures reaching 0 degrees C
        //can show up as negative numbers.  To fix this, we discount
        //values that have the 2 MSB's set.
        if(((l_dts[k] & DTS_INVALID_MASK) == DTS_INVALID_MASK) ||
           (l_dts[k] == 0))
        {
          l_dts[k] = 0;
          l_dtsCnt--;
        }

        if (l_dts[k] > l_core_hot)
        {
          l_core_hot = l_dts[k];
        }
      }
    } //for loop

    //The core DTSs are considered only if we have at least 1 valid core DTS and
    //a non-zero G_coreWeight.
    if (l_dtsCnt && G_coreWeight)
    {
      l_update_sensor = TRUE;
    }

    //The Quad DTS value is considered only if we have a valid Quad DTS and
    //a non-zero quad weight.
    if (i_core_data_ptr->dts.cache.fields.valid && G_quadWeight)
    {
      l_quadDts = i_core_data_ptr->dts.cache.fields.reading;
      l_update_sensor = TRUE;
    }

    // Update the thermal sensor associated with this core
    if(l_update_sensor)
    {
      //Formula:
      //                (Cwt(CoreDTS1 + CoreDTS2) + (Qwt*QuadDTS))
      //                ------------------------------------------
      //                         (2*Cwt + Qwt)
      if ((G_coreWeight && l_dtsCnt) || G_quadWeight)
      {
        l_coreTemp = ((G_coreWeight *(l_dts[0] + l_dts[1])) + (G_quadWeight * l_quadDts))
                     / ((l_dtsCnt * G_coreWeight) + G_quadWeight);
      }

      // Update sensors & Interim Data
      sensor_update( AMECSENSOR_ARRAY_PTR(TEMP4MSP0C0,i_core), l_coreTemp);
      g_amec->proc[0].core[i_core].dts_hottest = l_core_hot;
    }
  }
}

// Function Specification
//
// Name: amec_calc_freq_and_util_sensors
//
// Description: Compute the frequency and utilization sensors for a given core.
// This function is called every 2ms/core.
//
// Thread: RealTime Loop
//
// End Function Specification
// TEMP - Not supported yet.
#if 0 
void amec_calc_freq_and_util_sensors(gpe_bulk_core_data_t * i_core_data_ptr, uint8_t i_core)
{
  BOOLEAN  l_core_sleep_winkle = FALSE;
  uint32_t l_pm_state_hist_reg = 0;
  uint32_t temp32      = 0;
  uint32_t temp32a     = 0;
  uint16_t temp16      = 0;
  uint16_t temp16a     = 0;
  uint16_t l_core_util = 0;
  uint16_t l_core_freq = 0;
  uint16_t l_time_interval = 0;
  uint32_t l_cycles2ms = 0;
  int i;

  // Read the high-order bytes of PM State History register for this core
  l_pm_state_hist_reg = i_core_data_ptr->pcb_slave.pm_history.words.high_order;

  // If core is in fast/deep sleep mode or fast/winkle mode, then set a flag
  // indicating this
  if(l_pm_state_hist_reg & OCC_PAST_CORE_CLK_STOP)
  {
      l_core_sleep_winkle = TRUE;
  }

  // ------------------------------------------------------
  // Per Core Frequency
  // ------------------------------------------------------
  // <amec_formula>
  // Result: Calculated Core Frequency
  // Sensor: FREQA2MSP0C0
  // Timescale: 2ms
  // Units: MHz
  // Min/Max: 0/6000 (UPPER_LIMIT_PROC_FREQ_MHZ=6000)
  // Formula: cyc_delta = (RAW_CYCLES[t=now] - RAW_CYCLES[t=-2ms])
  //          time_delta = (TOD[t=now] - TOD[t=-2ms])
  //          frequency(MHz) = (cyc_delta / time_delta) * (2M TOD ticks / 1 second)
  //                         = (2 * cyc_delta) / time_delta
  // </amec_formula>

  // Compute Delta in PC_RAW_CYCLES
  temp32  = i_core_data_ptr->empath.raw_cycles;
  temp32a = g_amec->proc[0].core[i_core].prev_PC_RAW_CYCLES;
  temp32  = l_cycles2ms = temp32 - temp32a;

  if( (cfam_id() == CFAM_CHIP_ID_MURANO_10)
      || (cfam_id() == CFAM_CHIP_ID_MURANO_11)
      || (cfam_id() == CFAM_CHIP_ID_MURANO_12) )
  {
      temp32a = AMEC_US_PER_SMH_PERIOD; // using fixed 2000us is showing 3% error.
      temp32  = temp32 / temp32a;
  }
  else
  {
      temp32a = (i_core_data_ptr->empath.tod_2mhz -
                 g_amec->proc[0].core[i_core].prev_tod_2mhz);
      temp32  = (2 * temp32) / temp32a;
  }

  // TODO:  Remove this once we have the OHA Power Proxy legacy mode stuff working.
  if(temp32 < UPPER_LIMIT_PROC_FREQ_MHZ)
  {
      // Update Sensor for this core
      if(l_core_sleep_winkle)
      {
          l_core_freq = 0;
      }
      else
      {
          l_core_freq = (uint16_t) temp32;
      }
      sensor_update( AMECSENSOR_ARRAY_PTR(FREQA2MSP0C0,i_core), l_core_freq);
  }

  // ------------------------------------------------------
  // Per Core Utilization
  // ------------------------------------------------------
  // <amec_formula>
  // Result: Calculated Core Utilization
  // Sensor: UTIL2MSP0C0
  // Timescale: 2ms
  // Units: 0.01 %
  // Min/Max: 0/10000  (0/100%)
  // Formula: cyc_delta = (RAW_CYCLES[t=now] - RAW_CYCLES[t=-2ms])
  //          run_delta = (RUN_CYCLES[t=now] - RUN_CYCLES[t=-2ms])
  //
  //          UTIL(in %) = run_delta / cyc_delta
  // </amec_formula>

  // Compute Delta in PC_RUN_CYCLES
  temp32 = i_core_data_ptr->empath.run_cycles;
  temp32a = g_amec->proc[0].core[i_core].prev_PC_RUN_CYCLES;
  temp32 = temp32 - temp32a;

  temp32 = temp32 >> 8;       // Drop non-significant bits
  temp16 = (uint16_t) temp32; // Cast to uint16 for mult below
  temp16a = 10000;            // Mult * 10000 to get finer resolution for 0.01%
  temp32 = ((uint32_t)temp16a)*((uint32_t)temp16);

  temp32a = l_cycles2ms;   // Get Raw cycles
  temp32a = temp32a >> 8;  // Drop non-significant bits

  // Calculate Utilization
  temp32 = temp32 / temp32a;
  if(temp32a == 0)          // Prevent a divide by zero
  {
      temp32 = 0;
  }

  // Update Sensor for this core
  if(l_core_sleep_winkle)
  {
      l_core_util = 0;
  }
  else
  {
      l_core_util = (uint16_t) temp32;
  }
  sensor_update(AMECSENSOR_ARRAY_PTR(UTIL2MSP0C0, i_core), l_core_util);


  // ------------------------------------------------------
  // Per Thread Utilization
  // ------------------------------------------------------
  // <amec_formula>
  // Result: Calculated Core Utilization
  // Sensor: None
  // Timescale: 2ms
  // Units: 0.01 %
  // Min/Max: 0/10000  (0/100%)
  // Formula: cyc_delta = (RAW_CYCLES[t=now] - RAW_CYCLES[t=-2ms])
  //          run_delta = (RUN_CYCLES[t=now] - RUN_CYCLES[t=-2ms])
  //
  //          UTIL(in %) = run_delta / cyc_delta
  // </amec_formula>

  // Get RAW CYCLES for Thread
  temp32  = i_core_data_ptr->per_thread[0].raw_cycles;
  temp32a = g_amec->proc[0].core[i_core].prev_PC_RAW_Th_CYCLES;
  temp32 = l_cycles2ms = temp32 - temp32a;

  for(i=0; i<MAX_THREADS_PER_CORE; i++)
  {
    // Get Run Counters for Thread
    temp32 = i_core_data_ptr->per_thread[i].run_cycles;
    temp32a = g_amec->proc[0].core[i_core].thread[i].prev_PC_RUN_Th_CYCLES;
    temp32 = temp32 - temp32a;

    temp32 = temp32 >> 8;        // Drop non-significant bits
    temp16 = (uint16_t) temp32;  // Cast to uint16 for mult below
    temp16a = 10000;             // Mult * 10000 to get finer resolution for 0.01%
    temp32 = ((uint32_t)temp16a)*((uint32_t)temp16);

    temp32a = l_cycles2ms;
    temp32a = temp32a >> 8;      // Drop non-significant bits

    // Calculate Utilization
    temp32 = temp32 / temp32a;

    // Update per thread value for this core
    if(l_core_sleep_winkle)
    {
        temp32 = 0;
    }
    g_amec->proc[0].core[i_core].thread[i].util2ms_thread = (uint16_t) temp32;
  }

  // No sensors to update for perThread Util

  // ------------------------------------------------------
  // Per Core Sleep/Winkle Count
  // ------------------------------------------------------

  // Get Current Idle State of Chiplet
  // The SLEEPCNT and WINKLECNT sensors are updated in amec_slv_state_0() function
  temp16 = CONVERT_UINT64_UINT16_UPPER(i_core_data_ptr->pcb_slave.pm_history.value);
  temp16 = temp16 & 0xE000;
  temp16 = temp16 >> 13;
  switch(temp16)
  {
    case 0:                                 break;   // Run State
    case 1:                                 break;   // Special Wakeup
    case 2:                                 break;   // Nap
    case 3: SETBIT(g_amec->proc[0].sleep_cnt,i_core);  break;   // Legacy Sleep
    case 4: SETBIT(g_amec->proc[0].sleep_cnt,i_core);  break;   // Fast Sleep
    case 5: SETBIT(g_amec->proc[0].sleep_cnt,i_core);  break;   // Deep Sleep
    case 6: SETBIT(g_amec->proc[0].winkle_cnt,i_core); break;   // Fast Winkle
    case 7: SETBIT(g_amec->proc[0].winkle_cnt,i_core); break;   // Deep Winkle
  }

  // ------------------------------------------------------
  // Core Memory Hierarchy C LPARx Utilization counters
  // ------------------------------------------------------
  for(i=0; i<4; i++)
  {
      // Extract the utilization counter
      temp32 = i_core_data_ptr->per_partition_memory.count[i];

      // Convert counter to 0.01 Mrps resolution. Since we access every 2 ms:
      // ((2ms read * 500) / 10000)
      temp32a = temp32 - g_amec->proc[0].core[i_core].prev_lpar_mem_cnt[i];
      g_amec->proc[0].core[i_core].prev_lpar_mem_cnt[i] = temp32;
      temp32 = (temp32a * 5) / 100;

      // Store the bandwidth for this LPAR
      g_amec->proc[0].core[i_core].membw[i] = (uint16_t)temp32;
  }

  // Sum up all the memory bandwidth data from the LPARs
  temp32 = g_amec->proc[0].core[i_core].membw[0] +
      g_amec->proc[0].core[i_core].membw[1] +
      g_amec->proc[0].core[i_core].membw[2] +
      g_amec->proc[0].core[i_core].membw[3];

  // Divide by two due to a bug in the hardware
  temp32 = temp32/2;

  // See if core is sleeping/winkled
  if(l_core_sleep_winkle)
    {
        temp32 = 0;
    }
  // Update Sensor for this core
  sensor_update( AMECSENSOR_ARRAY_PTR(CMBW2MSP0C0,i_core), (uint16_t) temp32);

  // ------------------------------------------------------
  // Core Stall counters
  // ------------------------------------------------------
  temp32 = i_core_data_ptr->empath.freq_sens_busy;
  temp32a = g_amec->proc[0].core[i_core].prev_FREQ_SENS_BUSY;
  temp32 = temp32 - temp32a;
  temp32 = temp32 >> 8;

  // See if core is sleeping/winkled
  if(l_core_sleep_winkle)
    {
        temp32 = 0;
    }

  // Update Sensor for this core
  sensor_update( AMECSENSOR_ARRAY_PTR(NOTBZE2MSP0C0,i_core), (uint16_t) temp32);

  temp32 =  i_core_data_ptr->empath.freq_sens_finish;
  temp32a = g_amec->proc[0].core[i_core].prev_FREQ_SENS_FINISH;
  temp32 = temp32 - temp32a;
  temp32 = temp32 >> 8;

  // See if core is sleeping/winkled
  if(l_core_sleep_winkle)
    {
        temp32 = 0;
    }

  // Update Sensor for this core
  sensor_update( AMECSENSOR_ARRAY_PTR(NOTFIN2MSP0C0,i_core), (uint16_t) temp32);

  // ------------------------------------------------------
  // Per Core Normalized Average Utilization
  // ------------------------------------------------------
  // <amec_formula>
  // Result: Calculated Normalized Average Core Utilization
  // Sensor: NUTIL3SP0C0
  // Timescale: 2ms (3s rolling average)
  // Units: 0.01 %
  // Min/Max: 0/10000  (0/100%)
  // </amec_formula>

  // Determine the time interval for the rolling average calculation
  l_time_interval = AMEC_DPS_SAMPLING_RATE * AMEC_IPS_AVRG_INTERVAL;

  // Increment our sample count but prevent it from wrapping
  if(g_amec->proc[0].core[i_core].sample_count < UINT16_MAX)
  {
      g_amec->proc[0].core[i_core].sample_count++;
  }

  if(g_amec->proc[0].core[i_core].sample_count == l_time_interval)
  {
      // Increase resolution of the UTIL accumulator by two decimal places
      temp32 = AMECSENSOR_ARRAY_PTR(UTIL2MSP0C0,i_core)->accumulator * 100;
      // Calculate average utilization of this core
      temp32 = temp32 / g_amec->proc[0].core[i_core].sample_count;
      g_amec->proc[0].core[i_core].avg_util = temp32;

      // Increase resolution of the FREQA accumulator by two decimal places
      temp32 = AMECSENSOR_ARRAY_PTR(FREQA2MSP0C0,i_core)->accumulator * 100;
      // Calculate average frequency of this core
      temp32 = temp32 / g_amec->proc[0].core[i_core].sample_count;
      g_amec->proc[0].core[i_core].avg_freq = temp32;
  }

  if(g_amec->proc[0].core[i_core].sample_count > l_time_interval)
  {
      // Calculate average utilization for this core
      temp32 = (uint32_t) g_amec->proc[0].core[i_core].avg_util;
      temp32 = temp32 * (l_time_interval-1);
      temp32 = temp32 + l_core_util*100;
      g_amec->proc[0].core[i_core].avg_util = temp32 / l_time_interval;

      // Could be needed for increase accuracy
      //if(g_amec->proc[0].core[i_core].avg_util > 9000)
      //{ This rounds up only!
        //g_amec->proc[0].core[i_core].avg_util = (temp32+ (1500-1)) / 1500;
      //}

      // Calculate average frequency for this core
      temp32 = (uint32_t) g_amec->proc[0].core[i_core].avg_freq;
      temp32 = temp32 * (l_time_interval-1);
      temp32 = temp32 + l_core_freq*100;
      g_amec->proc[0].core[i_core].avg_freq = temp32 / l_time_interval;
  }

  // Calculate the normalized utilization for this core
  if(g_amec->proc[0].core[i_core].avg_freq != 0)
  {
      // First, revert back to the original resolution of the sensors
      temp32 = g_amec->proc[0].core[i_core].avg_util / 100;
      temp32a = g_amec->proc[0].core[i_core].avg_freq / 100;

      // Compute now the normalized utilization as follows:
      // Normalized utilization = (Average_utilization)/(Average_frequency) * Fnom
      // Note: The 100000 constant is to increase the precision of our division
      temp32 = (temp32 * 100000) / temp32a;
      temp32 = (temp32 * G_sysConfigData.sys_mode_freq.table[OCC_MODE_NOMINAL]) / 100000;

      // Update sensor for this core
      if(l_core_sleep_winkle)
      {
          sensor_update(AMECSENSOR_ARRAY_PTR(NUTIL3SP0C0, i_core), 0);
      }
      else
      {
          sensor_update(AMECSENSOR_ARRAY_PTR(NUTIL3SP0C0, i_core), (uint16_t)temp32);
      }
  }
}


void amec_calc_ips_sensors(gpe_bulk_core_data_t * i_core_data_ptr, uint8_t i_core)
{
#define     TWO_PWR_24_MASK                 0x00FFFFFF
#define     TWO_PWR_20_MASK                 0x000FFFFF

  /*------------------------------------------------------------------------*/
  /*  Local Variables                                                       */
  /*------------------------------------------------------------------------*/
  INT32                       cyc1 = 0;   //cycle counts
  INT32                       cyc2 = 0;
  UINT32                      fin1 = 0;   //finished instruction counts
  UINT32                      fin2 = 0;
  INT32                       disp1 = 0;  //dispatched instruction counts
  INT32                       disp2 = 0;
  UINT32                      temp32 = 0;
  UINT32                      ticks_2mhz = 0; // IPS sensor interval in 2mhz ticks
  BOOLEAN                     l_core_sleep_winkle = FALSE;
  uint32_t                    l_pm_state_hist_reg = 0;


  // Read the high-order bytes of PM State History register for this core
  l_pm_state_hist_reg = i_core_data_ptr->pcb_slave.pm_history.words.high_order;

  // If core is in fast/deep sleep mode or fast/winkle mode, then set a flag
  // indicating this
  if(l_pm_state_hist_reg & OCC_PAST_CORE_CLK_STOP)
  {
      l_core_sleep_winkle = TRUE;
  }

  /*------------------------------------------------------------------------*/
  /*  Code                                                                  */
  /*------------------------------------------------------------------------*/

  // Get Run Cycles
  cyc1 = i_core_data_ptr->empath.run_cycles;
  cyc2 = g_amec->proc[0].core[i_core].prev_PC_RUN_CYCLES;
  cyc2 = cyc1 - cyc2;

  // Following lines look bogus...the counters are supposed to be 32-bit
  // since we are doing 24-bit unsigned math, we need to account for the
  // overflow case. If this occurs, we mask off the "overflow" to make it behave
  // like a 32-bit subtraction overflow would. Commenting them out.
  //if ( cyc2 < 0 )
  //{
  //  cyc2 &= TWO_PWR_24_MASK;
  //}

  fin1 =  i_core_data_ptr->empath.completion;
  fin2 = g_amec->proc[0].core[i_core].prev_PC_COMPLETED;
  fin2 = fin1 - fin2;

  // Is this counting every completed instruction or 1 of every 16?
  // Why are we masking 20 bits of a 32-bit counter? Commenting these lines out.
  //if ( fin2 < 0 )
  //{
  //  fin2 &= TWO_PWR_20_MASK;
  //}

  disp1 = i_core_data_ptr->empath.dispatch;
  disp2 = g_amec->proc[0].core[i_core].prev_PC_DISPATCH;
  disp2 = disp1 - disp2;

  if ( disp2 < 0 )
  {
    disp2 &= TWO_PWR_20_MASK;
  }

  // ------------------------------------------------------
  // Per Core IPC Calculation
  // ------------------------------------------------------
  // <amec_formula>
  // Result: Calculated Instructions per Cycle
  // Sensor: None
  // Timescale: 2ms
  // Units: 0.01 IPC
  // Min/Max: ?
  // Formula: ipc_delta = (INST_COMPLETE[t=now] - INST_COMPLETE[t=-2ms])
  //          run_cycles = (RUN_CYCLES[t=now] - RUN_CYCLES[t=-2ms])
  //          100 = Convert 0.01 DPC
  //
  //          IPC(in 0.01 IPC) = (ipc_delta * 100) / run_cycles
  // </amec_formula>
  temp32 = (fin2 * 100);  // In units of IPS
  temp32 = temp32 / cyc2; // In units of 0.01 DPC
  g_amec->proc[0].core[i_core].ipc = temp32;


  // ------------------------------------------------------
  // Per Core DPC Calculation
  // ------------------------------------------------------
  // <amec_formula>
  // Result: Calculated dispatched Instructions per Cycle
  // Sensor: None
  // Timescale: 2ms
  // Units: 0.2Mips
  // Min/Max: ?
  // Formula: dpc_delta = (INST_DISPATCH[t=now] - INST_DISPATCH[t=-2ms])
  //          run_cycles = (RUN_CYCLES[t=now] - RUN_CYCLES[t=-2ms])
  //          100 = Convert 0.01 DPC
  //
  //          DPC(in 0.01DPC) = (dpc_delta * 100) / run_cycles
  // </amec_formula>
  temp32 = (disp2 * 100);  // In units of IPS
  temp32 = temp32 / cyc2;  // In units of 0.01 DPC
  g_amec->proc[0].core[i_core].dpc = temp32;

  // ------------------------------------------------------
  // Per Core DPS Calculation
  // ------------------------------------------------------
  // <amec_formula>
  // Result: Calculated dispatched Instructions per Second
  // Sensor: None
  // Timescale: 2ms
  // Units: 0.2Mips
  // Min/Max: ?
  // Formula: dps_delta = (INST_DISPATCH[t=now] - INST_DISPATCH[t=-2ms])
  //          500       = # of 2ms periods in 1 second
  //          50,000 = Convert IPS to 0.2MIPS
  //
  //          DPS(in 0.2Mips) = (dps_delta * 500) / 50,000
  // </amec_formula>
  temp32 = (disp2 * AMEC_SMH_PERIODS_IN_1SEC); // In untis of IPS
  temp32 = temp32 / 50000;  // In units of 0.2Mips (max 327675 Mips for uint16_t)
  g_amec->proc[0].core[i_core].dps = temp32;

  // ------------------------------------------------------
  // Per Core IPS Calculation
  // ------------------------------------------------------
  // <amec_formula>
  // Result: Calculated Instructions per Second
  // Sensor: IPS2MSP0C0
  // Timescale: 2ms
  // Units: 0.2Mips
  // Min/Max: ?
  // Formula:
  //    comp_delta = (INST_COMPLETE[t=now] - INST_COMPLETE[t=-2ms])
  //    ticks_delta = (TOD[t=now] - TOD[t=-2ms])
  //    MIPS = comp_delta (insns/interval) * (1 interval per ticks_delta 2mhz ticks) * (2M 2mhz ticks / s) / 1M
  //         = (2* fin2) / ticks_2mhz
  // Note: For best resolution do multiply first and division last.
  // </amec_formula>

  ticks_2mhz = i_core_data_ptr->empath.tod_2mhz -
      g_amec->proc[0].core[i_core].prev_tod_2mhz;
  temp32 = (fin2 << 1) / ticks_2mhz;
  // See if core is sleeping/winkled
  if(l_core_sleep_winkle)
    {
        temp32 = 0;
    }
  sensor_update( AMECSENSOR_ARRAY_PTR(IPS2MSP0C0,i_core), (uint16_t) temp32);
}


// Function Specification
//
// Name: amec_calc_spurr
//
// Description: Do SPURR calculation.  Must run after FreqA is calculated.
//
// Thread: RealTime Loop
//
// End Function Specification
void amec_calc_spurr(uint8_t i_core)
{
   uint16_t l_actual_freq = AMECSENSOR_ARRAY_PTR(FREQA2MSP0C0, i_core)->sample;
   uint16_t l_nominal = 2790;
   uint32_t temp32;

   // Sanity Check on Freq
   if(l_actual_freq < UPPER_LIMIT_PROC_FREQ_MHZ)
   {
     temp32 = ((uint32_t) (l_actual_freq * 1000) / l_nominal);

     // Scale for SPURR Register (64 = Nominal, 32 = Nom-50%)
     temp32 = (temp32 * 64) / 1000;

     sensor_update( AMECSENSOR_ARRAY_PTR(SPURR2MSP0C0,i_core), (uint16_t) temp32);
   }
}
#endif 

/*----------------------------------------------------------------------------*/
/* End                                                                        */
/*----------------------------------------------------------------------------*/
