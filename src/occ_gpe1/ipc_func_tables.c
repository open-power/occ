#include "ipc_api.h"
#include "gpe1_dimm.h"


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
IPC_HANDLER(gpe_dimm_sm, 0)                  // 0 - IPC_ST_DIMM_SM_FUNCID
IPC_HANDLER_DEFAULT                          // 1
IPC_HANDLER_DEFAULT                          // 2
IPC_HANDLER_DEFAULT                          // 3
IPC_HANDLER_DEFAULT                          // 4
IPC_HANDLER_DEFAULT                          // 5
IPC_HANDLER_DEFAULT                          // 6
IPC_HANDLER_DEFAULT                          // 7
IPC_HANDLER_DEFAULT                          // 8
IPC_HANDLER_DEFAULT                          // 9
IPC_HANDLER_DEFAULT                          // 10
IPC_HANDLER_DEFAULT                          // 11
IPC_HANDLER_DEFAULT                          // 12
IPC_HANDLER_DEFAULT                          // 13
IPC_HANDLER_DEFAULT                          // 14
IPC_HANDLER_DEFAULT                          // 15
IPC_ST_FUNC_TABLE_END
