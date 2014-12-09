/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/rtls/rtls_tables.c $                                  */
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

#include "rtls.h"
#include <timer.h> // timer reset task
#include "apss.h"
#include "dcom.h"
#include "state.h"
#include "proc_data.h"
#include "proc_data_control.h"
#include <centaur_data.h>
#include <centaur_control.h>
#include "amec_master_smh.h"

//flags for task table
#define APSS_TASK_FLAGS                  RTL_FLAG_MSTR | RTL_FLAG_NOTMSTR | RTL_FLAG_OBS | RTL_FLAG_ACTIVE | RTL_FLAG_MSTR_READY |                    RTL_FLAG_RUN

#define FLAGS_DCOM_RX_SLV_INBX           RTL_FLAG_MSTR | RTL_FLAG_NOTMSTR | RTL_FLAG_OBS | RTL_FLAG_ACTIVE |                                          RTL_FLAG_RUN | RTL_FLAG_STANDBY | RTL_FLAG_RST_REQ | RTL_FLAG_APSS_NOT_INITD
#define FLAGS_DCOM_TX_SLV_INBX           RTL_FLAG_MSTR |                    RTL_FLAG_OBS | RTL_FLAG_ACTIVE | RTL_FLAG_MSTR_READY |                    RTL_FLAG_RUN | RTL_FLAG_STANDBY | RTL_FLAG_RST_REQ | RTL_FLAG_APSS_NOT_INITD

#define FLAGS_DCOM_WAIT_4_MSTR           RTL_FLAG_MSTR | RTL_FLAG_NOTMSTR | RTL_FLAG_OBS | RTL_FLAG_ACTIVE | RTL_FLAG_MSTR_READY | RTL_FLAG_NO_APSS | RTL_FLAG_RUN | RTL_FLAG_STANDBY | RTL_FLAG_RST_REQ | RTL_FLAG_APSS_NOT_INITD
#define FLAGS_DCOM_RX_SLV_OUTBOX         RTL_FLAG_MSTR |                    RTL_FLAG_OBS | RTL_FLAG_ACTIVE | RTL_FLAG_MSTR_READY | RTL_FLAG_NO_APSS | RTL_FLAG_RUN | RTL_FLAG_STANDBY | RTL_FLAG_RST_REQ | RTL_FLAG_APSS_NOT_INITD
#define FLAGS_DCOM_TX_SLV_OUTBOX         RTL_FLAG_MSTR | RTL_FLAG_NOTMSTR | RTL_FLAG_OBS | RTL_FLAG_ACTIVE |                       RTL_FLAG_NO_APSS | RTL_FLAG_RUN | RTL_FLAG_STANDBY |                    RTL_FLAG_APSS_NOT_INITD

#define FLAGS_LOW_CORES_DATA             RTL_FLAG_MSTR | RTL_FLAG_NOTMSTR | RTL_FLAG_OBS | RTL_FLAG_ACTIVE | RTL_FLAG_MSTR_READY | RTL_FLAG_NO_APSS | RTL_FLAG_RUN |                                       RTL_FLAG_APSS_NOT_INITD
#define FLAGS_HIGH_CORES_DATA            RTL_FLAG_MSTR | RTL_FLAG_NOTMSTR | RTL_FLAG_OBS | RTL_FLAG_ACTIVE | RTL_FLAG_MSTR_READY | RTL_FLAG_NO_APSS | RTL_FLAG_RUN |                                       RTL_FLAG_APSS_NOT_INITD

//Start out with centaur tasks not running.
#define FLAGS_CENTAUR_DATA               RTL_FLAG_NONE
#define FLAGS_CENTAUR_CONTROL            RTL_FLAG_NONE

#define FLAGS_FAST_CORES_DATA            RTL_FLAG_MSTR | RTL_FLAG_NOTMSTR | RTL_FLAG_OBS | RTL_FLAG_ACTIVE | RTL_FLAG_MSTR_READY | RTL_FLAG_NO_APSS | RTL_FLAG_RUN |                                       RTL_FLAG_APSS_NOT_INITD

#define FLAGS_AMEC_SLAVE                 RTL_FLAG_MSTR | RTL_FLAG_NOTMSTR | RTL_FLAG_OBS | RTL_FLAG_ACTIVE | RTL_FLAG_MSTR_READY | RTL_FLAG_NO_APSS | RTL_FLAG_RUN |                                       RTL_FLAG_APSS_NOT_INITD
#define FLAGS_AMEC_MASTER                RTL_FLAG_MSTR |                    RTL_FLAG_OBS | RTL_FLAG_ACTIVE | RTL_FLAG_MSTR_READY | RTL_FLAG_NO_APSS | RTL_FLAG_RUN |                                       RTL_FLAG_APSS_NOT_INITD

#define FLAGS_APSS_START_MEAS            APSS_TASK_FLAGS
#define FLAGS_APSS_CONT_MEAS             APSS_TASK_FLAGS
#define FLAGS_APSS_DONE_MEAS             APSS_TASK_FLAGS

#define FLAGS_DCOM_PARSE_OCC_FW_MSG      RTL_FLAG_MSTR | RTL_FLAG_NOTMSTR | RTL_FLAG_OBS | RTL_FLAG_ACTIVE | RTL_FLAG_MSTR_READY | RTL_FLAG_NO_APSS | RTL_FLAG_RUN | RTL_FLAG_STANDBY | RTL_FLAG_RST_REQ | RTL_FLAG_APSS_NOT_INITD
#define FLAGS_CHECK_FOR_CHECKSTOP        RTL_FLAG_MSTR | RTL_FLAG_NOTMSTR | RTL_FLAG_OBS | RTL_FLAG_ACTIVE | RTL_FLAG_MSTR_READY | RTL_FLAG_NO_APSS | RTL_FLAG_RUN | RTL_FLAG_STANDBY | RTL_FLAG_RST_REQ | RTL_FLAG_APSS_NOT_INITD

#define FLAGS_CORE_DATA_CONTROL          RTL_FLAG_MSTR | RTL_FLAG_NOTMSTR |                RTL_FLAG_ACTIVE | RTL_FLAG_MSTR_READY | RTL_FLAG_NO_APSS | RTL_FLAG_RUN |                                       RTL_FLAG_APSS_NOT_INITD
// Don't run watchdog poke in all cases, expecially if a reset request is pending
#define FLAGS_POKE_WDT RTL_FLAG_RUN | RTL_FLAG_MSTR | RTL_FLAG_NOTMSTR | RTL_FLAG_STANDBY | \
                       RTL_FLAG_OBS | RTL_FLAG_ACTIVE | RTL_FLAG_MSTR_READY | RTL_FLAG_NO_APSS | RTL_FLAG_APSS_NOT_INITD

// Global tick sequences
// The number and size of these will vary as the real tick sequences are developed over time.

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
task_t G_task_table[TASK_END] = {
    // flags,                      func_ptr,                       data_ptr,// task_id_t
    { FLAGS_APSS_START_MEAS,       task_apss_start_pwr_meas,       NULL },  // TASK_ID_APSS_START
    { FLAGS_LOW_CORES_DATA,        task_core_data,                 (void *) &G_low_cores},
    { FLAGS_APSS_CONT_MEAS,        task_apss_continue_pwr_meas,    NULL },  // TASK_ID_APSS_CONT
    { FLAGS_HIGH_CORES_DATA,       task_core_data,                 (void *) &G_high_cores},
    { FLAGS_APSS_DONE_MEAS,        task_apss_complete_pwr_meas,    NULL },  // TASK_ID_APSS_DONE
    { FLAGS_FAST_CORES_DATA,       task_fast_core_data,            NULL },
    { FLAGS_DCOM_RX_SLV_INBX,      task_dcom_rx_slv_inbox,         NULL },  // TASK_ID_DCOM_RX_INBX
    { FLAGS_DCOM_TX_SLV_INBX,      task_dcom_tx_slv_inbox,         NULL },  // TASK_ID_DCOM_TX_INBX
    { FLAGS_POKE_WDT,              task_poke_watchdogs,            NULL },  // TASK_ID_POKE_WDT
    { FLAGS_DCOM_WAIT_4_MSTR,      task_dcom_wait_for_master,      NULL },  // TASK_ID_DCOM_WAIT_4_MSTR
    { FLAGS_DCOM_RX_SLV_OUTBOX,    task_dcom_rx_slv_outboxes,      NULL },  // TASK_ID_DCOM_RX_OUTBX
    { FLAGS_DCOM_TX_SLV_OUTBOX,    task_dcom_tx_slv_outbox,        NULL },  // TASK_ID_DCOM_TX_OUTBX
    { FLAGS_DCOM_PARSE_OCC_FW_MSG, task_dcom_parse_occfwmsg,       NULL },  // TASK_ID_DCOM_PARSE_FW_MSG
    { FLAGS_CHECK_FOR_CHECKSTOP,   task_check_for_checkstop,       NULL },  // TASK_ID_CHECK_FOR_CHECKSTOP
    { FLAGS_AMEC_SLAVE,            task_amec_slave,                NULL },  // TASK_ID_AMEC_SLAVE
    { FLAGS_AMEC_MASTER,           task_amec_master,               NULL },  // TASK_ID_AMEC_MASTER
    { FLAGS_CENTAUR_DATA,          task_centaur_data,              (void *) &G_centaur_data_task}, // TASK_ID_CENTAUR_DATA
    { FLAGS_CORE_DATA_CONTROL,     task_core_data_control,         NULL },  // TASK_ID_CORE_DATA_CONTROL
    { FLAGS_CENTAUR_CONTROL,       task_centaur_control,           (void *) &G_centaur_control_task },  // TASK_ID_CENTAUR_CONTROL
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

