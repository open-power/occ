/******************************************************************************
// @file rtls.h
// @brief OCC rtls component header file.
*/
/******************************************************************************
 *   @th00a             thallet   02/06/12  Sensors with fw timings            
 *
 *       @page ChangeLogs Change Logs
 *       @section _rtls_h rtls.h
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *                      np, dw    08/10/2011  created by nguyenp & dwoodham
 *   @pb004             pbavari   09/12/2011  Initialization section support
 *   @pb007             pbavari   09/27/2011  Watchdog timer task support
 *   @02                tapiar    10/03/2011  Changed BOOLEAN to bool, added tasks
 *   @03                nguyenp   10/10/2011  Added task_core_data_low and
 *                                            add task_core_data_high tasks
 *   @pb008             pbavari   10/04/2011  Combined watchdog reset tasks
 *   @05                abagepa   10/16/2011  Added slave to master tasks &
 *                                            master ready flag
 *   @dw000             dwoodham  10/21/2011  Added DPSS read status task
 *   @06                nguyenp   10/24/2011  Added fast_core_data task
 *   @07                abagepa   11/16/2011  Added dcom parse occ task
 *                                            & reset state machine task
 *   @08                tapiar    11/30/2011  Added Amec slave and master tasks
 *   @rc001             rickylie  12/30/2011  Moved debug trace defines to trac.h
 *   @rc002             rickylie  02/02/2012  Remove unused DPSS function
 *   @rc003             rickylie  02/03/2012  Verify & Clean Up OCC Headers & Comments
 *   @pb00E             pbavari   03/11/2012  Added correct include file
 *   @th00c             thallet   04/04/2012  Centaur Support
 *   @th010             thallet   07/11/2012  Pstate Enablement
 *   @th015             thallet   08/03/2012  Added deferred task flag setting
 *   @th022             thallet   10/03/2012  Changes to allow DCOM State/Mode setting
 *   @th031  878471     thallet   04/15/2013  Centaur Throttles
 *   @gm004  892961     milesg    07/25/2013  Allow mem autslew to start centaur tasks
 *   @jh00b  910184     joshych   01/10/2014  Add check for checkstop
 *   @gm037  925908     milesg    05/07/2014  Redundant OCC/APSS support
 *   @gm038  926761     milesg    05/16/2014  Centaur tasks were not running																					
 *
 *  @endverbatim
 *
 *///*************************************************************************/
 
#ifndef _RTLSCH_H
#define _RTLSCH_H

//*************************************************************************
// Includes
//*************************************************************************
//@pb00Ec - changed from common.h to occ_common.h for ODE support
#include <occ_common.h>
#include <ssx.h>
#include <ssx_app_cfg.h>

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************
typedef struct task {
    uint32_t flags;
    void (*func_ptr)(struct task*);
    void *data_ptr;
}task_t;

// Task ID values
// These are used as indices into the task table defined below.
// @pb007a - Added TASK_ID_RESET_405WDT  and TASK_ID_RESET_OCBT
// @pb008c - Combined ppc405 watchdog and OCB timer task into one
// @rc002  - Remove unused TASK_ID_DPSS_RD_STATUS
typedef enum {
    TASK_ID_APSS_START = 0x00,
    TASK_ID_CORE_DATA_LOW,          //@03a
    TASK_ID_APSS_CONT,
    TASK_ID_CORE_DATA_HIGH,         //@03a
    TASK_ID_APSS_DONE,
    TASK_ID_FAST_CORE_DATA,         //@06a
    TASK_ID_DCOM_RX_INBX,
    TASK_ID_DCOM_TX_INBX,
    TASK_ID_POKE_WDT,    // reset ppc405 watchdog and OCB timer
    TASK_ID_DCOM_WAIT_4_MSTR,
    TASK_ID_DCOM_RX_OUTBX,
    TASK_ID_DCOM_TX_OUTBX,
    TASK_ID_DCOM_PARSE_FW_MSG,
    TASK_ID_CHECK_FOR_CHECKSTOP,    // @jh00bc
    TASK_ID_AMEC_SLAVE,             // @08a amec smh tasks
    TASK_ID_AMEC_MASTER,            // @08a amec smh task
    TASK_ID_CENTAUR_DATA,           // @tgh00c centaur task
    TASK_ID_CORE_DATA_CONTROL,      // @th010
    TASK_ID_CENTAUR_CONTROL,        // @th031 centaur task
    TASK_END  // This must always be the last enum in this list,
              // so that TASK_END always equals the last task ID + 1.
} task_id_t;

// Structure containing the durations measured within a tick
typedef struct
{
  uint32_t rtl_dur;         // Duration of RTL tick interrupt
  uint32_t ameint_dur;      // Combined duration of mstr & slv AMEC tasks
  uint32_t amess_dur;       // Combined duration of last mstr & slv AMEC state
  uint8_t  amess_state;     // Last AMEC state that was run
  uint64_t rtl_start;       // SsxTimebase of Start of current RTL Tick
  uint64_t rtl_start_gpe;   // SsxTimebase of Start of current RTL Tick (for GPE > 250us meas)
  uint32_t gpe_dur[2];      // Duration of the GPE Engines / tick
  PoreFlex * gpe0_timing_request;   // GPE Request that facilitates GPE WC meas
  PoreFlex * gpe1_timing_request;   // GPE Request that facilitates GPE WC meas
} fw_timing_t;   // @th00a


// Bit flags to define when a task can run
// NOTE: whenever new flag is added, it must also be added to the
// RTL_ALL_FLAGS define.
#define RTL_FLAG_NONE           0x00000000  // Task has been turned off permanently
#define RTL_FLAG_RUN            0x00000001  // Task has been requested to run
#define RTL_FLAG_MSTR           0x00000002  // Task can run on the master
#define RTL_FLAG_NOTMSTR        0x00000004  // Task can run on non-masters
#define RTL_FLAG_OBS            0x00000008  // Task can run in observation state
#define RTL_FLAG_ACTIVE         0x00000010  // Task can run in active state
#define RTL_FLAG_RST_REQ        0x00000020  // Task can run after a reset request
#define RTL_FLAG_NO_APSS        0x00000040  // Task can run with no APSS present
#define RTL_FLAG_MSTR_READY     0x00000080  // Task can run Master is ready
#define RTL_FLAG_STANDBY        0x00000100  // Task can run in Standby state  // @th022
#define RTL_FLAG_APSS_NOT_INITD 0x00000200  // Task can run while APSS is not initialized -- gm037

//gm038
#define RTL_ALL_FLAGS   (RTL_FLAG_RUN | RTL_FLAG_MSTR | RTL_FLAG_NOTMSTR |    \
                         RTL_FLAG_OBS | RTL_FLAG_ACTIVE | RTL_FLAG_RST_REQ |  \
                         RTL_FLAG_NO_APSS | RTL_FLAG_MSTR_READY | RTL_FLAG_STANDBY | \
                         RTL_FLAG_APSS_NOT_INITD)

//gm038
#define CENTAUR_CONTROL_RTL_FLAGS (RTL_FLAG_MSTR | RTL_FLAG_NOTMSTR | RTL_FLAG_ACTIVE | \
                                   RTL_FLAG_MSTR_READY | RTL_FLAG_NO_APSS | RTL_FLAG_RUN | \
                                   RTL_FLAG_APSS_NOT_INITD)
//gm038
#define CENTAUR_DATA_RTL_FLAGS (RTL_FLAG_MSTR | RTL_FLAG_NOTMSTR | RTL_FLAG_OBS | \
                                RTL_FLAG_ACTIVE | RTL_FLAG_MSTR_READY | RTL_FLAG_NO_APSS | \
                                RTL_FLAG_RUN | RTL_FLAG_APSS_NOT_INITD)
        
// Tick Timer definitions
#define MICS_PER_TICK 250  // Number of micro-seconds per tick
#define MAX_NUM_TICKS 8    // Number of entries in the global tick table (power of 2)

//*************************************************************************
// Structures
//*************************************************************************

//*************************************************************************
// Globals
//*************************************************************************
// The value of the current tick
extern uint32_t G_current_tick;

// The durations measured within the current tick
extern fw_timing_t G_fw_timing;    // @th00a

// Preferred macro for accessing the current tick value
#define CURRENT_TICK G_current_tick

//*************************************************************************
// Function Prototypes
//*************************************************************************
// rtl_ocb_init
// Responsible for initializing the hardware timer in occ control block.
// It is the timer that supplies the periodic RTL interrupt.
void rtl_ocb_init(void) INIT_SECTION;

// rtl_do_tick
// RTL intr handler, a full-mode handler that invokes through the macro brige
// Runs all tasks in the current tick sequence.
void rtl_do_tick(void *private, SsxIrqId irq, int priority);

// rtl_start_task
// Request that a task runs, starting the next time it comes up
// in a tick sequence.
void rtl_start_task(const task_id_t i_task_id);

// rtl_stop_task
// Request that a task NOT run any time it comes up in a tick sequence.
void rtl_stop_task(const task_id_t i_task_id);

// rtl_task_is_runnable
// Find out if a task can run or not.
bool rtl_task_is_runnable(const task_id_t i_task_id);

// rtl_set_task_data
// Changes the data pointer for the specified task.
void rtl_set_task_data(const task_id_t i_task_id, void* i_data_ptr);

// rtl_set_run_mask
// Stores the bitwise-or of i_flag and the global run mask
// into the global run mask.
void rtl_set_run_mask(const uint32_t i_flags);
void rtl_set_run_mask_deferred( const uint32_t i_flags );

// rtl_clr_run_mask
// Stores the bitwise-and of the inverse of i_flag and the global run mask
// into the global run mask.
void rtl_clr_run_mask(const uint32_t i_flags);
void rtl_clr_run_mask_deferred( const uint32_t i_flags );

//*************************************************************************
// Functions
//*************************************************************************

#endif //_RTLSCH_H
