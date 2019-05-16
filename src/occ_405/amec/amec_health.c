/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/amec/amec_health.c $                              */
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
dimm_sensor_flags_t G_dimm_overtemp_logged_bitmap = {0};

// Have we already called out the dimm for timeout (bitmap of dimms)?
dimm_sensor_flags_t G_dimm_timeout_logged_bitmap = {0};

// Are any dimms currently in the timedout state (bitmap of dimm)?
dimm_sensor_flags_t G_dimm_temp_expired_bitmap = {0};

// Have we already called out the membuf for timeout (bitmap of membufs)
uint8_t G_membuf_timeout_logged_bitmap = 0;

// Have we already called out the membuf for overtemp (bitmap of membufs)
uint8_t G_membuf_overtemp_logged_bitmap = 0;

// Are any dimms currently in the timedout state (bitmap of membufs)
uint8_t G_membuf_temp_expired_bitmap = 0;

// Array to store the update tag of each core's temperature sensor
uint32_t G_core_temp_update_tag[MAX_NUM_CORES] = {0};

// Reading VRM Vdd temperature timedout?
bool G_vrm_vdd_temp_expired = false;

//*************************************************************************/
// Function Declarations
//*************************************************************************/

uint64_t amec_mem_get_huid(uint8_t i_membuf, uint8_t i_dimm)
{
    return 0;
}


// If i_dimm is 0xff it is assumed that the caller wishes to
// mark the membuf as being logged.  Otherwise, it is assumed
// that the dimm should be marked.
void amec_mem_mark_logged(uint8_t i_membuf,
                          uint8_t i_dimm,
                          uint8_t* i_clog_bitmap,
                          uint8_t* i_dlog_bitmap)
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
    uint16_t                    l_ot_error, l_cur_temp, l_max_temp;
    sensor_t                    *l_sensor;
    uint8_t                     l_dimm;
    uint8_t                     l_port;
    uint32_t                    l_callouts_count = 0;
    uint8_t                     l_new_callouts;
    uint64_t                    l_huid;
    errlHndl_t                  l_err = NULL;

    // Check to see if any dimms have reached the error temperature that
    // haven't been called out already
    if(G_dimm_overtemp_bitmap.bigword == G_dimm_overtemp_logged_bitmap.bigword)
    {
        return;
    }

    l_ot_error = g_amec->thermaldimm.ot_error;
    l_sensor = getSensorByGsid(TEMPDIMMTHRM);
    l_cur_temp = l_sensor->sample;
    l_max_temp = l_sensor->sample_max;

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

        TRAC_ERR("amec_health_check_dimm_temp: DIMM reached error temp[%d]. current[%d], hist_max[%d], port[%d]",
                 l_ot_error,
                 l_cur_temp,
                 l_max_temp,
                 l_port);

        //find the dimm(s) that need to be called out for this port
        for(l_dimm = 0; l_dimm < NUM_DIMMS_PER_OCMB; l_dimm++)
        {
            if (!(l_new_callouts & (DIMM_SENSOR0 >> l_dimm)))
            {
                continue;
            }

            fru_temp_t* l_fru;
            l_fru = &g_amec->proc[0].memctl[l_port].membuf.dimm_temps[l_dimm];
            amec_mem_mark_logged(l_port,
                                 l_dimm,
                                 &G_membuf_overtemp_logged_bitmap,
                                 &G_dimm_overtemp_logged_bitmap.bytes[l_port]);
            TRAC_ERR("amec_health_check_dimm_temp: DIMM%04X overtemp - %dC",
                     (l_port<<8)|l_dimm, l_fru->cur_temp);

            // Create single elog with up to MAX_CALLOUTS
            if(l_callouts_count < ERRL_MAX_CALLOUTS)
            {
                //If we don't have an error log for the callout, create one
                if(!l_err)
                {
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
    static dimm_sensor_flags_t L_temp_update_bitmap_prev = {0};
    dimm_sensor_flags_t l_need_inc, l_need_clr, l_temp_update_bitmap;
    uint8_t l_dimm, l_port;
    fru_temp_t* l_fru;
    errlHndl_t  l_err = NULL;
    uint32_t    l_callouts_count = 0;
    uint64_t    l_huid;
    static bool L_ran_once = FALSE;

    do
    {
        //For every dimm sensor there are 3 cases to consider
        //
        //1) sensor is enabled and not updated (need to increment timer and check for timeout)
        //2) sensor is enabled and updated but wasn't updated on previous check (need to clear timer)
        //3) sensor is enabled and updated and was updated on previous check (do nothing)

        //Grab snapshot of G_dimm_temp_updated_bitmap and clear it
        l_temp_update_bitmap.bigword = G_dimm_temp_updated_bitmap.bigword;
        G_dimm_temp_updated_bitmap.bigword = 0;

        //check if we need to increment any timers (haven't been updated in the last second)
        l_need_inc.bigword = G_dimm_enabled_sensors.bigword & ~l_temp_update_bitmap.bigword;

        //check if we need to clear any timers (updated now but not updated previously)
        l_need_clr.bigword = l_temp_update_bitmap.bigword & ~L_temp_update_bitmap_prev.bigword;

        //save off the previous bitmap of updated sensors for next time
        L_temp_update_bitmap_prev.bigword = l_temp_update_bitmap.bigword;

        //only go further if we actually have work to do here.
        if(!l_need_inc.bigword && !l_need_clr.bigword)
        {
            //nothing to do
            break;
        }

        //iterate across all ports incrementing dimm sensor timers as needed
        for(l_port = 0; l_port < MAX_NUM_OCMBS; l_port++)
        {
            //any dimm timers on this port need incrementing?
            if(!l_need_inc.bytes[l_port])
            {
                // All dimm sensors were updated for this port
                // Trace this fact and clear the expired byte for all DIMMs on this port
                if(G_dimm_temp_expired_bitmap.bytes[l_port])
                {
                    G_dimm_temp_expired_bitmap.bytes[l_port] = 0;
                    TRAC_INFO("All DIMM sensors for port %d have been updated", l_port);
                }
                continue;
            }

            //There's at least one dimm requiring an increment, find the dimm
            for(l_dimm = 0; l_dimm < NUM_DIMMS_PER_OCMB; l_dimm++)
            {
                //not this one, check if we need to clear the dimm timeout and go to the next one
                if(!(l_need_inc.bytes[l_port] & (DIMM_SENSOR0 >> l_dimm)))
                {
                    // Clear this one if needed
                    if(G_dimm_temp_expired_bitmap.bytes[l_port] & (DIMM_SENSOR0 >> l_dimm))
                    {
                        G_dimm_temp_expired_bitmap.bytes[l_port] &= ~(DIMM_SENSOR0 >> l_dimm);
                    }
                    continue;
                }

                //we found one.
                l_fru = &g_amec->proc[0].memctl[l_port].membuf.dimm_temps[l_dimm];

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
                if((l_fru->sample_age == 1) && (!G_simics_environment))
                {
                    TRAC_INFO("No new DIMM temperature available for DIMM%04X (cur_temp[%d] flags[0x%02X])",
                              (l_port<<8)|l_dimm, l_fru->cur_temp, l_fru->flags);
                }

                //check if the temperature reading is still useable
                if(g_amec->thermaldimm.temp_timeout == 0xff ||
                   l_fru->sample_age < g_amec->thermaldimm.temp_timeout)
                {
                    continue;
                }

                //temperature has expired.  Notify control algorithms which DIMM
                if(!(G_dimm_temp_expired_bitmap.bytes[l_port] & (DIMM_SENSOR0 >> l_dimm)))
                {
                    G_dimm_temp_expired_bitmap.bytes[l_port] |= (DIMM_SENSOR0 >> l_dimm);
                    TRAC_ERR("Timed out reading DIMM%04X temperature sensor", (l_port<<8)|l_dimm);
                }

                //If we've already logged an error for this FRU go to the next one.
                if(G_dimm_timeout_logged_bitmap.bytes[l_port] & (DIMM_SENSOR0 >> l_dimm))
                {
                    continue;
                }

                // To prevent DIMMs from incorrectly being called out, don't log errors if there have
                // been timeouts with GPE1 tasks not finishing
                if(G_error_history[ERRH_GPE1_NOT_IDLE] > g_amec->thermaldimm.temp_timeout)
                {
                    TRAC_ERR("Timed out reading DIMM temperature due to GPE1 issues");
                    // give notification that GPE1 error should now be logged which will reset the OCC
                    G_log_gpe1_error = TRUE;
                    // no reason to check anymore since all DIMMs are collected from the same GPE
                    break;
                }

                TRAC_ERR("Timed out reading DIMM%04X temperature (cur_temp[%d] flags[0x%02X])",
                         (l_port<<8)|l_dimm, l_fru->cur_temp, l_fru->flags);

                //Mark DIMM as logged so we don't log it more than once
                amec_mem_mark_logged(l_port,
                                     l_dimm,
                                     &G_membuf_timeout_logged_bitmap,
                                     &G_dimm_timeout_logged_bitmap.bytes[l_port]);

                // Create single elog with up to MAX_CALLOUTS
                if(l_callouts_count < ERRL_MAX_CALLOUTS)
                {
                    if(!l_err)
                    {
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
                                           ERRL_SEV_PREDICTIVE,               //Severity
                                           NULL,                              //Trace Buf
                                           DEFAULT_TRACE_SIZE,                //Trace Size
                                           g_amec->thermaldimm.temp_timeout,  //userdata1
                                           0);                                //userdata2
                    }

                    //Get the HUID for the DIMM and add callout
                    l_huid = amec_mem_get_huid(l_port, l_dimm);
                    addCalloutToErrl(l_err,
                                     ERRL_CALLOUT_TYPE_HUID,
                                     l_huid,
                                     ERRL_CALLOUT_PRIORITY_MED);

                    l_callouts_count++;
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

        //skip clearing if no dimms need it
        if(!l_need_clr.bigword)
        {
            break;
        }

        //iterate across all membufs/ports clearing dimm sensor timers as needed
        for(l_port = 0; l_port < MAX_NUM_OCMBS; l_port++)
        {

            if(!l_need_clr.bytes[l_port])
            {
                continue;
            }

            //iterate over all dimms
            for(l_dimm = 0; l_dimm < NUM_DIMMS_PER_OCMB; l_dimm++)
            {
                //not this one, go to next one
                if(!(l_need_clr.bytes[l_port] & (DIMM_SENSOR0 >> l_dimm)))
                {
                    continue;
                }

                //we found one.
                l_fru = &g_amec->proc[0].memctl[l_port].membuf.dimm_temps[l_dimm];

                //clear timer
                l_fru->sample_age = 0;

                // In Simics: the RTL timer is increased and a DIMM reading will not always
                // complete on each call.  Skip the "recovery" trace in Simics.
                if((L_ran_once) && (!G_simics_environment))
                {
                    TRAC_INFO("DIMM temperature collection has resumed for DIMM%04X temp[%d]",
                              (l_port<<8)|l_dimm, l_fru->cur_temp);
                }

            }//iterate over all dimms
        }//iterate over all membufs/ports
    }while(0);
    L_ran_once = TRUE;

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
    uint8_t                     l_new_callouts;
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
    static uint8_t L_temp_update_bitmap_prev = 0;
    uint8_t l_need_inc, l_need_clr, l_temp_update_bitmap;
    uint8_t l_membuf;
    fru_temp_t* l_fru;
    errlHndl_t  l_err = NULL;
    uint32_t    l_callouts_count = 0;
    uint64_t    l_huid;
    static bool L_ran_once = FALSE;

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
        l_need_inc = G_present_membufs & ~l_temp_update_bitmap;

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
            if(l_fru->sample_age == 1)
            {
                TRAC_INFO("Failed to read membuf temperature on membuf[%d] temp[%d] flags[0x%02X]",
                              l_membuf, l_fru->cur_temp, l_fru->flags);
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
                                   ERRL_SEV_PREDICTIVE,               //Severity
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
            if(L_ran_once)
            {
                TRAC_INFO("membuf temperature collection has resumed on membuf[%d] temp[%d]",
                          l_membuf, l_fru->cur_temp);
            }

        }//iterate over all membufs
    }while(0);
    L_ran_once = TRUE;
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
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    uint16_t                    l_ot_error;
    static uint32_t             L_error_count = 0;
    static BOOLEAN              L_ot_error_logged = FALSE;
    sensor_t                    *l_sensor;
    errlHndl_t                  l_err = NULL;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    do
    {
        // Get TEMPPROCTHRM sensor, which is hottest core temperature
        // in OCC processor
        l_sensor = getSensorByGsid(TEMPPROCTHRM);
        l_ot_error = g_amec->thermalproc.ot_error;

        // Check to see if we exceeded our error temperature
        if (l_sensor->sample > l_ot_error)
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

                TRAC_ERR("amec_health_check_error_temp: processor has exceeded OT error! temp[%u] ot_error[%u]",
                         l_sensor->sample,
                         l_ot_error);

                // Log an OT error
                /* @
                 * @errortype
                 * @moduleid    AMEC_HEALTH_CHECK_PROC_TEMP
                 * @reasoncode  PROC_ERROR_TEMP
                 * @userdata1   0
                 * @userdata2   Fru peak temperature sensor
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
                                   0,
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
        }
        else
        {
            // Trace that we have now dropped below the error threshold
            if (L_error_count >= AMEC_HEALTH_ERROR_TIMER)
            {
                TRAC_INFO("amec_health_check_proc_temp: We have dropped below error threshold for processors. error_count[%u]",
                          L_error_count);
            }

            // Reset the error counter for this FRU
            L_error_count = 0;
        }
    }while (0);

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
                TRAC_ERR("Timed out reading processor temperature on core_index[%u]",
                         l_bad_core_index);

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
