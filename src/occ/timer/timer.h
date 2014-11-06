/******************************************************************************
// @file timer.h
// @brief OCC watchdog timer functions and defines.
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section timer.h TIMER.H
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *   @pb007             pbavari   09/27/2011  Created
 *   @pb008             pbavari   10/04/2011  Combined watchdog reset tasks
 *   @rc003             rickylie  02/03/2012  Verify & Clean Up OCC Headers & Comments
 *   @th022             thallet   10/03/2012  Moved task flags to RTLS files
 *   @sb002  908891     sbroyles  12/09/2013  FFDC updates
 *   @sb022  910404     sbroyles  01/06/2014  Extend watchdog timeout
 *
 *  @endverbatim
 *
 *///*************************************************************************/

#ifndef _timer_h
#define _timer_h

//*************************************************************************
// Includes
//*************************************************************************
#include <common_types.h>        // defines for uint8_t,uint3_t..etc
#include <rtls.h>                // For RTL task
#include <errl.h>                // For errlHndl_t
#include <ssx_app_cfg.h>

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Macros
//*************************************************************************
// @sb022 Macro to switch watchdog on and off
#define ENABLE_WDOG G_wdog_enabled = TRUE
#define DISABLE_WDOG G_wdog_enabled = FALSE
#define WDOG_ENABLED (G_wdog_enabled)

//*************************************************************************
// Defines/Enums
//*************************************************************************

//*************************************************************************
// Structures
//*************************************************************************

//*************************************************************************
// Globals
//*************************************************************************
// @sb022 Variable to switch watchdog on and off
extern bool G_wdog_enabled;

//*************************************************************************
// Function Prototypes
//*************************************************************************
// Initialize watchdog timers
void initWatchdogTimers() INIT_SECTION;

// Task to reset PPC405 watchdog timer and reset OCB timer
void task_poke_watchdogs( struct task * i_self );

//*************************************************************************
// Functions
//*************************************************************************

#endif // _timer_h


