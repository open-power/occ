/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/proc/proc_pstate.h $                              */
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

#ifndef PROC_PSTATE_H
#define PROC_PSTATE_H

#include "ssx.h"
#include "cmdh_service_codes.h"
#include "errl.h"
#include "trac.h"
#include "rtls.h"
#include "occ_common.h"
#include "state.h"
#include "cmdh_fsp_cmds.h"
#include "cmdh_dbug_cmd.h"
//#include "gpsm.h"
//#include "pstates.h"

typedef struct __attribute__ ((packed))
{
    uint8_t              valid;
    uint8_t              version;
    // Sapphire_OCC_interface_v3.odp
    // throttle status 0x00 No throttle, 0x01 Powercap, 0x02 CPU overtemp, 0x03 power supply failure, 0x04 overcurrent, 0x05 OCC reset
    uint8_t              throttle;
    int8_t               pmin;
    int8_t               pnominal;
    int8_t               pmax;
    uint16_t             spare;
} opal_config_t;

typedef struct __attribute__ ((packed))
{
    int8_t               pstate;
    uint8_t              flag;
    uint8_t              evid_vdd;
    uint8_t              evid_vcs;
    uint32_t             freq_khz;
} opal_data_t;

#define PSTATE_ENTRY_NUMBER 256
// This size must be a multiple of 128
typedef struct __attribute__ ((packed))
{
    opal_config_t        config;
    uint64_t             reserved;
    opal_data_t          data[PSTATE_ENTRY_NUMBER];
    uint8_t              pad[112];
} opal_table_t __attribute__ ((aligned (128)));

enum {
    NO_THROTTLE = 0x00,
    POWERCAP = 0x01,
    CPU_OVERTEMP = 0x02,
    POWER_SUPPLY_FAILURE = 0x03,
    OVERCURRENT = 0x04,
    OCC_RESET = 0x05,
};

extern uint32_t    G_mhz_per_pstate;

extern opal_table_t G_opal_table;

// Initialize PState Key parameters
void proc_pstate_initialize(void);

// Helper function to translate from Frequency to nearest Pstate
Pstate proc_freq2pstate(uint32_t i_freq_mhz);

// Helper function to translate from Pstate to nearest Frequency
// TEMP -- Pstate no longer exists
//uint32_t proc_pstate2freq(Pstate i_pstate);

// Helper function to determine if we are in HW Pstate mode
inline bool proc_is_hwpstate_enabled(void);

// Copy pstate data to opal table
void populate_pstate_to_opal_tbl();

// Copy opal table to mainstore memory at OPAL_OFFSET_HOMER
void populate_opal_tbl_to_mem();

// Check if opal table needs update
void proc_check_for_opal_updates();
#endif
