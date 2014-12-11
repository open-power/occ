/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/amec/amec_smh.h $                                     */
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

#ifndef _AMEC_SMH_H
#define _AMEC_SMH_H

//*************************************************************************
// Includes
//*************************************************************************
#include <occ_common.h>
#include <ssx.h>
#include <ssx_app_cfg.h>
#include "amec_external.h"

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************

#define AMEC_STATE(i_state)             (i_state->state);
#define AMEC_SUBSTATE(i_state)          (i_state->substate);
#define AMEC_SUB_SUBSTATE(i_state)      (i_state->sub_substate);

#define AMEC_STATE_NEXT(i_state)        i_state->state++; i_state->state %= AMEC_SMH_STATES_PER_LVL;
#define AMEC_SUBSTATE_NEXT(i_state)     i_state->substate++; i_state->substate %= AMEC_SMH_STATES_PER_LVL;
#define AMEC_SUB_SUBSTATE_NEXT(i_state) i_state->sub_substate++; i_state->sub_substate %= AMEC_SMH_STATES_PER_LVL;

#define AMEC_INITIAL_STATE      0

#define AMEC_SMH_STATES_PER_LVL 8

// Number of uS in 1 RTL tick (250=250us)
#define AMEC_US_PER_TICK         MICS_PER_TICK
// Number of uS in 1 full period of the AMEC State Machine (2000=2mS, 8 RTL ticks)
#define AMEC_US_PER_SMH_PERIOD   (AMEC_SMH_STATES_PER_LVL * MICS_PER_TICK)
// Number of <AMEC_US_PER_SMH_PERIOD> that happen in 1 second
#define AMEC_SMH_PERIODS_IN_1SEC (10000000 / AMEC_US_PER_SMH_PERIOD)

//*************************************************************************
// Structures
//*************************************************************************
// Each State table (including Substates) will take up 64 bytes
// of SRAM space.
typedef struct smh_tbl
{
  void (*state)();
  const struct smh_tbl * substate;
} smh_tbl_t;

typedef struct
{
  uint8_t state;
  uint8_t substate;
  uint8_t sub_substate;
} smh_state_t;

typedef struct
{
  void (*update_sensor)(int, uint32_t);
} smh_state_timing_t;

//*************************************************************************
// Globals
//*************************************************************************

//*************************************************************************
// Function Prototypes
//*************************************************************************
void amec_generic_smh(const smh_tbl_t * i_smh_tbl, smh_state_t * i_smh_state, smh_state_timing_t * i_smh_timing);

#endif //_AMEC_SMH_H
