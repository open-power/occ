/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/pgpe/pgpe_interface.c $                           */
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

//#define PGPE_DEBUG

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
#include "pgpe_shared.h"
#include "amec_sys.h"
#include "common.h"             // For ignore_pgpe_error()

// Maximum waiting time (usec) for clip update IPC task
#define CLIP_UPDATE_TIMEOUT 500  // maximum waiting time (usec) for clip update IPC task

extern volatile pstateStatus G_proc_pstate_status;
extern volatile PMCR_OWNER G_proc_pmcr_owner;
extern volatile bool G_set_pStates;

extern uint16_t G_proc_fmax_mhz;
extern uint32_t G_present_cores;

extern bool G_simics_environment;

extern uint16_t G_allow_trace_flags;
// IPC GPE Requests
GpeRequest G_clip_update_req;
GpeRequest G_pmcr_set_req;
GpeRequest G_start_suspend_req;
GpeRequest G_wof_control_req;
GpeRequest G_wof_vrt_req;

// GPE parameter fields for PGPE IPC calls
GPE_BUFFER(ipcmsg_clip_update_t   G_clip_update_parms);
GPE_BUFFER(ipcmsg_set_pmcr_t      G_pmcr_set_parms);
GPE_BUFFER(ipcmsg_start_stop_t    G_start_suspend_parms);
GPE_BUFFER(ipcmsg_wof_control_t   G_wof_control_parms);
GPE_BUFFER(ipcmsg_wof_vrt_t       G_wof_vrt_parms);

// Used to track PGPE return code of start_suspend callback
volatile int G_ss_pgpe_rc = PGPE_RC_SUCCESS;


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
        err = pgpe_init_wof_vrt();
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
// Name: pgpe_init_wof_vrt
//
// Description: Create PGPE WOF vrt IPC messages (non-blocking).
//
// End Function Specification
errlHndl_t pgpe_init_wof_vrt(void)
{
    int rc;                    // return code
    errlHndl_t  err = NULL;    // Error handler

    do
    {
        //Initializes the GpeRequest object for pgpe wof vrt IPC
        rc = gpe_request_create(&G_wof_vrt_req,             // GpeRequest for the task
                                &G_async_gpe_queue2,        // Queue
                                IPC_MSGID_405_WOF_VRT,      // Function ID
                                &G_wof_vrt_parms,           // Task parameters
                                SSX_WAIT_FOREVER,           // Timeout (none)
                                (AsyncRequestCallback)wof_vrt_callback, // Callback
                                NULL,                       // Callback arguments
                                ASYNC_CALLBACK_IMMEDIATE);  // Options

        if( rc )
        {
            // If we failed to create the GpeRequest then there is a serious problem.
            MAIN_TRAC_ERR("pgpe_init_wof_vrt: Failure creating the "
                          "IPC_MSGID_405_WOF_VRT GpeRequest. [RC=0x%08x]",
                          rc );

            /*
             * @errortype
             * @moduleid    PGPE_INIT_WOF_VRT_MOD
             * @reasoncode  GPE_REQUEST_CREATE_FAILURE
             * @userdata1   gpe_request_create return code
             * @userdata4   OCC_NO_EXTENDED_RC
             * @devdesc     Failure to create wof control GpeRequest object
             */
            err = createErrl(
                PGPE_INIT_WOF_VRT_MOD,          //ModId
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
int pgpe_set_clip_blocking(Pstate_t i_pstate)
{
    errlHndl_t   err = NULL;
    uint8_t      wait_time = 0;
    int          rc = 0;  // Return code: 0 means success, otherwise an error.

    PGPE_DBG("<<pgpe_set_clip_blocking(pstate 0x%02X)", i_pstate);
    do
    {
        // verify that the clip update IPC task is not currently running
        while( !async_request_is_idle(&G_clip_update_req.request) )
        {
            if(wait_time > CLIP_UPDATE_TIMEOUT)
            {
                // an earlier clip update IPC call has not completed, trace and log an error
                // only trace and log an error if we are not to ignore
                if(!ignore_pgpe_error())
                {
                    TRAC_ERR("pgpe_set_clip_blocking: clip update IPC task is not Idle");

                    /*
                     * @errortype
                     * @moduleid    PGPE_SET_CLIP_BLOCKING_MOD
                     * @reasoncode  PGPE_FAILURE
                     * @userdata4   ERC_PGPE_CLIP_NOT_IDLE
                     * @devdesc     pgpe clip update not idle
                     */
                    err = createPgpeErrl(PGPE_SET_CLIP_BLOCKING_MOD,    //ModId
                                         PGPE_FAILURE,                  //Reasoncode
                                         ERC_PGPE_CLIP_NOT_IDLE,        //Extended reason code
                                         ERRL_SEV_PREDICTIVE,           //Severity
                                         0,                             //Userdata1
                                         0);                            //Userdata2
                }
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
            // save the pstate and submit the update
            G_desired_pstate = i_pstate;
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
                // only trace and log an error if we are not to ignore
                if(!ignore_pgpe_error())
                {
                    TRAC_ERR("pgpe_set_clip_blocking: clip update IPC task timeout!");
                    /*
                     * @errortype
                     * @moduleid    PGPE_SET_CLIP_BLOCKING_MOD
                     * @reasoncode  GPE_REQUEST_TASK_TIMEOUT
                     * @userdata4   OCC_NO_EXTENDED_RC
                     * @devdesc     pgpe clip update timeout
                     */
                    err = createPgpeErrl(PGPE_SET_CLIP_BLOCKING_MOD,      //ModId
                                         GPE_REQUEST_TASK_TIMEOUT,        //Reasoncode
                                         OCC_NO_EXTENDED_RC,              //Extended reason code
                                         ERRL_SEV_PREDICTIVE,             //Severity
                                         0,                               //Userdata1
                                         0);                              //Userdata2
                }
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
            // clip update IPC call failed, trace and log an error
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
            err = createPgpeErrl(PGPE_SET_CLIP_BLOCKING_MOD,      //ModId
                                 GPE_REQUEST_RC_FAILURE,          //Reasoncode
                                 OCC_NO_EXTENDED_RC,              //Extended reason code
                                 ERRL_SEV_PREDICTIVE,             //Severity
                                 G_clip_update_parms.msg_cb.rc,   //Userdata1
                                 0);                              //Userdata2

            rc = GPE_REQUEST_RC_FAILURE;
        }
    } while (0);

    // request reset
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
    int ext_rc = 0;              // function return code
    int schedule_rc = 0;         // return code from PGPE schedule
    errlHndl_t  err = NULL;      // Error handler
    uint32_t    l_wait_time = 0;
    static unsigned int L_last_pstate = 0xFF;
    static bool L_first_trace = TRUE;

    PGPE_DBG("<<pgpe_clip_update()");
    // Caller must check the completion of previous invocation of clip updates.
    // This check is a safety feature in case caller didn't check IPC is idle.
    while(!async_request_is_idle(&G_clip_update_req.request))
    {
        if(l_wait_time > CLIP_UPDATE_TIMEOUT)
        {
            // only trace and log an error if we are not to ignore
            if(!ignore_pgpe_error())
            {
                // an earlier clip update IPC call has not completed, trace and log an error
                TRAC_ERR("pgpe_clip_update: clip update IPC task is not Idle");

                /*
                 * @errortype
                 * @moduleid    PGPE_CLIP_UPDATE_MOD
                 * @reasoncode  PGPE_FAILURE
                 * @userdata1   0
                 * @userdata4   ERC_PGPE_CLIP_NOT_IDLE
                 * @devdesc     pgpe clip update not idle
                 */
                err = createPgpeErrl(PGPE_CLIP_UPDATE_MOD,            //ModId
                                     PGPE_FAILURE,                    //Reasoncode
                                     ERC_PGPE_CLIP_NOT_IDLE,          //Extended reason code
                                     ERRL_SEV_PREDICTIVE,             //Severity
                                     0,                               //Userdata1
                                     0);                              //Userdata2

                // Callout firmware
                addCalloutToErrl(err,
                                 ERRL_CALLOUT_TYPE_COMPONENT_ID,
                                 ERRL_COMPONENT_ID_FIRMWARE,
                                 ERRL_CALLOUT_PRIORITY_HIGH);

                commitErrl(&err);
            }

            ext_rc = ERC_PGPE_CLIP_NOT_IDLE;
            break;
        }

        // Sleep for 1us (allows context switching)
        ssx_sleep(SSX_MICROSECONDS(1));

        l_wait_time++;
    }

    const unsigned int pstate = G_desired_pstate;

    // Set clip bounds
    uint32_t l_steps = 0;
    G_clip_update_parms.ps_val_clip_min = proc_freq2pstate(g_amec->sys.fmin, &l_steps);
    G_clip_update_parms.ps_val_clip_min += l_steps;
    G_clip_update_parms.ps_val_clip_max = pstate;

    // Always send request on PowerVM, on OPAL only send the request if there was a change or need to force a send
    if( (pstate != L_last_pstate) || (!G_sysConfigData.system_type.kvm) || (G_set_pStates) )
    {
        G_set_pStates = FALSE;
        if (L_first_trace)
        {
            TRAC_IMP("pgpe_clip_update: Scheduling clip update: min[0x%02X], max[0x%02X]",
                     G_clip_update_parms.ps_val_clip_min, pstate);
            //L_first_trace = FALSE;
        }
        // always trace change on PowerVM since setting clips is very rare with PowerVM which uses PMCR set
        else if( (G_allow_trace_flags & ALLOW_CLIP_TRACE) || (!G_sysConfigData.system_type.kvm) )
        {
            TRAC_INFO("pgpe_clip_update: Scheduling clip update: min[0x%02X], max[0x%02X]",
                      G_clip_update_parms.ps_val_clip_min, pstate);
        }
        L_last_pstate = pstate;

        // Schedule PGPE clip update IPC task
        schedule_rc = pgpe_request_schedule(&G_clip_update_req);
    }

    // Confirm Successfull completion of PGPE clip update task
    if(schedule_rc != 0)
    {
        //Error in scheduling pgpe clip update task
        TRAC_ERR("pgpe_clip_update: Failed to schedule clip update pgpe task rc=%x",
                 schedule_rc);

        /* @
         * @errortype
         * @moduleid    PGPE_CLIP_UPDATE_MOD
         * @reasoncode  GPE_REQUEST_SCHEDULE_FAILURE
         * @userdata1   rc - gpe_request_schedule return code
         * @userdata2   max pstate
         * @userdata4   ERC_PGPE_CLIP_FAILURE
         * @devdesc     OCC Failed to schedule a GPE job for clip update
         */
        err = createPgpeErrl(PGPE_CLIP_UPDATE_MOD,            // modId
                             GPE_REQUEST_SCHEDULE_FAILURE,    // reasoncode
                             ERC_PGPE_CLIP_FAILURE,           // Extended reason code
                             ERRL_SEV_UNRECOVERABLE,          // Severity
                             schedule_rc,                     // userdata1
                             pstate);                         // Userdata2

        ext_rc = ERC_PGPE_CLIP_FAILURE;
        REQUEST_RESET(err);   //This will add a firmware callout for us
    }

    return ext_rc;

} // end pgpe_clip_update()


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
    G_ss_pgpe_rc = G_start_suspend_parms.msg_cb.rc;

    do
    {
        // Confirm Successfull completion of PGPE start suspend task
        if(G_ss_pgpe_rc != PGPE_RC_SUCCESS)
        {
            //Error in scheduling pgpe start suspend task
            G_proc_pstate_status = PSTATES_FAILED;
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
    int ext_rc = 0;            // return code
    int schedule_rc = 0;       // return code
    errlHndl_t  err = NULL;    // Error handler
    pstateStatus l_current_pstate_status = G_proc_pstate_status;

    // This task is only called on state changes and state changes should
    // always wait for this task to complete therefore this task should always
    // be idle when called.
    if(!async_request_is_idle(&G_start_suspend_req.request))
    {
        // only trace and log an error if we are not to ignore
        if(!ignore_pgpe_error())
        {
            TRAC_ERR("pgpe_start_suspend: Start suspend task NOT Idle");

            /*
             * @errortype
             * @moduleid    PGPE_START_SUSPEND_MOD
             * @reasoncode  PGPE_FAILURE
             * @userdata1   0
             * @userdata4   ERC_PGPE_START_SUSPEND_NOT_IDLE
             * @devdesc     pgpe start suspend task not idle
             */
            err = createPgpeErrl(PGPE_START_SUSPEND_MOD,          //ModId
                                 PGPE_FAILURE,                    //Reasoncode
                                 ERC_PGPE_START_SUSPEND_NOT_IDLE, //Extended reason code
                                 ERRL_SEV_PREDICTIVE,             //Severity
                                 0,                               //Userdata1
                                 0);                              //Userdata2

            // Callout firmware
            addCalloutToErrl(err,
                             ERRL_CALLOUT_TYPE_COMPONENT_ID,
                             ERRL_COMPONENT_ID_FIRMWARE,
                             ERRL_CALLOUT_PRIORITY_HIGH);

            commitErrl(&err);
        }
        ext_rc = ERC_PGPE_START_SUSPEND_NOT_IDLE;
    }

    else  // start suspend task is idle, ok to schedule
    {
       // set the IPC parameters
       G_start_suspend_parms.action     = action;
       G_start_suspend_parms.pmcr_owner = owner;

#if OCCHW_IRQ_ROUTE_OWNER == 4 // 405
       if (!G_simics_environment)
#endif
       {
           // set the G_proc_pstate_status to indicate transition
           G_proc_pstate_status = PSTATES_IN_TRANSITION;

           if (action == PGPE_ACTION_PSTATE_START)
           {
               TRAC_IMP("pgpe_start_suspend: scheduling enable of pstates (owner=0x%02X)", owner);
           }
           else if (action == PGPE_ACTION_PSTATE_STOP)
           {
               TRAC_IMP("pgpe_start_suspend: scheduling disable of pstates (owner=0x%02X)", owner);
           }

           // Schedule PGPE start_suspend task
           schedule_rc = pgpe_request_schedule(&G_start_suspend_req);
       }

       // couldn't schedule the IPC task?
       if(schedule_rc != 0)
       {
           // restore pState status since it isn't going to change
           G_proc_pstate_status = l_current_pstate_status;

           //Error in scheduling pgpe start suspend task
           TRAC_ERR("pgpe_start_suspend: Failed to schedule pgpe start_suspend task rc=%x",
                    schedule_rc);

           ext_rc = ERC_PGPE_START_SUSPEND_FAILURE;

           // Do NOT log an error and reset if this was called in OPAL
           // environment to set nominal prior to host control (OCC ownership)
           // if there is a real problem with setting ownership the error and
           // reset will happen when setting owner to Host
           if( (!G_sysConfigData.system_type.kvm) || (owner != PMCR_OWNER_OCC) )
           {

               /* @
                * @errortype
                * @moduleid    PGPE_START_SUSPEND_MOD
                * @reasoncode  GPE_REQUEST_SCHEDULE_FAILURE
                * @userdata1   rc - gpe_request_schedule return code
                * @userdata4   ERC_PGPE_START_SUSPEND_FAILURE
                * @devdesc     OCC Failed to schedule a PGPE job for start_suspend
                */
               err = createPgpeErrl(PGPE_START_SUSPEND_MOD,          // modId
                                    GPE_REQUEST_SCHEDULE_FAILURE,    // reasoncode
                                    ERC_PGPE_START_SUSPEND_FAILURE,  // Extended reason code
                                    ERRL_SEV_UNRECOVERABLE,          // Severity
                                    schedule_rc,                     // userdata1
                                    0);                              //Userdata2

               REQUEST_RESET(err);   //This will add a firmware callout for us
           }
      }

#if OCCHW_IRQ_ROUTE_OWNER == 4 // 405
      if (G_simics_environment)
      {
          pgpe_start_suspend_callback();
      }
#endif
    }

    return ext_rc;
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
    int ext_rc = 0;          // function return code
    int schedule_rc = 0;     // RC from schedule request
    errlHndl_t  err = NULL;  // Error handler

    do
    {
        // Caller must check the completion of previous invocation of pmcr set IPC call.
        // This check is a safety feature in case caller didn't check IPC is idle.
        if(!async_request_is_idle(&G_pmcr_set_req.request))
        {
            // an earlier PMCR update IPC call has not completed
            // only trace and log an error if we are not to ignore
            if(!ignore_pgpe_error())
            {
                TRAC_ERR("pgpe_pmcr_set: PMCR update IPC task is not Idle");

                /*
                 * @errortype
                 * @moduleid    PGPE_PMCR_SET_MOD
                 * @reasoncode  PGPE_FAILURE
                 * @userdata1   0
                 * @userdata4   ERC_PGPE_SET_PMCR_NOT_IDLE
                 * @devdesc     pgpe pmcr set not idle
                 */
                err = createPgpeErrl(PGPE_PMCR_SET_MOD,               //ModId
                                     PGPE_FAILURE,                    //Reasoncode
                                     ERC_PGPE_SET_PMCR_NOT_IDLE,      //Extended reason code
                                     ERRL_SEV_PREDICTIVE,             //Severity
                                     0,                               //Userdata1
                                     0);                              //Userdata2

                // Callout firmware
                addCalloutToErrl(err,
                                 ERRL_CALLOUT_TYPE_COMPONENT_ID,
                                 ERRL_COMPONENT_ID_FIRMWARE,
                                 ERRL_CALLOUT_PRIORITY_HIGH);

                commitErrl(&err);
            }
            ext_rc = ERC_PGPE_SET_PMCR_NOT_IDLE;
            break;
        }

#if OCCHW_IRQ_ROUTE_OWNER == 4 // 405
        if (!G_simics_environment)
#endif
        {
            // Schedule PGPE PMCR update IPC task
            schedule_rc = pgpe_request_schedule(&G_pmcr_set_req);
        }

        // Confirm Successfull completion of PGPE PMCR update task
        if(schedule_rc != 0)
        {
            //Error in scheduling pgpe PMCR update task
            TRAC_ERR("pgpe_pmcr_set: Failed to schedule PMCR set pgpe task rc=%x",
                     schedule_rc);
            ext_rc = ERC_PGPE_SET_PMCR_FAILURE;

            // Do NOT log an error and reset if this was called in OPAL
            // environment to set nominal prior to host control
            if(!G_sysConfigData.system_type.kvm)
            {
                /* @
                 * @errortype
                 * @moduleid    PGPE_PMCR_SET_MOD
                 * @reasoncode  GPE_REQUEST_SCHEDULE_FAILURE
                 * @userdata1   rc - gpe_request_schedule return code
                 * @userdata2   0
                 * @userdata4   ERC_PGPE_SET_PMCR_FAILURE
                 * @devdesc     OCC Failed to schedule a PGPE job for PMCR update
                 */
                err = createPgpeErrl(PGPE_PMCR_SET_MOD,               // modId
                                     GPE_REQUEST_SCHEDULE_FAILURE,    // reasoncode
                                     ERC_PGPE_SET_PMCR_FAILURE,       // Extended reason code
                                     ERRL_SEV_UNRECOVERABLE,          // Severity
                                     schedule_rc,                     // userdata1
                                     0);                              //Userdata2

                REQUEST_RESET(err);   //This will add a firmware callout for us
            }
        }
    }
    while(0);

    return ext_rc;
}

// Function Specification
//
// Name: set_nominal_pstate
//
// Description: Make OCC the PMCR owner (if not already) and set the Pstate to nominal
//              NOTE:  If this function needed to start Pstate protocol with OCC owner
//                     it will exit that way.  The caller is responsible to decide what
//                     to do with PMCR ownership / Pstate protocol afterwards
//
// End Function Specification
int set_nominal_pstate(void)
{
    int l_rc = 0;
    SsxTimebase l_start = ssx_timebase_get();
    SsxInterval l_timeout =  SSX_SECONDS(1);
    uint8_t     l_pstate;

   do
   {
       // Make sure we have received the frequency config data to know what nominal is
       if( (G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MODE_DISABLED] == 0) )
       {
            TRAC_ERR("set_nominal_pstate: frequency not known!");
            l_rc = ERC_FW_ZERO_FREQ_LIMIT;
            break;
       }
       // Make sure the set PMCR task is idle.
       if(!async_request_is_idle(&G_pmcr_set_req.request))
       {
            TRAC_ERR("set_nominal_pstate: Set PMCR task not idle! OCCFLG0[0x%08X]", in32(OCB_OCCFLG0));
            l_rc = ERC_PGPE_SET_PMCR_NOT_IDLE;
            break;
       }

       // Check if we need to enable Pstate protocol with OCC ownership
       if( (G_proc_pstate_status != PSTATES_ENABLED) ||
           (G_proc_pmcr_owner != PMCR_OWNER_OCC) )
       {
           // This should not be called if Pstate protocol is in transition
           if(G_proc_pstate_status == PSTATES_IN_TRANSITION)
           {
               TRAC_ERR("set_nominal_pstate: Pstate protocol in transtion! OCCFLG0[0x%08X]", in32(OCB_OCCFLG0));
               l_rc = ERC_PGPE_START_SUSPEND_NOT_IDLE;
               break;
           }

           l_rc = pgpe_start_suspend(PGPE_ACTION_PSTATE_START, PMCR_OWNER_OCC);
           if(l_rc)
           {
               TRAC_ERR("set_nominal_pstate: Failed to start pstate protocol rc[0x%04X]", l_rc);
               break;
           }
           // Wait for pstates to be enabled
           ssx_sleep(SSX_MICROSECONDS(5));
           while(G_proc_pstate_status != PSTATES_ENABLED)
           {
               if((ssx_timebase_get() - l_start) > l_timeout)
               {
                  l_rc = ERC_PGPE_TASK_TIMEOUT;
                  TRAC_ERR("set_nominal_pstate: Timeout waiting for Pstates to be enabled! OCCFLG0[0x%08X]", in32(OCB_OCCFLG0));
                  break;
               }
               ssx_sleep(SSX_MICROSECONDS(10));
           }
           if(l_rc)
           {
               break;
           }
       }

       // Pstate protocol is now enabled for OCC to set Pstate to mode off point
       uint32_t l_steps = 0;
       l_pstate = proc_freq2pstate(G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MODE_DISABLED], &l_steps);
       // Update pmcr value with nominal pstate and version (Version 1: Fast Request Mode)
       G_pmcr_set_parms.pmcr = ((uint64_t)l_pstate << 48) | 1;

       //call PGPE IPC function to set Pstates
       l_rc = pgpe_pmcr_set();
       if(l_rc == 0)
       {
           TRAC_IMP("set_nominal_pstate: Successfully set pstate[0x%02X]", l_pstate);
       }
       else
       {
           TRAC_ERR("set_nominal_pstate: Set pstate[0x%02X] failed with rc[0x%04X]", l_pstate, l_rc);
       }

   } while(0);

   return l_rc;
}

// Function Specification
//
// Name: pgpe_request_schedule
//
// Description: Wrapper function that takes care of any overhead associated
//              with scheduling a GpeRequest to the PGPE
//
// End Function Specification
int pgpe_request_schedule(GpeRequest* request)
{
    int rc = 0;
    static uint8_t L_traceCount = 10;
    // Before sending any IPC task to the PGPE, make sure there are cores
    // configured
    if( G_present_cores != 0 )
    {
        PGPE_DBG("<<pgpe_request_schedule - calling gpe_request_schedule()");
        rc = gpe_request_schedule(request);
    }
    else
    {
        if( L_traceCount )
        {
            // If we skip the IPC task, just send successful return code
            TRAC_INFO("No Configured Cores. Skipping PGPE-bound IPC tasks");
            L_traceCount--;
        }
    }
    return rc;
}
