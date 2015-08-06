/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/proc/proc_data.h $                                */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2015                        */
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
//#include "gpe_data.h"

//Returns 0 if the specified core is not present. Otherwise, returns none-zero.
#define CORE_PRESENT(occ_core_id) \
         ((CORE0_PRESENT_MASK >> occ_core_id) & G_present_cores)

//Takes an OCC core id and converts it to a core id that
//can be used by the hardware. The caller needs to send in
//a valid core id. Since type is uchar so there is no need to check for
//case less than 0. If core id is invalid then returns unconfigured core 16.
#define CORE_OCC2HW(occ_core_id) \
        ((occ_core_id <= 15) ? G_occ2hw_core_id[occ_core_id] : 16)

//Takes a hardware core id and returns a OCC core id.
//The caller needs to send in a valid core id. Since type is uchar so
//there is no need to check for case less than 0. If core id
//is invalid then returns unconfigured core 16.
#define CORE_HW2OCC(hw_core_id) \
    ((hw_core_id <= 15) ? G_hw2occ_core_id[hw_core_id] : 16)

#define ALL_CORES_MASK          0xffff0000
#define CORE0_PRESENT_MASK      0x80000000ul
#define CORE0_PRESENT_MASK_GPE  0x8000000000000000ull

#define    MAX_NUM_HW_CORES    16
#define MAX_NUM_FW_CORES    12

#define    THREADS_PER_CORE    8
#define    MAX_MEM_PARTS        4

#define NUM_FAST_CORE_DATA_BUFF     2
#define NUM_CORE_DATA_BUFF       7
#define NUM_CORE_DATA_DOUBLE_BUF 2
#define NUM_CORE_DATA_EMPTY_BUF  1

#define LO_CORES_MASK       0x7e000000
#define HI_CORES_MASK       0x007e0000
#define HW_CORES_MASK       0xffff0000

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
};

// TEMP -- CoreData no longer exists
//typedef CoreData gpe_bulk_core_data_t;

//Processor data collect structures used for task data pointers
//gpe_req.request.parameter points to GpeGetCoreDataParms
// TEMP -- CoreData / PoreFlex objects no longer exist
/*
struct bulk_core_data_task {
        uint8_t start_core;
        uint8_t current_core;
        uint8_t end_core;
        gpe_bulk_core_data_t * core_data_ptr;
        PoreFlex gpe_req;
} __attribute__ ((__packed__));
typedef struct bulk_core_data_task bulk_core_data_task_t;
*/

//Only PCBS_LOCAL_PSTATE_FREQ_TARGET_STATUS_REG register is being
//collected at this time. Other register will be added when needed.
struct fast_core_data {
    uint64_t    pcbs_lpstate_freq_target_sr;
} __attribute__ ((__packed__));
typedef struct fast_core_data fast_core_data_t;

//gpe fast core data structure
struct gpe_fast_core_data {
    uint32_t    tod;
    uint32_t    reserved;
    fast_core_data_t core_data[MAX_NUM_HW_CORES];
} __attribute__ ((__packed__));
typedef struct gpe_fast_core_data gpe_fast_core_data_t;

//Global low and high cores structures used for task data pointers
// TEMP -- CoreData / PoreFlex objects no longer exist
//extern bulk_core_data_task_t G_low_cores;
//extern bulk_core_data_task_t G_high_cores;

//Global G_present_cores is bitmask of all OCC core numbering
extern uint32_t G_present_cores;

//Global G_present_hw_cores is bitmask of all hardware cores
extern uint32_t G_present_hw_cores;
extern uint8_t G_occ2hw_core_id[MAX_NUM_HW_CORES];
extern uint8_t G_hw2occ_core_id[MAX_NUM_HW_CORES];

//AMEC needs to know when data for a core has been collected.
extern uint32_t G_updated_core_mask;

// External reference to empath error mask
extern uint32_t G_empath_error_core_mask;

//Returns 0 if the specified core is not present. Otherwise, returns none-zero.
#define CORE_PRESENT(occ_core_id) \
         ((CORE0_PRESENT_MASK >> occ_core_id) & G_present_cores)

//Returns 0 if the specified core is not updated. Otherwise, returns none-zero.
#define CORE_UPDATED(occ_core_id) \
         ((CORE0_PRESENT_MASK >> occ_core_id) & G_updated_core_mask)

//Returns 0 if the specified core is not updated. Otherwise, returns none-zero.
#define CLEAR_CORE_UPDATED(occ_core_id) \
         G_updated_core_mask &= ~(CORE0_PRESENT_MASK >> occ_core_id)

// Evaluates to true if an empath collection error has occurred on a core
#define CORE_EMPATH_ERROR(occ_core_id) \
        ((CORE0_PRESENT_MASK >> occ_core_id) & G_empath_error_core_mask)

//Takes an OCC core id and converts it to a core id that
//can be used by the hardware. The caller needs to send in
//a valid core id. Since type is uchar so there is no need to check for
//case less than 0. If core id is invalid then returns unconfigured core 16.
#define CORE_OCC2HW(occ_core_id) \
        ((occ_core_id <= 15) ? G_occ2hw_core_id[occ_core_id] : 16)

//Takes a hardware core id and returns a OCC core id.
//The caller needs to send in a valid core id. Since type is uchar so
//there is no need to check for case less than 0. If core id
//is invalid then returns unconfigured core 16.
#define CORE_HW2OCC(hw_core_id) \
    ((hw_core_id <= 15) ? G_hw2occ_core_id[hw_core_id] : 16)

//Collect bulk core data for all cores in specified range
void task_core_data( task_t * i_task );

//Initialize structures for collecting core data.
void proc_core_init( void ) INIT_SECTION;

//Collect fast core data for all configured cores on every tick
void task_fast_core_data( task_t * i_task );

//Returns a pointer to the most up-to-date bulk core data for the core
//associated with the specified OCC core id.
// TEMP -- CoreData / PoreFlex objects no longer exist
//gpe_bulk_core_data_t * proc_get_bulk_core_data_ptr( const uint8_t i_occ_core_id );

//Returns a pointer to the most up-to-date fast core data
// TEMP -- CoreData / PoreFlex objects no longer exist
//gpe_fast_core_data_t * proc_get_fast_core_data_ptr( void );

#endif //_PROC_DATA_H
