/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/amec/amec_wof.h $                                     */
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

#ifndef _AMEC_WOF_H
#define _AMEC_WOF_H

//*************************************************************************
// Includes
//*************************************************************************
#include <sensor.h>
#include <occ_common.h>
#include <occ_sys_config.h> //MAX_NUM_CORES
#include <ssx_api.h> // SsxSemaphore
#include <pstates.h> // GlobalPstateTable

//*************************************************************************
// Externs
//*************************************************************************

//WOF parameters defined in amec_wof.c
extern sensor_t g_amec_wof_ceff_ratio_sensor;
extern sensor_t g_amec_wof_core_wake_sensor;
extern sensor_t g_amec_wof_vdd_sense_sensor;
extern uint64_t g_amec_wof_wake_mask;
extern uint64_t g_amec_wof_wake_mask_save;
extern uint8_t g_amec_wof_make_check;
extern uint8_t g_amec_wof_check;
extern GlobalPstateTable g_amec_wof_pstate_table_0;
extern GlobalPstateTable g_amec_wof_pstate_table_1;
extern uint8_t g_amec_wof_current_pstate_table;
extern uint8_t g_amec_wof_pstate_table_ready;
extern uint16_t G_amec_wof_thread_counter;
extern SsxSemaphore G_amecWOFThreadWakeupSem;

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************
typedef enum
{
    AMEC_WOF_ERROR_NONE,
    AMEC_WOF_ERROR_SCOM_1,
    AMEC_WOF_ERROR_SCOM_2,
    AMEC_WOF_ERROR_SCOM_3,
    AMEC_WOF_ERROR_SCOM_4,
    AMEC_WOF_ERROR_CORE_COUNT,
    AMEC_WOF_ERROR_UNKNOWN_STATE
} AMEC_WOF_ERROR_ENUM;

//*************************************************************************
// Structures
//*************************************************************************

//Structure used in g_amec
typedef struct amec_wof
{
    // Total loadline resistance in micro-ohm (R_ll + R_drop)
    uint16_t            loadline;
    // Vdd regulator efficiency in 0.01% units
    uint16_t            vdd_eff;
    // Chip Vdd current in 0.01 A (out of regulator)
    uint16_t            cur_out;
    // Last Vdd current accumulator to compute 2ms average
    uint32_t            cur_out_last;
    // Voltage at chip silicon (Vreg - V_loadline_droop)
    uint16_t            v_chip;
    // First index into iddq table for interpolation
    uint8_t             iddq_i;
    // Check interpolation of iddq table
    uint16_t            iddq85c;
    // I_DC_extracted is the estimated temperature-corrected leakage current
    uint16_t            iddq;
    // I_AC extracted
    uint16_t            ac;
    // Effective capacitance for TDP workload @ Turbo.
    uint32_t            ceff_tdp;
    // Effective capacitance right now.
    uint32_t            ceff;
    // Effective capacitance old.
    uint32_t            ceff_old;
    // Effective capacitance ratio
    uint16_t            ceff_ratio;
    // Uplift frequency adjustment
    int16_t             f_uplift;
    // Frequency vote. Lowest vote, until WOF is initialized with safe Turbo freq.
    uint16_t            f_vote;
    // Voltage set at regulator associated with wof vote
    uint16_t            vote_vreg;
    // Voltage at chip associated with wof vote at present current.
    uint16_t            vote_vchip;
    // Non-zero is a WOF error flag
    uint8_t             error;
    // User changes the WOF algorithm (and enable/disable) by setting
    // g_amec->wof.enable_parm from the Amester parameter interface.
    // OCC will check this against the current setting (g_amec->wof.enable) and
    // within 250us, do initialization for the next setting and start the new WOF algorithm.
    // Parameter-set next state: 1=WOF runs. 0=WOF selects highest frequency.
    uint8_t             enable_parm;
    // Current state: 1=WOF runs. 0=WOF selects highest frequency.
    //                0xFF=invalid (will cause init of enable_parm setting)
    uint8_t             enable;
    // Count number of cores on
    uint8_t             cores_on;
    // WOF state
    uint8_t             state;
    // pmstate for debugging
    uint8_t             pm_state[MAX_NUM_CORES];

} amec_wof_t;

//*************************************************************************
// Globals
//*************************************************************************

//*************************************************************************
// Function Prototypes
//*************************************************************************
void amec_wof_main(void);
void amec_wof_helper(void);
void amec_wof_init(void) INIT_SECTION;
void amec_update_wof_sensors(void);


#endif
