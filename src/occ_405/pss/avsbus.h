/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/pss/avsbus.h $                                    */
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

#ifndef _AVSBUS_H
#define _AVSBUS_H

#include <errl.h>

extern bool G_avsbus_vdd_monitoring;

#define AVSBUS_STATUS_OVER_CURRENT_MASK     0x4000
#define AVSBUS_STATUS_UNDER_VOLTAGE_MASK    0x2000
#define AVSBUS_STATUS_OVER_TEMPERATURE_MASK 0x1000
#define AVSBUS_STATUS_OVER_POWER_MASK       0x0800

#define AVSBUS_STATUS_ONGOING 0x80000000 // o2s_ongoing
#define AVSBUS_STATUS_ERRORS  0x05000000 // write_while_bridge_busy_error | FSM error

// this is only for types that OCC directly reads from AVSBUS currently only VDD (for temperature)
typedef enum
{
    AVSBUS_VDD = 0x00,
    AVSBUS_TYPE_MAX = 1 // Number of bus types
} avsbus_type_e;

// bitmask for types that PGPE is reading via AVSBUS and then OCC reads from OCC-PGPE shared memory
#define    AVSBUS_PGPE_VDD 0x01
#define    AVSBUS_PGPE_VCS 0x02
#define    AVSBUS_PGPE_VIO 0x04
#define    AVSBUS_PGPE_VDN 0x08

typedef enum
{
    // This enum contains the AVS Bus CmdDataType that can be read
    AVSBUS_TEMPERATURE      = 0x03,
    AVSBUS_STATUS           = 0x0E,
    AVSBUS_CMDS_MAX = 2 // Number of supported AVS bus commands
} avsbus_cmdtype_e;

// Setup the AVS Bus for reading
void avsbus_init();

// Initiate AVS Bus read for specified cmdtype (Temperature)
// (results can then be read on the next tick)
void initiate_avsbus_reads(avsbus_cmdtype_e i_cmdType);

// Initiate read for specified type (Vdd) and cmd (Temperature)
void avsbus_read_start(const avsbus_type_e i_type,
                       const avsbus_cmdtype_e i_cmdtype);

// Process AVS Bus read results (or errors) for specified bus/cmdtype.
// Returns the data requested (voltage units are mV, current units are in 10mA)
// Predictive error will be logged after MAX_READ_ATTEMPTS failures on the specific
// bus/cmdtype and an OCC reset will be requested
uint16_t avsbus_read(const avsbus_type_e i_type,
                     const avsbus_cmdtype_e i_cmdtype);

// Initiate read of AVS Bus Status
// (results can then be read on the next tick)
void initiate_avsbus_read_status();

// Read the status from AVS Bus
// Error history counters will be incremented for any over-current condition.
void process_avsbus_status();

// Calculate chip voltage and power from AVSbus readings and update sensors
void update_avsbus_power_sensors(const uint8_t i_type);

// Error history counters will be incremented for any over-current condition.
uint32_t clear_status_errors(const uint8_t i_bus, const uint32_t i_status_mask);
#endif //_AVSBUS_H
