/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/amec/amec_health.c $                                  */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2014                        */
/* [+] Google Inc.                                                        */
/* [+] International Business Machines Corp.                              */
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

//*************************************************************************
// Includes
//*************************************************************************
#include "amec_health.h"
#include "amec_sys.h"
#include "amec_service_codes.h"
#include "occ_service_codes.h"
#include <centaur_data.h>
#include <thrm_thread.h>
#include <proc_data.h>

//*************************************************************************
// Externs
//*************************************************************************
extern thrm_fru_data_t      G_thrm_fru_data[DATA_FRU_MAX];

//*************************************************************************
// Defines/Enums
//*************************************************************************

//*************************************************************************
// Globals
//*************************************************************************

// Have we already called out the dimm for overtemp (bitmap of dimms)?
cent_sensor_flags_t G_dimm_overtemp_logged_bitmap = {0};

// Have we already called out the dimm for timeout (bitmap of dimms)?
cent_sensor_flags_t G_dimm_timeout_logged_bitmap = {0};

// Are any dimms currently in the timedout state (bitmap of centaurs)?
// Note: this only tells you which centaur, not which dimm.
uint8_t             G_dimm_temp_expired_bitmap = 0;

// Have we already called out the centaur for timeout (bitmap of centaurs)?
uint8_t G_cent_timeout_logged_bitmap = 0;

// Have we already called out the centaur for overtemp (bitmap of centaurs)?
uint8_t G_cent_overtemp_logged_bitmap = 0;

// Are any dimms currently in the timedout state (bitmap of centaurs)?
uint8_t G_cent_temp_expired_bitmap = 0;

// Array to store the update tag of each core's temperature sensor
uint32_t G_core_temp_update_tag[MAX_NUM_CORES] = {0};

//*************************************************************************
// Function Declarations
//*************************************************************************

//*************************************************************************
// Functions
//*************************************************************************
uint64_t amec_mem_get_huid(uint8_t i_cent, uint8_t i_dimm)
{
    uint64_t l_huid;

    if(i_dimm == 0xff)
    {
        //we're being asked for a centaur huid
        l_huid = G_sysConfigData.centaur_huids[i_cent];
    }
    else
    {
        //we're being asked for a dimm huid
        l_huid = G_sysConfigData.dimm_huids[i_cent][i_dimm];
        if(!l_huid)
        {
            //if we don't have a valid dimm huid, use the
            //centaur huid.
            //TODO: this will not work for ISDIMMS.
            l_huid = G_sysConfigData.centaur_huids[i_cent];
        }
    }
    return l_huid;
}

//If i_dimm is 0xff it is assumed that the caller wishes to
//mark the centaur as being logged.  Otherwise, it is assumed
//that the dimm should be marked.
void amec_mem_mark_logged(uint8_t i_cent,
                          uint8_t i_dimm,
                          uint8_t* i_clog_bitmap,
                          uint8_t* i_dlog_bitmap)
{
    if(i_dimm == 0xff)
    {
        //mark the centaur as being called out.
        *i_clog_bitmap |= CENTAUR0_PRESENT_MASK >> i_cent;
    }
    else
    {
        //mark the dimm as being called out.
        *i_dlog_bitmap |= DIMM_SENSOR0 >> i_dimm;
    }
}

void amec_health_check_dimm_temp()
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    uint16_t                    l_ot_error, l_cur_temp, l_max_temp;
    sensor_t                    *l_sensor;
    uint32_t                    l_cent, l_dimm;
    uint32_t                    l_callouts_count = 0;
    uint8_t                     l_new_callouts;
    uint64_t                    l_huid;
    errlHndl_t                  l_err = NULL;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    // Check to see if any dimms have reached the error temperature that
    // haven't been called out already
    if(G_dimm_overtemp_bitmap.bigword == G_dimm_overtemp_logged_bitmap.bigword)
    {
        return;
    }

    l_ot_error = g_amec->thermaldimm.ot_error;
    l_sensor = getSensorByGsid(TEMP2MSDIMM);
    l_cur_temp = l_sensor->sample;
    l_max_temp = l_sensor->sample_max;
    TRAC_ERR("amec_health_check_dimm_temp: DIMM reached error temp[%d]. cur_max[%d], hist_max[%d]",
             l_ot_error,
             l_cur_temp,
             l_max_temp);

    //iterate over all centaurs
    for(l_cent = 0; l_cent < MAX_NUM_CENTAURS; l_cent++)
    {
        //only callout a dimm if it hasn't been called out already
        l_new_callouts = G_dimm_overtemp_bitmap.bytes[l_cent] ^
                         G_dimm_overtemp_logged_bitmap.bytes[l_cent];

        //skip to next centaur if no new callouts for this one
        if(!l_new_callouts)
        {
            continue;
        }

        //find the dimm(s) that need to be called out behind this centaur
        for(l_dimm = 0; l_dimm < NUM_DIMMS_PER_CENTAUR; l_dimm++)
        {
            if(!(l_new_callouts & (DIMM_SENSOR0 >> l_dimm)))
            {
                continue;
            }

            l_huid = amec_mem_get_huid(l_cent, l_dimm);

            amec_mem_mark_logged(l_cent,
                                 l_dimm,
                                 &G_cent_overtemp_logged_bitmap,
                                 &G_dimm_overtemp_logged_bitmap.bytes[l_cent]);

            //If we don't have an error log for the callout, create one
            if(!l_err)
            {
                /* @
                 * @errortype
                 * @moduleid    AMEC_HEALTH_CHECK_DIMM_TEMP
                 * @reasoncode  DIMM_ERROR_TEMP
                 * @userdata1   Maximum dimm temperature
                 * @userdata2   Dimm temperature threshold
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

            //If we found all of the callouts for this centaur, go to the next one
            if(!l_new_callouts)
            {
                break;
            }
        }//iterate over dimms
    }//iterate over centaurs

    if(l_err)
    {
        commitErrl(&l_err);
    }
}

void amec_health_check_cent_temp()
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    uint16_t                    l_ot_error, l_cur_temp, l_max_temp;
    sensor_t                    *l_sensor;
    uint32_t                    l_cent;
    uint32_t                    l_callouts_count = 0;
    uint8_t                     l_new_callouts;
    uint64_t                    l_huid;
    errlHndl_t                  l_err = NULL;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    // Check to see if any centaurs have reached the error temperature that
    // haven't been called out already
    l_new_callouts = G_cent_overtemp_bitmap ^ G_cent_overtemp_logged_bitmap;
    if(!l_new_callouts)
    {
        return;
    }

    l_ot_error = g_amec->thermalcent.ot_error;
    l_sensor = getSensorByGsid(TEMP2MSCENT);
    l_cur_temp = l_sensor->sample;
    l_max_temp = l_sensor->sample_max;
    TRAC_ERR("amec_health_check_cent_temp: Centaur reached error temp[%d]. cur_max[%d], hist_max[%d] bitmap[0x%02X]",
             l_ot_error,
             l_cur_temp,
             l_max_temp,
             l_new_callouts);

    //find the centaur(s) that need to be called out
    for(l_cent = 0; l_cent < MAX_NUM_CENTAURS; l_cent++)
    {
        if(!(l_new_callouts & (CENTAUR0_PRESENT_MASK >> l_cent)))
        {
            continue;
        }

        l_huid = amec_mem_get_huid(l_cent, 0xff);

        amec_mem_mark_logged(l_cent,
                             0xff,
                             &G_cent_overtemp_logged_bitmap,
                             &G_dimm_overtemp_logged_bitmap.bytes[l_cent]);

        //If we don't have an error log for the callout, create one
        if(!l_err)
        {
            /* @
             * @errortype
             * @moduleid    AMEC_HEALTH_CHECK_CENT_TEMP
             * @reasoncode  CENT_ERROR_TEMP
             * @userdata1   Maximum centaur temperature
             * @userdata2   Centaur temperature threshold
             * @userdata4   OCC_NO_EXTENDED_RC
             * @devdesc     Centaur memory controller(s) exceeded maximum safe
             *              temperature.
             */
            l_err = createErrl(AMEC_HEALTH_CHECK_CENT_TEMP,    //modId
                               CENT_ERROR_TEMP,                //reasoncode
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

        // Callout centaur
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

    }//iterate over centaurs

    if(l_err)
    {
        commitErrl(&l_err);
    }
}

void amec_health_check_dimm_timeout()
{
    static cent_sensor_flags_t L_temp_update_bitmap_prev = {0};
    cent_sensor_flags_t l_need_inc, l_need_clr, l_temp_update_bitmap;
    uint8_t l_dimm, l_cent;
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
        l_need_inc.bigword = G_cent_enabled_sensors.bigword & ~l_temp_update_bitmap.bigword;

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

        //iterate across all centaurs incrementing dimm sensor timers as needed
        for(l_cent = 0; l_cent < MAX_NUM_CENTAURS; l_cent++)
        {
            //any dimm timers behind this centaur need incrementing?
            if(!l_need_inc.bytes[l_cent])
            {
                //all dimm sensors were updated for this centaur. Clear the dimm timeout bit for this centaur.
                if(G_dimm_temp_expired_bitmap & (CENTAUR0_PRESENT_MASK >> l_cent))
                {
                    G_dimm_temp_expired_bitmap &= ~(CENTAUR0_PRESENT_MASK >> l_cent);
                    TRAC_INFO("All dimm sensors for centaur %d have been updated", l_cent);
                }
                continue;
            }

            //There's at least one dimm requiring an increment, find the dimm
            for(l_dimm = 0; l_dimm < NUM_DIMMS_PER_CENTAUR; l_dimm++)
            {
                //not this one, go to next one
                if(!(l_need_inc.bytes[l_cent] & (DIMM_SENSOR0 >> l_dimm)))
                {
                    continue;
                }

                //we found one.
                l_fru = &g_amec->proc[0].memctl[l_cent].centaur.dimm_temps[l_dimm];

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
                    TRAC_INFO("Failed to read dimm temperature on cent[%d] dimm[%d] temp[%d] flags[0x%02X]",
                              l_cent, l_dimm, l_fru->cur_temp, l_fru->flags);
                }

                //check if the temperature reading is still useable
                if(g_amec->thermaldimm.temp_timeout == 0xff ||
                   l_fru->sample_age < g_amec->thermaldimm.temp_timeout)
                {
                    continue;
                }

                //temperature has expired.  Notify control algorithms which centaur.
                if(!(G_dimm_temp_expired_bitmap & (CENTAUR0_PRESENT_MASK >> l_cent)))
                {
                    G_dimm_temp_expired_bitmap |= CENTAUR0_PRESENT_MASK >> l_cent;
                    TRAC_ERR("Timed out reading dimm temperature sensor on cent %d.",
                             l_cent);
                }

                //If we've already logged an error for this FRU go to the next one.
                if(G_dimm_timeout_logged_bitmap.bytes[l_cent] & (DIMM_SENSOR0 >> l_dimm))
                {
                    continue;
                }

                TRAC_ERR("Timed out reading dimm temperature on cent[%d] dimm[%d] temp[%d] flags[0x%02X]",
                         l_cent, l_dimm, l_fru->cur_temp, l_fru->flags);

                if(!l_err)
                {
                    /* @
                     * @errortype
                     * @moduleid    AMEC_HEALTH_CHECK_DIMM_TIMEOUT
                     * @reasoncode  FRU_TEMP_TIMEOUT
                     * @userdata1   timeout value in seconds
                     * @userdata2   0
                     * @userdata4   OCC_NO_EXTENDED_RC
                     * @devdesc     Failed to read a memory DIMM temperature
                     *
                     */
                    l_err = createErrl(AMEC_HEALTH_CHECK_DIMM_TIMEOUT,    //modId
                                       FRU_TEMP_TIMEOUT,                 //reasoncode
                                       OCC_NO_EXTENDED_RC,                //Extended reason code
                                       ERRL_SEV_PREDICTIVE,               //Severity
                                       NULL,                              //Trace Buf
                                       DEFAULT_TRACE_SIZE,                //Trace Size
                                       g_amec->thermaldimm.temp_timeout,  //userdata1
                                       0);                                //userdata2

                    l_callouts_count = 0;
                }

                //Get the HUID for the dimm
                l_huid = amec_mem_get_huid(l_cent, l_dimm);

                // Callout dimm
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

                //Mark dimm as logged so we don't log it more than once
                amec_mem_mark_logged(l_cent,
                                     l_dimm,
                                     &G_cent_timeout_logged_bitmap,
                                     &G_dimm_timeout_logged_bitmap.bytes[l_cent]);
            } //iterate over all dimms

        } //iterate over all centaurs

        if(l_err)
        {
            commitErrl(&l_err);
        }

        //skip clearing if no dimms need it
        if(!l_need_clr.bigword)
        {
            break;
        }

        //iterate across all centaurs clearing dimm sensor timers as needed
        for(l_cent = 0; l_cent < MAX_NUM_CENTAURS; l_cent++)
        {

            if(!l_need_clr.bytes[l_cent])
            {
                continue;
            }

            //iterate over all dimms
            for(l_dimm = 0; l_dimm < NUM_DIMMS_PER_CENTAUR; l_dimm++)
            {
                //not this one, go to next one
                if(!(l_need_clr.bytes[l_cent] & (DIMM_SENSOR0 >> l_dimm)))
                {
                    continue;
                }

                //we found one.
                l_fru = &g_amec->proc[0].memctl[l_cent].centaur.dimm_temps[l_dimm];

                //clear timer
                l_fru->sample_age = 0;

                //info trace each time we recover
                if(L_ran_once)
                {
                    TRAC_INFO("DIMM temperature collection has resumed on cent[%d] dimm[%d] temp[%d]",
                              l_cent, l_dimm, l_fru->cur_temp);
                }

            }//iterate over all dimms
        }//iterate over all centaurs
    }while(0);
    L_ran_once = TRUE;
    G_thrm_fru_data[DATA_FRU_DIMM].read_failure = G_dimm_temp_expired_bitmap;
}

void amec_health_check_cent_timeout()
{
    static uint8_t L_temp_update_bitmap_prev = 0;
    uint8_t l_need_inc, l_need_clr, l_temp_update_bitmap;
    uint8_t l_cent;
    fru_temp_t* l_fru;
    errlHndl_t  l_err = NULL;
    uint32_t    l_callouts_count = 0;
    uint64_t    l_huid;
    static bool L_ran_once = FALSE;

    do
    {
        //For every centaur sensor there are 3 cases to consider
        //
        //1) centaur is present and not updated (need to increment timer and check for timeout)
        //2) centaur is present and updated but wasn't updated on previous check (need to clear timer)
        //3) centaur is present and updated and was updated on previous check (do nothing)

        //Grab snapshot of G_cent_temp_update_bitmap and clear it
        l_temp_update_bitmap = G_cent_temp_updated_bitmap;
        G_cent_temp_updated_bitmap = 0;

        //check if we need to increment any timers
        l_need_inc = G_present_centaurs & ~l_temp_update_bitmap;

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

        //iterate across all centaurs incrementing timers as needed
        for(l_cent = 0; l_cent < MAX_NUM_CENTAURS; l_cent++)
        {
            //does this centaur timer need incrementing?
            if(!(l_need_inc & (CENTAUR0_PRESENT_MASK >> l_cent)))
            {
                //temperature was updated for this centaur. Clear the timeout bit for this centaur.
                if(G_cent_temp_expired_bitmap & (CENTAUR0_PRESENT_MASK >> l_cent))
                {
                    G_cent_temp_expired_bitmap &= ~(CENTAUR0_PRESENT_MASK >> l_cent);
                    TRAC_INFO("centaur %d temps have been updated", l_cent);
                }
                continue;
            }

            //This centaur requires an increment
            l_fru = &g_amec->proc[0].memctl[l_cent].centaur.centaur_hottest;

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
                TRAC_INFO("Failed to read centaur temperature on cent[%d] temp[%d] flags[0x%02X]",
                              l_cent, l_fru->cur_temp, l_fru->flags);
            }

            //check if the temperature reading is still useable
            if(g_amec->thermalcent.temp_timeout == 0xff ||
               l_fru->sample_age < g_amec->thermalcent.temp_timeout)
            {
                continue;
            }

            //temperature has expired.  Notify control algorithms which centaur.
            if(!(G_cent_temp_expired_bitmap & (CENTAUR0_PRESENT_MASK >> l_cent)))
            {
                G_cent_temp_expired_bitmap |= CENTAUR0_PRESENT_MASK >> l_cent;
                TRAC_ERR("Timed out reading centaur temperature sensor on cent %d",
                         l_cent);
            }

            //If we've already logged an error for this FRU go to the next one.
            if(G_cent_timeout_logged_bitmap & (CENTAUR0_PRESENT_MASK >> l_cent))
            {
                continue;
            }

            TRAC_ERR("Timed out reading centaur temperature on cent[%d] temp[%d] flags[0x%02X]",
                     l_cent, l_fru->cur_temp, l_fru->flags);

            if(!l_err)
            {
                /* @
                 * @errortype
                 * @moduleid    AMEC_HEALTH_CHECK_CENT_TIMEOUT
                 * @reasoncode  FRU_TEMP_TIMEOUT
                 * @userdata1   timeout value in seconds
                 * @userdata2   0
                 * @userdata4   OCC_NO_EXTENDED_RC
                 * @devdesc     Failed to read a centaur memory controller
                 *              temperature
                 *
                 */
                l_err = createErrl(AMEC_HEALTH_CHECK_CENT_TIMEOUT,    //modId
                                   FRU_TEMP_TIMEOUT,                  //reasoncode
                                   OCC_NO_EXTENDED_RC,                //Extended reason code
                                   ERRL_SEV_PREDICTIVE,               //Severity
                                   NULL,                              //Trace Buf
                                   DEFAULT_TRACE_SIZE,                //Trace Size
                                   g_amec->thermalcent.temp_timeout,  //userdata1
                                   0);                                //userdata2

                l_callouts_count = 0;
            }

            //Get the HUID for the centaur
            l_huid = amec_mem_get_huid(l_cent, 0xff);

            // Callout centaur
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

            //Mark centaur as logged so we don't log it more than once
            amec_mem_mark_logged(l_cent,
                                 0xff,
                                 &G_cent_timeout_logged_bitmap,
                                 &G_dimm_timeout_logged_bitmap.bytes[l_cent]);
        } //iterate over all centaurs

        if(l_err)
        {
            commitErrl(&l_err);
        }

        //skip clearing timers if no centaurs need it
        if(!l_need_clr)
        {
            break;
        }

        //iterate across all centaurs clearing timers as needed
        for(l_cent = 0; l_cent < MAX_NUM_CENTAURS; l_cent++)
        {
            //not this one, go to next one
            if(!(l_need_clr & (CENTAUR0_PRESENT_MASK >> l_cent)))
            {
                continue;
            }

            //we found one.
            l_fru = &g_amec->proc[0].memctl[l_cent].centaur.centaur_hottest;

            //clear timer
            l_fru->sample_age = 0;

            //info trace each time we recover
            if(L_ran_once)
            {
                TRAC_INFO("centaur temperature collection has resumed on cent[%d] temp[%d]",
                          l_cent, l_fru->cur_temp);
            }

        }//iterate over all centaurs
    }while(0);
    L_ran_once = TRUE;
    G_thrm_fru_data[DATA_FRU_CENTAUR].read_failure = G_cent_temp_expired_bitmap;
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
    static uint32_t             l_error_count = 0;
    static BOOLEAN              l_ot_error_logged = FALSE;
    sensor_t                    *l_sensor;
    errlHndl_t                  l_err = NULL;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    do
    {
        // Get TEMP2MSP0PEAK sensor, which is hottest core temperature
        // in OCC processor
        l_sensor = getSensorByGsid(TEMP2MSP0PEAK);
        l_ot_error = g_amec->thermalproc.ot_error;

        // Check to see if we exceeded our error temperature
        if (l_sensor->sample > l_ot_error)
        {
            // Increment the error counter for this FRU
            l_error_count++;

            // Trace and log error the first time this occurs
            if (l_error_count == AMEC_HEALTH_ERROR_TIMER)
            {
                // Have we logged an OT error for this FRU already?
                if (l_ot_error_logged == TRUE)
                {
                    break;
                }

                l_ot_error_logged = TRUE;

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
            if (l_error_count >= AMEC_HEALTH_ERROR_TIMER)
            {
                TRAC_INFO("amec_health_check_proc_temp: We have dropped below error threshold for processors. error_count[%u]",
                          l_error_count);
            }

            // Reset the error counter for this FRU
            l_error_count = 0;
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
    gpe_bulk_core_data_t        *l_core_data_ptr = NULL;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    do
    {
        for(i=0; i<MAX_NUM_CORES; i++)
        {
            if(!CORE_PRESENT(i))
            {
                // If this core is not present, move on
                continue;
            }

            // Check if this core's temperature sensor has been updated
            l_sensor = AMECSENSOR_ARRAY_PTR(TEMP2MSP0C0,i);
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

                // Trace some critical registers to understand this error better
                TRAC_ERR("OHA_Status_Reg[0x%08X] PM_State_Hist_Reg[0x%08X]",
                         l_core_data_ptr->oha.oha_ro_status_reg.words.low_order,
                         l_core_data_ptr->pcb_slave.pm_history.words.high_order);
                TRAC_ERR("SensorV0[0x%016X] SensorV1[0x%016X]",
                         l_core_data_ptr->dts_cpm.sensors_v0.value,
                         l_core_data_ptr->dts_cpm.sensors_v1.value);

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
// Name:  amec_health_check_proc_vrhot
//
// Description: This function checks if VRHOT signal from processor regulator
// has been asserted. The VRHOT signal is actually derived in firmware: if
// VR_FAN signal is assserted and the 'fans_full_speed' GPIO is ON, then OCC
// will considered VR_HOT as being asserted.
//
// End Function Specification
void amec_health_check_proc_vrhot()
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    static BOOLEAN              L_error_logged = FALSE;
    sensor_t                    *l_sensor;
    errlHndl_t                  l_err = NULL;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    // Get VRHOT50USPROC sensor
    l_sensor = getSensorByGsid(VRHOT250USPROC);

    // Check to see if we have exceeded our ERROR_COUNT
    if(l_sensor->sample >= g_amec->vrhotproc.setpoint)
    {
        // We have reached the number of successive VRHOT samples allowed. Need
        // to log an error (only once per OCC reset).
        if(!L_error_logged)
        {
            L_error_logged = TRUE;

            TRAC_ERR("amec_health_check_proc_vrhot: VRHOT has been asserted! num_samples[%u]",
                     l_sensor->sample);

            /* @
             * @errortype
             * @moduleid    AMEC_HEALTH_CHECK_PROC_VRHOT
             * @reasoncode  VRM_ERROR_TEMP
             * @userdata1   VRHOT error threshold
             * @userdata2   0
             * @userdata4   OCC_NO_EXTENDED_RC
             * @devdesc     VRHOT signal has been asserted long enough to
             *              exceed its error threshold.
             *
             */
            l_err = createErrl(AMEC_HEALTH_CHECK_PROC_VRHOT,
                               VRM_ERROR_TEMP,
                               OCC_NO_EXTENDED_RC,
                               ERRL_SEV_PREDICTIVE,
                               NULL,
                               DEFAULT_TRACE_SIZE,
                               g_amec->vrhotproc.setpoint,
                               0);

            // Callout the Ambient procedure
            addCalloutToErrl(l_err,
                             ERRL_CALLOUT_TYPE_COMPONENT_ID,
                             ERRL_COMPONENT_ID_OVER_TEMPERATURE,
                             ERRL_CALLOUT_PRIORITY_HIGH);

            // Callout backplane
            addCalloutToErrl(l_err,
                             ERRL_CALLOUT_TYPE_HUID,
                             G_sysConfigData.backplane_huid,
                             ERRL_CALLOUT_PRIORITY_MED);

            // Commit Error
            commitErrl(&l_err);
        }
    }
}

/*----------------------------------------------------------------------------*/
/* End                                                                        */
/*----------------------------------------------------------------------------*/
