/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/timer/timer.h $                                   */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2016                        */
/* [+] International Business Machines Corp.                              */
/*                                                                        */
/*                                                                        */
/* Licensed under the Apache License, Version 2.0 (the "License");        */
/* you may not use this file except in compliance with the License.       */
/* You may obtain a copy of the License at                                */
/*                                                                        */
/*     http://www.apache.org/licenses/LICENSE-2.0                         */
/*                                                                        */
/* Unless required by applicable law or agreed to in writing, software    */
/* distributed under the License is distributed on an "AS IS" BASIS,      */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or        */
/* implied. See the License for the specific language governing           */
/* permissions and limitations under the License.                         */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */

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
// Macro to switch watchdog on and off
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
// Variable to switch watchdog on and off
extern bool G_wdog_enabled;

//*************************************************************************
// Function Prototypes
//*************************************************************************
// Initialize watchdog timers
void initWatchdogTimers() INIT_SECTION;

// Task to reset PPC405 watchdog timer and reset OCB timer
void task_poke_watchdogs(struct task *i_self);

void manage_mem_deadman_task(void);
void init_mem_deadman_reset_task(void);
void check_pgpe_beacon(void);

//*************************************************************************
// Functions
//*************************************************************************

#endif // _timer_h


