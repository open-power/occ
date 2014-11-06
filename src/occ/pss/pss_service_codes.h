/******************************************************************************
// @file pss_service_codes.h
// @brief Error codes for pss component.
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section _pss_service_codes_h pss_service_codes.h
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *   @cc000             cjcain    08/04/2011  Created             
 *   @dw001             dwoodham  09/22/2011  Added code for dpss
 *   @rc003             rickylie  02/03/2012  Verify & Clean Up OCC Headers & Comments
 *   @pb00E             pbavari   03/11/2012  Added correct include file
 *   @nh001             neilhsu   05/23/2012  Add missing error log tags 
 *   @gm037  925908     milesg    05/07/2014  Redundant OCC/APSS support
 *
 *  @endverbatim
 *
 *///*************************************************************************/

#ifndef _PSS_SERVICE_CODES_H_
#define _PSS_SERVICE_CODES_H_

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
enum pssModuleId  // @dw001c
{
    PSS_MID_APSS_INIT              =  PSS_COMP_ID | 0x00,
    PSS_MID_APSS_START_MEAS        =  PSS_COMP_ID | 0x01,
    PSS_MID_APSS_CONT_MEAS         =  PSS_COMP_ID | 0x02,
    PSS_MID_APSS_COMPLETE_MEAS     =  PSS_COMP_ID | 0x03,
    PSS_MID_DPSS_INIT              =  PSS_COMP_ID | 0x04,  // @dw001a
    PSS_MID_DPSS_OVS_IRQ_INIT      =  PSS_COMP_ID | 0x05,  // @dw001a
    PSS_MID_DPSS_RD_STATUS         =  PSS_COMP_ID | 0x06,  // @dw001a
    PSS_MID_DO_APSS_RECOVERY       =  PSS_COMP_ID | 0x07,  // @gm037
};

// move return code to occ_service_code.h as ERC codes  // @nh001d


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

#endif /* #ifndef _PSS_SERVICE_CODES_H_ */
