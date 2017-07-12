/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe0/ipc_func_tables.c $                              */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2017                        */
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
#include "ipc_api.h"
#include "ipc_ping.h"
#include "core_data.h"
#include "proc_shared.h"

void apss_init_gpio(ipc_msg_t* cmd, void* arg);
void apss_init_mode(ipc_msg_t* cmd, void* arg);
void apss_start_pwr_meas_read(ipc_msg_t* cmd, void* arg);
void apss_continue_pwr_meas_read(ipc_msg_t* cmd, void* arg);
void apss_complete_pwr_meas_read(ipc_msg_t* cmd, void* arg);
void gpe_get_core_data(ipc_msg_t* cmd, void* arg);
void gpe_get_nest_dts(ipc_msg_t* cmd, void* arg);
void gpe_get_tod(ipc_msg_t* cmd, void* arg);

void ipc_scom_operation(ipc_msg_t* cmd, void* arg);
void ipc_fir_collection(ipc_msg_t* cmd, void* arg);

void gpe0_nop(ipc_msg_t* cmd, void* arg);

extern ipc_msgq_t G_gpe0_test_msgq0;

// Function table for multi target (common) functions
IPC_MT_FUNC_TABLE_START

#ifdef IPC_ENABLE_PING
IPC_HANDLER(ipc_ping_handler, 0)             // 0 - IPC_MT_PING
#else
IPC_HANDLER_DEFAULT                          // 0
#endif

IPC_HANDLER_DEFAULT                          // 1
IPC_HANDLER_DEFAULT                          // 2
IPC_HANDLER_DEFAULT                          // 3
IPC_HANDLER_DEFAULT                          // 4
IPC_HANDLER_DEFAULT                          // 5
IPC_HANDLER_DEFAULT                          // 6
IPC_HANDLER_DEFAULT                          // 7
IPC_MT_FUNC_TABLE_END

// Function table for single target (processor-specific) functions
IPC_ST_FUNC_TABLE_START
IPC_MSGQ_HANDLER(&G_gpe0_test_msgq0)         // 0 - IPC_ST_TEST_FUNC0
IPC_HANDLER(apss_init_gpio, 0)               // 1 - IPC_ST_APSS_INIT_GPIO_FUNCID
IPC_HANDLER(apss_init_mode, 0)               // 2 - IPC_ST_APSS_INIT_MODE_FUNCID
IPC_HANDLER(apss_start_pwr_meas_read, 0)     // 3 - IPC_ST_APSS_START_PWR_MEAS_READ_FUNCID
IPC_HANDLER(apss_continue_pwr_meas_read, 0)  // 4 - IPC_ST_APSS_CONTINUE_PWR_MEAS_READ_FUNCID
IPC_HANDLER(apss_complete_pwr_meas_read, 0)  // 5 - IPC_ST_APSS_COMPLETE_PWR_MEAS_READ_FUNCID
IPC_HANDLER(gpe_get_core_data, 0)            // 6 - IPC_ST_GET_CORE_DATA_FUNCID
IPC_HANDLER(ipc_scom_operation, 0)           // 7 - IPC_ST_SCOM_OPERATION
IPC_HANDLER(gpe0_nop, 0)                     // 8 - IPC_ST_GPE0_NOP
IPC_HANDLER(gpe_get_nest_dts, 0)             // 9 - IPC_ST_GET_NEST_DTS_FUNCID
IPC_HANDLER(ipc_fir_collection, 0)           // 10 - IPC_ST_FIR_COLLECTION
IPC_HANDLER(gpe_get_tod, 0)                  // 11 - IPC_ST_GET_TOD_FUNCID
IPC_HANDLER_DEFAULT                          // 12
IPC_HANDLER_DEFAULT                          // 13
IPC_HANDLER_DEFAULT                          // 14
IPC_HANDLER_DEFAULT                          // 15
IPC_ST_FUNC_TABLE_END
