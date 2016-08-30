/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/cent/centaur_control.h $                          */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015                             */
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

#ifndef _CENTAUR_CONTROL_H
#define _CENTAUR_CONTROL_H

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

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Globals
//*************************************************************************

//Global memory structures used for centaur task data pointers
extern memory_control_task_t G_memory_control_task;

//*************************************************************************
// Function Prototypes
//*************************************************************************

//Collect centaur data for all centaur in specified range
void centaur_control( task_t* i_task );

//Initialize structures for collecting centaur data.
//void centaur_control_init( void ) INIT_SECTION;
void centaur_control_init( void );

bool check_centaur_checkstop(uint8_t cent);

#endif //_CENTAUR_CONTROL_H
