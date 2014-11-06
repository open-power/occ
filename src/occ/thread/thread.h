/******************************************************************************
// @file thread.h
// @brief OCC THREAD COMPONENT header file
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section _thread_h thread.h
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *                      abagepa   07/15/2011  Created Threads definition header
 *   @pb004             pbavari   09/12/2011  Test applet thread support
 *   @pb009             pbavari   10/20/2011  Removed unused thread related data
 *   @rc003             rickylie  02/03/2012  Verify & Clean Up OCC Headers & Comments
 *   @pb00E             pbavari   03/11/2012  Added correct include file
 *   @th022             thallet   10/03/2012  Changes to allow DCOM State/Mode setting
 *
 *  @endverbatim
 *
 *///*************************************************************************/
 
#ifndef _THREAD_H
#define _THREAD_H

//*************************************************************************
// Includes
//*************************************************************************
//@pb00Ec - changed from common.h to occ_common.h for ODE support
#include <occ_common.h>
#include "ssx.h"

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************
// Thread priorities for Thread creation.
typedef enum
{
    THREAD_PRIORITY_0, //reserved for high priority
    THREAD_PRIORITY_1, //reserved for high priority
    THREAD_PRIORITY_2,
    THREAD_PRIORITY_3,
    THREAD_PRIORITY_4,
    THREAD_PRIORITY_5,
    THREAD_PRIORITY_6,
    THREAD_PRIORITY_7,
    THREAD_PRIORITY_8,
}THREAD_PRIORITY;

/*----------------------------------------------------------------------------*/
/* Stack Size and Declaration                                                 */
/*----------------------------------------------------------------------------*/

/** \defgroup Stack Stack Information
 * Stack sizes are defined by entity
 * - Non-Critical Stack used by non-critical interrupt handlers, including timer callbacks
 * - Critical Stack used for critical interrupts
 * - Stacks for each thread
 */

#define NONCRITICAL_STACK_SIZE 8192    // 8kB
#define CRITICAL_STACK_SIZE    4096    // 4kB
#define THREAD_STACK_SIZE      4096    // 4kB

//*************************************************************************
// Structures
//*************************************************************************

//*************************************************************************
// Globals
//*************************************************************************
/*@{*/
extern uint8_t main_thread_stack[THREAD_STACK_SIZE];
extern uint8_t Cmd_hndl_thread_stack[THREAD_STACK_SIZE];
extern uint8_t App_thread_stack[THREAD_STACK_SIZE];
//@pb004a - added
extern uint8_t testAppletThreadStack[THREAD_STACK_SIZE];

/*@}*/  // Ending tag for stack module in doxygen

/*----------------------------------------------------------------------------*/
/*SsxTimer and SsxThread Declarations                                         */
/*----------------------------------------------------------------------------*/
/** \defgroup TimersAndThreads Timer and Thread Information */ /*@{*/

/// Our timer based on TIMER_INTERVAL that kicks off most of the work.  See #timer_routine
//SsxTimer timer;  // @tgh001 -- unused

/// Our idle thread.  See #main_thread_routine  
extern SsxThread Main_thread;

//Command handler thread
extern SsxThread Cmd_Hndl_thread;

//Application manager thread  
extern SsxThread App_thread;  

//@pb004a - Test applet thread
extern SsxThread TestAppletThread;

//Application manager thread  
extern SsxThread Dcom_thread;  

/*@}*/  // Ending tag for TimersAndThreads module in doxygen

//*************************************************************************
// Function Prototypes
//*************************************************************************
void Main_thread_routine(void *private);

void Cmd_Hndl_thread_routine(void *arg);

void App_thread_routine(void *arg);

void Dcom_thread_routine(void *arg);

//@pb004a -added
void testAppletThreadRoutine(void *arg);

//*************************************************************************
// Functions
//*************************************************************************

#endif //_THREAD_H
