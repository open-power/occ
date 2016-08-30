/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/cent/centaur_data.h $                                 */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2014,2015                        */
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

#ifndef _CENTAUR_DATA_H
#define _CENTAUR_DATA_H

//*************************************************************************
// Includes
//*************************************************************************
#include <occ_common.h>
#include <ssx.h>
#include "rtls.h"
//#include "gpe_data.h"
#include "occ_sys_config.h"
#include "memory.h"

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************

// Mask that is used by procedure to specify which centaurs are present
#define ALL_CENTAURS_MASK          0x000000ff
#define ALL_CENTAURS_MASK_GPE      0x000000ff00000000ull
// Centaur0, used by OCC
#define CENTAUR0_PRESENT_MASK      0x00000080ul
#define CENTAUR0_PRESENT_MASK_GPE  0x0000008000000000ull

// Used for specifing buffer allocations
#define NUM_CENTAUR_DATA_BUFF       MAX_NUM_CENTAURS
#define NUM_CENTAUR_DOUBLE_BUF      1
#define NUM_CENTAUR_DATA_EMPTY_BUF  1

// Specify a command BAR addresses for all centaur operations
#define CENTAUR_MEM_DATA_BAR        MEM_DATA_BAR_SELECT_1

// Enum for specifying each Centaur
enum eOccCentaurs
{
  CENTAUR_0  = 0,
  CENTAUR_1  = 1,
  CENTAUR_2  = 2,
  CENTAUR_3  = 3,
  CENTAUR_4  = 4,
  CENTAUR_5  = 5,
  CENTAUR_6  = 6,
  CENTAUR_7  = 7,
};

//*************************************************************************
// Macros
//*************************************************************************
//Returns 0 if the specified centaur is not present. Otherwise, returns none-zero.
#define CENTAUR_PRESENT(occ_cent_id) \
         ((CENTAUR0_PRESENT_MASK >> occ_cent_id) & G_present_centaurs)

//Returns 0 if the specified centaur is not updated. Otherwise, returns none-zero.
#define CENTAUR_UPDATED(occ_cent_id) \
         ((CENTAUR0_PRESENT_MASK >> occ_cent_id) & G_updated_centaur_mask)

//Returns 0 if the specified centaur is not updated. Otherwise, returns none-zero.
#define CLEAR_CENTAUR_UPDATED(occ_cent_id) \
         G_updated_centaur_mask &= ~(CENTAUR0_PRESENT_MASK >> occ_cent_id)

//Returns the bitmask for the passed in Centaur (uint32_t)
#define CENTAUR_BY_MASK(occ_cent_id) \
         ((CENTAUR0_PRESENT_MASK >> occ_cent_id) & ALL_CENTAURS_MASK)

//Returns the bitmask for the passed in Centaur (uint64_t)
#define CENTAUR_BY_MASK_GPE(occ_cent_id) \
         (((uint64_t) CENTAUR0_PRESENT_MASK_GPE >> occ_cent_id) & ALL_CENTAURS_MASK_GPE)

#define DIMM_SENSOR0 0x80

#define CENTAUR_SENSOR_ENABLED(occ_cent_id, sensor_num) \
        (G_dimm_enabled_sensors.bytes[occ_cent_id] & (DIMM_SENSOR0 >> (sensor_num)))

#define MBA_CONFIGURED(occ_cent_id, mba_num) \
        (G_configured_mbas & (1 << ((occ_cent_id * 2) + mba_num)))
//*************************************************************************
// Structures
//*************************************************************************

//Centaur data collect structures used for task data pointers
// TEMP -- PORE ISSUES
/*
struct centaur_data_task {
        uint8_t start_centaur;
        uint8_t current_centaur;
        uint8_t end_centaur;
        uint8_t prev_centaur;
        MemData * centaur_data_ptr;
        PoreFlex gpe_req;
} __attribute__ ((__packed__));
typedef struct centaur_data_task centaur_data_task_t;
*/

//*************************************************************************
// Globals
//*************************************************************************

//Global centaur structures used for task data pointers
// TEMP -- COMMENTED OUT DUE TO PORE ISSUES
//extern centaur_data_task_t G_centaur_data_task;

//Global is bitmask of centaurs
extern uint32_t G_present_centaurs;

//AMEC needs to know when data for a centaur has been collected.
extern uint32_t G_updated_centaur_mask;

//global bitmap of enabled dimm sensors
extern dimm_sensor_flags_t G_dimm_enabled_sensors;

//global bitmap of dimms that have ever gone over the error temperature
extern dimm_sensor_flags_t G_dimm_overtemp_bitmap;

//global bitmap of dimms temps that have been updated
extern dimm_sensor_flags_t G_dimm_temp_updated_bitmap;

//global bitmap flagging the dimms which we already calledout due to timeout (bitmap of dimms)
extern dimm_sensor_flags_t G_dimm_timeout_logged_bitmap;

//global bitmap flagging the centaurs which we already calledout due to timeout (bitmap of centaurs)
extern uint8_t G_cent_timeout_logged_bitmap;

//global bitmap of centaurs that have ever gone over the error temperature
extern uint8_t G_cent_overtemp_bitmap;

//global bitmap of centaur temperatures that have been updated
extern uint8_t G_cent_temp_updated_bitmap;

//bitmap of configured MBA's (2 per centaur, lsb is centaur0/mba0)
extern uint16_t G_configured_mbas;

//*************************************************************************
// Function Prototypes
//*************************************************************************

//Collect centaur data for all centaur in specified range
void task_centaur_data( task_t * i_task );

//Initialize structures for collecting centaur data.
//void centaur_init( void ) INIT_SECTION;
void centaur_init( void );

//handles centaur i2c recovery and other workarounds
void cent_recovery(uint32_t i_cent);

//Determines if a centaur has a channel checkstop (returns TRUE is it has a
//channel checkstop, FALSE otherwise)
bool cent_chan_checkstop(const uint8_t i_cent);

//Returns a pointer to the most up-to-date centaur data for the centaur
//associated with the specified OCC centaur id.
// TEMP -- WHERE IS MemData ??
//MemData * cent_get_centaur_data_ptr( const uint8_t i_centaur_id );

#endif //_CENTAUR_DATA_H

