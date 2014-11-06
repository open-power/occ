/******************************************************************************
// @file amec_master_smh.h
// @brief OCC AMEC Master SMH header file
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section _amec_master_smh_h amec_master_smh.h
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *                      thallet   11/08/2011  New file
 *   @th00a             thallet   02/03/2012  Worst case FW timings in AMEC Sensors
 *   @rc003             rickylie  02/03/2012  Verify & Clean Up OCC Headers & Comments
 *   @pb00E             pbavari   03/11/2012  Added correct include file
 *   @gs006  884384     gjsilva   05/30/2013  Support for mnfg auto-slewing function
 *   @db001  897459     deepthib  08/03/2013  Power cap mismatch & under pcap functions
 *   @gs015  905166     gjsilva   11/04/2013  Full support for IPS function
 *   @rt004  908817     tapiar    12/11/2013  Expand G_active_slave_pcaps arrray to also carry
 *                                            valid pcap byte from slaves
 *   @gs025  913663     gjsilva   01/30/2014  Full fupport for soft frequency boundaries
 *
 *  @endverbatim
 *
 *///*************************************************************************/
 
#ifndef _AMEC_MASTER_SMH_H
#define _AMEC_MASTER_SMH_H

//*************************************************************************
// Includes
//*************************************************************************
//@pb00Ec - changed from common.h to occ_common.h for ODE support
#include <occ_common.h>
#include <ssx.h>
#include <ssx_app_cfg.h>
#include <amec_smh.h>

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************
#define AMEC_MST_STATE()                AMEC_STATE(&G_amec_mst_state);
#define AMEC_MST_SUBSTATE()             AMEC_SUBSTATE(&G_amec_mst_state);
#define AMEC_MST_SUB_SUBSTATE()         AMEC_SUB_SUBSTATE(&G_amec_mst_state);

#define AMEC_MST_STATE_NEXT()           AMEC_STATE_NEXT(&G_amec_mst_state);
#define AMEC_MST_SUBSTATE_NEXT()        AMEC_SUBSTATE_NEXT(&G_amec_mst_state);
#define AMEC_MST_SUB_SUBSTATE_NEXT()    AMEC_SUB_SUBSTATE_NEXT(&G_amec_mst_state);

#define AMEC_MST_SET_MNFG_FMIN(a)       AMEC_MST_CUR_MNFG_FMIN() = a
#define AMEC_MST_SET_MNFG_FMAX(a)       AMEC_MST_CUR_MNFG_FMAX() = a
#define AMEC_MST_SET_MNFG_FSTEP(a)      g_amec->mnfg_parms.fstep = a
#define AMEC_MST_SET_MNFG_DELAY(a)      g_amec->mnfg_parms.delay = a
#define AMEC_MST_START_AUTO_SLEW()      g_amec->mnfg_parms.auto_slew = 1
#define AMEC_MST_STOP_AUTO_SLEW()       g_amec->mnfg_parms.auto_slew = 0
#define AMEC_MST_CUR_SLEW_COUNT()       g_amec->mnfg_parms.slew_counter
#define AMEC_MST_CUR_MNFG_FMIN()        g_amec->mnfg_parms.fmin
#define AMEC_MST_CUR_MNFG_FMAX()        g_amec->mnfg_parms.fmax

//*************************************************************************
// Structures
//*************************************************************************

//*************************************************************************
// Globals
//*************************************************************************
extern const smh_tbl_t amec_mst_state_table[AMEC_SMH_STATES_PER_LVL];
extern smh_state_t G_amec_mst_state;
extern smh_state_timing_t G_amec_mst_state_timings;

typedef struct
{
    uint16_t active_pcap;
    uint8_t  pcap_valid;
    uint8_t  reserved;
}slave_pcap_info_t;

extern slave_pcap_info_t G_slave_active_pcaps[MAX_OCCS]; //@db001a @rt004c
extern uint8_t G_pcaps_mismatch_count;          //@db001a
extern uint8_t G_over_cap_count;                //@db001a
extern uint16_t G_mst_soft_fmin;
extern uint16_t G_mst_soft_fmax;

//*************************************************************************
// Function Prototypes
//*************************************************************************
void amec_mst_update_smh_sensors(int i_smh_state, uint32_t i_duration);

// PRE: master common tasks
void amec_mst_common_tasks_pre(void);
// POST: master common tasks
void amec_mst_common_tasks_post(void);

// Master auto-slewing function
void amec_master_auto_slew(void);
// OCC Power cap mismatch function      @db001a
void amec_mst_check_pcaps_match(void);
// Check if under power cap function    @db001a
void amex_mst_check_under_pcap(void);
// Idle Power Saver main algorithm
void amec_mst_ips_main(void);
// Get the current Idle Power Saver active status
uint8_t AMEC_mst_get_ips_active_status();
void amec_mst_gen_soft_freq(void);

// Master States
void amec_mst_state_0(void);
void amec_mst_state_1(void);
void amec_mst_state_2(void);
void amec_mst_state_3(void);
void amec_mst_state_4(void);
void amec_mst_state_5(void);
void amec_mst_state_6(void);
void amec_mst_state_7(void);

// Master SubState 0
void amec_mst_substate_0_0(void);
void amec_mst_substate_0_1(void);
void amec_mst_substate_0_4(void);
void amec_mst_substate_0_7(void);

// Master SubState 3
void amec_mst_substate_3_0(void);
void amec_mst_substate_3_1(void);
void amec_mst_substate_3_2(void);
void amec_mst_substate_3_6(void);
void amec_mst_substate_3_7(void);

// Master Sub-SubState 3
void amec_mst_sub_substate_3_0_0(void);
void amec_mst_sub_substate_3_0_1(void);
void amec_mst_sub_substate_3_0_7(void);
                                
// Master SubState 6
void amec_mst_substate_6_0(void);
void amec_mst_substate_6_1(void);
void amec_mst_substate_6_2(void);
void amec_mst_substate_6_3(void);
void amec_mst_substate_6_4(void);
void amec_mst_substate_6_5(void);
void amec_mst_substate_6_6(void);
void amec_mst_substate_6_7(void);
                                
void amec_mst_substate_3_0(void);
void amec_mst_substate_3_1(void);

void amec_mst_sub_substate_6_1_0(void);
void amec_mst_sub_substate_6_1_1(void);
void amec_mst_sub_substate_6_1_2(void);
void amec_mst_sub_substate_6_1_3(void);
void amec_mst_sub_substate_6_1_4(void);
void amec_mst_sub_substate_6_1_5(void);
void amec_mst_sub_substate_6_1_6(void);
void amec_mst_sub_substate_6_1_7(void);
                                
//*************************************************************************
// Functions
//*************************************************************************

#endif
