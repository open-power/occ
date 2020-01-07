/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/cmdh/cmdh_mnfg_intf.h $                           */
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

#ifndef CMDH_MNFG_INTF_H
#define CMDH_MNFG_INTF_H

#include "cmdh_fsp.h"
#include "sensor.h"
#include "pstates_common.H"

typedef enum {
    MNFG_RUN_STOP_SLEW      = 0x02,
    MNFG_LIST_SENSORS       = 0x05,
    MNFG_GET_SENSOR         = 0x06,
    MNFG_OVERSUB_EMULATION  = 0x07,
    MNFG_MEMORY_SLEW        = 0x09,
    MNFG_READ_PSTATE_TABLE  = 0x0B,
    MNFG_SELECT_WOF_VRT     = 0x0F,
} MNFG_CMD;

#define MNFG_INTF_SLEW_START    0x00
#define MNFG_INTF_SLEW_STOP     0x01
#define MNFG_INTF_SINGLE_STEP   0x00
#define MNFG_INTF_FULL_SLEW     0x01

#define MNFG_INTF_RUN_STOP_SLEW_RSP_SIZE 6

// Used by OCC to get mnfg run/stop slew command
typedef struct __attribute__ ((packed))
{
    struct    cmdh_fsp_cmd_header;
    uint8_t   sub_cmd;
    uint8_t   version;
    uint8_t   action;
    uint8_t   bottom_mode;
    int8_t    bottom_percent;
    uint8_t   high_mode;
    int8_t    high_percent;
    uint8_t   step_mode;
    uint8_t   step_delay;
}mnfg_run_stop_slew_cmd_t;

// Used by OCC firmware to respond to mnfg run/stop slew command
typedef struct __attribute__ ((packed))
{
    struct    cmdh_fsp_rsp_header;
    uint16_t  slew_count;
    uint16_t  fstart;
    uint16_t  fstop;
    uint16_t  checksum;
}mnfg_run_stop_slew_rsp_t;

// Used by OCC to get mnfg memory slew command
typedef struct __attribute__ ((packed))
{
    struct    cmdh_fsp_cmd_header;
    uint8_t   sub_cmd;
    uint8_t   version;
    uint8_t   action;
}mnfg_mem_slew_cmd_t;

#define MNFG_INTF_MEM_SLEW_RSP_SIZE 2

// Used by OCC firmware to respond to mnfg memory slew command
typedef struct __attribute__ ((packed))
{
    struct    cmdh_fsp_rsp_header;
    uint16_t  slew_count;
    uint16_t  checksum;
}mnfg_mem_slew_rsp_t;

// Used by OCC to get mnfg emulate oversubscription command
typedef struct __attribute__ ((packed))
{
    struct    cmdh_fsp_cmd_header;
    uint8_t   sub_cmd;
    uint8_t   version;
    uint8_t   action;
    uint8_t   reserved;
}mnfg_emul_oversub_cmd_t;

// Used by OCC firmware to respond to mnfg emulate oversubscription command
typedef struct __attribute__ ((packed))
{
    struct    cmdh_fsp_rsp_header;
    uint8_t   state;
    uint16_t  checksum;
}mnfg_emul_oversub_rsp_t;

#define MFG_LIST_SENSOR_VERSION 0
// max number of sensors that can be returned with mfg test list sensors command
// subtract 2 bytes for non-sensor data fields in return data
// subtract 1 from number of sensors to allow +1 query to detect truncation
#define MFG_MAX_NUM_SENSORS ( ((CMDH_FSP_RSP_DATA_SIZE-2) / sizeof(sensorQueryList_t)) - 1 )

// Used by mfg to get sensor data
typedef struct __attribute__ ((packed))
{
    struct      cmdh_fsp_cmd_header;
    uint8_t     sub_cmd;
    uint8_t     version;
    uint16_t    start_gsid;
    uint8_t     present;
    uint16_t    location;
    uint16_t    type;
}cmdh_mfg_list_sensors_query_t;

// Used by OCC firmware to respond to the "MFG_LIST_SENSORS" mfg command.  Follows the TMGT/OCC specification.
typedef struct __attribute__ ((packed))
{
    struct                  cmdh_fsp_rsp_header;
    uint8_t                 truncated;
    uint8_t                 num_sensors;
    sensorQueryList_t       sensor[MFG_MAX_NUM_SENSORS];
    uint16_t                checksum;
}cmdh_mfg_list_sensors_resp_t;

#define MFG_GET_SENSOR_VERSION 0
// Used by mfg to get sensor data
typedef struct __attribute__ ((packed))
{
    struct      cmdh_fsp_cmd_header;
    uint8_t     sub_cmd;
    uint8_t     version;
    uint16_t    gsid;
}cmdh_mfg_get_sensor_query_t;

// Used by OCC firmware to respond to the "MFG_GET_SENSOR" mfg command. Follows the TMGT/OCC specification.
typedef struct __attribute__ ((packed))
{
    struct                  cmdh_fsp_rsp_header;
    uint16_t                gsid;
    uint16_t                sample;
    uint8_t                 status;
    uint32_t                accumulator;
    uint16_t                min;
    uint16_t                max;
    char                    name[MAX_SENSOR_NAME_SZ];
    char                    units[MAX_SENSOR_UNIT_SZ];
    uint32_t                freq;
    uint32_t                scalefactor;
    uint16_t                location;
    uint16_t                type;
    uint16_t                checksum;
}cmdh_mfg_get_sensor_resp_t;

#define MFG_SELECT_WOF_VRT_QUERY 0
#define MFG_SELECT_WOF_VRT_WRITE 1

// Used by OCC to mnfg request select WOF VRT command
typedef struct __attribute__ ((packed))
{
    struct    cmdh_fsp_cmd_header;
    uint8_t   sub_cmd;
    uint8_t   action;
    uint8_t   vcs_index;
    uint8_t   vdd_index;
    uint8_t   io_pwr_index;
    uint8_t   ambient_index;
    uint8_t   v_ratio_index;
}mnfg_select_wof_vrt_cmd_t;

// Used by OCC firmware to respond to mnfg request select WOF VRT command
typedef struct __attribute__ ((packed))
{
    struct    cmdh_fsp_rsp_header;
    uint16_t  vcs0;
    uint16_t  vcs_step;
    uint8_t   vcs_max_index;
    uint8_t   vcs_cur_index;
    uint16_t  reserved1;
    uint16_t  vdd0;
    uint16_t  vdd_step;
    uint8_t   vdd_max_index;
    uint8_t   vdd_cur_index;
    uint16_t  reserved2;
    uint16_t  io_pwr0;
    uint16_t  io_pwr_step;
    uint8_t   io_pwr_max_index;
    uint8_t   io_pwr_cur_index;
    uint16_t  reserved3;
    uint16_t  ambient0;
    uint16_t  ambient_step;
    uint8_t   ambient_max_index;
    uint8_t   ambient_cur_index;
    uint16_t  reserved4;
    uint16_t  v_ratio0;
    uint16_t  v_ratio_step;
    uint8_t   v_ratio_max_index;
    uint8_t   v_ratio_cur_index;
    uint16_t  reserved5;
}mnfg_select_wof_vrt_rsp_t;

#define MFG_PSTATE_READ_REQUEST_QUERY 0xFF
#define MFG_PSTATE_READ_QUERY_RSP_SIZE  8
#define MFG_PSTATE_READ_MAX_RSP_SIZE  1024

// Used by OCC to get mnfg read pstate table command

typedef struct __attribute__ ((packed))
{
    // Need 128 byte aligned buffer for BCE request
    uint8_t data[MFG_PSTATE_READ_MAX_RSP_SIZE];
} mfg_read_pstate_table_t __attribute ((aligned(128)));

typedef struct __attribute__ ((packed))
{
    struct    cmdh_fsp_cmd_header;
    uint8_t   sub_cmd;
    uint8_t   request;
}mnfg_read_pstate_table_cmd_t;

errlHndl_t cmdh_mnfg_test_parse (const cmdh_fsp_cmd_t * i_cmd_ptr,
                                     cmdh_fsp_rsp_t * o_rsp_ptr);

uint8_t cmdh_mnfg_emulate_oversub(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                        cmdh_fsp_rsp_t * o_rsp_ptr);

uint8_t cmdh_mnfg_list_sensors(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                     cmdh_fsp_rsp_t * o_rsp_ptr);

uint8_t cmdh_mnfg_get_sensor(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                     cmdh_fsp_rsp_t * o_rsp_ptr);

uint8_t cmdh_mnfg_run_stop_slew(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                      cmdh_fsp_rsp_t * o_rsp_ptr);

uint8_t cmdh_mnfg_select_wof_vrt(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                             cmdh_fsp_rsp_t * o_rsp_ptr);

uint8_t cmdh_mnfg_read_pstate_table(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                             cmdh_fsp_rsp_t * o_rsp_ptr);

#endif
