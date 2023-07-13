/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/amec/amec_sensors_ocmb.c $                        */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2023                        */
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

    uint16_t dts_reading = i_reading;

    // only difference between DDR4 and DDR5 is DDR4 bytes are swapped
    if(IS_OCM_DDR4_MEM_TYPE(G_sysConfigData.mem_type))
    {
       // Swap MSB and LSB
       dts_reading = (i_reading >> 8) | (i_reading << 8);
    }

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

    // defaults for DDR4
    bool    l_memdts0_valid = i_sensor_cache->status.fields.memdts0_valid;
    bool    l_memdts0_err = i_sensor_cache->status.fields.memdts0_err;
    bool    l_memdts1_valid = i_sensor_cache->status.fields.memdts1_valid;
    bool    l_memdts1_err = i_sensor_cache->status.fields.memdts1_err;
    bool    l_memdts2_valid = TRUE;
    bool    l_memdts2_err = FALSE;
    bool    l_memdts3_valid = TRUE;
    bool    l_memdts3_err = FALSE;

    OcmbMemDataDDR5* l_ddr5_sensor_cache = (OcmbMemDataDDR5*)i_sensor_cache;

    amec_membuf_t* l_membuf_ptr = &g_amec->proc[0].memctl[i_membuf].membuf;

    if(IS_OCM_DDR5_MEM_TYPE(G_sysConfigData.mem_type))
    {
         l_max_dts_per_membuf = NUM_DTS_PER_OCMB_DDR5;
         l_memdts0_valid = l_ddr5_sensor_cache->status.fields.memdts0_valid;
         l_memdts0_err = l_ddr5_sensor_cache->status.fields.memdts0_err;
         l_memdts1_valid = l_ddr5_sensor_cache->status.fields.memdts1_valid;
         l_memdts1_err = l_ddr5_sensor_cache->status.fields.memdts1_err;
         l_memdts2_valid = l_ddr5_sensor_cache->status.fields.memdts2_valid;
         l_memdts2_err = l_ddr5_sensor_cache->status.fields.memdts2_err;
         l_memdts3_valid = l_ddr5_sensor_cache->status.fields.memdts3_valid;
         l_memdts3_err = l_ddr5_sensor_cache->status.fields.memdts3_err;
    }
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
        // DDR4 and DDR5 decode differences handled in decode_ocmb_dimm_dts()
        if(IS_OCM_DDR5_MEM_TYPE(G_sysConfigData.mem_type))
            l_dimm_temp = decode_ocmb_dimm_dts(l_ddr5_sensor_cache->memdts[k]);
        else
            l_dimm_temp = decode_ocmb_dimm_dts(i_sensor_cache->memdts[k]);

        l_prev_temp = l_fru->cur_temp;
        if(!l_prev_temp)
        {
            l_prev_temp = l_dimm_temp;
        }

        //Check DTS status bits.  VALID NEW if valid and !err
        if((k == 0 && l_memdts0_valid && (!l_memdts0_err)) ||
           (k == 1 && l_memdts1_valid && (!l_memdts1_err)) ||
           (k == 2 && l_memdts2_valid && (!l_memdts2_err)) ||
           (k == 3 && l_memdts3_valid && (!l_memdts3_err)) )
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
            if((k == 0 && l_memdts0_err) ||
               (k == 1 && l_memdts1_err) ||
               (k == 2 && l_memdts2_err) ||
               (k == 3 && l_memdts3_err))
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
            if(l_memdts0_err || l_memdts1_err || l_memdts2_err || l_memdts3_err)
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
// Description: Updates mem buf temperature for given mem buf
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

    // defaults for DDR4
    bool    l_ubdts0_valid = i_sensor_cache->status.fields.ubdts0_valid;
    bool    l_ubdts0_err = i_sensor_cache->status.fields.ubdts0_err;

    OcmbMemDataDDR5* l_ddr5_sensor_cache = (OcmbMemDataDDR5*)i_sensor_cache;

    if(IS_OCM_DDR4_MEM_TYPE(G_sysConfigData.mem_type))
    {
        // The ubdts0 is in 1/100 deg C.
        // Add 50 for rounding
        l_sens_temp = (50l + i_sensor_cache->ubdts0)/100l;
    }
    else // DDR5
    {
        l_ubdts0_valid = l_ddr5_sensor_cache->status.fields.ubdts0_valid;
        l_ubdts0_err = l_ddr5_sensor_cache->status.fields.ubdts0_err;

        // Temp in 1/16 deg C
        l_sens_temp = l_ddr5_sensor_cache->ubdts0 >> 4;

        // round up if needed
        if((l_ddr5_sensor_cache->ubdts0 % 16) >= 8)
            l_sens_temp += 1;
    }

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
        if( (l_ubdts0_valid) && (!l_ubdts0_err) )
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
                    (l_ubdts0_err) )
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
    uint32_t l_hot_mb_dimm = 0;
    uint32_t l_hot_pmic = 0;
    uint32_t l_hot_ext_mb = 0;
    uint8_t  l_ot_error = 0;
    uint8_t  l_cur_temp = 0;
    uint8_t  l_fru_type = DATA_FRU_NOT_USED;
    uint8_t  l_max_dts_per_membuf = 0;
    static bool L_ot_traced[MAX_NUM_OCMBS][MAX_NUM_DTS_PER_OCMB] = {{false}};

    if(IS_OCM_DDR5_MEM_TYPE(G_sysConfigData.mem_type))
         l_max_dts_per_membuf = NUM_DTS_PER_OCMB_DDR5;
    else if(G_sysConfigData.mem_type == MEM_TYPE_OCM_DDR4)
         l_max_dts_per_membuf = NUM_DTS_PER_OCMB_DDR4;
    else // must be i2c, no "dimm" dts used from cache line
         l_max_dts_per_membuf = 0;

    for(k=0; k < MAX_NUM_OCMBS; k++)
    {
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

    // Don't update DIMM if I2C type, this will be updated in update_hottest_dimm()
    if(!(IS_I2C_MEM_TYPE(G_sysConfigData.mem_type)))
    {
        sensor_update(&g_amec->proc[0].tempdimmthrm,l_hot_dimm);
        AMEC_DBG("HotDimm=%d\n", l_hot_dimm);
    }

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
    uint16_t                    l_memutil = 0;
    uint16_t                    l_p1_memutil = 0;
    uint32_t                    l_cache_read_total  = 0;
    uint32_t                    l_cache_write_total = 0;
    uint32_t                    l_cache_p0_write = 0;
    uint32_t                    l_cache_p1_write = 0;
    uint32_t                    l_cache_p0_read = 0;
    uint32_t                    l_cache_p1_read = 0;
    uint32_t                    l_cache_frame_count = 0;
    uint32_t                    l_prev_read_total   = 0;
    uint32_t                    l_prev_p0_write  = 0;
    uint32_t                    l_prev_p1_write  = 0;
    uint32_t                    l_prev_p0_read  = 0;
    uint32_t                    l_prev_p1_read  = 0;
    uint32_t                    l_prev_write_total  = 0;
    uint32_t                    l_prev_frame_count = 0;
    uint32_t                    temp32        = 0;
    uint32_t                    tempreg       = 0;
    uint32_t                    l_num_ticks   = 0;
    uint32_t                    l_read_diff   = 0;
    uint32_t                    l_write_diff   = 0;
    uint32_t                    l_frame_count_diff   = 0;
    uint32_t                    l_num_ocmb_reads_per_1000s = G_num_ocmb_reads_per_1000s;
    uint32_t                    l_time_us = 0;
    uint64_t                    l_time_over_freq = 0;
    uint64_t                    l_sample_time_p001ns  = 0;
    uint64_t                    temp64        = 0;
    static uint8_t              L_accumulator_reads[MAX_NUM_OCMBS] = {0};
    static uint32_t             L_tick[MAX_NUM_OCMBS] = {0};
    static bool                 L_traced_no_util_support = FALSE;
    static bool                 L_traced_0_time = FALSE;
    static uint8_t              L_trace_count = 4; // number of traces when ALOW_MEM_TRACE set


    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    // DDR4
    OcmbMemData * l_sensor_cache = i_sensor_cache;
    // DDR5
    OcmbMemDataDDR5* l_ddr5_sensor_cache = (OcmbMemDataDDR5*)i_sensor_cache;

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

    if(IS_OCM_DDR4_MEM_TYPE(G_sysConfigData.mem_type))
    {
       l_cache_p0_write = l_sensor_cache->mba_wr;
       l_cache_p1_write = 0; // no port 1
       l_prev_p0_write = g_amec->proc[0].memctl[i_membuf].membuf.portpair[0].perf.wr_cnt_accum;
       l_prev_p1_write = 0; // no port 1

       l_cache_frame_count = l_sensor_cache->frame_count;
       l_prev_frame_count = g_amec->proc[0].memctl[i_membuf].membuf.portpair[0].perf.frame_count;
       l_frame_count_diff = amec_diff_adjust_for_overflow(l_cache_frame_count, l_prev_frame_count);
       l_sample_time_p001ns = (uint64_t)((uint64_t)l_frame_count_diff * (uint64_t)g_amec->proc[0].memctl[i_membuf].membuf.portpair[0].addr_clock_p001ns);
       l_time_us = (uint32_t)(l_sample_time_p001ns / 1000000);
       if(l_time_us)
           l_num_ocmb_reads_per_1000s = 1000000000 / l_time_us;
       else if(L_traced_0_time == FALSE)
       {
           L_traced_0_time = TRUE;  // only trace once
           TRAC_ERR("amec_perfcount_ocmb_getmc: sample time is 0us! frame_diff[0x%08X] addr_clock_p001ns[%d]",
                     l_frame_count_diff,
                     g_amec->proc[0].memctl[i_membuf].membuf.portpair[0].addr_clock_p001ns);
       }
    }
    else // DDR5
    {
       l_cache_p0_write = l_ddr5_sensor_cache->side0_wr;
       l_cache_p1_write = l_ddr5_sensor_cache->side1_wr;
       l_prev_p0_write = g_amec->proc[0].memctl[i_membuf].membuf.portpair[0].perf.wr_cnt_accum;
       l_prev_p1_write = g_amec->proc[0].memctl[i_membuf].membuf.portpair[1].perf.wr_cnt_accum;
       l_sample_time_p001ns = l_num_ticks * 500 * 1000 * 1000;  // ticks *500 --> us *1000 --> ns * 1000 --> .001ns
    }
    if( (L_accumulator_reads[i_membuf] >= MIN_OCMB_READS_FOR_BW_SENSOR)&&
        ((G_allow_trace_flags & ALLOW_MEM_TRACE) && L_trace_count) && l_frame_count_diff)
    {
        TRAC_INFO("amec_perfcount_ocmb_getmc: START Cache Frame count NEW[0x%08X] PREVIOUS[0x%08X]",
                   l_cache_frame_count, l_prev_frame_count);
        TRAC_INFO("amec_perfcount_ocmb_getmc: frame_diff[0x%08X] read time[%dus] (%d reads/1000s)",
                   l_frame_count_diff, l_time_us, l_num_ocmb_reads_per_1000s);
    }


    l_cache_write_total = l_cache_p0_write + l_cache_p1_write;
    l_prev_write_total = l_prev_p0_write + l_prev_p1_write;
    temp32 = amec_diff_adjust_for_overflow(l_cache_write_total, l_prev_write_total);

    // Save latest accumulator away for next time
    g_amec->proc[0].memctl[i_membuf].membuf.portpair[0].perf.wr_cnt_accum = l_cache_p0_write;
    g_amec->proc[0].memctl[i_membuf].membuf.portpair[1].perf.wr_cnt_accum = l_cache_p1_write;
    g_amec->proc[0].memctl[i_membuf].membuf.portpair[0].perf.frame_count = l_cache_frame_count;

    temp64 = (uint64_t)((uint64_t)temp32 * (uint64_t)l_num_ocmb_reads_per_1000s);
    if( (L_accumulator_reads[i_membuf] >= MIN_OCMB_READS_FOR_BW_SENSOR)&&
        ((G_allow_trace_flags & ALLOW_MEM_TRACE) && L_trace_count) )
    {
        TRAC_INFO("amec_perfcount_ocmb_getmc[0]: temp64[0x%08X%08X]",
                   (uint32_t)(temp64>>32), (uint32_t)temp64);
    }
    temp64 /= (uint64_t)10000000;
    tempreg = (uint32_t)temp64;

    g_amec->proc[0].memctl[i_membuf].membuf.portpair[0].perf.memwrite2ms = tempreg;

    if( (L_accumulator_reads[i_membuf] >= MIN_OCMB_READS_FOR_BW_SENSOR)&&
        ((G_allow_trace_flags & ALLOW_MEM_TRACE) && L_trace_count) )
    {
        TRAC_INFO("amec_perfcount_ocmb_getmc[1]: Updating sensor MWRM%d[%d] num 500us ticks[%d]",
                   i_membuf, g_amec->proc[0].memctl[i_membuf].membuf.portpair[0].perf.memwrite2ms, l_num_ticks);
        TRAC_INFO("amec_perfcount_ocmb_getmc[2]: Raw sensor cache Write data NEW[0x%08X] PREVIOUS[0x%08X]",
                   l_cache_write_total, l_prev_write_total);
        TRAC_INFO("amec_perfcount_ocmb_getmc[3]: Write Difference[0x%08X] temp64[0x%08X%08X]",
                   temp32, (uint32_t)(temp64>>32), (uint32_t)temp64);
    }

    // -------------------------------------------------------------------------
    // MRDMx (0.0001 Memory read requests per sec)
    // -------------------------------------------------------------------------

    if(IS_OCM_DDR4_MEM_TYPE(G_sysConfigData.mem_type))
    {
       l_cache_p0_read = l_sensor_cache->mba_rd;
       l_cache_p1_read = 0; // no port 1
       l_prev_p0_read = g_amec->proc[0].memctl[i_membuf].membuf.portpair[0].perf.rd_cnt_accum;
       l_prev_p1_read = 0; // no port 1
    }
    else // DDR5
    {
       l_cache_p0_read = l_ddr5_sensor_cache->side0_rd;
       l_cache_p1_read = l_ddr5_sensor_cache->side1_rd;
       l_prev_p0_read = g_amec->proc[0].memctl[i_membuf].membuf.portpair[0].perf.rd_cnt_accum;
       l_prev_p1_read = g_amec->proc[0].memctl[i_membuf].membuf.portpair[1].perf.rd_cnt_accum;
    }

    l_cache_read_total = l_cache_p0_read + l_cache_p1_read;
    l_prev_read_total = l_prev_p0_read + l_prev_p1_read;
    temp32 = amec_diff_adjust_for_overflow(l_cache_read_total, l_prev_read_total);

    // Save latest accumulator away for next time
    g_amec->proc[0].memctl[i_membuf].membuf.portpair[0].perf.rd_cnt_accum = l_cache_p0_read;
    g_amec->proc[0].memctl[i_membuf].membuf.portpair[1].perf.rd_cnt_accum = l_cache_p1_read;

    temp64 = (uint64_t)((uint64_t)temp32 * (uint64_t)l_num_ocmb_reads_per_1000s);
    if( (L_accumulator_reads[i_membuf] >= MIN_OCMB_READS_FOR_BW_SENSOR)&&
        ((G_allow_trace_flags & ALLOW_MEM_TRACE) && L_trace_count) )
    {
        TRAC_INFO("amec_perfcount_ocmb_getmc[0]: temp64[0x%08X%08X]",
                   (uint32_t)(temp64>>32), (uint32_t)temp64);
    }
    temp64 /= (uint64_t)10000000;
    tempreg = (uint32_t)temp64;

    g_amec->proc[0].memctl[i_membuf].membuf.portpair[0].perf.memread2ms = tempreg;

    // ---------------------------------------------------------------------------
    //  MEMUTILMx / MEMUTILP1Mx sensors (memory utilization 0.01% for ports 0 and 1) =
    //    [ ((diff in read accumulator) + (diff in write accumulator)) ] / (sample time in ns / ocmb clock ns) * burst_length
    //    ocmb clock ns = [1 / (ocmb_freq_Mbps / 2) * 1000]
    //    for better precision sample time and ocmb clock are in 0.001ns unit
    // ---------------------------------------------------------------------------
    // MEMUTIL sensors are only supported if we received memory freq and burst length which both must be non-zero
    if( (g_amec->proc[0].memctl[i_membuf].membuf.portpair[0].addr_clock_p001ns == 0) ||
        (g_amec->proc[0].memctl[i_membuf].membuf.portpair[0].burst_length == 0) ||
        ( IS_OCM_DDR5_MEM_TYPE(G_sysConfigData.mem_type) &&
          ( (g_amec->proc[0].memctl[i_membuf].membuf.portpair[1].addr_clock_p001ns == 0) ||
            (g_amec->proc[0].memctl[i_membuf].membuf.portpair[1].burst_length == 0)) ) )
    {
       l_memutil = 0;
       l_p1_memutil = 0;
       if(L_traced_no_util_support == FALSE)
       {
           L_traced_no_util_support = TRUE; // only trace once
           TRAC_INFO("amec_perfcount_ocmb_getmc: Missing data.  No support for OCMB[%d] utilization. data_rate_Mbps[%d] burst length[%d]",
                       i_membuf, g_amec->proc[0].memctl[i_membuf].membuf.portpair[0].data_rate_Mbps,
                       g_amec->proc[0].memctl[i_membuf].membuf.portpair[0].burst_length);
           if(IS_OCM_DDR5_MEM_TYPE(G_sysConfigData.mem_type))
           {
              TRAC_INFO("amec_perfcount_ocmb_getmc: OCMB port1 data_rate_Mbps[%d] port1 burst length[%d]",
                          g_amec->proc[0].memctl[i_membuf].membuf.portpair[1].data_rate_Mbps,
                          g_amec->proc[0].memctl[i_membuf].membuf.portpair[1].burst_length);
           }
       }
    }
    else
    {
       // calculate port 0 utilization
       l_time_over_freq = l_sample_time_p001ns / g_amec->proc[0].memctl[i_membuf].membuf.portpair[0].addr_clock_p001ns;
       l_read_diff = amec_diff_adjust_for_overflow(l_cache_p0_read, l_prev_p0_read);
       l_write_diff = amec_diff_adjust_for_overflow(l_cache_p0_write, l_prev_p0_write);
       temp64 = (l_read_diff + l_write_diff) * g_amec->proc[0].memctl[i_membuf].membuf.portpair[0].burst_length;
       temp64 *= 10000; // times 10000 to convert to 0.01% unit
       l_memutil = (uint16_t)(temp64 / l_time_over_freq);

       if((G_allow_trace_flags & ALLOW_MEM_TRACE) && L_trace_count)
       {
           TRAC_INFO("amec_perfcount_ocmb_getmc[UTILM%d]: l_sample_time_p001ns[0x%08X%08X]",
                      i_membuf, (uint32_t)(l_sample_time_p001ns>>32), (uint32_t)l_sample_time_p001ns);
           TRAC_INFO("amec_perfcount_ocmb_getmc[UTILM%d]: addr_clock_p001ns[%d]",
                      i_membuf,
                      g_amec->proc[0].memctl[i_membuf].membuf.portpair[0].addr_clock_p001ns);
           TRAC_INFO("amec_perfcount_ocmb_getmc[UTILM%d]: l_read_diff[0x%08X]  l_write_diff[0x%08X]",
                      i_membuf, l_read_diff, l_write_diff);
           TRAC_INFO("amec_perfcount_ocmb_getmc[UTILM%d] = 0x%04X;  temp64[0x%08X%08X]",
                      i_membuf, l_memutil, (uint32_t)(temp64>>32), (uint32_t)temp64);
       }

       l_p1_memutil = 0;
       if(IS_OCM_DDR5_MEM_TYPE(G_sysConfigData.mem_type))
       {
          // calculate port 1 utilization
          l_time_over_freq = l_sample_time_p001ns / g_amec->proc[0].memctl[i_membuf].membuf.portpair[1].addr_clock_p001ns;
          l_read_diff = amec_diff_adjust_for_overflow(l_cache_p1_read, l_prev_p1_read);
          l_write_diff = amec_diff_adjust_for_overflow(l_cache_p1_write, l_prev_p1_write);
          temp64 = (l_read_diff + l_write_diff) * g_amec->proc[0].memctl[i_membuf].membuf.portpair[1].burst_length;
          temp64 *= 10000; // times 10000 to convert to 0.01% unit
          l_p1_memutil = (uint16_t)(temp64 / l_time_over_freq);

          if((G_allow_trace_flags & ALLOW_MEM_TRACE) && L_trace_count)
          {
              TRAC_INFO("amec_perfcount_ocmb_getmc[UTILP1M%d]: l_sample_time_p001ns[0x%08X%08X]",
                         i_membuf, (uint32_t)(l_sample_time_p001ns>>32), (uint32_t)l_sample_time_p001ns);
              TRAC_INFO("amec_perfcount_ocmb_getmc[UTILP1M%d]: addr_clock_p001ns[%d]",
                         i_membuf,
                         g_amec->proc[0].memctl[i_membuf].membuf.portpair[1].addr_clock_p001ns);
              TRAC_INFO("amec_perfcount_ocmb_getmc[UTILP1M%d]: l_read_diff[0x%08X]  l_write_diff[0x%08X]",
                         i_membuf, l_read_diff, l_write_diff);
              TRAC_INFO("amec_perfcount_ocmb_getmc[UTILP1M%d] = 0x%04X;  temp64[0x%08X%08X]",
                         i_membuf, l_p1_memutil, (uint32_t)(temp64>>32), (uint32_t)temp64);
          }

       }
    }

    // only update sensors if have done defined minimum cache line reads to have a good starting accumulator
    if(L_accumulator_reads[i_membuf] >= MIN_OCMB_READS_FOR_BW_SENSOR)
    {
        if((G_allow_trace_flags & ALLOW_MEM_TRACE) && L_trace_count)
        {
            TRAC_INFO("amec_perfcount_ocmb_getmc[4]: Updating MRD%d[%d]",
                       i_membuf, g_amec->proc[0].memctl[i_membuf].membuf.portpair[0].perf.memread2ms);
            TRAC_INFO("amec_perfcount_ocmb_getmc[5]: Raw sensor cache Read data NEW[0x%08X] PREVIOUS[0x%08X]",
                       l_cache_read_total, l_prev_read_total);
            TRAC_INFO("amec_perfcount_ocmb_getmc[6]: Read Difference[0x%08X] temp64[0x%08X%08X]",
                       temp32, (uint32_t)(temp64>>32), (uint32_t)temp64);
        }
        sensor_update( (&(g_amec->proc[0].memctl[i_membuf].mrd)),
                        g_amec->proc[0].memctl[i_membuf].membuf.portpair[0].perf.memread2ms);

        sensor_update( (&(g_amec->proc[0].memctl[i_membuf].mwr)),
                       g_amec->proc[0].memctl[i_membuf].membuf.portpair[0].perf.memwrite2ms);

        sensor_update( (&(g_amec->proc[0].memctl[i_membuf].memutil)), l_memutil);
        sensor_update( (&(g_amec->proc[0].memctl[i_membuf].memutilp1)), l_p1_memutil);
    }
    else
    {
        L_accumulator_reads[i_membuf]++;

        if((G_allow_trace_flags & ALLOW_MEM_TRACE) && L_trace_count)
        {
            L_trace_count--;
            TRAC_INFO("amec_perfcount_ocmb_getmc: Skiping Membuf%d read[%d] write[%d] sensor updates",
                       i_membuf, g_amec->proc[0].memctl[i_membuf].membuf.portpair[0].perf.memread2ms,
                       g_amec->proc[0].memctl[i_membuf].membuf.portpair[0].perf.memwrite2ms);
            TRAC_INFO("amec_perfcount_ocmb_getmc: MRDM%d Raw sensor cache Read data NEW[%d] PREVIOUS[%d]",
                       i_membuf, l_cache_read_total, l_prev_read_total);
            TRAC_INFO("amec_perfcount_ocmb_getmc: MWRM%d Raw sensor cache Write data NEW[%d] PREVIOUS[%d]",
                       i_membuf, l_cache_write_total, l_prev_write_total);
        }
    }

    if(!(G_allow_trace_flags & ALLOW_MEM_TRACE))
    {
        // reset for ALLOW_MEM_TRACE to be set
        L_trace_count = 4;
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
