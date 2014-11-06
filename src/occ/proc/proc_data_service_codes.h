/******************************************************************************
// @file proc_data_service_codes.h
// @brief Error codes for proc component.
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section _proc_data_service_codes_h proc_data_service_codes.h
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *                      nguyenp   09/23/2011  Created 
 *   @01                nguyenp   10/19/2011  Added task fast core data module id.
 *   @rc003             rickylie  02/03/2012  Verify & Clean Up OCC Headers & Comments
 *   @pb00E             pbavari   03/11/2012  Added correct include file
 *   @th032             thallet   04/26/2013  Tuleta HW Bringup         
 *   @gm006  SW224414   milesg    09/16/2013  Reset and FFDC improvements 
 *   @gm025  915973     milesg    02/14/2014  Full support for sapphire (KVM) mode
 *
 *  @endverbatim
 *
 *///*************************************************************************/

#ifndef _PROC_DATA_SERVICE_CODES_H_
#define _PROC_DATA_SERVICE_CODES_H_

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
enum procModuleId
{
	PROC_TASK_CORE_DATA_MOD         = PROC_COMP_ID | 0x00,
	PROC_CORE_INIT_MOD              = PROC_COMP_ID | 0x01,
	PROC_TASK_FAST_CORE_DATA_MOD	= PROC_COMP_ID | 0x02,	//@01a
    PROC_GPST_INIT_FAILURE_MOD      = PROC_COMP_ID | 0x03,
    PROC_ENABLE_PSTATES_SMH_MOD     = PROC_COMP_ID | 0x04, //@gm006
    PROC_PSTATE_KVM_SETUP_MOD       = PROC_COMP_ID | 0x05, //@gm025
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

#endif /* #ifndef _PROC_DATA_SERVICE_CODES_H_ */
