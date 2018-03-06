/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/common/gpu_structs.h $                                    */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2016,2018                        */
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

/* This header file is used by both occ_405 and occ_gpe1.                 */
/* Contains common structures and globals.                                */

#ifndef _GPU_STRUCTS_H
#define _GPU_STRUCTS_H

#include "occ_util.h"
#include <gpe_export.h>
#include "gpe_err.h"

#define MAX_GPUS 3

#define GPU_RESET_REQ_MASTER        1
#define GPU_RESET_REQ_SLV           2
#define GPU_RESET_REQ_SLV_COMPLETE  3

typedef enum
{
    GPU_CAP_MEM     = 0x00,
    GPU_CAP_CORE    = 0x01
} GPU_CAPABILITIES;

typedef enum
{
    ID_GPU0 = 0x00,
    ID_GPU1 = 0x01,
    ID_GPU2 = 0x02,
    ID_ALL_GPUS = 0xFF
} GPU_ID;

typedef enum
{
    GPU_STATE_PRESENT = 0x00000001,
    GPU_STATE_FAILED  = 0x80000000,
} GPU_STATE;

// GPU Request Operations
typedef enum
{
    // Initialize the GPU state machine and I2C engine
    GPU_REQ_INIT                        = 0x01,

    // Read GPU core temperature
    GPU_REQ_READ_TEMP_START             = 0x02,
    GPU_REQ_READ_TEMP_FINISH            = 0x03,

    // Read GPU memory temperature
    GPU_REQ_READ_MEM_TEMP_START         = 0x04,
    GPU_REQ_READ_MEM_TEMP_2             = 0x05,
    GPU_REQ_READ_MEM_TEMP_3             = 0x06,
    GPU_REQ_READ_MEM_TEMP_FINISH        = 0x07,

    // Read thermal capabilities
    GPU_REQ_READ_CAPS_START             = 0x08,
    GPU_REQ_READ_CAPS_2                 = 0x09,
    GPU_REQ_READ_CAPS_3                 = 0x0A,
    GPU_REQ_READ_CAPS_FINISH            = 0x0B,

    // Set GPU power cap
    GPU_REQ_SET_PWR_LIMIT_1_START       = 0x20,
    GPU_REQ_SET_PWR_LIMIT_1_2           = 0x21,
    GPU_REQ_SET_PWR_LIMIT_1_3           = 0x22,
    GPU_REQ_SET_PWR_LIMIT_1_FINISH      = 0x23,
    GPU_REQ_SET_PWR_LIMIT_2_START       = 0x24,
    GPU_REQ_SET_PWR_LIMIT_2_2           = 0x25,
    GPU_REQ_SET_PWR_LIMIT_2_3           = 0x26,
    GPU_REQ_SET_PWR_LIMIT_2_FINISH      = 0x27,
    GPU_REQ_SET_PWR_LIMIT_3_START       = 0x28,
    GPU_REQ_SET_PWR_LIMIT_3_2           = 0x29,
    GPU_REQ_SET_PWR_LIMIT_3_3           = 0x2A,
    GPU_REQ_SET_PWR_LIMIT_3_FINISH      = 0x2B,
    GPU_REQ_SET_PWR_LIMIT_4_START       = 0x2C,
    GPU_REQ_SET_PWR_LIMIT_4_2           = 0x2D,
    GPU_REQ_SET_PWR_LIMIT_4_FINISH      = 0x2E,

    // Start check driver loaded
    GPU_REQ_CHECK_DRIVER_START          = 0x31,
    GPU_REQ_CHECK_DRIVER_2              = 0x32,
    GPU_REQ_CHECK_DRIVER_3              = 0x33,
    GPU_REQ_CHECK_DRIVER_FINISH         = 0x34,

    // Read power limit policy
    GPU_REQ_GET_PWR_LIMIT_1_START       = 0x40,
    GPU_REQ_GET_PWR_LIMIT_1_2           = 0x41,
    GPU_REQ_GET_PWR_LIMIT_1_3           = 0x42,
    GPU_REQ_GET_PWR_LIMIT_1_FINISH      = 0x43,
    GPU_REQ_GET_PWR_LIMIT_2_START       = 0x44,
    GPU_REQ_GET_PWR_LIMIT_2_2           = 0x45,
    GPU_REQ_GET_PWR_LIMIT_2_FINISH      = 0x46,
    GPU_REQ_GET_PWR_LIMIT_3_START       = 0x47,
    GPU_REQ_GET_PWR_LIMIT_3_2           = 0x48,
    GPU_REQ_GET_PWR_LIMIT_3_3           = 0x49,
    GPU_REQ_GET_PWR_LIMIT_3_FINISH      = 0x4A,
    GPU_REQ_GET_PWR_LIMIT_4_START       = 0x4B,
    GPU_REQ_GET_PWR_LIMIT_4_2           = 0x4C,
    GPU_REQ_GET_PWR_LIMIT_4_3           = 0x4D,
    GPU_REQ_GET_PWR_LIMIT_4_FINISH      = 0x4E,
    GPU_REQ_GET_PWR_LIMIT_5_START       = 0x4F,
    GPU_REQ_GET_PWR_LIMIT_5_2           = 0x50,
    GPU_REQ_GET_PWR_LIMIT_5_3           = 0x51,
    GPU_REQ_GET_PWR_LIMIT_5_FINISH      = 0x52,

    // Reset the I2C master and slave
    GPU_REQ_RESET                       = 0x60,
} gpu_op_req_e;

// GPU arguments
typedef struct
{
  GpeErrorStruct error;
  uint8_t gpu_id;
  uint8_t gpu_rc;
  uint8_t operation;
  uint64_t data[3];
} gpu_sm_args_t;

typedef struct
{
    uint32_t pib_master;
    uint32_t bus_voltage;
    uint32_t port[MAX_GPUS];
    uint32_t addr[MAX_GPUS];
} gpu_i2c_info_t;

typedef struct
{
  GpeErrorStruct error;
  gpu_i2c_info_t  gpu_i2c;
} gpu_init_args_t;

#endif // _GPU_STRUCTS_H

