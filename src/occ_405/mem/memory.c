/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/mem/memory.c $                                    */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2014,2018                        */
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
#include "memory_power_control.h"
#include "dimm_control.h"
#include "centaur_control.h"
#include "centaur_data.h"
#include "memory_service_codes.h"
#include <occ_service_codes.h>  // for SSX_GENERIC_FAILURE
#include "amec_sys.h"

extern bool G_mem_monitoring_allowed;
extern dimm_control_args_t  G_dimm_control_args;
extern task_t G_task_table[TASK_END];


// TODO: RTC 163359  - uncomment when Centaur code is enabled
//extern GpeScomParms G_centaur_control_reg_parms;

// This array identifies dimm throttle limits for both Centaurs (Cumulus) and
// rdimms (Nimbus) based systems.
//
// For Nimbus systems, only the first two rows (corresponding to memory controller
// pairs M01 and M23) are used.
//
// For Cumulus systems, only the first two columns (corresponding to the two mba
// pairs mba01 and mba23) are used.
memory_throttle_t G_memoryThrottleLimits[MAX_NUM_MEM_CONTROLLERS][MAX_NUM_MCU_PORTS] = {{{0}}};

//Memory structure used for task data pointers in both Cumulus (Centaur)
//and Nimbus (RDIMM) systems.
memory_control_task_t G_memory_control_task =
{
    .startMemIndex        = 0,  // First Memory Control Index (Centaur/MC_pair|port)
    .prevMemIndex         = 7,  // Previous Memory Control Index written to
    .curMemIndex          = 0,  // Current Memory Control Index
    .endMemIndex          = 7,  // Last Memory Control Index
    .traceThresholdFlags  = 0,  // Trace Throttle Flags
};


// Function Specification
//
// Name: task_memory_control
//
// Description: Performs system memory control:
//             - for Nimbus:  calls dimm_control() to control dimms power
//             - for Cumulus: calls centaur_control() to control centaur power
//
// End Function Specification

// wait up to 16 ticks before logging timeout failure
#define MEMORY_CONTROL_SCOM_TIMEOUT 16

void task_memory_control( task_t * i_task )
{
    //track # of consecutive failures on a specific Centaur/RDIMM
    static uint8_t L_scom_timeout[MAX_NUM_MEM_CONTROLLERS] = {0};

    errlHndl_t     l_err     = NULL;    // Error handler
    int            rc        = 0;       // Return code
    uint8_t        memIndex;
    static bool    L_gpe_scheduled   = FALSE;
    static uint8_t L_gpe_fail_logged = 0;
    static bool    L_gpe_idle_traced = FALSE;
    static bool    L_gpe_had_1_tick  = FALSE;

    uint32_t       gpe_rc = 0;

    uint8_t mc   = 0;
    uint8_t port = 0;

    // Pointer to the task data structure
    memory_control_task_t* memControlTask =  (memory_control_task_t*) i_task->data_ptr;


    if (MEM_TYPE_NIMBUS ==  G_sysConfigData.mem_type)
    {
        gpe_rc = G_dimm_control_args.error.rc;
    }
    else if (MEM_TYPE_CUMULUS ==  G_sysConfigData.mem_type)
    {
// TODO: RTC 163359 - uncomment when Centaur code is enabled
//        gpe_rc = G_centaur_control_reg_parms.rc;
    }

    do
    {
        memIndex = memControlTask->curMemIndex;
        mc   = memIndex>>2;
        port = memIndex&3;

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
                if (MEM_TYPE_CUMULUS ==  G_sysConfigData.mem_type)
                {
                    if(!(L_gpe_fail_logged & (CENTAUR0_PRESENT_MASK >> memIndex)))
                    {
// TODO: RTC 163359 - uncomment when Centaur code is enabled
/*                        if (!check_centaur_checkstop(memIndex))
                        {
                            L_gpe_fail_logged |= CENTAUR0_PRESENT_MASK >> memIndex;
                            }
*/
                    }
                }
                //Request failed. Keep count of failures and request a reset if we reach a
                //max retry count
                L_scom_timeout[memIndex]++;
                if(L_scom_timeout[memIndex] == MEMORY_CONTROL_SCOM_TIMEOUT)
                {
                    break;
                }

            }//if(!async_request_completed(&memControlTask->gpe_req.request) || l_parms->rc)
            else
            {
                //request completed successfully.  reset the timeout.
                L_scom_timeout[memIndex] = 0;

                // Store M value if needed
                if(G_dimm_control_args.dimmDenominatorValues.need_m)
                {
                    g_amec->sys.dimm_m_values[mc][port].m_value =
                        G_dimm_control_args.dimmDenominatorValues.m_value;
                    g_amec->sys.dimm_m_values[mc][port].need_m = FALSE;
                    TRAC_INFO("M Value for MC%d P%d is 0x%08X", mc, port, g_amec->sys.dimm_m_values[mc][port].m_value);
                }
            }
        }//if(L_gpe_scheduled)

        //The previous GPE job completed, get ready for the next job.
        L_gpe_scheduled = FALSE;

        //Update current dimm/centaur index if we didn't fail
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

        if (MEM_TYPE_NIMBUS ==  G_sysConfigData.mem_type)
        {
            if(!NIMBUS_DIMM_INDEX_THROTTLING_CONFIGURED(memIndex))
            {
                break;
            }

            // control dimm specified by mc,port
            mc   = memIndex>>2;
            port = memIndex&3;

            // Do the update_nlimit, calculate new N values, check whether throttle values
            // were updated, then Schedule GPE request, rc if problem, else L_gpe_schedule
            rc = dimm_control(mc, port);

            MEM_DBG("memIndex=%d, mc|port=0x%%04X, rc=%d",
                    memIndex, mc<<8| port, rc);

        }
        else if (MEM_TYPE_CUMULUS ==  G_sysConfigData.mem_type)
        {
            //If centaur is not present or neither MBA is configured then skip it.
            if(!CENTAUR_PRESENT(memIndex) ||
               (!MBA_CONFIGURED(memIndex, 0) && !MBA_CONFIGURED(memIndex, 1)))
            {
                break;
            }
// TODO RTC: 163359 - centaur code not ready yet
//            rc = centaur_control(memIndex);  // Control one centaur
        }

        if(rc)
        {
            rc = gpe_request_schedule(&G_memory_control_task.gpe_req);

            if( rc )
            {
                if (MEM_TYPE_NIMBUS ==  G_sysConfigData.mem_type)
                {
                    gpe_rc = G_dimm_control_args.error.rc;
                }
                else if (MEM_TYPE_CUMULUS ==  G_sysConfigData.mem_type)
                {
// TODO RTC: 163359 - uncomment when Centaur code is enabled
//              gpe_rc = G_centaur_control_reg_parms.rc;
                }

                //Error in schedule gpe memory (dimm/centaur) control
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


//Global centaur structures used for task data pointers


    if(L_scom_timeout[memIndex] == MEMORY_CONTROL_SCOM_TIMEOUT)
    {
        if (MEM_TYPE_NIMBUS ==  G_sysConfigData.mem_type)
        {
            if(memIndex>>2)
            {
                TRAC_ERR("task_memory_control: Timeout scomming MC23:port[%d]", memIndex&3);
            }
            else
            {
                TRAC_ERR("task_memory_control: Timeout scomming MC01:port[%d]", memIndex&3);
            }
        }
        else if (MEM_TYPE_CUMULUS ==  G_sysConfigData.mem_type)
        {
            TRAC_ERR("task_memory_control: Timeout scomming centaur[%d]", memIndex);
        }
        /* @
         * @errortype
         * @moduleid    MEM_MID_TASK_MEMORY_CONTROL
         * @reasoncode  INTERNAL_FAILURE
         * @userdata1   centaur/memIndex number
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

        //callout the centaur
        addCalloutToErrl(l_err,
                         ERRL_CALLOUT_TYPE_HUID,
                         G_sysConfigData.centaur_huids[memIndex],
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
        // Check if memory task is running (default task is for NIMBUS)
        const task_id_t mem_task = TASK_ID_DIMM_SM;
        if(!rtl_task_is_runnable(mem_task))
        {
            if (MEM_TYPE_NIMBUS ==  G_sysConfigData.mem_type)
            {
                // Init DIMM state manager IPC request
                memory_nimbus_init();
            }
            else
            {
                // TODO RTC: 163359 - CUMULUS NOT SUPPORTED YET IN PHASE1
#if 0
                TRAC_INFO("memory_init: calling centaur_init()");
                centaur_init(); //no rc, handles errors internally
#endif
                TRAC_ERR("memory_init: invalid memory type 0x%02X", G_sysConfigData.mem_type);
                /*
                 * @errortype
                 * @moduleid    MEM_MID_MEMORY_INIT
                 * @reasoncode  MEMORY_INIT_FAILED
                 * @userdata1   memory type
                 * @userdata2   0
                 * @devdesc     Invalid memory type detected
                 */
                errlHndl_t err = createErrl(MEM_MID_MEMORY_INIT,
                                            MEMORY_INIT_FAILED,
                                            OCC_NO_EXTENDED_RC,
                                            ERRL_SEV_PREDICTIVE,
                                            NULL,
                                            DEFAULT_TRACE_SIZE,
                                            G_sysConfigData.mem_type,
                                            0);
                REQUEST_RESET(err);
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
                G_task_table[mem_task].flags = MEMORY_DATA_RTL_FLAGS;
                G_task_table[TASK_ID_MEMORY_CONTROL].flags = MEMORY_CONTROL_RTL_FLAGS;
            }
        }
    }

    // if memory power control is enabled (version 21 memory configurtion
    // command is received), create GPE1 memory power control IPC task
    if((G_sysConfigData.ips_mem_pwr_ctl     != MEM_PWR_CTL_NO_SUPPORT ) &&
       (G_sysConfigData.default_mem_pwr_ctl != MEM_PWR_CTL_NO_SUPPORT ))
    {
        gpe_init_mem_power_control();
    }

} // end memory_init()

