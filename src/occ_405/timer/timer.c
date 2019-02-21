/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/timer/timer.c $                                   */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2019                        */
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
#include <dimm_structs.h>
#include <occ_sys_config.h>
#include <pgpe_shared.h>
#include <sensor.h>

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

// memory deadman is a per port timer that the MCU uses to verify that
// the memory's power and thermal are properly monitored. The memory deadman
// timers can be programmed 100 ms to 28 s. Reading the deadman timer's SCOM
// register resets its value. If the OCC fails to reset the deadman SCOM
// and the timer is expired, emergency throttle mode will be enforced.
GpeRequest G_reset_mem_deadman_request;                              // IPC request
GPE_BUFFER(reset_mem_deadman_args_t G_gpe_reset_mem_deadman_args);   // IPC args

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
// Name: init_mem_deadman_reset_task
//
// Description:
//
// End Function Specification
void init_mem_deadman_reset_task(void)
{
    errlHndl_t l_err = NULL;
    int        rc = 0;

    // Initialize memory deadman timer reset task arguments
    G_gpe_reset_mem_deadman_args.error.error = 0;
    G_gpe_reset_mem_deadman_args.error.ffdc  = 0;
    G_gpe_reset_mem_deadman_args.mca         = 0;

    TRAC_INFO("init_mem_deadman_reset_task: Creating request for GPE deadman reset task");
    rc = gpe_request_create(&G_reset_mem_deadman_request,       // request
                       &G_async_gpe_queue1,                     // GPE1 queue
                       IPC_ST_RESET_MEM_DEADMAN,                // Function ID
                       &G_gpe_reset_mem_deadman_args,           // GPE argument_ptr
                       SSX_SECONDS(5),                          // timeout
                       NULL,                                    // callback
                       NULL,                                    // callback arg
                       ASYNC_CALLBACK_IMMEDIATE);               // options

    // If we couldn't create the GpeRequest objects, there must be a major problem
    // so we will log an error and halt OCC.
    if(rc)
    {
        //Failed to create GpeRequest object, log an error.
        TRAC_ERR("Failed to create memory deadman GpeRequest object[0x%x]", rc);

        /* @
         * @errortype
         * @moduleid    INIT_WD_TIMERS
         * @reasoncode  GPE_REQUEST_CREATE_FAILURE
         * @userdata1   gpe_request_create return code
         * @userdata2   0
         * @userdata4   OCC_NO_EXTENDED_RC
         * @devdesc     Failure to create GpeRequest object for
         *              memory deadman reset IPC task.
         *
         */
        l_err = createErrl(
            INIT_WD_TIMERS,                     //modId
            GPE_REQUEST_CREATE_FAILURE,         //reasoncode
            OCC_NO_EXTENDED_RC,                 //Extended reason code
            ERRL_SEV_PREDICTIVE,                //Severity
            NULL,                               //Trace Buf
            DEFAULT_TRACE_SIZE,                 //Trace Size
            rc,                                 //userdata1
            0                                   //userdata2
            );

        REQUEST_RESET(l_err);   //This will add a firmware callout for us
    }

}

// Function Specification
//
// Name: task_poke_watchdogs
//
// Description: Called every 500us on both master and slaves while in observation
//               and active state. It performs the following:
//               1. Enable/Reset the OCC heartbeat, setting the count to 8ms.
//               2. Reset memory deadman timer for 1 MCA (by a GPE1 IPC task).
//               3. Every 4ms (every other time called):
//                  Verify PGPE is still functional by reading PGPE Beacon from
//                  SRAM if after 8ms (2 consecutive checks) there is no change
//                  to the PGPE Beacon count then log an error and request reset.
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

// 2. Reset memory deadman timer
    if(G_sysConfigData.mem_type == MEM_TYPE_NIMBUS)
    {
        manage_mem_deadman_task();
    }

// 3. Verify PGPE Beacon is not frozen for 8 ms if there are cores configured
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
}

// Function Specification
//
// Name: manage_mem_deadman_task
//
// Description: Verify that if a memory deadman_task was scheduled on GPE1 last cycle
//              then it is completed. Then if there is a new task to be scheduled
//              for this cycle, then schedule it on the GPE1 engine. This task
//              is also used to collect memory performance measurements.
//              Called every 500us.  This should only be called on NIMBUS!
//
// End Function Specification

// MAX number of timeout cycles allowed for memory deadman IPC task
// before logging an error
#define MEM_DEADMAN_TASK_TIMEOUT 8

void manage_mem_deadman_task(void)
{
    //if a task is scheduled, verify that it is completed ...
    //track # of consecutive failures on a specific RDIMM
    static uint8_t L_scom_timeout[NUM_NIMBUS_MCAS] = {0};

    errlHndl_t     l_err     = NULL; // Error handler
    int            rc        = 0;    // Return code
    uint8_t        mca;              // MCA of last memory deadman task (scheduled/not-configured)
    static bool    L_gpe_scheduled      = false;
    static bool    L_gpe_idle_traced    = false;
    static bool    L_gpe_timeout_logged = false;
    static bool    L_gpe_had_1_tick     = false;
    static bool    L_init_complete      = false;

    if(!L_init_complete)
    {
       // Initialize the GPE1 IPC task that resets the deadman timer.
       init_mem_deadman_reset_task();
       L_init_complete = true;
    }

    // There is no way to synchronously start or stop the counter, so must
    // take the difference between two readings.
    static perf_mon_count0_t L_last_reading[NUM_NIMBUS_MCAS] = {{0}};
    uint32_t l_rd_wr_diff = 0;

    uint32_t       gpe_rc = G_gpe_reset_mem_deadman_args.error.rc;  // IPC task rc

    do
    {   // mca of last memory deadman task (either not-configured or scheduled).
        mca = G_gpe_reset_mem_deadman_args.mca;

        //First, check to see if the previous GPE request still running
        if( !(async_request_is_idle(&G_reset_mem_deadman_request.request)) )
        {
            L_scom_timeout[mca]++;
            //This can happen due to variability in when the task runs
            if(!L_gpe_idle_traced && L_gpe_had_1_tick)
            {
                TRAC_INFO("manage_mem_deadman_task: GPE is still running. mca[%d]", mca);
                L_gpe_idle_traced = true;
            }
            L_gpe_had_1_tick = true;
            break;
        }
        else
        {
            //Request is idle
            L_gpe_had_1_tick = false;
            if(L_gpe_idle_traced)
            {
                TRAC_INFO("manage_mem_deadman_task: GPE completed. mca[%d]", mca);
                L_gpe_idle_traced = false;
            }
        }

        //check scom status
        if(L_gpe_scheduled)
        {
            if(!async_request_completed(&G_reset_mem_deadman_request.request) || gpe_rc)
            {
                //Request failed. Keep count of failures and log an error if we reach a
                //max retry count
                L_scom_timeout[mca]++;
                if(L_scom_timeout[mca] >= MEM_DEADMAN_TASK_TIMEOUT)
                {
                    break;
                }

            }
            else // A Task was scheduled last cycle, completed successfully, no errors
            {
                //Reset the timeout.
                L_scom_timeout[mca] = 0;

                // Update read/write sensors
                l_rd_wr_diff = G_gpe_reset_mem_deadman_args.rd_wr_counts.mba_read_cnt
                     - L_last_reading[mca].mba_read_cnt;
                l_rd_wr_diff = ((l_rd_wr_diff*25)/1000);
                sensor_update(AMECSENSOR_ARRAY_PTR(MRDM0,mca), l_rd_wr_diff);

                l_rd_wr_diff = G_gpe_reset_mem_deadman_args.rd_wr_counts.mba_write_cnt
                     - L_last_reading[mca].mba_write_cnt;
                l_rd_wr_diff = ((l_rd_wr_diff*25)/1000);
                sensor_update(AMECSENSOR_ARRAY_PTR(MWRM0,mca), l_rd_wr_diff);
                // Update last read/write measurement
                L_last_reading[mca] = G_gpe_reset_mem_deadman_args.rd_wr_counts;
            }
        }

        //The previous GPE job completed. Now get ready for the next job.
        L_gpe_scheduled = false;


        //We didn't fail, update mca (irrespective of whether it will be scheduled)
        mca++;
        if ( mca >= NUM_NIMBUS_MCAS )
        {
            mca  = 0;
        }
        G_gpe_reset_mem_deadman_args.mca = mca;

        // If the MCA is not configured, break
        if(!NIMBUS_DIMM_INDEX_THROTTLING_CONFIGURED(mca))
        {
            break;
        }

        // The MCA is configured, and the previous IPC task completed successfully
        rc = gpe_request_schedule(&G_reset_mem_deadman_request);

        // Always log an error if gpe request schedule fails
        if( rc )
        {
            //Error in schedule gpe memory deadman reset task
            TRAC_ERR("manage_mem_deadman_task: Failed to schedule memory deadman reset task rc=%x",
                     rc);

            /* @
             * @errortype
             * @moduleid    POKE_WD_TIMERS
             * @reasoncode  GPE_REQUEST_SCHEDULE_FAILURE
             * @userdata1   rc - gpe_request_schedule return code
             * @userdata2   0
             * @userdata4   OCC_NO_EXTENDED_RC
             * @devdesc     OCC Failed to schedule a GPE job for memory deadman reset
             */
            l_err = createErrl(
                POKE_WD_TIMERS,                         // modId
                GPE_REQUEST_SCHEDULE_FAILURE,           // reasoncode
                OCC_NO_EXTENDED_RC,                     // Extended reason code
                ERRL_SEV_UNRECOVERABLE,                 // Severity
                NULL,                                   // Trace Buf
                DEFAULT_TRACE_SIZE,                     // Trace Size
                rc,                                     // userdata1
                0                                       // userdata2
                );

            addUsrDtlsToErrl(
                l_err,                                            //io_err
                (uint8_t *) &(G_reset_mem_deadman_request.ffdc),  //i_dataPtr,
                sizeof(G_reset_mem_deadman_request.ffdc),         //i_size
                ERRL_USR_DTL_STRUCT_VERSION_1,                    //version
                ERRL_USR_DTL_BINARY_DATA);                        //type

            REQUEST_RESET(l_err);   //This will add a firmware callout for us
            break;

        }

        // Successfully scheduled a new memory deadman timer gpe IPC request
        L_gpe_scheduled = true;

    } while(0);


    if(L_scom_timeout[mca] >= MEM_DEADMAN_TASK_TIMEOUT && L_gpe_timeout_logged == false)
    {
        TRAC_ERR("manage_mem_deadman_task: Timeout scomming MCA[%d]", mca);

        /* @
         * @errortype
         * @moduleid    POKE_WD_TIMERS
         * @reasoncode  GPE_REQUEST_TASK_TIMEOUT
         * @userdata1   mca number
         * @userdata2   0
         * @userdata4   OCC_NO_EXTENDED_RC
         * @devdesc     Timed out trying to reset the memory deadman timer.
         */

        l_err = createErrl(
                POKE_WD_TIMERS,                         // modId
                GPE_REQUEST_TASK_TIMEOUT,               // reasoncode
                OCC_NO_EXTENDED_RC,                     // Extended reason code
                ERRL_SEV_PREDICTIVE,                    // Severity
                NULL,                                   // Trace Buf
                DEFAULT_TRACE_SIZE,                     // Trace Size
                mca,                                    // userdata1
                0                                       // userdata2
                );

        addUsrDtlsToErrl(l_err,                                   //io_err
                (uint8_t *) &(G_reset_mem_deadman_request.ffdc),  //i_dataPtr,
                sizeof(G_reset_mem_deadman_request.ffdc),         //i_size
                ERRL_USR_DTL_STRUCT_VERSION_1,                    //version
                ERRL_USR_DTL_BINARY_DATA);                        //type

        // Commit Error Log
        commitErrl(&l_err);

        L_gpe_timeout_logged = true;
    }

    return;
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
    errlHndl_t      l_err                       = NULL;  // Error handler
    static bool L_unchanged_traced = false;
    static int  L_unchanged_count = 0;

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
                if(L_unchanged_traced == false)
                {
                    TRAC_ERR("Error PGPE Beacon didnt change for 4ms: 0x%08X", pgpe_beacon);
                    L_unchanged_traced = true;
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
                if (L_unchanged_traced)
                {
                    TRAC_ERR("check_pgpe_beacon: PGPE Beacon changed: 0x%08X (after %d calls)", pgpe_beacon, L_unchanged_count);
                    L_unchanged_traced = false;
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
