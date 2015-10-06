/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/timer/timer.c $                                   */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2015                        */
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

//*************************************************************************/
// Includes
//*************************************************************************/
#include <timer.h>                  // timer defines
#include "ssx.h"
#include <trac.h>                   // Trace macros
#include <occhw_common.h>             // PGP common defines
#include <occhw_ocb.h>                // OCB timer interfaces
#include <occ_service_codes.h>      // Reason codes
#include <timer_service_codes.h>    // Module Id
#include <cmdh_fsp.h>               // for RCs in the checkpoint macros

//*************************************************************************/
// Externs
//*************************************************************************/
// Variable holding main thread loop count
extern uint32_t G_mainThreadLoopCounter;

//*************************************************************************/
// Macros
//*************************************************************************/
// PPC405 watchdog timer handler
SSX_IRQ_FAST2FULL(ppc405WDTHndler, ppc405WDTHndlerFull);
// OCB timer handler
SSX_IRQ_FAST2FULL(ocbTHndler, ocbTHndlerFull);

//*************************************************************************/
// Defines/Enums
//*************************************************************************/
// Change watchdog reset control to take no action on state TSR[WIS]=1
// and TSR[ENW]=1
// Watchdog reset control set to "No reset"
#define OCC_TCR_WRC 0
// Bump up wdog period to ~1s
#define OCC_TCR_WP 3
// 4ms represented in nanoseconds
#define OCB_TIMER_TIMOUT    4000000

//*************************************************************************/
// Structures
//*************************************************************************/

//*************************************************************************/
// Globals
//*************************************************************************/
bool G_wdog_enabled = FALSE;

//*************************************************************************/
// Function Prototypes
//*************************************************************************/

//*************************************************************************/
// Functions
//*************************************************************************/

// Function Specification
//
// Name: initWatchdogTimers
//
// Description:
//
// End Function Specification
void initWatchdogTimers()
{
    int l_rc = SSX_OK;
    errlHndl_t l_err = NULL;

    TRAC_IMP("Initializing ppc405 watchdog. period=%d, reset_ctrl=%d",
             OCC_TCR_WP,
             OCC_TCR_WRC);

    // set up PPC405 watchdog timer
    l_rc = ppc405_watchdog_setup(OCC_TCR_WP,          // watchdog period
                                 OCC_TCR_WRC,         // watchdog reset control
                                 ppc405WDTHndler,     // interrupt handler
                                 NULL);               // argument to handler

    if (SSX_OK != l_rc)
    {
        TRAC_ERR("Error setting up ppc405 watchdog timer: l_rc: %d",l_rc);
        /*
         * @errortype
         * @moduleid    INIT_WD_TIMERS
         * @reasoncode  INTERNAL_HW_FAILURE
         * @userdata1   Return code of PPC405 watchdog timer setup
         * @userdata4   ERC_PPC405_WD_SETUP_FAILURE
         * @devdesc     Failure on hardware related function
         */
        l_err = createErrl(INIT_WD_TIMERS,                 // mod id
                           INTERNAL_HW_FAILURE,            // reason code
                           ERC_PPC405_WD_SETUP_FAILURE,    // Extended reason code
                           ERRL_SEV_UNRECOVERABLE,         // severity
                           NULL,                           // trace buffer
                           0,                              // trace size
                           l_rc,                           // userdata1
                           0);                             // userdata2

        // Callout firmware
        addCalloutToErrl(l_err,
                         ERRL_CALLOUT_TYPE_COMPONENT_ID,
                         ERRL_COMPONENT_ID_FIRMWARE,
                         ERRL_CALLOUT_PRIORITY_HIGH);

        // Commit error log
        commitErrl(&l_err);
    }

    if (SSX_OK != l_rc)
    {
        TRAC_ERR("Error setting up OCB timer: l_rc: %d",l_rc);
        /*
        * @errortype
        * @moduleid    INIT_WD_TIMERS
        * @reasoncode  INTERNAL_HW_FAILURE
        * @userdata1   Return code of OCB timer setup
        * @userdata4   ERC_OCB_WD_SETUP_FAILURE
        * @devdesc     Failure on hardware related function
        */
        l_err = createErrl(INIT_WD_TIMERS,                 // mod id
                           INTERNAL_HW_FAILURE,            // reason code
                           ERC_OCB_WD_SETUP_FAILURE,       // Extended reason code
                           ERRL_SEV_UNRECOVERABLE,         // severity
                           NULL,                           // trace buffer
                           0,                              // trace size
                           l_rc,                           // userdata1
                           0);                             // userdata2

        // Callout firmware
        addCalloutToErrl(l_err,
                         ERRL_CALLOUT_TYPE_COMPONENT_ID,
                         ERRL_COMPONENT_ID_FIRMWARE,
                         ERRL_CALLOUT_PRIORITY_HIGH);

        // Commit error log
        commitErrl(&l_err);
    }
}

// Function Specification
//
// Name: task_poke_watchdogs
//
// Description: Reset PPC405 watchdog timer by clearing TSR[ENW] bit and reset OCB timer
//
// End Function Specification
void task_poke_watchdogs(struct task * i_self)
{
    // Read the PMC status register on every RTL, this is how the OCC
    // generates a PMC hearbeat.
    pmc_status_reg_t psr;
// TMP: Not Ready yet: PMC => PGPE
//    psr.value = in32(PMC_STATUS_REG);
}

// Function Specification
//
// Name: ppc405WDTHndlerFull
//
// Description: PPC405 watchdog interrupt handler
//
// End Function Specification
void ppc405WDTHndlerFull(void * i_arg, SsxIrqId i_irq, int i_priority)
{
    static uint8_t l_wdog_intrpt_cntr = 0;

    // Always reset the watchdog interrupt status in the TSR.  If we halt
    // and leave TSR[WIS]=1 then the watchdog counter will eventually set
    // TSR[ENW]=1 and upon expiration of the next watchdog period the 405 will
    // take whatever action is in TCR[WRC] potentially resetting the OCC while
    // we have it in a halted state, an undesirable outcome.
    // Always clear TSR[ENW,WIS] to reset the watchdog state machine.
    mtspr(SPRN_TSR, (TSR_ENW | TSR_WIS));

    if (WDOG_ENABLED)
    {
        // When enabled, always increment this local static counter
        l_wdog_intrpt_cntr++;
        // The hardware timer should be set to around a second, on the third
        // interrupt we go to halt if the main thread counter hasn't incremented
        // since the last time it was reset.
        if (l_wdog_intrpt_cntr == 3)
        {
            l_wdog_intrpt_cntr = 0;
            // The watchdog interrupt has fired three times, time to check the
            // state of the main thread by looking at the main thread loop
            // counter, it must be non-zero else we will halt the occ
            if (G_mainThreadLoopCounter > 0)
            {
                // The main thread has run at least once in the last ~6 seconds
                G_mainThreadLoopCounter = 0;
            }
            else
            {

// TEMP -- NOT SUPPORTED IN PHASE1
//                OCC_HALT(ERRL_RC_WDOG_TIMER);
TRAC_ERR("Should have halted here due to WDOG");
            }
        }
    }
}

// Function Specification
//
// Name: ocbTHndlerFull
//
// Description: OCB timer interrupt handler
//
// End Function Specification
void ocbTHndlerFull(void * i_arg, SsxIrqId i_irq, int i_priority)
{
    // OCC_HALT with exception code passed in.
// TEMP -- NOT SUPPORTED IN PHASE1
//    OCC_HALT(ERRL_RC_OCB_TIMER);
TRAC_ERR("Should have halted here due to THndlerFull");
}
