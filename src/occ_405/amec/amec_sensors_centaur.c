/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/amec/amec_sensors_centaur.c $                     */
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

/******************************************************************************/
/* includes                                                                   */
/******************************************************************************/
#include <occ_common.h>
#include <ssx.h>
#include <errl.h>               // Error logging
#include "sensor.h"
#include "rtls.h"
#include "occ_sys_config.h"
#include "occ_service_codes.h"  // for SSX_GENERIC_FAILURE
#include "dcom.h"
#include "memory.h"
#include "centaur_data.h"
#include "amec_smh.h"
#include "amec_slave_smh.h"
#include <trac.h>
#include "amec_sys.h"
#include "sensor_enum.h"
#include "amec_service_codes.h"
#include <amec_sensors_centaur.h>
#include "centaur_mem_data.h"

/******************************************************************************/
/* Globals                                                                    */
/******************************************************************************/
dimm_sensor_flags_t G_dimm_overtemp_bitmap = {0};
dimm_sensor_flags_t G_dimm_temp_updated_bitmap = {0};
uint8_t             G_cent_overtemp_bitmap = 0;
uint8_t             G_cent_temp_updated_bitmap = 0;
extern uint8_t      G_centaur_needs_recovery;
extern uint8_t      G_centaur_nest_lfir6;
extern uint64_t G_inject_dimm;
extern uint32_t G_inject_dimm_trace[MAX_NUM_CENTAURS][NUM_DIMMS_PER_CENTAUR];


/******************************************************************************/
/* Forward Declarations                                                       */
/******************************************************************************/
void amec_update_dimm_dts_sensors(CentaurMemData * i_sensor_cache, uint8_t i_centaur);
void amec_update_centaur_dts_sensors(CentaurMemData * i_sensor_cache, uint8_t i_centaur);
void amec_perfcount_getmc( CentaurMemData * i_sensor_cache, uint8_t i_centaur);

/******************************************************************************/
/* Code                                                                       */
/******************************************************************************/

// Function Specification
//
// Name: amec_update_dimm_dts_sensors
//
// Description: Updates sensors that have data grabbed by the fast core data
// task.
//
// Thread: RealTime Loop
//
// End Function Specification
void amec_update_centaur_sensors(uint8_t i_centaur)
{
    if(CENTAUR_PRESENT(i_centaur))
    {
        CentaurMemData * l_sensor_cache = cent_get_centaur_data_ptr(i_centaur);
        if(CENTAUR_UPDATED(i_centaur))
        {
            amec_update_dimm_dts_sensors(l_sensor_cache, i_centaur);
            amec_update_centaur_dts_sensors(l_sensor_cache, i_centaur);
            amec_perfcount_getmc(l_sensor_cache, i_centaur);
        }
        CLEAR_CENTAUR_UPDATED(i_centaur);
    }
}

// Function Specification
//
// Name: amec_update_dimm_dts_sensors
//
// Description: Updates sensors that have data grabbed by the fast core data
// task.
//
// Thread: RealTime Loop
//
// End Function Specification
void amec_update_dimm_dts_sensors(CentaurMemData * i_sensor_cache, uint8_t i_centaur)
{
#define MIN_VALID_DIMM_TEMP 1
#define MAX_VALID_DIMM_TEMP 125 //according to Mike Pardiek
#define MAX_MEM_TEMP_CHANGE 2

    uint32_t k, l_hottest_dimm_temp;
    uint16_t l_dts[NUM_DIMMS_PER_CENTAUR] = {0};
    uint32_t l_hottest_dimm_loc = NUM_DIMMS_PER_CENTAUR;
    uint32_t l_sens_status;
    int32_t  l_dimm_temp, l_prev_temp;
    static uint8_t L_ran_once[MAX_NUM_CENTAURS] = {FALSE};

    // Harvest thermal data for all dimms
    for(k=0; k < NUM_DIMMS_PER_CENTAUR; k++)
    {
        if(!CENTAUR_SENSOR_ENABLED(i_centaur, k))
        {
            continue;
        }


        if (g_amec->proc[0].memctl[i_centaur].centaur.dimm_temps[k].temp_sid) // DIMM has sensor ID
        {
            if ((G_inject_dimm & ((uint64_t)1 << ((i_centaur * 8) + k))) == 0)
            {
                if (G_inject_dimm_trace[i_centaur][k] != 0)
                {
                    TRAC_INFO("amec_update_dimm_dts_sensors: stopping injection of errors for DIMM%04X", (i_centaur<<8)|k);
                    G_inject_dimm_trace[i_centaur][k] = 0;
                }
            }
            else
            {
                if (G_inject_dimm_trace[i_centaur][k] == 0)
                {
                    TRAC_INFO("amec_update_dimm_dts_sensors: injecting errors for DIMM%04X", (i_centaur<<8)|k);
                    G_inject_dimm_trace[i_centaur][k] = 1;
                }
                continue; // Skip this DIMM
            }
        }


        l_sens_status = i_sensor_cache->scache.dimm_thermal_sensor[k].fields.status;
        fru_temp_t* l_fru = &g_amec->proc[0].memctl[i_centaur].centaur.dimm_temps[k];

        l_dimm_temp = i_sensor_cache->scache.dimm_thermal_sensor[k].fields.temperature;
        l_prev_temp = l_fru->cur_temp;
        if(!l_prev_temp)
        {
            l_prev_temp = l_dimm_temp;
        }

        //Check DTS status bits.
        if(l_sens_status == DIMM_SENSOR_STATUS_VALID_NEW)
        {
            //make sure temperature is within a 'reasonable' range.
            if(l_dimm_temp < MIN_VALID_DIMM_TEMP ||
               l_dimm_temp > MAX_VALID_DIMM_TEMP)
            {
                //set a flag so that if we end up logging an error we have something to debug why
                l_fru->flags |= FRU_TEMP_OUT_OF_RANGE;
                l_dts[k] = l_prev_temp;
            }
            else
            {
                //don't allow temp to change more than is reasonable for 2ms
                if(l_dimm_temp > (l_prev_temp + MAX_MEM_TEMP_CHANGE))
                {
                    l_dts[k] = l_prev_temp + MAX_MEM_TEMP_CHANGE;
                    if(!l_fru->flags)
                    {
                        TRAC_INFO("dimm temp rose faster than reasonable: cent[%d] dimm[%d] prev[%d] cur[%d]",
                                  i_centaur, k, l_prev_temp, l_dimm_temp);
                        l_fru->flags |= FRU_TEMP_FAST_CHANGE;
                    }
                }
                else if (l_dimm_temp < (l_prev_temp - MAX_MEM_TEMP_CHANGE))
                {
                    l_dts[k] = l_prev_temp - MAX_MEM_TEMP_CHANGE;
                    if(!l_fru->flags)
                    {
                        TRAC_INFO("dimm temp fell faster than reasonable: cent[%d] dimm[%d] prev[%d] cur[%d]",
                                  i_centaur, k, l_prev_temp, l_dimm_temp);
                        l_fru->flags |= FRU_TEMP_FAST_CHANGE;
                    }
                }
                else //reasonable amount of change occurred
                {
                    l_dts[k] = l_dimm_temp;
                    l_fru->flags &= ~FRU_TEMP_FAST_CHANGE;
                }

                //Notify thermal thread that temperature has been updated
                G_dimm_temp_updated_bitmap.bytes[i_centaur] |= DIMM_SENSOR0 >> k;

                //clear error flags
                l_fru->flags &= FRU_TEMP_FAST_CHANGE;
            }
        }
        else //status was VALID_OLD, ERROR, or STALLED
        {
            //convert status number to a flag
            uint8_t l_status_flag = 1 << l_sens_status;

            if(L_ran_once[i_centaur])
            {
                //Trace the error if we haven't traced it already for this sensor
                if((l_sens_status != DIMM_SENSOR_STATUS_VALID_OLD) &&
                   !(l_status_flag & l_fru->flags))
                {
                    TRAC_INFO("Centaur%d sensor%d error: %d", i_centaur, k, l_sens_status);
                }

                l_fru->flags |= l_status_flag;
            }

            //use last temperature
            l_dts[k] = l_prev_temp;

            //request recovery (disable and re-enable sensor cache collection)
            if(l_sens_status == DIMM_SENSOR_STATUS_ERROR)
            {
                G_centaur_needs_recovery |= CENTAUR0_PRESENT_MASK >> i_centaur;
            }
        }

        //Check if at or above the error temperature
        if(l_dts[k] >= g_amec->thermaldimm.ot_error)
        {
            //Set a bit so that this dimm can be called out by the thermal thread
            G_dimm_overtemp_bitmap.bytes[i_centaur] |= 1 << k;
        }
    }

    // Find hottest temperature from all DIMMs for this centaur
    for(l_hottest_dimm_temp = 0, k = 0; k < NUM_DIMMS_PER_CENTAUR; k++)
    {
        if(l_dts[k] > l_hottest_dimm_temp)
        {
            l_hottest_dimm_temp = l_dts[k];
            l_hottest_dimm_loc = k;
        }
        g_amec->proc[0].memctl[i_centaur].centaur.dimm_temps[k].cur_temp =  l_dts[k];
    }

    amec_centaur_t* l_centaur_ptr = &g_amec->proc[0].memctl[i_centaur].centaur;

    //only update location if hottest dimm temp is greater than previous maximum
    if(l_hottest_dimm_temp > l_centaur_ptr->tempdimmax.sample_max)
    {
        sensor_update(&l_centaur_ptr->locdimmax, l_hottest_dimm_loc);
    }

    //update the max dimm temperature sensor for this centaur
    sensor_update(&l_centaur_ptr->tempdimmax, l_hottest_dimm_temp);

    L_ran_once[i_centaur] = TRUE;
    AMEC_DBG("Centaur[%d]: HotDimm=%d\n",i_centaur,l_hottest_dimm_temp);
}

// Function Specification
//
// Name: amec_update_centaur_dts_sensors
//
// Description: Updates sensors that have data grabbed by the fast core data
//
// Thread: RealTime Loop
//
// End Function Specification
void amec_update_centaur_dts_sensors(CentaurMemData * i_sensor_cache, uint8_t i_centaur)
{
#define MIN_VALID_CENT_TEMP 1
#define MAX_VALID_CENT_TEMP 125 //according to Mike Pardiek
#define CENT_SENSOR_STATUS_VALID 0x0001
#define CENT_SENSOR_TEMP_MASK    0xfff0

    uint16_t l_dts, l_sens_value0, l_sens_value1;
    uint16_t l_sens_status0, l_sens_status1, l_sens_status;
    uint16_t l_sens_temp0, l_sens_temp1, l_cent_temp;
    int32_t  l_prev_temp;
    static uint8_t L_ran_once[MAX_NUM_CENTAURS] = {FALSE};

    l_sens_value0 = i_sensor_cache->scache.centaur_thermal_sensor[0].value;
    l_sens_value1 = i_sensor_cache->scache.centaur_thermal_sensor[1].value;
    l_sens_status0 = l_sens_value0 & CENT_SENSOR_STATUS_VALID;
    l_sens_status1 = l_sens_value1 & CENT_SENSOR_STATUS_VALID;
    l_sens_temp0 = (l_sens_status0)? (l_sens_value0 & CENT_SENSOR_TEMP_MASK) >>  4:
                   0;
    l_sens_temp1 = (l_sens_status1)? (l_sens_value1 & CENT_SENSOR_TEMP_MASK) >>  4:
                   0;
    l_sens_status = l_sens_status0 | l_sens_status1;
    l_cent_temp = (l_sens_temp0 > l_sens_temp1)? l_sens_temp0: l_sens_temp1;

    fru_temp_t* l_fru = &g_amec->proc[0].memctl[i_centaur].centaur.centaur_hottest;

    l_prev_temp = l_fru->cur_temp;
    if(!l_prev_temp)
    {
        l_prev_temp = l_cent_temp;
    }

    //Check DTS status bits and NEST LFIR bit 6 for a valid temperature.
    if(l_sens_status == CENT_SENSOR_STATUS_VALID &&
       !(G_centaur_nest_lfir6 & (CENTAUR0_PRESENT_MASK >> i_centaur)))
    {
        //make sure temperature is within a 'reasonable' range.
        if(l_cent_temp < MIN_VALID_CENT_TEMP ||
           l_cent_temp > MAX_VALID_CENT_TEMP)
        {
            //set a flag so that if we end up logging an error we have something to debug why
            l_fru->flags |= FRU_TEMP_OUT_OF_RANGE;
            l_dts = l_prev_temp;
        }
        else
        {
            //don't allow temp to change more than is reasonable for 2ms
            if(l_cent_temp > (l_prev_temp + MAX_MEM_TEMP_CHANGE))
            {
                l_dts = l_prev_temp + MAX_MEM_TEMP_CHANGE;
                if(!l_fru->flags)
                {
                    TRAC_INFO("centaur temp rose faster than reasonable: cent[%d] prev[%d] cur[%d]",
                              i_centaur, l_prev_temp, l_cent_temp);
                    l_fru->flags |= FRU_TEMP_FAST_CHANGE;
                }
            }
            else if (l_cent_temp < (l_prev_temp - MAX_MEM_TEMP_CHANGE))
            {
                l_dts = l_prev_temp - MAX_MEM_TEMP_CHANGE;
                if(!l_fru->flags)
                {
                    TRAC_INFO("centaur temp fell faster than reasonable: cent[%d] prev[%d] cur[%d]",
                                  i_centaur, l_prev_temp, l_cent_temp);
                    l_fru->flags |= FRU_TEMP_FAST_CHANGE;
                }
            }
            else //reasonable amount of change occurred
            {
                l_dts = l_cent_temp;
                l_fru->flags &= ~FRU_TEMP_FAST_CHANGE;
            }

            //Notify thermal thread that temperature has been updated
            G_cent_temp_updated_bitmap |= CENTAUR0_PRESENT_MASK >> i_centaur;

            //clear error flags
            l_fru->flags &= FRU_TEMP_FAST_CHANGE;
        }
    }
    else //status was INVALID
    {
        if(L_ran_once[i_centaur])
        {
            //Trace the error if we haven't traced it already for this sensor
            //and this wasn't only caused by LFIR[6] (which is traced elsewhere).
            if(!(l_fru->flags & FRU_SENSOR_STATUS_INVALID) &&
               !(l_sens_status == CENT_SENSOR_STATUS_VALID))
            {
                TRAC_INFO("Centaur%d temp invalid. nest_lfir6=0x%02x", i_centaur, G_centaur_nest_lfir6);
            }

            l_fru->flags |= FRU_SENSOR_STATUS_INVALID;

            if(G_centaur_nest_lfir6 & (CENTAUR0_PRESENT_MASK >> i_centaur))
            {
                l_fru->flags |= FRU_SENSOR_CENT_NEST_FIR6;
            }
        }

        //use last temperature
        l_dts = l_prev_temp;
    }

    L_ran_once[i_centaur] = TRUE;

    //Check if at or above the error temperature
    if(l_dts >= g_amec->thermalcent.ot_error)
    {
        //Set a bit so that this dimm can be called out by the thermal thread
        G_cent_overtemp_bitmap |= (CENTAUR0_PRESENT_MASK >> i_centaur);
    }

    // Update Interim Data - later this will get picked up to form centaur sensor
    g_amec->proc[0].memctl[i_centaur].centaur.centaur_hottest.cur_temp = l_dts;

    AMEC_DBG("Centaur[%d]: HotCentaur=%d\n",i_centaur,l_dts);
}

// Function Specification
//
// Name: amec_update_centaur_temp_sensors
//
// Description: Updates thermal sensors that have data grabbed by the centaur.
//
// Thread: RealTime Loop
//
// End Function Specification
void amec_update_centaur_temp_sensors(void)
{
    uint32_t k;
    uint16_t l_hot_centaur = 0;
    uint16_t l_hot_dimm = 0;

    // -----------------------------------------------------------
    // Find hottest Centaur and DIMM temp from all centaurs for this Proc chip
    // -----------------------------------------------------------
    for(k=0; k < MAX_NUM_CENTAURS; k++)
    {
        if(CENTAUR_PRESENT(k))
        {
            // check if this is the hottest Centaur
            if(g_amec->proc[0].memctl[k].centaur.centaur_hottest.cur_temp > l_hot_centaur)
            {
                l_hot_centaur = (uint16_t)g_amec->proc[0].memctl[k].centaur.centaur_hottest.cur_temp;
            }

            // check if this Centaur has the hottest DIMM
            if(g_amec->proc[0].memctl[k].centaur.tempdimmax.sample > l_hot_dimm)
            {
                l_hot_dimm = g_amec->proc[0].memctl[k].centaur.tempdimmax.sample;
            }
        }
    }

    sensor_update(&g_amec->proc[0].temp2mscent,l_hot_centaur);
    sensor_update(&g_amec->proc[0].tempdimmthrm,l_hot_dimm);
    AMEC_DBG("HotCentaur=[%d]  HotDimm=[%d]",l_hot_centaur, l_hot_dimm);
}

// Function Specification
//
// Name: amec_diff_adjust_for_overflow
//
// Description: Calculates the diff between 32-bit accumulator values
//              while accounting for overflow.
//
// End Function Specification
uint32_t amec_diff_adjust_for_overflow(uint32_t i_new_value, uint32_t i_old_value)
{
    uint32_t l_result = 0;
    uint64_t l_overflow = 0;

    if(i_new_value < i_old_value)
    {
        l_overflow = 0xFFFFFFFF + i_new_value;
        l_overflow = l_overflow - i_old_value;
        l_result = l_overflow;
    }
    else
    {
        l_result = i_new_value - i_old_value;
    }

    return l_result;
}

// Function Specification
//
// Name: amec_perfcount_getmc
//
// Description: Updates performance sensors that have data grabbed by the
// centaur.
//
// Thread: RealTime Loop
//
// End Function Specification
void amec_perfcount_getmc( CentaurMemData * i_sensor_cache,
                           uint8_t i_centaur)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    UINT32          tempu = 0;
    UINT32          templ = 0;
    UINT32          temp32new = 0;
    UINT32          temp32 = 0;
    UINT16          tempreg = 0;
    uint8_t         i_mc_id = 0;

    static uint32_t L_num_ticks[MAX_NUM_CENTAURS] = {0};

    #define NUM_TICKS_TO_DELAY_SENSOR_UPDATE 10

    #define AMECSENSOR_PORTPAIR_PTR(sensor_base,idx,idx2) \
        (&(g_amec->proc[0].memctl[idx].centaur.portpair[idx2].sensor_base))

    /*------------------------------------------------------------------------*/
    /*Code                                                                    */
    /*------------------------------------------------------------------------*/

    CentaurMemData * l_sensor_cache = i_sensor_cache;

    for(i_mc_id=0; i_mc_id<2; i_mc_id++)
    {
        if (i_mc_id == 0)
        {
            tempu = l_sensor_cache->scache.mba01_rd;
            templ = l_sensor_cache->scache.mba01_wr;
        }
        else
        {
            tempu = l_sensor_cache->scache.mba23_rd;
            templ = l_sensor_cache->scache.mba23_wr;
        }

        // ---------------------------------------------------------------------------
        // Interim Calculation: MWRMx (0.01 Mrps) Memory write requests per sec
        // ---------------------------------------------------------------------------

        // Extract write bandwidth
        temp32new = (templ);
        temp32 = g_amec->proc[0].memctl[i_centaur].centaur.portpair[i_mc_id].perf.wr_cnt_accum;
        temp32 = amec_diff_adjust_for_overflow(temp32new, temp32);
        g_amec->proc[0].memctl[i_centaur].centaur.portpair[i_mc_id].perf.wr_cnt_accum = temp32new; // Save latest accumulator away for next time

        // Read every 8 ms....to convert to 0.01 Mrps = ((8ms read * 125)/10000)
        tempreg = ((temp32*125)/10000);

        g_amec->proc[0].memctl[i_centaur].centaur.portpair[i_mc_id].perf.memwrite2ms = tempreg;

        // -------------------------------------------------------------------------
        // Interim Calculation: MRDMx (0.01 Mrps) Memory read requests per sec
        // -------------------------------------------------------------------------

        // Extract read bandwidth
        temp32new = (tempu);
        temp32 = g_amec->proc[0].memctl[i_centaur].centaur.portpair[i_mc_id].perf.rd_cnt_accum;
        temp32 = amec_diff_adjust_for_overflow(temp32new, temp32);
        g_amec->proc[0].memctl[i_centaur].centaur.portpair[i_mc_id].perf.rd_cnt_accum = temp32new; // Save latest accumulator away for next time

        // Read every 8 ms....to convert to 0.01 Mrps = ((8ms read * 125)/10000)
        tempreg = ((temp32*125)/10000);

        g_amec->proc[0].memctl[i_centaur].centaur.portpair[i_mc_id].perf.memread2ms = tempreg;
    }

    // ------------------------------------------------------------
    // Sensor: MRDMx (0.01 Mrps) Memory read requests per sec
    // ------------------------------------------------------------
    tempreg = g_amec->proc[0].memctl[i_centaur].centaur.portpair[0].perf.memread2ms;
    tempreg += g_amec->proc[0].memctl[i_centaur].centaur.portpair[1].perf.memread2ms;
    if(L_num_ticks[i_centaur] >= NUM_TICKS_TO_DELAY_SENSOR_UPDATE)
    {
        sensor_update( (&(g_amec->proc[0].memctl[i_centaur].mrd)), tempreg);
    }

    // -------------------------------------------------------------
    // Sensor: MWRMx (0.01 Mrps) Memory write requests per sec
    // -------------------------------------------------------------
    tempreg = g_amec->proc[0].memctl[i_centaur].centaur.portpair[0].perf.memwrite2ms;
    tempreg += g_amec->proc[0].memctl[i_centaur].centaur.portpair[1].perf.memwrite2ms;
    if(L_num_ticks[i_centaur] >= NUM_TICKS_TO_DELAY_SENSOR_UPDATE)
    {
        sensor_update( (&(g_amec->proc[0].memctl[i_centaur].mwr)), tempreg);
    }

    if(L_num_ticks[i_centaur] < NUM_TICKS_TO_DELAY_SENSOR_UPDATE)
    {
        L_num_ticks[i_centaur]++;
    }

    return;
}

/*----------------------------------------------------------------------------*/ /* End */
/*----------------------------------------------------------------------------*/
