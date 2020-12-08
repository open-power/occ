/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/amec/amec_sensors_core.c $                        */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2020                        */
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
void amec_calc_droop_sensors(CoreData * i_core_data_ptr, uint8_t i_core);

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
  uint32_t  l_temp32 = 0;
  uint16_t  l_core_temp = 0;
  uint16_t  l_quad_temp = 0;
  uint16_t  l_temp16 = 0;
  uint16_t  l_core_util = 0;
  uint16_t  l_core_freq = 0;
  uint16_t  l_time_interval = 0;
  uint8_t   i = 0;
  uint8_t   l_quad = i_core / 4;     // Quad this core resides in

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
    // Skip this update if there was an empath collection error or if previously offline
    if (!CORE_EMPATH_ERROR(i_core) && !CORE_OFFLINE(i_core))
    {
        amec_calc_freq_and_util_sensors(l_core_data_ptr,i_core);
    }

    //-------------------------------------------------------
    // Performance counter - This function should be called
    // after amec_calc_freq_and_util_sensors().
    //-------------------------------------------------------
    if(!CORE_OFFLINE(i_core))
    {
        amec_calc_dps_util_counters(i_core);
    }

    //-------------------------------------------------------
    // IPS
    //-------------------------------------------------------
    // Skip this update if there was an empath collection error
    if (!CORE_EMPATH_ERROR(i_core) && !CORE_OFFLINE(i_core))
    {
        amec_calc_ips_sensors(l_core_data_ptr,i_core);
    }

    //-------------------------------------------------------
    // Update voltage droop counters
    //-------------------------------------------------------
    amec_calc_droop_sensors(l_core_data_ptr, i_core);

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
        g_amec->proc[0].core[i_core].prev_tod_2mhz         = l_core_data_ptr->tod_2mhz;
        g_amec->proc[0].core[i_core].prev_FREQ_SENS_BUSY   = l_core_data_ptr->empath.freq_sens_busy;
        g_amec->proc[0].core[i_core].prev_FREQ_SENS_FINISH = l_core_data_ptr->empath.freq_sens_finish;
    }

    // Need to sum up all thread data for full core data
    g_amec->proc[0].core[i_core].prev_PC_COMPLETED = 0;
    g_amec->proc[0].core[i_core].prev_PC_DISPATCH = 0;
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

    // Core must be online that it was updated and now that the sensors have been updated make sure
    // the core offline bit is off for this core.  Clearing this prior to updating the temperature
    // sensors may result in a false processor timeout error in health monitor
    CLEAR_CORE_OFFLINE(i_core);
  } // if core present and updated

  else if(CORE_OFFLINE(i_core))
  {
    // core wasn't updated due to being offline, update sensors accordingly

    // Determine "core" temperature that will be returned in the poll for fan control
    // If there is at least 1 core online within the same quad use the quad temp else use the nest
    // verify quad temp is valid (not zero) this may be 0 if there were no valid quad DTS
    l_quad_temp = AMECSENSOR_ARRAY_PTR(TEMPQ0, l_quad)->sample;
    if( (QUAD_ONLINE(l_quad)) && l_quad_temp )
    {
       l_core_temp = l_quad_temp;
    }
    else
    {
       uint16_t l_core_temp2 = getSensorByGsid(TEMPNEST1)->sample;
       l_core_temp = getSensorByGsid(TEMPNEST0)->sample;
       if(l_core_temp)
       {
           if(l_core_temp2)
           {
               l_core_temp = (l_core_temp + l_core_temp2)/2;
           }
       }
       else
       {
           l_core_temp = l_core_temp2;
       }
    }
    if(l_core_temp)
    {
       sensor_update(AMECSENSOR_ARRAY_PTR(TEMPPROCTHRMC0,i_core), l_core_temp);
    }

    // Update utilization and frequency sensors to 0
    sensor_update(AMECSENSOR_ARRAY_PTR(NUTILC0, i_core), 0);
    sensor_update(AMECSENSOR_ARRAY_PTR(UTILC0, i_core), 0);
    sensor_update(AMECSENSOR_ARRAY_PTR(IPSC0, i_core), 0);
    sensor_update(AMECSENSOR_ARRAY_PTR(NOTBZEC0, i_core), 0);
    sensor_update(AMECSENSOR_ARRAY_PTR(NOTFINC0, i_core), 0);
    sensor_update(AMECSENSOR_ARRAY_PTR(FREQAC0, i_core), 0);
    for(i=0; i<MAX_THREADS_PER_CORE; i++)
    {
      g_amec->proc[0].core[i_core].thread[i].util4ms_thread = 0;
    }

    // Make updates for rolling average
    // Determine the time interval for the rolling average calculation
    l_time_interval = AMEC_DPS_SAMPLING_RATE * AMEC_IPS_AVRG_INTERVAL;

    // Increment sample count
    if(g_amec->proc[0].core[i_core].sample_count < UINT16_MAX)
    {
       g_amec->proc[0].core[i_core].sample_count++;
    }

    if(g_amec->proc[0].core[i_core].sample_count == l_time_interval)
    {
        // Increase resolution of the UTIL accumulator by two decimal places
        l_temp32 = (uint32_t)AMECSENSOR_ARRAY_PTR(UTILC0,i_core)->accumulator * 100;
        // Calculate average utilization of this core
        l_temp32 = l_temp32 / g_amec->proc[0].core[i_core].sample_count;
        g_amec->proc[0].core[i_core].avg_util = l_temp32;

        // Increase resolution of the FREQA accumulator by two decimal places
        l_temp32 = (uint32_t)AMECSENSOR_ARRAY_PTR(FREQAC0,i_core)->accumulator * 100;
        // Calculate average frequency of this core
        l_temp32 = l_temp32 / g_amec->proc[0].core[i_core].sample_count;
        g_amec->proc[0].core[i_core].avg_freq = l_temp32;
    }
    else if(g_amec->proc[0].core[i_core].sample_count > l_time_interval)
    {
        // Calculate average utilization for this core
        l_temp32 = (uint32_t) g_amec->proc[0].core[i_core].avg_util;
        l_temp32 = l_temp32 * (l_time_interval-1);
        l_temp32 = l_temp32 + l_core_util*100;
        g_amec->proc[0].core[i_core].avg_util = l_temp32 / l_time_interval;

        // Calculate average frequency for this core
        l_temp32 = (uint32_t) g_amec->proc[0].core[i_core].avg_freq;
        l_temp32 = l_temp32 * (l_time_interval-1);
        l_temp32 = l_temp32 + l_core_freq*100;
        g_amec->proc[0].core[i_core].avg_freq = l_temp32 / l_time_interval;
    }
  } // else if core offline
}

// Function Specification
//
// Name: amec_calc_dts_sensors
//
// Description: Compute core temperature. This function is called every
// CORE_DATA_COLLECTION_US/core.
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

    errlHndl_t    l_errl = NULL;
    uint32_t      l_coreTemp = 0;
    uint8_t       k = 0;
    uint16_t      l_coreDts[DTS_PER_CORE] = {0};
    BOOLEAN       l_update_sensor = FALSE;
    uint16_t      l_core_hot = 0;
    uint8_t       l_coreDtsCnt = 0; // Number of valid Core DTSs
    uint8_t       l_L3Dts = 0;
    uint8_t       l_L3DtsCnt = 0;
    uint8_t       l_raceTrackDts = 0;
    uint8_t       l_racetrackDtsCnt = 0;
    uint32_t      l_dtsAvg = 0;     // Average of the two core or quad dts readings

    uint8_t       cWt = 0;        // core weight: zero unless at least one valid core dts reading
    uint8_t       l3Wt = 0;       // L3 cache weight: zero unless we have a valid L3 cache dts reading
    uint8_t       rtWt = 0;       // racetrack weight: zero unless we have a valid racetrack dts reading
    uint8_t       l_quad = 0;     // Quad this core resides in

    static bool   L_bad_read_trace = FALSE;
    static bool   L_core_dts_error_logged[MAX_NUM_CORES][DTS_PER_CORE] = {{FALSE}};
    static bool   L_L3_dts_error_logged[MAX_NUM_CORES] = {FALSE};
    static bool   L_RT_dts_error_logged[MAX_NUM_CORES] = {FALSE};

    if (i_core_data_ptr != NULL)
    {
        //the Core DTS temperatures are considered in the calculation only if:
        //  - They are valid.
        //  - Non-zero.  Module test will detect bad DTS and write coefficients to force 0 reading
        //  - less than DTS saturate temperature
        for (k = 0; k < DTS_PER_CORE; k++)
        {
            //Check validity
            if (i_core_data_ptr->dts.core[k].fields.valid)
            {
                // temperature is only 8 bits of reading field
                l_coreDts[k] = (i_core_data_ptr->dts.core[k].fields.reading & 0xFF);
                l_coreDtsCnt++;

                //Throw out any DTS that is bad
                if(l_coreDts[k] == 0)
                {
                    l_coreDtsCnt--;
                }
                if(l_coreDts[k] >= DTS_MAX_TEMP)
                {
                    // log mfg error for this DTS if haven't already
                    if(L_core_dts_error_logged[i_core][k] == FALSE)
                    {
                         L_core_dts_error_logged[i_core][k] = TRUE;
                         TRAC_ERR("amec_calc_dts_sensors: core[%d] DTS[%d] has invalid reading[%d]",
                                   i_core, k, l_coreDts[k]);
                         /* @
                          * @errortype
                          * @moduleid    AMEC_CALC_DTS_SENSORS
                          * @reasoncode  INVALID_DTS
                          * @userdata1   Core number
                          * @userdata2   DTS reading
                          * @userdata4   ERC_CORE
                          * @devdesc     core DTS bad
                          */
                         l_errl = createErrl(AMEC_CALC_DTS_SENSORS,        //modId
                                             INVALID_DTS,                  //reasoncode
                                             ERC_CORE,                     //Extended reason code
                                             ERRL_SEV_INFORMATIONAL,       //Severity
                                             NULL,                         //Trace Buf
                                             DEFAULT_TRACE_SIZE,           //Trace Size
                                             i_core,                       //userdata1
                                             l_coreDts[k]);                //userdata2

                         // set the mfg action flag (allows callout to be added to info error)
                         setErrlActions(l_errl, ERRL_ACTIONS_MANUFACTURING_ERROR);

                         // add processor callout
                         addCalloutToErrl(l_errl,
                                          ERRL_CALLOUT_TYPE_HUID,
                                          G_sysConfigData.proc_huid,
                                          ERRL_CALLOUT_PRIORITY_HIGH);

                         // Commit Error
                         commitErrl(&l_errl);
                    }
                    // throw out this DTS
                    l_coreDts[k] = 0;
                    l_coreDtsCnt--;
                }

                if (l_coreDts[k] > l_core_hot)
                {
                    l_core_hot = l_coreDts[k];
                }
            }
        } //for loop

        // Set the L3 DTS reading if it is valid
        if( (i_core_data_ptr->dts.cache.fields.valid) &&
            (i_core_data_ptr->dts.cache.fields.reading != 0) )
        {
            // Don't use DTS if it reached max limit
            if(i_core_data_ptr->dts.cache.fields.reading >= DTS_MAX_TEMP)
            {
                // log mfg error for this DTS if haven't already
                if(L_L3_dts_error_logged[i_core] == FALSE)
                {
                     L_L3_dts_error_logged[i_core] = TRUE;
                     TRAC_ERR("amec_calc_dts_sensors: core[%d] L3 DTS has invalid reading[%d]",
                               i_core, i_core_data_ptr->dts.cache.fields.reading);
                     /* @
                      * @errortype
                      * @moduleid    AMEC_CALC_DTS_SENSORS
                      * @reasoncode  INVALID_DTS
                      * @userdata1   Core number
                      * @userdata2   DTS reading
                      * @userdata4   ERC_L3
                      * @devdesc     L3 DTS bad
                      */
                     l_errl = createErrl(AMEC_CALC_DTS_SENSORS,        //modId
                                         INVALID_DTS,                  //reasoncode
                                         ERC_L3,                       //Extended reason code
                                         ERRL_SEV_INFORMATIONAL,       //Severity
                                         NULL,                         //Trace Buf
                                         DEFAULT_TRACE_SIZE,           //Trace Size
                                         i_core,                       //userdata1
                                         i_core_data_ptr->dts.cache.fields.reading);

                     // set the mfg action flag (allows callout to be added to info error)
                     setErrlActions(l_errl, ERRL_ACTIONS_MANUFACTURING_ERROR);

                     // add processor callout
                     addCalloutToErrl(l_errl,
                                      ERRL_CALLOUT_TYPE_HUID,
                                      G_sysConfigData.proc_huid,
                                      ERRL_CALLOUT_PRIORITY_HIGH);

                     // Commit Error
                     commitErrl(&l_errl);
                }
            }
            else  // DTS reading is good
            {
                l_L3Dts = i_core_data_ptr->dts.cache.fields.reading;
                l_L3DtsCnt++;
            }
        } // if L3 DTS valid

        // Set the racetrack DTS reading if it is valid
        if( (i_core_data_ptr->dts.racetrack.fields.valid) &&
            (i_core_data_ptr->dts.racetrack.fields.reading != 0) )
        {
            // Don't use DTS if it reached max limit
            if(i_core_data_ptr->dts.racetrack.fields.reading >= DTS_MAX_TEMP)
            {
                // log mfg error for this DTS if haven't already
                if(L_RT_dts_error_logged[i_core] == FALSE)
                {
                     L_RT_dts_error_logged[i_core] = TRUE;
                     TRAC_ERR("amec_calc_dts_sensors: core[%d] Racetrack DTS has invalid reading[%d]",
                               i_core, i_core_data_ptr->dts.racetrack.fields.reading);
                     /* @
                      * @errortype
                      * @moduleid    AMEC_CALC_DTS_SENSORS
                      * @reasoncode  INVALID_DTS
                      * @userdata1   Core number
                      * @userdata2   DTS reading
                      * @userdata4   ERC_RACETRACK
                      * @devdesc     Racetrack DTS bad
                      */
                     l_errl = createErrl(AMEC_CALC_DTS_SENSORS,        //modId
                                         INVALID_DTS,                  //reasoncode
                                         ERC_RACETRACK,                //Extended reason code
                                         ERRL_SEV_INFORMATIONAL,       //Severity
                                         NULL,                         //Trace Buf
                                         DEFAULT_TRACE_SIZE,           //Trace Size
                                         i_core,                       //userdata1
                                         i_core_data_ptr->dts.racetrack.fields.reading);

                     // set the mfg action flag (allows callout to be added to info error)
                     setErrlActions(l_errl, ERRL_ACTIONS_MANUFACTURING_ERROR);

                     // add processor callout
                     addCalloutToErrl(l_errl,
                                      ERRL_CALLOUT_TYPE_HUID,
                                      G_sysConfigData.proc_huid,
                                      ERRL_CALLOUT_PRIORITY_HIGH);

                     // Commit Error
                     commitErrl(&l_errl);
                }
            }
            else  // DTS reading is good
            {
                l_raceTrackDts = i_core_data_ptr->dts.racetrack.fields.reading;
                l_racetrackDtsCnt++;
            }
        } // if RT DTS valid

        // The core DTSs are considered only if we have at least 1 valid DTS value
        // between the L3 DTS and the 2 core DTSs along with a non-zero weight for
        // the respective DTS. However we want to keep track of the raw core DTS
        // values regardless of weight.
        if (l_coreDtsCnt || l_L3DtsCnt)
        {
            if( (G_data_cnfg->thrm_thresh.proc_core_weight) && (l_coreDtsCnt) )
            {
                l_update_sensor = TRUE;
                cWt = G_data_cnfg->thrm_thresh.proc_core_weight;
            }

            if( (G_data_cnfg->thrm_thresh.proc_L3_weight) && (l_L3DtsCnt) )
            {
                l_update_sensor = TRUE;
                l3Wt = G_data_cnfg->thrm_thresh.proc_L3_weight;
            }

            // Update the raw non-weighted core DTS reading (average of the two core + L3)
            l_dtsAvg = (l_coreDts[0] + l_coreDts[1] + l_L3Dts) / (l_coreDtsCnt + l_L3DtsCnt);
            sensor_update( AMECSENSOR_ARRAY_PTR(TEMPC0, i_core), l_dtsAvg);
        }

        // The Quad/racetrack DTS value is considered only if we have a valid racetrack DTS and
        // a non-zero racetrack weight. However we want to keep track of the raw Quad/racetrack
        // DTS values regardless of weight.
        if(l_racetrackDtsCnt)
        {
            if (G_data_cnfg->thrm_thresh.proc_racetrack_weight)
            {
                l_update_sensor = TRUE;
                rtWt = G_data_cnfg->thrm_thresh.proc_racetrack_weight;
            }

            // Determine the quad this core resides in.
            l_quad = i_core / 4;

            // Update the quad sensor reading with the racetrack DTS
            sensor_update( AMECSENSOR_ARRAY_PTR(TEMPQ0, l_quad), l_raceTrackDts);

            if(l_raceTrackDts == 0)
                 rtWt = 0;  // No quad temp to include in average
        }

        // Update the thermal sensor associated with this core
        if(l_update_sensor)
        {
            do
            {
                // Make sure data is valid
                if ( !((cWt && l_coreDtsCnt) || rtWt || l3Wt) )
                {
                    if(FALSE == L_bad_read_trace)
                    {
                        TRAC_ERR("amec_calc_dts_sensors: updating DTS sensors skipped. "
                                 "core weight: %d, core DTSs: %d, racetrack weight: %d "
                                 "L3 weight: %d",
                                 cWt, l_coreDtsCnt, rtWt, l3Wt);
                        L_bad_read_trace = TRUE;
                    }

                    // Avoid divide by zero
                    break;
                }

                //Formula:
                //                (cWt(CoreDTS1 + CoreDTS2) + rtWt(RaceTrackDTS) + l3Wt(L3DTS) )
                //                ---------------------------------------------------------------
                //                                   (2*cWt + rtWt + l3Wt)

                l_coreTemp = ( (cWt * (l_coreDts[0] + l_coreDts[1])) + (rtWt * l_raceTrackDts) + (l3Wt * l_L3Dts) ) /
                //           --------------------------------------------------------------------------------------
                                                  ( (l_coreDtsCnt * cWt) + rtWt + l3Wt );

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
// This function is called CORE_DATA_COLLECTION_US per core.
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
  uint16_t l_core_util = 0;
  uint16_t l_core_freq = 0;
  uint16_t l_time_interval = 0;
  uint32_t l_cycles4ms = 0;

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
  // Timescale: CORE_DATA_COLLECTION_US
  // Units: MHz
  // Min/Max: 0/6000 (UPPER_LIMIT_PROC_FREQ_MHZ=6000)
  // Formula: cyc_delta(cycles) = (RAW_CYCLES[t=now] - RAW_CYCLES[t=-CORE_DATA_COLLECTION_US])
  //          time_delta(TOD ticks) = (TOD[t=now] - TOD[t=-CORE_DATA_COLLECTION_US])
  //          frequency(MHz) = (cyc_delta / time_delta) * (2M TOD ticks / 1 second)
  //                         = (2 * cyc_delta) / time_delta
  // NOTE: cyc_delta is the total number of cycles in CORE_DATA_COLLECTION_US time for the core
  // NOTE: In the HWP where we aquire the TOD count, we shift the counter by 8
  //       which causes each TOD tick here to equate to 0.5us. This is why we
  //       are multiplying by 2 in the above equation.
  // </amec_formula>

  // Compute Delta in PC_RAW_CYCLES
  temp32  = i_core_data_ptr->empath.raw_cycles;
  temp32a = g_amec->proc[0].core[i_core].prev_PC_RAW_CYCLES;
  temp32  = l_cycles4ms = temp32 - temp32a;
  temp32a = (i_core_data_ptr->tod_2mhz -
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
  // Timescale: CORE_DATA_COLLECTION_US
  // Units: 0.01 %
  // Min/Max: 0/10000  (0/100%)
  // Formula: cyc_delta = (RAW_CYCLES[t=now] - RAW_CYCLES[t=-CORE_DATA_COLLECTION_US])
  //          run_delta = (RUN_CYCLES[t=now] - RUN_CYCLES[t=-CORE_DATA_COLLECTION_US])
  //          UTIL(in %) = run_delta / cyc_delta
  //
  // NOTE: cyc_delta is the total number of cycles in CORE_DATA_COLLECTION_US time for the core
  // NOTE: run_delta is the total number of cycles utilized by a specific core in CORE_DATA_COLLECTION_US
  // </amec_formula>

  // Compute Delta in PC_RUN_CYCLES
  temp32 = i_core_data_ptr->empath.run_cycles;
  temp32a = g_amec->proc[0].core[i_core].prev_PC_RUN_CYCLES;
  temp32 = temp32 - temp32a;

  temp32 = temp32 >> 8;       // Drop non-significant bits
  temp32 = temp32 * 10000;    // .01% resolution

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
  // Timescale: CORE_DATA_COLLECTION_US
  // Units: 0.01 %
  // Min/Max: 0/10000  (0/100%)
  // Formula: cyc_delta = (RAW_CYCLES[t=now] - RAW_CYCLES[t=-CORE_DATA_COLLECTION_US])
  //          run_delta = (RUN_CYCLES[t=now] - RUN_CYCLES[t=-CORE_DATA_COLLECTION_US])
  //          UTIL(in %) = run_delta / cyc_delta
  //
  // NOTE: cyc_delta is the total number of cycles run by the core in CORE_DATA_COLLECTION_US
  // NOTE: run_delta is the total number of cycles run by a specific thread in CORE_DATA_COLLECTION_US
  // </amec_formula>

  // Get RAW CYCLES for Thread
  // Thread raw cycles are the same as core raw cycles
  temp32  = i_core_data_ptr->empath.raw_cycles;
  temp32a = g_amec->proc[0].core[i_core].prev_PC_RAW_Th_CYCLES;
  l_cycles4ms = temp32 - temp32a;

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
  // Timescale: CORE_DATA_COLLECTION_US (3s rolling average)
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
      temp32 = (temp32 * G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MODE_DISABLED]) / 100000;

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
  // Timescale: CORE_DATA_COLLECTION_US
  // Units: 0.01 IPC
  // Min/Max: ?
  // Formula: ipc_delta = (INST_COMPLETE[t=now] - INST_COMPLETE[t=-CORE_DATA_COLLECTION_US])
  //          run_cycles = (RUN_CYCLES[t=now] - RUN_CYCLES[t=-CORE_DATA_COLLECTION_US])
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
  // Timescale: CORE_DATA_COLLECTION_US
  // Units: 0.01 DPC
  // Min/Max: ?
  // Formula: dpc_delta  = (INST_DISPATCH[t=now] - INST_DISPATCH[t=-CORE_DATA_COLLECTION_US])
  //          run_cycles = (RUN_CYCLES[t=now] - RUN_CYCLES[t=-CORE_DATA_COLLECTION_US])
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
  // Timescale: CORE_DATA_COLLECTION_US
  // Units: 0.2Mips
  // Min/Max: ?
  // Formula: dps_delta = (INST_DISPATCH[t=now] - INST_DISPATCH[t=-CORE_DATA_COLLECTION_US])
  //          250       = # of CORE_DATA_COLLECTION_US periods in 1 second
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
  // Timescale: CORE_DATA_COLLECTION_US
  // Units: 0.2Mips
  // Min/Max: ?
  // Formula:
  //    comp_delta = (INST_COMPLETE[t=now] - INST_COMPLETE[t=-CORE_DATA_COLLECTION_US])
  //    ticks_delta = (TOD[t=now] - TOD[t=-CORE_DATA_COLLECTION_US])
  //    MIPS = comp_delta (insns/interval) * (1 interval per ticks_delta 2mhz ticks) * (2M 2mhz ticks / s) / 1M
  //         = (2* fin2) / ticks_2mhz
  //
  // Note: For best resolution do multiply first and division last.
  // Note: For an explanation regarding the multiply by 2, see the note under FREQAC0.
  // </amec_formula>

  ticks_2mhz = i_core_data_ptr->tod_2mhz -
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

// -------------------------------------------------
//  Droop count sum for core and quad
// ------------------------------------------------
void amec_calc_droop_sensors(CoreData * i_core_data_ptr, uint8_t i_core)
{
    //CoreData only has any new droop events since the last time CoreData was read
    uint32_t l_core_droops = i_core_data_ptr->droop.v_droop_small;
    sensor_t * l_core_sensor = AMECSENSOR_ARRAY_PTR(VOLTDROOPCNTC0, i_core);
    sensor_update( l_core_sensor, l_core_droops);

    // Update ERRH counters so it is known voltage droops are happening in call home data
    if(l_core_droops)
    {
       INCREMENT_ERR_HISTORY(ERRH_CORE_SMALL_DROOP);
    }

    // Handle Digitial Droop Sensor (DDS) data
    // save data to calculate average of DDS_DATA across all cores
    if(i_core_data_ptr->dds.fields.dds_valid)
    {
        G_chip_dds.sum += i_core_data_ptr->dds.fields.dds_reading;
        G_chip_dds.sum_num_cores++;
    }

    // check if this core has the new minimum DDS_MIN
    if( (i_core_data_ptr->dds.fields.dds_min_valid) &&
        (i_core_data_ptr->dds.fields.dds_min < G_chip_dds.min) )
    {
        G_chip_dds.min = i_core_data_ptr->dds.fields.dds_min;
        G_chip_dds.min_core = i_core;
    }
}

// Function Specification
//
// Name: amec_update_proc_level_sensors
//
// Description: combine core data into processor level sensors
//
// Thread: RealTime Loop
//
// End Function Specification
void amec_update_proc_level_sensors(void)
{
  uint16_t  l_avg = 0;

  // Update Digital Droop sensor average
  if(G_chip_dds.sum_num_cores)
  {
    l_avg = G_chip_dds.sum / G_chip_dds.sum_num_cores;
    if(l_avg)
        sensor_update( AMECSENSOR_PTR(DDSAVG), l_avg);
  }
  // Digitial Droop sensor minimum
  if( (G_chip_dds.min != 0xffff) && (G_chip_dds.min_core != 0xff) )
  {
    sensor_update( AMECSENSOR_PTR(DDSMIN), G_chip_dds.min);
    // save the core that was minimum into sensor status
    AMECSENSOR_PTR(DDSMIN)->status.sample_info = G_chip_dds.min_core;
  }
  // reset fields for new core readings
  G_chip_dds.sum = 0;
  G_chip_dds.sum_num_cores = 0;
  G_chip_dds.min_core = 0xff;
  G_chip_dds.min = 0xffff;
}

/*----------------------------------------------------------------------------*/
/* End                                                                        */
/*----------------------------------------------------------------------------*/
