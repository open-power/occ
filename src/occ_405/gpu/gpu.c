/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/gpu/gpu.c $                                       */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2021                        */
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
#include "i2c.h"

// Number calls with assumption the GPU SM task is called every other tick
#define GPU_TEMP_READ_1S  ( 1000000 / (MICS_PER_TICK * 2) )
#define GPU_TIMEOUT ( 10000000 / (MICS_PER_TICK *2) )

#define GPU_TICKS_TO_100MS ( 100000 / (MICS_PER_TICK * 2) )
#define GPU_TICKS_TO_1S ( 1000000 / (MICS_PER_TICK * 2) )

// Number of consecutive failures to ignore after GPU is taken out of reset to give GPU init time
#define GPU_INIT_ERROR_COUNT 300  // approximately 300 seconds

#define MAX_CONSECUTIVE_GPU_RESETS      5
#define MAX_GPU_RESET_STATE_RETRY 3
#define MAX_RESET_STATE_NOT_DONE_COUNT 100
#define MAX_GPU_READ_ATTEMPT    3
#define GPU_ERRORS_BEFORE_I2C_RESET 5

// consecutive error counts for GPU command failures before error is logged if GPU is not in reset
#define GPU_CHECK_DRIVER_ERROR_COUNT 5
#define GPU_READ_MEM_CAP_ERROR_COUNT 5
#define GPU_READ_PWR_LIMIT_ERROR_COUNT 5
#define GPU_SET_PWR_LIMIT_ERROR_COUNT 5

#define GPU_I2C_ENGINE      PIB_I2C_ENGINE_C

extern data_cnfg_t * G_data_cnfg;
extern PWR_READING_TYPE  G_pwr_reading_type;

// this is the global GPU task sm state each task within the GPU SM may have its own "state"
// to allow several calls to complete the task
gpuState_e G_gpu_state = GPU_STATE_IDLE;

bool     G_gpu_monitoring_allowed = FALSE;   // Set to true if GPU is present
bool     G_gpu_i2c_reset_required = FALSE;
uint32_t G_gpu_reset_cause = 0;

// GPE Requests
GpeRequest G_gpu_op_request;
GpeRequest G_gpu_init_request;

// GPE arguments
GPE_BUFFER(gpu_sm_args_t  G_gpu_op_req_args);
GPE_BUFFER(gpu_init_args_t G_gpu_init_args);

gpu_sm_args_t G_new_gpu_req_args = {{{{0}}}};

uint8_t G_current_gpu_id = 0;   // ID 0..2 of GPU currently being processed

gpuTimingTable_t G_gpu_tick_times;

void update_gpu_tick_sensor(gpuTimingSensor_t *sensor, uint32_t ticks)
{
    if(ticks > sensor->max)
    {
        sensor->max = ticks;
    }

    if(ticks > GPU_TICKS_TO_1S)
    {
        sensor->count_1s++;
    }
    else if( (ticks > GPU_TICKS_TO_100MS) )
    {
        sensor->count_100ms++;
    }
    else
    {
        sensor->count_lt100ms++;
    }

    sensor->count++;
    sensor->accum += ticks;
    sensor->avg = sensor->accum / sensor->count;
}

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

    // checking for driver loaded can be repeated until driver is loaded which may never happen
    // to avoid infinite loop checking the same GPU over and over we will use a static and each call
    // will start looking at the next GPU, after all GPUs checked will allow none before re-checking all GPUs
    static uint8_t L_current_gpu_id = 0;

    if(L_current_gpu_id == 0xFF)
    {
       // checked all GPUs once, do not check any this time and start over with GPU 0 on next call
       L_current_gpu_id = 0;
    }
    else
    {
       for (i=L_current_gpu_id; i<MAX_NUM_GPU_PER_DOMAIN; i++)
       {
           // only check for driver after i2c comm (readOnce) has been established
           if((GPU_PRESENT(i)) && (!g_amec->gpu[i].status.disabled) &&
              (g_amec->gpu[i].status.readOnce) && (g_amec->gpu[i].status.checkDriverLoaded))
           {
              gpu_id = i;
              break;
           }
       }

       //  setup L_current_gpu_id for next call based on what is happening this time
       if(gpu_id == 0xFF)
       {
          // no GPU needs checking start back at 0 next time
          L_current_gpu_id = 0;
       }
       else if(gpu_id == (MAX_NUM_GPU_PER_DOMAIN - 1) )
       {
          // last GPU is having driver checked do not check any next time
          L_current_gpu_id = 0xFF;
       }
       else
       {
          // next time look at the next GPU ID first
          L_current_gpu_id = gpu_id + 1;
       }
    }

    return gpu_id;
}

uint8_t gpu_id_need_memory_temp_capability_check(void)
{
    uint8_t gpu_id = 0xFF;  // default none needs checking
    uint8_t i = 0;

    // checking for memory temp capability will be repeated until memory temp is capable which may never happen
    // to avoid infinite loop checking the same GPU over and over we will use a static and each call
    // will start looking at the next GPU, after all GPUs checked will allow none before re-checking all GPUs
    static uint8_t L_current_gpu_id = 0;

    if(L_current_gpu_id == 0xFF)
    {
       // checked all GPUs once, do not check any this time and start over with GPU 0 on next call
       L_current_gpu_id = 0;
    }
    else
    {
       for (i=L_current_gpu_id; i<MAX_NUM_GPU_PER_DOMAIN; i++)
       {
           // driver must be loaded for memory temp capability
           if( (!g_amec->gpu[i].status.disabled) && (g_amec->gpu[i].status.driverLoaded) &&
               (g_amec->gpu[i].status.checkMemTempSupport) )
           {
              gpu_id = i;
              break;
           }
       }

       //  setup L_current_gpu_id for next call based on what is happening this time
       if(gpu_id == 0xFF)
       {
          // no GPU needs checking start back at 0 next time
          L_current_gpu_id = 0;
       }
       else if(gpu_id == (MAX_NUM_GPU_PER_DOMAIN - 1) )
       {
          // last GPU is having memory capability checked do not check any next time
          L_current_gpu_id = 0xFF;
       }
       else
       {
          // next time look at the next GPU ID first
          L_current_gpu_id = gpu_id + 1;
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
            (g_amec->gpu[i].status.memTempSupported) )  // memTempSupported implies that driver is loaded
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
               (g_amec->gpu[next_gpu].status.memTempSupported) )  // memTempSupported implies that driver is loaded
           {
              break;
           }
        }while(next_gpu != G_current_gpu_id);
    }

    if(next_gpu == get_first_mem_temp_capable_gpu())
    {
        next_gpu = 0xFF;
    }
    else if( (next_gpu != 0xFF) && (!g_amec->gpu[next_gpu].status.memTempSupported) )
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

    static uint8_t L_current_gpu_id = 0;

    if(0xFF == L_current_gpu_id)
    {
        // We attempted to read power limits for all GPUs
        // do not check any this time and start over with GPU 0 on next call
        L_current_gpu_id = 0;
    }
    else
    {
        for (i=L_current_gpu_id; i<MAX_NUM_GPU_PER_DOMAIN; i++)
        {
            // to read power limits requires that the driver is loaded
            if( (g_amec->gpu[i].status.driverLoaded) &&
                (g_amec->gpu[i].pcap.check_pwr_limit))
            {
                // If there is no power capping support skip reading power limits
                if(G_pwr_reading_type == PWR_READING_TYPE_NONE)
                {
                    g_amec->gpu[i].pcap.check_pwr_limit = false;
                }
                else
                {
                    gpu_id = i;
                    break;
                }
            }
        }

        if(0xFF == gpu_id)
        {
            // We don't need to read power limits from any GPUs at the moment
            L_current_gpu_id = 0;
        }
        else if( (MAX_NUM_GPU_PER_DOMAIN - 1) == gpu_id)
        {
            // We're reading from last GPU, do not check any next time
            L_current_gpu_id = 0xFF;
        }
        else
        {
            // Next time look at next GPU ID first
            L_current_gpu_id = gpu_id + 1;
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

    static uint8_t L_current_gpu_id = 0;

    if(0xFF == L_current_gpu_id)
    {
        // We've checked to see if all GPUs need a power cap set, start over
        // with GPU 0 next time
        L_current_gpu_id = 0;
    }
    else
    {
        for (i=L_current_gpu_id; i<MAX_NUM_GPU_PER_DOMAIN; i++)
        {
            // to set power limit requires that the driver is loaded and power limits were read
            if( (g_amec->gpu[i].status.driverLoaded) && (g_amec->gpu[i].pcap.pwr_limits_read) &&
                (!g_amec->gpu[i].pcap.set_failed) && (g_amec->gpu[i].pcap.gpu_desired_pcap_mw != 0) &&
                (g_amec->gpu[i].pcap.gpu_desired_pcap_mw != g_amec->gpu[i].pcap.gpu_requested_pcap_mw) )
            {
               gpu_id = i;
               break;
            }
        }

        if(0xFF == gpu_id)
        {
            // no GPU needs checking start back at 0 next time
            L_current_gpu_id = 0;
        }
        else if( (MAX_NUM_GPU_PER_DOMAIN - 1) == gpu_id )
        {
            // Last GPU is being set do not check any next time
            L_current_gpu_id = 0xFF;
        }
        else
        {
            // next time look at the next GPU ID first
            L_current_gpu_id = gpu_id + 1;
        }
    }

    return gpu_id;
}

// For the given GPU clear status/data that requires GPU driver to be loaded
void clear_gpu_driver_status(uint8_t i_gpu_num)
{
    g_amec->gpu[i_gpu_num].status.checkDriverLoaded = false;
    g_amec->gpu[i_gpu_num].status.driverLoaded = false;

    // Reading memory temperature requires driver to be loaded.
    g_amec->gpu[i_gpu_num].status.checkMemTempSupport = false;
    g_amec->gpu[i_gpu_num].status.memTempSupported = false;
    g_amec->gpu[i_gpu_num].status.memErrorCount = 0;

    // Power capping requires driver to be loaded.  Clear GPU power limits
    g_amec->gpu[i_gpu_num].pcap.check_pwr_limit = false;
    g_amec->gpu[i_gpu_num].pcap.pwr_limits_read = false;
    g_amec->gpu[i_gpu_num].pcap.set_failed = false;
    g_amec->gpu[i_gpu_num].pcap.gpu_min_pcap_mw = 0;
    g_amec->gpu[i_gpu_num].pcap.gpu_max_pcap_mw = 0;
    g_amec->gpu[i_gpu_num].pcap.gpu_requested_pcap_mw = 0;
    g_amec->gpu[i_gpu_num].pcap.gpu_default_pcap_mw = 0;
    //amec will need to recalculate after power limits are read to handle any clipping with new GPU min/max
    g_amec->gpu[i_gpu_num].pcap.gpu_desired_pcap_mw = 0;
}

// Handles GPU not able to process request due to driver load or un-load
void handle_driver_change(void)
{
    // Clear out driver status while driver change completes and is determined if loaded/un-loaded
    clear_gpu_driver_status(G_current_gpu_id);

    // memory temp only available when driver is loaded. clear error and set not available
    g_amec->gpu[G_current_gpu_id].status.memTempFailure = false;
    g_amec->gpu[G_current_gpu_id].status.memTempNotAvailable = true;

    // when driver change is complete we must re-query to see if driver is loaded or not
    g_amec->gpu[G_current_gpu_id].status.checkDriverLoaded = true;
}

// For all GPUs read GPU reset status and take action if reset status has changed
void update_gpu_reset_status(void)
{
    uint8_t gpu_num = 0;

    // GPU reset status is in the OCC FLAGS register and is updated by OPAL
    // Read the current reset status for all GPUs.  A reset status of '1' indicates NOT in reset
    ocb_occflg_t occ_flags = {0};
    occ_flags.value = in32(OCB_OCCFLG0);
    bool not_in_reset[3] = {occ_flags.fields.gpu0_reset_status,
                            occ_flags.fields.gpu1_reset_status,
                            occ_flags.fields.gpu2_reset_status};

    // reset status of '0' (IN reset) is the default
    // the OCC will still try to read GPU when IN reset but will not log errors
    // this is so we still communicate with the GPUs without OPAL support to indicate
    // a GPU is not in reset.
    // Full OCC support below for when OPAL starts updating the reset status
    for (gpu_num=0; gpu_num<MAX_NUM_GPU_PER_DOMAIN; gpu_num++)
    {
        if(not_in_reset[gpu_num] != g_amec->gpu[gpu_num].status.notReset)
        {
            INTR_TRAC_IMP("update_gpu_reset_status: GPU%d NOT in reset is now = %d",
                           gpu_num,
                           not_in_reset[gpu_num]);

            // There has been a change to the reset status clear everything out except for errors logged so we don't log again
            clear_gpu_driver_status(gpu_num);
            g_amec->gpu[gpu_num].status.errorCount = 0;
            g_amec->gpu[gpu_num].status.retryCount = 0;

            // readOnce of false will force comm to be established and once established then checkDriverLoaded will get set
            g_amec->gpu[gpu_num].status.readOnce = false;

            if(not_in_reset[gpu_num])
            {
                // GPU was taken out of reset clear disabled to allow communication again
                g_amec->gpu[gpu_num].status.disabled = false;
            }
            else
            {
                // GPU was put in reset.  Clear temperature sensor errors and set to not available
                g_amec->gpu[gpu_num].status.coreTempFailure = false;
                g_amec->gpu[gpu_num].status.coreTempNotAvailable = true;
                g_amec->gpu[gpu_num].status.memTempFailure = false;
                g_amec->gpu[gpu_num].status.memTempNotAvailable = true;
            }

            g_amec->gpu[gpu_num].status.notReset = not_in_reset[gpu_num];

        }  // if GPU reset status changed
    }  // for each GPU
}  // end update_gpu_reset_status()

// Disable GPU monitoring for all GPUs
void disable_all_gpus(void)
{
    uint8_t i = 0;

    G_gpu_monitoring_allowed = FALSE;

    // mark all GPUs as disabled
    for (i=0; i<MAX_NUM_GPU_PER_DOMAIN; i++)
    {
        g_amec->gpu[i].status.disabled = TRUE;
    }
}


// schedule request to init gpu info on gpe1
void schedule_gpe_gpu_init_req()
{
    errlHndl_t err = NULL;
    int rc = 0;

    memset(&G_gpu_init_args, 0, sizeof(G_gpu_init_args));
    // Need to add gpu i2c info
    // G_gpu_init_args.gpu_i2c


    rc = gpe_request_schedule(&G_gpu_init_request);
    if (rc)
    {
        INTR_TRAC_ERR
            ("schedule_gpe_gpu_init_req: gpe gpu init schedule failed w/rc=0x%08X", rc);
        /*
         * @errortype
         * @moduleid    GPU_MID_GPE_GPU_INIT_SCHED_REQ
         * @reasoncode  SSX_GENERIC_FAILURE
         * @userdata1   GPE schedule returned code
         * @userdata4   ERC_GPU_SCHEDULE_FAILURE
         * @devdesc     Failed to schedule GPE GPU initial request
         */
        err = createErrl(GPU_MID_GPE_GPU_INIT_SCHED_REQ,
                         SSX_GENERIC_FAILURE,
                         ERC_GPU_SCHEDULE_FAILURE,
                         ERRL_SEV_PREDICTIVE,
                         NULL,
                         DEFAULT_TRACE_SIZE,
                         rc,
                         0);
        commitErrl(&err);

        // release I2C lock to the host for this engine and stop monitoring
        occ_i2c_lock_release(GPU_I2C_ENGINE);
        G_gpu_monitoring_allowed = FALSE;
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

        // Initialize GPU support on GPE1
        GPU_DBG("gpu_ipc_init: Creating GPE1 IPC request for GPU initialization");
        rc = gpe_request_create(&G_gpu_init_request,
                                &G_async_gpe_queue1,
                                IPC_ST_GPE_GPU_INIT_FUNCID,
                                &G_gpu_init_args,
                                SSX_WAIT_FOREVER,
                                NULL, // no callback
                                NULL, // no args
                                ASYNC_CALLBACK_IMMEDIATE);
        if (rc)
        {
            TRAC_ERR("gpu_ipc_init: Failed to create GPE1 GPU init request. (rc=%d)", rc);
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
    else
    {
        // if (redundant ps policy is set)
        if (G_sysConfigData.system_type.non_redund_ps == false)
        {
            // gpe gpu init only needs to be done once, so do it here.
            schedule_gpe_gpu_init_req();
        }
    }
}

// Called after a failure reading core temp for a specified GPU.  The error will
// be counted and if threshold is reached, an error will be created with
// the GPU as a callout if the GPU is not in reset
void mark_gpu_failed(const gpu_sm_args_t *i_arg)
{
    uint32_t gpu_id = i_arg->gpu_id;

    do
    {
        if((false == g_amec->gpu[gpu_id].status.disabled) &&
           (true == g_amec->gpu[gpu_id].status.readOnce))
        {
            GPU_DBG("mark_gpu_failed: GPU%d failed in op/rc/count=0x%06X "
                    "(ffdc 0x%08X%08X)",
                    gpu_id, (i_arg->operation << 16) | (i_arg->error.rc << 8) | g_amec->gpu[gpu_id].status.errorCount,
                    WORD_HIGH(i_arg->error.ffdc), WORD_LOW(i_arg->error.ffdc));
        }

        // Always inc retry count for I2C reset regardless of if GPU is in reset or not
        g_amec->gpu[gpu_id].status.retryCount++;

        // Only inc error count if it is known that GPU is NOT in reset
        // NOTE: Default is IN reset so this will only be true when OPAL/OS supports telling the OCC reset status
        // if OS never tells the OCC reset status the OCC will never disable or log a comm error
        if(g_amec->gpu[gpu_id].status.notReset)
        {
            // INC count and check if reached error threshold
            if( ++g_amec->gpu[gpu_id].status.errorCount > GPU_INIT_ERROR_COUNT)
            {
               // set that GPU temperature readings failed
               g_amec->gpu[gpu_id].status.memTempFailure = true;
               g_amec->gpu[gpu_id].status.memTempNotAvailable = true;
               g_amec->gpu[gpu_id].status.coreTempFailure = true;
               g_amec->gpu[gpu_id].status.coreTempNotAvailable = true;

               // Disable this GPU.  GPU will get re-enabled if detected that GPU is put in reset and then taken out
               g_amec->gpu[gpu_id].status.disabled = true;

               INTR_TRAC_ERR("mark_gpu_failed: disabling GPU%d due to %d consecutive errors (op=%d)",
                             gpu_id, g_amec->gpu[gpu_id].status.errorCount, i_arg->operation);

               if(g_amec->gpu[gpu_id].status.commErrorLogged == false)
               {

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
                                      ERRL_CALLOUT_TYPE_GPU_ID,
                                      G_sysConfigData.gpu_sensor_ids[gpu_id],
                                      ERRL_CALLOUT_PRIORITY_MED);
                  }

                  commitErrl(&l_err);
                  g_amec->gpu[gpu_id].status.commErrorLogged = true;

               } // if !commErrorLogged

            } // if errorCount > threshold

        } // if notReset

    } while(0);

    // Do an I2C reset if reached retry count
    // don't want to do I2C reset every time since could be that this GPU really is in reset and
    // while resetting I2C we are unable to read other GPUs that may not be in reset
    if( g_amec->gpu[gpu_id].status.retryCount > GPU_ERRORS_BEFORE_I2C_RESET)
    {
       g_amec->gpu[gpu_id].status.retryCount = 0;
       G_gpu_i2c_reset_required = true;
       G_gpu_reset_cause = gpu_id<<24 | (i_arg->error.rc & 0xFFFF);
    }

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
            case GPU_REQ_READ_CAPS_2:
            case GPU_REQ_READ_CAPS_3:
            case GPU_REQ_READ_CAPS_FINISH:
                break;

            // Read GPU core temp
            case GPU_REQ_READ_TEMP_START:
            case GPU_REQ_READ_TEMP_FINISH:
                break;

            // Read GPU memory temp
            case GPU_REQ_READ_MEM_TEMP_START:
            case GPU_REQ_READ_MEM_TEMP_2:
            case GPU_REQ_READ_MEM_TEMP_3:
            case GPU_REQ_READ_MEM_TEMP_FINISH:
                break;

            // Check if driver is loaded
            case GPU_REQ_CHECK_DRIVER_START:
            case GPU_REQ_CHECK_DRIVER_2:
            case GPU_REQ_CHECK_DRIVER_3:
            case GPU_REQ_CHECK_DRIVER_FINISH:
                break;

            // Read GPU Power Limit
            case GPU_REQ_GET_PWR_LIMIT_1_START:
            case GPU_REQ_GET_PWR_LIMIT_1_2:
            case GPU_REQ_GET_PWR_LIMIT_1_3:
            case GPU_REQ_GET_PWR_LIMIT_1_FINISH:
            case GPU_REQ_GET_PWR_LIMIT_2_START:
            case GPU_REQ_GET_PWR_LIMIT_2_2:
            case GPU_REQ_GET_PWR_LIMIT_2_FINISH:
            case GPU_REQ_GET_PWR_LIMIT_3_START:
            case GPU_REQ_GET_PWR_LIMIT_3_2:
            case GPU_REQ_GET_PWR_LIMIT_3_3:
            case GPU_REQ_GET_PWR_LIMIT_3_FINISH:
            case GPU_REQ_GET_PWR_LIMIT_4_START:
            case GPU_REQ_GET_PWR_LIMIT_4_2:
            case GPU_REQ_GET_PWR_LIMIT_4_3:
            case GPU_REQ_GET_PWR_LIMIT_4_FINISH:
            case GPU_REQ_GET_PWR_LIMIT_5_START:
            case GPU_REQ_GET_PWR_LIMIT_5_2:
            case GPU_REQ_GET_PWR_LIMIT_5_3:
            case GPU_REQ_GET_PWR_LIMIT_5_FINISH:
                break;

            // Set GPU Power Limit
            case GPU_REQ_SET_PWR_LIMIT_1_START:
            case GPU_REQ_SET_PWR_LIMIT_1_2:
            case GPU_REQ_SET_PWR_LIMIT_1_3:
            case GPU_REQ_SET_PWR_LIMIT_1_FINISH:
            case GPU_REQ_SET_PWR_LIMIT_2_START:
            case GPU_REQ_SET_PWR_LIMIT_2_2:
            case GPU_REQ_SET_PWR_LIMIT_2_3:
            case GPU_REQ_SET_PWR_LIMIT_2_FINISH:
            case GPU_REQ_SET_PWR_LIMIT_3_START:
            case GPU_REQ_SET_PWR_LIMIT_3_2:
            case GPU_REQ_SET_PWR_LIMIT_3_3:
            case GPU_REQ_SET_PWR_LIMIT_3_FINISH:
            case GPU_REQ_SET_PWR_LIMIT_4_START:
            case GPU_REQ_SET_PWR_LIMIT_4_2:
            case GPU_REQ_SET_PWR_LIMIT_4_FINISH:
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
            G_gpu_op_req_args.gpu_rc = 0;
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
                 * @reasoncode  GPU_NO_GPE_SUPPORT
                 * @userdata1   0
                 * @userdata2   0
                 * @userdata4   ERC_GPU_NO_GPE_SUPPORT
                 * @devdesc     GPE1 image doesn't support GPU communication
                 */
                errlHndl_t err = createErrl(GPU_MID_GPU_RESET_SM,
                                         GPU_NO_GPE_SUPPORT,
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
                        INTR_TRAC_ERR("gpu_reset_sm: Max Resets reached! state[0x%02X] rc[0x%08X] addr[0x%08X] ffdc[0x%08X%08X]",
                                      L_reset_state, G_gpu_op_req_args.error.rc, G_gpu_op_req_args.error.addr,
                                      (uint32_t) (G_gpu_op_req_args.error.ffdc >> 32), (uint32_t) G_gpu_op_req_args.error.ffdc);

                        /*
                         * @errortype
                         * @moduleid    GPU_MID_GPU_RESET_SM
                         * @reasoncode  GPU_FAILURE
                         * @userdata1   GPU reset state
                         * @userdata2   GPE failure RC
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
                                               G_gpu_op_req_args.error.rc);
                        commitErrl(&err);

                        disable_all_gpus();

                        L_reset_state = GPU_RESET_STATE_NEW;
                        return FALSE;  // GPUs are not ready for communication
                    }
                    else  // try the reset again from the beginning
                    {
                        L_consec_reset_failure_count++;
                        L_state_retry_count = 0;
                        L_reset_state = GPU_RESET_STATE_INIT_BUS;
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
            case GPU_RESET_STATE_INIT_BUS:
                // Setup I2C Interrupt Mask Register
                L_scheduled = schedule_gpu_req(GPU_REQ_INIT, G_new_gpu_req_args);
                break;

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

            case GPU_RESET_STATE_RESET_FINISH:
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
// Name:  gpu_check_driver_loaded_sm
//
// Description: Called from gpu_task_sm to check if driver is loaded for G_current_gpu_id
//              This function should only return that complete is TRUE when the check
//              is complete (or determined failed) and ready for a different GPU
//
// Pre-Req:  Caller must have G_current_gpu_id set for GPU to check
//
// End Function Specification
bool gpu_check_driver_loaded_sm()
{
    bool l_complete = FALSE;   // only return TRUE when the read is complete or failed
    bool l_new_driver_loaded = FALSE;
    static bool L_scheduled = FALSE;  // indicates if a GPU GPE request was scheduled
    static uint8_t L_check_driver_failure_count[MAX_NUM_GPU_PER_DOMAIN] = {0};
    static uint8_t L_state_failure_count = 0;
    static gpuCheckDriverLoadedState_e L_check_driver_state = GPU_STATE_CHECK_DRIVER_LOADED_NEW;
    static bool L_error_logged[MAX_NUM_GPU_PER_DOMAIN] = {FALSE};

    static uint32_t L_num_ticks = 0;

    L_num_ticks++;

    if (async_request_is_idle(&G_gpu_op_request.request))
    {
       // If not starting a new read then need to check status of current state before moving on
       // stay in current state if the schedule failed or the state isn't finished/failed
       if( (L_check_driver_state != GPU_STATE_CHECK_DRIVER_LOADED_NEW) &&
           (!L_scheduled || (GPE_RC_SUCCESS != G_gpu_op_req_args.error.rc)) )
       {
          // Check if failure was due to driver change
          if(G_gpu_op_req_args.error.rc == GPE_RC_GPU_DRIVER_CHANGE)
          {
             handle_driver_change();
             // Request can't be processed by GPU at this time so we are done with this GPU
             // setup to start new request
             L_state_failure_count = 0;
             L_check_driver_failure_count[G_current_gpu_id] = 0; // clear driver failure count since there's a driver change
             L_check_driver_state = GPU_STATE_CHECK_DRIVER_LOADED_NEW;
             return TRUE;  // Done with this GPU, let GPU SM move to next
          }

          // If reached state retry count give up on this read
          else if(L_state_failure_count > MAX_GPU_READ_ATTEMPT)
          {
             // if GPU is not in reset then INC error count and check if reached threshold
             if(g_amec->gpu[G_current_gpu_id].status.notReset)
             {
                if(++L_check_driver_failure_count[G_current_gpu_id] > GPU_CHECK_DRIVER_ERROR_COUNT)
                {
                    INTR_TRAC_ERR("gpu_check_driver_loaded: Failed to check driver loaded for GPU%d RC: 0x%02X",
                                   G_current_gpu_id,
                                   G_gpu_op_req_args.gpu_rc);

                    // give up checking driver loaded for this GPU
                    // It will be retried if detected that GPU is put in reset and then taken out
                    g_amec->gpu[G_current_gpu_id].status.checkDriverLoaded = false;
                    L_check_driver_failure_count[G_current_gpu_id] = 0;

                    // without driver loaded cannot read memory temp, mark memory temp as failed
                    g_amec->gpu[G_current_gpu_id].status.memTempFailure = true;
                    g_amec->gpu[G_current_gpu_id].status.memTempNotAvailable = true;

                    // log one time error that driver loaded couldn't be determined
                    if(!L_error_logged[G_current_gpu_id])
                    {
                       L_error_logged[G_current_gpu_id] = TRUE;

                       // Log error
                       /* @
                        * @errortype
                        * @moduleid    GPU_MID_GPU_CHECK_DRIVER_LOADED
                        * @reasoncode  GPU_FAILURE
                        * @userdata1   GPU ID
                        * @userdata2   GPU RC
                        * @userdata4   ERC_GPU_CHECK_DRIVER_LOADED_FAILURE
                        * @devdesc     Failure to check GPU driver loaded
                        *
                        */
                       errlHndl_t l_err = createErrl(GPU_MID_GPU_CHECK_DRIVER_LOADED,
                                                     GPU_FAILURE,
                                                     ERC_GPU_CHECK_DRIVER_LOADED_FAILURE,
                                                     ERRL_SEV_PREDICTIVE,
                                                     NULL,
                                                     DEFAULT_TRACE_SIZE,
                                                     G_current_gpu_id,
                                                     G_gpu_op_req_args.gpu_rc);

                       // Callout the GPU if have sensor ID for it
                       if(G_sysConfigData.gpu_sensor_ids[G_current_gpu_id])
                       {
                          addCalloutToErrl(l_err,
                                           ERRL_CALLOUT_TYPE_GPU_ID,
                                           G_sysConfigData.gpu_sensor_ids[G_current_gpu_id],
                                           ERRL_CALLOUT_PRIORITY_MED);
                       }

                       // Commit Error
                       commitErrl(&l_err);
                    } // if error not logged
                } // if reached error count
             } // if notReset

             L_check_driver_state = GPU_STATE_CHECK_DRIVER_LOADED_NEW;
             L_state_failure_count = 0;
             return TRUE;  // Done with this GPU, let GPU SM move to next
          } // if reached state retry count
          else
          {
             // INC failure count and retry current state
             L_state_failure_count++;
          }
       }
       else // success on last state go to next state and process it
       {
          L_state_failure_count = 0;
          L_check_driver_state++;
       }

       L_scheduled = FALSE;  // default nothing scheduled

       switch (L_check_driver_state)
       {
           case GPU_STATE_CHECK_DRIVER_LOADED_START:
               L_num_ticks = 1;
               L_scheduled = schedule_gpu_req(GPU_REQ_CHECK_DRIVER_START, G_new_gpu_req_args);
               break;

           case GPU_STATE_CHECK_DRIVER_LOADED_2:
               L_scheduled = schedule_gpu_req(GPU_REQ_CHECK_DRIVER_2, G_new_gpu_req_args);
               break;

           case GPU_STATE_CHECK_DRIVER_LOADED_3:
               L_scheduled = schedule_gpu_req(GPU_REQ_CHECK_DRIVER_3, G_new_gpu_req_args);
               break;

           case GPU_STATE_CHECK_DRIVER_LOADED_READ:
               L_scheduled = schedule_gpu_req(GPU_REQ_CHECK_DRIVER_FINISH, G_new_gpu_req_args);
               break;

           case GPU_STATE_CHECK_DRIVER_LOADED_COMPLETE:
                // Update GPU tick timing table
                update_gpu_tick_sensor(&G_gpu_tick_times.checkdriver[G_current_gpu_id], L_num_ticks);

               // Update driver loaded
               l_new_driver_loaded = G_gpu_op_req_args.data[0] & 0x01;
               if(l_new_driver_loaded != g_amec->gpu[G_current_gpu_id].status.driverLoaded)
               {
                  // Driver loaded status changed
                  GPU_DBG("gpu_check_driver_loaded: GPU%d driver loaded changed to %d",
                          G_current_gpu_id,
                          l_new_driver_loaded);

                  if(l_new_driver_loaded)
                  {
                     // Driver is now loaded do checking that required driver to be loaded
                     g_amec->gpu[G_current_gpu_id].pcap.check_pwr_limit = true;
                     g_amec->gpu[G_current_gpu_id].status.checkMemTempSupport = true;
                     // done checking for driver to be loaded
                     g_amec->gpu[G_current_gpu_id].status.checkDriverLoaded = false;
                  }
                  else
                  {
                     // Driver is no longer loaded
                     clear_gpu_driver_status(G_current_gpu_id);

                     // memory temp only available when driver is loaded
                     // clear error and set not available
                     g_amec->gpu[G_current_gpu_id].status.memTempFailure = false;
                     g_amec->gpu[G_current_gpu_id].status.memTempNotAvailable = true;

                     // Need to keep query for driver loaded to detect when driver is loaded
                     g_amec->gpu[G_current_gpu_id].status.checkDriverLoaded = true;
                  }

                  g_amec->gpu[G_current_gpu_id].status.driverLoaded = l_new_driver_loaded;
               }

               // Done with this GPU ready to move to new one
               L_check_driver_failure_count[G_current_gpu_id] = 0;
               L_check_driver_state = GPU_STATE_CHECK_DRIVER_LOADED_NEW;
               l_complete = TRUE;
               break;

           default:
               INTR_TRAC_ERR("gpu_check_driver_loaded: INVALID STATE: 0x%02X", L_check_driver_state);
               L_check_driver_state = GPU_STATE_CHECK_DRIVER_LOADED_NEW;
               l_complete = TRUE;
               break;
       } // switch L_check_driver_state

       if(L_scheduled)
       {
          GPU_DBG("gpu_check_driver_loaded: Scheduled check driver loaded state 0x%02X at tick %d",
                   L_check_driver_state, GPU_TICK);
       }
       else if(!l_complete)  // if not complete there must have been a failure on the schedule
       {
          INTR_TRAC_ERR("gpu_check_driver_loaded: failed to schedule state 0x%02X", L_check_driver_state);
       }

    } // if async_request_is_idle
    else
    {
       INTR_TRAC_ERR("gpu_check_driver_loaded: NOT idle for state 0x%02X", L_check_driver_state);
    }

    return l_complete;
} // end gpu_check_driver_loaded_sm()

// Function Specification
//
// Name:  gpu_read_pwr_limit_sm
//
// Description: Called from gpu_task_sm to read GPU power limits for G_current_gpu_id
//              This function should only return that complete is TRUE when the read
//              is complete (or determined failed) and ready for a different GPU
//
// Pre-Req:  Caller must have G_current_gpu_id set for GPU to read
//
// End Function Specification
bool gpu_read_pwr_limit_sm()
{
    bool l_complete = FALSE;   // only return TRUE when the read is complete or failed
    static bool L_scheduled = FALSE;  // indicates if a GPU GPE request was scheduled
    static uint8_t L_read_pwr_limit_failure_count[MAX_NUM_GPU_PER_DOMAIN] = {0};
    static uint8_t L_state_failure_count = 0;
    static gpuReadPwrLimitState_e L_read_pwr_limit_state = GPU_STATE_READ_PWR_LIMIT_NEW;
    static bool L_error_logged[MAX_NUM_GPU_PER_DOMAIN] = {FALSE};
    static uint32_t L_attempts = 0;

    static uint32_t L_last_min[MAX_NUM_GPU_PER_DOMAIN] = {0};
    static uint32_t L_last_max[MAX_NUM_GPU_PER_DOMAIN] = {0};

    static uint32_t L_num_ticks = 0;

    static bool L_retry_necessary = FALSE;

    L_num_ticks++;

    if (async_request_is_idle(&G_gpu_op_request.request))
    {
        // If not starting a new read then need to check status of current state before moving on
        // stay in current state if the schedule failed or the state isn't finished/failed
        if( (L_read_pwr_limit_state != GPU_STATE_READ_PWR_LIMIT_NEW) &&
            (!L_scheduled || (GPE_RC_SUCCESS != G_gpu_op_req_args.error.rc)) )
        {
            // Repeat step 2 until it succeeds. More details on this in GPE code.
            if( (L_read_pwr_limit_state == GPU_STATE_READ_PWR_LIMIT_2_FINISH) &&
                (GPE_RC_NOT_COMPLETE == G_gpu_op_req_args.error.rc) &&
                (L_attempts <= GPU_TIMEOUT) )
            {
                L_read_pwr_limit_state = GPU_STATE_READ_PWR_LIMIT_2_START;
                L_state_failure_count = 0;
                L_attempts++;
            }
            else if( (L_read_pwr_limit_state == GPU_STATE_READ_PWR_LIMIT_1_3) &&
                     (GPE_RC_GPU_BUSY == G_gpu_op_req_args.error.rc) )
            {
                L_read_pwr_limit_state = GPU_STATE_READ_PWR_LIMIT_1_FINISH;
                L_retry_necessary = TRUE;
            }
            // Check if failure was due to driver change
            else if(G_gpu_op_req_args.error.rc == GPE_RC_GPU_DRIVER_CHANGE)
            {
                handle_driver_change();
                // Request can't be processed by GPU at this time so we are done with this GPU
                // setup to start new request
                L_state_failure_count = 0;
                L_read_pwr_limit_failure_count[G_current_gpu_id] = 0; // clear failure count since there's a driver change
                L_read_pwr_limit_state = GPU_STATE_READ_PWR_LIMIT_NEW;
                L_attempts = 0;
                L_retry_necessary = FALSE;
                return TRUE;  // Done with this GPU, let GPU SM move to next
            }

            // If reached retry count give up on this read
            else if( (L_state_failure_count > MAX_GPU_READ_ATTEMPT) ||
                     (L_attempts > GPU_TIMEOUT) )
            {
                if(L_attempts > GPU_TIMEOUT)
                {
                    // give up trying to read power limits for this GPU
                    // It will be retried if detected that GPU is put in reset and then taken out
                    g_amec->gpu[G_current_gpu_id].pcap.check_pwr_limit = false;
                    L_read_pwr_limit_failure_count[G_current_gpu_id] = 0;
                }
                // if GPU is not in reset then INC error count and check if reached threshold
                if(g_amec->gpu[G_current_gpu_id].status.notReset)
                {
                    if(++L_read_pwr_limit_failure_count[G_current_gpu_id] > GPU_READ_PWR_LIMIT_ERROR_COUNT)
                    {
                        INTR_TRAC_ERR("gpu_read_pwr_limit_sm: Failed to read power limits for GPU%d RC: 0x%02X",
                                       G_current_gpu_id,
                                       G_gpu_op_req_args.gpu_rc);

                        // give up trying to read power limits for this GPU
                        // It will be retried if detected that GPU is put in reset and then taken out
                        g_amec->gpu[G_current_gpu_id].pcap.check_pwr_limit = false;
                        L_read_pwr_limit_failure_count[G_current_gpu_id] = 0;

                        // log one time error that power limits could not be read
                        if(!L_error_logged[G_current_gpu_id])
                        {
                            L_error_logged[G_current_gpu_id] = TRUE;

                            // Log error
                            /* @
                             * @errortype
                             * @moduleid    GPU_MID_GPU_READ_PWR_LIMIT
                             * @reasoncode  GPU_FAILURE
                             * @userdata1   GPU ID
                             * @userdata2   GPU RC
                             * @userdata4   ERC_GPU_READ_PWR_LIMIT_FAILURE
                             * @devdesc     Failure to read GPU power limits
                             *
                             */
                            errlHndl_t l_err = createErrl(GPU_MID_GPU_READ_PWR_LIMIT,
                                                          GPU_FAILURE,
                                                          ERC_GPU_READ_PWR_LIMIT_FAILURE,
                                                          ERRL_SEV_PREDICTIVE,
                                                          NULL,
                                                          DEFAULT_TRACE_SIZE,
                                                          G_current_gpu_id,
                                                          G_gpu_op_req_args.gpu_rc);

                            // Callout the GPU if have sensor ID for it
                            if(G_sysConfigData.gpu_sensor_ids[G_current_gpu_id])
                            {
                                addCalloutToErrl(l_err,
                                                 ERRL_CALLOUT_TYPE_GPU_ID,
                                                 G_sysConfigData.gpu_sensor_ids[G_current_gpu_id],
                                                 ERRL_CALLOUT_PRIORITY_MED);
                            }

                            // Commit Error
                            commitErrl(&l_err);
                        } // if error not logged
                    } // if reached error count
                } // if notReset

                L_read_pwr_limit_state = GPU_STATE_READ_PWR_LIMIT_NEW;
                L_state_failure_count = 0;
                L_attempts = 0;
                return TRUE;  // Done with this GPU, let GPU SM move to next
            } // if reached retry count
            else
            {
                // INC failure count and retry current state
                L_state_failure_count++;
            }
        }
        else // success on last state go to next state and process it
        {
            L_state_failure_count = 0;
            if( (GPU_STATE_READ_PWR_LIMIT_1_FINISH == L_read_pwr_limit_state) &&
                (L_retry_necessary) )
            {
                // Let SM move on
                L_read_pwr_limit_state = GPU_STATE_READ_PWR_LIMIT_NEW;
                return TRUE;
            }
            else
            {
                L_read_pwr_limit_state++;
            }
        }

        L_scheduled = FALSE;  // default nothing scheduled

        switch (L_read_pwr_limit_state)
        {
            // Step 1
            case GPU_STATE_READ_PWR_LIMIT_1_START:
                if(!L_retry_necessary) L_num_ticks = 1;
                L_retry_necessary = FALSE;
                L_scheduled = schedule_gpu_req(GPU_REQ_GET_PWR_LIMIT_1_START, G_new_gpu_req_args);
                break;

            case GPU_STATE_READ_PWR_LIMIT_1_2:
                L_scheduled = schedule_gpu_req(GPU_REQ_GET_PWR_LIMIT_1_2, G_new_gpu_req_args);
                break;

            case GPU_STATE_READ_PWR_LIMIT_1_3:
                L_scheduled = schedule_gpu_req(GPU_REQ_GET_PWR_LIMIT_1_3, G_new_gpu_req_args);
                break;

            case GPU_STATE_READ_PWR_LIMIT_1_FINISH:
                L_attempts = 0;
                L_scheduled = schedule_gpu_req(GPU_REQ_GET_PWR_LIMIT_1_FINISH, G_new_gpu_req_args);
                break;

            // Step 2
            case GPU_STATE_READ_PWR_LIMIT_2_START:
                L_scheduled = schedule_gpu_req(GPU_REQ_GET_PWR_LIMIT_2_START, G_new_gpu_req_args);
                break;

            case GPU_STATE_READ_PWR_LIMIT_2_2:
                L_scheduled = schedule_gpu_req(GPU_REQ_GET_PWR_LIMIT_2_2, G_new_gpu_req_args);
                break;

            case GPU_STATE_READ_PWR_LIMIT_2_FINISH:
                L_scheduled = schedule_gpu_req(GPU_REQ_GET_PWR_LIMIT_2_FINISH, G_new_gpu_req_args);
                break;

            // Step 3
            case GPU_STATE_READ_PWR_LIMIT_3_START:
                GPU_DBG("gpu_read_pwr_limit_sm: took %d ticks to finish read pcap for GPU%d", L_attempts, G_current_gpu_id);
                L_scheduled = schedule_gpu_req(GPU_REQ_GET_PWR_LIMIT_3_START, G_new_gpu_req_args);
                break;

            case GPU_STATE_READ_PWR_LIMIT_3_2:
                L_scheduled = schedule_gpu_req(GPU_REQ_GET_PWR_LIMIT_3_2, G_new_gpu_req_args);
                break;

            case GPU_STATE_READ_PWR_LIMIT_3_3:
                L_scheduled = schedule_gpu_req(GPU_REQ_GET_PWR_LIMIT_3_3, G_new_gpu_req_args);
                break;

            case GPU_STATE_READ_PWR_LIMIT_3_FINISH:
                L_scheduled = schedule_gpu_req(GPU_REQ_GET_PWR_LIMIT_3_FINISH, G_new_gpu_req_args);
                break;

            // Step 4
            case GPU_STATE_READ_PWR_LIMIT_4_START:
                G_new_gpu_req_args.data[0] = G_gpu_op_req_args.data[0];
                L_scheduled = schedule_gpu_req(GPU_REQ_GET_PWR_LIMIT_4_START, G_new_gpu_req_args);
                break;

            case GPU_STATE_READ_PWR_LIMIT_4_2:
                L_scheduled = schedule_gpu_req(GPU_REQ_GET_PWR_LIMIT_4_2, G_new_gpu_req_args);
                break;

            case GPU_STATE_READ_PWR_LIMIT_4_3:
                L_scheduled = schedule_gpu_req(GPU_REQ_GET_PWR_LIMIT_4_3, G_new_gpu_req_args);
                break;

            case GPU_STATE_READ_PWR_LIMIT_4_FINISH:
                L_scheduled = schedule_gpu_req(GPU_REQ_GET_PWR_LIMIT_4_FINISH, G_new_gpu_req_args);
                break;

            // Step 5
            case GPU_STATE_READ_PWR_LIMIT_5_START:
                G_new_gpu_req_args.data[0] = G_gpu_op_req_args.data[0];
                G_new_gpu_req_args.data[1] = G_gpu_op_req_args.data[1];
                L_scheduled = schedule_gpu_req(GPU_REQ_GET_PWR_LIMIT_5_START, G_new_gpu_req_args);
                break;

            case GPU_STATE_READ_PWR_LIMIT_5_2:
                L_scheduled = schedule_gpu_req(GPU_REQ_GET_PWR_LIMIT_5_2, G_new_gpu_req_args);
                break;

            case GPU_STATE_READ_PWR_LIMIT_5_3:
                L_scheduled = schedule_gpu_req(GPU_REQ_GET_PWR_LIMIT_5_3, G_new_gpu_req_args);
                break;

            case GPU_STATE_READ_PWR_LIMIT_5_FINISH:
                L_scheduled = schedule_gpu_req(GPU_REQ_GET_PWR_LIMIT_5_FINISH, G_new_gpu_req_args);
                break;

            case GPU_STATE_READ_PWR_LIMIT_COMPLETE:
                update_gpu_tick_sensor(&G_gpu_tick_times.getpcap[G_current_gpu_id], L_num_ticks);

                g_amec->gpu[G_current_gpu_id].pcap.check_pwr_limit = FALSE;
                // Update power limits
                g_amec->gpu[G_current_gpu_id].pcap.pwr_limits_read = TRUE;
                g_amec->gpu[G_current_gpu_id].pcap.gpu_min_pcap_mw = G_gpu_op_req_args.data[0];
                g_amec->gpu[G_current_gpu_id].pcap.gpu_max_pcap_mw =  G_gpu_op_req_args.data[1];
                g_amec->gpu[G_current_gpu_id].pcap.gpu_default_pcap_mw =  G_gpu_op_req_args.data[2];

                if( (g_amec->gpu[G_current_gpu_id].pcap.gpu_min_pcap_mw != L_last_min[G_current_gpu_id]) ||
                    (g_amec->gpu[G_current_gpu_id].pcap.gpu_max_pcap_mw != L_last_max[G_current_gpu_id]) )
                {
                    L_last_min[G_current_gpu_id] = g_amec->gpu[G_current_gpu_id].pcap.gpu_min_pcap_mw;
                    L_last_max[G_current_gpu_id] = g_amec->gpu[G_current_gpu_id].pcap.gpu_max_pcap_mw;
                    TRAC_IMP("gpu_read_pwr_limit: GPU%d min=0x%08XmW max=0x%08XmW",
                             G_current_gpu_id,
                             g_amec->gpu[G_current_gpu_id].pcap.gpu_min_pcap_mw,
                             g_amec->gpu[G_current_gpu_id].pcap.gpu_max_pcap_mw);
                }

                // Done with this GPU ready to move to new one
                L_read_pwr_limit_failure_count[G_current_gpu_id] = 0;
                L_read_pwr_limit_state = GPU_STATE_READ_PWR_LIMIT_NEW;
                L_attempts = 0;
                l_complete = TRUE;
                break;

            default:
                INTR_TRAC_ERR("gpu_read_pwr_limit: INVALID STATE: 0x%02X", L_read_pwr_limit_state);
                L_read_pwr_limit_state = GPU_STATE_READ_PWR_LIMIT_NEW;
                l_complete = TRUE;
                break;
        } // switch L_read_pwr_limit_state

        if(L_scheduled)
        {
            GPU_DBG("gpu_read_pwr_limit: Scheduled check driver loaded state 0x%02X at tick %d",
                     L_read_pwr_limit_state, GPU_TICK);
        }
        else if(!l_complete)  // if not complete there must have been a failure on the schedule
        {
            INTR_TRAC_ERR("gpu_read_pwr_limit: failed to schedule state 0x%02X", L_read_pwr_limit_state);
        }

    } // if async_request_is_idle
    else
    {
       INTR_TRAC_ERR("gpu_read_pwr_limit: NOT idle for state 0x%02X", L_read_pwr_limit_state);
    }

    return l_complete;
} // end gpu_read_pwr_limit_sm()

// Function Specification
//
// Name:  gpu_set_pwr_limit_sm
//
// Description: Called from gpu_task_sm to set GPU power limit for G_current_gpu_id
//              This function should only return that complete is TRUE when the set
//              is complete (or determined failed) and ready for a different GPU
//
// Pre-Req:  Caller must have G_current_gpu_id set for GPU to read
//
// End Function Specification
bool gpu_set_pwr_limit_sm()
{
    bool l_complete = FALSE;   // only return TRUE when complete or failed
    static bool L_scheduled = FALSE;  // indicates if a GPU GPE request was scheduled
    static uint8_t L_state_failure_count = 0;
    static uint8_t L_set_pwr_limit_failure_count[MAX_NUM_GPU_PER_DOMAIN] = {0};
    static gpuSetPwrLimitState_e L_set_pwr_limit_state = GPU_STATE_SET_PWR_LIMIT_NEW;
    static bool L_error_logged[MAX_NUM_GPU_PER_DOMAIN] = {FALSE};
    static uint32_t L_attempts = 0;

    static uint32_t L_last_pcap[MAX_NUM_GPU_PER_DOMAIN] = {0};

    static uint32_t L_num_ticks = 0;

    static bool L_retry_necessary = FALSE;

    L_num_ticks++;

    if (async_request_is_idle(&G_gpu_op_request.request))
    {
        // If not starting a new set limit then need to check status of current state before moving on
        // stay in current state if the schedule failed or the state isn't finished/failed
        if( (L_set_pwr_limit_state != GPU_STATE_SET_PWR_LIMIT_NEW) &&
            (!L_scheduled || (GPE_RC_SUCCESS != G_gpu_op_req_args.error.rc)) )
        {
            // Repeat step 4 until it succeeds. More details on this in GPE code.
            if( (L_set_pwr_limit_state == GPU_STATE_SET_PWR_LIMIT_4_FINISH) &&
                (GPE_RC_NOT_COMPLETE == G_gpu_op_req_args.error.rc) &&
                (L_attempts <= GPU_TIMEOUT) )
            {
                L_set_pwr_limit_state = GPU_STATE_SET_PWR_LIMIT_4_START;
                L_state_failure_count = 0;
                L_attempts++;
            }
            else if( (L_set_pwr_limit_state == GPU_STATE_SET_PWR_LIMIT_3_3) &&
                     (GPE_RC_GPU_BUSY == G_gpu_op_req_args.error.rc) )
            {
                 L_set_pwr_limit_state = GPU_STATE_SET_PWR_LIMIT_3_FINISH;
                 L_retry_necessary = TRUE;
            }
            // Check if failure was due to driver change
            else if(G_gpu_op_req_args.error.rc == GPE_RC_GPU_DRIVER_CHANGE)
            {
                handle_driver_change();
                // Request can't be processed by GPU at this time so we are done with this GPU
                // setup to start new request
                L_state_failure_count = 0;
                L_set_pwr_limit_failure_count[G_current_gpu_id] = 0; // clear failure count since there's a driver change
                L_set_pwr_limit_state = GPU_STATE_SET_PWR_LIMIT_NEW;
                L_attempts = 0;
                L_retry_necessary = FALSE;
                return TRUE;  // Done with this GPU, let GPU SM move to next
            }
            // If reached retry count give up on this read
            else if( (L_state_failure_count > MAX_GPU_READ_ATTEMPT) ||
                     (L_attempts > GPU_TIMEOUT) )
            {

                if(L_attempts > GPU_TIMEOUT)
                {
                    // give up trying to set power limit for this GPU
                    // It will be retried if detected that GPU is put in reset and then taken out or driver change
                    g_amec->gpu[G_current_gpu_id].pcap.set_failed = true;
                    L_set_pwr_limit_failure_count[G_current_gpu_id] = 0;
                    INTR_TRAC_ERR("gpu_set_pwr_limit: Timedout setting power limit %d for GPU%d [attempts:%d][state_fail:%d]",
                                  G_gpu_op_req_args.data[0], G_current_gpu_id, L_attempts, L_state_failure_count);
                }
                // if GPU is not in reset then INC error count and check if reached threshold
                if(g_amec->gpu[G_current_gpu_id].status.notReset)
                {
                    if(++L_set_pwr_limit_failure_count[G_current_gpu_id] > GPU_SET_PWR_LIMIT_ERROR_COUNT)
                    {
                        INTR_TRAC_ERR("gpu_set_pwr_limit: Failed to set power limit %d for GPU%d RC: 0x%02X",
                                       G_gpu_op_req_args.data[0],
                                       G_current_gpu_id,
                                       G_gpu_op_req_args.gpu_rc);

                        // give up trying to set power limit for this GPU
                        // It will be retried if detected that GPU is put in reset and then taken out or driver change
                        g_amec->gpu[G_current_gpu_id].pcap.set_failed = true;
                        L_set_pwr_limit_failure_count[G_current_gpu_id] = 0;

                        // log error that power limit could not be set
                        if(!L_error_logged[G_current_gpu_id])
                        {
                            L_error_logged[G_current_gpu_id] = TRUE;

                            // Log error
                            /* @
                             * @errortype
                             * @moduleid    GPU_MID_GPU_SET_PWR_LIMIT
                             * @reasoncode  GPU_FAILURE
                             * @userdata1   GPU ID
                             * @userdata2   GPU RC
                             * @userdata4   ERC_GPU_SET_PWR_LIMIT_FAILURE
                             * @devdesc     Failure to set GPU power limit
                             *
                             */
                            errlHndl_t l_err = createErrl(GPU_MID_GPU_SET_PWR_LIMIT,
                                                          GPU_FAILURE,
                                                          ERC_GPU_SET_PWR_LIMIT_FAILURE,
                                                          ERRL_SEV_PREDICTIVE,
                                                          NULL,
                                                          DEFAULT_TRACE_SIZE,
                                                          G_current_gpu_id,
                                                          G_gpu_op_req_args.gpu_rc);

                            // Callout the GPU if have sensor ID for it
                            if(G_sysConfigData.gpu_sensor_ids[G_current_gpu_id])
                            {
                                addCalloutToErrl(l_err,
                                                 ERRL_CALLOUT_TYPE_GPU_ID,
                                                 G_sysConfigData.gpu_sensor_ids[G_current_gpu_id],
                                                 ERRL_CALLOUT_PRIORITY_MED);
                            }

                            // Commit Error
                            commitErrl(&l_err);
                        } // if error not logged
                    } // if reached error count
                } // if notReset

                L_set_pwr_limit_state = GPU_STATE_SET_PWR_LIMIT_NEW;
                L_state_failure_count = 0;
                L_attempts = 0;
                return TRUE;  // Done with this GPU, let GPU SM move to next
            } // if reached retry count
            else
            {
                // INC failure count and retry current state
                L_state_failure_count++;
            }
        }
        else // success on last state go to next state and process it
        {
            L_state_failure_count = 0;
            if( (GPU_STATE_SET_PWR_LIMIT_4_FINISH == L_set_pwr_limit_state ) &&
                (L_retry_necessary) )
            {
                // Let SM move to next
                L_set_pwr_limit_state = GPU_STATE_SET_PWR_LIMIT_NEW;
                return TRUE;
            }
            else
            {
                L_set_pwr_limit_state++;
            }
        }

        L_scheduled = FALSE;  // default nothing scheduled

        switch (L_set_pwr_limit_state)
        {
            // Step 1
            case GPU_STATE_SET_PWR_LIMIT_1_START:
                if(!L_retry_necessary) L_num_ticks = 1;
                L_retry_necessary = FALSE;
                L_scheduled = schedule_gpu_req(GPU_REQ_SET_PWR_LIMIT_1_START, G_new_gpu_req_args);
                break;

            case GPU_STATE_SET_PWR_LIMIT_1_2:
                L_scheduled = schedule_gpu_req(GPU_REQ_SET_PWR_LIMIT_1_2, G_new_gpu_req_args);
                break;

            case GPU_STATE_SET_PWR_LIMIT_1_3:
                L_scheduled = schedule_gpu_req(GPU_REQ_SET_PWR_LIMIT_1_3, G_new_gpu_req_args);
                break;

            case GPU_STATE_SET_PWR_LIMIT_1_FINISH:
                L_scheduled = schedule_gpu_req(GPU_REQ_SET_PWR_LIMIT_1_FINISH, G_new_gpu_req_args);
                break;

            // Step 2
            case GPU_STATE_SET_PWR_LIMIT_2_START:
                // send the desired GPU power cap to the GPE to send to GPU
                GPU_DBG("gpu_set_pwr_limit_sm: setting power limit to %dmW on GPU%d",
                         g_amec->gpu[G_current_gpu_id].pcap.gpu_desired_pcap_mw, G_current_gpu_id);
                G_new_gpu_req_args.data[1] = g_amec->gpu[G_current_gpu_id].pcap.gpu_desired_pcap_mw;
                L_scheduled = schedule_gpu_req(GPU_REQ_SET_PWR_LIMIT_2_START, G_new_gpu_req_args);
                break;

            case GPU_STATE_SET_PWR_LIMIT_2_2:
                L_scheduled = schedule_gpu_req(GPU_REQ_SET_PWR_LIMIT_2_2, G_new_gpu_req_args);
                break;

            case GPU_STATE_SET_PWR_LIMIT_2_3:
                L_scheduled = schedule_gpu_req(GPU_REQ_SET_PWR_LIMIT_2_3, G_new_gpu_req_args);
                break;

            case GPU_STATE_SET_PWR_LIMIT_2_FINISH:
                L_scheduled = schedule_gpu_req(GPU_REQ_SET_PWR_LIMIT_2_FINISH, G_new_gpu_req_args);
                break;

            // Step 3
            case GPU_STATE_SET_PWR_LIMIT_3_START:
                L_scheduled = schedule_gpu_req(GPU_REQ_SET_PWR_LIMIT_3_START, G_new_gpu_req_args);
                break;

            case GPU_STATE_SET_PWR_LIMIT_3_2:
                L_scheduled = schedule_gpu_req(GPU_REQ_SET_PWR_LIMIT_3_2, G_new_gpu_req_args);
                break;

            case GPU_STATE_SET_PWR_LIMIT_3_3:
                L_scheduled = schedule_gpu_req(GPU_REQ_SET_PWR_LIMIT_3_3, G_new_gpu_req_args);
                break;

            case GPU_STATE_SET_PWR_LIMIT_3_FINISH:
                L_attempts = 0;
                L_scheduled = schedule_gpu_req(GPU_REQ_SET_PWR_LIMIT_3_FINISH, G_new_gpu_req_args);
                break;

            // Step 4
            case GPU_STATE_SET_PWR_LIMIT_4_START:
                L_scheduled = schedule_gpu_req(GPU_REQ_SET_PWR_LIMIT_4_START, G_new_gpu_req_args);
                break;

            case GPU_STATE_SET_PWR_LIMIT_4_2:
                L_scheduled = schedule_gpu_req(GPU_REQ_SET_PWR_LIMIT_4_2, G_new_gpu_req_args);
                break;

            case GPU_STATE_SET_PWR_LIMIT_4_FINISH:
                L_scheduled = schedule_gpu_req(GPU_REQ_SET_PWR_LIMIT_4_FINISH, G_new_gpu_req_args);
                break;

            case GPU_STATE_SET_PWR_LIMIT_COMPLETE:
                update_gpu_tick_sensor(&G_gpu_tick_times.setpcap[G_current_gpu_id], L_num_ticks);
                GPU_DBG("gpu_set_pwr_limit_sm: took %d ticks to finish setting pcap for GPU%d", L_attempts, G_current_gpu_id);
                // Update the requested power limit since it was successfully sent
                // NOTE: want this value to be sent back from the GPE to know what was set in case AMEC
                // has caluclated a new desired pcap while this one was already in process of being set
                g_amec->gpu[G_current_gpu_id].pcap.gpu_requested_pcap_mw = (uint32_t) G_gpu_op_req_args.data[0];
                if(g_amec->gpu[G_current_gpu_id].pcap.gpu_requested_pcap_mw != L_last_pcap[G_current_gpu_id])
                {
                    L_last_pcap[G_current_gpu_id] = g_amec->gpu[G_current_gpu_id].pcap.gpu_requested_pcap_mw;
                    TRAC_IMP("gpu_set_pwr_limit_sm: successfully set power limit to %dmW on GPU%d",
                              g_amec->gpu[G_current_gpu_id].pcap.gpu_desired_pcap_mw, G_current_gpu_id);
                }

                // Done with this GPU ready to move to new one
                L_set_pwr_limit_failure_count[G_current_gpu_id] = 0;
                L_set_pwr_limit_state = GPU_STATE_SET_PWR_LIMIT_NEW;
                L_attempts = 0;
                l_complete = TRUE;
                break;

            default:
                INTR_TRAC_ERR("gpu_set_pwr_limit: INVALID STATE: 0x%02X", L_set_pwr_limit_state);
                L_set_pwr_limit_state = GPU_STATE_SET_PWR_LIMIT_NEW;
                l_complete = TRUE;
                break;
        } // switch L_set_pwr_limit_state

        if(L_scheduled)
        {
            GPU_DBG("gpu_set_pwr_limit: Scheduled set power cap state 0x%02X at tick %d",
                    L_set_pwr_limit_state, GPU_TICK);
        }
        else if(!l_complete)  // if not complete there must have been a failure on the schedule
        {
            INTR_TRAC_ERR("gpu_set_pwr_limit: failed to schedule state 0x%02X", L_set_pwr_limit_state);
        }
    } // if async_request_is_idle
    else
    {
        INTR_TRAC_ERR("gpu_set_pwr_limit: NOT idle for state 0x%02X", L_set_pwr_limit_state);
    }

    return l_complete;
} // end gpu_set_pwr_limit_sm()

// Function Specification
//
// Name:  gpu_read_temp_sm
//
// Description: Called from gpu_task_sm to read GPU core temperature of G_current_gpu_id
//              This function should only return that complete is TRUE when the temperature
//              read is complete (or determined failed) and ready to start reading a different GPU
//
// Pre-Req:     Caller must have G_current_gpu_id set for GPU to read and
//              verified G_gpu_op_request is idle to allow scheduling
// End Function Specification
bool gpu_read_temp_sm()
{
    bool l_complete = FALSE;   // only return TRUE when the read is complete or failed
    uint16_t l_temp = 0;
    static bool L_scheduled = FALSE;  // indicates if a GPU GPE request was scheduled
    static uint8_t L_read_failure_count = 0; // Used for I2C errors
    static bool L_trace_success = FALSE;
    static gpuReadTempState_e L_read_temp_state = GPU_STATE_READ_TEMP_NEW;  // 1st state for reading temp

    static uint32_t L_num_ticks = 0;

    L_num_ticks++;

    if (async_request_is_idle(&G_gpu_op_request.request))
    {
        // If not starting a new read then need to check status of current state before moving on
        // stay in current state if the schedule failed or the state isn't finished/failed
        if( (L_read_temp_state != GPU_STATE_READ_TEMP_NEW) &&
             (!L_scheduled || (GPE_RC_SUCCESS != G_gpu_op_req_args.error.rc)) )
        {
            // If reached retry count give up on this GPU
            if( (L_read_failure_count > MAX_GPU_READ_ATTEMPT) ||
                (GPE_RC_I2C_ERROR == G_gpu_op_req_args.error.rc) )
            {
                mark_gpu_failed(&G_gpu_op_req_args);

                L_read_temp_state = GPU_STATE_READ_TEMP_NEW;
                L_read_failure_count = 0;
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
                L_num_ticks = 1;
                L_scheduled = schedule_gpu_req(GPU_REQ_READ_TEMP_START, G_new_gpu_req_args);
                break;

            case GPU_STATE_READ_TEMP_FINISH:
                L_scheduled = schedule_gpu_req(GPU_REQ_READ_TEMP_FINISH, G_new_gpu_req_args);
                break;

            case GPU_STATE_READ_TEMP_COMPLETE:
                update_gpu_tick_sensor(&G_gpu_tick_times.coretemp[G_current_gpu_id], L_num_ticks);
                if( (!g_amec->gpu[G_current_gpu_id].status.readOnce) &&
                    (0 != G_gpu_op_req_args.data[0]) )
                {
                    g_amec->gpu[G_current_gpu_id].status.readOnce = true;

                    // Only trace this once
                    if(FALSE == L_trace_success)
                    {
                        TRAC_INFO("First successful attempt to read temp from GPU%d was on tick %d",
                                   G_current_gpu_id, CURRENT_TICK);
                        L_trace_success = TRUE;
                    }

                    // comm is now established update for capability checking to take place
                    g_amec->gpu[G_current_gpu_id].status.checkDriverLoaded = TRUE;
                }
                // Update sensor
                l_temp = G_gpu_op_req_args.data[0];
                sensor_update(AMECSENSOR_PTR(TEMPGPU0 + G_current_gpu_id), l_temp);

                // Clear all past errors
                g_amec->gpu[G_current_gpu_id].status.coreTempFailure = false;
                g_amec->gpu[G_current_gpu_id].status.coreTempNotAvailable = false;
                g_amec->gpu[G_current_gpu_id].status.errorCount = 0;
                g_amec->gpu[G_current_gpu_id].status.retryCount = 0;

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
                                         ERRL_CALLOUT_TYPE_GPU_ID,
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
    static uint8_t L_read_mem_cap_failure_count[MAX_NUM_GPU_PER_DOMAIN] = {0};
    static uint8_t L_state_failure_count = 0;
    static gpuReadMemTempCapableState_e L_read_cap_state = GPU_STATE_READ_MEM_TEMP_CAPABLE_NEW;
    static bool L_error_logged[MAX_NUM_GPU_PER_DOMAIN] = {FALSE};

    static uint32_t L_num_ticks = 0;

    L_num_ticks++;

    if (async_request_is_idle(&G_gpu_op_request.request))
    {
       // If not starting a new read then need to check status of current state before moving on
       // stay in current state if the schedule failed or the state isn't finished/failed
       if( (L_read_cap_state != GPU_STATE_READ_MEM_TEMP_CAPABLE_NEW) &&
           (!L_scheduled || (GPE_RC_SUCCESS != G_gpu_op_req_args.error.rc)) )
       {
          // Check if failure was due to driver change
          if(G_gpu_op_req_args.error.rc == GPE_RC_GPU_DRIVER_CHANGE)
          {
             handle_driver_change();
             // Request can't be processed by GPU at this time so we are done with this GPU
             // setup to start new request
             L_state_failure_count = 0;
             L_read_mem_cap_failure_count[G_current_gpu_id] = 0; // clear failure count since there's a driver change
             L_read_cap_state = GPU_STATE_READ_MEM_TEMP_CAPABLE_NEW;
             return TRUE;  // Done with this GPU, let GPU SM move to next
          }
          // If reached state retry count give up on this read
          else if(L_state_failure_count > MAX_GPU_READ_ATTEMPT)
          {
             // if GPU is not in reset then INC error count and check if reached threshold
             if(g_amec->gpu[G_current_gpu_id].status.notReset)
             {
                if(++L_read_mem_cap_failure_count[G_current_gpu_id] > GPU_READ_MEM_CAP_ERROR_COUNT)
                {
                    INTR_TRAC_ERR("gpu_read_mem_temp_capable: Failed to read capability for GPU%d RC: 0x%02X",
                                   G_current_gpu_id,
                                   G_gpu_op_req_args.gpu_rc);

                    // give up trying to read mem temp capability for this GPU
                    // It will be retried if detected that GPU driver is re-loaded
                    g_amec->gpu[G_current_gpu_id].status.checkMemTempSupport = FALSE;
                    L_read_mem_cap_failure_count[G_current_gpu_id] = 0;

                    // cannot determine memory temp capability, mark memory temp as failed
                    g_amec->gpu[G_current_gpu_id].status.memTempFailure = true;
                    g_amec->gpu[G_current_gpu_id].status.memTempNotAvailable = true;

                    // log one time error that memory temp capability couldn't be determined
                    if(!L_error_logged[G_current_gpu_id])
                    {
                       L_error_logged[G_current_gpu_id] = TRUE;

                       // Log error
                       /* @
                        * @errortype
                        * @moduleid    GPU_MID_GPU_READ_MEM_TEMP_CAPABLE
                        * @reasoncode  GPU_FAILURE
                        * @userdata1   GPU ID
                        * @userdata2   GPU RC
                        * @userdata4   ERC_GPU_READ_MEM_TEMP_CAPABLE_FAILURE
                        * @devdesc     Failure to read memory temp capability
                        *
                        */
                       errlHndl_t l_err = createErrl(GPU_MID_GPU_READ_MEM_TEMP_CAPABLE,
                                                     GPU_FAILURE,
                                                     ERC_GPU_READ_MEM_TEMP_CAPABLE_FAILURE,
                                                     ERRL_SEV_PREDICTIVE,
                                                     NULL,
                                                     DEFAULT_TRACE_SIZE,
                                                     G_current_gpu_id,
                                                     G_gpu_op_req_args.gpu_rc);

                       // Callout the GPU if have sensor ID for it
                       if(G_sysConfigData.gpu_sensor_ids[G_current_gpu_id])
                       {
                          addCalloutToErrl(l_err,
                                           ERRL_CALLOUT_TYPE_GPU_ID,
                                           G_sysConfigData.gpu_sensor_ids[G_current_gpu_id],
                                           ERRL_CALLOUT_PRIORITY_MED);
                       }

                       // Commit Error
                       commitErrl(&l_err);
                    } // if error not logged
                } // if reached error count
             } // if notReset

             L_read_cap_state = GPU_STATE_READ_MEM_TEMP_CAPABLE_NEW;
             L_state_failure_count = 0;
             return TRUE;  // Done with this GPU, let GPU SM move to next
          } // if reached state retry count
          else
          {
             // INC failure count and retry current state
             L_state_failure_count++;
          }
       }
       else // success on last state go to next state and process it
       {
          L_state_failure_count = 0;
          L_read_cap_state++;
       }

       L_scheduled = FALSE;  // default nothing scheduled

       switch (L_read_cap_state)
       {
           case GPU_STATE_READ_MEM_TEMP_CAPABLE_START:
               L_num_ticks = 1;
               L_scheduled = schedule_gpu_req(GPU_REQ_READ_CAPS_START, G_new_gpu_req_args);
               break;

           case GPU_STATE_READ_MEM_TEMP_CAPABLE_2:
               L_scheduled = schedule_gpu_req(GPU_REQ_READ_CAPS_2, G_new_gpu_req_args);
               break;

           case GPU_STATE_READ_MEM_TEMP_CAPABLE_3:
               L_scheduled = schedule_gpu_req(GPU_REQ_READ_CAPS_3, G_new_gpu_req_args);
               break;

           case GPU_STATE_READ_MEM_TEMP_CAPABLE_READ:
               L_scheduled = schedule_gpu_req(GPU_REQ_READ_CAPS_FINISH, G_new_gpu_req_args);
               break;

           case GPU_STATE_READ_MEM_TEMP_CAPABLE_COMPLETE:
               update_gpu_tick_sensor(&G_gpu_tick_times.capabilities[G_current_gpu_id], L_num_ticks);
               // Update capability
               g_amec->gpu[G_current_gpu_id].status.memTempSupported = G_gpu_op_req_args.data[0] & 0x01;

               if(g_amec->gpu[G_current_gpu_id].status.memTempSupported)
               {
                  // mem temp is supported no need to re-check capability
                  g_amec->gpu[G_current_gpu_id].status.checkMemTempSupport = FALSE;
               }
               else
               {
                  // Need to keep query for mem temp capability to detect if ever changes to capable
                  g_amec->gpu[G_current_gpu_id].status.checkMemTempSupport = TRUE;
               }

               // Done with this GPU ready to move to new one
               L_read_mem_cap_failure_count[G_current_gpu_id] = 0;
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

    static uint32_t L_num_ticks = 0;

    L_num_ticks++;

    if (async_request_is_idle(&G_gpu_op_request.request))
    {
       // If not starting a new read then need to check status of current state before moving on
       // stay in current state if the schedule failed or the state isn't finished/failed
       if( (L_read_temp_state != GPU_STATE_READ_MEM_TEMP_NEW) &&
           (!L_scheduled || (GPE_RC_SUCCESS != G_gpu_op_req_args.error.rc)) )
       {
          // Check if failure was due to driver change
          if(G_gpu_op_req_args.error.rc == GPE_RC_GPU_DRIVER_CHANGE)
          {
             handle_driver_change();
             // Request can't be processed by GPU at this time so we are done with this GPU
             // setup to start new request
             L_read_failure_count = 0;
             g_amec->gpu[G_current_gpu_id].status.memErrorCount = 0;
             L_read_temp_state = GPU_STATE_READ_MEM_TEMP_NEW;
             return TRUE;  // Done with this GPU, let GPU SM move to next
          }

          // If reached retry count or GPU indicated cmd not supported then give up on this read
          else if( (L_read_failure_count > MAX_GPU_READ_ATTEMPT) ||
                   (G_gpu_op_req_args.error.rc == GPE_RC_GPU_CMD_NOT_SUPPORTED) )
          {
             // if GPU is not in reset or the GPU responded with command not supported then
             // INC memory error count and check if reached timeout for new mem temp
             if( (g_amec->gpu[G_current_gpu_id].status.notReset) ||
                 (G_gpu_op_req_args.error.rc == GPE_RC_GPU_CMD_NOT_SUPPORTED) )
             {
                g_amec->gpu[G_current_gpu_id].status.memErrorCount++;

                uint8_t max_read_timeout = G_data_cnfg->thrm_thresh.data[DATA_FRU_GPU_MEM].max_read_timeout;
                if((max_read_timeout) && (max_read_timeout != 0xFF) &&
                   (g_amec->gpu[G_current_gpu_id].status.memErrorCount >= max_read_timeout) )
                {
                    // Disable memory temp reading for this GPU and log error
                    g_amec->gpu[G_current_gpu_id].status.memTempSupported = FALSE;
                    // so BMC knows there is an error for fan control set failure
                    g_amec->gpu[G_current_gpu_id].status.memTempFailure = true;
                    g_amec->gpu[G_current_gpu_id].status.memTempNotAvailable = true;

                    INTR_TRAC_ERR("gpu_read_memory_temp: disabling memory temp for GPU%d due to %d consecutive errors",
                                    G_current_gpu_id, g_amec->gpu[G_current_gpu_id].status.memErrorCount);

                    if(g_amec->gpu[G_current_gpu_id].status.commErrorLogged == false)
                    {
                       INTR_TRAC_ERR("notReset: %d rc: 0x%0X", g_amec->gpu[G_current_gpu_id].status.notReset,
                                     G_gpu_op_req_args.error.rc);
                       // Log error
                       /* @
                        * @errortype
                        * @moduleid    GPU_MID_GPU_READ_MEM_TEMP
                        * @reasoncode  GPU_FAILURE
                        * @userdata1   GPU ID
                        * @userdata2   GPU RC
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
                                                     G_gpu_op_req_args.gpu_rc,
                                                     g_amec->gpu[G_current_gpu_id].status.memErrorCount);

                       // Callout the GPU if have sensor ID for it
                       if(G_sysConfigData.gpu_sensor_ids[G_current_gpu_id])
                       {
                          addCalloutToErrl(l_err,
                                           ERRL_CALLOUT_TYPE_GPU_ID,
                                           G_sysConfigData.gpu_sensor_ids[G_current_gpu_id],
                                           ERRL_CALLOUT_PRIORITY_MED);
                       }

                       // Commit Error
                       commitErrl(&l_err);
                       g_amec->gpu[G_current_gpu_id].status.commErrorLogged = true;
                    } // if !commErrorLogged
                } // if timeout error
                else if(G_gpu_op_req_args.error.rc == GPE_RC_GPU_CMD_NOT_SUPPORTED)
                {
                    // GPU indicated command not supported, re-check mem temp capability
                    // if we try to read mem temp again that means mem temp was reported capable
                    // and if this continues to fail eventually an error will be logged above at timeout
                    g_amec->gpu[G_current_gpu_id].status.checkMemTempSupport = true;
                    g_amec->gpu[G_current_gpu_id].status.memTempSupported = false;
                }
             } // if notReset or command not supported

             // setup to start new request
             L_read_failure_count = 0;
             L_read_temp_state = GPU_STATE_READ_MEM_TEMP_NEW;
             return TRUE;  // Done with this GPU, let GPU SM move to next
          }  // else if failure count exceeded or command not supported
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
               L_num_ticks = 1;
               L_scheduled = schedule_gpu_req(GPU_REQ_READ_MEM_TEMP_START, G_new_gpu_req_args);
               break;

           case GPU_STATE_READ_MEM_TEMP_2:
               L_scheduled = schedule_gpu_req(GPU_REQ_READ_MEM_TEMP_2, G_new_gpu_req_args);
               break;

           case GPU_STATE_READ_MEM_TEMP_3:
               L_scheduled = schedule_gpu_req(GPU_REQ_READ_MEM_TEMP_3, G_new_gpu_req_args);
               break;

            case GPU_STATE_READ_MEM_TEMP_READ:
                L_scheduled = schedule_gpu_req(GPU_REQ_READ_MEM_TEMP_FINISH, G_new_gpu_req_args);
                break;

           case GPU_STATE_READ_MEM_TEMP_COMPLETE:
               update_gpu_tick_sensor(&G_gpu_tick_times.memtemp[G_current_gpu_id], L_num_ticks);
               // Update sensor
               l_temp = G_gpu_op_req_args.data[0];
               sensor_update(AMECSENSOR_PTR(TEMPGPU0MEM + G_current_gpu_id), l_temp);

               // Clear past errors
               g_amec->gpu[G_current_gpu_id].status.memTempFailure = false;
               g_amec->gpu[G_current_gpu_id].status.memTempNotAvailable = false;
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
                                       ERRL_CALLOUT_TYPE_GPU_ID,
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

        // 3.  Time to start new temperature reads?
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
        // 4.  Need to check if driver is loaded?
        l_gpu_id = gpu_id_need_driver_check();
        if(l_gpu_id != 0xFF)
        {
            // Found a GPU that needs driver checked
            G_current_gpu_id = l_gpu_id;
            G_gpu_state = GPU_STATE_CHECK_DRIVER_LOADED;
            l_new_state = TRUE;
            break;
        }

        // 5.  Need to read power limits?
        l_gpu_id = gpu_id_need_power_limits();
        if(l_gpu_id != 0xFF)
        {
            // Found a GPU that needs power limits read
            G_current_gpu_id = l_gpu_id;
            G_gpu_state = GPU_STATE_READ_PWR_LIMIT;
            l_new_state = TRUE;
            break;
        }

        // 6.  Need to read memory temps?
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
        // Read and update reset status for all GPUs
        update_gpu_reset_status();

        // Initialize the IPC commands if this is our first run
        if(L_gpu_first_run)
        {
            gpu_ipc_init();
            L_gpu_first_run = FALSE;
        }

        // Check if time to start reading temperatures
        // GPU temperatures (core and memory) are only used for fan control which happens every 1s
        // so there is no need to read the GPU temperatures any faster than every 1s
        if(!L_read_temp_start_needed)
        {
            L_numCallsForTempRead++;
            if(L_numCallsForTempRead >= GPU_TEMP_READ_1S)
            {
                 L_read_temp_start_needed = TRUE;
                 L_mem_temp_needed = FALSE;  // will get set to TRUE when core temp reads finish
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
                      L_mem_temp_needed = FALSE;  // will get set to TRUE when core temp reads finish
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
                      G_gpu_state = GPU_STATE_IDLE;
                      l_start_next_state = TRUE;
                   }
                   break;

               case GPU_STATE_CHECK_DRIVER_LOADED:
                   // Check if driver is loaded for current GPU
                   if(gpu_check_driver_loaded_sm())
                   {
                      // Driver check complete for this GPU,
                      // NOTE: Do not set status.checkDriverLoaded to false here, if driver is
                      // not loaded we need to keep checking for driver to be loaded this is decided
                      // inside gpu_check_driver_loaded_sm()

                      // go to IDLE state to let IDLE SM decide what to do next
                      G_gpu_state = GPU_STATE_IDLE;
                      l_start_next_state = TRUE;
                   }
                   break;

               case GPU_STATE_READ_PWR_LIMIT:
                   // Read power limits for current GPU
                   if(gpu_read_pwr_limit_sm())
                   {
                      // Read power limits complete for this GPU, go to IDLE state
                      // to let IDLE SM decide what to do next
                      G_gpu_state = GPU_STATE_IDLE;
                      l_start_next_state = TRUE;
                   }
                   break;

               case GPU_STATE_SET_PWR_LIMIT:
                   // Set power limit on current GPU
                   if(gpu_set_pwr_limit_sm())
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

               // before starting the reset check if OPAL needs the lock
               L_occ_owns_lock = check_and_update_i2c_lock(GPU_I2C_ENGINE);
               if (L_occ_owns_lock)
               {
                   // We still own the lock start the reset
                   G_gpu_state = GPU_STATE_RESET;
                   l_start_next_state = TRUE;
               }
               else
               {
                   // We don't own the lock, the reset will happen when we get the lock back
                   G_gpu_state = GPU_STATE_NO_LOCK;
                   l_start_next_state = FALSE;
               }
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
                     L_mem_temp_needed = FALSE;  // will get set to TRUE when core temp reads finish
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
