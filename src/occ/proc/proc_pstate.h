/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/proc/proc_pstate.h $                                  */
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
#include "cmdhDbugCmd.h"
#include "appletManager.h"
#include "gpsm.h"
#include "pstates.h"

// GPSM DCM Synchronization - used by MBOX to transfer between DCM M & S
typedef struct
{
    uint8_t dcm_pair_id;
    uint8_t sync_state_master :4;
    uint8_t sync_state_slave :4;
    uint8_t pstate_v;
    uint8_t pstate_f;
} proc_gpsm_dcm_sync_occfw_t;

typedef struct __attribute__ ((packed))
{
    uint8_t              valid;
    uint8_t              version;
    // Sapphire_OCC_interface_v3.odp
    // throttle status 0x00 No throttle, 0x01 Powercap, 0x02 CPU overtemp, 0x03 power supply failure, 0x04 overcurrent, 0x05 OCC reset
    uint8_t              throttle;
    int8_t               pmin;
    int8_t               pnominal;
    int8_t               turbo;
    int8_t               ultraTurbo;
    uint8_t              spare;
} sapphire_config_t;

typedef struct __attribute__ ((packed))
{
    int8_t               pstate;
    uint8_t              flag;
    uint8_t              evid_vdd;
    uint8_t              evid_vcs;
    uint32_t             freq_khz;
} sapphire_data_t;

#define PSTATE_ENTRY_NUMBER 256
// This size must be a multiple of 128
// Version 0x02 of Sapphire data
typedef struct __attribute__ ((packed))
{
    sapphire_config_t    config;                            //8 bytes
    uint64_t             reserved;                          //8 bytes
    sapphire_data_t      data[PSTATE_ENTRY_NUMBER];         //256 * 8 = 2048 bytes
    int8_t               activeCore_max_pstate[MAX_CORES];  //One entry per core
    uint8_t              pad[100];                          //Whatever is needed to make size a multiple of 128
} sapphire_table_t __attribute__ ((aligned (128)));



typedef struct __attribute__ ((packed))
{
    uint8_t	valid;
    uint8_t	version;
    uint16_t	core_mask;
//System Sensors
    uint16_t	ambient_temperature;
    uint16_t	power;
    uint16_t	fan_power;
    uint16_t	io_power;
    uint16_t	storage_power;
    uint16_t	gpu_power;
    uint16_t	fan_speed;
//Processor Sensors
    uint16_t 	pwr250us;
    uint16_t 	pwr250usvdd;
    uint16_t	pwr250usvcs;
    uint16_t	pwr250usmem;
    uint64_t	chip_bw;
//Core sensors
    uint16_t	core_temp[12];
    uint64_t 	count;
    uint32_t	chip_energy;
    uint32_t	system_energy;
    uint8_t	pad[54];
} sapphire_sensor_t __attribute__ ((aligned (128)));

enum {
    NO_THROTTLE = 0x00,
    POWERCAP = 0x01,
    CPU_OVERTEMP = 0x02,
    POWER_SUPPLY_FAILURE = 0x03,
    OVERCURRENT = 0x04,
    OCC_RESET = 0x05,
    PCAP_ABOVE_TURBO = 0x06,           //Exceeded PCAP while at or above Turbo Pstate. For debug only.
    CPU_OT_ABOVE_TURBO = 0x07          //Processer over temperature at or above Turbo Pstate. For debug only.
};

extern GlobalPstateTable G_global_pstate_table;

extern uint32_t    G_mhz_per_pstate;

extern sapphire_table_t G_sapphire_table;

// Initialize PState Table
errlHndl_t proc_gpsm_pstate_initialize(const PstateSuperStructure* i_pss);

// Entry function for enabling Pstates once table is installed
void proc_gpsm_dcm_sync_enable_pstates_smh(void);

// Get DCM Sync State
proc_gpsm_dcm_sync_occfw_t proc_gpsm_dcm_sync_get_state(void);

// Pull down DCM pair's Sync State & Info via Mbox
void proc_gpsm_dcm_sync_update_from_mbox(proc_gpsm_dcm_sync_occfw_t * i_dcm_sync_state);

// Helper function to translate from Frequency to nearest Pstate
Pstate proc_freq2pstate(uint32_t i_freq_mhz);

// Helper function to translate from Pstate to nearest Frequency
uint32_t proc_pstate2freq(Pstate i_pstate);

// Helper function to determine if we are a DCM
inline bool proc_is_dcm();

// Helper function to determine if we are in HW Pstate mode
inline bool proc_is_hwpstate_enabled(void);

// Copy pstate data to sapphire table
void populate_pstate_to_sapphire_tbl();

// Copy sapphire table to mainstore memory at SAPPHIRE_OFFSET_IN_HOMER
void populate_sapphire_tbl_to_mem();

// Check if sapphire table needs update
void proc_check_for_sapphire_updates();
#endif
