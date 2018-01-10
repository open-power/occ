/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/occ_service_codes.h $                             */
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

#ifndef _OCC_SERVICE_CODES_H_
#define _OCC_SERVICE_CODES_H_

#include <comp_ids.h>

// Error log reason codes.
enum occReasonCode
{
    /// Generic OCC firmware error log with extended srcs.
    INTERNAL_FAILURE                = 0x00,
    /// Informational periodic call home log
    GEN_CALLHOME_LOG                = 0x01,
    /// Failure within the OCC Complex of the processor
    PREP_FOR_RESET                  = 0x02,
    /// Invalid Input Data received from FSP
    INVALID_INPUT_DATA              = 0x03,
    /// Oversubscription was asserted
    OVERSUB_ALERT                   = 0x05,
    /// Failure to maintain a hard power cap
    POWER_CAP_FAILURE               = 0x06,
    /// Timed out reading a FRU temperature
    FRU_TEMP_TIMEOUT                = 0x09,
    /// Processor reached error threshold
    PROC_ERROR_TEMP                 = 0x10,
    /// Timed out reading processor temperature
    PROC_TEMP_TIMEOUT               = 0x11,
    // OCI write did not retain value
    OCI_WRITE_FAILURE               = 0x12,
    /// Processor SCOM failure
    PROC_SCOM_ERROR                 = 0x16,
    /// Any failure coming from the SSX RTOS code
    SSX_GENERIC_FAILURE             = 0x17,
    /// Failure to handshake with an external fw entity (HB, FSP, PHYP, etc)
    EXTERNAL_INTERFACE_FAILURE      = 0x18,
    /// VRM Vdd reached error temperature threshold
    VRM_VDD_ERROR_TEMP              = 0x20,
    /// VR_FAN  - AVS Bus over-temperature reported
    VRM_VRFAN_WARNING               = 0x22,
    /// GPIO_VR_HOT_MEM_PROC signal from APSS asserted
    VR_HOT_MEM_PROC_ASSERTED        = 0x23,
    /// DIMM reached error threshold
    DIMM_ERROR_TEMP                 = 0x30,
    /// Frequency limited due to oversubscription condition
    OVERSUB_LIMIT_ALERT             = 0x33,
    /// Invalid configuration data (MRW, etc.)
    INVALID_CONFIG_DATA             = 0x34,
    /// Centaur reached error threshold
    CENT_ERROR_TEMP                 = 0x40,
    /// Centaur in-band scom failure
    CENT_SCOM_ERROR                 = 0x41,
    /// Centaur FIR bit set
    CENT_LFIR_ERROR                 = 0x42,
    AVSBUS_TIMEOUT                  = 0x50,
    AVSBUS_ERROR                    = 0x51,
    AVSBUS_CRC_ERROR                = 0x52,
    /// Throttle in nominal or turbo mode due to the bulk power limit being reached with both power supplies good
    PCAP_THROTTLE_POWER_LIMIT       = 0x61,
    /// Firmware Failure: equivalent to assertion failures
    INTERNAL_FW_FAILURE             = 0xA0,

    /// Build problem, gpe1 image doesn't support GPU interface
    GPU_NO_GPE_SUPPORT              = 0xA1,
    /// Error with GPU tasks
    GPU_FAILURE                     = 0xA2,
    /// GPU core reached error threshold
    GPU_ERROR_TEMP                  = 0xA3,
    /// GPU memory reached error threshold
    GPU_MEMORY_ERROR_TEMP           = 0xA4,

    /// Failure within the OCC Complex of the processor
    INTERNAL_HW_FAILURE             = 0xB0,
    /// OCC GPE halted due to checkstop
    OCC_GPE_HALTED                  = 0xB1,
    /// Data passed as an argument or returned from a function is invalid
    INTERNAL_INVALID_INPUT_DATA     = 0xB3,
    /// RTL detected a system checkstop
    OCC_SYSTEM_HALTED               = 0xB5,
    ///  Reset PBA Queue due to PBAX Read Failure
    PBAX_QUEUE_RESET                = 0xBA,
    ///  Request to read APSS data failed.
    APSS_GPE_FAILURE                = 0xC0,
    /// AVS Bus output over-current reported
    VRM_OVER_CURRENT_WARNING        = 0xC1,
    /// Slave OCC failed to receive new APSS data over a short time interval
    APSS_SLV_SHORT_TIMEOUT          = 0xC3,
    /// Slave OCC failed to receive new APSS data over a long time interval
    APSS_SLV_LONG_TIMEOUT           = 0xC4,
    /// APSS failed to return data or returned bad data over a long time interval
    APSS_HARD_FAILURE               = 0xC5,
    ///  Request to read redundant APSS data failed
    REDUNDANT_APSS_GPE_FAILURE      = 0xCB,
    MEMORY_INIT_FAILED              = 0xD1,
    DIMM_INVALID_STATE              = 0xD2,

    // PGPE Generic RC
    PGPE_FAILURE                    = 0xD3,

    /// GPE IPC TASK RCs
    GPE_REQUEST_CREATE_FAILURE      = 0xD4,
    GPE_REQUEST_SCHEDULE_FAILURE    = 0xD5,
    GPE_REQUEST_TASK_TIMEOUT        = 0xD6,
    GPE_REQUEST_TASK_NOT_IDLE       = 0xD7,
    GPE_REQUEST_RC_FAILURE          = 0xD8,

    WOF_DISABLED_RC                 = 0xDA,
    INVALID_MAGIC_NUMBER            = 0xDC,
    INVALID_FREQUENCY               = 0xDE,
    WOF_RE_ENABLED                  = 0xDF,


// NOTE: 0xE0 - 0xEF can NOT be used these are reserved for critical
// OCC errors.  (H)TMGT will be looking for 0xEy ERRL_RC in cmd response RC
// and create an OCC error log with OCC component ID and 0xEy RC if found

    SENSOR_MAIN_MEM_ERROR           = 0xF0,  // Error copying sensors to main memory
    INBAND_CMD_ERROR                = 0xF1,
    OPAL_TABLE_UPDATE_ERROR         = 0xF2,

    /// Success!
    OCC_SUCCESS_REASON_CODE         = 0xFF,
};

// Extended reason codes
enum occExtReasonCode
{
    OCC_NO_EXTENDED_RC                          = 0x0000,

    ERC_GENERIC_TIMEOUT                         = 0x0001,
    ERC_INVALID_INPUT_DATA                      = 0x0002,
    ERC_MMU_MAP_FAILURE                         = 0x0003,
    ERC_MMU_UNMAP_FAILURE                       = 0x0004,
    ERC_BCE_REQUEST_CREATE_FAILURE              = 0x0005,
    ERC_BCE_REQUEST_SCHEDULE_FAILURE            = 0x0006,

    ERC_RUNNING_SEM_PENDING_FAILURE             = 0x0007,
    ERC_RUNNING_SEM_POSTING_FAILURE             = 0x0008,
    ERC_WAKEUP_SEM_PENDING_FAILURE              = 0x0009,
    ERC_WAKEUP_SEM_POSTING_FAILURE              = 0x000a,
    ERC_FINISHED_SEM_PENDING_FAILURE            = 0x000b,
    ERC_FINISHED_SEM_POSTING_FAILURE            = 0x000c,
    ERC_CALLER_SEM_POSTING_FAILURE              = 0x000d,
    ERC_CREATE_SEM_FAILURE                      = 0x000e,

    ERC_LOW_CORE_GPE_REQUEST_CREATE_FAILURE     = 0x000f,
    ERC_HIGH_CORE_GPE_REQUEST_CREATE_FAILURE    = 0x0010,

    ERC_SSX_IRQ_SETUP_FAILURE                   = 0x0012,
    ERC_SSX_IRQ_HANDLER_SET_FAILURE             = 0x0013,
    ERC_PPC405_WD_SETUP_FAILURE                 = 0x0014,
    ERC_OCB_WD_SETUP_FAILURE                    = 0x0015,
    ERC_ARG_POINTER_FAILURE                     = 0x0016,

    ERC_PSS_GPIO_INIT_FAIL                      = 0x0017,
    ERC_PSS_COMPOSITE_MODE_FAIL                 = 0x0019,

    ERC_PROC_CONTROL_TASK_FAILURE               = 0x001a,

    ERC_CENTAUR_PORE_FLEX_CREATE_FAILURE        = 0x0021,
    ERC_CENTAUR_PORE_FLEX_SCHEDULE_FAILURE      = 0x0022,
    ERC_CENTAUR_INTERNAL_FAILURE                = 0x0023,

    ERC_APSS_GPIO_OUT_OF_RANGE_FAILURE          = 0x0024,
    ERC_APSS_GPIO_DUPLICATED_FAILURE            = 0x0025,
    ERC_APSS_ADC_OUT_OF_RANGE_FAILURE           = 0x0026,
    ERC_APSS_ADC_DUPLICATED_FAILURE             = 0x0027,
    ERC_APSS_NO_VALID_DATA                      = 0x0028,
    ERC_APSS_RESET_FAILURE                      = 0x0029,

    ERC_AMEC_PCAPS_MISMATCH_FAILURE             = 0x002A,
    ERC_AMEC_UNDER_PCAP_FAILURE                 = 0x002B,

    ERC_AMEC_SLAVE_OVS_STATE                    = 0x002D,
    ERC_AMEC_SLAVE_POWERCAP                     = 0x002E,

    ERC_AMEC_PROC_ERROR_OVER_TEMPERATURE        = 0x002F,

    ERC_AMEC_VRM_VDD_TEMP_TIMEOUT               = 0x0030,
    ERC_AMEC_DIMM_TEMP_TIMEOUT                  = 0x0031,
    ERC_AMEC_CENT_TEMP_TIMEOUT                  = 0x0032,

    ERC_CMDH_MBOX_REQST_FAILURE                 = 0x0040,
    ERC_CMDH_INTERNAL_FAILURE                   = 0x0041,
    ERC_CMDH_THRM_DATA_MISSING                  = 0x0042,
    ERC_CMDH_IPS_DATA_MISSING                   = 0x0043,
    ERC_CMDH_INVALID_ATTN_DATA                  = 0x0044,

    ERC_CHIP_IDS_INVALID                        = 0x0050,
    ERC_GETSCOM_FAILURE                         = 0x0051,
    ERC_GETSCOM_TPC_GP0_FAILURE                 = 0x0052,
    ERC_PNOR_OWNERSHIP_NOT_AVAILABLE            = 0x0053,

    ERC_HOMER_MAIN_ACCESS_ERROR                 = 0x0060,
    ERC_HOMER_MAIN_SSX_ERROR                    = 0x0061,

    ERC_APSS_SCHEDULE_FAILURE                   = 0x0062,
    ERC_APSS_COMPLETE_FAILURE                   = 0x0063,

    ERC_PROC_CONTROL_INIT_FAILURE               = 0x0064,
    ERC_PROC_PSTATE_INSTALL_FAILURE             = 0x0065,
    ERC_PROC_CORE_DATA_EMPATH_ERROR             = 0x0066,
    ERC_NEST_DTS_GPE_REQUEST_CREATE_FAILURE     = 0x0067,

    ERC_BCE_REQ_CREATE_READ_FAILURE             = 0x0070,
    ERC_BCE_REQ_SCHED_READ_FAILURE              = 0x0071,
    ERC_BCE_REQ_CREATE_INPROG_FAILURE           = 0x0072,
    ERC_BCE_REQ_SCHED_INPROG_FAILURE            = 0x0073,
    ERC_BCE_REQ_CREATE_WRITE_FAILURE            = 0x0074,
    ERC_BCE_REQ_SCHED_WRITE_FAILURE             = 0x0075,
    ERC_BCE_REQ_CALLBACK_TIMEOUT                = 0x0076,

    ERC_DIMM_SCHEDULE_FAILURE                   = 0x0080,
    ERC_DIMM_COMPLETE_FAILURE                   = 0x0081,

    ERC_MEM_CONTROL_SCHEDULE_FAILURE            = 0x0080,
    ERC_MEM_CONTROL_COMPLETE_FAILURE            = 0x0081,

    ERC_FW_ZERO_FREQ_LIMIT                      = 0x0090,

    ERC_TLB_ENTRY_CREATION_FAILURE              = 0x00A0,

    ERC_AVSBUS_VDD_VOLTAGE_FAILURE              = 0x00AA,
    ERC_AVSBUS_VDD_CURRENT_FAILURE              = 0x00AB,
    ERC_AVSBUS_VDN_VOLTAGE_FAILURE              = 0x00AC,
    ERC_AVSBUS_VDN_CURRENT_FAILURE              = 0x00AD,
    ERC_AVSBUS_STATUS_FAILURE                   = 0x00AE,
    ERC_AVSBUS_VDD_TEMPERATURE_FAILURE          = 0x00AF,

    ERC_PGPE_BEACON_TIMEOUT                     = 0x00B0,
    ERC_PGPE_CLIP_NOT_IDLE                      = 0x00B1,
    ERC_PGPE_SET_PMCR_NOT_IDLE                  = 0x00B2,
    ERC_PGPE_START_SUSPEND_NOT_IDLE             = 0x00B3,
    ERC_PGPE_SET_PMCR_FAILURE                   = 0x00B4,
    ERC_PGPE_START_SUSPEND_FAILURE              = 0x00B5,
    ERC_PGPE_SET_NOMINAL_FAILURE                = 0x00B6,
    ERC_PGPE_CLIP_FAILURE                       = 0x00B7,
    ERC_PGPE_PPMR_OPPB_SIZE_MISMATCH            = 0x00B8,
    ERC_PGPE_ACTIVE_TO_OBSERVATION_TIMEOUT      = 0x00B9,
    ERC_PGPE_TASK_TIMEOUT                       = 0x00BA,
    ERC_PGPE_INVALID_ADDRESS                    = 0x00BB,

    ERC_WOF_QUAD_COUNT_FAILURE                  = 0x00C0,
    ERC_WOF_CONTROL_ERROR                       = 0x00C1,

    ERC_24X7_GPE_CREATE_FAILURE                 = 0x00D0,
    ERC_24X7_GPE_SCHEDULE_FAILURE               = 0x00D1,

    ERC_SMGR_NO_VALID_MODE_TRANSITION_CALL      = 0x00E0,
    ERC_SMGR_NO_VALID_STATE_TRANSITION_CALL     = 0x00E1,

    ERC_GPU_COMPLETE_FAILURE                    = 0x00F0,
    ERC_GPU_SCHEDULE_FAILURE                    = 0x00F1,
    ERC_GPU_RESET_FAILURE                       = 0x00F2,
    ERC_GPU_RESET_TIMEOUT                       = 0x00F3,
    ERC_GPU_READ_TEMP_TIMEOUT                   = 0x00F4,
    ERC_GPU_READ_MEM_TEMP_TIMEOUT               = 0x00F5,
    ERC_GPU_READ_MEM_TEMP_CAPABLE_FAILURE       = 0x00F6,
    ERC_GPU_INVALID_GPU_OPERATION               = 0x00F7,
    ERC_GPU_N_MODE_PCAP_CALC_FAILURE            = 0x00F8,
    ERC_GPU_N_PLUS_1_MODE_PCAP_CALC_FAILURE     = 0x00F9,
    ERC_GPU_NO_GPE_SUPPORT                      = 0x00FF,
    ERC_GPU_CHECK_DRIVER_LOADED_FAILURE         = 0x0100,
    ERC_GPU_READ_PWR_LIMIT_FAILURE              = 0x0101,
    ERC_GPU_SET_PWR_LIMIT_FAILURE               = 0x0102,

    ERC_STATE_FROM_OBS_TO_ACT_FAILURE           = 0x0123,
    ERC_STATE_FROM_ACT_TO_CHR_FAILURE           = 0x0124,
    ERC_STATE_FROM_CHR_TO_ACT_FAILURE           = 0x0125,
    ERC_STATE_FROM_CHR_TO_OBS_FAILURE           = 0x0126,
    ERC_STATE_FROM_OBS_TO_CHR_FAILURE           = 0x0127,
    ERC_STATE_FROM_STB_TO_CHR_FAILURE           = 0x0128,
    ERC_STATE_FROM_STB_TO_OBS_FAILURE           = 0x0129,

    ERC_STATE_HEARTBEAT_CFG_FAILURE             = 0x0180,
    ERC_RTL_TIME_EXCEEDED                       = 0x0190,
};

// Error log Module Ids
enum occModuleId
{
    MAIN_MID                        =  MAIN_COMP_ID | 0x01,
    MAIN_THRD_ROUTINE_MID           =  MAIN_COMP_ID | 0x02,
    MAIN_THRD_TIMER_MID             =  MAIN_COMP_ID | 0x03,
    MAIN_THRD_SEM_INIT_MID          =  MAIN_COMP_ID | 0x04,
    MAIN_STATE_TRANSITION_MID       =  MAIN_COMP_ID | 0x05,
    MAIN_MODE_TRANSITION_MID        =  MAIN_COMP_ID | 0x06,
    MAIN_SYSTEM_HALTED_MID          =  MAIN_COMP_ID | 0x07,
    OCC_IPC_SETUP                   =  MAIN_COMP_ID | 0x08,
    GETSCOM_FFDC_MID                =  MAIN_COMP_ID | 0x0a,
    PUTSCOM_FFDC_MID                =  MAIN_COMP_ID | 0x0b,
    HMON_ROUTINE_MID                =  MAIN_COMP_ID | 0x0c,
    FIR_DATA_MID                    =  MAIN_COMP_ID | 0x0e,
    CMDH_DBUG_MID                   =  MAIN_COMP_ID | 0x0f,
    I2C_LOCK_UPDATE                 =  MAIN_COMP_ID | 0x10,
    CREATE_TLB_ENTRY                =  MAIN_COMP_ID | 0x11,
    READ_PGPE_HEADER                =  MAIN_COMP_ID | 0x12,
    READ_PPMR_HEADER                =  MAIN_COMP_ID | 0x13,
    READ_OPPB_PARAMS                =  MAIN_COMP_ID | 0x14,
    MAIN_SMGR_MID                   =  MAIN_COMP_ID | 0x15,
    READ_WOF_HEADER                 =  MAIN_COMP_ID | 0x16,
};

enum occUserDataType
{
    OCC_FULL_ELOG_TYPE              =   0x0000,                 // complete error log data
    OCC_TRACE_TYPE                  =   0x0002,                 // OCC trace (from TMGT)
    OCC_EXCEPTION_TYPE              =   0x00E0,                 // OCC exception data (from TMGT)
};

enum occUserDataVersion
{
    OCC_FULL_ELOG_TYPE_VER1         =   0x0001,                 // complete error log data ver 1
};

#endif /* #ifndef _OCC_SERVICE_CODES_H_ */
