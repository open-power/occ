/******************************************************************************
// @file amec_smh.h
// @brief OCC AMEC SMH header file
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section _amec_smh_h amec_smh.h
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *                      thallet   11/08/2011  New file
 *   @rc003             rickylie  02/03/2012  Verify & Clean Up OCC Headers & Comments
 *   @th00b             thallet   02/28/2012  Added defines for scalable tick times
 *   @pb00E             pbavari   03/11/2012  Added correct include file
 *
 *  @endverbatim
 *
 *///*************************************************************************/

#ifndef _AMEC_SMH_H
#define _AMEC_SMH_H

//*************************************************************************
// Includes
//*************************************************************************
//@pb00Ec - changed from common.h to occ_common.h for ODE support
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

//*************************************************************************
// Functions
//*************************************************************************

#endif //_AMEC_SMH_H
