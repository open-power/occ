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

#define MEMBUF0_PRESENT_MASK      0x00008000ul

typedef enum
{
    MEM_TYPE_UNKNOWN = 0x00,
    MEM_TYPE_OCM     = 0xA0,
} MEMORY_TYPE;

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

// Enum for specifying each MemBuf
enum eOccMemBuf
{
  MEMBUF_0   = 0,
  MEMBUF_1   = 1,
  MEMBUF_2   = 2,
  MEMBUF_3   = 3,
  MEMBUF_4   = 4,
  MEMBUF_5   = 5,
  MEMBUF_6   = 6,
  MEMBUF_7   = 7,
  MEMBUF_8   = 8,
  MEMBUF_9   = 9,
  MEMBUF_10  = 10,
  MEMBUF_11  = 11,
  MEMBUF_12  = 12,
  MEMBUF_13  = 13,
  MEMBUF_14  = 14,
  MEMBUF_15  = 15,
};

//per slot/mba throttle values used for dimm/membuf control
typedef struct
{
    uint16_t max_n_per_mba;      //mode and OVS dependent, from config data
    uint16_t max_n_per_chip;     //mode and OVS dependent, from config data
    uint16_t min_n_per_mba;      //from config data
} memory_throttle_t;

// 128 bits encoding different bit fields corresponding to dimms
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

//Global is bitmask of membufs
extern uint32_t G_present_membufs;

//global bitmap of dimms that have ever gone over the error temperature
extern dimm_sensor_flags_t G_dimm_overtemp_bitmap;

//global bitmap of dimms temps that have been updated
extern dimm_sensor_flags_t G_dimm_temp_updated_bitmap;

//global bitmap flagging the dimms which we already calledout due to timeout (bitmap of dimms)
extern dimm_sensor_flags_t G_dimm_timeout_logged_bitmap;

//global bitmap flagging the membufs which we already calledout due to timeout (bitmap of membufs)
extern uint16_t G_membuf_timeout_logged_bitmap;

//global bitmap of membufs that have ever gone over the error temperature
extern uint16_t G_membuf_overtemp_bitmap;

//global bitmap of membuf temperatures that have been updated
extern uint16_t G_membuf_temp_updated_bitmap;


extern uint16_t G_configured_mbas;
extern dimm_sensor_flags_t G_dimm_enabled_sensors;

//AMEC needs to know when data for a membuf has been collected.
extern uint32_t G_updated_membuf_mask;

extern memory_control_task_t G_memory_control_task;


//Returns 0 if the specified membuf is not present. Otherwise, returns none-zero.
#define MEMBUF_PRESENT(occ_membuf_id) \
         ((MEMBUF0_PRESENT_MASK >> occ_membuf_id) & G_present_membufs)

//Returns 0 if the specified membuf is not updated. Otherwise, returns none-zero.
#define MEMBUF_UPDATED(occ_membuf_id) \
         ((MEMBUF0_PRESENT_MASK >> occ_membuf_id) & G_updated_membuf_mask)

//Returns 0 if the specified membuf is not updated. Otherwise, returns none-zero.
#define CLEAR_MEMBUF_UPDATED(occ_membuf_id) \
         G_updated_membuf_mask &= ~(MEMBUF0_PRESENT_MASK >> occ_membuf_id)


void task_memory_control( task_t * i_task );

void memory_init();

void disable_all_dimms();

//Collect membuf data for all membuf in specified range
void ocmb_data( void );

#endif // _MEMORY_H
