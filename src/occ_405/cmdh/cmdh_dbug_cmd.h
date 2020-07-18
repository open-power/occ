/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/cmdh/cmdh_dbug_cmd.h $                            */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2020                        */
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

//*************************************************************************/
// Includes
//*************************************************************************/
#include <occ_common.h>
#include <cmdh_fsp.h>
#include <cmdh_fsp_cmds.h>

//*************************************************************************/
// Externs
//*************************************************************************/

//*************************************************************************/
// Macros
//*************************************************************************/

//*************************************************************************/
// Defines/Enums
//*************************************************************************/
///Max string length of trace component name
#define OCC_TRACE_NAME_SIZE     4

// Size of data returned for debug DMA read
#define CMDH_DEBUG_DMA_READ_SIZE  1024

// Enum of the various Debug commands that may be sent to OCC
// over the TMGT<->OCC interface.  This is the first byte of data in the debug command
typedef enum
{
    DBUG_DUMP_WOF_DATA      = 0x01,
    DBUG_FORCE_WOF_RESET    = 0x02,
    DBUG_GET_TRACE          = 0x03,
    DBUG_CLEAR_TRACE        = 0x04,
    DBUG_ALLOW_TRACE        = 0x05,
    DBUG_DUMP_OPPB          = 0x06,
    DBUG_GET_AME_SENSOR     = 0x07,
    DBUG_DUMP_GPU_TIMINGS   = 0x08,
    DBUG_PEEK               = 0x09,
//  free = 0x0A,
//  free = 0x0B,
//  free = 0x0C,
    DBUG_DUMP_RAW_AD        = 0x0D,
    DBUG_CLEAR_ERRH         = 0x0E,  // clear all error history counters
//  free = 0x0F,
//  free = 0x10,
//  free = 0x11,
    DBUG_TRIGGER_EPOW       = 0x12,
    DBUG_INJECT_PGPE_ERRL   = 0x13,
    DBUG_INJECT_ERRL        = 0x14,
    DBUG_DIMM_INJECT        = 0x15,
//  free  = 0x16
//  free  = 0x17,
//  free  = 0x18,
//  free  = 0x19,
//  free  = 0x1A,
//  free  = 0x1B,
//  free  = 0x1C,
//  free  = 0x1D,
//  free  = 0x1E,
    DBUG_INTERNAL_FLAGS     = 0x1F,
    DBUG_FLUSH_DCACHE       = 0x20,
//  free  = 0x21,
    DBUG_CENTAUR_SENSOR_CACHE = 0x22,
    DBUG_DUMP_PROC_DATA     = 0x23,
    DBUG_GEN_CHOM_LOG       = 0x24,
    DBUG_DUMP_APSS_DATA     = 0x25,
    DBUG_DUMP_AME_SENSOR    = 0x26,
    DBUG_CLEAR_AME_SENSOR   = 0x27,
    DBUG_WOF_CONTROL        = 0x28,
    DBUG_WOF_OCS            = 0x29
} DBUG_CMD;

//*************************************************************************/
// Structures
//*************************************************************************/

// Used by OCC tool to get trace, version 0.
typedef struct __attribute__ ((packed))
{
    struct    cmdh_fsp_cmd_header;
    int8_t    sub_cmd;
    int16_t   size_request;
    int8_t    comp[OCC_TRACE_NAME_SIZE];
}cmdh_dbug_get_trace_query_t;

// Used by OCC to response "get trace" cmd, version 0.
typedef struct __attribute__ ((packed))
{
    struct    cmdh_fsp_rsp_header;
    uint8_t   data[0];
}cmdh_dbug_get_trace_resp_t;

#define CMDH_DBUG_GET_TRACE_RESP_LEN 6  // size_request(2) and occ_comp_sram_offset(4)

// Used by occtool to get AME sensor data
typedef struct __attribute__ ((packed))
{
    struct     cmdh_fsp_cmd_header;
    uint8_t    sub_cmd;
    uint16_t   type;
    uint16_t   location;
}cmdh_dbug_get_sensor_query_t;

// Max number of sensors that can be returned with cmdh_dbug_get_ame_sensor command (subtract 2 bytes for num_sensors)
#define CMDH_DBUG_MAX_NUM_SENSORS  ((CMDH_FSP_RSP_DATA_SIZE-2) / sizeof(cmdh_dbug_sensor_list_t))

// Size of standard response header (5 bytes) plus checksum (2 bytes)
#define CMDH_DBUG_FSP_RESP_LEN     7

// Used by OCC firmware to respond "cmdh_dbug_get_ame_sensor" debug command
typedef struct __attribute__ ((packed))
{
    struct                  cmdh_fsp_rsp_header;
    uint16_t                num_sensors;
    cmdh_dbug_sensor_list_t sensor[CMDH_DBUG_MAX_NUM_SENSORS];
}cmdh_dbug_get_sensor_resp_t;

/**
 * struct cmdh_dbug_inject_errl_query_t;
 * Used by debug command to create elog, version 0.
 */
typedef struct __attribute__ ((packed))
{
    struct      cmdh_fsp_cmd_header;
    uint8_t     sub_cmd;
    char        comp[OCC_TRACE_NAME_SIZE];
}cmdh_dbug_inject_errl_query_t;

// Used by OCC to debug on real hardware
typedef struct __attribute__ ((packed))
{
    struct    cmdh_fsp_cmd_header;
    uint8_t   sub_cmd;
    uint8_t   type;
    uint16_t  size;
    uint32_t  address;
}cmdh_dbug_peek_t;

typedef struct __attribute__ ((packed))
{
    uint8_t     func;
    uint16_t    raw;
    uint16_t    calculated;
    uint32_t    ipmi_sid;
    uint32_t    offset;
    uint32_t    gain;
}cmdh_dbug_apss_data_t;

// Used to get the APSS raw values
typedef struct __attribute__ ((packed))
{
    struct                  cmdh_fsp_rsp_header;
    cmdh_dbug_apss_data_t   ApssCh[MAX_APSS_ADC_CHANNELS];
    uint8_t                 checksum[2];
} cmdh_dbug_apss_data_resp_t;

// DBUG_DUMP_AME_SENSOR command struct
typedef struct __attribute__ ((packed))
{
    struct   cmdh_fsp_cmd_header;  // Standard command header
    uint8_t  sub_cmd;              // Debug sub-command
    uint16_t gsid;                 // Global Sensor ID
} cmdh_dbug_dump_ame_sensor_cmd_t;

// DBUG_DUMP_AME_SENSOR response struct
typedef struct __attribute__ ((packed))
{
    struct        cmdh_fsp_rsp_header;               // Standard response header
    sensor_info_t sensor_info;                       // Static sensor fields
    sensor_t      sensor;                            // Dynamic sensor fields
    uint8_t       checksum[CMDH_FSP_CHECKSUM_SIZE];  // Checksum
} cmdh_dbug_dump_ame_sensor_rsp_t;

// DBUG_CLEAR_AME_SENSOR command struct
typedef struct __attribute__ ((packed))
{
    struct   cmdh_fsp_cmd_header;  // Standard command header
    uint8_t  sub_cmd;              // Debug sub-command
    uint16_t gsid;                 // Global Sensor ID
    uint16_t clear_type;           // Fields to clear (AMEC_SENSOR_CLEAR_TYPE)
} cmdh_dbug_clear_ame_sensor_cmd_t;

// DBUG_CLEAR_AME_SENSOR response struct
typedef struct __attribute__ ((packed))
{
    struct   cmdh_fsp_rsp_header;               // Standard response header
    sensor_t sensor;                            // Dynamic sensor fields
    uint8_t  checksum[CMDH_FSP_CHECKSUM_SIZE];  // Checksum
} cmdh_dbug_clear_ame_sensor_rsp_t;

// DBUG_WOF_CONTROL command struct
typedef struct __attribute__ ((packed))
{
    struct      cmdh_fsp_cmd_header;    // Standard command header
    uint8_t     sub_cmd;                // Debug sub-command
    uint8_t     action;                 // CLEAR(0) or SET(1)
    uint32_t    wof_rc;                 // Bit to set
} cmdh_dbug_wof_control_cmd_t;

// DBUG_WOF_CONTROL response struct
typedef struct __attribute__ ((packed))
{
    struct      cmdh_fsp_rsp_header;
    uint32_t    wof_disabled;
    uint8_t     checksum[CMDH_FSP_CHECKSUM_SIZE];
} cmdh_dbug_wof_control_rsp_t;

// DBUG_WOF_OCS command struct
typedef struct __attribute__ ((packed))
{
    struct      cmdh_fsp_cmd_header;    // Standard command header
    uint8_t     sub_cmd;                // Debug sub-command
    uint16_t    ceff_up_amount;           // OCS Ceff Addr going up
    uint16_t    ceff_down_amount;         // OCS Ceff Addr going down
} cmdh_dbug_wof_ocs_cmd_t;

// DBUG_WOF_OCS response struct
typedef struct __attribute__ ((packed))
{
    struct      cmdh_fsp_rsp_header;
    uint16_t    ceff_up_amount;
    uint16_t    ceff_down_amount;
    uint8_t     checksum[CMDH_FSP_CHECKSUM_SIZE];
} cmdh_dbug_wof_ocs_rsp_t;

// DBUG_ALLOW_TRACE command struct
typedef struct __attribute__ ((packed))
{
    struct      cmdh_fsp_cmd_header;
    uint8_t     sub_cmd;
    uint8_t     action;
    uint16_t    trace_flags;
}cmdh_dbug_allow_trace_cmd_t;

// DBUG_ALLOW_TRACE response struct
typedef struct __attribute__ ((packed))
{
    struct      cmdh_fsp_rsp_header;
    uint16_t    trace_flags;
    uint8_t     checksum[CMDH_FSP_CHECKSUM_SIZE];
}cmdh_dbug_allow_trace_rsp_t;

// DBUG_DIMM_INJECT command struct
typedef struct __attribute__ ((packed))
{
    struct      cmdh_fsp_cmd_header;
    uint8_t     sub_cmd;
    uint64_t    inject_mask;
}cmdh_dbug_dimm_inject_cmd_t;

// DBUG_DIMM_INJECT response struct
typedef struct __attribute__ ((packed))
{
    struct      cmdh_fsp_rsp_header;
    uint64_t    inject_mask;
    uint8_t     checksum[CMDH_FSP_CHECKSUM_SIZE];
}cmdh_dbug_dimm_inject_rsp_t;

// DBUG_INTERNAL_FLAGS command struct
typedef struct __attribute__ ((packed))
{
    struct      cmdh_fsp_cmd_header;
    uint8_t     sub_cmd;
    uint32_t    flags;
}cmdh_dbug_internal_flags_cmd_t;

// DBUG_INTERNAL_FLAGS response struct
typedef struct __attribute__ ((packed))
{
    struct      cmdh_fsp_rsp_header;
    uint32_t    flags;
    uint8_t     checksum[CMDH_FSP_CHECKSUM_SIZE];
}cmdh_dbug_internal_flags_rsp_t;

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

