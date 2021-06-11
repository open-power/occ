/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/mem/memory.c $                                    */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2016,2019                        */
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

#include <trac_interface.h>
#include <trac.h>

#include "memory.h"
#include "memory_data.h"
#include "ocmb_membuf.h"
#include "membuf_structs.h"
#include "memory_service_codes.h"
#include <occ_service_codes.h>  // for SSX_GENERIC_FAILURE
#include "amec_sys.h"

extern bool G_mem_monitoring_allowed;
extern task_t G_task_table[TASK_END];

//bitmap of configured MBA's (2 per memory buffer, lsb is membuf 0/mba 0)
//same variable used for tracking bit maps of rdimms (4 per mc, 2 mc pairs)
uint16_t G_configured_mbas = 0;

extern MemBufScomParms_t G_membuf_control_reg_parms;

// This array identifies dimm throttle limits
memory_throttle_t G_memoryThrottleLimits[MAX_NUM_MEM_CONTROLLERS] = {{0}};

//Memory structure used for task data pointers
memory_control_task_t G_memory_control_task =
{
    .startMemIndex        = 0,                            // First Memory Control Index (membuf/MC_pair|port)
    .prevMemIndex         = (MAX_NUM_MEM_CONTROLLERS-2),  // Previous Memory Control Index written to
    .curMemIndex          = (MAX_NUM_MEM_CONTROLLERS-1),  // Current Memory Control Index
    .endMemIndex          = (MAX_NUM_MEM_CONTROLLERS-1),  // Last Memory Control Index
    .traceThresholdFlags  = 0,                            // Trace Throttle Flags
};

void ocmb_init(void);


// Function Specification
//
// Name: task_memory_control
//
// Description: Performs system memory control: to control memory power
//
// End Function Specification

// wait up to 16 ticks before logging timeout failure
#define MEMORY_CONTROL_SCOM_TIMEOUT 16

void task_memory_control( task_t * i_task )
{
    //track # of consecutive failures on a specific membuf/DIMM
    static uint8_t L_scom_timeout[MAX_NUM_MEM_CONTROLLERS] = {0};

    errlHndl_t     l_err     = NULL;    // Error handler
    int            rc        = 0;       // Return code
    bool           change_requested  = FALSE;
    uint8_t        memIndex;
    static bool    L_gpe_scheduled   = FALSE;
    static bool    L_gpe_idle_traced = FALSE;
    static bool    L_gpe_had_1_tick  = FALSE;

    uint32_t       gpe_rc = 0;

    // Pointer to the task data structure
    memory_control_task_t* memControlTask =  (memory_control_task_t*) i_task->data_ptr;

    gpe_rc = G_membuf_control_reg_parms.error.rc;

    do
    {
        memIndex = memControlTask->curMemIndex;

        //First, check to see if the previous GPE request still running
        //A request is considered idle if it is not attached to any of the
        //asynchronous request queues
        if( !(async_request_is_idle(&memControlTask->gpe_req.request)) )
        {
            L_scom_timeout[memIndex]++;
            //This can happen due to variability in when the task runs
            if(!L_gpe_idle_traced && L_gpe_had_1_tick)
            {
                TRAC_INFO("task_memory_control: GPE is still running. memIndex[%d]", memIndex);
                memControlTask->traceThresholdFlags |= MEMORY_CONTROL_GPE_STILL_RUNNING;
                L_gpe_idle_traced = TRUE;
            }
            L_gpe_had_1_tick = TRUE;
            break;
        }
        else
        {
            //Request is idle
            L_gpe_had_1_tick = FALSE;
            if(L_gpe_idle_traced)
            {
                TRAC_INFO("task_memory_control: GPE completed. memIndex[%d]", memIndex);
                L_gpe_idle_traced = FALSE;
            }
        }

        //check scom status
        if(L_gpe_scheduled)
        {
            if(!async_request_completed(&memControlTask->gpe_req.request) || gpe_rc)
            {
                //Request failed. Keep count of failures and request a reset if we reach a
                //max retry count
                //Assume the requested speed didn't take effect, so clear it so it gets resent.
                g_amec->proc[0].memctl[memIndex].membuf.portpair[0].last_mem_speed_sent.word32 = 0;
                L_scom_timeout[memIndex]++;
                if(L_scom_timeout[memIndex] == MEMORY_CONTROL_SCOM_TIMEOUT)
                {
                    break;
                }
            }
            else
            {
                //request completed successfully.  reset the timeout.
                L_scom_timeout[memIndex] = 0;
            }
        }

        //The previous GPE job completed, get ready for the next job.
        L_gpe_scheduled = FALSE;

        //Update current dimm/membuf index if we didn't fail
        memControlTask->prevMemIndex = memIndex;
        if ( memIndex >= memControlTask->endMemIndex )
        {
            memIndex  = memControlTask->startMemIndex;
        }
        else
        {
            memIndex++;
        }
        memControlTask->curMemIndex = memIndex;

        change_requested = ocmb_control(memControlTask);
        if(change_requested)
        {
            rc = gpe_request_schedule(&G_memory_control_task.gpe_req);

            if( rc )
            {
                //Error in schedule gpe memory (dimm/membuf) control
                TRAC_ERR("task_memory_control: Failed to schedule memory control gpe rc=%x",
                         rc);

                /* @
                 * @errortype
                 * @moduleid    MEM_MID_TASK_MEMORY_CONTROL
                 * @reasoncode  SSX_GENERIC_FAILURE
                 * @userdata1   rc - Return code of failing function
                 * @userdata2   0
                 * @userdata4   ERC_MEM_CONTROL_SCHEDULE_FAILURE
                 * @devdesc     OCC Failed to schedule a GPE job for memory control
                 */
                l_err = createErrl(
                    MEM_MID_TASK_MEMORY_CONTROL,            // modId
                    SSX_GENERIC_FAILURE,                    // reasoncode
                    ERC_MEM_CONTROL_SCHEDULE_FAILURE,       // Extended reason code
                    ERRL_SEV_UNRECOVERABLE,                 // Severity
                    NULL,                                   // Trace Buf
                    DEFAULT_TRACE_SIZE,                     // Trace Size
                    rc,                                     // userdata1
                    gpe_rc                                  // userdata2
                    );

                addUsrDtlsToErrl(
                    l_err,                                        //io_err
                    (uint8_t *) &(memControlTask->gpe_req.ffdc),  //i_dataPtr,
                    sizeof(memControlTask->gpe_req.ffdc),         //i_size
                    ERRL_USR_DTL_STRUCT_VERSION_1,                //version
                    ERRL_USR_DTL_BINARY_DATA);                    //type

                REQUEST_RESET(l_err);   //This will add a firmware callout for us
                break;
            }

            L_gpe_scheduled = TRUE;
        }

    } while(0);

    //Global membuf structures used for task data pointers

    if(L_scom_timeout[memIndex] == MEMORY_CONTROL_SCOM_TIMEOUT)
    {
        TRAC_ERR("task_memory_control: Timeout scomming memory buffer[%d]", memIndex);
        /* @
         * @errortype
         * @moduleid    MEM_MID_TASK_MEMORY_CONTROL
         * @reasoncode  INTERNAL_FAILURE
         * @userdata1   membuf/memIndex number
         * @userdata2   0
         * @userdata4   OCC_NO_EXTENDED_RC
         * @devdesc     Timed out trying to set the memory throttle settings
         *              throttle settings.
         */
        l_err = createErrl(
                MEM_MID_TASK_MEMORY_CONTROL,            // modId
                INTERNAL_FAILURE,                       // reasoncode
                OCC_NO_EXTENDED_RC,                     // Extended reason code
                ERRL_SEV_PREDICTIVE,                    // Severity
                NULL,                                   // Trace Buf
                DEFAULT_TRACE_SIZE,                     // Trace Size
                memIndex,                               // userdata1
                0                                       // userdata2
                );

        addUsrDtlsToErrl(l_err,                               //io_err
                (uint8_t *) &(memControlTask->gpe_req.ffdc),  //i_dataPtr,
                sizeof(memControlTask->gpe_req.ffdc),         //i_size
                ERRL_USR_DTL_STRUCT_VERSION_1,                //version
                ERRL_USR_DTL_BINARY_DATA);                    //type

        //callout the membuf
        addCalloutToErrl(l_err,
                         ERRL_CALLOUT_TYPE_HUID,
                         G_sysConfigData.membuf_huids[memIndex],
                         ERRL_CALLOUT_PRIORITY_MED);

        //callout the processor
        addCalloutToErrl(l_err,
                         ERRL_CALLOUT_TYPE_HUID,
                         G_sysConfigData.proc_huid,
                         ERRL_CALLOUT_PRIORITY_MED);

        REQUEST_RESET(l_err);
    }

    return;
}


// Initialize the memory task data
void memory_init()
{
    if(G_mem_monitoring_allowed)
    {
        // Check if memory task is running
        if(!rtl_task_is_runnable(TASK_ID_MEMORY_DATA))
        {
            if (MEM_TYPE_OCM == G_sysConfigData.mem_type)
            {
                ocmb_init();
            }
            else
            {
                TRAC_ERR("memory_init: Uknown memory type");
            }

            // check if the init resulted in a reset
            if(isSafeStateRequested())
            {
                TRAC_ERR("memory_init: OCC is being reset, memory init failed (type=0x%02X)",
                         G_sysConfigData.mem_type);
            }
            else
            {
                // Initialization was successful.  Set task flags to allow memory
                // tasks to run and also prevent from doing initialization again.
                G_task_table[TASK_ID_MEMORY_DATA].flags = MEMORY_DATA_RTL_FLAGS;
                G_task_table[TASK_ID_MEMORY_CONTROL].flags = MEMORY_CONTROL_RTL_FLAGS;
            }
        }
    }

} // end memory_init()


void disable_all_dimms()
{
    if (G_mem_monitoring_allowed)
    {
        TRAC_INFO("disable_all_dimms: DIMM temp collection is being stopped");
        G_mem_monitoring_allowed = false;
    }
}

