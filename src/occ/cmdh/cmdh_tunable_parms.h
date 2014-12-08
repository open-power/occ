/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/cmdh/cmdh_tunable_parms.h $                           */
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

#ifndef CMDH_TUNABLE_PARMS_H
#define CMDH_TUNABLE_PARMS_H

#include "cmdh_fsp_cmds.h"

typedef enum {
    TUNABLE_PARMS_QUERY     = 0x00,
    TUNABLE_PARMS_WRITE     = 0x01,
    TUNABLE_PARMS_RESTORE   = 0x02,
} TUNABLE_PARMS_CMD;


// Used by OCC to get tunable parms query command
typedef struct __attribute__ ((packed))
{
    struct    cmdh_fsp_cmd_header;
    uint8_t   sub_cmd;
    uint8_t   version;
}tunable_parms_query_cmd_t;

#define TUNABLE_PARMS_MAX_PARMS 29

#define TUNABLE_PARMS_QUERY_VERSION 0
#define TUNABLE_PARMS_WRITE_VERSION 0

typedef struct __attribute__ ((packed))
{
    uint8_t id;
    uint8_t value[2];
}tunable_parm_write_entry_t;

// Used by OCC to get tunable parms write command
typedef struct __attribute__ ((packed))
{
    struct                      cmdh_fsp_cmd_header;
    uint8_t                     sub_cmd;
    uint8_t                     version;
    uint8_t                     numParms;
    tunable_parm_write_entry_t  data[TUNABLE_PARMS_MAX_PARMS];
}tunable_parms_write_cmd_t;


// Used by OCC to get tunable parms query response
typedef struct __attribute__ ((packed))
{
    struct                      cmdh_fsp_rsp_header;
    uint8_t                     version;
    uint8_t                     numParms;
    cmdh_tunable_param_table_t  data[TUNABLE_PARMS_MAX_PARMS];
}tunable_parms_query_rsp_t;

errlHndl_t cmdh_tunable_parms(  const cmdh_fsp_cmd_t * i_cmd_ptr,
                                cmdh_fsp_rsp_t * o_rsp_ptr);

uint8_t cmdh_tunable_parms_query(   const cmdh_fsp_cmd_t * i_cmd_ptr,
                                    cmdh_fsp_rsp_t * o_rsp_ptr);

uint8_t cmdh_tunable_parms_write(   const cmdh_fsp_cmd_t * i_cmd_ptr,
                                    cmdh_fsp_rsp_t * o_rsp_ptr);

uint8_t cmdh_tunable_parms_restore( const cmdh_fsp_cmd_t * i_cmd_ptr,
                                    cmdh_fsp_rsp_t * o_rsp_ptr);


#endif
