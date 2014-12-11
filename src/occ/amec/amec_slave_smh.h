/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/amec/amec_slave_smh.h $                               */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2014                        */
/* [+] Google Inc.                                                        */
/* [+] International Business Machines Corp.                              */
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

//*************************************************************************
// Function Prototypes
//*************************************************************************
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

// Slave SubState 6
void amec_slv_substate_6_0(void);
void amec_slv_substate_6_1(void);
void amec_slv_substate_6_2(void);
void amec_slv_substate_6_3(void);
void amec_slv_substate_6_4(void);
void amec_slv_substate_6_5(void);
void amec_slv_substate_6_6(void);
void amec_slv_substate_6_7(void);

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
