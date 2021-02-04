/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/proc/proc_pstate.h $                              */
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

#ifndef PROC_PSTATE_H
#define PROC_PSTATE_H

#include "ssx.h"
#include "cmdh_service_codes.h"
#include "errl.h"
#include "trac.h"
#include "rtls.h"
#include "occ_common.h"
#include "pstates_common.H"
#include "state.h"
#include "cmdh_fsp_cmds.h"
#include "cmdh_dbug_cmd.h"
//#include "gpsm.h"
//#include "pstates.h"

// The pstate associated with highest possible frequency
// is always 0 in POWER10.
#define PMAX 0

#define MAX_PARAM_DESCRIPTION_SZ  27 // including NULL

// Parameter IDs
#define PARAM_ID_MODE     0x01
#define PARAM_ID_IPS      0x02
#define PARAM_ID_FMIN     0x03
#define PARAM_ID_NOMINAL  0x04
#define PARAM_ID_BASE     0x05
#define PARAM_ID_SYS_MAX  0x06
#define PARAM_ID_CHIP_MAX 0x07

// Unique Parameter Value IDs
#define PARAM_VALUE_ID_NOT_SUPPORTED 0xE0

// Bit masks for parameter table flags byte
#define PARAM_FLAG_REPORT     0x80  // this parameter should be reported to the OS
#define PARAM_FLAG_DATA       0x40  // parameter value is in the param_data field
#define PARAM_FLAG_MASTER     0x20  // parameter only available from master OCC
#define PARAM_FLAG_FOLD       0x10  // enable processor folding


typedef enum
{
    PSTATES_IN_TRANSITION = -1,
    PSTATES_DISABLED      = 0,
    PSTATES_ENABLED       = 1,
    PSTATES_FAILED        = 2,
} pstateStatus;


typedef enum
{
    OPAL_STATIC = 0,
    OPAL_DYNAMIC = 1,
} opalDataType;


typedef struct __attribute__ ((packed))
{
    uint8_t              valid;
    uint8_t              major_version;
    uint8_t              occ_role;
    uint8_t              pmin;
    uint8_t              pfixed;
    uint8_t              pwof_base;
    uint8_t              puturbo;
    uint8_t              pfmax;
    uint8_t              minor_version;
    uint8_t              pthrottle;
    uint8_t              reserved[6];  // pad to 16 bytes

} opal_config_t;


typedef struct __attribute__ ((packed))
{
    uint8_t              pstate;
    uint8_t              valid;
    uint16_t             reserved;
    uint32_t             freq_khz;
} opal_pstate_data_t;

typedef struct __attribute__ ((packed))
{
    uint8_t              param_table_version;
    uint8_t              param_table_num_entries;
    uint8_t              reserved[6];
} param_table_header_t;

typedef struct __attribute__ ((packed))
{
    uint8_t              param_id;
    uint8_t              param_value_id;
    uint8_t              param_flags;
    uint16_t             param_data;
    char                 param_description[MAX_PARAM_DESCRIPTION_SZ];
} param_table_entry_t;

typedef struct __attribute__ ((packed))
{
    uint8_t              occ_state;
    uint8_t              dynamic_major_version;
    uint8_t              dynamic_minor_version;
    uint8_t              gpus_present;
    uint8_t              reserved[1];
    uint8_t              proc_throt_status;
    uint8_t              mem_throt_status;
    uint8_t              quick_power_drop;
    uint8_t              power_shift_ratio;
    uint8_t              power_cap_type;
    uint16_t             min_power_cap;
    uint16_t             max_power_cap;
    uint16_t             current_power_cap;
    uint16_t             soft_min_power_cap;
    uint8_t              current_power_mode; // PHYP only
    uint8_t              ips_status;         // PHYP only
} opal_dynamic_t;

// This size must be a multiple of 128
typedef struct __attribute__ ((packed))
{
    opal_dynamic_t       dynamic;       // Dynamic OPAL parameters: 20B
    uint8_t              pad[108];      // Reserved dynamic space: 108B
} opal_dynamic_table_t __attribute__ ((aligned (128)));

#define PSTATE_ENTRIES 256    // number of generated PSTATES entries in OPAL table
#define MAX_PARAM_TABLE_ENTRIES  25   // number of generated PSTATES entries in OPAL table
#define DYNAMIC_MINOR_VERSION 2  // Current Dynamic table data minor version 

// This size must be a multiple of 128
typedef struct __attribute__ ((packed))
{
    opal_config_t        config;                  // Static OPAL config parameters: 16B
    uint8_t              reserved[8];             // Reserved static space: 8B
    opal_pstate_data_t   pstates[PSTATE_ENTRIES]; // Generated Pstates Table: 2048B
    param_table_header_t param_header;            // Parameter Table Header: 8B
    param_table_entry_t  param_table[MAX_PARAM_TABLE_ENTRIES]; // Parameter table: 800B
    uint8_t              pad[64];                 // Padding in reserved static space for multiple 128B
} opal_static_table_t __attribute__ ((aligned (128)));


extern uint32_t    G_mhz_per_pstate;

extern uint8_t     G_desired_pstate;

extern opal_dynamic_table_t G_opal_dynamic_table;
extern opal_static_table_t G_opal_static_table;

// States for updating opal data in main memory
typedef enum
{
    OPAL_TABLE_UPDATE_IDLE = 0x00,            // No dynmaic table update in process
    OPAL_TABLE_UPDATE_DYNAMIC_COPY = 0x01,    // BCE scheduled to copy dynamic date to main memory
    OPAL_TABLE_UPDATE_NOTIFY_HOST = 0x02,     // BCE copy finished notify host
    OPAL_TABLE_UPDATE_BCE_FAIL = 0x03,        // BCE failed, retry
    OPAL_TABLE_UPDATE_CRITICAL_ERROR = 0x04,  // Critical BCE error, stop trying to update dynamic data
} OPAL_TABLE_UPDATE_STATE;

extern volatile uint8_t G_opal_table_update_state;

// Helper function to translate from Frequency to nearest Pstate
// and number of pstate steps into the throttle space
Pstate_t proc_freq2pstate(uint32_t i_freq_mhz, uint32_t *o_steps);

// Helper function to translate from Pstate to nearest Frequency
// and number of pstate steps into the throttle space
uint32_t proc_pstate2freq(Pstate_t i_pstate, uint32_t *o_steps);

// Helper function to determine if we are in HW Pstate mode
inline bool proc_is_hwpstate_enabled(void);

// Copy pstate data to opal table
void populate_opal_dynamic_data(void);

// Copy all opal static data to opal table
void populate_opal_static_data(void);

// Copy pstates sections of opal static data to opal table
void populate_opal_static_pstates_data(void);

// Copy config section of  opal static data to opal table
void populate_opal_static_config_data(void);

// Copy parameter table of opal static data to opal table
void populate_parameter_table_data(void);

// Copy opal static/dynamic table to mainstore memory at OPAL_OFFSET_HOMER
void populate_opal_tbl_to_mem(opalDataType opal_data_type);

// Check if opal table needs update
void check_for_opal_updates(void);

// update dynamic opal data in SRAM & main memory
void update_dynamic_opal_data (void);

void proc_pstate_kvm_setup(void);

#endif
