/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe1/ipc_func_tables.c $                              */
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
#include "ipc_async_cmd.h"
#include "gpe1_dimm.h"
#include "gpu_structs.h"

void gpe_dimm_control(ipc_msg_t* cmd, void* arg);
void gpe1_nop(ipc_msg_t* cmd, void* arg);
void gpe_reset_mem_deadman(ipc_msg_t* cmd, void* arg);
void gpe_24x7(ipc_msg_t* cmd, void* arg);
void gpe_mem_power_control(ipc_msg_t* cmd, void* arg);
void gpe_gpu_init(ipc_msg_t* cmd, void* arg);

#ifdef OCC_GPU_SUPPORT
void gpe_gpu_sm(ipc_msg_t* cmd, void* arg);
#else
void gpe_gpu_sm(ipc_msg_t* cmd, void* arg)
{
  // No GPU support.  The 405 should only be calling this on OCC GPU supported
  // systems.  Those systems require a different OCC GPE1 image with GPU support.
  // This is indication of an OCC image build issue.
  // Return error so the 405 can log an error and disable GPU monitoring.
  int      rc;
  ipc_async_cmd_t *async_cmd = (ipc_async_cmd_t*)cmd;
  gpu_sm_args_t *args = (gpu_sm_args_t*)async_cmd->cmd_data;

  // set error return code for no GPU support
  args->error.rc = GPE_RC_NO_GPU_SUPPORT;
  PK_TRACE("E>gpu_sm: No GPU support!");

  // Send back IPC response of success (IPC operation itself succeeded)
  // 405 will handle no support set in error
  rc = ipc_send_rsp(cmd, IPC_RC_SUCCESS);
  if(rc)
  {
      PK_TRACE("E>gpu_sm: Failed to send response back. Halting GPE1", rc);
      pk_halt();
  }
}
#endif

// Function table for multi target (common) functions
IPC_MT_FUNC_TABLE_START
IPC_HANDLER_DEFAULT                          // 0
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
IPC_HANDLER(gpe_dimm_sm, 0)                // 0 - IPC_ST_DIMM_SM_FUNCID
IPC_HANDLER(gpe_dimm_control, 0)           // 1 - IPC_ST_DIMM_CONTROL_FUNCID
IPC_HANDLER(gpe1_nop, 0)                   // 2 - IPC_ST_GPE1_NOP
IPC_HANDLER(gpe_reset_mem_deadman, 0)      // 3 - IPC_ST_RESET_MEM_DEADMAN
IPC_HANDLER(gpe_24x7, 0)                   // 4 - IPC_ST_24_X_7_FUNCID
IPC_HANDLER(gpe_mem_power_control, 0)      // 5 - IPC_ST_MEM_POWER_CONTROL_FUNCID
IPC_HANDLER(gpe_gpu_sm, 0)                 // 6 - IPC_ST_GPU_SM_FUNCID
IPC_HANDLER(gpe_gpu_init, 0)               // 7 - IPC_ST_GPE_GPU_INIT_FUNCID
IPC_HANDLER_DEFAULT                        // 8
IPC_HANDLER_DEFAULT                        // 9
IPC_HANDLER_DEFAULT                        // 10
IPC_HANDLER_DEFAULT                        // 11
IPC_HANDLER_DEFAULT                        // 12
IPC_HANDLER_DEFAULT                        // 13
IPC_HANDLER_DEFAULT                        // 14
IPC_HANDLER_DEFAULT                        // 15
IPC_ST_FUNC_TABLE_END
