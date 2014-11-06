/******************************************************************************
// @file threadSch.h
// @brief OCC THREAD COMPONENT Thread Scheduler header file
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section _threadSch_h threadSch.h
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *                      abagepa   07/15/2011  Created Thread scheduler methods
 *   @01                tapiar    08/05/2011  Moved define to common.h
 *   @pb004             pbavari   09/02/2011  Initialize section support
 *   @pb009             pbavari   10/20/2011  Main thread support
 *   @rc003             rickylie  02/03/2012  Verify & Clean Up OCC Headers & Comments
 *   @pb00E             pbavari   03/11/2012  Added correct include file
 *
 *  @endverbatim
 *
 *///*************************************************************************/

#ifndef _THREADSCH_H
#define _THREADSCH_H

//*************************************************************************
// Includes
//*************************************************************************
//@pb00Ec - changed from common.h to occ_common.h for ODE support
#include <occ_common.h>
#include <thread.h>
#include "ssx.h"
#include <errl.h>

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

//*************************************************************************
// Globals
//*************************************************************************

//*************************************************************************
// Function Prototypes
//*************************************************************************
/* Function to reprioritize the threads in the array */
void threadSwapcallback(void * arg);

/* Function to initilize the threads and the thread schedule timer */
//@pb009c - Changed to return void and handle error internally similar to other
//          calls from main thread
void initThreadScheduler(void) INIT_SECTION;

//@pb009a - Made function available to used by other components (used by main)
/* Function to create and resume thread. Externalizing as it is used to
 * create main thread
 */
int createAndResumeThreadHelper(SsxThread         *io_thread,
                                SsxThreadRoutine  i_thread_routine,
                                void              *io_arg,
                                SsxAddress        i_stack,
                                size_t            i_stack_size,
                                THREAD_PRIORITY   i_priority);
								

//*************************************************************************
// Functions
//*************************************************************************
								
#endif //_THREADSCH_H
