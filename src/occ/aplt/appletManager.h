/******************************************************************************
// @file appletManager.h
// @brief OCC Applet Manager header file
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section _appletManager_h appletManager.h
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *                      tapiar    07/15/2011  Created applet manager methods
 *   @pb004             pbavari   09/12/2011  Test Applet and initialization
 *                                            section support
 *   @02                tapiar    10/03/2011  Changed BOOLEAN to bool
 *   @pb009             pbavari   10/20/2011  Return void for initAppletManager
 *   @pb00A             pbavari   11/14/2011  Moved out sensor_init from applet
 *   @th005             thallet   11/23/2011  Added querySensorList applet
 *   @pb00B             pbavari   12/13/2011  Fixed runApplet non-blocking call
 *   @dw000             dwoodham  12/13/2011  Moved OCC_APLT to incl/appletId.h
 *   @rc003             rickylie  02/03/2012  Verify & Clean Up OCC Headers & Comments
 *   @pb00E             pbavari   03/11/2012  Added correct include file
 *   @pb00F             pbavari   06/11/2012  Removed redefinition of applet address
 *   @th029             thallet   01/23/2013  Moved struct & extern from c file
 *
 *  @endverbatim
 *
 *///*************************************************************************/

#ifndef _appletManager_H
#define _appletManager_H

/** \defgroup OCC Applet Manager Component
 * 
 */

//*************************************************************************
// Includes
//*************************************************************************
//@pb00Ec - changed from common.h to occ_common.h for ODE support
#include <occ_common.h>
#include "errl.h"
#include <appletId.h>  // @dw000a

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************
// @dw000d Moved OCC_APLT to incl/appletId.h

//@pb00B - Changed to have more specific status codes
// These are the status codes that the applet manager code may return.
typedef enum
{
    OCC_APLT_SUCCESS             = 0x00,
    OCC_APLT_PRE_START_FAILURE   = 0x01,
    OCC_APLT_POST_START_FAILURE  = 0x02,
    OCC_APLT_EXECUTE_FAILURE     = 0x03,

} OCC_APLT_STATUS_CODES;

// Applet types
typedef enum
{
    APLT_TYPE_PRODUCT = 0x00,
    APLT_TYPE_TEST    = 0x01,
    APLT_TYPE_INVALID = 0xFF
}OCC_APLT_TYPE;

//@pb004m - moved ApltAddress_t to appletManager.c as it was used internally

//*************************************************************************
// Structures
//*************************************************************************
// Applet address/size structure
typedef struct
{
    uint32_t iv_aplt_address;
    uint32_t iv_size;

} ApltAddress_t;   // @th029a

//*************************************************************************
// Globals
//*************************************************************************
// Currently NOT externalized, only used in appletManager.c

// Used as a way to pass in parameters to each applet
// Applet Manager will copy arguments passed in into this 
// global and pass them to applet
//extern void *         G_ApltParms;

extern ApltAddress_t  G_ApltAddressTable[ OCC_APLT_LAST ];   // @th029a

//*************************************************************************
// Function Prototypes
//*************************************************************************
//@pb00Bc - changed to return void as return code and o_status was returning
// same thing. This way caller can avoid checking for return code and o_status
// which were same.
/* run an applet */
void runApplet(
    const OCC_APLT  i_applet,
    void *          i_parms,
    const bool      i_block,
    SsxSemaphore   *io_appletComplete,
    errlHndl_t     *o_errHndl,
    OCC_APLT_STATUS_CODES * o_status);

/* initialize the applet manager */
//@pb004c - Changed to add init section
//@pb009c - Changed to return void similar to other calls from main thread
void initAppletManager( void ) INIT_SECTION;

//*************************************************************************
// Functions
//*************************************************************************

#endif //_appletManager_H
