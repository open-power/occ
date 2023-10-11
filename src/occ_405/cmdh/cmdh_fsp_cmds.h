/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/cmdh/cmdh_fsp_cmds.h $                            */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2023                        */
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
#include "cmdh_fsp_cmds_datacnfg.h"
#include "sensor.h"
#include "apss.h"
#include "occ_sys_config.h"

// Enum of the various commands that may be sent to OCC
typedef enum
{
    CMDH_POLL                   = 0x00,
    CMDH_CLEARERRL              = 0x12,
    CMDH_OCMBRECOVERYSTATUS     = 0x13,
    CMDH_SETMODESTATE           = 0x20,
    CMDH_SETCONFIGDATA          = 0x21,
    CMDH_SET_USER_PCAP          = 0x22,
    CMDH_RESET_PREP             = 0x25,
    CMDH_SEND_AMBIENT_TEMP      = 0x30,
    CMDH_DEBUGPT                = 0x40,
    CMDH_AME_PASS_THROUGH       = 0x41,
    CMDH_GET_FIELD_DEBUG_DATA   = 0x42,
    CMDH_MFG_TEST_CMD           = 0x53,
    CMDH_CLEAR_SENSOR_DATA      = 0xD0,
    CMDH_SET_PCAP_INBAND        = 0xD1,
    CMDH_WRITE_PSR              = 0xD2,
    CMDH_SELECT_SENSOR_GROUPS   = 0xD3,
    CMDH_INBAND_WOF_CONTROL     = 0xD4,
} eCmdhCommands;

#define SENSOR_TEMP "TEMP"
#define SENSOR_FREQ "FREQ"
#define SENSOR_POWR "POWR"
#define SENSOR_CAPS "CAPS"
#define SENSOR_EXTN "EXTN"
#define EXTN_NAME_FMIN    0x464D494E // "FMIN"
#define EXTN_NAME_FDIS    0x46444953 // "FDIS"
#define EXTN_NAME_FBAS    0x46424153 // "FBAS"
#define EXTN_NAME_FUTURBO 0x46555400 // "FUT"
#define EXTN_NAME_FMAX    0x464D4158 // "FMAX"
#define EXTN_NAME_CLIP    0x434C4950 // "CLIP"
#define EXTN_NAME_WOFC    0x574F4643 // "WOFC"
#define EXTN_NAME_ERRHIST 0x45525248 // "ERRH"

#define MAX_EXTN_SENSORS 32
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
            uint8_t _reserved_6    : 1;
            uint8_t _reserved_5    : 1;
            uint8_t pmcr_owner     : 1;   // 1 => Indicates OCC sets the PMCR / Pstates
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
            uint8_t dvfs_due_to_ot     : 1;   // 1 => OCC clipped max Pstate due to a processor over temp.
            uint8_t dvfs_due_to_pwr    : 1;   // 1 => OCC clipped max Psate due to reaching pcap limit.
            uint8_t mthrot_due_to_ot   : 1;   // 1 => OCC throttled memory due to an over temp.
            uint8_t n_power            : 1;   // 1 => Server running without redundant power.
            uint8_t dvfs_due_to_vdd_ot : 1;   // 1 => OCC clipped max Pstate due to VRM Vdd over temp.
            uint8_t gpu_throttle       : 3;   // 1 => Bitmask of GPUs being throttled. LSB = GPU0. MSB = GPU2.
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
    // BYTE 15: Error Log Source
    uint8_t   errl_source;
    // BYTE 16: GPU Configuration
    uint8_t   gpu_presence;
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
    uint32_t id;       // Sensor id.
    uint8_t  fru_type;
    uint8_t  value;    // current temperature sensor reading in degrees C
    uint8_t  throttle;
    uint8_t  _reserved;
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

typedef struct __attribute__ ((packed)) cmdh_poll_powr_no_apss_sensor
{
    uint32_t sys_pwr_id;            // Sensor id - to represent total system power.
    uint16_t sys_pwr_update_time;   // Time in us that system power is read
    uint16_t sys_pwr_current;       // Most recent system power reading in watts
    uint32_t sys_pwr_update_tag;    // Count of number of samples represented by sys pwr accumulator
    uint64_t sys_pwr_accumul;       // Accumulation of system power readings
    uint32_t reserved;
    uint16_t proc_pwr_update_time;  // Time in us that processor power is updated
    uint16_t proc_pwr_current;      // Most recent processor power reading in watts
    uint32_t proc_pwr_update_tag;   // Count of number of samples represented by proc accumulator
    uint64_t proc_pwr_accumul;      // Accumulation of processor power readings
    uint16_t vdd_pwr_current;       // Most recent processor Vdd power reading in watts
    uint32_t vdd_pwr_update_tag;    // Count of number of samples represented by Vdd accumulator
    uint64_t vdd_pwr_accumul;       // Accumulation of processor Vdd power readings
    uint16_t vdn_pwr_current;       // Most recent processor Vdn power reading in watts
    uint32_t vdn_pwr_update_tag;    // Count of number of samples represented by Vdn accumulator
    uint64_t vdn_pwr_accumul;       // Accumulation of processor Vdn power readings
} cmdh_poll_power_no_apss_sensor_t;

// Only available from master occ.
typedef struct __attribute__ ((packed)) cmdh_poll_caps_sensor
{
    uint16_t current;   // Current power cap in 1W units.
    uint16_t system;    // Current system power in 1W units.
    uint16_t n;         // Oversubscription; output pcap limit when theres no redundant power.
    uint16_t max;       // Maximum power cap in 1W units.
    uint16_t hard_min;  // Minimum hard power cap in 1W units.
    uint16_t soft_min;  // Minimum soft power cap in 1W units.
    uint16_t user;      // Power cap set by user in 1W units.
    uint8_t  source;    // User Power Limit Source
} cmdh_poll_pcaps_sensor_t;

typedef struct __attribute__ ((packed)) cmdh_poll_extn_sensor
{
    uint32_t name;          // Sensor name or id
    uint8_t  flags;         // Flags indicating any special processing
    uint8_t  reserved;
    uint8_t  data[6];       // Data associated with sensor
} cmdh_poll_extn_sensor_t;

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

// Query packet used by the FSP for setting the OCC state, version 0x30.
typedef struct __attribute__ ((__packed__)) smgr_setmodestate_v30_query
{
    // Standard TMGT
    struct    cmdh_fsp_cmd_header;
    // Version
    uint8_t       version;
    // New state from OCC_STATE_*
    uint8_t       occ_state;
    // New mode from OCC_MODE_*
    uint8_t       occ_mode;
    // Additional mode parameter (either freq for FFO or OCC_FREQ_PT_PARM_* for SFP)
    uint16_t      mode_parm;

    uint8_t       _reserved;
} smgr_setmodestate_v30_query_t;

//---------------------------------------------------------
// Clear Elog Command
//---------------------------------------------------------
#define CLEAR_ELOG_V1_CMD_LEN    4

// Used by TMGT to clear elog data state, version 0.
typedef struct __attribute__ ((packed))
{
    struct    cmdh_fsp_cmd_header;
    uint8_t   elog_id;
}cmdh_clear_elog_query_t;

// Used by TMGT to clear elog data state, version 1.
typedef struct __attribute__ ((packed))
{
    struct    cmdh_fsp_cmd_header;
    uint8_t   version;
    uint8_t   elog_id;
    uint8_t   elog_source;
    uint8_t   reserved;
}cmdh_clear_elog_version_t;

//---------------------------------------------------------
// OCMB Recovery Status Command
//---------------------------------------------------------
#define OCMB_RECOVERY_STATUS_CMD_LEN    12

// Used by (H)TMGT to send status of OCMB recovery request
typedef struct __attribute__ ((packed))
{
    struct    cmdh_fsp_cmd_header;
    uint8_t   version;   // 1
    uint64_t  ocmb_id;
    uint8_t   status;
    uint16_t  reserved;
}cmdh_ocmb_recovery_status_t;

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
// Send Ambient Temperature
//---------------------------------------------------------

#define SEND_AMBIENT_VERSION_0 0x00
#define ALTITUDE_NOT_AVAILABLE 0xFFFF

typedef struct __attribute__ ((packed)) cmdh_send_ambient_temp
{
    // Standard TMGT Header
    struct cmdh_fsp_cmd_header;
    // Command Version
    uint8_t version;
    // Ambient Reading Status (0x00 = Sucess, 0xFF = Failure)
    uint8_t  ambient_status;
    // Ambient Temperature Reading
    uint8_t  ambient;
    uint16_t altitude;  // altitude in meters
    uint8_t  reserved[3];
} cmdh_send_ambient_temp_t;

//---------------------------------------------------------
// Get Field Debug Data
//---------------------------------------------------------

// Size of trace data for a trace buffer
#define CMDH_FIELD_TRACE_DATA_SIZE  1024
// Max number of sensors for field debug data
#define CMDH_FIELD_MAX_NUM_SENSORS  40

// list of sensors to include in field debug data
extern const uint16_t G_field_debug_gsids[];

typedef struct cmdh_dbug_sensor_list
{
    char        name[MAX_SENSOR_NAME_SZ];
    uint16_t    gsid;
    uint16_t    sample;
    uint16_t    sample_min;
    uint16_t    sample_max;
}cmdh_dbug_sensor_list_t;

// Used by OCC to response "GET_FIELD_DEBUG_DATA" cmd
typedef struct __attribute__ ((packed))
{
    struct                  cmdh_fsp_rsp_header;
    // version for parsing purposes
    uint8_t                 version;
    // OCC id
    uint8_t                 occ_id;
    // OCC role
    uint8_t                 occ_role;
    // ERR trace
    char                    trace_err[CMDH_FIELD_TRACE_DATA_SIZE];
    // IMP trace
    char                    trace_imp[CMDH_FIELD_TRACE_DATA_SIZE];
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


#define CMDH_SET_USER_PCAP_DATALEN 2

//---------------------------------------------------------
// Clear Sensor Data Command
//---------------------------------------------------------

typedef struct __attribute__ ((packed))
{
    uint8_t                 sensor_owner_id;
    uint8_t                 reserved[3];
}cmdh_clear_sensor_cmd_data_t;

typedef struct __attribute__ ((packed))
{
    uint8_t                 sensor_owner_id;
    uint8_t                 reserved[3];
}cmdh_clear_sensor_rsp_data_t;

//---------------------------------------------------------
// Set Power Cap In-band Command
//---------------------------------------------------------

typedef struct __attribute__ ((packed))
{
    uint8_t                power_cap[2];
}cmdh_set_inband_pcap_cmd_data_t;

typedef struct __attribute__ ((packed))
{
    uint8_t                power_cap[2];
}cmdh_set_inband_pcap_rsp_data_t;

//---------------------------------------------------------
// Write Power Shifting Ratio Command
//---------------------------------------------------------

typedef struct __attribute__ ((packed))
{
    uint8_t                 psr;
}cmdh_write_psr_cmd_data_t;

typedef struct __attribute__ ((packed))
{
    uint8_t                 psr;
}cmdh_write_psr_rsp_data_t;

//---------------------------------------------------------
// Select Sensor Groups Command
//---------------------------------------------------------

typedef struct __attribute__ ((packed))
{
    uint8_t                sensor_groups[2];
}cmdh_select_sensor_groups_cmd_data_t;

typedef struct __attribute__ ((packed))
{
    uint8_t                sensor_groups[2];
}cmdh_select_sensor_groups_rsp_data_t;

//---------------------------------------------------------
// Inband WOF Control Command
//---------------------------------------------------------
// Reason
typedef enum
{
    INBAND_WOF_CONTROL_DISABLE     = 0x00,
    INBAND_WOF_CONTROL_ENABLE      = 0x01,
    // Fmax mode, by definition WOF is off in this mode
    INBAND_WOF_CONTROL_FMAX        = 0x02,
    // User wants WOF enabled but OCC can't enable
    INBAND_WOF_CONTROL_OCC_DISABLE = 0xE0,
} eInbandWofControl;

typedef struct __attribute__ ((packed))
{
    uint8_t                 wof_control;
}cmdh_inband_wof_control_cmd_data_t;

typedef struct __attribute__ ((packed))
{
    uint8_t                 wof_control;
}cmdh_inband_wof_control_rsp_data_t;

errlHndl_t cmdh_tmgt_setmodestate(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                        cmdh_fsp_rsp_t * i_rsp_ptr);

void cmdh_dbug_cmd (const cmdh_fsp_cmd_t * i_cmd_ptr,
                          cmdh_fsp_rsp_t * i_rsp_ptr);

errlHndl_t cmdh_tmgt_poll (const cmdh_fsp_cmd_t * i_cmd_ptr,
                                 cmdh_fsp_rsp_t * i_rsp_ptr);

errlHndl_t cmdh_clear_elog (const cmdh_fsp_cmd_t * i_cmd_ptr,
                                  cmdh_fsp_rsp_t * i_rsp_ptr);

errlHndl_t cmdh_ocmb_recovery_status(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                           cmdh_fsp_rsp_t * i_rsp_ptr);

errlHndl_t cmdh_amec_pass_through(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                        cmdh_fsp_rsp_t * i_rsp_ptr);

errlHndl_t cmdh_reset_prep(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                 cmdh_fsp_rsp_t * i_rsp_ptr);

errlHndl_t cmdh_tmgt_get_field_debug_data(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                                cmdh_fsp_rsp_t * i_rsp_ptr);

errlHndl_t cmdh_set_user_pcap(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                    cmdh_fsp_rsp_t * o_rsp_ptr);

errlHndl_t cmdh_send_ambient_temp(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                        cmdh_fsp_rsp_t * o_rsp_ptr);
//------------------------------------------------------------------------------------
// Commands supported via in-band interface must have additional inputs/output/return
// Any out of band commands that are to be supported in-band should change to this
// common format to allow calling by either
//------------------------------------------------------------------------------------
uint8_t cmdh_clear_sensor_data(const uint16_t  i_cmd_data_length,
                                    const uint8_t*  i_cmd_data_ptr,
                                    const uint16_t  i_max_rsp_data_length,
                                          uint16_t* o_rsp_data_length,
                                          uint8_t*  o_rsp_data_ptr);

uint8_t cmdh_set_pcap_inband(const uint16_t  i_cmd_data_length,
                                  const uint8_t*  i_cmd_data_ptr,
                                  const uint16_t  i_max_rsp_data_length,
                                        uint16_t* o_rsp_data_length,
                                        uint8_t*  o_rsp_data_ptr);

uint8_t cmdh_write_psr(const uint16_t  i_cmd_data_length,
                            const uint8_t*  i_cmd_data_ptr,
                            const uint16_t  i_max_rsp_data_length,
                                  uint16_t* o_rsp_data_length,
                                  uint8_t*  o_rsp_data_ptr);

uint8_t cmdh_select_sensor_groups(const uint16_t  i_cmd_data_length,
                                       const uint8_t*  i_cmd_data_ptr,
                                       const uint16_t  i_max_rsp_data_length,
                                             uint16_t* o_rsp_data_length,
                                             uint8_t*  o_rsp_data_ptr);

uint8_t cmdh_inband_wof_control(const uint16_t  i_cmd_data_length,
                                const uint8_t*  i_cmd_data_ptr,
                                const uint16_t  i_max_rsp_data_length,
                                      uint16_t* o_rsp_data_length,
                                      uint8_t*  o_rsp_data_ptr);
#endif

