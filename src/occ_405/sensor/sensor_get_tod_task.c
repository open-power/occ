/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/sensor/sensor_get_tod_task.c $                    */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2017,2017                        */
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

/**
 * @file sensor_get_tod_task.c
 *
 * This file defines the functions and global variables for the task that gets
 * the current Time Of Day (TOD).
 *
 * The register holding the time of day value is read via a SCOM address, and
 * the OCC 405 cannot read from this address.  To work around this, the IPC
 * framework is used to request GPE0 to read the register value.  GPE0 returns
 * the current time of day value using a GPE_BUFFER.
 */

//******************************************************************************
// Includes
//******************************************************************************
#include <sensor_get_tod_task.h>    // Primary header
#include <stdint.h>                 // For uint*_t
#include <common_types.h>           // For bool
#include <sensor.h>                 // For G_tod
#include <occ_common.h>             // For GPE_BUFFER
#include <occ_service_codes.h>      // For OCC reason codes
#include <sensor_service_codes.h>   // For GET_TOD_*_MOD module ids
#include <trac.h>                   // For trace macros
#include <errl.h>                   // For error logging functions and types
#include <occhw_async.h>            // For gpe_request_*(), async_request_is_idle()
#include <get_tod_structs.h>        // For gpe_get_tod_args_t and TOD_VALUE_UNKNOWN
#include <occ_sys_config.h>         // For G_sysConfigData


//******************************************************************************
// Defines
//******************************************************************************

/**
 * Number of errors before we trace.
 */
#define GET_TOD_ERRORS_BEFORE_TRACE 1

/**
 * Number of errors before we set G_tod to TOD_VALUE_UNKNOWN.
 */
#define GET_TOD_ERRORS_BEFORE_UNKNOWN 4

/**
 * Number of errors before we log the error.
 */
#define GET_TOD_ERRORS_BEFORE_LOG 16

/**
 * Maximum number of errors to count.  Must fit in a uint8_t.
 */
#define GET_TOD_MAX_ERRORS GET_TOD_ERRORS_BEFORE_LOG


//******************************************************************************
// Structs and Enums
//******************************************************************************

/**
 * Type of data to store in the user details section of an error log.
 */
typedef enum 
{
    GET_TOD_ERRL_USR_DTLS_NONE,         ///< No data to store
    GET_TOD_ERRL_USR_DTLS_GPE_FFDC      ///< ffdc value from GPE0 in G_get_tod_args
} GET_TOD_ERRL_USR_DTLS_DATA;


//******************************************************************************
// Globals
//******************************************************************************

/**
 * Buffer holding arguments for the IPC function.  Used to pass data to/from GPE0.
 */
GPE_BUFFER(gpe_get_tod_args_t G_get_tod_args);

/**
 * GPE request structure.  Used by GPE functions to schedule request.
 */
GpeRequest G_get_tod_request;

/**
 * Specifies whether the GPE request was scheduled.  If false, the request
 * finished or has never been scheduled/initialized.
 */
bool G_get_tod_req_scheduled = false;

/**
 * Specifies whether the results of the GPE request are available.  Must be
 * declared volatile since it is used by both regular and callback functions.
 */
volatile bool G_get_tod_results_available = false;

/**
 * Number of consecutive errors that have occurred.  Cleared when we
 * successfully obtain the current time of day.
 */
uint8_t G_get_tod_error_count = 0;

/**
 * Specifies whether this task is enabled.  If the task is disabled it will no
 * longer attempt to get the current time of day.
 */
bool G_get_tod_enabled = true;


//******************************************************************************
// Private Functions
//******************************************************************************

/**
 * Increment the error count.
 */
void get_tod_increment_error_count()
{
    // Increment error count if we are below the maximum
    if (G_get_tod_error_count < GET_TOD_MAX_ERRORS)
    {
        ++G_get_tod_error_count;
    }

    // Set time of day to unknown if needed
    if (G_get_tod_error_count == GET_TOD_ERRORS_BEFORE_UNKNOWN)
    {
        G_tod = TOD_VALUE_UNKNOWN;
    }
}


/**
 * Clear the error count.
 *
 * If there had previously been errors, trace that we have recovered.
 */
void get_tod_clear_error_count()
{
    // If one or more errors had occurred
    if (G_get_tod_error_count > 0)
    {
        // Trace that we recovered
        TRAC_INFO("get_tod_clear_error_count: Task recovered after %u errors",
                  G_get_tod_error_count);

        // Clear error count
        G_get_tod_error_count = 0;
    }
}


/**
 * Logs and commits an unrecoverable error.  Calls out the processor.  Creates a
 * user details section if needed containing the specified additional data.
 * Does nothing if an error has already been logged.
 *
 * Disables this task.  We will no longer try to read the current time of day.
 *
 * Note that the required error log comment containing tags like 'userdata4' and
 * 'devdesc' must be located by the call to this function.  It is not located
 * inside this function because the value of those tags varies.
 *
 * @param i_modId          Module ID
 * @param i_reasonCode     Reason code
 * @param i_extReasonCode  Extended reason code
 * @param i_userData1      Userdata1 value
 * @param i_userData2      Userdata2 value
 * @param i_usrDtlsData    Data to store in a user details section (if any)
 */
void get_tod_log_error(uint16_t i_modId, uint8_t i_reasonCode,
                       uint16_t i_extReasonCode, uint32_t i_userData1,
                       uint32_t i_userData2, GET_TOD_ERRL_USR_DTLS_DATA i_usrDtlsData)
{
    // Exit if we have already logged an error
    static bool L_error_logged = false;
    if (L_error_logged)
    {
        return;
    }

    // Create unrecoverable error
    errlHndl_t l_errl = createErrl(i_modId,                // Module ID
                                   i_reasonCode,           // Reason code
                                   i_extReasonCode,        // Extended reason code
                                   ERRL_SEV_UNRECOVERABLE, // Severity
                                   NULL,                   // Trace Buffers
                                   DEFAULT_TRACE_SIZE,     // Trace Size
                                   i_userData1,            // Userdata1
                                   i_userData2);           // Userdata2

    // If specified, add user details section to hold ffdc field from GPE
    if (i_usrDtlsData == GET_TOD_ERRL_USR_DTLS_GPE_FFDC)
    {
        addUsrDtlsToErrl(l_errl,
                         (uint8_t *) &(G_get_tod_args.error.ffdc),
                         sizeof(G_get_tod_args.error.ffdc),
                         ERRL_USR_DTL_STRUCT_VERSION_1,
                         ERRL_USR_DTL_BINARY_DATA);
    }

    // Add processor callout
    addCalloutToErrl(l_errl,
                     ERRL_CALLOUT_TYPE_HUID,
                     G_sysConfigData.proc_huid,
                     ERRL_CALLOUT_PRIORITY_MED);

    // Commit error
    commitErrl(&l_errl);
    L_error_logged = true;

    // Disable this task
    TRAC_ERR("get_tod_log_error: Disabled task due to logging an error");
    G_get_tod_enabled = false;
}


/**
 * Returns whether the global GPE request struct is idle and ready for re-use.
 * Returns true immediately if the request was not scheduled.  If the request
 * was scheduled, checks to see if it has finished.
 *
 * @return true if GPE request is idle, false otherwise
 */
bool get_tod_is_request_idle(void)
{
    // If the request was not previously scheduled, then it is idle.  This also
    // handles the case where the request has not been initialized yet.
    if (!G_get_tod_req_scheduled)
    {
        return true;
    }

    // Request was scheduled; check if it finished and is now idle
    if (async_request_is_idle(&G_get_tod_request.request))
    {
        // Request is now idle and ready for re-use
        G_get_tod_req_scheduled = false;
        return true;
    }

    // Request was scheduled but has not finished.  Increment error count.
    get_tod_increment_error_count();

    // Trace if necessary
    if (G_get_tod_error_count == GET_TOD_ERRORS_BEFORE_TRACE)
    {
        TRAC_ERR("get_tod_is_request_idle: Waiting for request to finish");
    }

    // Log error if necessary
    if (G_get_tod_error_count == GET_TOD_ERRORS_BEFORE_LOG)
    {
        /* @
         * @errortype
         * @moduleid    GET_TOD_IS_REQ_IDLE_MOD
         * @reasoncode  GPE_REQUEST_TASK_NOT_IDLE
         * @userdata1   0
         * @userdata2   0
         * @userdata4   ERC_GENERIC_TIMEOUT
         * @devdesc     GPE request not finished after waiting repeatedly
         */
        get_tod_log_error(GET_TOD_IS_REQ_IDLE_MOD, GPE_REQUEST_TASK_NOT_IDLE,
                          ERC_GENERIC_TIMEOUT, 0, 0, GET_TOD_ERRL_USR_DTLS_NONE);
    }

    // Return false since request is not idle
    return false;
}


/**
 * Callback that is invoked when the GPE request completes.
 *
 * @param i_arg Callback argument specified during gpe_request_create().  Not used.
 */
int get_tod_callback(void * i_arg)
{
    // NOTE: No tracing allowed in callback functions

    // If GPE request was successful copy current time of day into G_tod.  We do
    // this in a callback so G_tod is updated as soon as possible.
    if (G_get_tod_args.error.rc == GPE_RC_SUCCESS)
    {
        G_tod = G_get_tod_args.tod;
    }

    // Set flag indicating results of GPE request are available.  Any errors
    // will be handled later since callbacks cannot trace.
    G_get_tod_results_available = true;

    // Return 0 indicating to async framework that callback ran successfully
    return 0;
}


/**
 * Handles the results of the previous GPE request.
 */
void get_tod_handle_request_results(void)
{
    // Check if results from previous GPE request are available
    if (!G_get_tod_results_available)
    {
        // No results available, so there is nothing to do.  There was no
        // previous request, or previous request didn't complete due to errors.
        return;
    }

    // Check whether GPE request successfully read time of day
    if (G_get_tod_args.error.rc == GPE_RC_SUCCESS)
    {
        // Request succeeded.  Callback already set G_tod.  Clear error counter.
        get_tod_clear_error_count();
    }
    else
    {
        // Request failed to read time of day; increment error count
        get_tod_increment_error_count();

        // Trace if needed
        if (G_get_tod_error_count == GET_TOD_ERRORS_BEFORE_TRACE)
        {
            TRAC_ERR("get_tod_handle_request_results: GPE0 error reading TOD register: "
                     "addr=0x%08X, rc=0x%08X, ffdc=0x%08X%08X",
                     G_get_tod_args.error.addr,
                     G_get_tod_args.error.rc, 
                     (uint32_t) (G_get_tod_args.error.ffdc >> 32),
                     (uint32_t) (G_get_tod_args.error.ffdc & 0x00000000FFFFFFFFull));
        }

        // Log error if needed
        if (G_get_tod_error_count == GET_TOD_ERRORS_BEFORE_LOG)
        {
            /* @
             * @errortype
             * @moduleid    GET_TOD_HNDL_REQ_RSLT_MOD
             * @reasoncode  GPE_REQUEST_RC_FAILURE
             * @userdata1   SCOM address
             * @userdata2   RC from GPE IPC function
             * @userdata4   ERC_GETSCOM_FAILURE
             * @devdesc     GPE request to read SCOM TOD register failed
             */
            get_tod_log_error(GET_TOD_HNDL_REQ_RSLT_MOD, GPE_REQUEST_RC_FAILURE,
                              ERC_GETSCOM_FAILURE, G_get_tod_args.error.addr,
                              G_get_tod_args.error.rc, GET_TOD_ERRL_USR_DTLS_GPE_FFDC);
        }
    }

    // Clear flag since we handled the results of the previous GPE request
    G_get_tod_results_available = false;
}


/**
 * Schedules a GPE request to read the TOD register to get current time of day.
 */
void get_tod_schedule_request(void)
{
    // Create (initialize) GPE request if needed
    static bool L_request_created = false;
    if (!L_request_created)
    {
        int l_rc = gpe_request_create(&G_get_tod_request,         // GpeRequest
                                      &G_async_gpe_queue0,        // Queue for GPE0
                                      IPC_ST_GET_TOD_FUNCID,      // IPC Function ID
                                      &G_get_tod_args,            // IPC Command Data
                                      SSX_WAIT_FOREVER,           // Timeout (none)
                                      get_tod_callback,           // Callback
                                      NULL,                       // Callback argument
                                      ASYNC_CALLBACK_IMMEDIATE);  // Options
        if (l_rc != 0)
        {
            // Create failed; trace and log error, then exit.  Ignore error
            // count because we don't retry creates.
            TRAC_ERR("get_tod_schedule_request: Request create failure: rc=0x%08X", l_rc);
            /* @
             * @errortype
             * @moduleid    GET_TOD_SCHED_REQ_MOD
             * @reasoncode  GPE_REQUEST_CREATE_FAILURE
             * @userdata1   Return code from gpe_request_create()
             * @userdata2   0
             * @userdata4   OCC_NO_EXTENDED_RC
             * @devdesc     Failed to create GPE request
             */
            get_tod_log_error(GET_TOD_SCHED_REQ_MOD, GPE_REQUEST_CREATE_FAILURE,
                              OCC_NO_EXTENDED_RC, l_rc, 0, GET_TOD_ERRL_USR_DTLS_NONE);
            return;
        }
        L_request_created = true;
    }

    // Schedule GPE request
    int l_rc = gpe_request_schedule(&G_get_tod_request);
    if (l_rc != 0)
    {
        // Schedule failed; increment error count
        get_tod_increment_error_count();

        // Trace if needed
        if (G_get_tod_error_count == GET_TOD_ERRORS_BEFORE_TRACE)
        {
            TRAC_ERR("get_tod_schedule_request: Request schedule failure: rc=0x%08X", l_rc);
        }

        // Log error if needed
        if (G_get_tod_error_count == GET_TOD_ERRORS_BEFORE_LOG)
        {
            /* @
             * @errortype
             * @moduleid    GET_TOD_SCHED_REQ_MOD
             * @reasoncode  GPE_REQUEST_SCHEDULE_FAILURE
             * @userdata1   Return code from gpe_request_schedule()
             * @userdata2   0
             * @userdata4   OCC_NO_EXTENDED_RC
             * @devdesc     Failed to schedule GPE request
             */
            get_tod_log_error(GET_TOD_SCHED_REQ_MOD, GPE_REQUEST_SCHEDULE_FAILURE,
                              OCC_NO_EXTENDED_RC, l_rc, 0, GET_TOD_ERRL_USR_DTLS_NONE);
        }
        return;
    }

    // Successfully scheduled request.  Request is not blocking, so we will not
    // get the results until later.  Set flag indicating request is scheduled.
    G_get_tod_req_scheduled = true;
}


//******************************************************************************
// Public Functions
//******************************************************************************

// See description in header file
void task_get_tod(task_t * i_self)
{
    // Exit if this task is disabled
    if (!G_get_tod_enabled)
    {
        return;
    }

    // Exit if the previous GPE request has not finished
    if (!get_tod_is_request_idle())
    {
        return;
    }

    // Handle results of previous GPE request
    get_tod_handle_request_results();
    if (!G_get_tod_enabled)
    {
        // Task disabled due to errors while handling request results
        return;
    }

    // Schedule new GPE request to get time of day
    get_tod_schedule_request();
}
