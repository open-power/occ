/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/dimm/dimm.h $                                     */
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

#ifndef _DIMM_H
#define _DIMM_H

#include <occ_common.h>
#include <trac_interface.h>
#include <errl.h>
#include <rtls.h>

// If DIMM has huid/sensor then it is present
#define NIMBUS_DIMM_PRESENT(port,dimm) (0 != G_sysConfigData.dimm_huids[port][dimm])

// A bit vector that indicated the dimms (mbas) configured in a system.
// Bit 0 (MSB) encodes the configuration status for port 0 on MC01
// (or mba01 on centaur 0) a 1 indicated a dimm (mba) is configured
// for throttling. Only most significant 8 bits used in Nimbus.
extern uint16_t G_configured_mbas;

// DIMM Throttle Limits have been configured?
#define NIMBUS_DIMM_THROTTLING_CONFIGURED(vector,mc,port) \
    (0 != (vector & (0x8000 >> ((port) + ((mc) * (MAX_NUM_MCU_PORTS))))))

#define NIMBUS_DIMM_INDEX_THROTTLING_CONFIGURED(memIndex) \
    (0 != (G_configured_mbas & (0x8000 >> memIndex)))

#define NIMBUS_DIMM_PORT_THROTTLING_CONFIGURED(mc,port)            \
    (0 != (G_configured_mbas & (0x8000 >> ((mc) * (MAX_NUM_MCU_PORTS)))))

// Configure NIMBUS_DIMM Throttling data on MC01/MC02's (0/1) port (0-3)
#define CONFIGURE_NIMBUS_DIMM_THROTTLING(vector,mc,port)  \
    vector |= (0x8000 >> ((port) + (mc) * (MAX_NUM_MCU_PORTS)))


#define NUM_DIMM_PORTS           2
// On Nimbus, we are using the centaur number as the I2C port (keep same structure)
// There can be 8 DIMMs under a Centaur and 8 DIMMs per I2C port (max of 2 ports)
// DIMM code assumed that NUM_DIMMS_PER_I2CPORT == NUM_DIMMS_PER_CENTAUR
#define NUM_DIMMS_PER_I2CPORT           8


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
    MEM_TYPE_UNKNOWN = 0x00,
    MEM_TYPE_CUMULUS = 0xCC,
    MEM_TYPE_NIMBUS  = 0xFF   // Nimbus must be 0xFF per interface with (H)TMGT
} MEMORY_TYPE;

// Generic memory initialization to handle init for all memory types
void memory_init();

// Nimbus specific init
void memory_nimbus_init();

// DIMM I2C state machine (for NIMBUS)
void task_dimm_sm(struct task *i_self);

#endif //_DIMM_H
