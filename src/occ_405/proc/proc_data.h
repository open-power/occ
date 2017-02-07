/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/proc/proc_data.h $                                */
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

#ifndef _PROC_DATA_H
#define _PROC_DATA_H

#include <occ_common.h>
#include <ssx.h>
#include "rtls.h"
#include "proc_shared.h"

//Returns 0 if the specified core is not present. Otherwise, returns none-zero.
#define CORE_PRESENT(occ_core_id) \
         ((CORE0_PRESENT_MASK >> occ_core_id) & G_present_cores)

#define ALL_CORES_MASK          0xffffff00
#define CORE0_PRESENT_MASK      0x80000000ul
#define CORE0_PRESENT_MASK_GPE  0x8000000000000000ull

#define MAX_NUM_HW_CORES    24
#define MAX_NUM_FW_CORES    24

#define CORE_MID_POINT (MAX_NUM_FW_CORES / 2)

#define THREADS_PER_CORE    4

#define NUM_CORE_DATA_BUFF       7
#define NUM_CORE_DATA_DOUBLE_BUF 2
#define NUM_CORE_DATA_EMPTY_BUF  1

#define LO_CORES_MASK       0xfff00000
#define HI_CORES_MASK       0x000fff00
#define HW_CORES_MASK       0xffffff00

enum eOccProcCores
{
  CORE_0  = 0,
  CORE_1  = 1,
  CORE_2  = 2,
  CORE_3  = 3,
  CORE_4  = 4,
  CORE_5  = 5,
  CORE_6  = 6,
  CORE_7  = 7,
  CORE_8  = 8,
  CORE_9  = 9,
  CORE_10 = 10,
  CORE_11 = 11,
  CORE_12 = 12,
  CORE_13 = 13,
  CORE_14 = 14,
  CORE_15 = 15,
  CORE_16 = 16,
  CORE_17 = 17,
  CORE_18 = 18,
  CORE_19 = 19,
  CORE_20 = 20,
  CORE_21 = 21,
  CORE_22 = 22,
  CORE_23 = 23,
};

//Processor data collect structures used for task data pointers
//gpe_req.cmd_data points to ipc_core_data_parms_t
struct bulk_core_data_task {
        uint8_t start_core;
        uint8_t current_core;
        uint8_t end_core;
        CoreData * core_data_ptr;
        GpeRequest gpe_req;
} __attribute__ ((__packed__));
typedef struct bulk_core_data_task bulk_core_data_task_t;

//Global low and high cores structures used for task data pointers
extern bulk_core_data_task_t G_low_cores;
extern bulk_core_data_task_t G_high_cores;

//Global G_present_cores is bitmask of all OCC core numbering
extern uint32_t G_present_cores;

//AMEC needs to know when data for a core has been collected.
extern uint32_t G_updated_core_mask;

// External reference to empath error mask
extern uint32_t G_empath_error_core_mask;

extern bool G_nest_dts_data_valid;

//Returns 0 if the specified core is not updated. Otherwise, returns none-zero.
#define CORE_UPDATED(occ_core_id) \
         ((CORE0_PRESENT_MASK >> occ_core_id) & G_updated_core_mask)

//Returns 0 if the specified core is not updated. Otherwise, returns none-zero.
#define CLEAR_CORE_UPDATED(occ_core_id) \
         G_updated_core_mask &= ~(CORE0_PRESENT_MASK >> occ_core_id)

// Evaluates to true if an empath collection error has occurred on a core
#define CORE_EMPATH_ERROR(occ_core_id) \
        ((CORE0_PRESENT_MASK >> occ_core_id) & G_empath_error_core_mask)

//Collect bulk core data for all cores in specified range
void task_core_data( task_t * i_task );

//Initialize structures for collecting core data.
void proc_core_init( void ) INIT_SECTION;

// Collect nest dts temperature sensors
void task_nest_dts( task_t * i_task );

// Initialize structures for collecting nest dts temps
void nest_dts_init( void ) INIT_SECTION;

//Returns a pointer to the most up-to-date bulk core data for the core
//associated with the specified OCC core id.
CoreData * proc_get_bulk_core_data_ptr( const uint8_t i_occ_core_id );

// 24x7 data collection
void task_24x7( task_t * i_task );


#endif //_PROC_DATA_H
