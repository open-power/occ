/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/rtls/rtls.h $                                     */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2019                        */
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

#ifndef _RTLSCH_H
#define _RTLSCH_H

#include <occ_common.h>
#include <ssx.h>
#include <ssx_app_cfg.h>
#include "occhw_async.h"

typedef struct task {
    uint32_t flags;
    void (*func_ptr)(struct task*);
    void *data_ptr;
}task_t;

// Task ID values
// These are used as indices into the task table defined below.
typedef enum {
    TASK_ID_APSS_START = 0x00,
    TASK_ID_CORE_DATA_LOW,
    TASK_ID_APSS_CONT,
    TASK_ID_CORE_DATA_HIGH,
    TASK_ID_APSS_DONE,
    TASK_ID_DCOM_RX_INBX,
    TASK_ID_DCOM_TX_INBX,
    TASK_ID_POKE_WDT,              // Reset ppc405 watchdog and OCB timer
    TASK_ID_DCOM_WAIT_4_MSTR,
    TASK_ID_DCOM_RX_OUTBX,
    TASK_ID_DCOM_TX_OUTBX,
    TASK_ID_MISC_405_CHECKS,       // Miscellaneous checks to be done by 405
    TASK_ID_DCOM_PARSE_FW_MSG,
    TASK_ID_AMEC_SLAVE,            // AMEC SMH tasks
    TASK_ID_AMEC_MASTER,           // AMEC SMH tasks
    TASK_ID_CORE_DATA_CONTROL,
    TASK_ID_GPU_SM,                 // GPU State Machine
    TASK_ID_MEMORY_DATA,           // Memory data collection task
    TASK_ID_MEMORY_CONTROL,        // Memory control task
    TASK_ID_NEST_DTS,
    TASK_ID_24X7,                  // 24x7 data collection task
    TASK_ID_GPE_TIMINGS,
    TASK_ID_GET_TOD,               // Get time of day task
    TASK_ID_APSS_RESET,            // (HW) reset APSS
    TASK_ID_SEND_VRT_TO_PGPE,
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
  uint64_t rtl_start_gpe[2];   // SsxTimebase of Start of current RTL Tick (for GPE > 1 tick)
  uint32_t gpe_dur[2];      // Duration of the GPE Engines / tick
  GpeRequest* gpe0_timing_request;   // GPE Request that facilitates GPE WC meas
  GpeRequest* gpe1_timing_request;   // GPE Request that facilitates GPE WC meas
} fw_timing_t;

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
#define RTL_FLAG_STANDBY        0x00000100  // Task can run in Standby state
#define RTL_FLAG_APSS_NOT_INITD 0x00000200  // Task can run while APSS is not initialized

#define RTL_ALL_FLAGS   (RTL_FLAG_RUN | RTL_FLAG_MSTR | RTL_FLAG_NOTMSTR |    \
                         RTL_FLAG_OBS | RTL_FLAG_ACTIVE | RTL_FLAG_RST_REQ |  \
                         RTL_FLAG_NO_APSS | RTL_FLAG_MSTR_READY | RTL_FLAG_STANDBY | \
                         RTL_FLAG_APSS_NOT_INITD)

#define MEMORY_CONTROL_RTL_FLAGS (RTL_FLAG_MSTR | RTL_FLAG_NOTMSTR | RTL_FLAG_ACTIVE | \
                                  RTL_FLAG_MSTR_READY | RTL_FLAG_NO_APSS | RTL_FLAG_RUN | \
                                  RTL_FLAG_APSS_NOT_INITD)

#define MEMORY_DATA_RTL_FLAGS (RTL_FLAG_MSTR | RTL_FLAG_NOTMSTR | RTL_FLAG_OBS | \
                               RTL_FLAG_ACTIVE | RTL_FLAG_MSTR_READY | RTL_FLAG_NO_APSS | \
                               RTL_FLAG_RUN | RTL_FLAG_APSS_NOT_INITD)

#define GPU_RTL_FLAGS (RTL_FLAG_MSTR | RTL_FLAG_NOTMSTR | RTL_FLAG_OBS | RTL_FLAG_ACTIVE | \
                       RTL_FLAG_MSTR_READY | RTL_FLAG_NO_APSS | RTL_FLAG_RUN | \
                       RTL_FLAG_APSS_NOT_INITD)

// Tick Timer definitions:
#define MICS_PER_TICK G_mics_per_tick // Number of micro-seconds per tick
#define MAX_NUM_TICKS 16   // Number of entries in the global tick table (power of 2)
// MICS_PER_TICK must be lower than 62ms to guarantee the tick table completes in < 1s (for health monitor)
#define HW_MICS_PER_TICK        500
#define SIMICS_MICS_PER_TICK  20000 // slow down RTL to 20ms for Simics
#define DCOM_TX_APSS_WAIT_TIME G_dcom_tx_apss_wait_time

// core data collection time.  All cores collected one time thru tick table
#define CORE_DATA_COLLECTION_US (MICS_PER_TICK * MAX_NUM_TICKS)
//Number of samples per second for performance-related algorithms (e.g. UTILCy)
#define AMEC_DPS_SAMPLING_RATE  (1000000 / CORE_DATA_COLLECTION_US)
//Time interval for averaging utilization and frequency (IPS algorithm) 3 seconds
#define AMEC_IPS_AVRG_INTERVAL           3
// NOTE: for IPS timings to work, need to make sure the check for amec_mst_ips_main() is at the same frequency
// i.e. core data is collected every CORE_DATA_COLLECTION_US the checking for amec_mst_ips_main() must be same.
// If core data collection time changes must adjust the checking for amec_mst_ips_main() to match

// The value of the current tick
extern uint32_t G_current_tick;

// The number of micro-seconds per tick
extern uint32_t G_mics_per_tick;

// The number of micro-seconds to wait for APSS data to complete
extern uint32_t G_dcom_tx_apss_wait_time;

// The durations measured within the current tick
extern fw_timing_t G_fw_timing;

// Preferred macro for accessing the current tick value
#define CURRENT_TICK G_current_tick

// Responsible for initializing the hardware timer in occ control block.
// It is the timer that supplies the periodic RTL interrupt.
void rtl_ocb_init(void) INIT_SECTION;

// RTL intr handler, a full-mode handler that invokes through the macro brige
// Runs all tasks in the current tick sequence.
void rtl_do_tick(void *private, SsxIrqId irq, int priority);

// Request that a task runs, starting the next time it comes up
// in a tick sequence.
void rtl_start_task(const task_id_t i_task_id);

// Request that a task NOT run any time it comes up in a tick sequence.
void rtl_stop_task(const task_id_t i_task_id);

// Find out if a task can run or not.
bool rtl_task_is_runnable(const task_id_t i_task_id);

// Changes the data pointer for the specified task.
void rtl_set_task_data(const task_id_t i_task_id, void* i_data_ptr);

// Stores the bitwise-or of i_flag and the global run mask
// into the global run mask.
void rtl_set_run_mask(const uint32_t i_flags);
void rtl_set_run_mask_deferred( const uint32_t i_flags );

// Stores the bitwise-and of the inverse of i_flag and the global run mask
// into the global run mask.
void rtl_clr_run_mask(const uint32_t i_flags);
void rtl_clr_run_mask_deferred( const uint32_t i_flags );

#endif //_RTLSCH_H
