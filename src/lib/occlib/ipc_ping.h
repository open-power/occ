/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/occlib/ipc_ping.h $                                   */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015                             */
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

typedef struct
{
    ipc_msg_t msg;
    KERN_SEMAPHORE sem;
}ipc_ping_cmd_t;

#ifdef IPC_ENABLE_PING
//server side ping message handler
void ipc_ping_handler(ipc_msg_t* cmd, void* arg);

//Initialize a ping command.  
int ipc_ping_cmd_init(ipc_ping_cmd_t* ping_cmd);

//function for handling the ping response on the local processor
void ipc_ping_response(ipc_msg_t* rsp, void* arg);

//Statically initialize a ping command
#define IPC_PING_CMD_CREATE(name) \
ipc_ping_cmd_t name = \
{\
    .msg = IPC_MSG_INIT(IPC_MT_PING, ipc_ping_response, 0), \
    .sem = KERN_SEMAPHORE_INITIALIZATION(0, 1) \
}

//blocking command that can be run in a thread context to ping another target
int ipc_ping(ipc_ping_cmd_t* ping_cmd, uint32_t target_id);
#endif /*IPC_ENABLE_PING*/
