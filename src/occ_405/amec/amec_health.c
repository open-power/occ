/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/amec/amec_health.c $                              */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2021                        */
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

//*************************************************************************/
// Includes
//*************************************************************************/
#include "amec_health.h"
#include "amec_sys.h"
#include "amec_service_codes.h"
#include "occ_service_codes.h"
#include <memory.h>
#include <memory_data.h>
#include <proc_data.h>

//*************************************************************************/
// Externs
//*************************************************************************/
extern bool G_simics_environment;
extern bool G_log_gpe1_error;

//*************************************************************************/
// Defines/Enums
//*************************************************************************/

//*************************************************************************/
// Globals
//*************************************************************************/

// Have we already called out the dimm for overtemp (bitmap of dimms)?
dimm_sensor_flags_t G_dimm_overtemp_logged_bitmap = {{0}};

// Have we already called out the dimm for timeout (bitmap of dimms)?
dimm_sensor_flags_t G_dimm_timeout_logged_bitmap = {{0}};

// Are any dimms currently in the timedout state (bitmap of dimm)?
dimm_sensor_flags_t G_dimm_temp_expired_bitmap = {{0}};

// Have we already called out the membuf for timeout (bitmap of membufs)
uint16_t G_membuf_timeout_logged_bitmap = 0;
// Timedout state of OCMB "DIMM" sensors by fru type (bitmap of DTS type)
uint8_t G_ocmb_dts_type_expired_bitmap = 0;


// Have we already called out the membuf for overtemp (bitmap of membufs)
uint16_t G_membuf_overtemp_logged_bitmap = 0;

// Are any mem controllers currently in the timedout state (bitmap of membufs)
uint16_t G_membuf_temp_expired_bitmap = 0;

// Array to store the update tag of each core's temperature sensor
uint32_t G_core_temp_update_tag[MAX_NUM_CORES] = {0};

// Reading VRM Vdd temperature timedout?
bool G_vrm_vdd_temp_expired = false;

//*************************************************************************/
// Function Declarations
//*************************************************************************/

uint64_t amec_mem_get_huid(uint8_t i_membuf, uint8_t i_dimm)
{
    uint64_t l_huid = 0;

    if( (i_membuf < MAX_NUM_OCMBS) && (i_dimm < NUM_DIMMS_PER_OCMB) )
    {
        l_huid = (uint64_t)G_sysConfigData.dimm_huids[i_membuf][i_dimm];

        if(l_huid == 0)
        {
            TRAC_ERR("amec_mem_get_huid: 0 HUID found for membuf[%02X] dimm[0x%02X]",
                      i_membuf, i_dimm);
        }
    }
    else
    {
        TRAC_ERR("amec_mem_get_huid: membuf[%02X] dimm[0x%02X] out of range",
                  i_membuf, i_dimm);
    }
    return l_huid;
}


// If i_dimm is 0xff it is assumed that the caller wishes to
// mark the membuf as being logged.  Otherwise, it is assumed
// that the dimm should be marked.
void amec_mem_mark_logged(uint8_t i_membuf,
                          uint8_t i_dimm,
                          uint16_t* i_clog_bitmap,
                          uint8_t*  i_dlog_bitmap)
{
    if(i_dimm == 0xff)
    {
        //mark the membuf as being called out.
        *i_clog_bitmap |= MEMBUF0_PRESENT_MASK >> i_membuf;
    }
    else
    {
        //mark the dimm as being called out.
        *i_dlog_bitmap |= DIMM_SENSOR0 >> i_dimm;
    }
}


/*
 * Function Specification
 *
 * Name: amec_health_check_dimm_temp
 *
 * Description: Check if DIMM temperature exceeds the error temperature
 *              as defined in thermal control thresholds
 *              (ERROR field for DIMM FRU Type)
 *
 * End Function Specification
 */
void amec_health_check_dimm_temp()
{
    uint16_t                    l_ot_error, l_max_temp;
    sensor_t                    *l_sensor;
    uint8_t                     l_dimm;
    uint8_t                     l_port;
    uint32_t                    l_callouts_count = 0;
    uint8_t                     l_new_callouts;
    uint64_t                    l_huid;
    errlHndl_t                  l_err = NULL;

    // Check to see if any dimms have reached the error temperature that
    // haven't been called out already
    if( (G_dimm_overtemp_bitmap.dw[0] == G_dimm_overtemp_logged_bitmap.dw[0]) &&
        (G_dimm_overtemp_bitmap.dw[1] == G_dimm_overtemp_logged_bitmap.dw[1]) )
    {
        return;
    }

    //iterate over all dimms
    for(l_port = 0; l_port < MAX_NUM_OCMBS; l_port++)
    {
        //only callout a dimm if it hasn't been called out already
        l_new_callouts = G_dimm_overtemp_bitmap.bytes[l_port] ^
                         G_dimm_overtemp_logged_bitmap.bytes[l_port];

        //skip to next port if no new callouts for this one
        if (!l_new_callouts || (G_dimm_overtemp_bitmap.bytes[l_port] == 0))
        {
            continue;
        }

        // if the previous port had errors commit it so this port gets new error log
        if(l_err)
        {
           commitErrl(&l_err);
           l_callouts_count = 0;
        }

        //find the dimm(s) that need to be called out for this port
        for(l_dimm = 0; l_dimm < NUM_DIMMS_PER_OCMB; l_dimm++)
        {
            if (!(l_new_callouts & (DIMM_SENSOR0 >> l_dimm)))
            {
                continue;
            }

            fru_temp_t* l_fru;
            l_fru = &g_amec->proc[0].memctl[l_port].membuf.dimm_temps[l_dimm];
            switch(l_fru->temp_fru_type)
            {
               case DATA_FRU_DIMM:
                  l_ot_error = g_amec->thermaldimm.ot_error;
                  l_sensor = getSensorByGsid(TEMPDIMMTHRM);
                  l_max_temp = l_sensor->sample_max;
                  break;

               case DATA_FRU_MEMCTRL_DRAM:
                  l_ot_error = g_amec->thermalmcdimm.ot_error;
                  l_sensor = getSensorByGsid(TEMPMCDIMMTHRM);
                  l_max_temp = l_sensor->sample_max;
                  break;

               case DATA_FRU_PMIC:
                  l_ot_error = g_amec->thermalpmic.ot_error;
                  l_sensor = getSensorByGsid(TEMPPMICTHRM);
                  l_max_temp = l_sensor->sample_max;
                  break;

               case DATA_FRU_MEMCTRL_EXT:
                  l_ot_error = g_amec->thermalmcext.ot_error;
                  l_sensor = getSensorByGsid(TEMPMCEXTTHRM);
                  l_max_temp = l_sensor->sample_max;
                  break;

               default:
                  // this is a code bug trace and let the error be logged for debug
                  TRAC_ERR("amec_health_check_dimm_temp: sensor[%04X] marked as OT has invalid type[%d]",
                     (l_port<<8)|l_dimm, l_fru->temp_fru_type);
                  l_ot_error = 0xff;
                  l_max_temp = 0xff;
                  break;
            }
            TRAC_ERR("amec_health_check_dimm_temp: sensor[%04X] type[0x%02X] reached error temp[%d] current[%d]",
                     (l_port<<8)|l_dimm, l_fru->temp_fru_type, l_ot_error, l_fru->cur_temp);

            amec_mem_mark_logged(l_port,
                                 l_dimm,
                                 &G_membuf_overtemp_logged_bitmap,
                                 &G_dimm_overtemp_logged_bitmap.bytes[l_port]);
            TRAC_ERR("amec_health_check_dimm_temp: DIMM%04X overtemp - %dC",
                     (l_port<<8)|l_dimm, l_fru->cur_temp);

            // Create single elog with up to MAX_CALLOUTS
            // this will be generic regardless of temperature sensor type, the callouts will be correct
            // and the traces will point to specific types/thresholds
            if(l_callouts_count < ERRL_MAX_CALLOUTS)
            {
                //If we don't have an error log for the callout, create one
                if(!l_err)
                {
                    TRAC_ERR("amec_health_check_dimm_temp: Creating log for port[%d] OT bitmap[0x%02X] logged bitmap[0x%02X]",
                             l_port,
                             G_dimm_overtemp_bitmap.bytes[l_port],
                             G_dimm_overtemp_logged_bitmap.bytes[l_port]);

                    /* @
                     * @errortype
                     * @moduleid    AMEC_HEALTH_CHECK_DIMM_TEMP
                     * @reasoncode  DIMM_ERROR_TEMP
                     * @userdata1   Maximum DIMM temperature
                     * @userdata2   DIMM temperature threshold
                     * @userdata4   OCC_NO_EXTENDED_RC
                     * @devdesc     Memory DIMM(s) exceeded maximum safe
                     *              temperature.
                     */
                    l_err = createErrl(AMEC_HEALTH_CHECK_DIMM_TEMP,    //modId
                                       DIMM_ERROR_TEMP,               //reasoncode
                                       OCC_NO_EXTENDED_RC,            //Extended reason code
                                       ERRL_SEV_PREDICTIVE,           //Severity
                                       NULL,                          //Trace Buf
                                       DEFAULT_TRACE_SIZE,            //Trace Size
                                       l_max_temp,                    //userdata1
                                       l_ot_error);                   //userdata2

                    // Callout the "over temperature" procedure
                    addCalloutToErrl(l_err,
                                     ERRL_CALLOUT_TYPE_COMPONENT_ID,
                                     ERRL_COMPONENT_ID_OVER_TEMPERATURE,
                                     ERRL_CALLOUT_PRIORITY_HIGH);
                    l_callouts_count = 1;
                }

                // Callout dimm
                l_huid = amec_mem_get_huid(l_port, l_dimm);
                addCalloutToErrl(l_err,
                                 ERRL_CALLOUT_TYPE_HUID,
                                 l_huid,
                                 ERRL_CALLOUT_PRIORITY_MED);

                l_callouts_count++;
            }
        }//iterate over dimms
    }//iterate over ports

    if(l_err)
    {
        commitErrl(&l_err);
    }

} // end amec_health_check_dimm_temp()


/*
 * Function Specification
 *
 * Name: amec_health_check_dimm_timeout
 *
 * Description: Check for membuf/dimm timeout condition
 *              as defined in thermal control thresholds
 *              (MAX_READ_TIMEOUT field for membuf/DIMM FRU Type)
 *
 * End Function Specification
 */
void amec_health_check_dimm_timeout()
{
    static dimm_sensor_flags_t L_temp_update_bitmap_prev = {{0}};
    static dimm_sensor_flags_t L_trace_resume = {{0}};
    dimm_sensor_flags_t l_need_inc, l_need_clr, l_temp_update_bitmap;
    uint8_t l_dimm, l_other_dimm, l_membuf, l_temp_timeout;
    fru_temp_t* l_fru;
    errlHndl_t  l_err = NULL;
    errlHndl_t  l_redundancy_lost_err = NULL;
    uint32_t    l_callouts_count = 0;
    uint32_t    l_redundancy_lost_callouts_count = 0;
    uint64_t    l_huid;
    uint8_t     l_max_membuf = 0; // number of membufs
    uint8_t     l_max_dimm_per_membuf = 0; // dimms per membuf
    uint8_t     l_ocm_dts_type_expired_bitmap = 0;
    bool        l_redundancy_lost = FALSE;
    ERRL_SEVERITY l_severity = ERRL_SEV_PREDICTIVE;

    do
    {
        //For every dimm sensor there are 3 cases to consider
        //
        //1) sensor is enabled and not updated (need to increment timer and check for timeout)
        //2) sensor is enabled and updated but wasn't updated on previous check (need to clear timer)
        //3) sensor is enabled and updated and was updated on previous check (do nothing)

        //Grab snapshot of G_dimm_temp_updated_bitmap and clear it
        l_temp_update_bitmap.dw[0] = G_dimm_temp_updated_bitmap.dw[0];
        l_temp_update_bitmap.dw[1] = G_dimm_temp_updated_bitmap.dw[1];
        G_dimm_temp_updated_bitmap.dw[0] = 0;
        G_dimm_temp_updated_bitmap.dw[1] = 0;

        //check if we need to increment any timers (haven't been updated in the last second)
        l_need_inc.dw[0] = G_dimm_enabled_sensors.dw[0] & ~l_temp_update_bitmap.dw[0];
        l_need_inc.dw[1] = G_dimm_enabled_sensors.dw[1] & ~l_temp_update_bitmap.dw[1];

        //check if we need to clear any timers (updated now but not updated previously)
        l_need_clr.dw[0] = l_temp_update_bitmap.dw[0] & ~L_temp_update_bitmap_prev.dw[0];
        l_need_clr.dw[1] = l_temp_update_bitmap.dw[1] & ~L_temp_update_bitmap_prev.dw[1];

        //save off the previous bitmap of updated sensors for next time
        L_temp_update_bitmap_prev.dw[0] = l_temp_update_bitmap.dw[0];
        L_temp_update_bitmap_prev.dw[1] = l_temp_update_bitmap.dw[1];

        //only go further if we actually have work to do here.
        if(!l_need_inc.dw[0] && !l_need_inc.dw[1] &&
           !l_need_clr.dw[0] && !l_need_clr.dw[1])
        {
            //nothing to do
            break;
        }

        l_max_membuf = MAX_NUM_OCMBS;
        l_max_dimm_per_membuf = NUM_DIMMS_PER_OCMB;

        //iterate across all ports incrementing dimm sensor timers as needed
        for(l_membuf = 0; l_membuf < MAX_NUM_OCMBS; l_membuf++)
        {
            //any dimm timers on this port need incrementing?
            if(!l_need_inc.bytes[l_membuf])
            {
                // All dimm sensors were updated for this port
                // Trace this fact and clear the expired byte for all DIMMs on this port
                if(G_dimm_temp_expired_bitmap.bytes[l_membuf])
                {
                    G_dimm_temp_expired_bitmap.bytes[l_membuf] = 0;
                    TRAC_INFO("All DIMM sensors for membuf %d have been updated", l_membuf);
                }
                continue;
            }

            //There's at least one dimm requiring an increment, find the dimm
            for(l_dimm = 0; l_dimm < NUM_DIMMS_PER_OCMB; l_dimm++)
            {
                //not this one, check if we need to clear the dimm timeout and go to the next one
                if(!(l_need_inc.bytes[l_membuf] & (DIMM_SENSOR0 >> l_dimm)))
                {
                    // Clear this one if needed
                    if(G_dimm_temp_expired_bitmap.bytes[l_membuf] & (DIMM_SENSOR0 >> l_dimm))
                    {
                        G_dimm_temp_expired_bitmap.bytes[l_membuf] &= ~(DIMM_SENSOR0 >> l_dimm);
                    }
                    continue;
                }

                //we found one.
                l_redundancy_lost = FALSE;
                l_fru = &g_amec->proc[0].memctl[l_membuf].membuf.dimm_temps[l_dimm];

                //increment timer
                l_fru->sample_age++;

                //handle wrapping
                if(!l_fru->sample_age)
                {
                    l_fru->sample_age = -1;
                }

                // In Simics: the RTL timer is increased and a DIMM reading will not always
                // complete on each call.  (an error will still be logged if reading does not
                // meet the DIMM MAX_READ_TIMEOUT.)
                if((l_fru->sample_age == 2) && (!G_simics_environment))
                {
                    TRAC_INFO("No new DIMM temperature available for DIMM%04X (cur_temp[%d] flags[0x%02X])",
                              (l_membuf<<8)|l_dimm, l_fru->cur_temp, l_fru->flags);

                    // set flag to trace if this ever recovers
                    L_trace_resume.bytes[l_membuf] |= (DIMM_SENSOR0 >> l_dimm);
                }

                //check if the temperature reading is still useable
                if(l_fru->temp_fru_type == DATA_FRU_DIMM)
                {
                   l_temp_timeout = g_amec->thermaldimm.temp_timeout;
                }

                else if(l_fru->temp_fru_type == DATA_FRU_MEMCTRL_DRAM)
                {
                   l_temp_timeout = g_amec->thermalmcdimm.temp_timeout;
                }

                else if(l_fru->temp_fru_type == DATA_FRU_PMIC)
                {
                   l_temp_timeout = g_amec->thermalpmic.temp_timeout;
                }

                else if(l_fru->temp_fru_type == DATA_FRU_MEMCTRL_EXT)
                {
                   l_temp_timeout = g_amec->thermalmcext.temp_timeout;
                }

                else // invalid type or not used, ignore
                   l_temp_timeout = 0xff;

                if( (l_temp_timeout == 0xff) ||
                    (l_fru->sample_age < l_temp_timeout) )
                {
                    continue;
                }

                //temperature has expired.  Notify control algorithms which DIMM DTS and type
                if(!(G_dimm_temp_expired_bitmap.bytes[l_membuf] & (DIMM_SENSOR0 >> l_dimm)))
                {
                    G_dimm_temp_expired_bitmap.bytes[l_membuf] |= (DIMM_SENSOR0 >> l_dimm);
                    TRAC_ERR("Timed out reading DIMM%04X temperature sensor type[0x%02X]",
                             (l_membuf<<8)|l_dimm,
                             l_fru->temp_fru_type);

                    // mark that this sensor is lost
                    l_fru->flags |= FRU_SENSOR_STATUS_REDUNDANCY_LOST;

                    // if there is a good "DIMM" sensor then this is loss of redundancy only
                    for(l_other_dimm = 0; l_other_dimm < NUM_DIMMS_PER_OCMB; l_other_dimm++)
                    {
                       // make sure the other sensor is being used
                       if( (l_other_dimm != l_dimm) &&
                           (MEMBUF_SENSOR_ENABLED(l_membuf, l_other_dimm)) )
                       {
                           if( !(g_amec->proc[0].memctl[l_membuf].membuf.dimm_temps[l_other_dimm].flags &
                                 FRU_SENSOR_STATUS_REDUNDANCY_LOST) )
                           {
                               l_redundancy_lost = TRUE;
                               // since there is another sensor mark that this DIMM sensor is no longer enabled
                               G_dimm_enabled_sensors.bytes[l_membuf] &= ( ~(DIMM_SENSOR0 >> l_dimm) );
                           }
                       }
                    }
                }
                //If we've already logged an error for this FRU go to the next one.
                if(G_dimm_timeout_logged_bitmap.bytes[l_membuf] & (DIMM_SENSOR0 >> l_dimm))
                {
                    continue;
                }

                // To prevent DIMMs from incorrectly being called out, don't log errors if there have
                // been timeouts with GPE1 tasks not finishing
                if(G_error_history[ERRH_GPE1_NOT_IDLE] > l_temp_timeout)
                {
                    TRAC_ERR("Timed out reading DIMM temperature due to GPE1 issues");
                    // give notification that GPE1 error should now be logged which will reset the OCC
                    G_log_gpe1_error = TRUE;
                    // no reason to check anymore since all DIMMs are collected from the same GPE
                    break;
                }

                TRAC_ERR("Timed out reading DIMM%04X temperature (cur_temp[%d] flags[0x%02X] redundancy[%d])",
                         (l_membuf<<8)|l_dimm, l_fru->cur_temp, l_fru->flags, l_redundancy_lost);

                //Mark DIMM as logged so we don't log it more than once
                amec_mem_mark_logged(l_membuf,
                                     l_dimm,
                                     &G_membuf_timeout_logged_bitmap,
                                     &G_dimm_timeout_logged_bitmap.bytes[l_membuf]);

                // Create single elog for error and single for redundancy lost with up to MAX_CALLOUTS
                if( (!l_redundancy_lost) && (l_callouts_count < ERRL_MAX_CALLOUTS) )
                {
                    if(!l_err)
                    {
                        // make this info only in simics
                        if(G_simics_environment)
                            l_severity = ERRL_SEV_INFORMATIONAL;

                        /* @
                         * @errortype
                         * @moduleid    AMEC_HEALTH_CHECK_DIMM_TIMEOUT
                         * @reasoncode  FRU_TEMP_TIMEOUT
                         * @userdata1   timeout value in seconds
                         * @userdata2   0
                         * @userdata4   ERC_AMEC_DIMM_TEMP_TIMEOUT
                         * @devdesc     Failed to read a memory DIMM temperature
                         *
                         */
                        l_err = createErrl(AMEC_HEALTH_CHECK_DIMM_TIMEOUT,    //modId
                                           FRU_TEMP_TIMEOUT,                  //reasoncode
                                           ERC_AMEC_DIMM_TEMP_TIMEOUT,        //Extended reason code
                                           l_severity,                        //Severity
                                           NULL,                              //Trace Buf
                                           DEFAULT_TRACE_SIZE,                //Trace Size
                                           l_temp_timeout,                    //userdata1
                                           0);                                //userdata2
                    }

                    //Get the HUID for the DIMM and add callout
                    l_huid = amec_mem_get_huid(l_membuf, l_dimm);
                    addCalloutToErrl(l_err,
                                     ERRL_CALLOUT_TYPE_HUID,
                                     l_huid,
                                     ERRL_CALLOUT_PRIORITY_MED);

                    l_callouts_count++;
                }
                else if( l_redundancy_lost && (l_redundancy_lost_callouts_count < ERRL_MAX_CALLOUTS) )
                {
                    if(!l_redundancy_lost_err)
                    {
                        /* @
                         * @errortype
                         * @moduleid    AMEC_HEALTH_CHECK_DIMM_TIMEOUT
                         * @reasoncode  FRU_TEMP_REDUNDANCY_LOST
                         * @userdata1   timeout value in seconds
                         * @userdata2   0
                         * @userdata4   ERC_AMEC_DIMM_TEMP_TIMEOUT
                         * @devdesc     Failed to read a memory DIMM temperature
                         *
                         */
                        l_redundancy_lost_err = createErrl(AMEC_HEALTH_CHECK_DIMM_TIMEOUT,    //modId
                                                           FRU_TEMP_REDUNDANCY_LOST,          //reasoncode
                                                           ERC_AMEC_DIMM_TEMP_TIMEOUT,        //Extended reason code
                                                           ERRL_SEV_INFORMATIONAL,            //Severity
                                                           NULL,                              //Trace Buf
                                                           DEFAULT_TRACE_SIZE,                //Trace Size
                                                           l_temp_timeout,                    //userdata1
                                                           0);                                //userdata2

                        // set the mfg action flag (allows callout to be added to info error)
                        setErrlActions(l_redundancy_lost_err, ERRL_ACTIONS_MANUFACTURING_ERROR);
                    }

                    //Get the HUID for the DIMM and add callout
                    l_huid = amec_mem_get_huid(l_membuf, l_dimm);
                    addCalloutToErrl(l_redundancy_lost_err,
                                     ERRL_CALLOUT_TYPE_HUID,
                                     l_huid,
                                     ERRL_CALLOUT_PRIORITY_MED);

                    l_redundancy_lost_callouts_count++;
                }
            } //iterate over all dimms
            if(G_log_gpe1_error)
            {
                // Going to be resetting so no reason to check anymore ports
                break;
            }
        } //iterate over all ports

        if(l_err)
        {
            commitErrl(&l_err);
        }

        if(l_redundancy_lost_err)
        {
            commitErrl(&l_redundancy_lost_err);
        }

        //skip clearing if no dimms need it
        if( (!l_need_clr.dw[0]) && (!l_need_clr.dw[1]) )
        {
            break;
        }

        //iterate across all membufs/ports clearing dimm sensor timers as needed
        for(l_membuf = 0; l_membuf < MAX_NUM_OCMBS; l_membuf++)
        {

            if(!l_need_clr.bytes[l_membuf])
            {
                continue;
            }

            //iterate over all dimms
            for(l_dimm = 0; l_dimm < NUM_DIMMS_PER_OCMB; l_dimm++)
            {
                //not this one, go to next one
                if(!(l_need_clr.bytes[l_membuf] & (DIMM_SENSOR0 >> l_dimm)))
                {
                    continue;
                }

                //we found one.
                l_fru = &g_amec->proc[0].memctl[l_membuf].membuf.dimm_temps[l_dimm];

                //clear timer
                l_fru->sample_age = 0;

                // Trace recovery
                if(L_trace_resume.bytes[l_membuf] & (DIMM_SENSOR0 >> l_dimm))
                {
                    L_trace_resume.bytes[l_membuf] &= ~(DIMM_SENSOR0 >> l_dimm);
                    TRAC_INFO("DIMM temperature collection has resumed for DIMM%04X temp[%d]",
                              (l_membuf<<8)|l_dimm, l_fru->cur_temp);
                }

            }//iterate over all dimms
        }//iterate over all membufs/ports
    }while(0);

    // For OCM the "DIMM" dts are used for different types.  Need to determine what type the
    // "DIMM" DTS readings are for so the control loop will handle timeout based on correct type
    if(G_dimm_temp_expired_bitmap.dw[0] || G_dimm_temp_expired_bitmap.dw[1])
    {
        // at least one sensor expired.  Set type for each expired sensor
        //iterate across all OCMBs
        for(l_membuf = 0; l_membuf < l_max_membuf; l_membuf++)
        {
            //iterate over all "dimm" DTS readings
            for(l_dimm = 0; l_dimm < l_max_dimm_per_membuf; l_dimm++)
            {
                if(G_dimm_temp_expired_bitmap.bytes[l_membuf] & (DIMM_SENSOR0 >> l_dimm))
                {
                    // found an expired sensor
                    l_ocm_dts_type_expired_bitmap |=
                        g_amec->proc[0].memctl[l_membuf].membuf.dimm_temps[l_dimm].dts_type_mask;
                }
            }//iterate over all dimms
        }//iterate over all OCMBs
    } // if temp expired

    // check if there is a change to any type expired
    if(G_ocmb_dts_type_expired_bitmap != l_ocm_dts_type_expired_bitmap)
    {
        TRAC_INFO("DIMM DTS type expired bitmap changed from[0x%04X] to[0x%04X]",
                  G_ocmb_dts_type_expired_bitmap, l_ocm_dts_type_expired_bitmap);
        G_ocmb_dts_type_expired_bitmap = l_ocm_dts_type_expired_bitmap;
    }

} // end amec_health_check_dimm_timeout()



/*
 * Function Specification
 *
 * Name: amec_health_check_membuf_temp
 *
 * Description: Check if the membuf's dimm chips temperature exceeds the error
 *               temperature as defined in thermal control thresholds
 *              (ERROR field for membuf FRU Type)
 *
 * End Function Specification
 */
void amec_health_check_membuf_temp()
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    uint16_t                    l_ot_error, l_cur_temp, l_max_temp;
    sensor_t                    *l_sensor;
    uint32_t                    l_membuf;
    uint32_t                    l_callouts_count = 0;
    uint16_t                    l_new_callouts;
    uint64_t                    l_huid;
    errlHndl_t                  l_err = NULL;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    // Check to see if any membufs have reached the error temperature that
    // haven't been called out already
    l_new_callouts = G_membuf_overtemp_bitmap ^ G_membuf_overtemp_logged_bitmap;
    if(!l_new_callouts)
    {
        return;
    }

    l_ot_error = g_amec->thermalmembuf.ot_error;
    // Get hottest membuf sensor
    l_sensor = getSensorByGsid(TEMPMEMBUFTHRM);
    l_cur_temp = l_sensor->sample;
    l_max_temp = l_sensor->sample_max;
    TRAC_ERR("amec_health_check_membuf_temp: membuf reached error temp[%d]. current[%d], hist_max[%d], bitmap[0x%02X]",
             l_ot_error,
             l_cur_temp,
             l_max_temp,
             l_new_callouts);

    //find the membuf(s) that need to be called out
    for(l_membuf = 0; l_membuf < MAX_NUM_OCMBS; l_membuf++)
    {
        if(!(l_new_callouts & (MEMBUF0_PRESENT_MASK >> l_membuf)))
        {
            continue;
        }

        l_huid = G_sysConfigData.membuf_huids[l_membuf];

        amec_mem_mark_logged(l_membuf,
                             0xff,
                             &G_membuf_overtemp_logged_bitmap,
                             &G_dimm_overtemp_logged_bitmap.bytes[l_membuf]);

        //If we don't have an error log for the callout, create one
        if(!l_err)
        {
            /* @
             * @errortype
             * @moduleid    AMEC_HEALTH_CHECK_MEMBUF_TEMP
             * @reasoncode  MEMBUF_ERROR_TEMP
             * @userdata1   Maximum membuf temperature
             * @userdata2   membuf temperature threshold
             * @userdata4   OCC_NO_EXTENDED_RC
             * @devdesc     memory controller(s) exceeded maximum safe
             *              temperature.
             */
            l_err = createErrl(AMEC_HEALTH_CHECK_MEMBUF_TEMP,  //modId
                               MEMBUF_ERROR_TEMP,              //reasoncode
                               OCC_NO_EXTENDED_RC,             //Extended reason code
                               ERRL_SEV_PREDICTIVE,            //Severity
                               NULL,                           //Trace Buf
                               DEFAULT_TRACE_SIZE,             //Trace Size
                               l_max_temp,                     //userdata1
                               l_ot_error);                    //userdata2

            // Callout the "over temperature" procedure
            addCalloutToErrl(l_err,
                             ERRL_CALLOUT_TYPE_COMPONENT_ID,
                             ERRL_COMPONENT_ID_OVER_TEMPERATURE,
                             ERRL_CALLOUT_PRIORITY_HIGH);
            l_callouts_count = 1;
        }

        // Callout membuf
        addCalloutToErrl(l_err,
                         ERRL_CALLOUT_TYPE_HUID,
                         l_huid,
                         ERRL_CALLOUT_PRIORITY_MED);

        l_callouts_count++;

        //If we've reached the max # of callouts for an error log
        //commit the error log
        if(l_callouts_count == ERRL_MAX_CALLOUTS)
        {
            commitErrl(&l_err);
        }

    }//iterate over membufs

    if(l_err)
    {
        commitErrl(&l_err);
    }
}

/*
 * Function Specification
 *
 * Name: amec_health_check_membuf_timeout
 *
 * Description: Check for membuf timeout condition
 *              as defined in thermal control thresholds
 *              (MAX_READ_TIMEOUT field for membuf FRU Type)
 *
 * End Function Specification
 */
void amec_health_check_membuf_timeout()
{
    static uint16_t L_temp_update_bitmap_prev = 0;
    uint16_t l_need_inc, l_need_clr, l_temp_update_bitmap;
    uint16_t l_membuf;
    fru_temp_t* l_fru;
    errlHndl_t  l_err = NULL;
    uint32_t    l_callouts_count = 0;
    uint64_t    l_huid;
    static uint16_t L_trace_resume = 0;
    ERRL_SEVERITY l_severity = ERRL_SEV_PREDICTIVE;

    do
    {
        //For every membuf sensor there are 3 cases to consider
        //
        //1) membuf is present and not updated (need to increment timer and check for timeout)
        //2) membuf is present and updated but wasn't updated on previous check (need to clear timer)
        //3) membuf is present and updated and was updated on previous check (do nothing)

        //Grab snapshot of G_membuf_temp_update_bitmap and clear it
        l_temp_update_bitmap = G_membuf_temp_updated_bitmap;
        G_membuf_temp_updated_bitmap = 0;

        //check if we need to increment any timers
        l_need_inc = G_membuf_dts_enabled & ~l_temp_update_bitmap;

        //check if we need to clear any timers
        l_need_clr = l_temp_update_bitmap & ~L_temp_update_bitmap_prev;

        //only go further if we actually have work to do here.
        if(!l_need_inc && !l_need_clr)
        {
            //nothing to do
            break;
        }

        //save off the previous bitmap of updated sensors
        L_temp_update_bitmap_prev = l_temp_update_bitmap;

        //iterate across all membufs incrementing timers as needed
        for(l_membuf = 0; l_membuf < MAX_NUM_OCMBS; l_membuf++)
        {
            //does this membuf timer need incrementing?
            if(!(l_need_inc & (MEMBUF0_PRESENT_MASK >> l_membuf)))
            {
                //temperature was updated for this membuf. Clear the timeout bit for this membuf.
                if(G_membuf_temp_expired_bitmap & (MEMBUF0_PRESENT_MASK >> l_membuf))
                {
                    G_membuf_temp_expired_bitmap &= ~(MEMBUF0_PRESENT_MASK >> l_membuf);
                    TRAC_INFO("membuf %d temps have been updated", l_membuf);
                }
                continue;
            }

            //This membuf requires an increment
            l_fru = &g_amec->proc[0].memctl[l_membuf].membuf.membuf_hottest;

            //increment timer
            l_fru->sample_age++;

            //handle wrapping
            if(!l_fru->sample_age)
            {
                l_fru->sample_age = -1;
            }

            //info trace each transition to not having a new temperature
            if(l_fru->sample_age == 2)
            {
                TRAC_INFO("Failed to read membuf temperature on membuf[%d] temp[%d] flags[0x%02X]",
                              l_membuf, l_fru->cur_temp, l_fru->flags);

                // set flag to trace if this ever recovers
                L_trace_resume |= (MEMBUF0_PRESENT_MASK >> l_membuf);
            }

            //check if the temperature reading is still useable
            if(g_amec->thermalmembuf.temp_timeout == 0xff ||
               l_fru->sample_age < g_amec->thermalmembuf.temp_timeout)
            {
                continue;
            }

            //temperature has expired.  Notify control algorithms which membuf.
            if(!(G_membuf_temp_expired_bitmap & (MEMBUF0_PRESENT_MASK >> l_membuf)))
            {
                G_membuf_temp_expired_bitmap |= MEMBUF0_PRESENT_MASK >> l_membuf;
                TRAC_ERR("Timed out reading membuf temperature sensor on membuf %d",
                         l_membuf);
            }

            //If we've already logged an error for this FRU go to the next one.
            if(G_membuf_timeout_logged_bitmap & (MEMBUF0_PRESENT_MASK >> l_membuf))
            {
                continue;
            }

            // To prevent membufs from incorrectly being called out, don't log errors if there have
            // been timeouts with GPE1 tasks not finishing
            if(G_error_history[ERRH_GPE1_NOT_IDLE] > g_amec->thermalmembuf.temp_timeout)
            {
                TRAC_ERR("Timed out reading membuf temperature due to GPE1 issues");
                // give notification that GPE1 error should now be logged which will reset the OCC
                G_log_gpe1_error = TRUE;
                // no reason to check anymore since all membufs are collected from the same GPE
                break;
            }

            TRAC_ERR("Timed out reading membuf temperature on membuf[%d] temp[%d] flags[0x%02X]",
                     l_membuf, l_fru->cur_temp, l_fru->flags);

            if(!l_err)
            {
                // make this info only in simics
                if(G_simics_environment)
                     l_severity = ERRL_SEV_INFORMATIONAL;

                /* @
                 * @errortype
                 * @moduleid    AMEC_HEALTH_CHECK_MEMBUF_TIMEOUT
                 * @reasoncode  FRU_TEMP_TIMEOUT
                 * @userdata1   timeout value in seconds
                 * @userdata2   0
                 * @userdata4   ERC_AMEC_MEMBUF_TEMP_TIMEOUT
                 * @devdesc     Failed to read a membuf memory controller
                 *              temperature
                 *
                 */
                l_err = createErrl(AMEC_HEALTH_CHECK_MEMBUF_TIMEOUT,  //modId
                                   FRU_TEMP_TIMEOUT,                  //reasoncode
                                   ERC_AMEC_MEMBUF_TEMP_TIMEOUT,      //Extended reason code
                                   l_severity,                        //Severity
                                   NULL,                              //Trace Buf
                                   DEFAULT_TRACE_SIZE,                //Trace Size
                                   g_amec->thermalmembuf.temp_timeout,  //userdata1
                                   0);                                //userdata2

                l_callouts_count = 0;
            }

            //Get the HUID for the membuf
            l_huid = G_sysConfigData.membuf_huids[l_membuf];

            // Callout membuf
            addCalloutToErrl(l_err,
                             ERRL_CALLOUT_TYPE_HUID,
                             l_huid,
                             ERRL_CALLOUT_PRIORITY_MED);

            l_callouts_count++;

            //If we've reached the max # of callouts for an error log
            //commit the error log
            if(l_callouts_count == ERRL_MAX_CALLOUTS)
            {
                commitErrl(&l_err);
            }

            //Mark membuf as logged so we don't log it more than once
            amec_mem_mark_logged(l_membuf,
                                 0xff,
                                 &G_membuf_timeout_logged_bitmap,
                                 &G_dimm_timeout_logged_bitmap.bytes[l_membuf]);
        } //iterate over all membufs

        if(l_err)
        {
            commitErrl(&l_err);
        }

        //skip clearing timers if no membufs need it
        if(!l_need_clr)
        {
            break;
        }

        //iterate across all membufs clearing timers as needed
        for(l_membuf = 0; l_membuf < MAX_NUM_OCMBS; l_membuf++)
        {
            //not this one, go to next one
            if(!(l_need_clr & (MEMBUF0_PRESENT_MASK >> l_membuf)))
            {
                continue;
            }

            //we found one.
            l_fru = &g_amec->proc[0].memctl[l_membuf].membuf.membuf_hottest;

            //clear timer
            l_fru->sample_age = 0;

            //info trace each time we recover
            if(L_trace_resume & (MEMBUF0_PRESENT_MASK >> l_membuf))
            {
                L_trace_resume &= ~(MEMBUF0_PRESENT_MASK >> l_membuf);

                TRAC_INFO("membuf temperature collection has resumed on membuf[%d] temp[%d]",
                          l_membuf, l_fru->cur_temp);
            }

        }//iterate over all membufs
    }while(0);
}


// Function Specification
//
// Name:  amec_health_check_proc_temp
//
// Description: This function checks if the proc temperature has
// exceeded the error temperature as define in data format 0x13.
//
// End Function Specification
void amec_health_check_proc_temp()
{
    #define NUM_PROC_SENSOR_TYPES 2
    #define CORE_DTS_INDEX        0
    #define IO_DTS_INDEX          1
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    uint16_t                    l_ot_error;
    uint16_t                    l_io_ot_error;
    uint16_t                    l_cur_temp;
    uint16_t                    l_cur_io_temp;
    uint8_t                     l_core_num;
    uint8_t                     l_quad;
    static uint32_t             L_error_count[NUM_PROC_SENSOR_TYPES] = {0};
    // history of core and IO temperatures for tracing when logging OT error
    static uint16_t             L_temp_history[AMEC_HEALTH_ERROR_TIMER][NUM_PROC_SENSOR_TYPES] = {{0}};
    static BOOLEAN              L_ot_error_logged = FALSE;
    sensor_t                    *l_sensor;
    sensor_t                    *l_io_sensor;
    sensor_t                    *l_temp_sensor;
    errlHndl_t                  l_err = NULL;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    // Only check for OT if we haven't already logged an OT error
    if (L_ot_error_logged == FALSE)
    {
        // Get TEMPPROCTHRM sensor, which is hottest core temperature
        // in OCC processor
        l_sensor = getSensorByGsid(TEMPPROCTHRM);
        l_cur_temp = l_sensor->sample;
        l_core_num = l_sensor->vector->max_pos;
        l_ot_error = g_amec->thermalproc.ot_error;

        // Get TEMPPROCIOTHRM sensor, which is hottest IO temperature
        // in OCC processor
        l_io_sensor = getSensorByGsid(TEMPPROCIOTHRM);
        l_cur_io_temp = l_io_sensor->sample;
        l_io_ot_error = g_amec->thermalprocio.ot_error;


        // Check to see if we exceeded our error temperature for the core
        if(l_cur_temp > l_ot_error)
        {
            // save the temperatures for tracing later
            L_temp_history[L_error_count[CORE_DTS_INDEX]][CORE_DTS_INDEX] = l_cur_temp;

            // Increment the error counter for this FRU
            L_error_count[CORE_DTS_INDEX]++;
        }
        else
        {
            // Reset the error counter for this FRU
            L_error_count[CORE_DTS_INDEX] = 0;
        }

        // Check to see if we exceeded our error temperature for IO
        if(l_cur_io_temp > l_io_ot_error)
        {
            // save the temperatures for tracing later
            L_temp_history[L_error_count[IO_DTS_INDEX]][IO_DTS_INDEX] = l_cur_io_temp;

            // Increment the error counter for this FRU
            L_error_count[IO_DTS_INDEX]++;
        }
        else
        {
            // Reset the error counter for this FRU
            L_error_count[IO_DTS_INDEX] = 0;
        }

        // Trace and log error the first time this occurs
        // same callouts for processor and processor IO so treat the same and only
        // log one error for either being OT
        if( (L_error_count[CORE_DTS_INDEX] == AMEC_HEALTH_ERROR_TIMER) ||
            (L_error_count[IO_DTS_INDEX] == AMEC_HEALTH_ERROR_TIMER) )
        {
            L_ot_error_logged = TRUE;

            if(L_error_count[CORE_DTS_INDEX] == AMEC_HEALTH_ERROR_TIMER)
            {
                TRAC_ERR("amec_health_check_error_temp: processor has exceeded OT error! core[%u] is hottest current temp[%u] ot_error[%u]",
                         l_core_num,
                         l_cur_temp,
                         l_ot_error);
                TRAC_ERR("amec_health_check_error_temp: previous 4 readings [%u] [%u] [%u] [%u]",
                         L_temp_history[3][CORE_DTS_INDEX], L_temp_history[2][CORE_DTS_INDEX],
                         L_temp_history[1][CORE_DTS_INDEX], L_temp_history[0][CORE_DTS_INDEX]);

                // trace the individual temps used to calculate the current hottest core temp
                // non-weighted avg of 2 core DTS and 1 L3 DTS
                l_temp_sensor = getSensorByGsid((uint16_t)(TEMPC0 + l_core_num));
                TRAC_ERR("amec_health_check_error_temp: Avg core and L3 DTS Current temp[%u] max temp[%u]",
                         l_temp_sensor->sample, l_temp_sensor->sample_max);

                // non-weighted racetrack DTS
                // Determine the quad this core resides in.
                l_quad = l_core_num / 4;
                l_temp_sensor = getSensorByGsid((uint16_t)(TEMPQ0 + l_quad));
                TRAC_ERR("amec_health_check_error_temp: Quad[%u] Racetrack current temp[%u] max temp[%u]",
                         l_quad, l_temp_sensor->sample, l_temp_sensor->sample_max);
            }
            if(L_error_count[IO_DTS_INDEX] == AMEC_HEALTH_ERROR_TIMER)
            {
                TRAC_ERR("amec_health_check_error_temp: processor IO has exceeded OT error! current max IO temp[%u] ot_error[%u]",
                         l_cur_io_temp,
                         l_io_ot_error);
                TRAC_ERR("amec_health_check_error_temp: previous 4 IO readings [%u] [%u] [%u] [%u]",
                         L_temp_history[3][IO_DTS_INDEX], L_temp_history[2][IO_DTS_INDEX],
                         L_temp_history[1][IO_DTS_INDEX], L_temp_history[0][IO_DTS_INDEX]);

                // trace the 4 individual IO temps so it is known which one is hot
                l_temp_sensor = getSensorByGsid(TEMPPROCIO00);
                TRAC_ERR("amec_health_check_error_temp: SE PAU Current temp[%u] max temp[%u]",
                         l_temp_sensor->sample, l_temp_sensor->sample_max);
                l_temp_sensor = getSensorByGsid(TEMPPROCIO01);
                TRAC_ERR("amec_health_check_error_temp: NE PAU Current temp[%u] max temp[%u]",
                         l_temp_sensor->sample, l_temp_sensor->sample_max);
                l_temp_sensor = getSensorByGsid(TEMPPROCIO10);
                TRAC_ERR("amec_health_check_error_temp: SW PAU Current temp[%u] max temp[%u]",
                         l_temp_sensor->sample, l_temp_sensor->sample_max);
                l_temp_sensor = getSensorByGsid(TEMPPROCIO11);
                TRAC_ERR("amec_health_check_error_temp: NW PAU Current temp[%u] max temp[%u]",
                         l_temp_sensor->sample, l_temp_sensor->sample_max);
            }

            // Log an OT error
            /* @
             * @errortype
             * @moduleid    AMEC_HEALTH_CHECK_PROC_TEMP
             * @reasoncode  PROC_ERROR_TEMP
             * @userdata1   Fru peak procesor IO temperature sensor
             * @userdata2   Fru peak procesor temperature sensor
             * @devdesc     Processor FRU has reached error temperature
             *              threshold and is called out in this error log.
             *
             */
            l_err = createErrl(AMEC_HEALTH_CHECK_PROC_TEMP,
                               PROC_ERROR_TEMP,
                               ERC_AMEC_PROC_ERROR_OVER_TEMPERATURE,
                               ERRL_SEV_PREDICTIVE,
                               NULL,
                               DEFAULT_TRACE_SIZE,
                               l_io_sensor->sample_max,
                               l_sensor->sample_max);

            // Callout the Ambient procedure
            addCalloutToErrl(l_err,
                             ERRL_CALLOUT_TYPE_COMPONENT_ID,
                             ERRL_COMPONENT_ID_OVER_TEMPERATURE,
                             ERRL_CALLOUT_PRIORITY_HIGH);
            // Callout to processor
            addCalloutToErrl(l_err,
                             ERRL_CALLOUT_TYPE_HUID,
                             G_sysConfigData.proc_huid,
                             ERRL_CALLOUT_PRIORITY_MED);

            // Commit Error
            commitErrl(&l_err);
        }

    } // if OT error not logged
}

// Function Specification
//
// Name:  amec_health_check_proc_temp_timeout
//
// Description: This function checks if OCC has failed to read the processor
// temperature and if it has exceeded the maximum allowed number of retries.
//
// End Function Specification
void amec_health_check_proc_timeout()
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    errlHndl_t                  l_err = NULL;
    sensor_t                    *l_sensor = NULL;
    BOOLEAN                     l_core_fail_detected = FALSE;
    static uint32_t             L_read_fail_cnt = 0;
    uint8_t                     i = 0;
    uint8_t                     l_bad_core_index = 0;
    CoreData                    *l_core_data_ptr = NULL;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    do
    {
        for(i=0; i<MAX_NUM_CORES; i++)
        {
            if(!CORE_PRESENT(i) || CORE_OFFLINE(i))
            {
                // If this core is not present, move on
                continue;
            }

            // Check if this core's temperature sensor has been updated
            l_sensor = AMECSENSOR_ARRAY_PTR(TEMPPROCTHRMC0,i);
            if (l_sensor->update_tag == G_core_temp_update_tag[i])
            {
                // If the update tag is not changing, then this core's
                // temperature sensor is not being updated.
                l_core_fail_detected = TRUE;
                l_bad_core_index = i;
            }

            // Take a snapshot of the update tag
            G_core_temp_update_tag[i] = l_sensor->update_tag;
        }

        // Have we found at least one core that has reading failures?
        if(!l_core_fail_detected)
        {
            // We were able to read all cores' temperature sensors so clear our
            // counter
            L_read_fail_cnt = 0;
        }
        else
        {
            // We've failed to read a core's temperature sensor so increment
            // our counter
            L_read_fail_cnt++;

            // Check if we have reached the maximum read time allowed
            if((L_read_fail_cnt == g_amec->thermalproc.temp_timeout) &&
               (g_amec->thermalproc.temp_timeout != 0xFF))
            {
                TRAC_ERR("Timed out reading processor temperature on core_index[%u] (core ipc min[%u] max[%u])",
                         l_bad_core_index, AMECSENSOR_PTR(CORE_IPCdur)->sample_min, AMECSENSOR_PTR(CORE_IPCdur)->sample_max);

                // Get pointer to core data
                l_core_data_ptr = proc_get_bulk_core_data_ptr(l_bad_core_index);


                TRAC_ERR("Core Sensors[0x%04X%04X] L3Cache Sensor[0x%04X] Racetrack Sensor [0x%04X]",
                         (uint16_t)(l_core_data_ptr->dts.core[0].result ),
                         (uint16_t)(l_core_data_ptr->dts.core[1].result ),
                         (uint16_t)(l_core_data_ptr->dts.cache.result),
                         (uint16_t)(l_core_data_ptr->dts.racetrack.result));

                /* @
                 * @errortype
                 * @moduleid    AMEC_HEALTH_CHECK_PROC_TIMEOUT
                 * @reasoncode  PROC_TEMP_TIMEOUT
                 * @userdata1   timeout value in seconds
                 * @userdata2   0
                 * @userdata4   OCC_NO_EXTENDED_RC
                 * @devdesc     Failed to read processor temperature.
                 *
                 */
                l_err = createErrl(AMEC_HEALTH_CHECK_PROC_TIMEOUT,    //modId
                                   PROC_TEMP_TIMEOUT,                 //reasoncode
                                   OCC_NO_EXTENDED_RC,                //Extended reason code
                                   ERRL_SEV_PREDICTIVE,               //Severity
                                   NULL,                              //Trace Buf
                                   DEFAULT_TRACE_SIZE,                //Trace Size
                                   g_amec->thermalproc.temp_timeout,  //userdata1
                                   0);                                //userdata2

                // Callout the processor
                addCalloutToErrl(l_err,
                                 ERRL_CALLOUT_TYPE_HUID,
                                 G_sysConfigData.proc_huid,
                                 ERRL_CALLOUT_PRIORITY_MED);

                // Commit error log and request reset
                REQUEST_RESET(l_err);
            }
        }
    }while(0);
}

// Function Specification
//
// Name:  amec_health_check_vrm_vdd_temp
//
// Description: This function checks if the VRM Vdd temperature has
// exceeded the error temperature sent in data format 0x13.
//
// End Function Specification
void amec_health_check_vrm_vdd_temp(const sensor_t *i_sensor)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    uint16_t                    l_ot_error;
    static uint32_t             L_error_count = 0;
    static BOOLEAN              L_ot_error_logged = FALSE;
    errlHndl_t                  l_err = NULL;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    do
    {
        l_ot_error = g_amec->thermalvdd.ot_error;

        static bool L_logged_invalid = false;
        if (i_sensor->sample > 255)
        {
            if (!L_logged_invalid)
            {
                TRAC_ERR("amec_health_check_vrm_vdd_temp: VRM vdd temp was out of range: temp[%u] ot_error[%u]",
                         i_sensor->sample,
                         l_ot_error);
                L_logged_invalid = true;
            }
        }
        else
        {
            L_logged_invalid = false;
        }

        // Check to see if we exceeded our error temperature
        if ((l_ot_error != 0) && (i_sensor->sample > l_ot_error))
        {
            // Increment the error counter for this FRU
            L_error_count++;

            // Trace and log error the first time this occurs
            if (L_error_count == AMEC_HEALTH_ERROR_TIMER)
            {
                // Have we logged an OT error for this FRU already?
                if (L_ot_error_logged == TRUE)
                {
                    break;
                }

                L_ot_error_logged = TRUE;

                TRAC_ERR("amec_health_check_vrm_vdd_temp: VRM vdd has exceeded OT error! temp[%u] ot_error[%u]",
                         i_sensor->sample,
                         l_ot_error);

                // Log an OT error
                /* @
                 * @errortype
                 * @moduleid    AMEC_HEALTH_CHECK_VRM_VDD_TEMP
                 * @reasoncode  VRM_VDD_ERROR_TEMP
                 * @userdata1   0
                 * @userdata2   Fru peak temperature sensor
                 * @devdesc     VRM Vdd has reached error temperature
                 *              threshold and is called out in this error log.
                 *
                 */
                l_err = createErrl(AMEC_HEALTH_CHECK_VRM_VDD_TEMP,
                                   VRM_VDD_ERROR_TEMP,
                                   ERC_AMEC_PROC_ERROR_OVER_TEMPERATURE,
                                   ERRL_SEV_PREDICTIVE,
                                   NULL,
                                   DEFAULT_TRACE_SIZE,
                                   0,
                                   i_sensor->sample_max);

                // Callout the Ambient procedure
                addCalloutToErrl(l_err,
                                 ERRL_CALLOUT_TYPE_COMPONENT_ID,
                                 ERRL_COMPONENT_ID_OVER_TEMPERATURE,
                                 ERRL_CALLOUT_PRIORITY_HIGH);

                // Callout VRM Vdd
                addCalloutToErrl(l_err,
                                 ERRL_CALLOUT_TYPE_HUID,
                                 G_sysConfigData.vrm_vdd_huid,
                                 ERRL_CALLOUT_PRIORITY_MED);

                // Commit Error
                commitErrl(&l_err);
            }
        }
        else
        {
            // Trace that we have now dropped below the error threshold
            if (L_error_count >= AMEC_HEALTH_ERROR_TIMER)
            {
                TRAC_INFO("amec_health_check_vrm_vdd_temp: VRM Vdd temp [%u] now below error temp [%u] after error_count [%u]",
                          i_sensor->sample, l_ot_error, L_error_count);
            }

            // Reset the error counter for this FRU
            L_error_count = 0;
        }
    }while (0);

}

// Function Specification
//
// Name:  amec_health_check_vrm_vdd_temp_timeout
//
// Description: This function checks if OCC has failed to read the VRM Vdd
// temperature and if it has exceeded the maximum allowed number of retries.
//
// End Function Specification
void amec_health_check_vrm_vdd_temp_timeout()
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    errlHndl_t                  l_err = NULL;
    uint32_t                    l_update_tag = 0;
    static uint32_t             L_read_fail_cnt = 0;
    static BOOLEAN              L_error_logged = FALSE;
    static uint32_t             L_vdd_temp_update_tag = 0;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    // Check if VRM Vdd temperature sensor has been updated by checking the sensor update tag
    // If the update tag is not changing, then temperature sensor is not being updated.
    l_update_tag = AMECSENSOR_PTR(TEMPVDD)->update_tag;
    if (l_update_tag != L_vdd_temp_update_tag)
    {
        // We were able to read VRM Vdd temperature
        L_read_fail_cnt = 0;
        G_vrm_vdd_temp_expired = false;
        L_vdd_temp_update_tag = l_update_tag;
    }
    else
    {
        // Failed to read VRM Vdd temperature sensor
        L_read_fail_cnt++;

        // Check if we have reached the maximum read time allowed
        if((L_read_fail_cnt == g_amec->thermalvdd.temp_timeout) &&
           (g_amec->thermalvdd.temp_timeout != 0xFF))
        {
            //temperature has expired.  Notify control algorithms
            G_vrm_vdd_temp_expired = true;

            // Log error one time
            if (L_error_logged == FALSE)
            {
                L_error_logged = TRUE;

                TRAC_ERR("Timed out reading VRM Vdd temperature for timeout[%u]",
                          g_amec->thermalvdd.temp_timeout);

                /* @
                 * @errortype
                 * @moduleid    AMEC_HEALTH_CHECK_VRM_VDD_TIMEOUT
                 * @reasoncode  FRU_TEMP_TIMEOUT
                 * @userdata1   timeout value in seconds
                 * @userdata2   0
                 * @userdata4   ERC_AMEC_VRM_VDD_TEMP_TIMEOUT
                 * @devdesc     Failed to read VRM Vdd temperature.
                 *
                 */
                l_err = createErrl(AMEC_HEALTH_CHECK_VRM_VDD_TIMEOUT, //modId
                                   FRU_TEMP_TIMEOUT,                  //reasoncode
                                   ERC_AMEC_VRM_VDD_TEMP_TIMEOUT,     //Extended reason code
                                   ERRL_SEV_PREDICTIVE,               //Severity
                                   NULL,                              //Trace Buf
                                   DEFAULT_TRACE_SIZE,                //Trace Size
                                   g_amec->thermalvdd.temp_timeout,   //userdata1
                                   0);                                //userdata2

                // Callout the VRM
                addCalloutToErrl(l_err,
                                 ERRL_CALLOUT_TYPE_HUID,
                                 G_sysConfigData.vrm_vdd_huid,
                                 ERRL_CALLOUT_PRIORITY_MED);

                // Commit error log and request reset
                REQUEST_RESET(l_err);
            }
        } // if reached timeout
    } // else failed to read temp
}

/*----------------------------------------------------------------------------*/
/* End                                                                        */
/*----------------------------------------------------------------------------*/
