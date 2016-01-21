/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/dimm/dimm.h $                                     */
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

#ifndef _DIMM_H
#define _DIMM_H

#include <occ_common.h>
#include <trac_interface.h>
#include <errl.h>
#include <rtls.h>

#define NUM_DIMM_PORTS           2

#define DIMM_TICK (CURRENT_TICK % MAX_NUM_TICKS)

typedef enum
{
    DIMM_READ_SUCCESS =             0x00,
    DIMM_READ_IN_PROGRESS =         0x01,
    DIMM_READ_SCHEDULE_FAILED =     0xFA,
    DIMM_READ_NOT_COMPLETE =        0xFF,
} readStatus_e;

typedef enum
{
    PIB_I2C_ENGINE_C = 0x01,
    PIB_I2C_ENGINE_D = 0x02,
    PIB_I2C_ENGINE_E = 0x03,
    PIB_I2C_ENGINE_ALL = 0xFF,
} PIB_I2C_ENGINE;

typedef enum
{
    MEM_TYPE_CUMULUS = 0x00,
    MEM_TYPE_NIMBUS  = 0xFF
} MEMORY_TYPE;

// Generic memory initialization to handle init for all memory types
void memory_init();

// Nimbus specific init
void memory_nimbus_init();

// DIMM I2C state machine (for NIMBUS)
void task_dimm_sm(struct task *i_self);

#endif //_DIMM_H
