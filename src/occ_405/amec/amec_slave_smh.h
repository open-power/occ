/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/amec/amec_slave_smh.h $                               */
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

#ifndef _AMEC_SLAVE_SMH_H
#define _AMEC_SLAVE_SMH_H

//*************************************************************************
// Includes
//*************************************************************************
#include <occ_common.h>
#include <ssx.h>
#include <ssx_app_cfg.h>
#include <amec_smh.h>
#include <amec_amester.h>

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************
#define AMEC_SLV_STATE()                AMEC_STATE(&G_amec_slv_state);
#define AMEC_SLV_SUBSTATE()             AMEC_SUBSTATE(&G_amec_slv_state);
#define AMEC_SLV_SUB_SUBSTATE()         AMEC_SUB_SUBSTATE(&G_amec_slv_state);

#define AMEC_SLV_STATE_NEXT()           AMEC_STATE_NEXT(&G_amec_slv_state);
#define AMEC_SLV_SUBSTATE_NEXT()        AMEC_SUBSTATE_NEXT(&G_amec_slv_state);
#define AMEC_SLV_SUB_SUBSTATE_NEXT()    AMEC_SUB_SUBSTATE_NEXT(&G_amec_slv_state);

//*************************************************************************
// Structures
//*************************************************************************

//*************************************************************************
// Globals
//*************************************************************************
extern const smh_tbl_t amec_slv_state_table[AMEC_SMH_STATES_PER_LVL];
extern smh_state_t G_amec_slv_state;
extern smh_state_timing_t G_amec_slv_state_timings;
extern bool G_apss_lower_pmax_rail;

//*************************************************************************
// Function Prototypes
//*************************************************************************
void amec_slv_check_apss_fail(void);

void amec_slv_update_main_mem_sensors(void);

void amec_update_proc_core_group(uint8_t);

// PRE: slave common tasks
void amec_slv_common_tasks_pre(void);
// POST: slave common tasks
void amec_slv_common_tasks_post(void);

// Slave States
void amec_slv_state_0(void);
void amec_slv_state_1(void);
void amec_slv_state_2(void);
void amec_slv_state_3(void);
void amec_slv_state_4(void);
void amec_slv_state_5(void);
void amec_slv_state_6(void);
void amec_slv_state_7(void);

// Slave SubState 1
void amec_slv_substate_1_0(void);
void amec_slv_substate_1_1(void);
void amec_slv_substate_1_2(void);
void amec_slv_substate_1_3(void);
void amec_slv_substate_1_4(void);
void amec_slv_substate_1_5(void);
void amec_slv_substate_1_6(void);
void amec_slv_substate_1_7(void);

// Slave SubState 2 (odd SubStates currently unused)
void amec_slv_substate_2_even(void);

// Slave SubState 3
void amec_slv_substate_3_0(void);
void amec_slv_substate_3_1(void);
void amec_slv_substate_3_2(void);
void amec_slv_substate_3_3(void);
void amec_slv_substate_3_4(void);
void amec_slv_substate_3_5(void);
void amec_slv_substate_3_6(void);
void amec_slv_substate_3_7(void);

// Slave SubState 5
void amec_slv_substate_5_0(void);
void amec_slv_substate_5_1(void);
void amec_slv_substate_5_2(void);
void amec_slv_substate_5_3(void);
void amec_slv_substate_5_4(void);
void amec_slv_substate_5_5(void);
void amec_slv_substate_5_6(void);
void amec_slv_substate_5_7(void);

// Slave SubState 6 called every SubState - check for inband cmd every 4ms
void amec_slv_substate_6_all(void);

// Slave SubState 7
void amec_slv_substate_7_0(void);
void amec_slv_substate_7_1(void);
void amec_slv_substate_7_2(void);
void amec_slv_substate_7_3(void);
void amec_slv_substate_7_4(void);
void amec_slv_substate_7_5(void);
void amec_slv_substate_7_6(void);
void amec_slv_substate_7_7(void);

#endif
