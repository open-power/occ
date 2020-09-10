/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/mem/memory_data.c $                               */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2020                        */
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


#include <ssx.h>
#include <occhw_async.h>

#include <trac_interface.h>
#include <trac.h>
#include <occ_common.h>
#include <comp_ids.h>
#include <occ_service_codes.h>
#include <state.h>
#include <occ_sys_config.h>
#include "sensor.h"
#include "amec_sys.h"
#include "lock.h"
#include "common.h"
#include "memory.h"
#include "memory_data.h"
#include "amec_health.h"
#include "i2c.h"

extern bool G_mem_monitoring_allowed;
extern uint8_t G_read_ocmb_num_8ms_ticks;

// Function Specification
//
// Name:  task_memory_data
//
// Description: Called every 8ms (tick0 only) to collect memory temperatures
//
// Task Flags: RTL_FLAG_ACTIVE
//
// End Function Specification
void task_memory_data(struct task *i_self)
{
    static unsigned int L_dimms_enabled = false;
    static uint8_t L_num_8ms_ticks = 0;

    if (!L_dimms_enabled)
    {
        L_dimms_enabled = true;
        TRAC_INFO("task_memory_data: Memory temp collection is being started");
    }

    if(G_mem_monitoring_allowed)
    {
        L_num_8ms_ticks++;

        // check if it is time to read 1 OCMB
        if(L_num_8ms_ticks == G_read_ocmb_num_8ms_ticks)
        {
            L_num_8ms_ticks = 0;
            ocmb_data();
        }
    }

} // end task_memory_data()
