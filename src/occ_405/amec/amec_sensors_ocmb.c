/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/amec/amec_sensors_ocmb.c $                        */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2022                        */
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
#include "memory_data.h"
#include "amec_smh.h"
#include "amec_slave_smh.h"
#include <trac.h>
#include "amec_sys.h"
#include "sensor_enum.h"
#include "amec_service_codes.h"
#include "amec_sensors_ocmb.h"
#include "ocmb_mem_data.h"
#include "common.h"

/******************************************************************************/
/* Globals                                                                    */
/******************************************************************************/
dimm_sensor_flags_t G_dimm_overtemp_bitmap = {{0}};
dimm_sensor_flags_t G_dimm_temp_updated_bitmap = {{0}};
uint16_t            G_membuf_overtemp_bitmap = 0;
uint16_t            G_membuf_temp_updated_bitmap = 0;

extern uint8_t      G_membuf_needs_recovery;
extern uint64_t G_inject_dimm;
extern uint32_t G_inject_dimm_trace[MAX_NUM_OCMBS][MAX_NUM_DTS_PER_OCMB];
extern uint32_t G_num_ocmb_reads_per_1000s;
extern uint16_t G_allow_trace_flags;

uint32_t amec_diff_adjust_for_overflow(uint32_t i_new_value, uint32_t i_old_value);

// number of times OCMB cache line must be read before starting to update bandwidth sensors
// this is required to have a good starting accumulator
#define MIN_OCMB_READS_FOR_BW_SENSOR 5
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
    if(MEMBUF_PRESENT(i_membuf))
    {
        OcmbMemData * l_sensor_cache = get_membuf_data_ptr(i_membuf);
        if(MEMBUF_UPDATED(i_membuf))
        {
            amec_update_ocmb_dimm_dts_sensors(l_sensor_cache, i_membuf);
            amec_update_ocmb_dts_sensors(l_sensor_cache, i_membuf);
            amec_perfcount_ocmb_getmc(l_sensor_cache, i_membuf);
        }
        CLEAR_MEMBUF_UPDATED(i_membuf);
    }
}

// Function Specification
//
// Name: decode_ocmb_dimm_dts
//
// Description: Decode DTS readings from OCMB
//
// End Function Specification

int32_t decode_ocmb_dimm_dts(uint16_t i_reading)
{
    int32_t l_dimm_temp = 0;

    // Swap MSB and LSB
    uint16_t dts_reading = (i_reading >> 8) | (i_reading << 8);

    dts_reading &= 0x3fff;  // mask of TCRIT HIGH LOW bits
    if(dts_reading & 0x1000) // check sign bit
    {
        dts_reading |= 0xe000;  // Sign extend
    }
    l_dimm_temp = dts_reading;  // Temp in (1/16) deg C
    l_dimm_temp /= 16;
    if(dts_reading & 0x0008) // fraction - round up
        ++l_dimm_temp;

    return l_dimm_temp;
}

// Function Specification
//
// Name: amec_update_ocmb_dimm_dts_sensors
//
// Description: Updates sensors that have data from the membuf sensor cache
//
// Thread: RealTime Loop
//
// Note: The ocmb cache data uses the same space as the
//       membuf cache data, so some of the membuf attributes apply to
//       Ocmb data as well as membuf.
//
// End Function Specification
void amec_update_ocmb_dimm_dts_sensors(OcmbMemData * i_sensor_cache, uint8_t i_membuf)
{
// confirmed ok to use same values for all types (internal mc, dimm, external mc, pmic...)
#define MIN_VALID_DIMM_TEMP 1
#define MAX_VALID_DIMM_TEMP 125 //according to Mike Pardiek 04/23/2019
#define MAX_MEM_TEMP_CHANGE 4

    uint32_t k;
    uint16_t l_dts[MAX_NUM_DTS_PER_OCMB] = {0};
    uint8_t  l_max_dts_per_membuf = 0;
    int32_t  l_dimm_temp, l_prev_temp;
    static uint8_t L_ran_once[MAX_NUM_OCMBS] = {FALSE};

    amec_membuf_t* l_membuf_ptr = &g_amec->proc[0].memctl[i_membuf].membuf;

    if(G_sysConfigData.mem_type == MEM_TYPE_OCM_DDR5)
         l_max_dts_per_membuf = NUM_DTS_PER_OCMB_DDR5;
    else if(G_sysConfigData.mem_type == MEM_TYPE_OCM_DDR4)
         l_max_dts_per_membuf = NUM_DTS_PER_OCMB_DDR4;
    else // must be i2c, no "dimm" dts used from cache line
         l_max_dts_per_membuf = 0;

    // Harvest thermal data for all dimms
    for(k=0; k < l_max_dts_per_membuf; k++)
    {
        if(!MEMBUF_SENSOR_ENABLED(i_membuf, k))
        {
            continue;
        }

#ifdef __INJECT_DIMM__
        if (l_membuf_ptr->dimm_temps[k].temp_sid) // DIMM has sensor ID
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

        fru_temp_t* l_fru = &l_membuf_ptr->dimm_temps[k];

        // The dimm dts reading is mangled
        // see ekb/chips/ocmb/explorer/procedures/hwp/memory/lab/sdk/temp_sensor/exp_temperature_sensor_utils.H
        l_dimm_temp = decode_ocmb_dimm_dts(i_sensor_cache->memdts[k]);

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
                //don't allow temp to change more than is reasonable since last read
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
                G_dimm_temp_updated_bitmap.bytes[i_membuf] |= (DIMM_SENSOR0 >> k);

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
                G_membuf_needs_recovery |= MEMBUF0_PRESENT_MASK >> i_membuf;
            }
        }

        // overtemp will be checked in amec_update_ocmb_temp_sensors() based on dts FRU type
    }

    // Update the temperatures for this membuf
    for(k = 0; k < l_max_dts_per_membuf; k++)
    {
        l_membuf_ptr->dimm_temps[k].cur_temp =  l_dts[k];
    }

    L_ran_once[i_membuf] = TRUE;
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
    int32_t l_sens_temp;
    int32_t  l_prev_temp;
    static uint8_t L_ran_once[MAX_NUM_OCMBS] = {FALSE};

    // The ubdts0 is in 1/100 deg C.
    // Add 50 for rounding
    l_sens_temp = (50l + i_sensor_cache->ubdts0)/100l;

    amec_membuf_t* l_membuf_ptr = &g_amec->proc[0].memctl[i_membuf].membuf;
    fru_temp_t* l_fru = &l_membuf_ptr->membuf_hottest;

    if(l_fru->temp_fru_type == DATA_FRU_MEMBUF)
    {
        l_prev_temp = l_fru->cur_temp;
        if(!l_prev_temp)
        {
            l_prev_temp = l_sens_temp;
        }

        //Check DTS status bits
        if( (i_sensor_cache->status.fields.ubdts0_valid) &&
            (!i_sensor_cache->status.fields.ubdts0_err) )
        {
            //make sure temperature is within a 'reasonable' range.
            if( (l_sens_temp < MIN_VALID_MEMBUF_TEMP) ||
                (l_sens_temp > MAX_VALID_MEMBUF_TEMP) )
            {
                //set a flag so that if we end up logging an error we have something to debug why
                l_fru->flags |= FRU_TEMP_OUT_OF_RANGE;
                l_dts = l_prev_temp;
            }
            else
            {
                //don't allow temp to change more than is reasonable since last read
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
                G_membuf_temp_updated_bitmap |= (MEMBUF0_PRESENT_MASK >> i_membuf);

                //clear error flags
                l_fru->flags &= FRU_TEMP_FAST_CHANGE;
            }
        }
        else //status was INVALID
        {
            if(L_ran_once[i_membuf])
            {
                //Trace the error if we haven't traced it already for this sensor
                if( (!(l_fru->flags & FRU_SENSOR_STATUS_INVALID)) &&
                    (i_sensor_cache->status.fields.ubdts0_err) )
                {
                    TRAC_ERR("Membuf %d temp sensor error.", i_membuf);
                }

                l_fru->flags |= FRU_SENSOR_STATUS_INVALID;
            }

            //use last temperature
            l_dts = l_prev_temp;
        }

        L_ran_once[i_membuf] = TRUE;

        // Update Interim Data - later this will get picked up to form membuf sensor
        l_fru->cur_temp = l_dts;

        //Check if at or above the error temperature
        if(l_dts >= g_amec->thermalmembuf.ot_error)
        {
            //Set a bit so that this dimm can be called out by the thermal thread
            G_membuf_overtemp_bitmap |= (MEMBUF0_PRESENT_MASK >> i_membuf);
        }

        // Update Interim Data - later this will get picked up to form membuf sensor
        l_membuf_ptr->membuf_hottest.cur_temp = l_dts;

        // Update individual membuf temperature
        sensor_update(&l_membuf_ptr->tempmembuf, l_dts);

        AMEC_DBG("Membuf[%d]: HotMembuf=%d\n",i_membuf,l_dts);
    }
    else // internal sensor not being used
    {
        // make sure temperature is 0 indicating not present
        l_fru->cur_temp = 0;

        //Notify thermal thread that temperature has been updated so no timeout error is logged
        G_membuf_temp_updated_bitmap |= MEMBUF0_PRESENT_MASK >> i_membuf;

        //clear error flags
        l_fru->flags = 0;
    }
}

// Function Specification
//
// Name: amec_update_ocmb_temp_sensors
//
// Description: Updates thermal sensors that have data grabbed by the membuf.
//
// Thread: RealTime Loop
//
// End Function Specification
void amec_update_ocmb_temp_sensors(void)
{
    uint32_t k, l_dimm;
    uint32_t l_hot_dimm = 0;
    uint32_t l_hot_mb = 0;
    uint32_t l_hot_mb_dimm = 0;
    uint32_t l_hot_pmic = 0;
    uint32_t l_hot_ext_mb = 0;
    uint8_t  l_ot_error = 0;
    uint8_t  l_cur_temp = 0;
    uint8_t  l_fru_type = DATA_FRU_NOT_USED;
    uint8_t  l_max_dts_per_membuf = 0;
    static bool L_ot_traced[MAX_NUM_OCMBS][MAX_NUM_DTS_PER_OCMB] = {{false}};

    if(G_sysConfigData.mem_type == MEM_TYPE_OCM_DDR5)
         l_max_dts_per_membuf = NUM_DTS_PER_OCMB_DDR5;
    else if(G_sysConfigData.mem_type == MEM_TYPE_OCM_DDR4)
         l_max_dts_per_membuf = NUM_DTS_PER_OCMB_DDR4;
    else // must be i2c, no "dimm" dts used from cache line
         l_max_dts_per_membuf = 0;

    for(k=0; k < MAX_NUM_OCMBS; k++)
    {
        // Find hottest temperature from all internal membufs for this Proc chip
        if(g_amec->proc[0].memctl[k].membuf.membuf_hottest.cur_temp > l_hot_mb)
        {
            l_hot_mb = g_amec->proc[0].memctl[k].membuf.membuf_hottest.cur_temp;
        }

        // process each of the thermal sensors (stored as "dimm" temps)
        // based on what type they are for and finding the hottest for each type
        for(l_dimm=0; l_dimm < l_max_dts_per_membuf; l_dimm++)
        {
           l_fru_type = g_amec->proc[0].memctl[k].membuf.dimm_temps[l_dimm].temp_fru_type;
           l_cur_temp = g_amec->proc[0].memctl[k].membuf.dimm_temps[l_dimm].cur_temp;

           switch(l_fru_type)
           {
               case DATA_FRU_DIMM:
                  l_ot_error = g_amec->thermaldimm.ot_error;
                  if(l_cur_temp > l_hot_dimm)
                  {
                     l_hot_dimm = l_cur_temp;
                  }
                  break;

               case DATA_FRU_MEMCTRL_DRAM:
                  l_ot_error = g_amec->thermalmcdimm.ot_error;
                  if(l_cur_temp > l_hot_mb_dimm)
                  {
                     l_hot_mb_dimm = l_cur_temp;
                  }
                  break;

               case DATA_FRU_PMIC:
                  l_ot_error = g_amec->thermalpmic.ot_error;
                  if(l_cur_temp > l_hot_pmic)
                  {
                     l_hot_pmic = l_cur_temp;
                  }
                  break;

               case DATA_FRU_MEMCTRL_EXT:
                  l_ot_error = g_amec->thermalmcext.ot_error;
                  if(l_cur_temp > l_hot_ext_mb)
                  {
                     l_hot_ext_mb = l_cur_temp;
                  }
                  break;

               case DATA_FRU_NOT_USED:
               default:
                  // ignore reading
                  l_ot_error = 0;
                  break;
           } // end switch fru type

           // check if this "DIMM" sensor is over its error temperature
           if( l_ot_error && (l_cur_temp >= l_ot_error) )
           {
               //Set a bit so that this sensor can be called out by the thermal thread
               G_dimm_overtemp_bitmap.bytes[k] |= (DIMM_SENSOR0 >> l_dimm);
               // trace first time OT per DIMM DTS sensor
               if( !L_ot_traced[k][l_dimm] )
               {
                  TRAC_ERR("amec_update_ocmb_temp_sensors: OCMB[%d] DTS[%d] type[0x%02X] reached error temp[%d]. current[%d]",
                           k,
                           l_dimm,
                           l_fru_type,
                           l_ot_error,
                           l_cur_temp);
                  L_ot_traced[k][l_dimm] = true;
               }
           }
        } // end for each "dimm" thermal sensor
    } // end for each OCMB

    sensor_update(&g_amec->proc[0].tempdimmthrm,l_hot_dimm);
    AMEC_DBG("HotMembuf=%d, HotDimm=%d\n", l_hot_mb, l_hot_dimm);

    sensor_update(&g_amec->proc[0].tempmcdimmthrm,l_hot_mb_dimm);
    AMEC_DBG("HotMCDimm=%d\n",l_hot_mb_dimm);

    sensor_update(&g_amec->proc[0].temppmicthrm,l_hot_pmic);
    AMEC_DBG("HotPmic=%d\n",l_hot_pmic);

    sensor_update(&g_amec->proc[0].tempmcextthrm,l_hot_ext_mb);
    AMEC_DBG("HotExternalMembuf=%d\n",l_hot_ext_mb);
}

// Function Specification
//
// Name: amec_perfcount_ocmb_getmc
//
// Description: Updates performance sensors that have data grabbed by the
// membuf.
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
    uint32_t                    l_cache_read  = 0;
    uint32_t                    l_cache_write = 0;
    uint32_t                    l_prev_read   = 0;
    uint32_t                    l_prev_write  = 0;
    uint32_t                    temp32        = 0;
    uint64_t                    temp64        = 0;
    uint32_t                    tempreg       = 0;
    uint32_t                    l_num_ticks   = 0;
    static uint8_t              L_accumulator_reads[MAX_NUM_OCMBS] = {0};
    static uint32_t             L_tick[MAX_NUM_OCMBS] = {0};

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    OcmbMemData * l_sensor_cache = i_sensor_cache;

    l_num_ticks = amec_diff_adjust_for_overflow(CURRENT_TICK, L_tick[i_membuf]);
    L_tick[i_membuf] = CURRENT_TICK;

    // ---------------------------------------------------------------------------
    //  MWR/MRD sensors = [ ((difference in accumulator) * (1000/read time in ms)) / 10000 ]
    //  For better precision with read time (no floating point) for (1000/read time in ms)
    //  we use G_num_ocmb_reads_per_1000s calculated in data_store_mem_cfg()
    //  MWR/MRD sensors = [ ((difference in accumulator) * G_num_ocmb_reads_per_1000s) / 10000000 ]
    // ---------------------------------------------------------------------------

    // -------------------------------------------------------------------------
    // MWRMx (0.0001 Memory write requests per sec)
    // -------------------------------------------------------------------------

    l_cache_write = l_sensor_cache->mba_wr;
    l_prev_write = g_amec->proc[0].memctl[i_membuf].membuf.portpair[0].perf.wr_cnt_accum;
    temp32 = amec_diff_adjust_for_overflow(l_cache_write, l_prev_write);

    // Save latest accumulator away for next time
    g_amec->proc[0].memctl[i_membuf].membuf.portpair[0].perf.wr_cnt_accum = l_cache_write;

    temp64 = (uint64_t)((uint64_t)temp32 * (uint64_t)G_num_ocmb_reads_per_1000s);
    if( (L_accumulator_reads[i_membuf] >= MIN_OCMB_READS_FOR_BW_SENSOR)&&
        (G_allow_trace_flags & ALLOW_MEM_TRACE) )
    {
        TRAC_INFO("amec_perfcount_ocmb_getmc[0]: temp64[0x%08X%08X]",
                   (uint32_t)(temp64>>32), (uint32_t)temp64);
    }
    temp64 /= (uint64_t)10000000;
    tempreg = (uint32_t)temp64;

    g_amec->proc[0].memctl[i_membuf].membuf.portpair[0].perf.memwrite2ms = tempreg;

    if( (L_accumulator_reads[i_membuf] >= MIN_OCMB_READS_FOR_BW_SENSOR)&&
        (G_allow_trace_flags & ALLOW_MEM_TRACE) )
    {
        TRAC_INFO("amec_perfcount_ocmb_getmc[1]: Updating sensor MWRM%d[%d] num 500us ticks[%d]",
                   i_membuf, g_amec->proc[0].memctl[i_membuf].membuf.portpair[0].perf.memwrite2ms, l_num_ticks);
        TRAC_INFO("amec_perfcount_ocmb_getmc[2]: Raw sensor cache Write data NEW[0x%08X] PREVIOUS[0x%08X]",
                   l_cache_write, l_prev_write);
        TRAC_INFO("amec_perfcount_ocmb_getmc[3]: Write Difference[0x%08X] temp64[0x%08X%08X]",
                   temp32, (uint32_t)(temp64>>32), (uint32_t)temp64);
    }

    // -------------------------------------------------------------------------
    // MRDMx (0.0001 Memory read requests per sec)
    // -------------------------------------------------------------------------

    l_cache_read = l_sensor_cache->mba_rd;
    l_prev_read = g_amec->proc[0].memctl[i_membuf].membuf.portpair[0].perf.rd_cnt_accum;
    temp32 = amec_diff_adjust_for_overflow(l_cache_read, l_prev_read);

    // Save latest accumulator away for next time
    g_amec->proc[0].memctl[i_membuf].membuf.portpair[0].perf.rd_cnt_accum = l_cache_read;

    temp64 = (uint64_t)((uint64_t)temp32 * (uint64_t)G_num_ocmb_reads_per_1000s);
    if( (L_accumulator_reads[i_membuf] >= MIN_OCMB_READS_FOR_BW_SENSOR)&&
        (G_allow_trace_flags & ALLOW_MEM_TRACE) )
    {
        TRAC_INFO("amec_perfcount_ocmb_getmc[0]: temp64[0x%08X%08X]",
                   (uint32_t)(temp64>>32), (uint32_t)temp64);
    }
    temp64 /= (uint64_t)10000000;
    tempreg = (uint32_t)temp64;

    g_amec->proc[0].memctl[i_membuf].membuf.portpair[0].perf.memread2ms = tempreg;

    // only update sensors if have done defined minimum cache line reads to have a good starting accumulator
    if(L_accumulator_reads[i_membuf] >= MIN_OCMB_READS_FOR_BW_SENSOR)
    {
        if(G_allow_trace_flags & ALLOW_MEM_TRACE)
        {
            TRAC_INFO("amec_perfcount_ocmb_getmc[4]: Updating MRD%d[%d]",
                       i_membuf, g_amec->proc[0].memctl[i_membuf].membuf.portpair[0].perf.memread2ms);
            TRAC_INFO("amec_perfcount_ocmb_getmc[5]: Raw sensor cache Read data NEW[0x%08X] PREVIOUS[0x%08X]",
                       l_cache_read, l_prev_read);
            TRAC_INFO("amec_perfcount_ocmb_getmc[6]: Read Difference[0x%08X] temp64[0x%08X%08X]",
                       temp32, (uint32_t)(temp64>>32), (uint32_t)temp64);
        }
        sensor_update( (&(g_amec->proc[0].memctl[i_membuf].mrd)),
                        g_amec->proc[0].memctl[i_membuf].membuf.portpair[0].perf.memread2ms);

        sensor_update( (&(g_amec->proc[0].memctl[i_membuf].mwr)),
                       g_amec->proc[0].memctl[i_membuf].membuf.portpair[0].perf.memwrite2ms);
    }
    else
    {
        L_accumulator_reads[i_membuf]++;

        if(G_allow_trace_flags & ALLOW_MEM_TRACE)
        {
            TRAC_INFO("amec_perfcount_ocmb_getmc: Skiping Membuf%d read[%d] write[%d] sensor updates",
                       i_membuf, g_amec->proc[0].memctl[i_membuf].membuf.portpair[0].perf.memread2ms,
                       g_amec->proc[0].memctl[i_membuf].membuf.portpair[0].perf.memwrite2ms);
            TRAC_INFO("amec_perfcount_ocmb_getmc: MRDM%d Raw sensor cache Read data NEW[%d] PREVIOUS[%d]",
                       i_membuf, l_cache_read, l_prev_read);
            TRAC_INFO("amec_perfcount_ocmb_getmc: MWRM%d Raw sensor cache Write data NEW[%d] PREVIOUS[%d]",
                       i_membuf, l_cache_write, l_prev_write);
        }
    }

    return;
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
