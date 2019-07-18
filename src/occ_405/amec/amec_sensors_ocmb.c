/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/amec/amec_sensors_ocmb.c $                        */
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
#include "amec_sensors_ocmb.h"
#include "ocmb_mem_data.h"

/******************************************************************************/
/* Globals                                                                    */
/******************************************************************************/
extern dimm_sensor_flags_t G_dimm_overtemp_bitmap;
extern dimm_sensor_flags_t G_dimm_temp_updated_bitmap;
extern uint8_t             G_cent_overtemp_bitmap;
extern uint8_t             G_cent_temp_updated_bitmap;
extern uint8_t      G_centaur_needs_recovery;
extern uint64_t G_inject_dimm;
extern uint32_t G_inject_dimm_trace[MAX_NUM_OCMBS][NUM_DIMMS_PER_OCMB];

uint32_t amec_diff_adjust_for_overflow(uint32_t i_new_value, uint32_t i_old_value);

/******************************************************************************/
/* Forward Declarations                                                       */
/******************************************************************************/
void amec_update_ocmb_dimm_dts_sensors(OcmbMemData * i_sensor_cache, uint8_t i_membuf);
void amec_update_ocmb_dts_sensors(OcmbMemData * i_sensor_cache, uint8_t i_membuf);
void amec_perfcount_ocmb_getmc( OcmbMemData * i_sensor_cache, uint8_t i_membuf);

/******************************************************************************/
/* Code                                                                       */
/******************************************************************************/

// Function Specification
//
// Name: amec_update_ocmb_sensors
//
// Description: Updates sensors that have data grabbed by the fast core data
// task.
//
// Thread: RealTime Loop
//
// End Function Specification
void amec_update_ocmb_sensors(uint8_t i_membuf)
{
    if(CENTAUR_PRESENT(i_membuf))
    {
        OcmbMemData * l_sensor_cache =
            (OcmbMemData *)cent_get_centaur_data_ptr(i_membuf);
        if(CENTAUR_UPDATED(i_membuf))
        {
            amec_update_ocmb_dimm_dts_sensors(l_sensor_cache, i_membuf);
            amec_update_ocmb_dts_sensors(l_sensor_cache, i_membuf);
            amec_perfcount_ocmb_getmc(l_sensor_cache, i_membuf);
        }
        CLEAR_CENTAUR_UPDATED(i_membuf);
    }
}

// Function Specification
//
// Name: amec_update_ocmb_dimm_dts_sensors
//
// Description: Updates sensors that have data from the membuf sensor cache
//
// Thread: RealTime Loop
//
// Note: The ocmb cache data uses the same space as the Centaur
//       membuf cache data, so some of the centaur attributes apply to
//       Ocmb data as well as Centaur.
//
// End Function Specification
void amec_update_ocmb_dimm_dts_sensors(OcmbMemData * i_sensor_cache, uint8_t i_membuf)
{
#define MIN_VALID_DIMM_TEMP 1
#define MAX_VALID_DIMM_TEMP 125 //according to Mike Pardiek 04/23/2019
#define MAX_MEM_TEMP_CHANGE 2

    uint32_t k, l_hottest_dimm_temp;
    uint16_t l_dts[NUM_DIMMS_PER_OCMB] = {0};
    uint32_t l_hottest_dimm_loc = NUM_DIMMS_PER_OCMB;
    int32_t  l_dimm_temp, l_prev_temp;
    static uint8_t L_ran_once[MAX_NUM_OCMBS] = {FALSE};
    static bool    L_ot_traced[MAX_NUM_OCMBS][NUM_DIMMS_PER_OCMB] = {{false}};

    // Harvest thermal data for all dimms
    for(k=0; k < NUM_DIMMS_PER_OCMB; k++)
    {
        if(!CENTAUR_SENSOR_ENABLED(i_membuf, k))
        {
            continue;
        }

#ifdef __INJECT_DIMM__
        if (g_amec->proc[0].memctl[i_membuf].centaur.dimm_temps[k].temp_sid) // DIMM has sensor ID
        {
            if ((G_inject_dimm & ((uint64_t)1 << ((i_membuf * 8) + k))) == 0)
            {
                if (G_inject_dimm_trace[i_membuf][k] != 0)
                {
                    TRAC_INFO("amec_update_ocmb_dimm_dts_sensors: stopping injection of errors for DIMM%04X", (i_membuf<<8)|k);
                    G_inject_dimm_trace[i_membuf][k] = 0;
                }
            }
            else
            {
                if (G_inject_dimm_trace[i_membuf][k] == 0)
                {
                    TRAC_INFO("amec_update_ocmb_dimm_dts_sensors: injecting errors for DIMM%04X", (i_membuf<<8)|k);
                    G_inject_dimm_trace[i_membuf][k] = 1;
                }
                continue; // Skip this DIMM
            }
        }
#endif

        fru_temp_t* l_fru = &g_amec->proc[0].memctl[i_membuf].centaur.dimm_temps[k];

        l_dimm_temp = i_sensor_cache->memdts[k];
        l_prev_temp = l_fru->cur_temp;
        if(!l_prev_temp)
        {
            l_prev_temp = l_dimm_temp;
        }

        //Check DTS status bits.  VALID NEW if valid and !err
        if((k == 0 &&
            i_sensor_cache->status.fields.memdts0_valid &&
            (!i_sensor_cache->status.fields.memdts0_err)) ||
           (k == 1 &&
            i_sensor_cache->status.fields.memdts1_valid &&
            (!i_sensor_cache->status.fields.memdts1_err)))
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
                        TRAC_INFO("dimm temp rose faster than reasonable: membuf[%d] dimm[%d] prev[%d] cur[%d]",
                                  i_membuf, k, l_prev_temp, l_dimm_temp);
                        l_fru->flags |= FRU_TEMP_FAST_CHANGE;
                    }
                }
                else if (l_dimm_temp < (l_prev_temp - MAX_MEM_TEMP_CHANGE))
                {
                    l_dts[k] = l_prev_temp - MAX_MEM_TEMP_CHANGE;
                    if(!l_fru->flags)
                    {
                        TRAC_INFO("dimm temp fell faster than reasonable: membuf[%d] dimm[%d] prev[%d] cur[%d]",
                                  i_membuf, k, l_prev_temp, l_dimm_temp);
                        l_fru->flags |= FRU_TEMP_FAST_CHANGE;
                    }
                }
                else //reasonable amount of change occurred
                {
                    l_dts[k] = l_dimm_temp;
                    l_fru->flags &= ~FRU_TEMP_FAST_CHANGE;
                }

                //Notify thermal thread that temperature has been updated
                G_dimm_temp_updated_bitmap.bytes[i_membuf] |= DIMM_SENSOR0 >> k;

                //clear error flags
                l_fru->flags &= FRU_TEMP_FAST_CHANGE;
            }
        }
        else //status was VALID_OLD or ERROR
        {
            //convert status number to a flag
            uint8_t l_status_flag = 0;
            if((k == 0 && i_sensor_cache->status.fields.memdts0_err) ||
               (k == 1 && i_sensor_cache->status.fields.memdts1_err))
            {
                l_status_flag = FRU_SENSOR_STATUS_ERROR;
            }
            else
            {
                l_status_flag = FRU_SENSOR_STATUS_VALID_OLD;
            }

            if(L_ran_once[i_membuf])
            {
                //Trace the error if we haven't traced it already for this sensor
                if((l_status_flag != FRU_SENSOR_STATUS_VALID_OLD) &&
                   !(l_status_flag & l_fru->flags))
                {
                    TRAC_ERR("Membuf %d dimm sensor%d reported an error.", i_membuf, k);
                }

                l_fru->flags |= l_status_flag;
            }

            //use last temperature
            l_dts[k] = l_prev_temp;

            //request recovery (disable and re-enable sensor cache collection)
            if(i_sensor_cache->status.fields.memdts0_err ||
               i_sensor_cache->status.fields.memdts1_err)
            {
                G_centaur_needs_recovery |= CENTAUR0_PRESENT_MASK >> i_membuf;
            }
        }

        //Check if at or above the error temperature
        if(l_dts[k] >= g_amec->thermaldimm.ot_error)
        {
            //Set a bit so that this dimm can be called out by the thermal thread
            G_dimm_overtemp_bitmap.bytes[i_membuf] |= (DIMM_SENSOR0 >> k);
            // trace first time OT per DIMM
            if( !L_ot_traced[i_membuf][k] )
            {
               TRAC_ERR("amec_update_ocmb_dimm_dts_sensors: Mem Buf[%d] DIMM[%d] reached error temp[%d]. current[%d]",
                        i_membuf,
                        k,
                        g_amec->thermaldimm.ot_error,
                        l_dts[k]);
               L_ot_traced[i_membuf][k] = true;
            }
        }
    }

    // Find hottest temperature from all DIMMs for this centaur
    for(l_hottest_dimm_temp = 0, k = 0; k < NUM_DIMMS_PER_OCMB; k++)
    {
        if(l_dts[k] > l_hottest_dimm_temp)
        {
            l_hottest_dimm_temp = l_dts[k];
            l_hottest_dimm_loc = k;
        }
        g_amec->proc[0].memctl[i_membuf].centaur.dimm_temps[k].cur_temp =  l_dts[k];
    }

    amec_centaur_t* l_centaur_ptr = &g_amec->proc[0].memctl[i_membuf].centaur;

    //only update location if hottest dimm temp is greater than previous maximum
    if(l_hottest_dimm_temp > l_centaur_ptr->tempdimmax.sample_max)
    {
        sensor_update(&l_centaur_ptr->locdimmax, l_hottest_dimm_loc);
    }

    //update the max dimm temperature sensor for this centaur
    sensor_update(&l_centaur_ptr->tempdimmax, l_hottest_dimm_temp);

    L_ran_once[i_membuf] = TRUE;
    AMEC_DBG("Centaur[%d]: HotDimm=%d\n",i_membuf,l_hottest_dimm_temp);
}

// Function Specification
//
// Name: amec_update_ocmb_dts_sensors
//
// Description: Updates sensors that have data grabbed by the fast core data
//
// Thread: RealTime Loop
//
// End Function Specification
void amec_update_ocmb_dts_sensors(OcmbMemData * i_sensor_cache, uint8_t i_membuf)
{
#define MIN_VALID_MEMBUF_TEMP 1
#define MAX_VALID_MEMBUF_TEMP 125 //according to Mike Pardiek

    uint16_t l_dts;
    uint16_t l_sens_temp;
    int32_t  l_prev_temp;
    static uint8_t L_ran_once[MAX_NUM_OCMBS] = {FALSE};

    l_sens_temp = i_sensor_cache->ubdts0;

    fru_temp_t* l_fru = &g_amec->proc[0].memctl[i_membuf].centaur.centaur_hottest;

    l_prev_temp = l_fru->cur_temp;
    if(!l_prev_temp)
    {
        l_prev_temp = l_sens_temp;
    }

    //Check DTS status bits
    if(i_sensor_cache->status.fields.ubdts0_valid &&
       (!i_sensor_cache->status.fields.ubdts0_err))
    {
        //make sure temperature is within a 'reasonable' range.
        if(l_sens_temp < MIN_VALID_MEMBUF_TEMP ||
           l_sens_temp > MAX_VALID_MEMBUF_TEMP)
        {
            //set a flag so that if we end up logging an error we have something to debug why
            l_fru->flags |= FRU_TEMP_OUT_OF_RANGE;
            l_dts = l_prev_temp;
        }
        else
        {
            //don't allow temp to change more than is reasonable for 2ms
            if(l_sens_temp > (l_prev_temp + MAX_MEM_TEMP_CHANGE))
            {
                l_dts = l_prev_temp + MAX_MEM_TEMP_CHANGE;
                if(!l_fru->flags)
                {
                    TRAC_INFO("membuf temp rose faster than reasonable: membuf[%d] prev[%d] cur[%d]",
                              i_membuf, l_prev_temp, l_sens_temp);
                    l_fru->flags |= FRU_TEMP_FAST_CHANGE;
                }
            }
            else if (l_sens_temp < (l_prev_temp - MAX_MEM_TEMP_CHANGE))
            {
                l_dts = l_prev_temp - MAX_MEM_TEMP_CHANGE;
                if(!l_fru->flags)
                {
                    TRAC_INFO("membuf temp fell faster than reasonable: cent[%d] prev[%d] cur[%d]",
                                  i_membuf, l_prev_temp, l_sens_temp);
                    l_fru->flags |= FRU_TEMP_FAST_CHANGE;
                }
            }
            else //reasonable amount of change occurred
            {
                l_dts = l_sens_temp;
                l_fru->flags &= ~FRU_TEMP_FAST_CHANGE;
            }

            //Notify thermal thread that temperature has been updated
            G_cent_temp_updated_bitmap |= CENTAUR0_PRESENT_MASK >> i_membuf;

            //clear error flags
            l_fru->flags &= FRU_TEMP_FAST_CHANGE;
        }
    }
    else //status was INVALID
    {
        if(L_ran_once[i_membuf])
        {
            //Trace the error if we haven't traced it already for this sensor
            if(!(l_fru->flags & FRU_SENSOR_STATUS_INVALID) &&
               i_sensor_cache->status.fields.ubdts0_err)
            {
                TRAC_ERR("Membuf %d temp sensor error.", i_membuf);
            }

            l_fru->flags |= FRU_SENSOR_STATUS_INVALID;
        }

        //use last temperature
        l_dts = l_prev_temp;
    }

    L_ran_once[i_membuf] = TRUE;

    //Check if at or above the error temperature
    if(l_dts >= g_amec->thermalcent.ot_error)
    {
        //Set a bit so that this dimm can be called out by the thermal thread
        G_cent_overtemp_bitmap |= (CENTAUR0_PRESENT_MASK >> i_membuf);
    }

    // Update Interim Data - later this will get picked up to form centaur sensor
    g_amec->proc[0].memctl[i_membuf].centaur.centaur_hottest.cur_temp = l_dts;

    AMEC_DBG("Membuf[%d]: HotMembuf=%d\n",i_membuf,l_dts);
}

// Function Specification
//
// Name: amec_update_ocmb_temp_sensors
//
// Description: Updates thermal sensors that have data grabbed by the centaur.
//
// Thread: RealTime Loop
//
// End Function Specification
void amec_update_ocmb_temp_sensors(void)
{
    uint32_t k;
    uint32_t l_hot_dimm = 0;
    uint32_t l_hot_mb = 0;

    // -----------------------------------------------------------
    // Find hottest temperature from all membufs for this Proc chip
    // Find hottest temperature from all DIMMs for this Proc chip
    // -----------------------------------------------------------
    for(k=0; k < MAX_NUM_OCMBS; k++)
    {
        if(g_amec->proc[0].memctl[k].centaur.centaur_hottest.cur_temp > l_hot_mb)
        {
            l_hot_mb = g_amec->proc[0].memctl[k].centaur.centaur_hottest.cur_temp;
        }
        if(g_amec->proc[0].memctl[k].centaur.tempdimmax.sample > l_hot_dimm)
        {
            l_hot_dimm = g_amec->proc[0].memctl[k].centaur.tempdimmax.sample;
        }
    }
    sensor_update(&g_amec->proc[0].temp2mscent,l_hot_mb);
    AMEC_DBG("HotMembuf=%d\n",l_hot_mb);

    sensor_update(&g_amec->proc[0].tempdimmthrm,l_hot_dimm);
    AMEC_DBG("HotDimm=%d\n",l_hot_dimm);

}


// Function Specification
//
// Name: amec_perfcount_ocmb_getmc
//
// Description: Updates performance sensors that have data grabbed by the
// centaur.
//
// Thread: RealTime Loop
//
// End Function Specification

void amec_perfcount_ocmb_getmc( OcmbMemData * i_sensor_cache,
                           uint8_t i_membuf)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    uint32_t                    tempu       = 0;
    uint32_t                    templ       = 0;
    uint32_t                    temp32new   = 0;
    uint32_t                    temp32      = 0;
    uint32_t                    tempreg     = 0;

    #define AMECSENSOR_PORTPAIR_PTR(sensor_base,idx,idx2) \
        (&(g_amec->proc[0].memctl[idx].centaur.portpair[idx2].sensor_base))

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    OcmbMemData * l_sensor_cache = i_sensor_cache;

    tempu = l_sensor_cache->mba_rd;
    templ = l_sensor_cache->mba_wr;

    // ---------------------------------------------------------------------------
    //  Interim Calculation:  MWRMx (0.01 Mrps) Memory write requests per sec
    // ---------------------------------------------------------------------------

    // Extract write bandwidth
    temp32new = (templ);

    temp32 = g_amec->proc[0].memctl[i_membuf].centaur.portpair[0].perf.wr_cnt_accum;
    temp32 = amec_diff_adjust_for_overflow(temp32new, temp32);
    g_amec->proc[0].memctl[i_membuf].centaur.portpair[0].perf.wr_cnt_accum = temp32new;  // Save latest accumulator away for next time

    // Read every 8 ms....to convert to 0.01 Mrps = ((8ms read * 125)/10000)
    tempreg = ((temp32*125)/10000);

    g_amec->proc[0].memctl[i_membuf].centaur.portpair[0].perf.memwrite2ms = tempreg;

    // -------------------------------------------------------------------------
    // Interim Calculation:  MRDMx (0.01 Mrps) Memory read requests per sec
    // -------------------------------------------------------------------------

    // Extract read bandwidth
    temp32new = (tempu);

    temp32 = g_amec->proc[0].memctl[i_membuf].centaur.portpair[0].perf.rd_cnt_accum;
    temp32 = amec_diff_adjust_for_overflow(temp32new, temp32);
    g_amec->proc[0].memctl[i_membuf].centaur.portpair[0].perf.rd_cnt_accum = temp32new;  // Save latest accumulator away for next time

    // Read every 8 ms....to convert to 0.01 Mrps = ((8ms read * 125)/10000)
    tempreg = ((temp32*125)/10000);

    g_amec->proc[0].memctl[i_membuf].centaur.portpair[0].perf.memread2ms = tempreg;

    // Go after second MC performance counter (power ups and activations)
    tempu = l_sensor_cache->mba_act;
    templ = l_sensor_cache->mba_powerups;

    // ------------------------------------------------------------
    // Sensor:  MRDMx (0.01 Mrps) Memory read requests per sec
    // ------------------------------------------------------------
    tempreg = g_amec->proc[0].memctl[i_membuf].centaur.portpair[0].perf.memread2ms;
    tempreg += g_amec->proc[0].memctl[i_membuf].centaur.portpair[1].perf.memread2ms;
    sensor_update( (&(g_amec->proc[0].memctl[i_membuf].mrd)), tempreg);

    // -------------------------------------------------------------
    // Sensor:  MWRMx (0.01 Mrps) Memory write requests per sec
    // -------------------------------------------------------------
    tempreg = g_amec->proc[0].memctl[i_membuf].centaur.portpair[0].perf.memwrite2ms;
    tempreg += g_amec->proc[0].memctl[i_membuf].centaur.portpair[1].perf.memwrite2ms;
    sensor_update( (&(g_amec->proc[0].memctl[i_membuf].mwr)), tempreg);

    return;
}

/*----------------------------------------------------------------------------*/
/* End                                                                        */
/*----------------------------------------------------------------------------*/
