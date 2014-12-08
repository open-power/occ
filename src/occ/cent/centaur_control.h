/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/cent/centaur_control.h $                              */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* COPYRIGHT International Business Machines Corp. 2011,2014              */
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

#ifndef _CENTAUR_CONTROL_H
#define _CENTAUR_CONTROL_H

//*************************************************************************
// Includes
//*************************************************************************
#include <occ_common.h>
#include <ssx.h>
#include "rtls.h"
#include "gpe_data.h"
#include "occ_sys_config.h"

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Structures
//*************************************************************************

typedef enum
{
    CENTAUR_CONTROL_GPE_STILL_RUNNING = 0x01,
    CENTAUR_CONTROL_RESERVED_1        = 0x02,
    CENTAUR_CONTROL_RESERVED_2        = 0x04,
    CENTAUR_CONTROL_RESERVED_3        = 0x08,
    CENTAUR_CONTROL_RESERVED_4        = 0x10,
    CENTAUR_CONTROL_RESERVED_5        = 0x20,
    CENTAUR_CONTROL_RESERVED_6        = 0x40,
    CENTAUR_CONTROL_RESERVED_7        = 0x80,
} eCentaurControlTraceFlags;

//Centaur data collect structures used for task data pointers
struct centaur_control_task {
        uint8_t  startCentaur;
        uint8_t  prevCentaur;
        uint8_t  curCentaur;
        uint8_t  endCentaur;
        uint8_t  traceThresholdFlags;
        PoreFlex gpe_req;
} __attribute__ ((__packed__));
typedef struct centaur_control_task centaur_control_task_t;

//per mba throttle values
typedef struct
{
   uint16_t max_n_per_mba;      //mode and OVS dependent, from config data
   uint16_t max_n_per_chip;     //mode and OVS dependent, from config data
   uint16_t min_n_per_mba;      //from config data
   uint16_t m;
} centaur_throttle_t;

//*************************************************************************
// Globals
//*************************************************************************

//Global centaur structures used for task data pointers
extern centaur_control_task_t G_centaur_control_task;

//*************************************************************************
// Function Prototypes
//*************************************************************************

//Collect centaur data for all centaur in specified range
void task_centaur_control( task_t * i_task );

//Initialize structures for collecting centaur data.
//void centaur_control_init( void ) INIT_SECTION;
void centaur_control_init( void );

#endif //_CENTAUR_CONTROL_H

