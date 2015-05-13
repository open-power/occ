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

// Vdd regulator efficiency in 0.01% units
extern uint16_t g_amec_wof_vdd_eff;
// Total loadline resistance in micro-ohm (R_ll + R_drop)
extern uint16_t g_amec_wof_loadline;
// chip Vdd current in 0.01 A (out of regulator) @cl020
extern uint16_t g_amec_wof_cur_out;
// Voltage at chip silicon (Vreg - V_loadline_droop)
extern uint16_t g_amec_wof_v_chip;
// first index into iddq table for interpolation
extern uint8_t g_amec_wof_iddq_i;
// check interpolation of iddq table
extern uint16_t g_amec_wof_iddq85c;
// I_DC_extracted is the estimated temperature-corrected leakage current
extern uint16_t g_amec_wof_iddq;
// I_AC_extracted
extern uint16_t g_amec_wof_ac;
extern uint32_t g_amec_wof_ceff_tdp;
extern uint32_t g_amec_wof_ceff;
extern uint32_t g_amec_wof_ceff_old;
extern uint16_t g_amec_wof_ceff_ratio;
extern int16_t g_amec_wof_f_uplift; // uplift frequency adjustment
extern uint16_t g_amec_wof_f_vote; // frequency vote
extern uint16_t g_amec_wof_vote_vreg; // voltage associated with wof vote
// voltage at chip associated with wof vote at present current.
extern uint16_t g_amec_wof_vote_vchip;
extern uint8_t g_amec_wof_error; // non-zero is a WOF error flag
// User changes the WOF algorithm (and enable/disable) by setting
// g_amec_wof_enable_parm from the Amester parameter interface.  OCC
// will check this against the current setting (g_amec_wof_enable) and
// within 250us, do initialization for the next setting and start the
// new WOF algorithm.
// parameter-set next state: 0=No WOF, 1 or higher = WOF algorithm.
// 0xff is uninitialized state.
extern uint8_t g_amec_wof_enable_parm;
extern uint8_t g_amec_wof_cores_on; // Count number of cores on
extern uint8_t g_amec_wof_state; // WOF state
extern sensor_t g_amec_wof_ceff_ratio_sensor;
extern sensor_t g_amec_wof_core_wake_sensor;
extern sensor_t g_amec_wof_vdd_sense_sensor;
extern uint64_t g_amec_wof_wake_mask;
extern uint64_t g_amec_wof_wake_mask_save;
extern uint8_t g_amec_wof_pm_state[MAX_NUM_CORES];
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

//*************************************************************************
// Structures
//*************************************************************************

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
