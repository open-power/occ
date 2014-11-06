/******************************************************************************
// @file sensorQueryList.c
// @brief sensor sensorQueryList product applet
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section sensorQueryList.c SENSORQUERYLIST.c
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *   @th005             thallet   11/21/2011  Created 
 *   @dw000             dwoodham  12/12/2011  Update call to IMAGE_HEADER
 *   @pb00C             pbavari   01/20/2012  Changed printf to SNR_DBG
 *   @rc003             rickylie  02/03/2012  Verify & Clean Up OCC Headers & Comments
 *   @at003             alvinwan  03/19/2012  Add o_sensorInfoPtrs to querySensorListAppletArg_t
 *   @nh001             neilhsu   05/23/2012  Add missing error log tags 
 *   @gm002   885429    milesg    05/30/2013  change loc/type to 16 bit bitmask
 *   @wb003   920760    wilbryan  03/25/2014  Update SRCs to match TPMD SRCs
 *
 *  @endverbatim
 *
 *///*************************************************************************/
 
//*************************************************************************
// Includes
//*************************************************************************
#include <common_types.h>   // imageHdr_t declaration and image header macro
#include <occ_common.h>         // imageHdr_t declaration and image header macro
#include <errl.h>           // For error handle
#include "ssx_io.h"         // For sprintf
#include <trac.h>           // For traces
#include <sensor_service_codes.h> // sensor module ids
#include <occ_service_codes.h>    // sensor module ids
#include <sensor.h>               // For Sensor defines
#include <sensorQueryList.h>      // For args to applet 
#include <appletId.h>       // For applet ID num dw000a

/*****************************************************************************/
// C Source File Includes for this Applet
// --------------------------------------
//  \_ Must be done to give this applet access to the G_sensor_info 
//     sensor list
/*****************************************************************************/
#include "sensor_info.c"

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************
#define SENSOR_QUERY_ID  "SNSR QueryList\0"

//*************************************************************************
// Structures
//*************************************************************************

//*************************************************************************
// Globals
//*************************************************************************

//*************************************************************************
// Function Prototypes
//*************************************************************************

//*************************************************************************
// Functions
//*************************************************************************


// Function Specification 
//
//  Name: printSensorInfo
//
//  Description: Dump a sensor's info via printf
//
//  Flow: --/--/----     FN= None
//
// End Function Specification 
void printSensorInfo(uint16_t i_gsid)
{
    //@pb00Cc - Changed to initialize variable j and k with NULL and then
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
}


// Function Specification 
//
//  Name: printAllSensors
//
//  Description: Dump all sensors via printf.
//
//  Flow: --/--/----     FN= none
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
//  Flow: 06/27/2011     FN= querySensorList
//
// End Function Specification

// When this moved to applet, instead of getting the data
// via sensor_t, it now has to get some of it via sensor_info_t.  The sensor
// info array is still be indexed by gsid, so this should be easy.
errlHndl_t querySensorList(const querySensorListAppletArg_t * i_argPtr)
{
  errlHndl_t l_err = NULL;

  if(i_argPtr != NULL)
  {
    uint16_t            i_startGsid     = i_argPtr->i_startGsid;
    uint8_t             i_present       = i_argPtr->i_present;  
    uint16_t            i_type          = i_argPtr->i_type;     
    uint16_t            i_loc           = i_argPtr->i_loc;      
    uint16_t *          io_numOfSensors = i_argPtr->io_numOfSensors;
    sensorQueryList_t * o_sensors       = i_argPtr->o_sensors;  
    sensor_info_t     * o_sensorInfoPtrs= i_argPtr->o_sensorInfoPtrs; //@at003A

    // Validate input parameters
    if( (i_startGsid >= NUMBER_OF_SENSORS_IN_LIST) ||
        ((o_sensors == NULL) && (o_sensorInfoPtrs ==NULL)) ||   //@at003M
        (io_numOfSensors == NULL))
    {
      TRAC_ERR("Invalid input pointers OR start GSID is out of range: "
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
       * @devdesc     NULL pointer passed for querySensorList applet output args
       */
      l_err = createErrl(SENSOR_QUERY_LIST,                 //modId
          ((i_startGsid >= NUMBER_OF_SENSORS_IN_LIST)?  
           INTERNAL_INVALID_INPUT_DATA : INTERNAL_FAILURE), //reasoncode  // @wb003
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
      for(;(l_cnt < NUMBER_OF_SENSORS_IN_LIST && ((*io_numOfSensors) < l_num));
          l_cnt++)
      {
        // If sample value is not zero then it means sensor is present.
        // This is currently only used by debug/mfg purpose
        // If user is looking for present sensors and sample is zero,
        // then don't include current sensor in the query list
        if( (i_present) && (G_amec_sensor_list[l_cnt]->sample == 0))
        {
            continue;
        }

        // If user is NOT looking for any sensor type and input type,
        // does not match the current sensor type, then don't include
        // current sensor in the query list
        if((i_type & G_sensor_info[l_cnt].sensor.type) == 0)
        {
            continue;
        }

        // If user is NOT looking for any sensor location and input loc,
        // does not match the current sensor location, then don't include
        // current sensor in the query list
        if((i_loc & G_sensor_info[l_cnt].sensor.location) == 0)
        {
            continue;
        }

        //@at003M begin<
        if( o_sensors != NULL)
        {
            // All conditions match. Include current sensor in the query list
            // Copy gsid, name and sample
            o_sensors->gsid = l_cnt;
            strncpy(o_sensors->name,G_sensor_info[l_cnt].name,
                    MAX_SENSOR_NAME_SZ);
            o_sensors->sample = G_amec_sensor_list[l_cnt]->sample;
            o_sensors++;
        }

        if( o_sensorInfoPtrs != NULL)
        {
            memcpy( o_sensorInfoPtrs, &G_sensor_info[l_cnt], sizeof(sensor_info_t));
            o_sensorInfoPtrs++;
        }
        // @at003M end>

        (*io_numOfSensors)++;
      } // end for loop
    } // valid input parameter path
  }
  else
  {
      TRAC_ERR("Invalid applet argument pointer = NULL");

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
          INTERNAL_INVALID_INPUT_DATA,  // Reason Code   // @wb003
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



/*****************************************************************************/
// Image Header
/*****************************************************************************/
// @dw000 - Add applet ID arg to IMAGE_HEADER macro call
IMAGE_HEADER (G_querySensorList,querySensorList,SENSOR_QUERY_ID,OCC_APLT_SNSR_QUERY);

