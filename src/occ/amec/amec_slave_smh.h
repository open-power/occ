/******************************************************************************
// @file amec_slave_smh.h
// @brief Slave State Machine header file
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section _amec_slave_smh_h amec_slave_smh.h
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *                      thallet   11/08/2011  New file
 *   @rc001             rickylie  01/10/2012  Added trac.h
 *   @rc003             rickylie  02/03/2012  Verify & Clean Up OCC Headers & Comments
 *   @pb00E             pbavari   03/11/2012  Added correct include file
 *   @th00a             thallet   02/09/2012  G_ added as per codign convention
 *   @gs002   854460    gjsilva   09/25/2012  Support for slave state machine
 *   @gs027   918066    gjsilva   03/12/2014  Misc functions from ARL
 *
 *  @endverbatim
 *
 *///*************************************************************************/
 
#ifndef _AMEC_SLAVE_SMH_H
#define _AMEC_SLAVE_SMH_H

//*************************************************************************
// Includes
//*************************************************************************
//@pb00Ec - changed from common.h to occ_common.h for ODE support
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

//*************************************************************************
// Functions
//*************************************************************************

#endif
