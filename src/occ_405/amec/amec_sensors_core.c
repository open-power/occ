/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/amec/amec_sensors_core.c $                        */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2017                        */
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
extern data_cnfg_t * G_data_cnfg;

/******************************************************************************/
/* Forward Declarations                                                       */
/******************************************************************************/
void amec_calc_dts_sensors(CoreData * i_core_data_ptr, uint8_t i_core);
void amec_calc_freq_and_util_sensors(CoreData * i_core_data_ptr, uint8_t i_core);
void amec_calc_ips_sensors(CoreData * i_core_data_ptr, uint8_t i_core);

//*************************************************************************/
// Code
//*************************************************************************/

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
  CoreData  *l_core_data_ptr;
  uint16_t  l_temp16 = 0;
  uint32_t  l_temp32 = 0;
  uint8_t   i = 0;

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
    amec_calc_dps_util_counters(i_core);

    //-------------------------------------------------------
    // IPS
    //-------------------------------------------------------
    // Skip this update if there was an empath collection error
    if (!CORE_EMPATH_ERROR(i_core))
    {
        amec_calc_ips_sensors(l_core_data_ptr,i_core);
    }

    // ------------------------------------------------------
    // Update PREVIOUS values for next time
    // ------------------------------------------------------

    // Thread raw cycles are equivalent to core raw cycles.
    g_amec->proc[0].core[i_core].prev_PC_RAW_Th_CYCLES = l_core_data_ptr->empath.raw_cycles;

    // Skip empath updates if there was an empath collection error on this core
    if (!CORE_EMPATH_ERROR(i_core))
    {
        g_amec->proc[0].core[i_core].prev_PC_RAW_CYCLES    = l_core_data_ptr->empath.raw_cycles;
        g_amec->proc[0].core[i_core].prev_PC_RUN_CYCLES    = l_core_data_ptr->empath.run_cycles;
        g_amec->proc[0].core[i_core].prev_tod_2mhz         = l_core_data_ptr->empath.tod_2mhz;
        g_amec->proc[0].core[i_core].prev_FREQ_SENS_BUSY   = l_core_data_ptr->empath.freq_sens_busy;
        g_amec->proc[0].core[i_core].prev_FREQ_SENS_FINISH = l_core_data_ptr->empath.freq_sens_finish;
    }

    // Need to sum up all thread data for full core data
    g_amec->proc[0].core[i_core].prev_PC_COMPLETED = 0;
    g_amec->proc[0].core[i_core].prev_PC_DISPATCH = 0;
    for(i=0; i<MAX_THREADS_PER_CORE; i++)
    {
        g_amec->proc[0].core[i_core].prev_PC_COMPLETED +=
            l_core_data_ptr->per_thread[i].completion;
        g_amec->proc[0].core[i_core].prev_PC_DISPATCH +=
            l_core_data_ptr->per_thread[i].dispatch;
        g_amec->proc[0].core[i_core].thread[i].prev_PC_RUN_Th_CYCLES = l_core_data_ptr->per_thread[i].run_cycles;
    }

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

// Function Specification
//
// Name: amec_calc_dts_sensors
//
// Description: Compute core temperature. This function is called every
// 4ms/core.
//
// PreCondition: The core is present.
//
// Thread: RealTime Loop
//
// End Function Specification
void amec_calc_dts_sensors(CoreData * i_core_data_ptr, uint8_t i_core)
{
#define DTS_PER_CORE     2
#define QUAD_DTS_PER_CORE     2
#define DTS_INVALID_MASK 0x0C00


    uint32_t      l_coreTemp = 0;
    uint8_t       k = 0;
    uint16_t      l_coreDts[DTS_PER_CORE] = {0};
    uint16_t      l_quadDts[QUAD_DTS_PER_CORE] = {0};
    uint16_t      l_quadDtsTemp = 0;  // The one Quad DTS temp closest to the core
    BOOLEAN       l_update_sensor = FALSE;
    uint16_t      l_core_hot = 0;
    uint8_t       l_coreDtsCnt = 0; // Number of valid Core DTSs
    uint8_t       l_quadDtsCnt = 0; // Number of valid Quad DTSs
    uint32_t      l_dtsAvg = 0;     // Average of the two core or quad dts readings

    uint8_t       cWt = 0;        // core weight: zero unless at least one valid core dts reading
    uint8_t       qWt = 0;        // quad weight: zero unless we have a valid quad dts reading

    uint8_t       l_quad = 0;     // Quad this core resides in

    static bool   L_bad_read_trace = FALSE;

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
                l_coreDts[k] = i_core_data_ptr->dts.core[k].fields.reading;
                l_coreDtsCnt++;

                //Hardware bug workaround:  Temperatures reaching 0 degrees C
                //can show up as negative numbers.  To fix this, we discount
                //values that have the 2 MSB's set.
                if(((l_coreDts[k] & DTS_INVALID_MASK) == DTS_INVALID_MASK) ||
                    (l_coreDts[k] == 0))
                {
                    l_coreDts[k] = 0;
                    l_coreDtsCnt--;
                }

                if (l_coreDts[k] > l_core_hot)
                {
                    l_core_hot = l_coreDts[k];
                }
            }
        } //for loop

        // The core DTSs are considered only if we have at least 1 valid core DTS and
        // a non-zero G_coreWeight. However we want to keep track of the raw core DTS
        // values regardless of weight.
        if (l_coreDtsCnt)
        {
            if (G_data_cnfg->thrm_thresh.proc_core_weight)
            {
                l_update_sensor = TRUE;
                cWt = G_data_cnfg->thrm_thresh.proc_core_weight;
            }

            // Update the raw core DTS reading (average of the two)
            l_dtsAvg = (l_coreDts[0] + l_coreDts[1]) / l_coreDtsCnt;
            sensor_update( AMECSENSOR_ARRAY_PTR(TEMPC0, i_core), l_dtsAvg);
        }

        // The Quad DTS value is considered only if we have a valid Quad DTS and
        // a non-zero quad weight. However we want to keep track of the raw Quad
        // DTS values regardless of weight.
        for (k = 0; k < QUAD_DTS_PER_CORE; k++)
        {
            if (i_core_data_ptr->dts.cache[k].fields.valid)
            {
                l_quadDts[k] = i_core_data_ptr->dts.cache[k].fields.reading;
                l_quadDtsCnt++;
            }
        }

        if(l_quadDtsCnt)
        {
            if (G_data_cnfg->thrm_thresh.proc_quad_weight)
            {
                l_update_sensor = TRUE;
                qWt = G_data_cnfg->thrm_thresh.proc_quad_weight;
            }

            // Determine the quad this core resides in.
            l_quad = i_core / 4;

            // Update the raw quad DTS reading (average of the two)
            l_dtsAvg = (l_quadDts[0] + l_quadDts[1]) / l_quadDtsCnt;
            sensor_update( AMECSENSOR_ARRAY_PTR(TEMPQ0, l_quad), l_dtsAvg);

            // Pick the 1 quad DTS closest to the core for updating the thermal sensor
            // only want 1 quad DTS to handle case when 2 cores from same EX are offline
            // last 2 cores use dts1, first 2 cores use dts0
            if(i_core & 0x02)
                 l_quadDtsTemp = l_quadDts[1];
            else
                 l_quadDtsTemp = l_quadDts[0];

            if(l_quadDtsTemp == 0)
                 qWt = 0;  // No quad temp to include in average
        }

        // Update the thermal sensor associated with this core
        if(l_update_sensor)
        {
            do
            {
                // Make sure data is valid
                if ( !((cWt && l_coreDtsCnt) || qWt) )
                {
                    if(FALSE == L_bad_read_trace)
                    {
                        TRAC_ERR("amec_calc_dts_sensors: updating DTS sensors skipped. "
                                 "core weight: %d, core DTSs: %d, quad weight: %d ",
                                 cWt, l_coreDtsCnt, qWt);
                        L_bad_read_trace = TRUE;
                    }

                    // Avoid divide by zero
                    break;
                }

                //Formula:
                //                (cWt(CoreDTS1 + CoreDTS2) + qWt(QuadDTS))
                //                ------------------------------------------
                //                              (2*cWt + qWt)

                l_coreTemp = ( (cWt * (l_coreDts[0] + l_coreDts[1])) + (qWt * l_quadDtsTemp) ) /
                //           ---------------------------------------------------------------------------------
                                            ( (l_coreDtsCnt * cWt) + qWt );

                // Update sensors & Interim Data
                sensor_update( AMECSENSOR_ARRAY_PTR(TEMPPROCTHRMC0,i_core), l_coreTemp);
                g_amec->proc[0].core[i_core].dts_hottest = l_core_hot;
            }  while(0);
        }
    }
}

// Function Specification
//
// Name: amec_calc_freq_and_util_sensors
//
// Description: Compute the frequency and utilization sensors for a given core.
// This function is called every 4ms/core.
//
// Thread: RealTime Loop
//
// End Function Specification
void amec_calc_freq_and_util_sensors(CoreData * i_core_data_ptr, uint8_t i_core)
{
  BOOLEAN  l_core_sleep_winkle = FALSE;
  uint32_t l_stop_state_hist_reg = 0;
  uint32_t temp32      = 0;
  uint32_t temp32a     = 0;
  uint16_t temp16      = 0;
  uint16_t temp16a     = 0;
  uint16_t l_core_util = 0;
  uint16_t l_core_freq = 0;
  uint16_t l_time_interval = 0;
  uint32_t l_cycles4ms = 0;
  int i;

  // Read the high-order bytes of OCC Stop State History Register
  l_stop_state_hist_reg = (uint32_t) (i_core_data_ptr->stop_state_hist >> 32);

  // If core is in fast/deep sleep mode or fast/winkle mode, then set a flag
  // indicating this
  if(l_stop_state_hist_reg & OCC_CORE_STOP_GATED)
  {
      l_core_sleep_winkle = TRUE;
  }

  // ------------------------------------------------------
  // Per Core Frequency
  // ------------------------------------------------------
  // <amec_formula>
  // Result: Calculated Core Frequency
  // Sensor: FREQAC0
  // Timescale: 4ms
  // Units: MHz
  // Min/Max: 0/6000 (UPPER_LIMIT_PROC_FREQ_MHZ=6000)
  // Formula: cyc_delta(cycles) = (RAW_CYCLES[t=now] - RAW_CYCLES[t=-4ms])
  //          time_delta(TOD ticks) = (TOD[t=now] - TOD[t=-4ms])
  //          frequency(MHz) = (cyc_delta / time_delta) * (2M TOD ticks / 1 second)
  //                         = (2 * cyc_delta) / time_delta
  // NOTE: cyc_delta is the total number of cycles in 4ms time for the core
  // NOTE: In the HWP where we aquire the TOD count, we shift the counter by 8
  //       which causes each TOD tick here to equate to 0.5us. This is why we
  //       are multiplying by 2 in the above equation.
  // </amec_formula>

  // Compute Delta in PC_RAW_CYCLES
  temp32  = i_core_data_ptr->empath.raw_cycles;
  temp32a = g_amec->proc[0].core[i_core].prev_PC_RAW_CYCLES;
  temp32  = l_cycles4ms = temp32 - temp32a;
  temp32a = (i_core_data_ptr->empath.tod_2mhz -
             g_amec->proc[0].core[i_core].prev_tod_2mhz);

  if (0 == temp32a) temp32 = 0;
  else temp32  = (2 * temp32) / temp32a;

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
      sensor_update( AMECSENSOR_ARRAY_PTR(FREQAC0,i_core), l_core_freq);
  }

  // ------------------------------------------------------
  // Per Core Utilization
  // ------------------------------------------------------
  // <amec_formula>
  // Result: Calculated Core Utilization
  // Sensor: UTILC0
  // Timescale: 4ms
  // Units: 0.01 %
  // Min/Max: 0/10000  (0/100%)
  // Formula: cyc_delta = (RAW_CYCLES[t=now] - RAW_CYCLES[t=-4ms])
  //          run_delta = (RUN_CYCLES[t=now] - RUN_CYCLES[t=-4ms])
  //          UTIL(in %) = run_delta / cyc_delta
  //
  // NOTE: cyc_delta is the total number of cycles in 4ms time for the core
  // NOTE: run_delta is the total number of cycles utilized by a specific core in 4ms
  // </amec_formula>

  // Compute Delta in PC_RUN_CYCLES
  temp32 = i_core_data_ptr->empath.run_cycles;
  temp32a = g_amec->proc[0].core[i_core].prev_PC_RUN_CYCLES;
  temp32 = temp32 - temp32a;

  temp32 = temp32 >> 8;       // Drop non-significant bits
  temp16 = (uint16_t) temp32; // Cast to uint16 for mult below
  temp16a = 10000;            // Mult * 10000 to get finer resolution for 0.01%
  temp32 = ((uint32_t)temp16a)*((uint32_t)temp16);

  temp32a = l_cycles4ms;   // Get Raw cycles
  temp32a = temp32a >> 8;  // Drop non-significant bits

  // Calculate Utilization
  if(0 == temp32a) temp32 = 0; // Prevent a divide by zero
  else temp32 = temp32 / temp32a;

  // Update Sensor for this core
  if(l_core_sleep_winkle)
  {
      l_core_util = 0;
  }
  else
  {
      l_core_util = (uint16_t) temp32;
  }
  sensor_update(AMECSENSOR_ARRAY_PTR(UTILC0, i_core), l_core_util);

  // ------------------------------------------------------
  // Per Thread Utilization
  // ------------------------------------------------------
  // <amec_formula>
  // Result: Calculated Core Utilization
  // Sensor: None
  // Timescale: 4ms
  // Units: 0.01 %
  // Min/Max: 0/10000  (0/100%)
  // Formula: cyc_delta = (RAW_CYCLES[t=now] - RAW_CYCLES[t=-4ms])
  //          run_delta = (RUN_CYCLES[t=now] - RUN_CYCLES[t=-4ms])
  //          UTIL(in %) = run_delta / cyc_delta
  //
  // NOTE: cyc_delta is the total number of cycles run by the core in 4ms
  // NOTE: run_delta is the total number of cycles run by a specific thread in 4ms
  // </amec_formula>

  // Get RAW CYCLES for Thread
  // Thread raw cycles are the same as core raw cycles
  temp32  = i_core_data_ptr->empath.raw_cycles;
  temp32a = g_amec->proc[0].core[i_core].prev_PC_RAW_Th_CYCLES;
  l_cycles4ms = temp32 - temp32a;

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

    temp32a = l_cycles4ms;
    temp32a = temp32a >> 8;      // Drop non-significant bits

    // Calculate Utilization
    if (0 == temp32a) temp32 = 0; // Prevent divide by 0
    else temp32 = temp32 / temp32a;

    // Update per thread value for this core
    if(l_core_sleep_winkle)
    {
        temp32 = 0;
    }
    g_amec->proc[0].core[i_core].thread[i].util4ms_thread = (uint16_t) temp32;
  }

  // No sensors to update for perThread Util

  // ------------------------------------------------------
  // Per Core Stop State Sensors
  // ------------------------------------------------------

  // Get deepest idle state requested since the last read. bits 12:15 OCC stop state hist reg
  temp16 = CONVERT_UINT64_UINT16_UPPER(i_core_data_ptr->stop_state_hist);
  temp16 &= 0x000F;
  if(temp16 != 0x000F) // Don't update with reset value
  {
    sensor_update(AMECSENSOR_ARRAY_PTR(STOPDEEPREQC0,i_core), temp16);
  }

  // Get deepest idle state entered by the chiplet since the last read bits 16:19 OCC stop state hist reg
  temp16 = CONVERT_UINT64_UINT16_MIDUPPER(i_core_data_ptr->stop_state_hist);
  temp16 = temp16 >> 12;
  temp16 = temp16 & 0x000F;
  if(temp16 != 0x000F) // Don't update with reset value
  {
    sensor_update(AMECSENSOR_ARRAY_PTR(STOPDEEPACTC0,i_core), temp16);
  }

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
  sensor_update( AMECSENSOR_ARRAY_PTR(NOTBZEC0,i_core), (uint16_t) temp32);

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
  sensor_update( AMECSENSOR_ARRAY_PTR(NOTFINC0,i_core), (uint16_t) temp32);

  // ------------------------------------------------------
  // Per Core Normalized Average Utilization
  // ------------------------------------------------------
  // <amec_formula>
  // Result: Calculated Normalized Average Core Utilization
  // Sensor: NUTILC0
  // Timescale: 4ms (3s rolling average)
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
      temp32 = (uint32_t)AMECSENSOR_ARRAY_PTR(UTILC0,i_core)->accumulator * 100;
      // Calculate average utilization of this core
      temp32 = temp32 / g_amec->proc[0].core[i_core].sample_count;
      g_amec->proc[0].core[i_core].avg_util = temp32;

      // Increase resolution of the FREQA accumulator by two decimal places
      temp32 = (uint32_t)AMECSENSOR_ARRAY_PTR(FREQAC0,i_core)->accumulator * 100;
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
          sensor_update(AMECSENSOR_ARRAY_PTR(NUTILC0, i_core), 0);
      }
      else
      {
          sensor_update(AMECSENSOR_ARRAY_PTR(NUTILC0, i_core), (uint16_t)temp32);
      }
  }
}

void amec_calc_ips_sensors(CoreData * i_core_data_ptr, uint8_t i_core)
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
  uint32_t                    l_stop_state_hist_reg = 0;
  uint8_t                     thread = 0;

  // Read the high-order bytes of OCC Stop State History Register
  l_stop_state_hist_reg = (uint32_t) (i_core_data_ptr->stop_state_hist >> 32);

  // If core is in fast/deep sleep mode or fast/winkle mode, then set a flag
  // indicating this
  if(l_stop_state_hist_reg & OCC_CORE_STOP_GATED)
  {
      l_core_sleep_winkle = TRUE;
  }

  /*------------------------------------------------------------------------*/
  /*  Code                                                                  */
  /*------------------------------------------------------------------------*/

  // Get current and last run Cycles
  cyc1 = i_core_data_ptr->empath.run_cycles;
  cyc2 = g_amec->proc[0].core[i_core].prev_PC_RUN_CYCLES;
  cyc2 = cyc1 - cyc2;

  // Calculate core completion and dispatch (sum of all threads)
  for ( thread = 0; thread < MAX_THREADS_PER_CORE; thread++ )
  {
    fin1 += i_core_data_ptr->per_thread[thread].completion;
    disp1 += i_core_data_ptr->per_thread[thread].dispatch;
  }

  // Calculate delta of completed instructions
  fin2 = g_amec->proc[0].core[i_core].prev_PC_COMPLETED;
  fin2 = fin1 - fin2;

  // Calculate delta of dispatched instructions
  disp2 = g_amec->proc[0].core[i_core].prev_PC_DISPATCH;
  disp2 = disp1 - disp2;

  // ------------------------------------------------------
  // Per Core IPC Calculation
  // ------------------------------------------------------
  // <amec_formula>
  // Result: Calculated Instructions per Cycle
  // Sensor: None
  // Timescale: 4ms
  // Units: 0.01 IPC
  // Min/Max: ?
  // Formula: ipc_delta = (INST_COMPLETE[t=now] - INST_COMPLETE[t=-4ms])
  //          run_cycles = (RUN_CYCLES[t=now] - RUN_CYCLES[t=-4ms])
  //          100 = Convert 0.01 IPC
  //
  //          IPC(in 0.01 IPC) = (ipc_delta * 100) / run_cycles
  // </amec_formula>
  temp32 = (fin2 * 100);        // Number of instructions completed (x100)
  if (0 == cyc2) temp32 = 0;    // Prevent divide by zero
  else temp32 = temp32 / cyc2;  // In units of 0.01 IPC
  g_amec->proc[0].core[i_core].ipc = temp32; // Currently unused


  // ------------------------------------------------------
  // Per Core DPC Calculation
  // ------------------------------------------------------
  // <amec_formula>
  // Result: Calculated dispatched Instructions per Cycle
  // Sensor: None
  // Timescale: 4ms
  // Units: 0.01 DPC
  // Min/Max: ?
  // Formula: dpc_delta  = (INST_DISPATCH[t=now] - INST_DISPATCH[t=-4ms])
  //          run_cycles = (RUN_CYCLES[t=now] - RUN_CYCLES[t=-4ms])
  //          100        = Convert 0.01 DPC
  //
  //          DPC(in 0.01DPC) = (dpc_delta * 100) / run_cycles
  // </amec_formula>
  temp32 = (disp2 * 100);       // Number of instructions dispatched (x100)
  if (0 == cyc2) temp32 = 0;    // Prevent divide by zero
  else temp32 = temp32 / cyc2;  // In units of 0.01 DPC
  g_amec->proc[0].core[i_core].dpc = temp32; // Currently unused

  // ------------------------------------------------------
  // Per Core DPS Calculation
  // ------------------------------------------------------
  // <amec_formula>
  // Result: Calculated dispatched Instructions per Second
  // Sensor: None
  // Timescale: 4ms
  // Units: 0.2Mips
  // Min/Max: ?
  // Formula: dps_delta = (INST_DISPATCH[t=now] - INST_DISPATCH[t=-4ms])
  //          250       = # of 4ms periods in 1 second
  //          50,000    = Convert IPS to 0.2MIPS
  //
  //          DPS(in 0.2Mips) = (dps_delta * 250) / 50,000
  // </amec_formula>

  temp32 = (disp2 * AMEC_CORE_COLLECTION_1SEC); // Number of instructions dispatched extrapolated to 1s.
  temp32 = temp32 / 50000;                      // In units of 0.2Mips (max 327675 Mips for uint16_t)
  g_amec->proc[0].core[i_core].dps = temp32;    // Currently unused

  // ------------------------------------------------------
  // Per Core IPS Calculation
  // ------------------------------------------------------
  // <amec_formula>
  // Result: Calculated Instructions per Second
  // Sensor: IPSC0
  // Timescale: 4ms
  // Units: 0.2Mips
  // Min/Max: ?
  // Formula:
  //    comp_delta = (INST_COMPLETE[t=now] - INST_COMPLETE[t=-4ms])
  //    ticks_delta = (TOD[t=now] - TOD[t=-4ms])
  //    MIPS = comp_delta (insns/interval) * (1 interval per ticks_delta 2mhz ticks) * (2M 2mhz ticks / s) / 1M
  //         = (2* fin2) / ticks_2mhz
  //
  // Note: For best resolution do multiply first and division last.
  // Note: For an explanation regarding the multiply by 2, see the note under FREQAC0.
  // </amec_formula>

  ticks_2mhz = i_core_data_ptr->empath.tod_2mhz -
      g_amec->proc[0].core[i_core].prev_tod_2mhz;

  if (0 == ticks_2mhz) temp32 = 0;
  else temp32 = (fin2 << 1) / ticks_2mhz;

  // See if core is sleeping/winkled
  if(l_core_sleep_winkle)
  {
      temp32 = 0;
  }
  sensor_update( AMECSENSOR_ARRAY_PTR(IPSC0,i_core), (uint16_t) temp32);
}

/*----------------------------------------------------------------------------*/
/* End                                                                        */
/*----------------------------------------------------------------------------*/
