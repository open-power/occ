/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/cmdh/cmdh_fsp_cmds.h $                            */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2017                        */
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

#ifndef _CMDH_FSP_CMDS_H
#define _CMDH_FSP_CMDS_H

#include "ssx.h"
#include "cmdh_service_codes.h"
#include "errl.h"
#include "trac.h"
#include "rtls.h"
#include "occ_common.h"
#include "state.h"
#include "cmdh_fsp.h"
//#include "gpsm.h"
//#include "pstates.h"
#include "cmdh_fsp_cmds_datacnfg.h"
#include "sensor.h"
#include "apss.h"

// Enum of the various commands that TMGT may send to OCC
typedef enum
{
    CMDH_POLL                   = 0x00,
    CMDH_CLEARERRL              = 0x12,
    CMDH_SETMODESTATE           = 0x20,
    CMDH_SETCONFIGDATA          = 0x21,
    CMDH_SET_USER_PCAP          = 0x22,
    CMDH_RESET_PREP             = 0x25,
    CMDH_SNAPSHOT_SYNC          = 0x26,
    CMDH_GET_SNAPSHOT_BUFFER    = 0x35,
    CMDH_DEBUGPT                = 0x40,
    CMDH_AME_PASS_THROUGH       = 0x41,
    CMDH_GET_FIELD_DEBUG_DATA   = 0x42,
    CMDH_MFG_TEST_CMD           = 0x53,
    CMDH_TUNABLE_PARMS          = 0x60,
} eCmdhCommands;

#define SENSOR_TEMP "TEMP"
#define SENSOR_FREQ "FREQ"
#define SENSOR_POWR "POWR"
#define SENSOR_CAPS "CAPS"

//---------------------------------------------------------
// Poll Command
//---------------------------------------------------------

// Length of Poll Response
#define CMDH_POLL_RESP_LEN_V20 40 // version 0x20 has at least 40 bytes.
// Poll Version 0x20
#define CMDH_POLL_VERSION20  0x20

// Struct used to parse Poll Cmd
typedef struct __attribute__ ((packed)) cmdh_poll_query
{
    // Standard TMGT
    struct    cmdh_fsp_cmd_header;
    // Poll Version
    uint8_t   version;
}cmdh_poll_query_t;

// Response packet used for Poll Cmd
typedef struct __attribute__ ((packed)) cmdh_poll_resp_v20
{
    // Standard TMGT/HTMGT
    struct    cmdh_fsp_rsp_header;
    // BYTE  1: Status
    union
    {
        struct
        {
            uint8_t master_occ     : 1;   // 1 => master, 0 => slave
            uint8_t fir_master     : 1;   // 1 => fir master.
            uint8_t _reserved_5    : 1;
            uint8_t _reserved_4    : 1;
            uint8_t simics         : 1;   // 1 => running in SIMICS environment
            uint8_t _reserved_2    : 1;
            uint8_t obs_ready      : 1;   // 1 => OCC received all data to support obs state.
            uint8_t active_ready   : 1;   // 1 => OCC received all data to support active state.
        };
        uint8_t word;
    } status;
    // BYTE  2: Extended Status
    union
    {
        struct
        {
            uint8_t dvfs_due_to_ot  : 1;   // 1 => OCC clipped max Pstate due to an over temp.
            uint8_t dvfs_due_to_pwr : 1;   // 1 => OCC clipped max Psate due to reaching pcap limit.
            uint8_t mthrot_due_to_ot: 1;   // 1 => OCC throttled memory due to an over temp.
            uint8_t n_power         : 1;   // 1 => Server running without redundant power.
            uint8_t _reserved_3     : 1;
            uint8_t sync_request    : 1;   // 1 => OCC needs to restart snapshot buffers
            uint8_t _reserved_1     : 1;
            uint8_t _reserved_0     : 1;
        };
        uint8_t word;
    } ext_status;
    // BYTE  3: OCCs Present
    uint8_t   occ_pres_mask;
    // BYTE  4: Config Data Requested
    uint8_t   config_data;
    // BYTE  5: Current OCC State
    uint8_t   state;
    // BYTE  6: Current OCC Mode
    uint8_t   mode;
    // BYTE  7: Current Idle Power Saver Status
    union
    {
        struct
        {
            uint8_t _reserved_7     : 1;
            uint8_t _reserved_6     : 1;
            uint8_t _reserved_5     : 1;
            uint8_t _reserved_4     : 1;
            uint8_t _reserved_3     : 1;
            uint8_t _reserved_2     : 1;
            uint8_t ips_active      : 1;
            uint8_t ips_enabled     : 1;
        };
        uint8_t word;
    } ips_status;
    // BYTE  8: Error Log ID
    uint8_t   errl_id;
    // BYTES  9 - 12: Error Log Start Address
    uint32_t  errl_address;
    // BYTES 13 - 14: Error Log Length
    uint16_t  errl_length;
    // BYTES 15 - 16: Reserved
    uint8_t   _reserved_15;
    uint8_t   _reserved_16;
    // BYTES 17 - 32 (16 bytes): OCC Code Level - ASCII string of OCC build level currently running.
    uint8_t   occ_level[16];
    // BYTES 33 - 38 (6 bytes):  ASCII eye catcher "SENSOR"
    uint8_t   sensor_ec[6];
    // BYTE  39: Number of sensor data blocks
    uint8_t   sensor_dblock_count;
    // BYTE  40: Sensor Data Block Header Version
    uint8_t   sensor_dblock_version;
    // No need to include the 2 bytes for checksum since they get added prior to sending
    // data back to tmgt.
}cmdh_poll_resp_v20_fixed_t;

typedef struct __attribute__ ((packed)) cmdh_poll_sensor_datablock
{
    uint8_t eyecatcher[4];
    uint8_t reserved;
    uint8_t format;
    uint8_t length;
    uint8_t count;
} cmdh_poll_sensor_db_t;

typedef struct __attribute__ ((packed)) cmdh_poll_temp_sensor
{
    uint32_t id;     // Sensor id.
    uint8_t  fru_type;
    uint8_t  value;  // current temperature sensor reading in degrees C
} cmdh_poll_temp_sensor_t;

typedef struct __attribute__ ((packed)) cmdh_poll_freq_sensor
{
    uint32_t id;     // Id to represent the frequency.
    uint16_t value;  // current frequency in MHZ
} cmdh_poll_freq_sensor_t;

// Only available from master occ.
typedef struct __attribute__ ((packed)) cmdh_poll_powr_sensor
{
    uint32_t id;            // Sensor id - to represent the power.
    uint8_t  function_id;   // Identify what the reading is for (ADC_CHANNEL_ID in xml file)
    uint8_t  apss_channel;  // APSS channel that the power was read from
    uint16_t reserved;
    uint32_t update_tag;    // Count of number of 250us samples represented by accumulator.
    uint64_t accumul;       // Accumulation of 250us power readings
    uint16_t current;       // Most recent 250us reading in watts.
} cmdh_poll_power_sensor_t;

// Only available from master occ.
typedef struct __attribute__ ((packed)) cmdh_poll_caps_sensor
{
    uint16_t current;   // Current power cap in 1W units.
    uint16_t system;    // Current system power in 1W units.
    uint16_t n;         // Oversubscription; output pcap limit when theres no redundant power.
    uint16_t max;       // Maximum power cap in 1W units.
    uint16_t min;       // Minimum power cap in 1W units.
    uint16_t user;      // Power cap set by user in 1W units.
    uint8_t  source;    // User Power Limit Source
} cmdh_poll_pcaps_sensor_t;

//---------------------------------------------------------
// Query FW Level Command
//---------------------------------------------------------

// Response packet used for Query Firmware Level Cmd
#define CMDH_FW_QUERY_RESP_LEN 16
typedef struct __attribute__ ((packed)) cmdh_fw_resp
{
    // Standard TMGT
    struct    cmdh_fsp_rsp_header;
    // Firmware Level (4 bytes)
    uint8_t   fw_level[CMDH_FW_QUERY_RESP_LEN];
    // Checksum
    uint8_t   checksum[2];
}cmdh_fw_resp_t;

extern uint8_t G_apss_ch_to_function[MAX_APSS_ADC_CHANNELS];
//---------------------------------------------------------
// Set Mode And State Command
//---------------------------------------------------------

// Query packet used by the FSP for setting the TPMF state, version 0.
struct smgr_setmodestate_v0_query
{
    // Standard TMGT
    struct    cmdh_fsp_cmd_header;
    // Version
    uint8_t       version;
    // New state from OCC_STATE_*
    uint8_t       occ_state;
    // New mode from OCC_MODE_*
    uint8_t       occ_mode;
} __attribute__ ((__packed__));

typedef struct smgr_setmodestate_v0_query smgr_setmodestate_v0_query_t;

#define SMGR_SETMODESTATE_CMD_LEN    3

// Complete Response packet for a set mode / state command
#define SMGR_SETMODESTATE_RESP_LEN   0
typedef cmdh_fsp_rsp_t smgr_setmodestate_resp_t;

// Set-mode-state TMGT command version
#define SMGR_SMS_CMD_VERSION         0

//---------------------------------------------------------
// Clear Elog Command
//---------------------------------------------------------

// Used by TMGT to clear elog data state, version 0.
typedef struct __attribute__ ((packed))
{
    struct    cmdh_fsp_cmd_header;
    uint8_t   elog_id;
}cmdh_clear_elog_query_t;

//---------------------------------------------------------
// Get Elog Command
//---------------------------------------------------------

// Used by TMGT to response occ elog data, version 0.
typedef struct __attribute__ ((packed))
{
    struct    cmdh_fsp_rsp_header;
    uint32_t  oci_address;
    uint8_t   elog_id;
}cmdh_get_elog_resp_t;

#define CMDH_GET_ELOG_RESP_LEN 5

//---------------------------------------------------------
// Reset Prep
//---------------------------------------------------------

// Minimum Data Length for Reset Prep Command
#define CMDH_RESET_PREP_MIN_DATALEN 1

// Version 0
#define CMDH_RESET_PREP_VERSION 0

// Reason
typedef enum
{
    // Non-failure. Code update, external user request (i.e. to load
    // new pState table). No FFDC error logs should be generated.
    CMDH_PREP_NONFAILURE       = 0x00,
    // Failure detected on this OCC. FFDC error log should be generated.
    CMDH_PREP_FAILON_THISOCC   = 0x01,
    // Failure detected on a different OCC within same node.
    // FFDC log is optional, if this OCC is master OCC it may want to
    // generate FFDC log.
    CMDH_PREP_FAILON_OTHEROCC  = 0x02,
    // Failure detected on a different OCC in different node. No
    // FFDC error log should be generated. Current assumption is that an
    // OCC in a different node should never be reason for an OCC failure.
    CMDH_PREP_FAILON_OTHERNODE = 0x03,
    // System is powering off, stop RTL tasks
    CMDH_PREP_POWER_OFF = 0xFF,
} eCmdhResetPrepReason;

#define CMDH_RESET_PREP_TRACE_SIZE 3072

// Struct used to parse Reset Prep Command
typedef struct __attribute__ ((packed)) cmdh_reset_prep
{
    // Standard TMGT
    struct    cmdh_fsp_cmd_header;
    // Poll Version
    uint8_t   version;
    // Reason
    uint8_t   reason;
}cmdh_reset_prep_t;

//---------------------------------------------------------
// Debug Command
//---------------------------------------------------------

///Max string length of trace component name
#define OCC_TRACE_NAME_SIZE     4

// Enum of the various Debug commands that may be sent to OCC
// over the TMGT<->OCC interface.
typedef enum
{
    DBUG_READ_SCOM          = 0x01,
    DBUG_PUT_SCOM           = 0x02,
    DBUG_GET_TRACE          = 0x03,
    DBUG_CLEAR_TRACE        = 0x04,
    // free = 0x05
    DBUG_SET_PEXE_EVENT     = 0x06,
    DBUG_GET_AME_SENSOR     = 0x07,
    // free = 0x08,
    DBUG_PEEK               = 0x09,
    DBUG_POKE               = 0x0A,
    DBUG_DUMP_THEMAL        = 0x0B,
    DBUG_DUMP_POWER         = 0x0C,
    DBUG_DUMP_RAW_AD        = 0x0D,
    DBUG_DVFS_SLEW          = 0x0E,
    DBUG_MEM_PWR_CTL        = 0x0F,
    DBUG_PERFCOUNT          = 0x10,
    DBUG_TEST_INTF          = 0x11,
    DBUG_SET_BUS_SPEED      = 0x12,
    DBUG_FAN_CONTROL        = 0x13,
    DBUG_INJECT_ERRL        = 0x14,
    DBUG_IIC_READ           = 0x15,
    DBUG_IIC_WRITE          = 0x16,
    DBUG_GPIO_READ          = 0x17,
    DBUG_FSP_ATTN           = 0x18,
    DBUG_CALCULATE_MAX_DIFF = 0x19,
    DBUG_FORCE_ELOG         = 0x1A,
    DBUG_SWITCH_PHASE       = 0x1B,
    DBUG_INJECT_ERR         = 0x1C,
    DBUG_VERIFY_V_F         = 0x1D,
    DBUG_DUMP_PPM_DATA      = 0x1E,

    DBUG_FLUSH_DCACHE       = 0x20,
    DBUG_INVALIDATE_DCACHE  = 0x21,
    DBUG_CENTAUR_SENSOR_CACHE = 0x22,
    DBUG_DUMP_PROC_DATA     = 0x23,
    DBUG_GEN_CHOM_LOG       = 0x24,
    DBUG_DUMP_APSS_DATA     = 0x25,
    DBUG_DUMP_AME_SENSOR    = 0x26,
    DBUG_CLEAR_AME_SENSOR   = 0x27
} DBUG_CMD;

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

typedef struct cmdh_dbug_sensor_list
{
    char        name[MAX_SENSOR_NAME_SZ];
    uint16_t    gsid;
    uint16_t    sample;
    uint16_t    sample_min;
    uint16_t    sample_max;
    uint32_t    ipmi_sid;
}cmdh_dbug_sensor_list_t;

// Max number of sensors that can be returned with cmdh_dbug_get_ame_sensor command
#define CMDH_DBUG_MAX_NUM_SENSORS  50
// Size of standard response header (5 bytes) plus checksum (2 bytes)
#define CMDH_DBUG_FSP_RESP_LEN     7

// Used by OCC firmware to respond "cmdh_dbug_get_ame_sensor" debug command
typedef struct __attribute__ ((packed))
{
    struct                  cmdh_fsp_rsp_header;
    uint16_t                num_sensors;
    cmdh_dbug_sensor_list_t sensor[CMDH_DBUG_MAX_NUM_SENSORS];
    uint8_t                 filler;
    uint16_t                checksum;
}cmdh_dbug_get_sensor_resp_t;

// Size of trace data for a trace buffer
#define CMDH_FIELD_TRACE_DATA_SIZE  1024
// Max number of sensors for field debug data
#define CMDH_FIELD_MAX_NUM_SENSORS  50

// Used by OCC to response "GET_FIELD_DEBUG_DATA" cmd
typedef struct __attribute__ ((packed))
{
    struct                  cmdh_fsp_rsp_header;
    // OCC node
    uint8_t                 occ_node;
    // OCC id
    uint8_t                 occ_id;
    // OCC role
    uint8_t                 occ_role;
    // ERR trace
    char                    trace_err[CMDH_FIELD_TRACE_DATA_SIZE];
    // INF trace
    char                    trace_inf[CMDH_FIELD_TRACE_DATA_SIZE];
    // Number of Sensors
    uint16_t                num_sensors;
    // Sensors list
    cmdh_dbug_sensor_list_t sensor[CMDH_FIELD_MAX_NUM_SENSORS];
    // Checksum
    uint8_t   checksum[2];
}cmdh_get_field_debug_data_resp_t;

// Used by TMGT to get field debug data
typedef struct __attribute__ ((packed))
{
    struct     cmdh_fsp_cmd_header;
    uint8_t    version;
}cmdh_get_field_debug_data_query_t;

// Used by OCC to debug on real hardware
typedef struct __attribute__ ((packed))
{
    struct    cmdh_fsp_cmd_header;
    uint8_t   sub_cmd;
    uint8_t   type;
    uint16_t  size;
    uint32_t  oci_address;
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

//---------------------------------------------------------
// Tunable Parameter Command
//---------------------------------------------------------

// Default number of tunable parameters available
// NOTE: update this value if any new tunable parameter is added into the table
#define CMDH_DEFAULT_TUNABLE_PARAM_NUM   9
// Max size for string containing the tunable parameter name
#define CMDH_TUNABLE_PARAM_NAME_MAX_SIZE 60

typedef struct cmdh_tunable_param_table
{
    uint8_t             id;
    char                name[CMDH_TUNABLE_PARAM_NAME_MAX_SIZE];
    uint8_t             unit_type;
    uint16_t            flag;
    uint16_t            value;
    uint16_t            min;
    uint16_t            max;
}cmdh_tunable_param_table_t;

typedef struct cmdh_tunable_param_table_ext
{
    // Default value for this parameter
    uint16_t            def_value;
    // Multiplier to convert the input value to correct internal resolution
    uint16_t            multiplier;
    // Adjusted value for this parameter (adjusted for internal resolution)
    uint16_t            adj_value;
}cmdh_tunable_param_table_ext_t;

extern uint8_t G_mst_tunable_parameter_overwrite;
extern cmdh_tunable_param_table_ext_t G_mst_tunable_parameter_table_ext[CMDH_DEFAULT_TUNABLE_PARAM_NUM];
extern cmdh_tunable_param_table_t G_mst_tunable_parameter_table[CMDH_DEFAULT_TUNABLE_PARAM_NUM];

#define CMDH_SET_USER_PCAP_DATALEN 2

errlHndl_t cmdh_tmgt_setmodestate(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                        cmdh_fsp_rsp_t * i_rsp_ptr);

void cmdh_dbug_cmd (const cmdh_fsp_cmd_t * i_cmd_ptr,
                          cmdh_fsp_rsp_t * i_rsp_ptr);

errlHndl_t cmdh_tmgt_poll (const cmdh_fsp_cmd_t * i_cmd_ptr,
                                 cmdh_fsp_rsp_t * i_rsp_ptr);

errlHndl_t cmdh_clear_elog (const cmdh_fsp_cmd_t * i_cmd_ptr,
                                  cmdh_fsp_rsp_t * i_rsp_ptr);

errlHndl_t cmdh_amec_pass_through(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                        cmdh_fsp_rsp_t * i_rsp_ptr);

errlHndl_t cmdh_reset_prep(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                 cmdh_fsp_rsp_t * i_rsp_ptr);

errlHndl_t cmdh_tmgt_get_field_debug_data(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                                cmdh_fsp_rsp_t * i_rsp_ptr);

errlHndl_t cmdh_set_user_pcap(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                    cmdh_fsp_rsp_t * o_rsp_ptr);

#endif

