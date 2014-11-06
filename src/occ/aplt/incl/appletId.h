/******************************************************************************
// @file appletId.h
// @brief Applet ID numbers
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section sensorQueryList.h SENSORQUERYLIST.H
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *   @dw000             dwoodham  12/13/2011  New file
 *   @rc003             rickylie  02/03/2012  Verify & Clean Up OCC Headers & Comments 
 *   @pb00E             pbavari   03/11/2012  Added correct include file
 *   @th00d             thallet   04/25/2012  Added CMDH Debug Applet
 *
 *  @endverbatim
 *
 *///*************************************************************************/

#ifndef _APPLET_ID_H
#define _APPLET_ID_H

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
// Original code & comments from occ/aplt/appletManager.h
// These are the current applets which exist. 
//@pb004a - Added OCC_APLT_RTL_INIT,OCC_APLT_APSS_INIT,OCC_APLT_SNSR_INIT and
//          OCC_APLT_TEST
//@pb00Ad - Removed sensor init applet init as it is not an applet anymore.
typedef enum
{
    OCC_APLT_APSS_INIT      = 0x00,
    OCC_APLT_SNSR_QUERY     = 0x01,
    OCC_APLT_CMDH_DBUG      = 0x02,
    OCC_APLT_TEST,
    OCC_APLT_LAST,
    OCC_APLT_INVALID        = 0xFF
} OCC_APLT;

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

#endif // _APPLET_ID_H
