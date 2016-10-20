/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/sensor/sensor_query_list.c $                      */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2016                        */
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
#include <common_types.h>   // imageHdr_t declaration and image header macro
#include <occ_common.h>     // imageHdr_t declaration and image header macro
#include <errl.h>           // For error handle
#include "ssx_io.h"         // For sprintf
#include <trac.h>           // For traces
#include <sensor_service_codes.h> // sensor module ids
#include <occ_service_codes.h>    // sensor module ids
#include <sensor.h>               // For Sensor defines
#include <sensor_query_list.h>      // For args to command

//*************************************************************************/
// Externs
//*************************************************************************/

//*************************************************************************/
// Macros
//*************************************************************************/

//*************************************************************************/
// Defines/Enums
//*************************************************************************/

//*************************************************************************/
// Structures
//*************************************************************************/

//*************************************************************************/
// Globals
//*************************************************************************/

//*************************************************************************/
// Function Prototypes
//*************************************************************************/

//*************************************************************************/
// Functions
//*************************************************************************/

// Function Specification
//
//  Name: printSensorInfo
//
//  Description: Dump a sensor's info via printf
//
// End Function Specification
void printSensorInfo(uint16_t i_gsid)
{
// This prevents warnings of set but not used since without SNSR_DEBUG
// being set, the SNSR_DBG macros don't do anything.
#ifdef SNSR_DEBUG

    // Initialize variable j and k with NULL and then
    // point to sensor to avoid compilation error when SNSR_DEBUG is not
    // defined. j and k are only used with SNSR_DBG which is no-op statement
    // when SNSR_DEBUG is not defined.
    sensor_t * k = NULL;
    k = G_amec_sensor_list[i_gsid];
    uint16_t * j = NULL;
    j = G_amec_sensor_list[i_gsid]->mini_sensor;

    // Print Sensors Information from Sensor_info_t Table
    SNSR_DBG("Sensor [%d] = Name: %s, Units: %s, Type: 0x%04x, Loc: 0x%04x, Num: %d, Freq: 0x%08x, Scale: 0x%08x\n",
             i_gsid,
             G_sensor_info[i_gsid].name,
             G_sensor_info[i_gsid].sensor.units,
             G_sensor_info[i_gsid].sensor.type,
             G_sensor_info[i_gsid].sensor.location,
             G_sensor_info[i_gsid].sensor.number,
             G_sensor_info[i_gsid].sensor.freq,
             G_sensor_info[i_gsid].sensor.scalefactor
    );

    // Print Sensor Information from Sensor_t
    SNSR_DBG("SensorPtr=0x%08x, Sample=%d, Max=%d, Min=%d, Tag=%d, MiniSensorPtr=0x%08x, MiniSensorVal=0x%04x\n",
             (uint32_t) k,
             k->sample,
             k->sample_max,
             k->sample_min,
             k->update_tag,
             (uint32_t)k->mini_sensor,
             (NULL != j) ? *j : 0
    );

#endif
}

// Function Specification
//
//  Name: printAllSensors
//
//  Description: Dump all sensors via printf.
//
// End Function Specification
void printAllSensors(void)
{
    int i;

    // Loop through sensor table and printf all sensors
    for(i=0; i < NUMBER_OF_SENSORS_IN_LIST; i++)
    {
        printSensorInfo(i);
    }
}

/*****************************************************************************/
// Entry point function
/*****************************************************************************/

// Function Specification
//
//  Name: querySensorList
//
//  Description: Query sensor list
//
// End Function Specification
errlHndl_t querySensorList(const querySensorListArg_t * i_argPtr)
{
    errlHndl_t l_err = NULL;

    if (i_argPtr != NULL)
    {
        uint16_t            i_startGsid     = i_argPtr->i_startGsid;
        uint8_t             i_present       = i_argPtr->i_present;
        uint16_t            i_type          = i_argPtr->i_type;
        uint16_t            i_loc           = i_argPtr->i_loc;
        uint16_t *          io_numOfSensors = i_argPtr->io_numOfSensors;
        sensorQueryList_t * o_sensors       = i_argPtr->o_sensors;
        sensor_info_t     * o_sensorInfoPtrs= i_argPtr->o_sensorInfoPtrs;

        // Validate input parameters
        if( (i_startGsid >= NUMBER_OF_SENSORS_IN_LIST) ||
            ((o_sensors == NULL) && (o_sensorInfoPtrs ==NULL)) ||
            (io_numOfSensors == NULL))
        {
            TRAC_ERR("querySensorList: Invalid input pointers OR start GSID is out of range: "
                     "i_startGsid: 0x%x, G_amec_sensor_count: 0x%x",
                     i_startGsid,G_amec_sensor_count);

            /* @
             * @errortype
             * @moduleid    SENSOR_QUERY_LIST
             * @reasoncode  INTERNAL_INVALID_INPUT_DATA
             * @userdata1   i_startGsid -- passed in Global Sensor ID
             * @userdata2   G_amec_sensor_count -- number of OCC sensors
             * @userdata4   OCC_NO_EXTENDED_RC
             * @devdesc     Firmware failure caused due to invalid GSID passed
             */

            /* @
             * @errortype
             * @moduleid    SENSOR_QUERY_LIST
             * @reasoncode  INTERNAL_FAILURE
             * @userdata1   i_startGsid -- passed in Global Sensor ID
             * @userdata2   G_amec_sensor_count -- number of OCC sensors
             * @userdata4   OCC_NO_EXTENDED_RC
             * @devdesc     NULL pointer passed for querySensorList output args
             */
            l_err = createErrl(SENSOR_QUERY_LIST,                 //modId
                    ((i_startGsid >= NUMBER_OF_SENSORS_IN_LIST) ? INTERNAL_INVALID_INPUT_DATA : INTERNAL_FAILURE), //reasoncode
                    OCC_NO_EXTENDED_RC,                               //Extended reason code
                    ERRL_SEV_PREDICTIVE,                              //Severity
                    NULL,                                             //Trace Buf
                    0,                                                //Trace Size
                    i_startGsid,                                      //userdata1
                    G_amec_sensor_count                               //userdata2
            );
        }
        else
        {
            uint32_t l_cnt = i_startGsid;
            uint32_t l_num = *io_numOfSensors;
            *io_numOfSensors = 0;

            // Traverse through sensor list starting at i_startGsid to find
            // matching sensor. Return it in the output variable
            for (; (l_cnt < NUMBER_OF_SENSORS_IN_LIST && ((*io_numOfSensors) < l_num)); l_cnt++)
            {
                // If sample value is not zero then it means sensor is present.
                // This is currently only used by debug/mfg purpose
                // If user is looking for present sensors and sample is zero,
                // then don't include current sensor in the query list
                if ((i_present) && (G_amec_sensor_list[l_cnt]->sample == 0))
                {
                    continue;
                }

                // If user is NOT looking for any sensor type and input type,
                // does not match the current sensor type, then don't include
                // current sensor in the query list
                if ((i_type & G_sensor_info[l_cnt].sensor.type) == 0)
                {
                    continue;
                }

                // If user is NOT looking for any sensor location and input loc,
                // does not match the current sensor location, then don't include
                // current sensor in the query list
                if ((i_loc & G_sensor_info[l_cnt].sensor.location) == 0)
                {
                    continue;
                }

                if (o_sensors != NULL)
                {
                    // All conditions match. Include current sensor in the query list
                    // Copy gsid, name and sample
                    o_sensors->gsid = l_cnt;
                    strncpy(o_sensors->name, G_sensor_info[l_cnt].name, MAX_SENSOR_NAME_SZ);
                    o_sensors->sample = G_amec_sensor_list[l_cnt]->sample;
                    o_sensors++;
                }

                if (o_sensorInfoPtrs != NULL)
                {
                    memcpy(o_sensorInfoPtrs, &G_sensor_info[l_cnt], sizeof(sensor_info_t));
                    o_sensorInfoPtrs++;
                }

                (*io_numOfSensors)++;
            }
        }
    }
    else
    {
        TRAC_ERR("querySensorList: Invalid argument pointer = NULL");

        /* @
         * @errortype
         * @moduleid    SENSOR_QUERY_LIST
         * @reasoncode  INTERNAL_INVALID_INPUT_DATA
         * @userdata1   NULL
         * @userdata2   NULL
         * @userdata4   ERC_ARG_POINTER_FAILURE
         * @devdesc     NULL pointer passed to querySensorList applet
         */
        l_err = createErrl(
                SENSOR_QUERY_LIST,            // Module ID
                INTERNAL_INVALID_INPUT_DATA,  // Reason Code
                ERC_ARG_POINTER_FAILURE,      // Extended reason code
                ERRL_SEV_PREDICTIVE,          // Severity
                NULL,                         // Trace
                0,                            // Trace Size
                0,                            // UserData 1
                0                             // UserData 2
              );
    }

    return l_err;
}
