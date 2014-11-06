/******************************************************************************
// @file dcom_service_codes.h
// @brief Error codes for dcom component.
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section _dcom_service_codes_h dcom_service_codes.h
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *                      abagepa   09/19/2011  Created new file    
 *   @01                tapiar    10/09/2011  State 2 updates
 *   @02                tapiar    11/09/2011  State 3 updates
 *   @rc003             rickylie  02/03/2012  Verify & Clean Up OCC Headers & Comments
 *   @pb00E             pbavari   03/11/2012  Added correct include file
 *   @gm037  925908     milesg    05/07/2014  Redundant OCC/APSS support
 *
 *  @endverbatim
 *
 *///*************************************************************************/

#ifndef _DCOM_SERVICE_CODES_H_
#define _DCOM_SERVICE_CODES_H_

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
enum dcomModuleId
{
    DCOM_MID_INIT_ROLES             =  DCOM_COMP_ID | 0x00,
    DCOM_MID_TASK_RX_SLV_INBOX      =  DCOM_COMP_ID | 0x01,
    DCOM_MID_TASK_TX_SLV_INBOX      =  DCOM_COMP_ID | 0x02,
    DCOM_MID_INIT_PBAX_QUEUES       =  DCOM_COMP_ID | 0x03,
    DCOM_MID_TASK_RX_SLV_OUTBOX     =  DCOM_COMP_ID | 0x04, //@01a
    DCOM_MID_TASK_TX_SLV_OUTBOX     =  DCOM_COMP_ID | 0x05, //@01a
    DCOM_MID_SLV_OUTBOX_TX_DOORBELL =  DCOM_COMP_ID | 0x06, //@01a
    DCOM_MID_TASK_WAIT_FOR_MASTER   =  DCOM_COMP_ID | 0x07, //@01a
    DCOM_MID_ERROR_CHECK            =  DCOM_COMP_ID | 0x08, //@02a
    DCOM_MID_WAIT_FOR_MASTER        =  DCOM_COMP_ID | 0x09, //@gm037
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

#endif /* #ifndef _DCOM_SERVICE_CODES_H_ */
