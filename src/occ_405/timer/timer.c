/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/timer/timer.c $                                   */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2020                        */
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
#include <occhw_common.h>           // PGP common defines
#include <occhw_ocb.h>              // OCB timer interfaces
#include <occ_service_codes.h>      // Reason codes
#include <timer_service_codes.h>    // Module Id
#include <cmdh_fsp.h>               // for RCs in the checkpoint macros
#include <occ_sys_config.h>
#include <pgpe_shared.h>
#include <sensor.h>
#include <common.h>

//*************************************************************************/
// Externs
//*************************************************************************/
// Variable holding main thread loop count
extern uint32_t G_mainThreadLoopCounter;
// Running in simics?
extern bool G_simics_environment;
// bit mask of configured cores
extern uint32_t G_present_cores;
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
bool G_wdog_enabled = false;
extern uint8_t G_occ_interrupt_type;
bool G_htmgt_notified_of_error = false;

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
                           DEFAULT_TRACE_SIZE,             //Trace Size
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

    // We can't initialize memory here since we do not know what type of memory we have
    // until comm is established with (H)TMGT to tell us
}

// Function Specification
//
// Name: task_poke_watchdogs
//
// Description: Called every 500us on both master and slaves while in observation
//               and active state. It performs the following:
//               1. Enable/Reset the OCC heartbeat, setting the count to 8ms.
//               2. Every 4ms (every other time called):
//                  Verify PGPE is still functional by reading PGPE Beacon from
//                  SRAM if after 8ms (2 consecutive checks) there is no change
//                  to the PGPE Beacon count then log an error and request reset.
//               3. Check for PGPE error in shared SRAM
//
// End Function Specification
#define TASK_PGPE_BEACON_RUN_TICK_COUNT 8
void task_poke_watchdogs(struct task * i_self)
{
    ocb_occhbr_t hbr;                          // OCC heart beat register
    static int L_check_pgpe_beacon_count = 0;  // Check GPE beacon this time?

// 1. Enable OCC heartbeat

    hbr.fields.occ_heartbeat_count = 8000; // count corresponding to 8 ms
    hbr.fields.occ_heartbeat_en   = true;  // enable heartbeat timer
    out32(OCB_OCCHBR, hbr.value);             // Enable heartbeat register, and set it

// 2. Verify PGPE Beacon is not frozen for 8 ms if there are cores configured
    if(G_present_cores != 0)
    {
        if(L_check_pgpe_beacon_count >= TASK_PGPE_BEACON_RUN_TICK_COUNT)
        {
            // Examine pgpe Beacon every 4ms
            if(!G_simics_environment) // PGPE Beacon is not implemented in simics
            {
                check_pgpe_beacon();
            }
            L_check_pgpe_beacon_count = 0;
        }
        else
        {
            L_check_pgpe_beacon_count++;
        }
    }

// 3. Check if PGPE has an error to report (non-FSP systems)
    if ((G_occ_interrupt_type != FSP_SUPPORTED_OCC) && (G_htmgt_notified_of_error == false))
    {
        unsigned int index = 0;
        for (; index < G_hcode_elog_table_slots; ++index)
        {
            if (in64(&G_hcode_elog_table[index]) != 0)
            {
                // Found HCODE elog
                notify_host(INTR_REASON_HTMGT_SERVICE_REQUIRED);
                G_htmgt_notified_of_error = true;
                break;
            }
        }
    }
}

// Function Specification
//
// Name: check_pgpe_beacon
//
// Description: Checks the PGPE Beacon every 4ms
//              logs an error and resets if it
//              doesn't change for 8 ms
//
// End Function Specification

void check_pgpe_beacon(void)
{
    uint32_t        pgpe_beacon;                         // PGPE Beacon value now
    static uint32_t L_prev_pgpe_beacon          = 0;     // PGPE Beacon value 4 ms ago
    static bool     L_first_pgpe_beacon_check   = true;  // First time examining Beacon?
    static bool     L_pgpe_beacon_unchanged_4ms = false; // pgpe beacon unchanged once (4ms)
    static bool     L_error_logged              = false; // trace and error log only once
    errlHndl_t      l_err = NULL;  // Error handler
    static bool L_unchanged_traced = false;
    static int  L_unchanged_count = 0;
    static unsigned int L_trace_change_limit = 6;

    do
    {
        // return PGPE Beacon (use non-cachable address)
        pgpe_beacon = in32(G_pgpe_header.beacon_sram_addr & 0xF7FFFFFF);

        // in first invocation, just initialize L_prev_pgpe_beacon
        // don't check if the PGPE Beacon value changed
        if(L_first_pgpe_beacon_check)
        {
            L_prev_pgpe_beacon = pgpe_beacon;
            L_first_pgpe_beacon_check = false;
            break;
        }

        // L_prev_pgpe_beacon has been initialized; Every 4ms verify
        // that PGPE Beacon has changed relative to previous reading
        if(pgpe_beacon == L_prev_pgpe_beacon)
        {
            ++L_unchanged_count;
            if(false == L_pgpe_beacon_unchanged_4ms)
            {
                // First time beacon unchaged (4ms), mark flag
                L_pgpe_beacon_unchanged_4ms = true;
                if (L_trace_change_limit > 0)
                {
                    if(L_unchanged_traced == false)
                    {
                        TRAC_ERR("Error PGPE Beacon didnt change for 4ms: 0x%08X", pgpe_beacon);
                        L_unchanged_traced = true;
                    }
                }
                break;
            }
            else
            {
                // Second time beacon unchanged (8ms), log timeout error
                if (false == L_error_logged)
                {
                    TRAC_ERR("Error PGPE Beacon didn't change for 8 ms: 0x%08X", pgpe_beacon);
                    L_error_logged = true;
                    /*
                     * @errortype
                     * @moduleid    POKE_WD_TIMERS
                     * @reasoncode  PGPE_FAILURE
                     * @userdata1   PGPE Beacon Value
                     * @userdata2   PGPE Beacon Address
                     * @userdata4   ERC_PGPE_BEACON_TIMEOUT
                     * @devdesc     PGPE Beacon timeout
                     */
                    l_err = createPgpeErrl(POKE_WD_TIMERS,                  // mod id
                                           PGPE_FAILURE,                    // reason code
                                           ERC_PGPE_BEACON_TIMEOUT,         // Extended reason code
                                           ERRL_SEV_UNRECOVERABLE,          // severity
                                           pgpe_beacon,                     // userdata1
                                           G_pgpe_header.beacon_sram_addr); // userdata2

                    //Add firmware callout
                    addCalloutToErrl(l_err,
                                     ERRL_CALLOUT_TYPE_COMPONENT_ID,
                                     ERRL_COMPONENT_ID_FIRMWARE,
                                     ERRL_CALLOUT_PRIORITY_HIGH);

                    //Add processor callout
                    addCalloutToErrl(l_err,
                                     ERRL_CALLOUT_TYPE_HUID,
                                     G_sysConfigData.proc_huid,
                                     ERRL_CALLOUT_PRIORITY_MED);

                    //Add planar callout
                    addCalloutToErrl(l_err,
                                     ERRL_CALLOUT_TYPE_HUID,
                                     G_sysConfigData.backplane_huid,
                                     ERRL_CALLOUT_PRIORITY_LOW);

                    // Commit error log and request reset
                    REQUEST_RESET(l_err);
                }
            }
        }
        else
        {
            if (L_pgpe_beacon_unchanged_4ms)
            {
                if (L_trace_change_limit > 0)
                {
                    --L_trace_change_limit; // started again (so decrement)
                    if (L_unchanged_traced)
                    {
                        TRAC_ERR("check_pgpe_beacon: PGPE Beacon changed: 0x%08X (after %d calls)", pgpe_beacon, L_unchanged_count);
                        L_unchanged_traced = false;
                    }
                }
            }
            // pgpe beacon changed over the last 4 ms
            L_pgpe_beacon_unchanged_4ms = false;
            L_unchanged_count = 0;
            L_prev_pgpe_beacon = pgpe_beacon;
        }
    } while(0);

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

                OCC_HALT(ERRL_RC_WDOG_TIMER);
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
    OCC_HALT(ERRL_RC_OCB_TIMER);
    TRAC_ERR("Should have halted here due to THndlerFull");
}
