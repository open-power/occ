/******************************************************************************
// @file sensorQueryList.h
// @brief Header Applet Args -- Query Sensor List for AMESTER or MNFG
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section sensorQueryList.h SENSORQUERYLIST.H
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *   @th005             thallet   11/22/2011  New file
 *   @rc003             rickylie  02/07/2012  Verify & Clean Up OCC Headers & Comments
 *   @at003             alvinwan  03/19/2012  Add o_sensorInfoPtrs to querySensorListAppletArg_t
 *   @pb00E             pbavari   03/11/2012  Added correct include file
 *   @gm002   885429    milesg    04/30/2013  change location/type to 16 bit bitmasks
 *
 *  @endverbatim
 *
 *///*************************************************************************/

#ifndef _SENSORQUERYLIST_H
#define _SENSORQUERYLIST_H

//*************************************************************************
// Includes
//*************************************************************************
//@pb00Ec - changed from common.h to occ_common.h for ODE support
#include <occ_common.h>

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************

//*************************************************************************
// Structures
//*************************************************************************
// Structure that is passed into querySensorList applet
// when it is called
typedef struct
{
  uint16_t            i_startGsid;
  uint8_t             i_present;
  uint16_t            i_type;           //gm002
  uint16_t            i_loc;            //gm002
  uint16_t *          io_numOfSensors;
  sensorQueryList_t * o_sensors;
  sensor_info_t * o_sensorInfoPtrs;  //@at002A
} querySensorListAppletArg_t;

//*************************************************************************
// Globals
//*************************************************************************

//*************************************************************************
// Function Prototypes
//*************************************************************************

//*************************************************************************
// Functions
//*************************************************************************

#endif

