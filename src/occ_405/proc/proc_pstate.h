/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/proc/proc_pstate.h $                              */
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

#ifndef PROC_PSTATE_H
#define PROC_PSTATE_H

#include "ssx.h"
#include "cmdh_service_codes.h"
#include "errl.h"
#include "trac.h"
#include "rtls.h"
#include "occ_common.h"
#include "p9_pstates_common.h"
#include "state.h"
#include "cmdh_fsp_cmds.h"
#include "cmdh_dbug_cmd.h"
//#include "gpsm.h"
//#include "pstates.h"

// The pstate associated with highest possible frequency
// is always 0 in POWER9.
#define PMAX 0

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
    uint8_t              version;
    uint8_t              occ_role;
    uint8_t              pmin;
    uint8_t              pnominal;
    uint8_t              pturbo;
    uint8_t              puturbo;
    uint8_t              reserved;

} opal_config_t;


typedef struct __attribute__ ((packed))
{
    int8_t               pstate;
    uint8_t              flag;
    uint16_t             reserved;
    uint32_t             freq_khz;
} opal_pstate_data_t;


typedef struct __attribute__ ((packed))
{
    uint8_t              occ_state;
    uint8_t              reserved[4];
    uint8_t              proc_throt_status;
    uint8_t              mem_throt_status;
    uint8_t              quick_power_drop;
    uint8_t              power_shift_ratio;
    uint8_t              power_cap_type;
    uint16_t             min_power_cap;
    uint16_t             max_power_cap;
    uint16_t             current_power_cap;

} opal_dynamic_t;

// This size must be a multiple of 128
typedef struct __attribute__ ((packed))
{
    opal_dynamic_t       dynamic;       // Dynamic OPAL parameters: 16B
    uint8_t              pad[112];      // Reserved dynamic space: 112B
} opal_dynamic_table_t __attribute__ ((aligned (128)));

#define PSTATE_ENTRIES 256    // number of generated PSTATES entries in OPAL table

// This size must be a multiple of 128
typedef struct __attribute__ ((packed))
{
    opal_config_t        config;                  // Static OPAL config parameters: 8B
    uint8_t              reserved[16];            // Reserved static space: 16B
    opal_pstate_data_t   pstates[PSTATE_ENTRIES]; // Generated Pstates Table: 2048B
    uint8_t              max_pstate[24];          // Maximum Pstate with N active cores is max_pstate[N-1]: 24B
    uint8_t              pad[80];                 // Padding in reserved static space: 80B
} opal_static_table_t __attribute__ ((aligned (128)));


extern uint32_t    G_mhz_per_pstate;

// Per quad pstate
extern uint8_t     G_desired_pstate[MAXIMUM_QUADS];

extern opal_dynamic_table_t G_opal_dynamic_table;
extern opal_static_table_t G_opal_static_table;

// Helper function to translate from Frequency to nearest Pstate
Pstate proc_freq2pstate(uint32_t i_freq_mhz);

// Helper function to translate from Pstate to nearest Frequency
uint32_t proc_pstate2freq(Pstate i_pstate);

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

// Copy opal static/dynamic table to mainstore memory at OPAL_OFFSET_HOMER
void populate_opal_tbl_to_mem(opalDataType opal_data_type);

// Check if opal table needs update
void check_for_opal_updates(void);

// update dynamic opal data in SRAM & main memory
void update_dynamic_opal_data (void);

void proc_pstate_kvm_setup(void);

#endif
