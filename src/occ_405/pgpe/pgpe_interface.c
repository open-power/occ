/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/pgpe/pgpe_interface.c $                           */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2017                        */
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

#include "occ_common.h"
#include "occhw_async.h"
#include "pgpe_interface.h"
#include "pstate_pgpe_occ_api.h"
#include "occ_service_codes.h"
#include "pgpe_service_codes.h"
#include "trac.h"
#include "state.h"
#include "proc_pstate.h"
#include "proc_data_control.h"
#include "occ_sys_config.h"
#include "ssx.h"
#include "wof.h"
#include "amec_sys.h"

// Maximum waiting time (usec) for clip update IPC task
#define CLIP_UPDATE_TIMEOUT 100  // maximum waiting time (usec) for clip update IPC task

extern pstateStatus G_proc_pstate_status;
extern PMCR_OWNER G_proc_pmcr_owner;

extern uint16_t G_proc_fmax_mhz;

extern bool G_simics_environment;

// IPC GPE Requests
GpeRequest G_clip_update_req;
GpeRequest G_pmcr_set_req;
GpeRequest G_start_suspend_req;
GpeRequest G_wof_control_req;
GpeRequest G_wof_vfrt_req;

// GPE parameter fields for PGPE IPC calls
GPE_BUFFER(ipcmsg_clip_update_t   G_clip_update_parms);
GPE_BUFFER(ipcmsg_set_pmcr_t      G_pmcr_set_parms);
GPE_BUFFER(ipcmsg_start_stop_t    G_start_suspend_parms);
GPE_BUFFER(ipcmsg_wof_control_t   G_wof_control_parms);
GPE_BUFFER(ipcmsg_wof_vfrt_t      G_wof_vfrt_parms);


// Function Specification
//
// Name: init_pgpe_ipcs
//
// Description: Create all PGPE IPC messages.
//
// End Function Specification
void init_pgpe_ipcs(void)
{
    errlHndl_t  err = NULL;    // Error handler

    do
    {
        err = pgpe_init_clips();
        if(err)
        {
            break;
        }
        err = pgpe_init_pmcr();
        if(err)
        {
            break;
        }
        err = pgpe_init_start_suspend();
        if(err)
        {
            break;
        }
        err = pgpe_init_wof_control();
        if(err)
        {
            break;
        }
        err = pgpe_init_wof_vfrt();
    }while(0);

    if(err)
    {
        REQUEST_RESET(err);
    }

    return;
}


// Function Specification
//
// Name: pgpe_init_clips
//
// Description: Create PGPE clip update IPC messages (non-blocking).
//
// End Function Specification
errlHndl_t pgpe_init_clips(void)
{
    int rc;                    // return code
    errlHndl_t  err = NULL;    // Error handler

    do
    {
        G_clip_update_parms.msg_cb.rc = PGPE_RC_SUCCESS;

        //Initializes the GpeRequest object for pgpe clips setting IPC
        rc = gpe_request_create(&G_clip_update_req,         // GpeRequest for the task
                                &G_async_gpe_queue2,        // Queue
                                IPC_MSGID_405_CLIPS,        // Function ID
                                &G_clip_update_parms,       // Task parameters
                                SSX_WAIT_FOREVER,           // Timeout (none)
                                NULL,                       // Callback
                                NULL,                       // Callback arguments
                                ASYNC_CALLBACK_IMMEDIATE ); // Options

        if( rc )
        {
            // If we failed to create the GpeRequest then there is a serious problem.
            MAIN_TRAC_ERR("pgpe_init_clips: Failure creating the "
                          "IPC_MSGID_405_CLIPS GpeRequest. [RC=0x%08x]", rc );

            /*
             * @errortype
             * @moduleid    PGPE_INIT_CLIPS_MOD
             * @reasoncode  GPE_REQUEST_CREATE_FAILURE
             * @userdata1   gpe_request_create return code
             * @userdata4   OCC_NO_EXTENDED_RC
             * @devdesc     Failure to create clips GpeRequest object
             */
            err = createErrl(
                PGPE_INIT_CLIPS_MOD,             //ModId
                GPE_REQUEST_CREATE_FAILURE,      //Reasoncode
                OCC_NO_EXTENDED_RC,              //Extended reason code
                ERRL_SEV_PREDICTIVE,             //Severity
                NULL,                            //Trace Buf
                DEFAULT_TRACE_SIZE,              //Trace Size
                rc,                              //Userdata1
                0                                //Userdata2
                );
        }

    } while (0);

    return(err);
}

// Function Specification
//
// Name: pgpe_init_pmcr
//
// Description: Create PGPE PMCR set IPC messages (non-blocking).
//
// End Function Specification
errlHndl_t pgpe_init_pmcr(void)
{
    int rc;                    // return code
    errlHndl_t  err = NULL;    // Error handler

    do
    {
        G_pmcr_set_parms.msg_cb.rc = PGPE_RC_SUCCESS;

        //Initializes the GpeRequest object for pgpe PMCR setting IPC
        rc = gpe_request_create(&G_pmcr_set_req,            // GpeRequest for the task
                                &G_async_gpe_queue2,        // Queue
                                IPC_MSGID_405_SET_PMCR,     // Function ID
                                &G_pmcr_set_parms,          // Task parameters
                                SSX_WAIT_FOREVER,           // Timeout (none)
                                NULL,                       // Callback
                                NULL,                       // Callback arguments
                                ASYNC_CALLBACK_IMMEDIATE);  // Options

        if( rc )
        {
            // If we failed to create the GpeRequest then there is a serious problem.
            MAIN_TRAC_ERR("pgpe_init_pmcr: Failure creating the "
                          "IPC_MSGID_405_SET_PMCR GpeRequest. [RC=0x%08x]",
                          rc );

            /*
             * @errortype
             * @moduleid    PGPE_INIT_PMCR_MOD
             * @reasoncode  GPE_REQUEST_CREATE_FAILURE
             * @userdata1   gpe_request_create return code
             * @userdata4   OCC_NO_EXTENDED_RC
             * @devdesc     Failure to create pmcr GpeRequest object
             */
            err = createErrl(
                PGPE_INIT_PMCR_MOD,              //ModId
                GPE_REQUEST_CREATE_FAILURE,      //Reasoncode
                OCC_NO_EXTENDED_RC,              //Extended reason code
                ERRL_SEV_PREDICTIVE,             //Severity
                NULL,                            //Trace Buf
                DEFAULT_TRACE_SIZE,              //Trace Size
                rc,                              //Userdata1
                0                                //Userdata2
                );
        }

    } while (0);

    return(err);
}

// Function Specification
//
// Name: pgpe_init_start_suspend
//
// Description: Create PGPE start/suspend IPC messages (blocking).
//
// End Function Specification
errlHndl_t pgpe_init_start_suspend(void)
{
    int rc;                    // return code
    errlHndl_t  err = NULL;    // Error handler

    do
    {
        //Initializes the GpeRequest object for pgpe start/suspend IPC
        rc = gpe_request_create(&G_start_suspend_req,                              // GpeRequest for the task
                                &G_async_gpe_queue2,                               // Queue
                                IPC_MSGID_405_START_SUSPEND,                       // Function ID
                                &G_start_suspend_parms,                            // Task parameters
                                SSX_WAIT_FOREVER,                                  // Timeout (none)
                                (AsyncRequestCallback)pgpe_start_suspend_callback, // Callback
                                NULL,                                              // Callback arguments
                                ASYNC_CALLBACK_IMMEDIATE );                        // Options

        if( rc )
        {
            // If we failed to create the GpeRequest then there is a serious problem.
            MAIN_TRAC_ERR("pgpe_init_start_suspend: Failure creating the "
                          "IPC_MSGID_405_START_SUSPEND GpeRequest. [RC=0x%08x]",
                          rc );

            /*
             * @errortype
             * @moduleid    PGPE_INIT_START_SUSPEND_MOD
             * @reasoncode  GPE_REQUEST_CREATE_FAILURE
             * @userdata1   gpe_request_create return code
             * @userdata4   OCC_NO_EXTENDED_RC
             * @devdesc     Failure to create start_suspend GpeRequest object
             */
            err = createErrl(
                PGPE_INIT_START_SUSPEND_MOD,     //ModId
                GPE_REQUEST_CREATE_FAILURE,      //Reasoncode
                OCC_NO_EXTENDED_RC,              //Extended reason code
                ERRL_SEV_PREDICTIVE,             //Severity
                NULL,                            //Trace Buf
                DEFAULT_TRACE_SIZE,              //Trace Size
                rc,                              //Userdata1
                0                                //Userdata2
                );
        }

    } while (0);

    return(err);
}


// Function Specification
//
// Name: pgpe_init_wof_control
//
// Description: Create PGPE WOF control IPC messages (non-blocking).
//
// End Function Specification
errlHndl_t pgpe_init_wof_control(void)
{
    int rc;                    // return code
    errlHndl_t  err = NULL;    // Error handler

    do
    {
        //Initializes the GpeRequest object for pgpe wof control IPC
        rc = gpe_request_create(&G_wof_control_req,         // GpeRequest for the task
                                &G_async_gpe_queue2,        // Queue
                                IPC_MSGID_405_WOF_CONTROL,  // Function ID
                                &G_wof_control_parms,       // Task parameters
                                SSX_WAIT_FOREVER,           // Timeout (none)
                                (AsyncRequestCallback)wof_control_callback, // Callback
                                NULL,                       // Callback arguments
                                ASYNC_CALLBACK_IMMEDIATE); // Options

        if( rc )
        {
            // If we failed to create the GpeRequest then there is a serious problem.
            MAIN_TRAC_ERR("pgpe_init_wof_control: Failure creating the "
                          "IPC_MSGID_405_WOF_CONTROL GpeRequest. [RC=0x%08x]",
                          rc );

            /*
             * @errortype
             * @moduleid    PGPE_INIT_WOF_CONTROL_MOD
             * @reasoncode  GPE_REQUEST_CREATE_FAILURE
             * @userdata1   gpe_request_create return code
             * @userdata4   OCC_NO_EXTENDED_RC
             * @devdesc     Failure to create wof control GpeRequest object
             */
            err = createErrl(
                PGPE_INIT_WOF_CONTROL_MOD,       //ModId
                GPE_REQUEST_CREATE_FAILURE,      //Reasoncode
                OCC_NO_EXTENDED_RC,              //Extended reason code
                ERRL_SEV_PREDICTIVE,             //Severity
                NULL,                            //Trace Buf
                DEFAULT_TRACE_SIZE,              //Trace Size
                rc,                              //Userdata1
                0                                //Userdata2
                );
        }

    } while (0);

    return(err);
}

// Function Specification
//
// Name: pgpe_init_wof_vfrt
//
// Description: Create PGPE WOF vfrt IPC messages (non-blocking).
//
// End Function Specification
errlHndl_t pgpe_init_wof_vfrt(void)
{
    int rc;                    // return code
    errlHndl_t  err = NULL;    // Error handler

    do
    {
        //Initializes the GpeRequest object for pgpe wof vfrt IPC
        rc = gpe_request_create(&G_wof_vfrt_req,            // GpeRequest for the task
                                &G_async_gpe_queue2,        // Queue
                                IPC_MSGID_405_WOF_VFRT,     // Function ID
                                &G_wof_vfrt_parms,          // Task parameters
                                SSX_WAIT_FOREVER,           // Timeout (none)
                                (AsyncRequestCallback)wof_vfrt_callback, // Callback
                                NULL,                       // Callback arguments
                                ASYNC_CALLBACK_IMMEDIATE);  // Options

        if( rc )
        {
            // If we failed to create the GpeRequest then there is a serious problem.
            MAIN_TRAC_ERR("pgpe_init_wof_vfrt: Failure creating the "
                          "IPC_MSGID_405_WOF_VFRT GpeRequest. [RC=0x%08x]",
                          rc );

            /*
             * @errortype
             * @moduleid    PGPE_INIT_WOF_VFRT_MOD
             * @reasoncode  GPE_REQUEST_CREATE_FAILURE
             * @userdata1   gpe_request_create return code
             * @userdata4   OCC_NO_EXTENDED_RC
             * @devdesc     Failure to create wof control GpeRequest object
             */
            err = createErrl(
                PGPE_INIT_WOF_VFRT_MOD,          //ModId
                GPE_REQUEST_CREATE_FAILURE,      //Reasoncode
                OCC_NO_EXTENDED_RC,              //Extended reason code
                ERRL_SEV_PREDICTIVE,             //Severity
                NULL,                            //Trace Buf
                DEFAULT_TRACE_SIZE,              //Trace Size
                rc,                              //Userdata1
                0                                //Userdata2
                );
        }

    } while (0);

    return(err);
}


// Function Specification
//
// Name: pgpe_set_clip_blocking
//
// Description: a blocking version of pgpe_clip_update
//              this call waits until the clip IPC task completes,
//              and verifies a successful completion.
//
// End Function Specification
int pgpe_set_clip_blocking(Pstate i_pstate)
{
    errlHndl_t   err = NULL;
    uint8_t      wait_time = 0;
    int          rc = 0;  // Return code: 0 means success, otherwise an error.

    do
    {
        // verify that the clip update IPC task is not currently running
        while( !async_request_is_idle(&G_clip_update_req.request) )
        {
            if(wait_time > CLIP_UPDATE_TIMEOUT)
            {
                // an earlier clip update IPC call has not completed, trace and log an error
                TRAC_ERR("pgpe_set_clip_blocking: clip update IPC task is not Idle");

                /*
                 * @errortype
                 * @moduleid    PGPE_SET_CLIP_BLOCKING_MOD
                 * @reasoncode  PGPE_FAILURE
                 * @userdata4   ERC_PGPE_NOT_IDLE
                 * @devdesc     pgpe clip update not idle
                 */
                err = createErrl(
                    PGPE_SET_CLIP_BLOCKING_MOD,    //ModId
                    PGPE_FAILURE,                    //Reasoncode
                    ERC_PGPE_NOT_IDLE,               //Extended reason code
                    ERRL_SEV_PREDICTIVE,             //Severity
                    NULL,                            //Trace Buf
                    DEFAULT_TRACE_SIZE,              //Trace Size
                    0,                               //Userdata1
                    0                                //Userdata2
                );

                rc = PGPE_FAILURE;
                break;
            }

            // sleep for 1 microsecond (allows context switching)
            ssx_sleep(SSX_MICROSECONDS(1));

            wait_time++;
        }

        // Previous task is finished
        if( !err )
        {
            // set the all quads to same pstate and submit the update
            memset(G_desired_pstate, i_pstate, MAXIMUM_QUADS);
            rc = pgpe_clip_update();
            if(rc)
            {
                break;
            }
        }
        else
        {
            // Previous task has still not finished
            break;
        }

        // Reset wait counter
        wait_time = 0;

        // Wait until the clip_update IPC task completes
        while( !async_request_is_idle(&G_clip_update_req.request) )
        {
            if(wait_time > CLIP_UPDATE_TIMEOUT)
            {
                TRAC_ERR("pgpe_set_clip_blocking: clip update IPC task timeout!");

                /*
                 * @errortype
                 * @moduleid    PGPE_SET_CLIP_BLOCKING_MOD
                 * @reasoncode  GPE_REQUEST_TASK_TIMEOUT
                 * @userdata4   OCC_NO_EXTENDED_RC
                 * @devdesc     pgpe clip update timeout
                 */
                err = createErrl(
                    PGPE_SET_CLIP_BLOCKING_MOD,    //ModId
                    GPE_REQUEST_TASK_TIMEOUT,        //Reasoncode
                    OCC_NO_EXTENDED_RC,              //Extended reason code
                    ERRL_SEV_PREDICTIVE,             //Severity
                    NULL,                            //Trace Buf
                    DEFAULT_TRACE_SIZE,              //Trace Size
                    0,                               //Userdata1
                    0                                //Userdata2
                    );

                rc = GPE_REQUEST_TASK_TIMEOUT;
                break;
            }

            // sleep for 1 microsecond (allows context switching)
            ssx_sleep(SSX_MICROSECONDS(1));

            wait_time++;
        }

        // clip update timed out
        if(err)
        {
            break;
        }

        // IPC task completed. check for errors
        if ( G_clip_update_parms.msg_cb.rc != PGPE_RC_SUCCESS )
        {
            // clip update IPC call has not completed, trace and log an error
            TRAC_ERR("pgpe_set_clip_blocking: clip update IPC task "
                     "returned an error [0x%08X]",
                     G_clip_update_parms.msg_cb.rc);

            /*
             * @errortype
             * @moduleid    PGPE_SET_CLIP_BLOCKING_MOD
             * @reasoncode  GPE_REQUEST_RC_FAILURE
             * @userdata1   PGPE clip update's rc
             * @userdata4   OCC_NO_EXTENDED_RC
             * @devdesc     pgpe clip update returned a failure code
             */
            err = createErrl(
                PGPE_SET_CLIP_BLOCKING_MOD,         //ModId
                GPE_REQUEST_RC_FAILURE,               //Reasoncode
                OCC_NO_EXTENDED_RC,                   //Extended reason code
                ERRL_SEV_PREDICTIVE,                  //Severity
                NULL,                                 //Trace Buf
                DEFAULT_TRACE_SIZE,                   //Trace Size
                G_clip_update_parms.msg_cb.rc,   //Userdata1
                0                                     //Userdata2
                );

            rc = GPE_REQUEST_RC_FAILURE;
        }
    } while (0);

    if(err)
    {
        REQUEST_RESET(err);
    }

    return(rc);
}


// Function Specification
//
// Name: pgpe_clip_update
//
// Description: call PGPE IPC to update clip ranges. Since this IPC call is
//              non-blocking, the calling function needs to assure previous
//              call completion before calling this call.
//
// End Function Specification
int pgpe_clip_update(void)
{
    int rc = 0;              // return code
    errlHndl_t  err = NULL;  // Error handler
    uint32_t    l_wait_time = 0;
    do
    {
        // Caller must check the completion of previous invocation of clip updates.
        // This check is a safety feature in case caller didn't check IPC is idle.
        while(!async_request_is_idle(&G_clip_update_req.request))
        {
            if(l_wait_time > CLIP_UPDATE_TIMEOUT)
            {
                // an earlier clip update IPC call has not completed, trace and log an error
                TRAC_ERR("pgpe_clip_update: clip update IPC task is not Idle");

                /*
                 * @errortype
                 * @moduleid    PGPE_CLIP_UPDATE_MOD
                 * @reasoncode  PGPE_FAILURE
                 * @userdata1   0
                 * @userdata4   ERC_PGPE_NOT_IDLE
                 * @devdesc     pgpe clip update not idle
                 */
                err = createErrl(
                    PGPE_CLIP_UPDATE_MOD,            //ModId
                    PGPE_FAILURE,                    //Reasoncode
                    ERC_PGPE_NOT_IDLE,               //Extended reason code
                    ERRL_SEV_PREDICTIVE,             //Severity
                    NULL,                            //Trace Buf
                    DEFAULT_TRACE_SIZE,              //Trace Size
                    0,                               //Userdata1
                    0                                //Userdata2
                    );

                // Callout firmware
                addCalloutToErrl(err,
                                 ERRL_CALLOUT_TYPE_COMPONENT_ID,
                                 ERRL_COMPONENT_ID_FIRMWARE,
                                 ERRL_CALLOUT_PRIORITY_HIGH);

                commitErrl(&err);

                rc = PGPE_FAILURE;
                break;
            }

            // Sleep for 1us (allows context switching)
            ssx_sleep(SSX_MICROSECONDS(1));

            l_wait_time++;
        }

        if (!G_simics_environment)
        {
            // Set clip bounds
            memset(G_clip_update_parms.ps_val_clip_min, proc_freq2pstate(g_amec->sys.fmin), MAXIMUM_QUADS);
            memcpy(G_clip_update_parms.ps_val_clip_max, G_desired_pstate, sizeof(G_desired_pstate));

            // Schedule PGPE clip update IPC task
            rc = gpe_request_schedule(&G_clip_update_req);
        }
        else
        {
            G_start_suspend_parms.msg_cb.rc = PGPE_RC_SUCCESS;
            rc = 0;
        }
        // Confirm Successfull completion of PGPE clip update task
        if(rc != 0)
        {
            //Error in scheduling pgpe clip update task
            TRAC_ERR("pgpe_clip_update: Failed to schedule clip update pgpe task rc=%x",
                     rc);

            /* @
             * @errortype
             * @moduleid    PGPE_CLIP_UPDATE_MOD
             * @reasoncode  GPE_REQUEST_SCHEDULE_FAILURE
             * @userdata1   rc - gpe_request_schedule return code
             * @userdata2   0
             * @userdata4   OCC_NO_EXTENDED_RC
             * @devdesc     OCC Failed to schedule a GPE job for clip update
             */
            err = createErrl(
                PGPE_CLIP_UPDATE_MOD,                   // modId
                GPE_REQUEST_SCHEDULE_FAILURE,           // reasoncode
                OCC_NO_EXTENDED_RC,                     // Extended reason code
                ERRL_SEV_UNRECOVERABLE,                 // Severity
                NULL,                                   // Trace Buf
                DEFAULT_TRACE_SIZE,                     // Trace Size
                rc,                                     // userdata1
                0                                       // userdata2
                );

            rc = GPE_REQUEST_SCHEDULE_FAILURE;
            REQUEST_RESET(err);   //This will add a firmware callout for us
        }
    }while(0);

    return rc;
}


// Function Specification
//
// Name: start_suspend_callback
//
// Description: callback function for the pgpe_start_suspend IPC function
//              gets called after the PGPE IPC enables/disables pstates.
//              Upon successful completion, it sets/clears the G_proc_pstate_status
//              flag to indicate whether it is enabled/disabled/in-transition.
//
// End Function Specification
void pgpe_start_suspend_callback(void)
{
    int pgpe_rc;               // pgpe return codes
    errlHndl_t  err = NULL;    // Error handler

    pgpe_rc = G_start_suspend_parms.msg_cb.rc;

    do
    {
        // Confirm Successfull completion of PGPE start suspend task
        if(pgpe_rc != PGPE_RC_SUCCESS)
        {
            //Error in scheduling pgpe start suspend task

            /* @
             * @errortype
             * @moduleid    PGPE_START_SUSPEND_CALLBACK_MOD
             * @reasoncode  GPE_REQUEST_SCHEDULE_FAILURE
             * @userdata1   pgpe_rc - PGPE return code
             * @userdata4   OCC_NO_EXTENDED_RC
             * @devdesc     OCC Failed to schedule a GPE job for start_suspend
             */
            err = createErrl(
                PGPE_START_SUSPEND_CALLBACK_MOD,        // modId
                PGPE_FAILURE,                           // reasoncode
                ERC_PGPE_UNSUCCESSFULL,                 // Extended reason code
                ERRL_SEV_UNRECOVERABLE,                 // Severity
                NULL,                                   // Trace Buf
                DEFAULT_TRACE_SIZE,                     // Trace Size
                pgpe_rc,                                // userdata1
                0                                       // userdata2
                );

            REQUEST_RESET(err);   //This will add a firmware callout for us
            break;
        }
        // task completed successfully
        else
        {
            // this was a command to enable pstates
            if(G_start_suspend_parms.action == PGPE_ACTION_PSTATE_START)
            {
                // Pstates are now enabled (enable Active State transition).
                G_proc_pstate_status = PSTATES_ENABLED;

                // upon successful pstates START, assign G_proc_pmcr_owner (OCC/CHAR/HOST)
                G_proc_pmcr_owner = G_start_suspend_parms.pmcr_owner;

                // Clear WOF Pstate status flag
                g_amec->wof.wof_disabled &= ~WOF_RC_PSTATE_PROTOCOL_OFF;
            }
            // this was a command to disable pstates
            else if(G_start_suspend_parms.action == PGPE_ACTION_PSTATE_STOP)
            {
                // Pstates are now disabled (disable Active State transition).
                G_proc_pstate_status = PSTATES_DISABLED;
            }
        }

    } while(0);

}

// Function Specification
//
// Name: pgpe_start_suspend
//
// Description: Schedule a PGPE IPC to enable/disable pstates. This will also tell
//              PGPE how to set PMCR mode register (OCC/OPAL control pstates).
//              when this IPC is successfully scheduled, it sets the G_proc_pstate_status
//              global variable to in-transition, a call back function
//              (pgpe_start_suspend_callback) will set it to the appropriate value upon
//              successfull completion.
//
// End Function Specification
int pgpe_start_suspend(uint8_t action, PMCR_OWNER owner)
{
    int rc = 0;                // return code
    errlHndl_t  err = NULL;    // Error handler

    // set the IPC parameters
    G_start_suspend_parms.action     = action;
    G_start_suspend_parms.pmcr_owner = owner;

    if (!G_simics_environment)
    {
        // Schedule PGPE start_suspend task
        rc = gpe_request_schedule(&G_start_suspend_req);
    }

    // couldn't schedule the IPC task?
    if(rc != 0)
    {
        //Error in scheduling pgpe start suspend task
        TRAC_ERR("pgpe_start_suspend: Failed to schedule pgpe start_suspend task rc=%x",
                 rc);
        /* @
         * @errortype
         * @moduleid    PGPE_START_SUSPEND_MOD
         * @reasoncode  GPE_REQUEST_SCHEDULE_FAILURE
         * @userdata1   rc - gpe_request_schedule return code
         * @userdata4   OCC_NO_EXTENDED_RC
         * @devdesc     OCC Failed to schedule a PGPE job for start_suspend
         */
        err = createErrl(
            PGPE_START_SUSPEND_MOD,                 // modId
            GPE_REQUEST_SCHEDULE_FAILURE,           // reasoncode
            OCC_NO_EXTENDED_RC,                     // Extended reason code
            ERRL_SEV_UNRECOVERABLE,                 // Severity
            NULL,                                   // Trace Buf
            DEFAULT_TRACE_SIZE,                     // Trace Size
            rc,                                     // userdata1
            0                                       // userdata2
            );

        rc = GPE_REQUEST_SCHEDULE_FAILURE;
        REQUEST_RESET(err);   //This will add a firmware callout for us
    }
    // successfully scheduled, set the G_proc_pstate_status to indicate transition
    else
    {
        G_proc_pstate_status = PSTATES_IN_TRANSITION;
    }

    if (G_simics_environment)
    {
        pgpe_start_suspend_callback();
    }

    return rc;
}


// Function Specification
//
// Name: pgpe_pmcr_set
//
// Description: call PGPE IPC to set the PMCR with the pstate values.
//              Since this IPC call is non-blocking, the calling
//              function needs to assure previous call completion
//              before calling this call.
//
// End Function Specification
int pgpe_pmcr_set(void)
{
    int rc = 0;              // return code
    errlHndl_t  err = NULL;  // Error handler

    do
    {
        // Caller must check the completion of previous invocation of pmcr set IPC call.
        // This check is a safety feature in case caller didn't check IPC is idle.
        if(!async_request_is_idle(&G_pmcr_set_req.request))
        {
            // an earlier PMCR update IPC call has not completed, trace and log an error
            TRAC_ERR("pgpe_pmcr_set: PMCR update IPC task is not Idle");

            /*
             * @errortype
             * @moduleid    PGPE_PMCR_SET_MOD
             * @reasoncode  PGPE_FAILURE
             * @userdata1   0
             * @userdata4   ERC_PGPE_NOT_IDLE
             * @devdesc     pgpe pmcr set not idle
             */
            err = createErrl(
                PGPE_PMCR_SET_MOD,               //ModId
                PGPE_FAILURE,                    //Reasoncode
                ERC_PGPE_NOT_IDLE,               //Extended reason code
                ERRL_SEV_PREDICTIVE,             //Severity
                NULL,                            //Trace Buf
                DEFAULT_TRACE_SIZE,              //Trace Size
                0,                               //Userdata1
                0                                //Userdata2
                );

            // Callout firmware
            addCalloutToErrl(err,
                             ERRL_CALLOUT_TYPE_COMPONENT_ID,
                             ERRL_COMPONENT_ID_FIRMWARE,
                             ERRL_CALLOUT_PRIORITY_HIGH);

            commitErrl(&err);

            rc = PGPE_FAILURE;
            break;
        }

        if (!G_simics_environment)
        {
            // Schedule PGPE PMCR update IPC task
            rc = gpe_request_schedule(&G_pmcr_set_req);
        }

        // Confirm Successfull completion of PGPE PMCR update task
        if(rc != 0)
        {
            //Error in scheduling pgpe PMCR update task
            TRAC_ERR("pgpe_pmcr_set: Failed to schedule PMCR setup pgpe task rc=%x",
                     rc);

            /* @
             * @errortype
             * @moduleid    PGPE_PMCR_SET_MOD
             * @reasoncode  GPE_REQUEST_SCHEDULE_FAILURE
             * @userdata1   rc - gpe_request_schedule return code
             * @userdata2   0
             * @userdata4   OCC_NO_EXTENDED_RC
             * @devdesc     OCC Failed to schedule a PGPE job for PMCR update
             */
            err = createErrl(
                PGPE_PMCR_SET_MOD,                      // modId
                GPE_REQUEST_SCHEDULE_FAILURE,           // reasoncode
                OCC_NO_EXTENDED_RC,                     // Extended reason code
                ERRL_SEV_UNRECOVERABLE,                 // Severity
                NULL,                                   // Trace Buf
                DEFAULT_TRACE_SIZE,                     // Trace Size
                rc,                                     // userdata1
                0                                       // userdata2
                );

            rc = GPE_REQUEST_SCHEDULE_FAILURE;
            REQUEST_RESET(err);   //This will add a firmware callout for us
        }
    }
    while(0);

    return rc;
}
