/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/mem/memory.h $                                    */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2014,2019                        */
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

#ifndef _MEMORY_H
#define _MEMORY_H

#include "occ_sys_config.h"

typedef enum
{
    MEMORY_CONTROL_GPE_STILL_RUNNING = 0x01,
    MEMORY_CONTROL_RESERVED_1        = 0x02,
    MEMORY_CONTROL_RESERVED_2        = 0x04,
    MEMORY_CONTROL_RESERVED_3        = 0x08,
    MEMORY_CONTROL_RESERVED_4        = 0x10,
    MEMORY_CONTROL_RESERVED_5        = 0x20,
    MEMORY_CONTROL_RESERVED_6        = 0x40,
    MEMORY_CONTROL_RESERVED_7        = 0x80,
} eMemoryControlTraceFlags;

//per slot/mba throttle values used for rdimm/centaur-dimm control
typedef struct
{
    uint16_t max_n_per_mba;      //mode and OVS dependent, from config data
    uint16_t max_n_per_chip;     //mode and OVS dependent, from config data
    uint16_t min_n_per_mba;      //from config data
} memory_throttle_t;

// 128 bits encoding different bit fields corresponding to dimms
// either on centaurs or on I2C ports
typedef union
{
    uint64_t dw[2];
    uint8_t  bytes[16];
} dimm_sensor_flags_t;


//Memory data collect structures used for task data pointers
struct memory_control_task {
        uint8_t    startMemIndex;
        uint8_t    prevMemIndex;
        uint8_t    curMemIndex;
        uint8_t    endMemIndex;
        uint8_t    traceThresholdFlags;
        GpeRequest gpe_req;
} __attribute__ ((__packed__));
typedef struct memory_control_task memory_control_task_t;

extern memory_throttle_t G_memoryThrottleLimits[MAX_NUM_MEM_CONTROLLERS][MAX_NUM_MCU_PORTS];

void task_memory_control( task_t * i_task );

#endif // _MEMORY_H
