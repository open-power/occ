/******************************************************************************
// @file amec_sensors_centaur.c
// @brief AMEC Centaur Sensor Calculations
*/
/**
 *      @page ChangeLogs Change Logs
 *      @section _amec_sensors_centaur_c amec_sensors_centaur.c
 *      @verbatim
 *      
 *  Flag     Def/Fea    Userid    Date      Description
 *  -------- ---------- --------  --------  --------------------------------------
 *  @th00X              thallet   08/15/2012  New file
 *  @th026   865074     thallet   12/21/2012  Updated Centaur sensors
 *   @gm013  907548     milesg    11/22/2013  Memory therm monitoring support
 *   @gm015  907601     milesg    12/06/2013  L4 Bank Delete circumvention and centaur i2c recovery
 *   @gm016  909061     milesg    12/10/2013  Support memory throttling due to temperature
 *   @gm039  922963     milesg    05/28/2014  Handle centaur nest LFIR 6
 *   @mw633  933716     mware     07/29/2014  Increased maintenance command threshold to 12 from 8
 *
 *  @endverbatim
 */

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
#include "centaur_data.h"
#include "amec_smh.h"
#include "amec_slave_smh.h"
#include <trac.h>
#include "amec_sys.h"
#include "sensor_enum.h"
#include "amec_service_codes.h"
#include <amec_sensors_centaur.h>

/******************************************************************************/
/* Globals                                                                    */
/******************************************************************************/
cent_sensor_flags_t G_dimm_overtemp_bitmap = {0};
cent_sensor_flags_t G_dimm_temp_updated_bitmap = {0};
uint8_t             G_cent_overtemp_bitmap = 0;
uint8_t             G_cent_temp_updated_bitmap = 0;
extern uint8_t      G_centaur_needs_recovery;
extern uint8_t      G_centaur_nest_lfir6; //gm039

/******************************************************************************/
/* Forward Declarations                                                       */
/******************************************************************************/
void amec_update_dimm_dts_sensors(MemData * i_sensor_cache, uint8_t i_centaur);
void amec_update_centaur_dts_sensors(MemData * i_sensor_cache, uint8_t i_centaur);
void amec_perfcount_getmc( MemData * i_sensor_cache, uint8_t i_centaur); // @th026



// Function Specification
//
// Name: amec_update_dimm_dts_sensors 
//
// Description: Updates sensors taht have data grabbed by the fast core data
//              
// Flow:    FN= 
//
// Thread: RealTime Loop
//
// Task Flags: 
//
// End Function Specification
void amec_update_centaur_sensors(uint8_t i_centaur)
{
    if(CENTAUR_PRESENT(i_centaur))
    {
        MemData * l_sensor_cache = cent_get_centaur_data_ptr(i_centaur);
        if(CENTAUR_UPDATED(i_centaur)) //gm016
        {
            amec_update_dimm_dts_sensors(l_sensor_cache, i_centaur);
            amec_update_centaur_dts_sensors(l_sensor_cache, i_centaur);
        }
        amec_perfcount_getmc(l_sensor_cache, i_centaur);           // @th026
        CLEAR_CENTAUR_UPDATED(i_centaur);
    }
  
}

// Function Specification
//
// Name: amec_update_dimm_dts_sensors 
//
// Description: Updates sensors taht have data grabbed by the fast core data
//              
// Flow:    FN= 
//
// Thread: RealTime Loop
//
// Task Flags: 
//
// End Function Specification
void amec_update_dimm_dts_sensors(MemData * i_sensor_cache, uint8_t i_centaur)
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
  
    // so we will need to convert them before they are used in loop below.
    for(k=0; k < NUM_DIMMS_PER_CENTAUR; k++)
    {
        if(!CENTAUR_SENSOR_ENABLED(i_centaur, k))
        {
            continue;
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

            //request recovery (disable and re-enable sensor cache collection) -- gm015
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
// Description: Updates sensors taht have data grabbed by the fast core data
//              
// Flow:    FN= 
//
// Thread: RealTime Loop
//
// Task Flags: 
//
// End Function Specification
void amec_update_centaur_dts_sensors(MemData * i_sensor_cache, uint8_t i_centaur)
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
       !(G_centaur_nest_lfir6 & (CENTAUR0_PRESENT_MASK >> i_centaur))) //gm039
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
               !(l_sens_status == CENT_SENSOR_STATUS_VALID)) //gm039
            {
                TRAC_INFO("Centaur%d temp invalid. nest_lfir6=0x%02x", i_centaur, G_centaur_nest_lfir6); //gm039
            }

            l_fru->flags |= FRU_SENSOR_STATUS_INVALID;

            if(G_centaur_nest_lfir6 & (CENTAUR0_PRESENT_MASK >> i_centaur)) //gm039
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
// Description: Updates sensors that have data grabbed by the centaur
//              
// Flow:    FN= 
//
// Thread: RealTime Loop
//
// Task Flags: 
//
// End Function Specification
void amec_update_centaur_temp_sensors(void)
{
    uint32_t k, l_hot;

    // ------------------------------------------------------
    // Find hottest temperature from all centaurs for this P8 chip
    // ------------------------------------------------------
    for(l_hot = 0, k=0; k < MAX_NUM_CENTAURS; k++)
    {
        if(g_amec->proc[0].memctl[k].centaur.centaur_hottest.cur_temp > l_hot)
        {
            l_hot = g_amec->proc[0].memctl[k].centaur.centaur_hottest.cur_temp;
        }
    } 
    sensor_update(&g_amec->proc[0].temp2mscent,l_hot);
    AMEC_DBG("HotCentaur=%d\n",l_hot);

    // ------------------------------------------------------
    // Find hottest temperature from all DIMMs for this P8 chip
    // ------------------------------------------------------
    for(l_hot = 0, k=0; k < MAX_NUM_CENTAURS; k++)
    {
        if(g_amec->proc[0].memctl[k].centaur.tempdimmax.sample > l_hot)
        {
            l_hot = g_amec->proc[0].memctl[k].centaur.tempdimmax.sample;
        }
    } 
    sensor_update(&g_amec->proc[0].temp2msdimm,l_hot);
    AMEC_DBG("HotDimm=%d\n",l_hot);
}


// Function Specification
//
// Name: amec_perfcount_getmc 
//
// Description: Updates sensors that have data grabbed by the centaur
//              
// Flow:    FN= 
//
// Thread: RealTime Loop
//
// Task Flags: 
//
// End Function Specification
void amec_perfcount_getmc( MemData * i_sensor_cache,
                           uint8_t i_centaur)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    UINT32                      tempu       = 0;
    UINT32                      templ       = 0;
    UINT32                      temp32new   = 0;
    UINT32                      temp32      = 0;
    UINT16                      tempreg     = 0;
    UINT16                      tempreg2    = 0;
    uint8_t                     i_mc_id     = 0;
    #define AMECSENSOR_PORTPAIR_PTR(sensor_base,idx,idx2) \
        (&(g_amec->proc[0].memctl[idx].centaur.portpair[idx2].sensor_base))

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    MemData * l_sensor_cache = i_sensor_cache;

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

        // ----------------------------------------------------
        //  Interim Calculation:  MWR2MSP0Mx (0.01 Mrps) Memory write requests per sec
        // ----------------------------------------------------

        // Extract write bandwidth
        temp32new = (templ); // left shift into top 20 bits of 32 bits

        temp32 = temp32new - g_amec->proc[0].memctl[i_centaur].centaur.portpair[i_mc_id].perf.wr_cnt_accum;
        g_amec->proc[0].memctl[i_centaur].centaur.portpair[i_mc_id].perf.wr_cnt_accum = temp32new;  // Save latest accumulator away for next time

        // Read every 2 ms....to convert to 0.01 Mrps = ((2ms read * 500)/10000)
        tempreg = ((temp32*5)/100); 

        g_amec->proc[0].memctl[i_centaur].centaur.portpair[i_mc_id].perf.memwrite2ms = tempreg;

        // ----------------------------------------------------
        // Interim Calculation:  MRD2MSP0Mx (0.01 Mrps) Memory read requests per sec
        // ----------------------------------------------------

        // Extract read bandwidth
        temp32new = (tempu); // left shift into top 20 bits of 32 bits

        temp32 = temp32new - g_amec->proc[0].memctl[i_centaur].centaur.portpair[i_mc_id].perf.rd_cnt_accum;
        g_amec->proc[0].memctl[i_centaur].centaur.portpair[i_mc_id].perf.rd_cnt_accum = temp32new;  // Save latest accumulator away for next time

        // Read every 2 ms....to convert to 0.01 Mrps = ((2ms read * 500)/10000)
        tempreg = ((temp32*5)/100); 

        g_amec->proc[0].memctl[i_centaur].centaur.portpair[i_mc_id].perf.memread2ms = tempreg;

        // Go after second MC performance counter (power ups and activations)
        if (i_mc_id == 0)
        {
            tempu = l_sensor_cache->scache.mba01_act;
            templ = l_sensor_cache->scache.mba01_powerups;
        }
        else
        {
            tempu = l_sensor_cache->scache.mba23_act;
            templ = l_sensor_cache->scache.mba23_powerups;
        }

        // ----------------------------------------------------
        // Sensor:  MPU2MSP0Mx (0.01 Mrps) Memory power-up requests per sec
        // ----------------------------------------------------
        // Extract power up count
        temp32new = (templ); // left shift into top 20 bits of 32 bits

        // For DD1.0, we only have 1 channel field that we use; DD2.0 we need to add another channel
        temp32 = temp32new - g_amec->proc[0].memctl[i_centaur].centaur.portpair[i_mc_id].perf.pwrup_cnt_accum;
        g_amec->proc[0].memctl[i_centaur].centaur.portpair[i_mc_id].perf.pwrup_cnt_accum = temp32new;  // Save latest accumulator away for next time
        tempreg=(UINT16)(temp32>>12);   // Select upper 20 bits
        g_amec->proc[0].memctl[i_centaur].centaur.portpair[i_mc_id].perf.pwrup_cnt=(UINT16)tempreg;
        sensor_update(AMECSENSOR_PORTPAIR_PTR(mpu2ms,i_centaur,i_mc_id), tempreg);

        // ----------------------------------------------------
        // Sensor:  MAC2MSP0Mx (0.01 Mrps)  Memory activation requests per sec
        // ----------------------------------------------------
        // Extract activation count
        temp32 = templ;
        temp32 += tempu;

        temp32new = (temp32);   // left shift into top 20 bits of 32 bits
        // For DD1.0, we only have 1 channel field that we use; DD2.0 we need to add another channel
        temp32 = temp32new - g_amec->proc[0].memctl[i_centaur].centaur.portpair[i_mc_id].perf.act_cnt_accum;
        g_amec->proc[0].memctl[i_centaur].centaur.portpair[i_mc_id].perf.act_cnt_accum = temp32new;  // Save latest accumulator away for next time
        tempreg=(UINT16)(temp32>>12);   // Select lower 16 of 20 bits
        g_amec->proc[0].memctl[i_centaur].centaur.portpair[i_mc_id].perf.act_cnt=(UINT16)tempreg;
        sensor_update(AMECSENSOR_PORTPAIR_PTR(mac2ms,i_centaur,i_mc_id), tempreg);

        // ----------------------------------------------------
        // Sensor:  MTS2MS (count) Last received Timestamp (frame count) from Centaur
        // ----------------------------------------------------
        // Extract framecount (clock is 266.6666666MHz * 0.032 / 4096)=2083.
        temp32new = l_sensor_cache->scache.frame_count;

        // For DD1.0, we only have 1 channel field that we use; DD2.0 we need to add another channel
        temp32 = temp32new - g_amec->proc[0].memctl[i_centaur].centaur.portpair[i_mc_id].perf.fr2_cnt_accum;
        g_amec->proc[0].memctl[i_centaur].centaur.portpair[i_mc_id].perf.fr2_cnt_accum = temp32new;  // Save latest accumulator away for next time
        tempreg=(UINT16)(temp32>>12);   // Select upper 20 bits

        g_amec->proc[0].memctl[i_centaur].centaur.portpair[i_mc_id].perf.fr2_cnt=(UINT16)tempreg;
        sensor_update(AMECSENSOR_PORTPAIR_PTR(mts2ms,i_centaur,i_mc_id), tempreg);

        if (i_mc_id == 0)
        {
            tempu = l_sensor_cache->scache.mba01_cache_hits_rd;
            templ = l_sensor_cache->scache.mba01_cache_hits_wr;
        }
        else
        {
            tempu = l_sensor_cache->scache.mba23_cache_hits_rd;
            templ = l_sensor_cache->scache.mba23_cache_hits_wr;
        }
        // ----------------------------------------------------
        // Sensor:  M4RD2MS  (0.01 Mrps) Memory cached (L4) read requests per sec
        // ----------------------------------------------------
        temp32new = (tempu); // left shift into top 20 bits of 32 bits
        temp32 = temp32new - g_amec->proc[0].memctl[i_centaur].centaur.portpair[i_mc_id].perf.l4_rd_cnt_accum;
        g_amec->proc[0].memctl[i_centaur].centaur.portpair[i_mc_id].perf.l4_rd_cnt_accum = temp32new;  // Save latest accumulator away for next time

        // Read every 2 ms....to convert to 0.01 Mrps = ((2ms read * 500)/10000)
        tempreg = ((temp32*5)/100);
        tempreg2 = g_amec->proc[0].memctl[i_centaur].centaur.portpair[i_mc_id].perf.memread2ms;
        // Firmware workaround for hardware bug: hits - memreads ~= hits  @mw565
        tempreg = tempreg - tempreg2;
        // Deal with maintenance commands or quantization in counters being off by 12 and force to 0 // @mw565 @mw633
        if ((tempreg > 32767) || (tempreg <= 12))
        {
            tempreg=0;
        }

        g_amec->proc[0].memctl[i_centaur].centaur.portpair[i_mc_id].perf.l4rd2ms = tempreg;
        sensor_update(AMECSENSOR_PORTPAIR_PTR(m4rd2ms,i_centaur,i_mc_id), tempreg);

        // ----------------------------------------------------
        // Sensor:  M4WR2MS  (0.01 Mrps) Memory cached (L4) write requests per sec
        // ----------------------------------------------------
        temp32new = (templ); // left shift into top 20 bits of 32 bits
        temp32 = temp32new - g_amec->proc[0].memctl[i_centaur].centaur.portpair[i_mc_id].perf.l4_wr_cnt_accum;
        g_amec->proc[0].memctl[i_centaur].centaur.portpair[i_mc_id].perf.l4_wr_cnt_accum = temp32new;  // Save latest accumulator away for next time

        // Read every 2 ms....to convert to 0.01 Mrps = ((2ms read * 500)/10000)
        tempreg = ((temp32*5)/100); 

        tempreg2 = g_amec->proc[0].memctl[i_centaur].centaur.portpair[i_mc_id].perf.memwrite2ms;
        // Firmware workaround for hardware bug: hits - memwrites ~= hits  @mw565
        tempreg = tempreg - tempreg2;
        // Deal with maintenance commands or quantization in counters being off by 12 and force to 0  // @mw565 @mw633
        if ((tempreg > 32767) || (tempreg <= 12))
        {
            tempreg=0;
        }

        g_amec->proc[0].memctl[i_centaur].centaur.portpair[i_mc_id].perf.l4wr2ms = tempreg;
        sensor_update(AMECSENSOR_PORTPAIR_PTR(m4wr2ms,i_centaur,i_mc_id), tempreg);

        // ----------------------------------------------------
        // Sensor:  MIRB2MS  (0.01 Mevents/s) Memory Inter-request arrival idle intervals
        // ----------------------------------------------------
        temp32new = (i_mc_id == 0) ? l_sensor_cache->scache.mba01_intreq_arr_cnt_base : l_sensor_cache->scache.mba23_intreq_arr_cnt_base;
        temp32 = temp32new - g_amec->proc[0].memctl[i_centaur].centaur.portpair[i_mc_id].perf.intreq_base_accum;
        g_amec->proc[0].memctl[i_centaur].centaur.portpair[i_mc_id].perf.intreq_base_accum = temp32new;  // Save latest accumulator away for next time

        // Read every 2 ms....to convert to 0.01 Mrps = ((2ms read * 500)/10000)
        tempreg = ((temp32*5)/100); 
        g_amec->proc[0].memctl[i_centaur].centaur.portpair[i_mc_id].perf.mirb2ms = tempreg;
        sensor_update(AMECSENSOR_PORTPAIR_PTR(mirb2ms,i_centaur,i_mc_id), tempreg);

        // ----------------------------------------------------
        // Sensor:  MIRL2MS  (0.01 Mevents/s) Memory Inter-request arrival idle intervals longer than low threshold
        // ----------------------------------------------------
        temp32new = (i_mc_id == 0) ? l_sensor_cache->scache.mba01_intreq_arr_cnt_low : l_sensor_cache->scache.mba23_intreq_arr_cnt_low;
        temp32 = temp32new - g_amec->proc[0].memctl[i_centaur].centaur.portpair[i_mc_id].perf.intreq_low_accum;
        g_amec->proc[0].memctl[i_centaur].centaur.portpair[i_mc_id].perf.intreq_low_accum = temp32new;  // Save latest accumulator away for next time

        // Read every 2 ms....to convert to 0.01 Mrps = ((2ms read * 500)/10000)
        tempreg = ((temp32*5)/100); 
        g_amec->proc[0].memctl[i_centaur].centaur.portpair[i_mc_id].perf.mirl2ms = tempreg;
        sensor_update(AMECSENSOR_PORTPAIR_PTR(mirl2ms,i_centaur,i_mc_id), tempreg);

        // ----------------------------------------------------
        // Sensor:  MIRM2MS  (0.01 Mevents/s) Memory Inter-request arrival idle intervals longer than medium threshold
        // ----------------------------------------------------
        temp32new = (i_mc_id == 0) ? l_sensor_cache->scache.mba01_intreq_arr_cnt_med : l_sensor_cache->scache.mba23_intreq_arr_cnt_med;
        temp32 = temp32new - g_amec->proc[0].memctl[i_centaur].centaur.portpair[i_mc_id].perf.intreq_med_accum;
        g_amec->proc[0].memctl[i_centaur].centaur.portpair[i_mc_id].perf.intreq_med_accum = temp32new;  // Save latest accumulator away for next time

        // Read every 2 ms....to convert to 0.01 Mrps = ((2ms read * 500)/10000)
        tempreg = ((temp32*5)/100); 
        g_amec->proc[0].memctl[i_centaur].centaur.portpair[i_mc_id].perf.mirm2ms = tempreg;
        sensor_update(AMECSENSOR_PORTPAIR_PTR(mirm2ms,i_centaur,i_mc_id), tempreg);

        // ----------------------------------------------------
        // Sensor:  MIRH2MS  (0.01 Mevents/s) Memory Inter-request arrival idle intervals longer than high threshold
        // ----------------------------------------------------
        temp32new = (i_mc_id == 0) ? l_sensor_cache->scache.mba01_intreq_arr_cnt_high : l_sensor_cache->scache.mba23_intreq_arr_cnt_high;
        temp32 = temp32new - g_amec->proc[0].memctl[i_centaur].centaur.portpair[i_mc_id].perf.intreq_high_accum;
        g_amec->proc[0].memctl[i_centaur].centaur.portpair[i_mc_id].perf.intreq_high_accum = temp32new;  // Save latest accumulator away for next time

        // Read every 2 ms....to convert to 0.01 Mrps = ((2ms read * 500)/10000)
        tempreg = ((temp32*5)/100); 
        g_amec->proc[0].memctl[i_centaur].centaur.portpair[i_mc_id].perf.mirh2ms = tempreg;
        sensor_update(AMECSENSOR_PORTPAIR_PTR(mirh2ms,i_centaur,i_mc_id), tempreg);
    }

    // ----------------------------------------------------
    // Sensor:  MIRC2MS  (0.01 Mevents/s) Memory Inter-request arrival idle interval longer than programmed threshold
    // ----------------------------------------------------
    temp32new = l_sensor_cache->scache.intreq_arr_cnt_high_latency;
    temp32 = temp32new - g_amec->proc[0].memctl[i_centaur].centaur.perf.intreq_highlatency_accum;
    g_amec->proc[0].memctl[i_centaur].centaur.perf.intreq_highlatency_accum = temp32new;  // Save latest accumulator away for next time
    
    // Read every 2 ms....to convert to 0.01 Mrps = ((2ms read * 500)/10000)
    tempreg = ((temp32*5)/100); 
    g_amec->proc[0].memctl[i_centaur].centaur.perf.mirc2ms = tempreg;
    sensor_update((&(g_amec->proc[0].memctl[i_centaur].centaur.mirc2ms)), tempreg);

    // ----------------------------------------------------
    // Sensor:  MLP2MS   (events/s) Number of LP2 exits
    // ----------------------------------------------------
    temp32new = l_sensor_cache->scache.lp2_exits;
    temp32 = temp32new - g_amec->proc[0].memctl[i_centaur].centaur.perf.lp2exit_accum;
    g_amec->proc[0].memctl[i_centaur].centaur.perf.lp2exit_accum = temp32new;  // Save latest accumulator away for next time
    
    // Read every 2 ms....to convert to 0.01 Mrps = ((2ms read * 500)/10000)
    tempreg = ((temp32*5)/100); 
    g_amec->proc[0].memctl[i_centaur].centaur.perf.mlp2_2ms = tempreg;
    sensor_update((&(g_amec->proc[0].memctl[i_centaur].centaur.mlp2ms)), tempreg);

    // ----------------------------------------------------
    // Sensor:  MRD2MSP0Mx (0.01 Mrps) Memory read requests per sec
    // ----------------------------------------------------
    //AMEC_SENSOR_UPDATE(io_mc_ptr->chpair[i_mc_id].memread32ms, tempreg);
    tempreg = g_amec->proc[0].memctl[i_centaur].centaur.portpair[0].perf.memread2ms;
    tempreg += g_amec->proc[0].memctl[i_centaur].centaur.portpair[1].perf.memread2ms;
    sensor_update( (&(g_amec->proc[0].memctl[i_centaur].mrd2ms)), tempreg);
    
    // ----------------------------------------------------
    // Sensor:  MWR2MSP0Mx (0.01 Mrps) Memory write requests per sec
    // ----------------------------------------------------
    //sensor_update(AMECSENSOR_ARRAY_PTR(MRD2MSP0M0,), tempreg )
    tempreg = g_amec->proc[0].memctl[i_centaur].centaur.portpair[0].perf.memwrite2ms;
    tempreg += g_amec->proc[0].memctl[i_centaur].centaur.portpair[1].perf.memwrite2ms;
    sensor_update( (&(g_amec->proc[0].memctl[i_centaur].mwr2ms)), tempreg);


    return;
}


