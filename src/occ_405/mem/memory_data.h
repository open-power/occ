/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/mem/memory_data.h $                               */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2019                        */
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

#ifndef _MEMORY_DATA_H
#define _MEMORY_DATA_H

#include <occ_common.h>
#include <trac_interface.h>
#include <errl.h>
#include <rtls.h>
#include <ocmb_mem_data.h>
#include <memory.h>


/// This code is established in the RC field prior to collecting the memory buffer
/// sensor cache data. If this RC is observed on a hard failure it most likely
/// indicates an error assiciated with the memory buffer whose data was being
/// collected.
#define MEMBUF_GET_MEM_DATA_SENSOR_CACHE_FAILED 6

/// This code is established in the RC field prior to "poking" the memory buffer (if
/// any) that is being updated this pass. If this RC is observed on a hard
/// failure it most likely indicates an error associated with the memory buffer
/// being updated.
#define MEMBUF_GET_MEM_DATA_UPDATE_FAILED 7

#define MEMBUF_SENSOR_ENABLED(occ_membuf_id, sensor_num) \
        (G_dimm_enabled_sensors.bytes[occ_membuf_id] & (DIMM_SENSOR0 >> (sensor_num)))

#define MBA_CONFIGURED(occ_membuf_id) \
        (G_configured_mbas & (1 << (occ_membuf_id)))


// Generic memory initialization to handle init for all memory types
void memory_init();

// Memory temperature collection task
void task_memory_data(struct task *i_self);

// Returns a pointer to the most up-to-date data for the membuf associated with the specified id.
OcmbMemData * get_membuf_data_ptr( const uint8_t i_membuf_id );

#endif //_MEMORY_DATA_H
