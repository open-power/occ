/******************************************************************************
// @file thread_service_codes.h
// @brief Error codes for thread component.
*
*  Note that the thread component will not actually generate errors
*  using the thread component.  This is due to the fact that  will
*  use trace so therefore we can induce non-ending recursion if trace
*  were to use ERRL.
*
*  A trace failure is not a system critical failure which is why we
*  can get away with it not reporting errors.
*
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section _thread_service_codes_h thread_service_codes.h
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *                      TEAM      06/16/2010  Port  
 *   @rc003             rickylie  02/03/2012  Verify & Clean Up OCC Headers & Comments
 *   @pb00E             pbavari   03/11/2012  Added correct include file
 *   @jh005  894560     joshych   08/14/2013  Create call home data logs every 24 hours
 *   @gs020  909320     gjsilva   12/12/2013  Support for VR_FAN thermal control
 *   @gs021  909855     gjsilva   12/18/2013  Support for processor OT condition
 *   @at023  910877     alvinwan  01/09/2014  Excessive fan increase requests error for mfg
 *   @fk006  914801     fmkassem  02/05/2014  Remove reason code list and merge with occ_service_codes list
 *
 *  @endverbatim
 *
 *///*************************************************************************/
 
#ifndef _THREAD_SERVICE_CODES_H_
#define _THREAD_SERVICE_CODES_H_

//*************************************************************************
// Includes
//*************************************************************************
#include <comp_ids.h>

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************
enum thrdModuleId
{
    THRD_MID_INIT_THREAD_SCHDLR     =  THRD_COMP_ID | 0x00,
    THRD_MID_THREAD_SWAP_CALLBACK   =  THRD_COMP_ID | 0x01,
    THRD_MID_GEN_CALLHOME_LOG       =  THRD_COMP_ID | 0x02,
    THRD_THERMAL_VRM_FAN_CONTROL    =  THRD_COMP_ID | 0x03,
    THRD_THERMAL_MAIN               =  THRD_COMP_ID | 0x04, // @at023a
};

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

#endif /* #ifndef _THREAD_SERVICE_CODES_H_ */
