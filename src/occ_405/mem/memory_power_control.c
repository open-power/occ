/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/mem/memory_power_control.c $                      */
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

#include <occhw_async.h>
#include <mem_structs.h>
#include <memory_power_control.h>
#include <memory_service_codes.h>
#include <amec_sys.h>

// GPE Requests
GpeRequest G_mem_power_control_req;

// GPE arguments
GPE_BUFFER(mem_power_control_args_t  G_mem_power_control_args);

/**
 * GPE shared data area for gpe1 tracebuffer and size
 */
extern gpe_shared_data_t G_shared_gpe_data;


// Function Specification
//
// Name: is_occ_in_ips
//
// Description: checks whether OCC is inside IPS (Idle Power Save).
//
// returns a bool:
//            True: if OCC is in IPS now (IPS requested frequency is not 0)
//            False: if OCC is not in IPS
//
// End Function Specification

inline bool is_occ_in_ips(void)
{
    // Check if IPS frequency request is sent by Master OCC
    return (g_amec->slv_ips_freq_request != 0);
}


// Function Specification
//
// Name: amec_mem_power_control
//
// Description: Performs memory Power control -if needed- through an IPC task
//          sent to the GPE1. This function is called only inside active state.
//
// End Function Specification

// max number of tick cycles to wait while memory power control
// task is still busy processing previous contron task
#define WAIT_GPE_MEM_PWR_CTRL_BUSY_LIMIT   1

void amec_mem_power_control(void)
{
    // a boolean indicating that a transition occured, and that
    // updating the memory power control registers is still in progress.
    // Set once the transition starts, and clear when all mc/port
    // memory power control and STR registers are already set.
    static bool L_memory_power_control_in_progress = false;

    // track the next memIndex (MC pair and port) to send
    // memory power control settings to.
    static uint8_t L_memIndex = 0;

    static uint8_t L_wait_idle_gpe = 0;     // tick cycles waited while GPE is still not idle

    // New memory Power control setting
    uint8_t new_mem_pwr_ctl;

    // OCC is in Idle Power Save
    if(is_occ_in_ips())
    {
        new_mem_pwr_ctl = G_sysConfigData.ips_mem_pwr_ctl;
    }
    else
    {
        new_mem_pwr_ctl = G_sysConfigData.default_mem_pwr_ctl;
    }

    if(!L_memory_power_control_in_progress)
    {
        // Apply new memory power control only if different than the current
        // setting (already sent to GPE1), and previous IPS trnsition's
        // memory power control is not currently in progress:
        // set in progress latch and new memory power control parameter.
        if(new_mem_pwr_ctl != g_amec->sys.current_mem_pwr_ctl)
        {
            // start sending memory power control IPC messages.
            L_memory_power_control_in_progress = true;

            // update the current memory power control setting
            g_amec->sys.current_mem_pwr_ctl = new_mem_pwr_ctl;
        }
    }

    if(L_memory_power_control_in_progress)
    {
        // return code from gpe_mem_power_control()
        int rc = 0;

        // send memory power control settings only if MC/port is configured
        // (through memory throttle config packet)
        if(NIMBUS_DIMM_INDEX_THROTTLING_CONFIGURED(L_memIndex))
        {
            rc = gpe_mem_power_control(g_amec->sys.current_mem_pwr_ctl,
                                       L_memIndex, L_wait_idle_gpe);
        }

        // if memory power control task is not idle, don't increment,
        // DIMM index, just wait for up to WAIT_GPE_MEM_PWR_CTRL_BUSY_LIMIT
        // additional ticks.
        if( (rc != GPE_REQUEST_TASK_NOT_IDLE) ||
            (L_wait_idle_gpe >= WAIT_GPE_MEM_PWR_CTRL_BUSY_LIMIT) )
        {
            L_memIndex++;
            L_wait_idle_gpe = 0;
        }
        else  // GPE task is not idle, and L_wait_idle_gpe is still within limit
        {
            L_wait_idle_gpe++;
        }

        // Memory power control settings are sent to all MC/port control registers
        if(L_memIndex >= NUM_NIMBUS_MCAS)
        {
            // turn off the memory power control in progress latch,
            // and reset memory index variable.
            L_memory_power_control_in_progress = false;
            L_memIndex = 0;
        }
    }

}


// Function Specification
//
// Name: gpe_init_mem_power_control
//
// Description: create a gpe request IPC task on GPE1 for memory power control
//
// End Function Specification

void gpe_init_mem_power_control(void)
{
    int rc;                    // return code
    errlHndl_t  err = NULL;    // Error handler

    do
    {
        //Initializes the GpeRequest object for gpe memory control IPC
        rc = gpe_request_create(&G_mem_power_control_req,          // GpeRequest for task
                                &G_async_gpe_queue1,               // Queue
                                IPC_ST_MEM_POWER_CONTROL_FUNCID,   // Function ID
                                &G_mem_power_control_args,         // Task parameters
                                SSX_WAIT_FOREVER,                  // Timeout (none)
                                NULL,                              // Callback
                                NULL,                              // Callback arguments
                                ASYNC_CALLBACK_IMMEDIATE );        // Options

        if( rc )
        {
            // If we failed to create the GpeRequest then there is a serious problem.
            MAIN_TRAC_ERR("gpe_init_mem_power_control: Failure creating the "
                          "IPC_ST_MEM_POWER_CONTROL_FUNCID GpeRequest. [RC=0x%08x]",
                          rc );

            /*
             * @errortype
             * @moduleid    MEM_MID_MEM_INIT_POWER_CONTROL
             * @reasoncode  GPE_REQUEST_CREATE_FAILURE
             * @userdata1   gpe_request_create return code
             * @userdata4   OCC_NO_EXTENDED_RC
             * @devdesc     Failure to create memory power control GpeRequest object
             */
            err = createErrl(
                MEM_MID_MEM_INIT_POWER_CONTROL,  //ModId
                GPE_REQUEST_CREATE_FAILURE,      //Reasoncode
                OCC_NO_EXTENDED_RC,              //Extended reason code
                ERRL_SEV_PREDICTIVE,             //Severity
                NULL,                            //Trace Buf
                DEFAULT_TRACE_SIZE,              //Trace Size
                rc,                              //Userdata1
                0                                //Userdata2
                );

            REQUEST_RESET(err);
        }

    } while (0);

    return;
}


// Function Specification
//
// Name: gpe_mem_power_control
//
// Description: schedule a memory power control IPC task on GPE1
//
// End Function Specification

int gpe_mem_power_control(uint8_t mem_pwr_ctl, uint8_t mca, uint8_t wait_idle_gpe)
{
    int rc = 0;                               // return code
    errlHndl_t  err = NULL;                   // Error handler

    static bool L_busy_error_traced  = false; // IPC task still busy
    static bool L_fail_error_traced  = false; // IPC task completed with errors
    static bool L_sched_error_traced = false; // IPC task couldn't be scheduled

    do
    {
        // Check the completion of previous invocation of memory power control.
        if(!async_request_is_idle(&G_mem_power_control_req.request))
        {
            // Report idle GPEs once only, then no need to track idle wait cycles.
            if(!L_busy_error_traced)
            {
                // gpe_mem_power_control() will no longer be called for this DIMM in this transition
                if(wait_idle_gpe >= WAIT_GPE_MEM_PWR_CTRL_BUSY_LIMIT )
                {
                    // an earlier memory power control IPC has not completed, trace and log an error
                    TRAC_ERR("gpe_mem_power_control: memory power control IPC task is not Idle");

                    /*
                     * @errortype
                     * @moduleid    MEM_MID_GPE_MEM_POWER_CONTROL
                     * @reasoncode  GPE_REQUEST_TASK_NOT_IDLE
                     * @userdata1   0
                     * @userdata4   OCC_NO_EXTENDED_RC
                     * @devdesc     gpe memory power control task not idle
                     */
                    err = createErrl(
                        MEM_MID_GPE_MEM_POWER_CONTROL,   //ModId
                        GPE_REQUEST_TASK_NOT_IDLE,       //Reasoncode
                        OCC_NO_EXTENDED_RC,              //Extended reason code
                        ERRL_SEV_INFORMATIONAL,          //Severity
                        NULL,                            //Trace Buf
                        DEFAULT_TRACE_SIZE,              //Trace Size
                        0,                               //Userdata1
                        0                                //Userdata2
                        );

                    addUsrDtlsToErrl(err,
                                     (uint8_t *) G_shared_gpe_data.gpe1_tb_ptr,
                                     G_shared_gpe_data.gpe1_tb_sz,
                                     ERRL_USR_DTL_STRUCT_VERSION_1,
                                     ERRL_USR_DTL_TRACE_DATA);

                    commitErrl(&err);

                    L_busy_error_traced = true;
                }
            }

            rc = GPE_REQUEST_TASK_NOT_IDLE;


            break;
        }

        // Verify that last memory power control (if any) completed with no errors.
        if(GPE_RC_SUCCESS != G_mem_power_control_args.error.rc)
        {
            if(!L_fail_error_traced)
            {
                // an earlier memory power control IPC call returned an error,
                // trace and log that error
                TRAC_ERR("gpe_mem_power_control: memory power control IPC task returned an error"
                         "rc[%x], MC-Pair[%d], Port[%d]",
                         G_mem_power_control_args.error.rc,
                         G_mem_power_control_args.mc,
                         G_mem_power_control_args.port);

                /*
                 * @errortype
                 * @moduleid    MEM_MID_GPE_MEM_POWER_CONTROL
                 * @reasoncode  GPE_REQUEST_RC_FAILURE
                 * @userdata1   rc
                 * @userdata2   mca
                 * @userdata4   OCC_NO_EXTENDED_RC
                 * @devdesc     gpe memory power control task returned an error
                 */
                err = createErrl(
                    MEM_MID_GPE_MEM_POWER_CONTROL,        //ModId
                    GPE_REQUEST_RC_FAILURE,               //Reasoncode
                    OCC_NO_EXTENDED_RC,                   //Extended reason code
                    ERRL_SEV_INFORMATIONAL,               //Severity
                    NULL,                                 //Trace Buf
                    DEFAULT_TRACE_SIZE,                   //Trace Size
                    G_mem_power_control_args.error.rc,    //Userdata1
                    ((G_mem_power_control_args.mc<<2) +
                     G_mem_power_control_args.port)       //Userdata2
                    );

                commitErrl(&err);

                L_fail_error_traced = true;
            }

            rc = GPE_REQUEST_RC_FAILURE;

            break;
        }

        // set memory power control arguments to GPE1
        G_mem_power_control_args.mem_pwr_ctl = mem_pwr_ctl;
        G_mem_power_control_args.port = mca & 0x03;
        G_mem_power_control_args.mc   = mca >> 2;

        // Schedule GPE1 memory power control IPC task
        rc = gpe_request_schedule(&G_mem_power_control_req);

        // Confirm Successfull completion of GPE1 memory power control task
        if(rc != 0)
        {
            if(!L_sched_error_traced)
            {
                //Error in scheduling memory power control task
                TRAC_ERR("gpe_mem_power_control: Failed to schedule memory power "
                         "control task rc=%x. Can't perform memory Power Control",
                         rc);

                /* @
                 * @errortype
                 * @moduleid    MEM_MID_GPE_MEM_POWER_CONTROL
                 * @reasoncode  GPE_REQUEST_SCHEDULE_FAILURE
                 * @userdata1   rc - gpe_request_schedule return code
                 * @userdata2   0
                 * @userdata4   OCC_NO_EXTENDED_RC
                 * @devdesc     OCC Failed to schedule memory power control IPC task
                 */
                err = createErrl(
                    MEM_MID_GPE_MEM_POWER_CONTROL,          // modId
                    GPE_REQUEST_SCHEDULE_FAILURE,           // reasoncode
                    OCC_NO_EXTENDED_RC,                     // Extended reason code
                    ERRL_SEV_UNRECOVERABLE,                 // Severity
                    NULL,                                   // Trace Buf
                    DEFAULT_TRACE_SIZE,                     // Trace Size
                    rc,                                     // userdata1
                    0                                       // userdata2
                    );

                commitErrl(&err);
                L_sched_error_traced = true;
            }

            rc = GPE_REQUEST_SCHEDULE_FAILURE;
        }
    }while(0);

    return rc;
}
