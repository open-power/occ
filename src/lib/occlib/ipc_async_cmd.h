/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/occlib/ipc_async_cmd.h $                              */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2016                        */
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
#ifndef __IPC_ASYNC_CMD_H__
#define __IPC_ASYNC_CMD_H__

/// \file ipc_async_cmd.h
/// \brief This header is shared between the 405 and GPE's that need to
/// support IPC command messages that are sent from the occhw_async_gpe.c
/// code.
///

#include "ipc_api.h"

#ifndef __ASSEMBLER__

typedef struct {
    ipc_msg_t   cmd;
    void*       cmd_data;
}ipc_async_cmd_t;

#endif /*__ASSEMBLER__*/
#endif /*__IPC_ASYNC_CMD_H__*/
