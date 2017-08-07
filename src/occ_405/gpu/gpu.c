/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/gpu/gpu.c $                                       */
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

//#define GPU_DEBUG
#ifdef GPU_DEBUG
  #define GPU_DBG(frmt,args...)   DBG_PRINT(frmt,##args)
#else
  #define GPU_DBG(frmt,args...)
#endif

#include <ssx.h>
#include <occhw_async.h>

#include <trac_interface.h>
#include <trac.h>
#include <occ_common.h>
#include <comp_ids.h>
#include <occ_service_codes.h>
#include <state.h>
#include <occ_sys_config.h>
#include "sensor.h"
#include "amec_sys.h"
#include "lock.h"
#include "common.h"
#include "amec_health.h"
#include "gpu.h"
#include "gpu_structs.h"
#include "gpu_service_codes.h"

#define GPU_TEMP_READ_1S  ( 1000000 / (MICS_PER_TICK * 2) )  // Number calls with assumption called every other tick

// Time in seconds to ignore errors from the start of GPU SM
// Right now this time must include PRST and GPU init time
// this may be reduced after adding in OS interlock for PRST
#define GPU_COMM_ESTAB_TIMEOUT_SECONDS 600

#define MAX_CONSECUTIVE_GPU_RESETS      3
#define MAX_GPU_RESET_STATE_RETRY 3
#define MAX_RESET_STATE_NOT_DONE_COUNT 100
#define MAX_GPU_READ_ATTEMPT    3
#define GPU_I2C_ENGINE      PIB_I2C_ENGINE_C

extern data_cnfg_t * G_data_cnfg;

// this is the global GPU task sm state each task within the GPU SM may have its own "state"
// to allow several calls to complete the task
gpuState_e G_gpu_state = GPU_STATE_IDLE;

bool     G_gpu_monitoring_allowed = FALSE;   // Set to true if GPU is present
bool     G_gpu_i2c_reset_required = FALSE;
uint32_t G_gpu_reset_cause = 0;
uint64_t G_gpu_sm_start_time = 0;

// GPE Requests
GpeRequest G_gpu_op_request;

// GPE arguments
GPE_BUFFER(gpu_sm_args_t  G_gpu_op_req_args);

gpu_sm_args_t G_new_gpu_req_args = {{{{0}}}};

uint8_t G_current_gpu_id = 0;   // ID 0..2 of GPU currently being processed
bool    G_gpu_read_issued = false;

// Read OCC_MISC register to see if an I2C interrupt was generated for
// the specified engine.
bool check_for_i2c_interrupt(const uint8_t i_engine);

// Find first present non-failed GPU. returns 0xFF if no GPUs present/functional
uint8_t get_first_gpu(void)
{
    uint8_t first_gpu = 0xFF;  // default no GPUs present/functional
    uint8_t i = 0;

    for (i=0; i<MAX_NUM_GPU_PER_DOMAIN; i++)
    {
        if((GPU_PRESENT(i)) && (!g_amec->gpu[i].status.disabled) )
        {
           first_gpu = i;
           break;
        }
    }
    return first_gpu;
}

// Get GPU number for next present non-failed GPU from G_current_gpu_id
// returns 0xFF if there is no next GPU i.e. wrapped back to first GPU
uint8_t get_next_gpu(void)
{
    uint8_t next_gpu = G_current_gpu_id;

    if(G_current_gpu_id != 0xFF)
    {
        do
        {
           if(++next_gpu == MAX_NUM_GPU_PER_DOMAIN)
           {
              next_gpu = 0;
           }
           if( (GPU_PRESENT(next_gpu)) && (!g_amec->gpu[next_gpu].status.disabled) )
           {
              break;
           }
        }while(next_gpu != G_current_gpu_id);
    }

    if(next_gpu == get_first_gpu())
    {
        next_gpu = 0xFF;
    }

    return next_gpu;
}

// Get GPU number for a GPU that needs to be checked if driver is loaded
// returns 0xFF if no GPU needs to be checked
uint8_t gpu_id_need_driver_check(void)
{
    uint8_t gpu_id = 0xFF;  // default none needs checking
    uint8_t i = 0;

    for (i=0; i<MAX_NUM_GPU_PER_DOMAIN; i++)
    {
        if((GPU_PRESENT(i)) && (g_amec->gpu[i].status.checkDriverLoaded))
        {
           gpu_id = i;
           break;
        }
    }
    return gpu_id;
}

uint8_t gpu_id_need_memory_temp_capability_check(void)
{
    uint8_t gpu_id = 0xFF;  // default none needs checking
    uint8_t i = 0;

    for (i=0; i<MAX_NUM_GPU_PER_DOMAIN; i++)
    {
        if((GPU_PRESENT(i)) && (g_amec->gpu[i].status.checkMemTempSupport))
        {
           gpu_id = i;
           break;
        }
    }
    return gpu_id;
}

// Find first functional GPU with memory temp capability
// returns 0xFF if no functional GPU has memory temp capability
uint8_t get_first_mem_temp_capable_gpu(void)
{
    uint8_t first_gpu = 0xFF;  // default no GPU with mem temp capability
    uint8_t i = 0;

    for (i=0; i<MAX_NUM_GPU_PER_DOMAIN; i++)
    {
        if( (!g_amec->gpu[i].status.disabled) &&
            (g_amec->gpu[i].status.memTempSupported) )
        {
           first_gpu = i;
           break;
        }
    }
    return first_gpu;
}

// Get GPU number for next functional GPU from G_current_gpu_id with mem temp capability
// returns 0xFF if there is no next GPU i.e. wrapped back to first GPU with mem temp
uint8_t get_next_mem_temp_capable_gpu(void)
{
    uint8_t next_gpu = G_current_gpu_id;

    if(G_current_gpu_id != 0xFF)
    {
        do
        {
           if(++next_gpu == MAX_NUM_GPU_PER_DOMAIN)
           {
              next_gpu = 0;
           }
           if( (!g_amec->gpu[next_gpu].status.disabled) &&
               (g_amec->gpu[next_gpu].status.memTempSupported) )
           {
              break;
           }
        }while(next_gpu != G_current_gpu_id);
    }

    if(next_gpu == get_first_mem_temp_capable_gpu())
    {
        next_gpu = 0xFF;
    }

    return next_gpu;
}


// Get GPU number for a GPU that needs power limits read
// returns 0xFF if no GPU needs power limits read
uint8_t gpu_id_need_power_limits(void)
{
    uint8_t gpu_id = 0xFF;  // default none
    uint8_t i = 0;

    for (i=0; i<MAX_NUM_GPU_PER_DOMAIN; i++)
    {
        // to read power limits requires that the driver is loaded
        if( (g_amec->gpu[i].status.driverLoaded) &&
            (g_amec->gpu[i].pcap.check_pwr_limit))
        {
           gpu_id = i;
           break;
        }
    }
    return gpu_id;
}

// Get GPU number for a GPU that needs power limit set
// returns 0xFF if no GPU needs power limit set
uint8_t gpu_id_need_set_power_limit(void)
{
    uint8_t gpu_id = 0xFF;  // default none
    uint8_t i = 0;

    for (i=0; i<MAX_NUM_GPU_PER_DOMAIN; i++)
    {
        // to set power limit requires that the driver is loaded
        if( (g_amec->gpu[i].status.driverLoaded) &&
            (g_amec->gpu[i].pcap.gpu_desired_pcap_mw != g_amec->gpu[i].pcap.gpu_requested_pcap_mw) )
        {
           gpu_id = i;
           break;
        }
    }
    return gpu_id;
}

// Disable GPU monitoring for all GPUs
void disable_all_gpus(void)
{
    uint8_t i = 0;

    // release I2C lock to the host for this engine and stop monitoring
    occ_i2c_lock_release(GPU_I2C_ENGINE);
    G_gpu_monitoring_allowed = FALSE;

    // mark all GPUs as disabled
    for (i=0; i<MAX_NUM_GPU_PER_DOMAIN; i++)
    {
        g_amec->gpu[i].status.disabled = TRUE;
    }
}

// Create GPU IPC requests
void gpu_ipc_init()
{
    errlHndl_t l_err = NULL;
    int rc = 0;

    do
    {
        // Initialize IPC request for GPU operation requests
        GPU_DBG("gpu_ipc_init: Creating GPE1 IPC request for GPU op requests");
        rc = gpe_request_create(&G_gpu_op_request,
                                &G_async_gpe_queue1,
                                IPC_ST_GPU_SM_FUNCID,
                                &G_gpu_op_req_args,
                                SSX_WAIT_FOREVER,
                                NULL, // no callback/arg
                                NULL,
                                ASYNC_CALLBACK_IMMEDIATE);
        if (rc)
        {
            TRAC_ERR("gpu_ipc_init: Failed to create GPE1 IPC request for GPU op req (rc=%d)", rc);
            break;
        }
    }
    while(0);

    if (rc)
    {
        /* @
         * @errortype
         * @moduleid    GPU_MID_INIT
         * @reasoncode  SSX_GENERIC_FAILURE
         * @userdata1   return code
         * @userdata4   OCC_NO_EXTENDED_RC
         * @devdesc     Failed to create GPE1 GPU IPC request
         */
        l_err = createErrl(GPU_MID_INIT,
                           SSX_GENERIC_FAILURE,
                           OCC_NO_EXTENDED_RC,
                           ERRL_SEV_PREDICTIVE,
                           NULL, // trace buffer
                           DEFAULT_TRACE_SIZE,
                           rc,
                           0);

        REQUEST_RESET(l_err);

        // release I2C lock to the host for this engine and stop monitoring
        occ_i2c_lock_release(GPU_I2C_ENGINE);
        G_gpu_monitoring_allowed = FALSE;
    }
}

// Called after a failure for a specified GPU.  The error will
// be counted and if threshold is reached, an error will be created with
// the GPU as a callout and then set flag to force reset
void mark_gpu_failed(const gpu_sm_args_t *i_arg)
{
    uint32_t gpu_id = i_arg->gpu_id;

    // ignore all errors if haven't reached timeout for comm established
    if( (false == g_amec->gpu[gpu_id].status.readOnce) &&
        (DURATION_IN_S_UNTIL_NOW_FROM(G_gpu_sm_start_time) < GPU_COMM_ESTAB_TIMEOUT_SECONDS) )
    {
         // do nothing at this time
         return;
    }
    if((false == g_amec->gpu[gpu_id].status.disabled) &&
       (true == g_amec->gpu[gpu_id].status.readOnce))
    {
        INTR_TRAC_ERR("mark_gpu_failed: GPU%d failed in op/rc/count=0x%06X "
                      "(ffdc 0x%08X%08X)",
                      gpu_id, (i_arg->operation << 16) | (i_arg->error.rc << 8) | g_amec->gpu[gpu_id].status.errorCount,
                      WORD_HIGH(i_arg->error.ffdc), WORD_LOW(i_arg->error.ffdc));
    }

    if( ( ++g_amec->gpu[gpu_id].status.errorCount > MAX_CONSECUTIVE_GPU_RESETS) &&
        (false == g_amec->gpu[gpu_id].status.disabled) &&
        (true == g_amec->gpu[gpu_id].status.readOnce))
    {
        G_gpu_state = GPU_STATE_IDLE;
        // Disable this GPU, collect FFDC and log error
        g_amec->gpu[gpu_id].status.disabled = true;

        INTR_TRAC_ERR("mark_gpu_failed: disabling GPU%d due to %d consecutive errors (op=%d)",
                      gpu_id, g_amec->gpu[gpu_id].status.errorCount, i_arg->operation);
        errlHndl_t l_err = NULL;
        /*
         * @errortype
         * @moduleid    GPU_MID_MARK_GPU_FAILED
         * @reasoncode  GPU_FAILURE
         * @userdata1   GPE returned rc code
         * @userdata4   ERC_GPU_COMPLETE_FAILURE
         * @devdesc     GPU failure
         */
        l_err = createErrl(GPU_MID_MARK_GPU_FAILED,
                           GPU_FAILURE,
                           ERC_GPU_COMPLETE_FAILURE,
                           ERRL_SEV_PREDICTIVE,
                           NULL,
                           DEFAULT_TRACE_SIZE,
                           i_arg->error.rc,
                           0);

        addUsrDtlsToErrl(l_err,
                         (uint8_t*)&i_arg->error.ffdc,
                         sizeof(i_arg->error.ffdc),
                         ERRL_STRUCT_VERSION_1,
                         ERRL_USR_DTL_BINARY_DATA);

        // Callout the GPU if have sensor ID for it
        if(G_sysConfigData.gpu_sensor_ids[gpu_id])
        {
           addCalloutToErrl(l_err,
                            ERRL_CALLOUT_TYPE_HUID,
                            G_sysConfigData.gpu_sensor_ids[gpu_id],
                            ERRL_CALLOUT_PRIORITY_MED);
        }

        commitErrl(&l_err);
    }

    // Reset GPU
    G_gpu_i2c_reset_required = true;
    G_gpu_reset_cause = gpu_id<<24 | (i_arg->error.rc & 0xFFFF);
} // end mark_gpu_failed()

// Schedule a GPE request for GPU operation
bool schedule_gpu_req(const gpu_op_req_e i_operation, gpu_sm_args_t i_new_args)
{
    bool l_scheduled = false;
    bool scheduleRequest = true;
    errlHndl_t err = NULL;

    GPU_DBG(">>schedule_gpu_req(op 0x%02X)", i_operation);

    if (!async_request_is_idle(&G_gpu_op_request.request))
    {
        INTR_TRAC_INFO("E>schedule_gpu_req: prior request (op 0x%02X) not idle when scheduling 0x%02X (tick=%d)",
                       G_gpu_op_req_args.operation, i_operation, GPU_TICK);
    }
    else
    {
        // Ready for next request
        G_gpu_op_req_args = i_new_args;
        switch(i_operation)
        {
            // Init
            case GPU_REQ_INIT:
                break;

            // Read GPU memory temp capability
            case GPU_REQ_READ_CAPS_START:
            case GPU_REQ_READ_CAPS_STOP:
            case GPU_REQ_READ_CAPS:
                break;

            // Read GPU memory temp
            case GPU_REQ_READ_TEMP_START:
            case GPU_REQ_READ_TEMP_STOP:
            case GPU_REQ_READ_TEMP:
                break;

            // Read GPU core temp
            case GPU_REQ_READ_TEMP_SIMPLE_START:
            case GPU_REQ_READ_TEMP_SIMPLE_STOP:
            case GPU_REQ_READ_TEMP_SIMPLE:
                break;

            // I2C reset
            case GPU_REQ_RESET:
                break;

            default:
                INTR_TRAC_ERR("schedule_gpu_req: Invalid GPU request operation: 0x%02X", i_operation);
                /*
                 * @errortype
                 * @moduleid    GPU_MID_GPU_SCHED_REQ
                 * @reasoncode  GPU_FAILURE
                 * @userdata1   operation
                 * @userdata2   0
                 * @userdata4   ERC_GPU_INVALID_GPU_OPERATION
                 * @devdesc     Invalid GPU request operation
                 */
                err = createErrl(GPU_MID_GPU_SCHED_REQ,
                                 GPU_FAILURE,
                                 ERC_GPU_INVALID_GPU_OPERATION,
                                 ERRL_SEV_PREDICTIVE,
                                 NULL,
                                 DEFAULT_TRACE_SIZE,
                                 i_operation,
                                 0);

                commitErrl(&err);
                scheduleRequest = false;

                // release I2C lock to the host for this engine and stop monitoring
                occ_i2c_lock_release(GPU_I2C_ENGINE);
                G_gpu_monitoring_allowed = FALSE;
                break;
        }

        if (scheduleRequest)
        {
            // Clear errors and init common arguments for GPE
            G_gpu_op_req_args.error.error = 0;
            G_gpu_op_req_args.operation = i_operation;
            G_gpu_op_req_args.gpu_id = G_current_gpu_id;

            GPU_DBG("schedule_gpu_req: Scheduling GPE1 GPU operation 0x%02X (tick %d)", i_operation, GPU_TICK);
            int l_rc = gpe_request_schedule(&G_gpu_op_request);
            if (0 == l_rc)
            {
                l_scheduled = true;
            }
            else
            {
                INTR_TRAC_ERR("schedule_gpu_req: schedule failed w/rc=0x%08X (%d us)",
                              l_rc, (int) ((ssx_timebase_get())/(SSX_TIMEBASE_FREQUENCY_HZ/1000000)));
                /*
                 * @errortype
                 * @moduleid    GPU_MID_GPU_SCHED_REQ
                 * @reasoncode  SSX_GENERIC_FAILURE
                 * @userdata1   GPE schedule returned code
                 * @userdata2   GPU operation
                 * @userdata4   ERC_GPU_SCHEDULE_FAILURE
                 * @devdesc     Failed to schedule GPU operation request
                 */
                err = createErrl(GPU_MID_GPU_SCHED_REQ,
                                 SSX_GENERIC_FAILURE,
                                 ERC_GPU_SCHEDULE_FAILURE,
                                 ERRL_SEV_PREDICTIVE,
                                 NULL,
                                 DEFAULT_TRACE_SIZE,
                                 l_rc,
                                 i_operation);
                commitErrl(&err);

                // release I2C lock to the host for this engine and stop monitoring
                occ_i2c_lock_release(GPU_I2C_ENGINE);
                G_gpu_monitoring_allowed = FALSE;
            }
        }
    }

    return l_scheduled;

} // end schedule_gpu_req()

// Function Specification
//
// Name:  gpu_reset_sm
//
// Description: GPU Reset State Machine.  This is not called per GPU if any handling is needed
//               per GPU this function must handle and not indicate that reset is complete
//               until all present GPUs are ready
//
// End Function Specification
bool gpu_reset_sm()
{
    bool l_complete = FALSE;   // only return TRUE when the reset AND initialization is complete
    static bool L_scheduled = FALSE;  // indicates if a GPU GPE request was scheduled
    static uint8_t L_state_retry_count = 0;
    static uint8_t L_consec_reset_failure_count = 0;
    static gpuResetState_e L_reset_state = GPU_RESET_STATE_NEW;  // 1st state for a reset

    if (async_request_is_idle(&G_gpu_op_request.request))
    {
       // check if the previous state was successfully scheduled and success/done
       if( (L_reset_state != GPU_RESET_STATE_NEW) &&
           (L_reset_state != GPU_RESET_STATE_RESET_SLAVE_WAIT) &&
           (!L_scheduled || (GPE_RC_SUCCESS != G_gpu_op_req_args.error.rc)) )
       {
          // Check if failure was due to GPE image not having GPU support
          if(G_gpu_op_req_args.error.rc == GPE_RC_NO_GPU_SUPPORT)
          {
             // No GPU Support, log error and disable all GPUs
             INTR_TRAC_ERR("gpu_reset_sm: GPE image doesn't support GPUs!");

             /*
              * @errortype
              * @moduleid    GPU_MID_GPU_RESET_SM
              * @reasoncode  GPU_FAILURE
              * @userdata1   0
              * @userdata2   0
              * @userdata4   ERC_GPU_NO_GPE_SUPPORT
              * @devdesc     GPE1 image doesn't support GPU communication
              */
             errlHndl_t err = createErrl(GPU_MID_GPU_RESET_SM,
                                         GPU_FAILURE,
                                         ERC_GPU_NO_GPE_SUPPORT,
                                         ERRL_SEV_UNRECOVERABLE,
                                         NULL,
                                         DEFAULT_TRACE_SIZE,
                                         0,
                                         0);
             commitErrl(&err);

             disable_all_gpus();

             L_reset_state = GPU_RESET_STATE_NEW;
             return FALSE;  // GPUs are not ready for communication
          }
          else
          {
             // Stay in current state if haven't reached state retry count
             if(L_state_retry_count < MAX_GPU_RESET_STATE_RETRY)
             {
                // INC state retry count and retry current state
                L_state_retry_count++;
             }
             else  // this reset attempt failed
             {
                // Stop trying if reached max resets
                if(L_consec_reset_failure_count > MAX_CONSECUTIVE_GPU_RESETS)
                {
                   INTR_TRAC_ERR("gpu_reset_sm: Max Resets reached failed at state 0x%02X",
                                  L_reset_state);

                   /*
                    * @errortype
                    * @moduleid    GPU_MID_GPU_RESET_SM
                    * @reasoncode  GPU_FAILURE
                    * @userdata1   GPU reset state
                    * @userdata2   0
                    * @userdata4   ERC_GPU_RESET_FAILURE
                    * @devdesc     Failure resetting GPU interface
                    */
                   errlHndl_t err = createErrl(GPU_MID_GPU_RESET_SM,
                                               GPU_FAILURE,
                                               ERC_GPU_RESET_FAILURE,
                                               ERRL_SEV_UNRECOVERABLE,
                                               NULL,
                                               DEFAULT_TRACE_SIZE,
                                               L_reset_state,
                                               0);
                   commitErrl(&err);

                   disable_all_gpus();

                   L_reset_state = GPU_RESET_STATE_NEW;
                   return FALSE;  // GPUs are not ready for communication
                }
                else  // try the reset again from the beginning
                {
                   L_consec_reset_failure_count++;
                   L_state_retry_count = 0;
                   L_reset_state = GPU_RESET_STATE_RESET_MASTER;
                }
             }  // else reset attempt failed
          }  // else GPE supports GPU
       }// if previous state failed
       else // success on last state go to next state and process it
       {
          L_state_retry_count = 0;
          L_reset_state++;
       }

       L_scheduled = FALSE;  // default nothing scheduled

       switch (L_reset_state)
       {
           case GPU_RESET_STATE_RESET_MASTER:
               G_new_gpu_req_args.data[0] = GPU_RESET_REQ_MASTER;
               L_scheduled = schedule_gpu_req(GPU_REQ_RESET, G_new_gpu_req_args);
               break;

           case GPU_RESET_STATE_RESET_SLAVE:
               G_new_gpu_req_args.data[0] = GPU_RESET_REQ_SLV;
               L_scheduled = schedule_gpu_req(GPU_REQ_RESET, G_new_gpu_req_args);
               break;

           case GPU_RESET_STATE_RESET_SLAVE_WAIT:
               // Delay to allow reset to complete
               GPU_DBG("gpu_reset_sm: waiting during slave port 4 reset");
               break;

           case GPU_RESET_STATE_RESET_SLAVE_COMPLETE:
               G_new_gpu_req_args.data[0] = GPU_RESET_REQ_SLV_COMPLETE;
               L_scheduled = schedule_gpu_req(GPU_REQ_RESET, G_new_gpu_req_args);
               break;

           case GPU_RESET_STATE_INIT:
               // Notify GPE which GPUs are present
               G_new_gpu_req_args.data[0] = (GPU_PRESENT(ID_GPU0)) ? GPU_STATE_PRESENT : 0;
               G_new_gpu_req_args.data[1] = (GPU_PRESENT(ID_GPU1)) ? GPU_STATE_PRESENT : 0;
               G_new_gpu_req_args.data[2] = (GPU_PRESENT(ID_GPU2)) ? GPU_STATE_PRESENT : 0;
               // Setup I2C Interrupt Mask Register and Mode
               L_scheduled = schedule_gpu_req(GPU_REQ_INIT, G_new_gpu_req_args);
               break;

           case GPU_RESET_STATE_INIT_COMPLETE:
               // Reset and init is complete ready to start sending commands to the GPUs
               l_complete = TRUE;
               L_consec_reset_failure_count = 0;
               // next time this is called will be to start a new reset
               L_reset_state = GPU_RESET_STATE_NEW;
               break;

           default:
               INTR_TRAC_ERR("gpu_reset_sm: INVALID STATE: 0x%02X when reset is required", L_reset_state);
               L_reset_state = GPU_RESET_STATE_NEW;
               break;
       } // switch L_reset_state

       if(L_scheduled)
       {
          GPU_DBG("gpu_reset_sm: Scheduled reset state 0x%02X", L_reset_state);
       }
       // check if the state was expected to have a schedule. Only new and slave wait
       // don't schedule for all other states the schedule must have failed
       else if( (L_reset_state != GPU_RESET_STATE_NEW) &&
                (L_reset_state != GPU_RESET_STATE_RESET_SLAVE_WAIT) )
       {
          INTR_TRAC_ERR("gpu_reset_sm: failed to schedule state 0x%02X", L_reset_state);
       }

    } // if async_request_is_idle
    else
    {
       INTR_TRAC_ERR("gpu_reset_sm: NOT idle for state 0x%02X", L_reset_state);
    }

    return l_complete;
} // end gpu_reset_sm()

// Function Specification
//
// Name:  gpu_read_temp_sm
//
// Description: Called from gpu_task_sm to read GPU core temperature of G_current_gpu_id
//              This function should only return that complete is TRUE when the temperature
//              read is complete (or determined failed) and ready to start reading a different GPU
//
// Pre-Req:  Caller must have G_current_gpu_id set for GPU to read and
//            verified G_gpu_op_request is idle to allow scheduling
// End Function Specification
bool gpu_read_temp_sm()
{
    bool l_complete = FALSE;   // only return TRUE when the read is complete or failed
    uint16_t l_temp = 0;
    static bool L_scheduled = FALSE;  // indicates if a GPU GPE request was scheduled
    static uint8_t L_read_failure_count = 0;
    static gpuReadTempState_e L_read_temp_state = GPU_STATE_READ_TEMP_NEW;  // 1st state for reading temp

    if (async_request_is_idle(&G_gpu_op_request.request))
    {
       // If not starting a new read then need to check status of current state before moving on
       // stay in current state if the schedule failed or the state isn't finished/failed
       if( (L_read_temp_state != GPU_STATE_READ_TEMP_NEW) &&
           (!L_scheduled || (GPE_RC_SUCCESS != G_gpu_op_req_args.error.rc)) )
       {
          // If reached retry count give up on this GPU
          if(L_read_failure_count > MAX_GPU_READ_ATTEMPT)
          {
             mark_gpu_failed(&G_gpu_op_req_args);

             L_read_temp_state = GPU_STATE_READ_TEMP_NEW;
             return TRUE;  // Done with this GPU, let GPU SM move to next
          }
          else
          {
             // INC failure count and retry current state
             L_read_failure_count++;
          }
       }
       else // success on last state go to next state and process it
       {
          L_read_failure_count = 0;
          L_read_temp_state++;
       }

       L_scheduled = FALSE;  // default nothing scheduled

       switch (L_read_temp_state)
       {
           case GPU_STATE_READ_TEMP_START:
               L_scheduled = schedule_gpu_req(GPU_REQ_READ_TEMP_SIMPLE_START, G_new_gpu_req_args);
               break;

           case GPU_STATE_READ_TEMP_STOP:
               L_scheduled = schedule_gpu_req(GPU_REQ_READ_TEMP_SIMPLE_STOP, G_new_gpu_req_args);
               break;

           case GPU_STATE_READ_TEMP_READ:
               L_scheduled = schedule_gpu_req(GPU_REQ_READ_TEMP_SIMPLE, G_new_gpu_req_args);
               break;

           case GPU_STATE_READ_TEMP_COMPLETE:
               if( (!g_amec->gpu[G_current_gpu_id].status.readOnce) &&
                   (0 != G_gpu_op_req_args.data[0]) ) // TODO: check for valid temp?
               {
                   g_amec->gpu[G_current_gpu_id].status.readOnce = true;
                   TRAC_INFO("First successful attempt to read temp from GPU%d was on tick %d",
                              G_current_gpu_id, CURRENT_TICK);
                   // comm is now established update for capability checking to take place
                   g_amec->gpu[G_current_gpu_id].status.checkMemTempSupport = TRUE;
                   g_amec->gpu[G_current_gpu_id].status.checkDriverLoaded = TRUE;
               }
               // Update sensor
               l_temp = G_gpu_op_req_args.data[0] >> 24;
               sensor_update(AMECSENSOR_PTR(TEMPGPU0 + G_current_gpu_id), l_temp);

               // Clear all past errors
               g_amec->gpu[G_current_gpu_id].status.errorCount = 0;

               // check if there is an overtemp that hasn't been reported
               if((G_data_cnfg->thrm_thresh.data[DATA_FRU_GPU].error) &&
                  (l_temp > G_data_cnfg->thrm_thresh.data[DATA_FRU_GPU].error) &&
                  (!g_amec->gpu[G_current_gpu_id].status.overtempError) )
               {
                   g_amec->gpu[G_current_gpu_id].status.overtempError = TRUE;

                   INTR_TRAC_ERR("gpu_read_temp: GPU%d OT! temp[%d]",
                                  G_current_gpu_id, l_temp);

                   // Log an OT error
                   /* @
                    * @errortype
                    * @moduleid    GPU_MID_GPU_READ_TEMP
                    * @reasoncode  GPU_ERROR_TEMP
                    * @userdata1   GPU ID
                    * @userdata2   GPU memory temperature
                    * @userdata4   OCC_NO_EXTENDED_RC
                    * @devdesc     GPU memory has reached error temperature
                    *
                    */
                   errlHndl_t l_err = createErrl(GPU_MID_GPU_READ_TEMP,
                                                 GPU_ERROR_TEMP,
                                                 OCC_NO_EXTENDED_RC,
                                                 ERRL_SEV_PREDICTIVE,
                                                 NULL,
                                                 DEFAULT_TRACE_SIZE,
                                                 G_current_gpu_id,
                                                 l_temp);

                   // Callout the over temperature procedure
                   addCalloutToErrl(l_err,
                                    ERRL_CALLOUT_TYPE_COMPONENT_ID,
                                    ERRL_COMPONENT_ID_OVER_TEMPERATURE,
                                    ERRL_CALLOUT_PRIORITY_HIGH);

                   // Callout the GPU if have sensor ID for it
                   if(G_sysConfigData.gpu_sensor_ids[G_current_gpu_id])
                   {
                      addCalloutToErrl(l_err,
                                       ERRL_CALLOUT_TYPE_HUID,
                                       G_sysConfigData.gpu_sensor_ids[G_current_gpu_id],
                                       ERRL_CALLOUT_PRIORITY_MED);
                   }

                   // Commit Error
                   commitErrl(&l_err);

               } // if OT error

               // Done with this GPU ready to move to new one
               L_read_temp_state = GPU_STATE_READ_TEMP_NEW;
               l_complete = TRUE;
               break;

           default:
               INTR_TRAC_ERR("gpu_read_temp_sm: INVALID STATE: 0x%02X", L_read_temp_state);
               L_read_temp_state = GPU_STATE_READ_TEMP_NEW;
               l_complete = TRUE;
               break;
       } // switch L_read_temp_state

       if(L_scheduled)
       {
          GPU_DBG("gpu_read_temp_sm: Scheduled read temp state 0x%02X at tick %d",
                   L_read_temp_state, GPU_TICK);
       }
       else if(!l_complete)  // if not complete there must have been a failure on the schedule
       {
          INTR_TRAC_ERR("gpu_read_temp_sm: failed to schedule state 0x%02X", L_read_temp_state);
       }

    } // if async_request_is_idle
    else
    {
       INTR_TRAC_ERR("gpu_read_temp_sm: NOT idle for state 0x%02X", L_read_temp_state);
    }

    return l_complete;
} // end gpu_read_temp_sm()

// Function Specification
//
// Name:  gpu_read_mem_temp_capability_sm
//
// Description: Called from gpu_task_sm to read GPU memory temp capability of G_current_gpu_id
//              This function should only return that complete is TRUE when the capability
//              read is complete (or determined failed) and ready to start reading a different GPU
//
// Pre-Req:  Caller must have G_current_gpu_id set for GPU to read
//
// End Function Specification
bool gpu_read_mem_temp_capability_sm()
{
    bool l_complete = FALSE;   // only return TRUE when the read is complete or failed
    static bool L_scheduled = FALSE;  // indicates if a GPU GPE request was scheduled
    static uint8_t L_read_failure_count = 0;
    static gpuReadMemTempCapableState_e L_read_cap_state = GPU_STATE_READ_MEM_TEMP_CAPABLE_NEW;

    if (async_request_is_idle(&G_gpu_op_request.request))
    {
       // If not starting a new read then need to check status of current state before moving on
       // stay in current state if the schedule failed or the state isn't finished/failed
       if( (L_read_cap_state != GPU_STATE_READ_MEM_TEMP_CAPABLE_NEW) &&
           (!L_scheduled || (GPE_RC_SUCCESS != G_gpu_op_req_args.error.rc)) )
       {
          // If reached retry count give up on this read
          if(L_read_failure_count > MAX_GPU_READ_ATTEMPT)
          {
             // log error that memory temp capability couldn't be determined
             //  memory temp support will be left as not supported
             INTR_TRAC_ERR("gpu_read_mem_temp_capable: Failed to read capability for GPU%d", G_current_gpu_id);

             // Log error
             /* @
              * @errortype
              * @moduleid    GPU_MID_GPU_READ_MEM_TEMP_CAPABLE
              * @reasoncode  GPU_FAILURE
              * @userdata1   GPU ID
              * @userdata2   0
              * @userdata4   ERC_GPU_READ_MEM_TEMP_CAPABLE_FAILURE
              * @devdesc     Failure to read GPU memory temp capability
              *
              */
             errlHndl_t l_err = createErrl(GPU_MID_GPU_READ_MEM_TEMP_CAPABLE,
                                           GPU_FAILURE,
                                           ERC_GPU_READ_MEM_TEMP_CAPABLE_FAILURE,
                                           ERRL_SEV_PREDICTIVE,
                                           NULL,
                                           DEFAULT_TRACE_SIZE,
                                           G_current_gpu_id,
                                           0);

             // Callout the GPU if have sensor ID for it
             if(G_sysConfigData.gpu_sensor_ids[G_current_gpu_id])
             {
                addCalloutToErrl(l_err,
                                 ERRL_CALLOUT_TYPE_HUID,
                                 G_sysConfigData.gpu_sensor_ids[G_current_gpu_id],
                                 ERRL_CALLOUT_PRIORITY_MED);
             }

             // Commit Error
             commitErrl(&l_err);

             L_read_cap_state = GPU_STATE_READ_MEM_TEMP_CAPABLE_NEW;
             return TRUE;  // Done with this GPU, let GPU SM move to next
          }
          else
          {
             // INC failure count and retry current state
             L_read_failure_count++;
          }
       }
       else // success on last state go to next state and process it
       {
          L_read_failure_count = 0;
          L_read_cap_state++;
       }

       L_scheduled = FALSE;  // default nothing scheduled

       switch (L_read_cap_state)
       {
           case GPU_STATE_READ_MEM_TEMP_CAPABLE_START:
               L_scheduled = schedule_gpu_req(GPU_REQ_READ_CAPS_START, G_new_gpu_req_args);
               break;

           case GPU_STATE_READ_MEM_TEMP_CAPABLE_STOP:
               L_scheduled = schedule_gpu_req(GPU_REQ_READ_CAPS_STOP, G_new_gpu_req_args);
               break;

           case GPU_STATE_READ_MEM_TEMP_CAPABLE_READ:
               L_scheduled = schedule_gpu_req(GPU_REQ_READ_CAPS, G_new_gpu_req_args);
               break;

           case GPU_STATE_READ_MEM_TEMP_CAPABLE_COMPLETE:
               // Update capability
               g_amec->gpu[G_current_gpu_id].status.memTempSupported = G_gpu_op_req_args.data[0] & 0x01;

               // Done with this GPU ready to move to new one
               L_read_cap_state = GPU_STATE_READ_MEM_TEMP_CAPABLE_NEW;
               l_complete = TRUE;
               break;

           default:
               INTR_TRAC_ERR("gpu_read_mem_temp_capable: INVALID STATE: 0x%02X", L_read_cap_state);
               L_read_cap_state = GPU_STATE_READ_MEM_TEMP_CAPABLE_NEW;
               l_complete = TRUE;
               break;
       } // switch L_read_cap_state

       if(L_scheduled)
       {
          GPU_DBG("gpu_read_mem_temp_capable: Scheduled read temp capability state 0x%02X at tick %d",
                   L_read_cap_state, GPU_TICK);
       }
       else if(!l_complete)  // if not complete there must have been a failure on the schedule
       {
          INTR_TRAC_ERR("gpu_read_mem_temp_capable: failed to schedule state 0x%02X", L_read_cap_state);
       }

    } // if async_request_is_idle
    else
    {
       INTR_TRAC_ERR("gpu_read_mem_temp_capable: NOT idle for state 0x%02X", L_read_cap_state);
    }

    return l_complete;
} // end gpu_read_mem_temp_capability_sm()

// Function Specification
//
// Name:  gpu_read_memory_temp_sm
//
// Description: Called from gpu_task_sm to read GPU memory temperature of G_current_gpu_id
//              This function should only return that complete is TRUE when the temperature
//              read is complete (or determined failed) and ready to start reading a different GPU
//
// Pre-Req:  Caller must have G_current_gpu_id set for GPU to read
//
// End Function Specification
bool gpu_read_memory_temp_sm()
{
    bool l_complete = FALSE;   // only return TRUE when the read is complete or failed
    uint16_t l_temp = 0;
    static bool L_scheduled = FALSE;  // indicates if a GPU GPE request was scheduled
    static uint8_t L_read_failure_count = 0;
    static gpuReadMemTempState_e L_read_temp_state = GPU_STATE_READ_MEM_TEMP_NEW;  // 1st state for reading temp

    if (async_request_is_idle(&G_gpu_op_request.request))
    {
       // If not starting a new read then need to check status of current state before moving on
       // stay in current state if the schedule failed or the state isn't finished/failed
       if( (L_read_temp_state != GPU_STATE_READ_MEM_TEMP_NEW) &&
           (!L_scheduled || (GPE_RC_SUCCESS != G_gpu_op_req_args.error.rc)) )
       {
          // If reached retry count give up on this read
          if(L_read_failure_count > MAX_GPU_READ_ATTEMPT)
          {
             // INC memory error count and check if reached timeout threshold for new mem temp
             uint8_t max_read_timeout = G_data_cnfg->thrm_thresh.data[DATA_FRU_GPU_MEM].max_read_timeout;
             g_amec->gpu[G_current_gpu_id].status.memErrorCount++;
             if((max_read_timeout) && (max_read_timeout != 0xFF) &&
                (g_amec->gpu[G_current_gpu_id].status.memErrorCount >= max_read_timeout) )
             {
                 // Disable memory temp reading for this GPU and log error
                 g_amec->gpu[G_current_gpu_id].status.memTempSupported = FALSE;
                 // so BMC knows there is an error for fan control set sensor to 0xFF
                 sensor_update(AMECSENSOR_PTR(TEMPGPU0MEM + G_current_gpu_id), 0xFFFF);

                 INTR_TRAC_ERR("gpu_read_memory_temp: disabling memory temp for GPU%d due to %d consecutive errors",
                      G_current_gpu_id, g_amec->gpu[G_current_gpu_id].status.memErrorCount);

                 // Log error
                 /* @
                  * @errortype
                  * @moduleid    GPU_MID_GPU_READ_MEM_TEMP
                  * @reasoncode  GPU_FAILURE
                  * @userdata1   GPU ID
                  * @userdata2   number consecutive read mem temp failures
                  * @userdata4   ERC_GPU_READ_MEM_TEMP_TIMEOUT
                  * @devdesc     Timeout reading new GPU memory temperature
                  *
                  */
                 errlHndl_t l_err = createErrl(GPU_MID_GPU_READ_MEM_TEMP,
                                               GPU_FAILURE,
                                               ERC_GPU_READ_MEM_TEMP_TIMEOUT,
                                               ERRL_SEV_PREDICTIVE,
                                               NULL,
                                               DEFAULT_TRACE_SIZE,
                                               G_current_gpu_id,
                                               g_amec->gpu[G_current_gpu_id].status.memErrorCount);

                 // Callout the GPU if have sensor ID for it
                 if(G_sysConfigData.gpu_sensor_ids[G_current_gpu_id])
                 {
                    addCalloutToErrl(l_err,
                                     ERRL_CALLOUT_TYPE_HUID,
                                     G_sysConfigData.gpu_sensor_ids[G_current_gpu_id],
                                     ERRL_CALLOUT_PRIORITY_MED);
                 }

                 // Commit Error
                 commitErrl(&l_err);

             } // if timeout error

             L_read_temp_state = GPU_STATE_READ_MEM_TEMP_NEW;
             return TRUE;  // Done with this GPU, let GPU SM move to next
          }
          else
          {
             // INC failure count and retry current state
             L_read_failure_count++;
          }
       }
       else // success on last state go to next state and process it
       {
          L_read_failure_count = 0;
          L_read_temp_state++;
       }

       L_scheduled = FALSE;  // default nothing scheduled

       switch (L_read_temp_state)
       {
           case GPU_STATE_READ_MEM_TEMP_START:
               L_scheduled = schedule_gpu_req(GPU_REQ_READ_TEMP_START, G_new_gpu_req_args);
               break;

           case GPU_STATE_READ_MEM_TEMP_STOP:
               L_scheduled = schedule_gpu_req(GPU_REQ_READ_TEMP_STOP, G_new_gpu_req_args);
               break;

           case GPU_STATE_READ_MEM_TEMP_READ:
               L_scheduled = schedule_gpu_req(GPU_REQ_READ_TEMP, G_new_gpu_req_args);
               break;

           case GPU_STATE_READ_MEM_TEMP_COMPLETE:
               // Update sensor
               l_temp = G_gpu_op_req_args.data[0] >> 24;
               sensor_update(AMECSENSOR_PTR(TEMPGPU0MEM + G_current_gpu_id), l_temp);

               // Clear past errors
               g_amec->gpu[G_current_gpu_id].status.memErrorCount = 0;

               // check if there is an overtemp that hasn't been reported
               if((G_data_cnfg->thrm_thresh.data[DATA_FRU_GPU_MEM].error) &&
                  (l_temp > G_data_cnfg->thrm_thresh.data[DATA_FRU_GPU_MEM].error) &&
                  (!g_amec->gpu[G_current_gpu_id].status.memOvertempError) )
               {
                   g_amec->gpu[G_current_gpu_id].status.memOvertempError = TRUE;

                   INTR_TRAC_ERR("gpu_read_memory_temp: GPU%d memory OT! temp[%d]",
                                  G_current_gpu_id, l_temp);

                   // Log an OT error
                   /* @
                    * @errortype
                    * @moduleid    GPU_MID_GPU_READ_MEM_TEMP
                    * @reasoncode  GPU_MEMORY_ERROR_TEMP
                    * @userdata1   GPU ID
                    * @userdata2   GPU memory temperature
                    * @userdata4   OCC_NO_EXTENDED_RC
                    * @devdesc     GPU memory has reached error temperature
                    *
                    */
                   errlHndl_t l_err = createErrl(GPU_MID_GPU_READ_MEM_TEMP,
                                                 GPU_MEMORY_ERROR_TEMP,
                                                 OCC_NO_EXTENDED_RC,
                                                 ERRL_SEV_PREDICTIVE,
                                                 NULL,
                                                 DEFAULT_TRACE_SIZE,
                                                 G_current_gpu_id,
                                                 l_temp);

                   // Callout the over temperature procedure
                   addCalloutToErrl(l_err,
                                    ERRL_CALLOUT_TYPE_COMPONENT_ID,
                                    ERRL_COMPONENT_ID_OVER_TEMPERATURE,
                                    ERRL_CALLOUT_PRIORITY_HIGH);

                   // Callout the GPU if have sensor ID for it
                   if(G_sysConfigData.gpu_sensor_ids[G_current_gpu_id])
                   {
                      addCalloutToErrl(l_err,
                                       ERRL_CALLOUT_TYPE_HUID,
                                       G_sysConfigData.gpu_sensor_ids[G_current_gpu_id],
                                       ERRL_CALLOUT_PRIORITY_MED);
                   }

                   // Commit Error
                   commitErrl(&l_err);

               } // if OT error

               // Done with this GPU ready to move to new one
               L_read_temp_state = GPU_STATE_READ_MEM_TEMP_NEW;
               l_complete = TRUE;
               break;

           default:
               INTR_TRAC_ERR("gpu_read_memory_temp_sm: INVALID STATE: 0x%02X", L_read_temp_state);
               L_read_temp_state = GPU_STATE_READ_MEM_TEMP_NEW;
               l_complete = TRUE;
               break;
       } // switch L_read_temp_state

       if(L_scheduled)
       {
          GPU_DBG("gpu_read_memory_temp_sm: Scheduled read temp state 0x%02X at tick %d",
                   L_read_temp_state, GPU_TICK);
       }
       else if(!l_complete)  // if not complete there must have been a failure on the schedule
       {
          INTR_TRAC_ERR("gpu_read_memory_temp_sm: failed to schedule state 0x%02X", L_read_temp_state);
       }

    } // if async_request_is_idle
    else
    {
       INTR_TRAC_ERR("gpu_read_memory_temp_sm: NOT idle for state 0x%02X", L_read_temp_state);
    }

    return l_complete;
} // end gpu_read_memory_temp_sm()


// Function Specification
//
// Name:  gpu_sm_handle_idle_state
//
// Description: Called when GPU SM is idle to determine what state (if any) should
//               be done next
// End Function Specification
bool gpu_sm_handle_idle_state(bool i_read_temp_start_needed, bool i_mem_temp_needed)
{
    bool l_new_state = FALSE;  // return TRUE if there is a new state for GPU communication
    uint8_t l_gpu_id = 0;

    do
    {
        // Check for next state in order of priority

        // 1.  Need to set a power limit on a GPU?
        l_gpu_id = gpu_id_need_set_power_limit();
        if(l_gpu_id != 0xFF)
        {
            // Found a GPU that needs a power limit set
            G_current_gpu_id = l_gpu_id;
            G_gpu_state = GPU_STATE_SET_PWR_LIMIT;
            l_new_state = TRUE;
            break;
        }

        // 2.  check if Host needs lock
        if (!check_and_update_i2c_lock(GPU_I2C_ENGINE))
        {
            // We don't own the lock anymore
            // can't do anything until we get ownership back
            G_gpu_state = GPU_STATE_NO_LOCK;
            l_new_state = FALSE;
            break;
        }

        // 3.  Need to check if driver is loaded?
        l_gpu_id = gpu_id_need_driver_check();
        if(l_gpu_id != 0xFF)
        {
            // Found a GPU that needs driver checked
            G_current_gpu_id = l_gpu_id;
            G_gpu_state = GPU_STATE_CHECK_DRIVER_LOADED;
            l_new_state = TRUE;
            break;
        }

        // 4.  Need to read power limits?
        l_gpu_id = gpu_id_need_power_limits();
        if(l_gpu_id != 0xFF)
        {
            // Found a GPU that needs power limits read
            G_current_gpu_id = l_gpu_id;
            G_gpu_state = GPU_STATE_READ_PWR_LIMIT;
            l_new_state = TRUE;
            break;
        }

        // 5.  Need to read memory temps?
        if(i_mem_temp_needed)
        {
            // first check if there is a GPU that needs memory temp capability checked
            l_gpu_id = gpu_id_need_memory_temp_capability_check();
            if(l_gpu_id != 0xFF)
            {
                // Determine memory temp capability for this GPU
                G_current_gpu_id = l_gpu_id;
                G_gpu_state = GPU_STATE_CHECK_MEM_TEMP_CAPABLE;
                l_new_state = TRUE;
                break;
            }
            else
            {
               // memory temp capability checking is done start reading memory temp from capable GPUs
               l_gpu_id = get_first_mem_temp_capable_gpu();
               if(l_gpu_id != 0xFF)
               {
                  // Read memory temp for this GPU
                  G_current_gpu_id = l_gpu_id;
                  G_gpu_state = GPU_STATE_READ_MEMORY_TEMP;
                  l_new_state = TRUE;
                  break;
               }
            }
        }

        // 6.  Time to start new temperature reads?
        if(i_read_temp_start_needed)
        {
            // Start reading core temp from first present and functional GPU
            l_gpu_id = get_first_gpu();
            if(l_gpu_id != 0xFF)
            {
               // Read core temp for this GPU
               G_current_gpu_id = l_gpu_id;
               G_gpu_state = GPU_STATE_READ_TEMP;
               l_new_state = TRUE;
               break;
            }
            else  // no functional GPUs
            {
               // release I2C lock to the host for this engine and stop monitoring
               occ_i2c_lock_release(GPU_I2C_ENGINE);
               G_gpu_state = GPU_STATE_NO_LOCK;
               G_gpu_monitoring_allowed = FALSE;
               l_new_state = FALSE;  // No new state for GPU communication
               break;
            }
        }

        // Else nothing stay idle
    }while(0);

    return l_new_state;
}

// Function Specification
//
// Name:  task_gpu_sm
//
// Description: GPU State Machine - Called from tick table to manage GPUs
//
// Task Flags: RTL_FLAG_ACTIVE
//
// End Function Specification
void task_gpu_sm(struct task *i_self)
{
    bool l_start_next_state = FALSE;
    bool l_next_state = FALSE;
    uint8_t l_gpu_id = 0;

    static bool L_occ_owns_lock = FALSE;
    static bool L_gpu_first_run = TRUE;
    static uint16_t L_numCallsForTempRead = 0;  // # of calls since last temp read was started
    static bool L_read_temp_start_needed = FALSE;  // set to true when it is time to start reading GPU temps
    static bool L_mem_temp_needed = FALSE;  // set to true after rading GPU core temp to read GPU memory temp

    // GPU monitoring is enabled if GPUs are present and will be disabled if no GPUs
    // are functional or GPU I2C interface is broken
    if(G_gpu_monitoring_allowed)
    {
        // Initialize the IPC commands if this is our first run
        if(L_gpu_first_run)
        {
            gpu_ipc_init();
            G_gpu_sm_start_time = ssx_timebase_get();  // used for timeout establishing comm
            L_gpu_first_run = FALSE;
        }

        // Check if time to start reading temperatures
        // GPU tempertures (core and memory) are only used for fan control which happens every 1s
        // so there is no need to read the GPU temperatures any faster than every 1s
        if(!L_read_temp_start_needed)
        {
            L_numCallsForTempRead++;
            if(L_numCallsForTempRead >= GPU_TEMP_READ_1S)
            {
                 L_read_temp_start_needed = TRUE;
            }
        }

        // make sure OCC owns the lock in order to send commands to the GPU
        if( (L_occ_owns_lock == FALSE) || (G_gpu_state == GPU_STATE_NO_LOCK) )
        {
            // Check if host gave up the I2C lock
            L_occ_owns_lock = check_and_update_i2c_lock(GPU_I2C_ENGINE);
            if (L_occ_owns_lock)
            {
                // We now own the lock start with reset and init state
                G_gpu_state = GPU_STATE_RESET;
            }
            else
            {
                // Don't own the lock can't do anything this time
                G_gpu_state = GPU_STATE_NO_LOCK;
            }
        }

        // Process GPE response for what was scheduled on the last call
        // and if that state finished schedule GPE job to start next state
        // This means that this state machine can be ran twice
        do
        {
           if(l_start_next_state)
           {
               // This is start of 2nd time processing state set next so we don't go thru here a 3rd time
               l_next_state = TRUE;
           }

           // make sure previous action didn't disable GPU monitoring
           if(!G_gpu_monitoring_allowed)
           {
              // release I2C lock to the host for this engine and stop monitoring
              occ_i2c_lock_release(GPU_I2C_ENGINE);
              L_occ_owns_lock = FALSE;
              G_gpu_state = GPU_STATE_NO_LOCK;
           }

           switch(G_gpu_state)
           {
               case GPU_STATE_RESET:
                   // Call the GPU Reset SM
                   if (gpu_reset_sm())
                   {
                      // Reset complete and GPUs are ready for communication
                      // Start first with reading core temp of first functional GPU
                      L_numCallsForTempRead = 0;  // to track start of next temp reading in 1s
                      L_read_temp_start_needed = FALSE;  // start is no longer needed
                      l_gpu_id = get_first_gpu();
                      if(l_gpu_id != 0xFF)
                      {
                          // Read core temp for this GPU
                          G_current_gpu_id = l_gpu_id;
                          G_gpu_state = GPU_STATE_READ_TEMP;
                          l_start_next_state = TRUE;
                      }
                      else  // no functional GPUs
                      {
                         // release I2C lock to the host for this engine and stop monitoring
                         occ_i2c_lock_release(GPU_I2C_ENGINE);
                         L_occ_owns_lock = FALSE;
                         G_gpu_state = GPU_STATE_NO_LOCK;
                         G_gpu_monitoring_allowed = FALSE;
                         l_start_next_state = FALSE;
                      }
                   }

                   break;

               case GPU_STATE_READ_TEMP:
                   // Call the read core GPU temperature SM for the current GPU being processed
                   if(gpu_read_temp_sm())
                   {
                      // Temp read complete for this GPU, move to next GPU
                      // or memory temps if all GPU core temps were read
                      l_gpu_id = get_next_gpu();
                      if(l_gpu_id == 0xFF)
                      {
                         // Done reading core temps, now read GPU memory temps
                         // set state to IDLE first to check if a higher priority
                         // action is needed before starting to read memory temps
                         L_mem_temp_needed = TRUE;
                         G_gpu_state = GPU_STATE_IDLE;
                      }
                      else
                      {
                         // Stay in temperature read state and read temp for next GPU
                         G_current_gpu_id = l_gpu_id;
                      }

                      l_start_next_state = TRUE;
                   }

                   break;

               case GPU_STATE_READ_MEMORY_TEMP:
                   // Call the read GPU memory temperature SM for the current GPU being processed
                   if(gpu_read_memory_temp_sm())
                   {
                      // Temp read complete for this GPU, move to next GPU
                      // or idle if all GPU memory temps were read
                      l_gpu_id = get_next_mem_temp_capable_gpu();
                      if(l_gpu_id == 0xFF)
                      {
                         // Done reading memory temps
                         G_gpu_state = GPU_STATE_IDLE;
                      }
                      else
                      {
                         // Stay in memory read state and read memory temp for next GPU
                         G_current_gpu_id = l_gpu_id;
                      }

                      l_start_next_state = TRUE;
                   }

                   break;

               case GPU_STATE_CHECK_MEM_TEMP_CAPABLE:
                   // Check if current GPU has memory temperature capability
                   if(gpu_read_mem_temp_capability_sm())
                   {
                      // Capability check complete for this GPU, go to IDLE state
                      // to let IDLE SM decide what to do next
                      g_amec->gpu[G_current_gpu_id].status.checkMemTempSupport = FALSE;
                      G_gpu_state = GPU_STATE_IDLE;
                      l_start_next_state = TRUE;
                   }
                   break;

               case GPU_STATE_CHECK_DRIVER_LOADED:
                   // Check if driver is loaded for current GPU
                   if(1) // TODO
                   {
                      // Driver check complete for this GPU, go to IDLE state
                      // to let IDLE SM decide what to do next
                      g_amec->gpu[G_current_gpu_id].status.checkDriverLoaded = FALSE;
                      g_amec->gpu[G_current_gpu_id].status.driverLoaded = FALSE;
                      G_gpu_state = GPU_STATE_IDLE;
                      l_start_next_state = TRUE;
                   }
                   break;

               case GPU_STATE_READ_PWR_LIMIT:
                   // Read power limits for current GPU
                   if(1) // TODO
                   {
                      // Read power limits complete for this GPU, go to IDLE state
                      // to let IDLE SM decide what to do next
                      g_amec->gpu[G_current_gpu_id].pcap.check_pwr_limit = FALSE;
                      G_gpu_state = GPU_STATE_IDLE;
                      l_start_next_state = TRUE;
                   }
                   break;

               case GPU_STATE_SET_PWR_LIMIT:
                   // Set power limit on current GPU
                   if(1) // TODO
                   {
                      // Set power limit complete for this GPU, go to IDLE state
                      // to let IDLE SM decide what to do next
                      G_gpu_state = GPU_STATE_IDLE;
                      l_start_next_state = TRUE;
                   }
                   break;

               case GPU_STATE_NO_LOCK:
                   // Host owns the I2C engine.  Need to wait until we get the lock
                   l_start_next_state = FALSE;
                   break;

               default:
                   // Nothing happened on last call
                   G_gpu_state = GPU_STATE_IDLE;
                   break;
           } // switch G_gpu_state

           // check if the previous action requires a reset
           if(G_gpu_i2c_reset_required)
           {
               G_gpu_i2c_reset_required = FALSE;
               G_gpu_state = GPU_STATE_RESET;
               l_start_next_state = TRUE;
               break;
           }
           else if(G_gpu_state == GPU_STATE_IDLE)
           {
               // time to decide what to do next
               l_start_next_state = gpu_sm_handle_idle_state(L_read_temp_start_needed, L_mem_temp_needed);
               if(l_start_next_state)
               {
                  if(G_gpu_state == GPU_STATE_READ_TEMP)
                  {
                     // new state to read core temp reset temperature reading timer
                     L_numCallsForTempRead = 0;
                     L_read_temp_start_needed = FALSE; // start no longer needed
                  }
                  else if(G_gpu_state == GPU_STATE_READ_MEMORY_TEMP)
                  {
                     // new state to start reading memory temps, reset mem temp needed
                     L_mem_temp_needed = FALSE;
                  }
               }
           }
        }while((l_start_next_state) && (!l_next_state));
    } // GPU monitoring enabled
} // end task_gpu_sm()
