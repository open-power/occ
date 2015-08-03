/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/aplt/incl/cmdhDbugCmd.h $                         */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2015                        */
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

#ifndef _CMDHDBUGCMD_H
#define _CMDHDBUGCMD_H

//*************************************************************************
// Includes
//*************************************************************************
#include <occ_common.h>
#include <cmdh_fsp.h>
#include <cmdh_fsp_cmds.h>

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************

//*************************************************************************
// Structures
//*************************************************************************
// Structure that is passed into cmdhDbugcmd applet
// when it is called
typedef struct
{
  cmdh_fsp_cmd_t * i_cmd_ptr;
  cmdh_fsp_rsp_t * io_rsp_ptr;
} cmdhDbugCmdAppletArg_t;


/**
 * struct cmdh_dbug_inject_errl_query_t;
 * Used by debug applet to create elog, version 0.
 */
typedef struct __attribute__ ((packed))
{
    struct      cmdh_fsp_cmd_header;
    uint8_t     sub_cmd;
    char        comp[OCC_TRACE_NAME_SIZE];
}cmdh_dbug_inject_errl_query_t;

//*************************************************************************
// Globals
//*************************************************************************

//*************************************************************************
// Function Prototypes
//*************************************************************************

//*************************************************************************
// Functions
//*************************************************************************

#endif

