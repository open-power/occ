/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/thread/thrm_thread.c $                            */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2015                        */
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

#include <cmdh_fsp_cmds_datacnfg.h>
#include <thrm_thread.h>
#include <trac.h>
#include <state.h>
#include <occ_service_codes.h>
#include <thread_service_codes.h>
#include <occ_sys_config.h>

// Global for notifying the thermal thread if it needs to update its copy
// of the thermal threshold data packet (format 0x13)
BOOLEAN                     G_thrm_parms_need_update = FALSE;

// Fan control loop time in seconds sent by TMGT (format 0x13)
uint8_t                     G_thrm_loop_time = 0;

// Cooling request status (0=no request; 1=send request)
uint8_t                     G_thrm_cooling_request = 0;

// Local copy of thermal thresholds sent by TMGT (format 0x13)
thrm_fru_control_t          G_thrm_fru_control[DATA_FRU_MAX];

// Global array to store dynamic information for each FRU
thrm_fru_data_t             G_thrm_fru_data[DATA_FRU_MAX];

// Function Specification
//
// Name:  THRM_thread_update_thresholds
//
// Description: Notify thermal thread to update its local copy of the
// thermal threshold data packet.
//
// End Function Specification
void THRM_thread_update_thresholds()
{
    G_thrm_parms_need_update = TRUE;
}

// Function Specification
//
// Name:  THRM_thread_get_cooling_request
//
// Description: Get the status of the cooling request from the thermal
// thread.
//
// End Function Specification
uint8_t THRM_thread_get_cooling_request()
{
    return G_thrm_cooling_request;
}

// Function Specification
//
// Name:  thrm_thread_load_thresholds
//
// Description: Read thermal thresholds sent by TMGT (data format 0x13)
// and store them for thermal thread consumption.
//
// End Function Specification
errlHndl_t thrm_thread_load_thresholds()
{
    errlHndl_t                  l_err = NULL;

/* TEMP -- THRM THREAD NOT YET ENABLED (NEED SENSOR / AMEC) */
#if 0
    cmdh_thrm_thresholds_t      *l_data = NULL;
    uint8_t                     i = 0;

    do
    {
        // Get thermal threshold data sent by TMGT
        l_err = DATA_get_thrm_thresholds(&l_data);
        if(l_err)
        {
            TRAC_ERR("thrm_thread_load_thresholds: Thermal control thresholds not available!");

            // This is a critical failure since we can't run the fan control
            // without the thermal thresholds. Make the fan control loop time
            // zero so that we don't execute any fan control
            G_thrm_loop_time = 0;
            break;
        }

        // Load fan control loop time
        G_thrm_loop_time = l_data->fan_control_loop_time;

        TRAC_INFO("thrm_thread_load_thresholds: Fan Control loop time[%u]",
                  G_thrm_loop_time);

        // Load FRU data
        for (i=0; i<DATA_FRU_MAX; i++)
        {
            // Load data that is not dependant on the system mode
            G_thrm_fru_control[i].warning = l_data->data[i].warning;
            G_thrm_fru_control[i].warning_reset = l_data->data[i].warning_reset;
            G_thrm_fru_control[i].t_inc_zone[0] = l_data->data[i].t_inc_zone1;
            G_thrm_fru_control[i].t_inc_zone[1] = l_data->data[i].t_inc_zone2;
            G_thrm_fru_control[i].t_inc_zone[2] = l_data->data[i].t_inc_zone3;
            G_thrm_fru_control[i].t_inc_zone[3] = l_data->data[i].t_inc_zone4;
            G_thrm_fru_control[i].t_inc_zone[4] = l_data->data[i].t_inc_zone5;
            G_thrm_fru_control[i].t_inc_zone[5] = l_data->data[i].t_inc_zone6;
            G_thrm_fru_control[i].t_inc_zone[6] = l_data->data[i].t_inc_zone7;
            G_thrm_fru_control[i].t_inc_zone[7] = l_data->data[i].t_inc_zone8;

            // Load data that depends on the system mode
            // Acoustic mode takes priority
            if (0) // TODO: Need to implement acoustic mode
            {
                G_thrm_fru_control[i].t_control = l_data->data[i].acoustic_t_control;
                G_thrm_fru_control[i].error = l_data->data[i].acoustic_error;
            }
            // ...then nominal mode
            else if (CURRENT_MODE() == OCC_MODE_NOMINAL)
            {
                G_thrm_fru_control[i].t_control = l_data->data[i].t_control;
                G_thrm_fru_control[i].error = l_data->data[i].error;
            }
            // ...then any other power management mode
            else
            {
                G_thrm_fru_control[i].t_control = l_data->data[i].pm_t_control;
                G_thrm_fru_control[i].error = l_data->data[i].pm_error;
            }

            TRAC_INFO("thrm_thread_load_thresholds: Using t_control[%u] error[%u] for FRU[0x%02X]",
                      G_thrm_fru_control[i].t_control,
                      G_thrm_fru_control[i].error,
                      i);
        }
    }while(0);
#endif

    return l_err;
}


// Function Specification
//
// Name:  thrm_thread_fan_control
//
// Description: Execute the fan control algorithm for the specified FRU.
// Returns TRUE or FALSE depending whether we need to increase fan speeds
// or not.
//
// End Function Specification
BOOLEAN thrm_thread_fan_control(const uint8_t i_fru_type,
                                const uint16_t i_fru_temperature)
{
    eConfigDataFruType          k = i_fru_type;
    BOOLEAN                     l_IncreaseFans = FALSE;
    uint8_t                     i = 0;
    uint16_t                    l_delta_temp = 0;

    //If there's a read error, assume temperature is t_control + 1
    if(G_thrm_fru_data[k].read_failure)
    {
        G_thrm_fru_data[k].Tcurrent = G_thrm_fru_control[k].t_control + 1;
    }
    else
    {
        // Collect the current FRU temperature
        G_thrm_fru_data[k].Tcurrent = i_fru_temperature;
    }

    // Is the current FRU temperature bigger than the FRU T_control?
    if (G_thrm_fru_data[k].Tcurrent > G_thrm_fru_control[k].t_control)
    {
        // Is the current temperature below the previous temperature?
        if (G_thrm_fru_data[k].Tcurrent < G_thrm_fru_data[k].Tprevious)
        {
            // Temperature is decreasing so we don't need to request any
            // additional cooling
            G_thrm_fru_data[k].temp_increasing = FALSE;
        }
        else if (G_thrm_fru_data[k].Tcurrent > G_thrm_fru_data[k].Tprevious)
        {
            // Temperature is increasing and we need to request more cooling
            G_thrm_fru_data[k].temp_increasing = TRUE;
            l_IncreaseFans = TRUE;
        }
        else
        {
            // Temperature is the same as previous iteration. Check the trend
            // to see whether is increasing or decreasing
            if (G_thrm_fru_data[k].temp_increasing == TRUE)
            {
                // Need to request additional cooling
                l_IncreaseFans = TRUE;
            }
            else
            {
                // No need to request additional cooling
            }
        }
    }

    // Prepare the cooling requests for all Zones
    l_delta_temp = G_thrm_fru_data[k].Tcurrent - G_thrm_fru_control[k].t_control;

    if (l_IncreaseFans == TRUE)
    {
        for (i = 0; i < THRM_MAX_NUM_ZONES; i++)
        {
            G_thrm_fru_data[k].FanIncZone[i] = l_delta_temp *
                G_thrm_fru_control[k].t_inc_zone[i];
        }
    }
    else
    {
        for (i = 0; i < THRM_MAX_NUM_ZONES; i++)
        {
            G_thrm_fru_data[k].FanIncZone[i] = 0;
        }
    }

    // Store the current temperature for next iteration
    G_thrm_fru_data[k].Tprevious = G_thrm_fru_data[k].Tcurrent;

    return(l_IncreaseFans);
}


// Function Specification
//
// Name:  thrm_thread_vrm_fan_control
//
// Description: Execute the fan control algorithm for VRM based on the
// VR_FAN signal. Returns TRUE or FALSE depending whether we need to increase
// fan speeds or not.
//
// End Function Specification
BOOLEAN thrm_thread_vrm_fan_control(const uint16_t i_vrfan)
{
    BOOLEAN                     l_IncreaseFans = FALSE;
    static BOOLEAN              L_error_logged = FALSE;
    uint8_t                     i = 0;
    errlHndl_t                  l_err = NULL;

    if ((i_vrfan == 1) || (G_thrm_fru_data[DATA_FRU_VRM].read_failure == 1))
    {
        // If VR_FAN has been asserted or we cannot read it, request one T_INC
        // for each zone to be controlled
        l_IncreaseFans = TRUE;

        for (i = 0; i < THRM_MAX_NUM_ZONES; i++)
        {
            G_thrm_fru_data[DATA_FRU_VRM].FanIncZone[i] =
                G_thrm_fru_control[DATA_FRU_VRM].t_inc_zone[i];
        }

        // Log an informational error the first time VR_FAN was asserted (this
        // is for manufacturing mode IPLs)
        if (i_vrfan == 1)
        {
            if (!L_error_logged)
            {
                L_error_logged = TRUE;

                TRAC_ERR("thrm_thread_vrm_fan_control: VR_FAN for this regulator has been asserted!");

                /* @
                 * @errortype
                 * @moduleid    THRD_THERMAL_VRM_FAN_CONTROL
                 * @reasoncode  VRM_VRFAN_ASSERTED
                 * @userdata1   state of VR_FAN signal
                 * @userdata2   0
                 * @userdata4   OCC_NO_EXTENDED_RC
                 * @devdesc     VR_FAN signal from regulator has been asserted.
                 *
                 */
                l_err = createErrl(THRD_THERMAL_VRM_FAN_CONTROL,  //modId
                                   VRM_VRFAN_ASSERTED,            //reasoncode
                                   OCC_NO_EXTENDED_RC,            //Extended reason code
                                   ERRL_SEV_INFORMATIONAL,        //Severity
                                   NULL,                          //Trace Buf
                                   DEFAULT_TRACE_SIZE,            //Trace Size
                                   i_vrfan,                       //userdata1
                                   0);                            //userdata2

                // Set the manufacturing error flag in the log
                setErrlActions(l_err, ERRL_ACTIONS_MANUFACTURING_ERROR);

                // Commit the error
                commitErrl(&l_err);
            }
        }
    }
    else
    {
        // If VR_FAN is de-asserted, don't send any cooling requests
        l_IncreaseFans = FALSE;

        for (i = 0; i < THRM_MAX_NUM_ZONES; i++)
        {
            G_thrm_fru_data[DATA_FRU_VRM].FanIncZone[i] = 0;
        }
    }

    return(l_IncreaseFans);
}

// Function Specification
//
// Name:  thrm_thread_main
//
// Description: Main thermal thread routine
//
// End Function Specification
void thrm_thread_main()
{
/* TEMP -- THRM THREAD NOT ENABLED YET (NEED SENSOR / AMEC) */
#if 0
    errlHndl_t                  l_err = NULL;
    BOOLEAN                     l_IncreaseFans = FALSE;
    sensor_t                    *l_sensor = NULL;
    static uint8_t              L_counter = 0;
    static BOOLEAN              L_full_speed_log = FALSE;

    // Execute fan control loop if OCC is in observation state or active state
    // AND if we have received the thermal threshold data packet
    if (((CURRENT_STATE() == OCC_STATE_OBSERVATION) ||
         (CURRENT_STATE() == OCC_STATE_ACTIVE))     &&
        (DATA_get_present_cnfgdata() & DATA_MASK_THRM_THRESHOLDS))
    {
        // Check if we need to update our local copy of the thermal threshold
        // data packet
        if (G_thrm_parms_need_update)
        {
            // Clear this flag now that we are going to update our local copy
            G_thrm_parms_need_update = FALSE;

            // Update our local copy of the thermal thresholds
            l_err = thrm_thread_load_thresholds();

            if (l_err)
            {
                // Commit error log for failing to load thermal thresholds
                commitErrl(&l_err);
            }
            else
            {
                // Clear our counter that determines when to run the fan
                // control loop
                L_counter = 0;
            }
        }

        // Increment our counter and make sure it doesn't wrap
        L_counter++;
        // The fan control loop time is only 1-byte long. A loop time of 0
        // indicates that we shouldn't run the fan control loop.
        if (L_counter == 0)
        {
            L_counter = 0xFF;
        }

        // Is it time to execute the fan control loop?
        if (L_counter == G_thrm_loop_time)
        {
            // Reset our counter
            L_counter = 0;

            // Determine if additional cooling is required for VRMs
            l_sensor = getSensorByGsid(VRFAN250USPROC);
            l_IncreaseFans |= thrm_thread_vrm_fan_control(l_sensor->sample);

            // Determine if additional cooling is required for processors
            // For processors, use the hottest of the core averages
            l_sensor = getSensorByGsid(TEMP2MSP0PEAK);
            l_IncreaseFans |= thrm_thread_fan_control(DATA_FRU_PROC,
                                                      l_sensor->sample);

            // Determine if additional cooling is required for centaurs
            l_sensor = getSensorByGsid(TEMP2MSCENT);
            l_IncreaseFans |= thrm_thread_fan_control(DATA_FRU_CENTAUR,
                                                      l_sensor->sample);

            // Determine if additional cooling is required for dimms
            l_sensor = getSensorByGsid(TEMP2MSDIMM);
            l_IncreaseFans |= thrm_thread_fan_control(DATA_FRU_DIMM,
                                                      l_sensor->sample);

            // Do we need to request a fan increase?
            if (l_IncreaseFans == TRUE)
            {
                // Set 'Cooling Request' for the poll response
                G_thrm_cooling_request = 1;

                // Send 'Service' attention to FSP to force TMGT to send poll
                // command
                cmdh_fsp_attention(OCC_ALERT_FSP_SERVICE_REQD);

                // Check if fans are currently already at max (known from APSS)
                // log informational B1812A08 and have the "Manufacturing error"
                // bit in the actions field set so that TMGT will flag this in manufacturing.
                // ** No longer reading gpio from APSS in GA1 due to instability in APSS composite mode
                uint8_t l_full_speed = 1; // low active
                // TODO: Should we continue to not read gpio from APSS?
                if( (L_full_speed_log == FALSE) &&
                    FALSE) //apss_gpio_get(l_full_speed_pin,&l_full_speed) )
                {
                    if(l_full_speed == 0) //low active
                    {
                        L_full_speed_log = TRUE; // log this error ONLY ONCE per IPL
                        TRAC_ERR("thrm_thread_main: Request a fan increase and fan is already in full speed");

                        /* @
                         * @errortype
                         * @moduleid    THRD_THERMAL_MAIN
                         * @reasoncode  FAN_FULL_SPEED
                         * @userdata1   0
                         * @userdata2   0
                         * @userdata4   OCC_NO_EXTENDED_RC
                         * @devdesc     Request a fan increase and fan is already in full speed
                         *
                         */
                        l_err = createErrl(THRD_THERMAL_MAIN,             //modId
                                           FAN_FULL_SPEED,                //reasoncode
                                           OCC_NO_EXTENDED_RC,            //Extended reason code
                                           ERRL_SEV_INFORMATIONAL,        //Severity
                                           NULL,                          //Trace Buf
                                           DEFAULT_TRACE_SIZE,            //Trace Size
                                           0,                             //userdata1
                                           0);                            //userdata2

                        // Set the manufacturing error flag in the log
                        setErrlActions(l_err, ERRL_ACTIONS_MANUFACTURING_ERROR);
                        commitErrl(&l_err);
                    }
                }
                //
            }
            else
            {
                // Reset the 'Cooling Request' since we don't need a fan increase
                G_thrm_cooling_request = 0;
            }
        }
    }
#endif
}
