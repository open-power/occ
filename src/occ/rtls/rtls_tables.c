/******************************************************************************
// @file rtls_tables.c
// @brief OCC RTLS COMPONENT TABLES
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section _rtls_tables_c rtls_tables.c
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *                      np, dw    08/10/2011  created by nguyenp & dwoodham
 *   @pb007             pbavari   09/27/2011  Watchdog timer support
 *   @01                abagepa   09/18/2011  updated task table with tasks
 *   @02                nguyenp   09/28/2011  Added task_core_data_low and
 *                                            add task_core_data_high tasks
 *   @pb008             pbavari   10/04/2011  Combined watchdog reset tasks
 *   @04                tapiar    10/16/2011  Added dcom slave to master tasks    
 *   @dw000             dwoodham  10/21/2011  Add DPSS read status task
 *   @05                nguyenp   10/24/2011  Added fast_core_data task
 *   @pb00A             pbavari   11/15/2011  APSS task enabled only on occ_alone
 *                                            simics model
 *   @06                abagepa   11/16/2011  Added dcom parse occ task
 *                                            & reset state machine task
 *   @07                tapiar    11/30/2011  Added amec slave and master state 
 *                                            machine tasks
 *   @rc002             rickylie  02/02/2012  Remove unused DPSS function
 *   @rc003             rickylie  02/03/2012  Verify & Clean Up OCC Headers & Comments
 *   @th00c             thallet   04/04/2012  Centaur Support
 *   @th010             thallet   07/11/2012  Pstate Enablement
 *   @th015             thallet   08/03/2012  Proc Data control in active only
 *   @th022             thallet   10/03/2012  Changes to allow DCOM State/Mode setting
 *   @th031  878471     thallet   04/15/2013  Centaur Throttles
 *   @th032             thallet   04/26/2013  Tuleta HW Bringup Changes - moved wait4mstr
 *   @th038             thallet   06/03/2013  Disable Centaur since it isn't ready on HW yet
 *   @gm004  892961     milesg    07/25/2013  Allow mem autoslew to start centaur tasks
 *   @th046  894648     thallet   08/08/2013  Piggyback a RESET flag change on the coreq fix
 *   @jh00b  910184     joshych   01/10/2014  Add check for checkstop
 *   @sb023  913769     sbroyles  02/08/2014  Change poke watchdog flags
 *   @gm029  917998     milesg    03/06/2014  Send slave inbox while in standby state
 *   @gm035  921471     milesg    03/31/2014  removed unused code
 *   @gm037  925908     milesg    05/07/2014  Redundant OCC/APSS support
 *  @endverbatim
 *
 *///*************************************************************************/

//*************************************************************************
// Includes
//*************************************************************************
#include "rtls.h"
#include <timer.h>          // timer reset task
#include "apss.h"
#include "dcom.h"
#include "state.h"
#include "proc_data.h"	        // @02a
#include "proc_data_control.h"	// @th010
#include <centaur_data.h>	
#include <centaur_control.h>	// @th031
#include "amec_master_smh.h"    // @07a

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************
//flags for task table
//@th022 - removed OCC_ALONE check
#define APSS_TASK_FLAGS                  RTL_FLAG_MSTR | RTL_FLAG_NOTMSTR | RTL_FLAG_OBS | RTL_FLAG_ACTIVE | RTL_FLAG_MSTR_READY |                    RTL_FLAG_RUN 

//>@th002
#define FLAGS_DCOM_RX_SLV_INBX           RTL_FLAG_MSTR | RTL_FLAG_NOTMSTR | RTL_FLAG_OBS | RTL_FLAG_ACTIVE |                                          RTL_FLAG_RUN | RTL_FLAG_STANDBY | RTL_FLAG_RST_REQ | RTL_FLAG_APSS_NOT_INITD
#define FLAGS_DCOM_TX_SLV_INBX           RTL_FLAG_MSTR |                    RTL_FLAG_OBS | RTL_FLAG_ACTIVE | RTL_FLAG_MSTR_READY |                    RTL_FLAG_RUN | RTL_FLAG_STANDBY | RTL_FLAG_RST_REQ | RTL_FLAG_APSS_NOT_INITD

//>@04
#define FLAGS_DCOM_WAIT_4_MSTR           RTL_FLAG_MSTR | RTL_FLAG_NOTMSTR | RTL_FLAG_OBS | RTL_FLAG_ACTIVE | RTL_FLAG_MSTR_READY | RTL_FLAG_NO_APSS | RTL_FLAG_RUN | RTL_FLAG_STANDBY | RTL_FLAG_RST_REQ | RTL_FLAG_APSS_NOT_INITD
#define FLAGS_DCOM_RX_SLV_OUTBOX         RTL_FLAG_MSTR |                    RTL_FLAG_OBS | RTL_FLAG_ACTIVE | RTL_FLAG_MSTR_READY | RTL_FLAG_NO_APSS | RTL_FLAG_RUN | RTL_FLAG_STANDBY | RTL_FLAG_RST_REQ | RTL_FLAG_APSS_NOT_INITD
#define FLAGS_DCOM_TX_SLV_OUTBOX         RTL_FLAG_MSTR | RTL_FLAG_NOTMSTR | RTL_FLAG_OBS | RTL_FLAG_ACTIVE |                       RTL_FLAG_NO_APSS | RTL_FLAG_RUN | RTL_FLAG_STANDBY |                    RTL_FLAG_APSS_NOT_INITD
//<@04

//>@02a
#define FLAGS_LOW_CORES_DATA             RTL_FLAG_MSTR | RTL_FLAG_NOTMSTR | RTL_FLAG_OBS | RTL_FLAG_ACTIVE | RTL_FLAG_MSTR_READY | RTL_FLAG_NO_APSS | RTL_FLAG_RUN |                                       RTL_FLAG_APSS_NOT_INITD
#define FLAGS_HIGH_CORES_DATA            RTL_FLAG_MSTR | RTL_FLAG_NOTMSTR | RTL_FLAG_OBS | RTL_FLAG_ACTIVE | RTL_FLAG_MSTR_READY | RTL_FLAG_NO_APSS | RTL_FLAG_RUN |                                       RTL_FLAG_APSS_NOT_INITD
//<@02a

//Start out with centaur tasks not running.
#define FLAGS_CENTAUR_DATA               RTL_FLAG_NONE
#define FLAGS_CENTAUR_CONTROL            RTL_FLAG_NONE

//>@05a
#define FLAGS_FAST_CORES_DATA            RTL_FLAG_MSTR | RTL_FLAG_NOTMSTR | RTL_FLAG_OBS | RTL_FLAG_ACTIVE | RTL_FLAG_MSTR_READY | RTL_FLAG_NO_APSS | RTL_FLAG_RUN |                                       RTL_FLAG_APSS_NOT_INITD
//<@05a
//<@th002

//>@06a
#define FLAGS_AMEC_SLAVE                 RTL_FLAG_MSTR | RTL_FLAG_NOTMSTR | RTL_FLAG_OBS | RTL_FLAG_ACTIVE | RTL_FLAG_MSTR_READY | RTL_FLAG_NO_APSS | RTL_FLAG_RUN |                                       RTL_FLAG_APSS_NOT_INITD
#define FLAGS_AMEC_MASTER                RTL_FLAG_MSTR |                    RTL_FLAG_OBS | RTL_FLAG_ACTIVE | RTL_FLAG_MSTR_READY | RTL_FLAG_NO_APSS | RTL_FLAG_RUN |                                       RTL_FLAG_APSS_NOT_INITD
//<@06a

#define FLAGS_APSS_START_MEAS            APSS_TASK_FLAGS
#define FLAGS_APSS_CONT_MEAS             APSS_TASK_FLAGS
#define FLAGS_APSS_DONE_MEAS             APSS_TASK_FLAGS

//>@06a
#define FLAGS_DCOM_PARSE_OCC_FW_MSG      RTL_FLAG_MSTR | RTL_FLAG_NOTMSTR | RTL_FLAG_OBS | RTL_FLAG_ACTIVE | RTL_FLAG_MSTR_READY | RTL_FLAG_NO_APSS | RTL_FLAG_RUN | RTL_FLAG_STANDBY | RTL_FLAG_RST_REQ | RTL_FLAG_APSS_NOT_INITD
#define FLAGS_CHECK_FOR_CHECKSTOP        RTL_FLAG_MSTR | RTL_FLAG_NOTMSTR | RTL_FLAG_OBS | RTL_FLAG_ACTIVE | RTL_FLAG_MSTR_READY | RTL_FLAG_NO_APSS | RTL_FLAG_RUN | RTL_FLAG_STANDBY | RTL_FLAG_RST_REQ | RTL_FLAG_APSS_NOT_INITD
//<@06a // @jh00bc

//>@th010
#define FLAGS_CORE_DATA_CONTROL          RTL_FLAG_MSTR | RTL_FLAG_NOTMSTR |                RTL_FLAG_ACTIVE | RTL_FLAG_MSTR_READY | RTL_FLAG_NO_APSS | RTL_FLAG_RUN |                                       RTL_FLAG_APSS_NOT_INITD
//<@th010
// @sb023 Don't run watchdog poke in all cases, expecially if a reset request
// is pending
#define FLAGS_POKE_WDT RTL_FLAG_RUN | RTL_FLAG_MSTR | RTL_FLAG_NOTMSTR | RTL_FLAG_STANDBY | \
                       RTL_FLAG_OBS | RTL_FLAG_ACTIVE | RTL_FLAG_MSTR_READY | RTL_FLAG_NO_APSS | RTL_FLAG_APSS_NOT_INITD



//*************************************************************************
// Structures
//*************************************************************************

//*************************************************************************
// Globals
//*************************************************************************
// Global tick sequences
// The number and size of these will vary as the real tick sequences are developed over time.
//@pb007a - added TASK_ID_RESET_405WDT and TASK_ID_RESET_OCBT to every tick
//@02a - added TASK_ID_CORE_DATA_LOW and TASK_ID_CORE_DATA_HIGH
//@pb008c - Combined ppc405 watchdog and OCB timer task into one
//@05a - added TASK_ID_FAST_CORE_DATA
//@rc002  - Remove unused TASK_ID_DPSS_RD_STATUS

//NOTE: Currently this is the only way the complete apss works in simics TODO need to revisit in 
// the future.

/* The Global Task Table
   Use task_id_t values to index into this table and find a specific task.
   Add rows to this table as TASK_END grows.

   Example use:
     #define TASK_0_FLAGS RTL_FLAG_MSTR | RTL_FLAG_OBS | RTL_FLAG_ACTIVE
     void task_0_func(struct task *i_self);
     uint32_t task_0_data = 0x00000000;  // Use whatever data type makes sense

     // flags,       func_ptr,     data_ptr,              // task_id_t
     { TASK_0_FLAGS, task_0_func, (void *)&task_0_data }, // TASK_ID_0 
*/
//@pb007a - added timer reset tasks
//@pb008c - Combined ppc405 watchdog and OCB timer task into one
//@rc002  - Remove unused flag task_dpss_read_status
task_t G_task_table[TASK_END] = {
    // flags,                      func_ptr,                       data_ptr,// task_id_t
    { FLAGS_APSS_START_MEAS,       task_apss_start_pwr_meas,       NULL },  // TASK_ID_APSS_START
    { FLAGS_LOW_CORES_DATA,        task_core_data,                 (void *) &G_low_cores}, //@02a
    { FLAGS_APSS_CONT_MEAS,        task_apss_continue_pwr_meas,    NULL },  // TASK_ID_APSS_CONT
    { FLAGS_HIGH_CORES_DATA,       task_core_data,                 (void *) &G_high_cores}, //@02a
    { FLAGS_APSS_DONE_MEAS,        task_apss_complete_pwr_meas,    NULL },  // TASK_ID_APSS_DONE
    { FLAGS_FAST_CORES_DATA,       task_fast_core_data,            NULL },  //@05a
    { FLAGS_DCOM_RX_SLV_INBX,      task_dcom_rx_slv_inbox,         NULL },  // TASK_ID_DCOM_RX_INBX
    { FLAGS_DCOM_TX_SLV_INBX,      task_dcom_tx_slv_inbox,         NULL },  // TASK_ID_DCOM_TX_INBX
    { FLAGS_POKE_WDT,              task_poke_watchdogs,            NULL },  // TASK_ID_POKE_WDT
    { FLAGS_DCOM_WAIT_4_MSTR,      task_dcom_wait_for_master,      NULL },  // TASK_ID_DCOM_WAIT_4_MSTR
    { FLAGS_DCOM_RX_SLV_OUTBOX,    task_dcom_rx_slv_outboxes,      NULL },  // TASK_ID_DCOM_RX_OUTBX
    { FLAGS_DCOM_TX_SLV_OUTBOX,    task_dcom_tx_slv_outbox,        NULL },  // TASK_ID_DCOM_TX_OUTBX
    { FLAGS_DCOM_PARSE_OCC_FW_MSG, task_dcom_parse_occfwmsg,       NULL },  // TASK_ID_DCOM_PARSE_FW_MSG
    { FLAGS_CHECK_FOR_CHECKSTOP,   task_check_for_checkstop,       NULL },  // TASK_ID_CHECK_FOR_CHECKSTOP // @jh00bc
    { FLAGS_AMEC_SLAVE,            task_amec_slave,                NULL },  // TASK_ID_AMEC_SLAVE @07a
    { FLAGS_AMEC_MASTER,           task_amec_master,               NULL },  // TASK_ID_AMEC_MASTER @07a
    { FLAGS_CENTAUR_DATA,          task_centaur_data,              (void *) &G_centaur_data_task}, // TASK_ID_CENTAUR_DATA @th00c
    { FLAGS_CORE_DATA_CONTROL,     task_core_data_control,         NULL },  // TASK_ID_CORE_DATA_CONTROL
    { FLAGS_CENTAUR_CONTROL,       task_centaur_control,           (void *) &G_centaur_control_task },  // TASK_ID_CENTAUR_CONTROL @th038
};

const uint8_t G_tick0_seq[] = { 
                                TASK_ID_APSS_START, 
                                TASK_ID_CORE_DATA_LOW,
                                TASK_ID_CENTAUR_DATA,
                                TASK_ID_APSS_CONT,
                                TASK_ID_CORE_DATA_HIGH,
                                TASK_ID_APSS_DONE,
                                TASK_ID_FAST_CORE_DATA,
                                TASK_ID_CENTAUR_CONTROL,
                                TASK_ID_CORE_DATA_CONTROL,
                                TASK_ID_DCOM_WAIT_4_MSTR,
                                TASK_ID_DCOM_RX_INBX,
                                TASK_ID_DCOM_RX_OUTBX,
                                TASK_ID_POKE_WDT,
                                TASK_ID_DCOM_TX_OUTBX,
                                TASK_ID_DCOM_TX_INBX, 
                                TASK_ID_AMEC_SLAVE,
                                TASK_ID_AMEC_MASTER,
                                TASK_ID_DCOM_PARSE_FW_MSG,
                                TASK_ID_CHECK_FOR_CHECKSTOP,
                                TASK_END };

const uint8_t G_tick1_seq[] = { 
                                TASK_ID_APSS_START,
                                TASK_ID_CORE_DATA_LOW,
                                TASK_ID_CENTAUR_DATA,
                                TASK_ID_APSS_CONT,
                                TASK_ID_CORE_DATA_HIGH, 
                                TASK_ID_APSS_DONE,
                                TASK_ID_FAST_CORE_DATA,
                                TASK_ID_CENTAUR_CONTROL,
                                TASK_ID_CORE_DATA_CONTROL,
                                TASK_ID_DCOM_WAIT_4_MSTR, 
                                TASK_ID_DCOM_RX_INBX,
                                TASK_ID_DCOM_RX_OUTBX,
                                TASK_ID_DCOM_TX_OUTBX,
                                TASK_ID_DCOM_TX_INBX, 
                                TASK_ID_AMEC_SLAVE,
                                TASK_ID_AMEC_MASTER,
                                TASK_ID_DCOM_PARSE_FW_MSG,
                                TASK_ID_CHECK_FOR_CHECKSTOP,
                                TASK_END };

const uint8_t G_tick2_seq[] = { 
                                TASK_ID_APSS_START, 
                                TASK_ID_CORE_DATA_LOW,
                                TASK_ID_CENTAUR_DATA,
                                TASK_ID_APSS_CONT, 
                                TASK_ID_CORE_DATA_HIGH,
                                TASK_ID_APSS_DONE,
                                TASK_ID_FAST_CORE_DATA,
                                TASK_ID_CENTAUR_CONTROL,
                                TASK_ID_CORE_DATA_CONTROL,
                                TASK_ID_DCOM_WAIT_4_MSTR, 
                                TASK_ID_DCOM_RX_INBX,
                                TASK_ID_DCOM_RX_OUTBX,
                                TASK_ID_DCOM_TX_OUTBX,
                                TASK_ID_DCOM_TX_INBX, 
                                TASK_ID_AMEC_SLAVE,
                                TASK_ID_AMEC_MASTER,
                                TASK_ID_DCOM_PARSE_FW_MSG,
                                TASK_ID_CHECK_FOR_CHECKSTOP,
                                TASK_END };
    
const uint8_t G_tick3_seq[] = { 
                                TASK_ID_APSS_START, 
                                TASK_ID_CORE_DATA_LOW,
                                TASK_ID_CENTAUR_DATA,
                                TASK_ID_APSS_CONT,
                                TASK_ID_CORE_DATA_HIGH,
                                TASK_ID_APSS_DONE,
                                TASK_ID_FAST_CORE_DATA,
                                TASK_ID_CENTAUR_CONTROL,
                                TASK_ID_CORE_DATA_CONTROL,
                                TASK_ID_DCOM_WAIT_4_MSTR, 
                                TASK_ID_DCOM_RX_INBX,
                                TASK_ID_DCOM_RX_OUTBX,
                                TASK_ID_DCOM_TX_OUTBX,
                                TASK_ID_DCOM_TX_INBX, 
                                TASK_ID_AMEC_SLAVE,
                                TASK_ID_AMEC_MASTER,
                                TASK_ID_DCOM_PARSE_FW_MSG,
                                TASK_ID_CHECK_FOR_CHECKSTOP,
                                TASK_END };

const uint8_t G_tick4_seq[] = { 
                                TASK_ID_APSS_START, 
                                TASK_ID_CORE_DATA_LOW,
                                TASK_ID_CENTAUR_DATA,
                                TASK_ID_APSS_CONT,
                                TASK_ID_CORE_DATA_HIGH, 
                                TASK_ID_APSS_DONE,
                                TASK_ID_FAST_CORE_DATA,
                                TASK_ID_CENTAUR_CONTROL,
                                TASK_ID_CORE_DATA_CONTROL,
                                TASK_ID_DCOM_WAIT_4_MSTR, 
                                TASK_ID_DCOM_RX_INBX,
                                TASK_ID_DCOM_RX_OUTBX,
                                TASK_ID_POKE_WDT,
                                TASK_ID_DCOM_TX_OUTBX,
                                TASK_ID_DCOM_TX_INBX, 
                                TASK_ID_AMEC_SLAVE,
                                TASK_ID_AMEC_MASTER,
                                TASK_ID_DCOM_PARSE_FW_MSG,
                                TASK_ID_CHECK_FOR_CHECKSTOP,
                                TASK_END };

const uint8_t G_tick5_seq[] = { 
                                TASK_ID_APSS_START, 
                                TASK_ID_CORE_DATA_LOW,
                                TASK_ID_CENTAUR_DATA,
                                TASK_ID_APSS_CONT,
                                TASK_ID_CORE_DATA_HIGH,
                                TASK_ID_APSS_DONE,
                                TASK_ID_FAST_CORE_DATA,
                                TASK_ID_CENTAUR_CONTROL,
                                TASK_ID_CORE_DATA_CONTROL,
                                TASK_ID_DCOM_WAIT_4_MSTR, 
                                TASK_ID_DCOM_RX_INBX,
                                TASK_ID_DCOM_RX_OUTBX,
                                TASK_ID_DCOM_TX_OUTBX,
                                TASK_ID_DCOM_TX_INBX, 
                                TASK_ID_AMEC_SLAVE,
                                TASK_ID_AMEC_MASTER,
                                TASK_ID_DCOM_PARSE_FW_MSG,
                                TASK_ID_CHECK_FOR_CHECKSTOP,
                                TASK_END };

const uint8_t G_tick6_seq[] = { 
                                TASK_ID_APSS_START, 
                                TASK_ID_CORE_DATA_LOW,
                                TASK_ID_CENTAUR_DATA,
                                TASK_ID_APSS_CONT,
                                TASK_ID_CORE_DATA_HIGH,
                                TASK_ID_APSS_DONE,
                                TASK_ID_FAST_CORE_DATA,
                                TASK_ID_CENTAUR_CONTROL,
                                TASK_ID_CORE_DATA_CONTROL,
                                TASK_ID_DCOM_WAIT_4_MSTR, 
                                TASK_ID_DCOM_RX_INBX,
                                TASK_ID_DCOM_RX_OUTBX,
                                TASK_ID_DCOM_TX_OUTBX,
                                TASK_ID_DCOM_TX_INBX, 
                                TASK_ID_AMEC_SLAVE,
                                TASK_ID_AMEC_MASTER,
                                TASK_ID_DCOM_PARSE_FW_MSG,
                                TASK_ID_CHECK_FOR_CHECKSTOP,
                                TASK_END };

const uint8_t G_tick7_seq[] = { 
                                TASK_ID_APSS_START, 
                                TASK_ID_CORE_DATA_LOW,
                                TASK_ID_CENTAUR_DATA,
                                TASK_ID_APSS_CONT,
                                TASK_ID_CORE_DATA_HIGH,
                                TASK_ID_APSS_DONE,
                                TASK_ID_FAST_CORE_DATA,
                                TASK_ID_CENTAUR_CONTROL,
                                TASK_ID_CORE_DATA_CONTROL,
                                TASK_ID_DCOM_WAIT_4_MSTR, 
                                TASK_ID_DCOM_RX_INBX,
                                TASK_ID_DCOM_RX_OUTBX,
                                TASK_ID_DCOM_TX_OUTBX,
                                TASK_ID_DCOM_TX_INBX, 
                                TASK_ID_AMEC_SLAVE,
                                TASK_ID_AMEC_MASTER,
                                TASK_ID_DCOM_PARSE_FW_MSG,
                                TASK_ID_CHECK_FOR_CHECKSTOP,
                                TASK_END };

// The Global Tick Table 
// This will change as the real tick sequences are developed.
const uint8_t *G_tick_table[MAX_NUM_TICKS] = {
    G_tick0_seq,
    G_tick1_seq,
    G_tick2_seq,
    G_tick3_seq,
    G_tick4_seq,
    G_tick5_seq,
    G_tick6_seq,
    G_tick7_seq
};

//*************************************************************************
// Function Prototypes
//*************************************************************************

//*************************************************************************
// Functions
//*************************************************************************
