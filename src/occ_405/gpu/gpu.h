/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/gpu/gpu.h $                                       */
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

#ifndef _GPU_H
#define _GPU_H

#include <occ_common.h>
#include <trac_interface.h>
#include <errl.h>
#include <rtls.h>
#include "gpu_structs.h"

#define GPU_TICK (CURRENT_TICK % MAX_NUM_TICKS)

// States for the GPU state machine (task_gpu_sm)
typedef enum
{
    GPU_STATE_RESET                     = 0x00, // Reset and initialize interface
    GPU_STATE_READ_TEMP                 = 0x10, // Read GPU core temperature
    GPU_STATE_READ_MEMORY_TEMP          = 0x20, // Read GPU memory temperature
    GPU_STATE_CHECK_MEM_TEMP_CAPABLE    = 0x30, // Read memory temperature capability
    GPU_STATE_CHECK_DRIVER_LOADED       = 0x40, // Check if Driver loaded
    GPU_STATE_READ_PWR_LIMIT            = 0x50, // Read Power Limits
    GPU_STATE_SET_PWR_LIMIT             = 0x70, // Set Power Limit
    GPU_STATE_IDLE                      = 0xFE, // Ok to schedule new task
    GPU_STATE_NO_LOCK                   = 0xFF  // Host owns, no communication allowed
} gpuState_e;

// States for the GPU reset state machine (gpu_reset_sm)
typedef enum
{
    GPU_RESET_STATE_NEW                       = 0x01, // new reset attempt
    GPU_RESET_STATE_INIT_BUS                  = 0x02,
    GPU_RESET_STATE_RESET_MASTER              = 0x03, // Reset master
    GPU_RESET_STATE_RESET_SLAVE               = 0x04, // Start of slave port 4 reset
    GPU_RESET_STATE_RESET_SLAVE_WAIT          = 0x05,
    GPU_RESET_STATE_RESET_SLAVE_COMPLETE      = 0x06,
    GPU_RESET_STATE_RESET_FINISH              = 0x07,
} gpuResetState_e;

// States for reading GPU core temperature (gpu_read_temp_sm)
typedef enum
{
    GPU_STATE_READ_TEMP_NEW      = 0x11, // new temp read
    GPU_STATE_READ_TEMP_START    = 0x12, // start write temp reg
    GPU_STATE_READ_TEMP_FINISH   = 0x13, // read temperature
    GPU_STATE_READ_TEMP_COMPLETE = 0x14, // store temperature read
} gpuReadTempState_e;

// States for reading GPU memory temperature (gpu_read_mem_temp_sm)
typedef enum
{
    GPU_STATE_READ_MEM_TEMP_NEW      = 0x21,
    GPU_STATE_READ_MEM_TEMP_START    = 0x22,
    GPU_STATE_READ_MEM_TEMP_2        = 0x23,
    GPU_STATE_READ_MEM_TEMP_3        = 0x24,
    GPU_STATE_READ_MEM_TEMP_READ     = 0x25,
    GPU_STATE_READ_MEM_TEMP_COMPLETE = 0x26,
} gpuReadMemTempState_e;

// States for checking GPU memory temperature capability (gpu_read_mem_temp_capability_sm)
typedef enum
{
    GPU_STATE_READ_MEM_TEMP_CAPABLE_NEW      = 0x31,
    GPU_STATE_READ_MEM_TEMP_CAPABLE_START    = 0x32,
    GPU_STATE_READ_MEM_TEMP_CAPABLE_2        = 0x33,
    GPU_STATE_READ_MEM_TEMP_CAPABLE_3        = 0x34,
    GPU_STATE_READ_MEM_TEMP_CAPABLE_READ     = 0x35,
    GPU_STATE_READ_MEM_TEMP_CAPABLE_COMPLETE = 0x36,
} gpuReadMemTempCapableState_e;

// States for checking if GPU driver is loaded (gpu_check_driver_loaded_sm)
typedef enum
{
    GPU_STATE_CHECK_DRIVER_LOADED_NEW      = 0x41,
    GPU_STATE_CHECK_DRIVER_LOADED_START    = 0x42,
    GPU_STATE_CHECK_DRIVER_LOADED_2        = 0x43,
    GPU_STATE_CHECK_DRIVER_LOADED_3        = 0x44,
    GPU_STATE_CHECK_DRIVER_LOADED_READ     = 0x45,
    GPU_STATE_CHECK_DRIVER_LOADED_COMPLETE = 0x46,
} gpuCheckDriverLoadedState_e;

// States for reading GPU power limits (gpu_read_pwr_limit_sm)
typedef enum
{
    GPU_STATE_READ_PWR_LIMIT_NEW      = 0x51,
    GPU_STATE_READ_PWR_LIMIT_1_START  = 0x52,
    GPU_STATE_READ_PWR_LIMIT_1_2      = 0x53,
    GPU_STATE_READ_PWR_LIMIT_1_3      = 0x54,
    GPU_STATE_READ_PWR_LIMIT_1_FINISH = 0x55,
    GPU_STATE_READ_PWR_LIMIT_2_START  = 0x56,
    GPU_STATE_READ_PWR_LIMIT_2_2      = 0x57,
    GPU_STATE_READ_PWR_LIMIT_2_FINISH = 0x58,
    GPU_STATE_READ_PWR_LIMIT_3_START  = 0x59,
    GPU_STATE_READ_PWR_LIMIT_3_2      = 0x5A,
    GPU_STATE_READ_PWR_LIMIT_3_3      = 0x5B,
    GPU_STATE_READ_PWR_LIMIT_3_FINISH = 0x5C,
    GPU_STATE_READ_PWR_LIMIT_4_START  = 0x5D,
    GPU_STATE_READ_PWR_LIMIT_4_2      = 0x5E,
    GPU_STATE_READ_PWR_LIMIT_4_3      = 0x5F,
    GPU_STATE_READ_PWR_LIMIT_4_FINISH = 0x60,
    GPU_STATE_READ_PWR_LIMIT_5_START  = 0x61,
    GPU_STATE_READ_PWR_LIMIT_5_2      = 0x62,
    GPU_STATE_READ_PWR_LIMIT_5_3      = 0x63,
    GPU_STATE_READ_PWR_LIMIT_5_FINISH = 0x64,
    GPU_STATE_READ_PWR_LIMIT_COMPLETE = 0x65,
} gpuReadPwrLimitState_e;

// States for setting GPU power limit (gpu_set_pwr_limit_sm)
typedef enum
{
    GPU_STATE_SET_PWR_LIMIT_NEW      = 0x71,
    GPU_STATE_SET_PWR_LIMIT_1_START  = 0x72,
    GPU_STATE_SET_PWR_LIMIT_1_1      = 0x73,
    GPU_STATE_SET_PWR_LIMIT_1_2      = 0x74,
    GPU_STATE_SET_PWR_LIMIT_1_FINISH = 0x75,
    GPU_STATE_SET_PWR_LIMIT_2_START  = 0x76,
    GPU_STATE_SET_PWR_LIMIT_2_1      = 0x77,
    GPU_STATE_SET_PWR_LIMIT_2_2      = 0x78,
    GPU_STATE_SET_PWR_LIMIT_2_FINISH = 0x79,
    GPU_STATE_SET_PWR_LIMIT_3_START  = 0x7A,
    GPU_STATE_SET_PWR_LIMIT_3_2      = 0x7B,
    GPU_STATE_SET_PWR_LIMIT_3_3      = 0x7C,
    GPU_STATE_SET_PWR_LIMIT_3_FINISH = 0x7D,
    GPU_STATE_SET_PWR_LIMIT_4_START  = 0x7E,
    GPU_STATE_SET_PWR_LIMIT_4_2      = 0x7F,
    GPU_STATE_SET_PWR_LIMIT_4_FINISH = 0x80,
    GPU_STATE_SET_PWR_LIMIT_COMPLETE = 0x81,
} gpuSetPwrLimitState_e;

// GPU IPC initialization
void gpu_ipc_init();

// GPU state machine
void task_gpu_sm(struct task *i_self);

typedef struct gpuTimingSensor
{
    uint32_t max;
    uint32_t avg;
    uint32_t count_1s;
    uint32_t count_100ms;
    uint32_t count_lt100ms;
    uint64_t accum;
    uint64_t count;
} gpuTimingSensor_t;

// Table for GPU timings
typedef struct gpuTimingTable
{
    gpuTimingSensor_t getpcap[MAX_NUM_GPU_PER_DOMAIN];
    gpuTimingSensor_t setpcap[MAX_NUM_GPU_PER_DOMAIN];
    gpuTimingSensor_t coretemp[MAX_NUM_GPU_PER_DOMAIN];
    gpuTimingSensor_t memtemp[MAX_NUM_GPU_PER_DOMAIN];
    gpuTimingSensor_t capabilities[MAX_NUM_GPU_PER_DOMAIN];
    gpuTimingSensor_t checkdriver[MAX_NUM_GPU_PER_DOMAIN];
} gpuTimingTable_t;

#endif //_GPU_H
