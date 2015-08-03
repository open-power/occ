/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/rtls/rtls.c $                                     */
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

#include "rtls.h"
#include "rtls_service_codes.h"
#include "threadSch.h"          // for DEFAULT_TRACE_SIZE
#include "occ_service_codes.h"  // for SSX_GENERIC_FAILURE
#include <pgp_common.h>         // OCB interrupt controller
#include <pgp_ocb.h>            // OCB timer
#include <errl.h>               // Error logging
#include "amec_external.h"
#include <trac.h>               // Traces

//Macro creates a 'bridge' handler that converts the initial fast-mode to full
//mode interrupt handler
SSX_IRQ_FAST2FULL(rtl_intr_handler, rtl_do_tick);

#define RTLOOP_TIMEOUT_IN_NS (MICS_PER_TICK * 1000)

#define GLOBAL_RUN_MASK RTL_FLAG_RUN | RTL_FLAG_STANDBY | RTL_FLAG_MSTR_READY | RTL_FLAG_APSS_NOT_INITD

// Required flag bits for running
// The run flag is always set
uint32_t G_run_mask = GLOBAL_RUN_MASK;
// Added deferred mask global
uint32_t G_run_mask_deferred = GLOBAL_RUN_MASK;

// The value of the current tick
uint32_t G_current_tick = 0xFFFFFFFF;

// The durations measured within the current tick
fw_timing_t G_fw_timing;

// The global TICK table
// See notes regarding this table in rtls_tables.c.
extern uint8_t *G_tick_table[MAX_NUM_TICKS];

// The global TASK table
// See notes regarding this table in rtls_tables.c.
extern task_t G_task_table[TASK_END];

// ARL Test Code Function from arl_test.c
// Added for enablement of Research use of OCC Environment
//TODO: Remove when no longer needed.
extern void arl_test(void);

// Function Specification
//
// Name: rtl_start_task
//
// Description: Request that a task runs.
//
// End Function Specification

void rtl_start_task(const task_id_t i_task_id)
{
    errlHndl_t l_err = NULL;   // Error handler
    tracDesc_t l_trace = NULL; // Temporary trace descriptor

    if ( i_task_id < TASK_END )
    {
        // Request the task be run
        G_task_table[i_task_id].flags |= RTL_FLAG_RUN;
    }
    else
    {
        // Invalid task ID for this operation
        // TODO use correct trace

        /*
        * @errortype
        * @moduleid    RTLS_START_TASK_MOD
        * @reasoncode  INTERNAL_INVALID_INPUT_DATA
        * @userdata1   Failing task ID
        * @userdata4   OCC_NO_EXTENDED_RC
        * @devdesc     Firmware failure starting task
        */
        l_err = createErrl(
                    RTLS_START_TASK_MOD,            //ModId
                    INTERNAL_INVALID_INPUT_DATA,    //Reasoncode
                    OCC_NO_EXTENDED_RC,             //Extended reasoncode
                    ERRL_SEV_PREDICTIVE,            //Severity
                    l_trace,                        //Trace Buf
                    DEFAULT_TRACE_SIZE,             //Trace Size
                    i_task_id,                      //Userdata1
                    0                               //Userdata2
                    );

        // Commit error log
        commitErrl( &l_err );
    }
}

// Function Specification
//
// Name: rtl_stop_task
//
// Description: Request that a task NOT run
//
// End Function Specification

void rtl_stop_task(const task_id_t i_task_id)
{

    errlHndl_t l_err = NULL;   // Error handler
    tracDesc_t l_trace = NULL; // Temporary trace descriptor

    if ( i_task_id < TASK_END )
    {
        // Request the task NOT be run
        G_task_table[i_task_id].flags &= ~RTL_FLAG_RUN;
    }
    else
    {
        // Invalid task ID for this operation
        // TODO use correct trace

        /*
        * @errortype
        * @moduleid    RTLS_STOP_TASK_MOD
        * @reasoncode  INTERNAL_INVALID_INPUT_DATA
        * @userdata1   Failing task ID
        * @userdata4   OCC_NO_EXTENDED_RC
        * @devdesc     Firmware failure stopping task
        */
        l_err = createErrl(
                    RTLS_STOP_TASK_MOD,             //ModId
                    INTERNAL_INVALID_INPUT_DATA,    //Reasoncode
                    OCC_NO_EXTENDED_RC,             //Extended reasoncode
                    ERRL_SEV_PREDICTIVE,            //Severity
                    l_trace,                        //Trace Buf
                    DEFAULT_TRACE_SIZE,             //Trace Size
                    i_task_id,                      //Userdata1
                    0                               //Userdata2
                    );

        // Commit error log
        commitErrl( &l_err );
    }
}

// Function Specification
//
// Name: rtl_task_is_runnable
//
// Description: Find out if a task can run or not.
//
// End Function Specification
bool rtl_task_is_runnable(const task_id_t i_task_id)
{
    bool task_can_run = FALSE; // Default: task can NOT run
    errlHndl_t l_err = NULL;   // Error handler
    tracDesc_t l_trace = NULL; // Temporary trace descriptor

    if ( i_task_id < TASK_END )
    {
        if ( G_task_table[i_task_id].flags & RTL_FLAG_RUN )
        {
            // Yes, the task CAN run
            task_can_run = TRUE;
        }
    }
    else
    {
        // Invalid task ID for this operation
        // TODO use correct trace

        /*
        * @errortype
        * @moduleid    RTLS_TASK_RUNABLE_MOD
        * @reasoncode  INTERNAL_FAILURE
        * @userdata1   Failing task ID
        * @userdata4   OCC_NO_EXTENDED_RC
        * @devdesc     Firmware failure on task operation
        */
        l_err = createErrl(
                    RTLS_TASK_RUNABLE_MOD,      //ModId
                    INTERNAL_FAILURE,           //Reasoncode
                    OCC_NO_EXTENDED_RC,         //Extended reasoncode
                    ERRL_SEV_UNRECOVERABLE,     //Severity
                    l_trace,                    //Trace Buf
                    DEFAULT_TRACE_SIZE,         //Trace Size
                    i_task_id,                  //Userdata1
                    0                           //Userdata2
                    );

        // Callout firmware
        addCalloutToErrl(l_err,
                         ERRL_CALLOUT_TYPE_COMPONENT_ID,
                         ERRL_COMPONENT_ID_FIRMWARE,
                         ERRL_CALLOUT_PRIORITY_HIGH);

        // Commit error log
        commitErrl( &l_err );
    }

    return task_can_run;
}

// Function Specification
//
// Name: rtl_ocb_init
//
// Description: responsible for initializing the hardware timer in occ control
//              block. It is the timer that supplies the periodic RTL interrupt.
//
// End Function Specification
void rtl_ocb_init(void)
{
    int rc = 0;
    errlHndl_t l_err = NULL;
    tracDesc_t l_trace = NULL;  // Temporary trace descriptor

    // Setup an OCB timer and interrupt handler
    // The ocb_timer_setup will do the following:
    // a) Setup the interrupt
    // b) Setup interrupt handler
    // c) Reset the timer
    // d) Enable the interrupt
    rc = ocb_timer_setup(OCB_TIMER0,
                    OCB_TIMER_AUTO_RELOAD,  //Auto reload
                    RTLOOP_TIMEOUT_IN_NS,   //250000ns or 250us for 1 tick
                    rtl_intr_handler,       //Intr handlers
                    0,
                    SSX_NONCRITICAL);
    if( rc )
    {
        //TODO: Add trace

        /*
        * @errortype
        * @moduleid    RTLS_OCB_INIT_MOD
        * @reasoncode  INTERNAL_HW_FAILURE
        * @userdata1   Return code for ocb_timer_setup
        * @userdata4   OCC_NO_EXTENDED_RC
        * @devdesc     Failure on hardware related function
        */
        l_err = createErrl(
            RTLS_OCB_INIT_MOD,              //ModId
            INTERNAL_HW_FAILURE,            //Reasoncode
            OCC_NO_EXTENDED_RC,             //Extended reasoncode
            ERRL_SEV_PREDICTIVE,            //Severity
            l_trace,                        //Trace Buf
            DEFAULT_TRACE_SIZE,             //Trace Size
            rc,                             //Userdata1
            0                               //Userdata2
            );

        // Commit error log
        commitErrl( &l_err );
    }
}

// Function Specification
//
// Name:  rtl_do_tick
//
// Description: RTL intr handler, a full-mode handler that invokes through the
//              macro bridge.  Runs all tasks in the current tick sequence.
//
// End Function Specification
void rtl_do_tick( void *private, SsxIrqId irq, int priority )
{
    uint64_t l_start = ssx_timebase_get();

    uint8_t *l_taskid_ptr = NULL;   // Pointer to the current task ID in the current tick sequence
    task_t *l_task_ptr = NULL;      // Pointer to the currently executing task
    errlHndl_t  l_err = NULL;       // Error handler
    tracDesc_t l_trace = NULL;      // Temporary trace descriptor (replace this when tracing is implemented)
    bool l_bad_id_reported = FALSE; // Has an invalid task ID already been reported?

    SsxMachineContext ctx;

    // Enter the protected context
    ssx_critical_section_enter( SSX_NONCRITICAL, &ctx );

    // Clear OCB timer0 status based on the interrupt
    ocb_timer_status_clear( irq );

    // Walk the tick sequence for the current tick, executing each runnable task listed.
    CURRENT_TICK++;

    // Save off start time of RTL tick
    G_fw_timing.rtl_start = l_start;

    // Set global run mask point to deferred mask so that any modification
    // to the deferred mask flag is taken into effect for this tick.
    G_run_mask = G_run_mask_deferred;

    RTLS_DBG("#### Tick %d ####\n",CURRENT_TICK);

    // Execute ARL Test Code before we run any tasks.  TODO: Remove when no longer needed.
    arl_test();

    // Index into the tick table to get a pointer to the tick sequence for the current tick.
    l_taskid_ptr = G_tick_table[ (MAX_NUM_TICKS - 1) & CURRENT_TICK ];

    do
    {
        if ( *l_taskid_ptr == TASK_END )
        {
            // We're done with this tick
            break;
        }

        if ( *l_taskid_ptr > TASK_END )
        {
            // Invalid task ID
            if ( ! l_bad_id_reported )
            {
                // First bad task ID we've seen this tick.  Log an unrecoverable error.

                // TODO use correct trace

                /*
                * @errortype
                * @moduleid    RTLS_DO_TICK_MOD
                * @reasoncode  INTERNAL_FAILURE
                * @userdata1   Invalid task ID
                * @userdata4   OCC_NO_EXTENDED_RC
                * @devdesc     Firmware failure on task operation
                */
                l_err = createErrl(
                            RTLS_DO_TICK_MOD,           //ModId
                            INTERNAL_FAILURE,           //Reasoncode
                            OCC_NO_EXTENDED_RC,         //Extended reasoncode
                            ERRL_SEV_UNRECOVERABLE,     //Severity
                            l_trace,                    //Trace Buf
                            DEFAULT_TRACE_SIZE,         //Trace Size
                            *l_taskid_ptr,              //Userdata1
                            0                           //Userdata2
                            );

                // Callout firmware
                addCalloutToErrl(l_err,
                                 ERRL_CALLOUT_TYPE_COMPONENT_ID,
                                 ERRL_COMPONENT_ID_FIRMWARE,
                                 ERRL_CALLOUT_PRIORITY_HIGH);

                // Commit error log
                commitErrl( &l_err );

                // Set our "bad ID reported" flag
                l_bad_id_reported = TRUE;
            }

            // Under all conditions, move on to the next task ID.
            l_taskid_ptr++;
            continue;
        }

        l_task_ptr = &(G_task_table[ *l_taskid_ptr ]);

        if ( ((l_task_ptr->flags & G_run_mask) == G_run_mask ) && (l_task_ptr->func_ptr != NULL) )
        {
            // The task's flags match the global run mask, and its function pointer is valid.
            // This task is ready to run.
            l_task_ptr->func_ptr(l_task_ptr);
        }

        // Increment to the next tick sequence before looping again.
        l_taskid_ptr++;

    } while(TRUE);

    // Save timing of RTL tick
    G_fw_timing.rtl_dur = DURATION_IN_US_UNTIL_NOW_FROM(l_start);

    RTLS_DBG("RTL Tick Duration: %d us\n",(int)G_fw_timing.rtl_dur);

    //exit the protected context
    ssx_critical_section_exit( &ctx );
}

// Function Specification
//
// Name: rtl_set_task_data
//
// Description: Changes the data pointer for the specified task
//
// End Function Specification

void rtl_set_task_data( const task_id_t i_task_id, void * i_data_ptr )
{
    errlHndl_t l_err = NULL;
    tracDesc_t l_trace = NULL;  // Temporary trace descriptor

    if ( i_task_id >= TASK_END )
    {
        //TODO: add trace
        // Task ID is invalid so log an error internally

        /*
        * @errortype
        * @moduleid    RTLS_SET_TASK_DATA_MOD
        * @reasoncode  INTERNAL_INVALID_INPUT_DATA
        * @userdata1   Failing task ID
        * @userdata4   OCC_NO_EXTENDED_RC
        * @devdesc     Firmware internal failure setting task data
        */
        l_err = createErrl(
                RTLS_SET_TASK_DATA_MOD,         //ModId
                INTERNAL_INVALID_INPUT_DATA,    //Reasoncode
                OCC_NO_EXTENDED_RC,             //Extended reasoncode
                ERRL_SEV_PREDICTIVE,            //Severity
                l_trace,                        //Trace Buf
                DEFAULT_TRACE_SIZE,             //Trace Size
                i_task_id,                      //Userdata1
                0                               //Userdata2
                );

        // Commit error log
        commitErrl( &l_err );
    }
    else
    {
        G_task_table[i_task_id].data_ptr = i_data_ptr;
    }
    return;
}

// Function Specification
//
// Name: rtl_set_run_mask
//
// Description: Stores the bitwise-or of i_flag and the global run mask into
//              the global run mask
//
// End Function Specification
void rtl_set_run_mask( const uint32_t i_flags )
{
    G_run_mask |= i_flags;
    G_run_mask_deferred |= i_flags;
    return;
}

// Function Specification
//
// Name: rtl_clr_run_mask
//
// Description: Stores the bitwise-and of the inverse of i_flag and the global
//              run mask into the global run mask
//
// End Function Specification
void rtl_clr_run_mask( const uint32_t i_flags )
{
    // Don't let the caller clear the run flag from the global mask.
    // We use an interim variable so we can keep i_flags const.
    uint32_t l_mask = i_flags & ~RTL_FLAG_RUN;

    G_run_mask &= ~l_mask;
    G_run_mask_deferred &= ~l_mask;
    return;
}

// Function Specification
//
// Name: rtl_set_run_mask_deferred
//
// Description: Stores the bitwise-or of i_flag and the global run mask
//              deferred into the global run mask deferred
//
// End Function Specification
void rtl_set_run_mask_deferred( const uint32_t i_flags )
{
    G_run_mask_deferred |= i_flags;
    return;
}

// Function Specification
//
// Name: rtl_clr_run_mask_deferred
//
// Description: Stores the bitwise-and of the inverse of i_flag and the global
//              run mask deferred into the global run mask deferred.
//
// End Function Specification
void rtl_clr_run_mask_deferred( const uint32_t i_flags )
{
    // Don't let the caller clear the run flag from the global mask.
    // We use an interim variable so we can keep i_flags const.
    uint32_t l_mask = i_flags & ~RTL_FLAG_RUN;

    G_run_mask_deferred &= ~l_mask;
    return;
}
