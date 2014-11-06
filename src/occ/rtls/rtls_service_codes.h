/******************************************************************************
// @file rtls_service_codes.h
// @brief Error codes for rtls component.
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section _rtls_service_codes_h rtls_service_codes.h
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *                      np, dw    08/10/2011  created by nguyenp & dwoodham
 *   @rc003             rickylie  02/03/2012  Verify & Clean Up OCC Headers & Comments
 *   @pb00E             pbavari   03/11/2012  Added correct include file
 *
 *  @endverbatim
 *
 *///*************************************************************************/
 
#ifndef _RTLS_SERVICE_CODES_H_
#define _RTLS_SERVICE_CODES_H_

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
enum rtlsModuleId
{
	RTLS_OCB_INIT_MOD		= RTLS_COMP_ID | 0x00,
	RTLS_DO_TICK_MOD		= RTLS_COMP_ID | 0x01,
	RTLS_START_TASK_MOD		= RTLS_COMP_ID | 0x02,
	RTLS_STOP_TASK_MOD		= RTLS_COMP_ID | 0x03,
	RTLS_TASK_RUNABLE_MOD		= RTLS_COMP_ID | 0x04,
	RTLS_SET_TASK_DATA_MOD		= RTLS_COMP_ID | 0x05,
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

#endif /* #ifndef _RTLS_SERVICE_CODES_H_ */
