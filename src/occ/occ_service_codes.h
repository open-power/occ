/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/occ_service_codes.h $                                 */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2016                        */
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
    /// Failsafe State
    FAILSAFE_ALERT                  = 0x07,
    /// Fans are in full speed
    FAN_FULL_SPEED                  = 0x08,
    /// Timed out reading a FRU temperature
    FRU_TEMP_TIMEOUT                = 0x09,
    /// Processor reached error threshold
    PROC_ERROR_TEMP                 = 0x10,
    /// Timed out reading processor temperature
    PROC_TEMP_TIMEOUT               = 0x11,
    /// Processor SCOM failure
    PROC_SCOM_ERROR                 = 0x16,
    /// Any failure coming from the SSX RTOS code
    SSX_GENERIC_FAILURE             = 0x17,
    /// Failure to handshake with an external fw entity (HB, FSP, PHYP, etc)
    EXTERNAL_INTERFACE_FAILURE      = 0x18,
    /// WOF Algorithm Failure
    AMEC_WOF_ERROR                  = 0x19,
    /// VRM reached error threshold (VR_HOT asserted)
    VRM_ERROR_TEMP                  = 0x20,
    /// Timed out reading VR_FAN signal from VRM
    VRM_VRFAN_TIMEOUT               = 0x21,
    /// VR_FAN signal from VRM has been asserted
    VRM_VRFAN_ASSERTED              = 0x22,
    /// DIMM reached error threshold
    DIMM_ERROR_TEMP                 = 0x30,
    /// Frequency limited due to oversubscription condition
    OVERSUB_LIMIT_ALERT             = 0x33,
    /// Centaur reached error threshold
    CENT_ERROR_TEMP                 = 0x40,
    /// Centaur in-band scom failure
    CENT_SCOM_ERROR                 = 0x41,
    /// Centaur FIR bit set
    CENT_LFIR_ERROR                 = 0x42,
    /// Throttle in nominal or turbo mode due to the bulk power limit being reached with both power supplies good
    PCAP_THROTTLE_POWER_LIMIT       = 0x61,
    /// Failure within the OCC Complex of the processor
    INTERNAL_HW_FAILURE             = 0xB0,
    /// OCC GPE halted due to checkstop
    OCC_GPE_HALTED                  = 0xB1,
    /// PMC Failure
    PMC_FAILURE                     = 0xB2,
    /// Data passed as an argument or returned from a function is invalid
    INTERNAL_INVALID_INPUT_DATA     = 0xB3,
    /// A core was not at the expected frequency
    TARGET_FREQ_FAILURE             = 0xB4,
    /// RTL detected a system checkstop
    OCC_SYSTEM_HALTED               = 0xB5,
    ///  Reset PBA Queue due to PBAX Read Failure
    PBAX_QUEUE_RESET                = 0xBA,
    ///  PBAX failure sending doorbell to slaves
    PBAX_ERROR                      = 0xBB,
    ///  Request to read APSS data failed.
    APSS_GPE_FAILURE                = 0xC0,
    /// Connector overcurrent pin still asserted.
    CONNECTOR_OC_PINS_WARNING       = 0xC1,
    CONNECTOR_OC_PINS_FAILURE       = 0xC2,
    /// Slave OCC failed to receive new APSS data over a short time interval
    APSS_SLV_SHORT_TIMEOUT          = 0xC3,
    /// Slave OCC failed to receive new APSS data over a long time interval
    APSS_SLV_LONG_TIMEOUT           = 0xC4,
    /// APSS failed to return data or returned bad data over a long time interval
    APSS_HARD_FAILURE               = 0xC5,
    ///  Request to read redundant APSS data failed
    REDUNDANT_APSS_GPE_FAILURE      = 0xCB,
    /// Success!
    OCC_SUCCESS_REASON_CODE         = 0xFF,
};

// Extended reason codes
enum occExtReasonCode
{
    OCC_NO_EXTENDED_RC                          = 0x00000000,

    ERC_GENERIC_TIMEOUT                         = 0x00000001,
    ERC_INVALID_INPUT_DATA                      = 0x00000002,
    ERC_MMU_MAP_FAILURE                         = 0x00000003,
    ERC_MMU_UNMAP_FAILURE                       = 0x00000004,
    ERC_BCE_REQUEST_CREATE_FAILURE              = 0x00000005,
    ERC_BCE_REQUEST_SCHEDULE_FAILURE            = 0x00000006,

    ERC_RUNNING_SEM_PENDING_FAILURE             = 0x00000007,
    ERC_RUNNING_SEM_POSTING_FAILURE             = 0x00000008,
    ERC_WAKEUP_SEM_PENDING_FAILURE              = 0x00000009,
    ERC_WAKEUP_SEM_POSTING_FAILURE              = 0x0000000a,
    ERC_FINISHED_SEM_PENDING_FAILURE            = 0x0000000b,
    ERC_FINISHED_SEM_POSTING_FAILURE            = 0x0000000c,
    ERC_CALLER_SEM_POSTING_FAILURE              = 0x0000000d,
    ERC_CREATE_SEM_FAILURE                      = 0x0000000e,

    ERC_LOW_CORE_PORE_FLEX_CREATE_FAILURE       = 0x0000000f,
    ERC_HIGH_CORE_PORE_FLEX_CREATE_FAILURE      = 0x00000010,
    ERC_FAST_CORE_PORE_FLEX_CREATE_FAILURE      = 0x00000011,

    ERC_SSX_IRQ_SETUP_FAILURE                   = 0x00000012,
    ERC_SSX_IRQ_HANDLER_SET_FAILURE             = 0x00000013,
    ERC_PPC405_WD_SETUP_FAILURE                 = 0x00000014,
    ERC_OCB_WD_SETUP_FAILURE                    = 0x00000015,
    ERC_ARG_POINTER_FAILURE                     = 0x00000016,

    ERC_PSS_GPIO_INIT_FAIL                      = 0x00000017,
    ERC_PSS_COMPOSITE_MODE_FAIL                 = 0x00000019,

    ERC_PROC_CONTROL_TASK_FAILURE               = 0x0000001a,

    ERC_MMU_MAP_APPLET_READ_FAILURE             = 0x0000001b,
    ERC_MMU_MAP_APPLET_WRITE_FAILURE            = 0x0000001c,
    ERC_MMU_MAP_APPLET_OVERWRITE_FAILURE        = 0x0000001d,
    ERC_MMU_UNMAP_APPLET_READ_FAILURE           = 0x0000001e,
    ERC_MMU_UNMAP_APPLET_WRITE_FAILURE          = 0x0000001f,
    ERC_MMU_UNMAP_APPLET_OVERWRITE_FAILURE      = 0x00000020,

    ERC_CENTAUR_PORE_FLEX_CREATE_FAILURE        = 0x00000021,
    ERC_CENTAUR_PORE_FLEX_SCHEDULE_FAILURE      = 0x00000022,
    ERC_CENTAUR_INTERNAL_FAILURE                = 0x00000023,

    ERC_APSS_GPIO_OUT_OF_RANGE_FAILURE          = 0x00000024,
    ERC_APSS_GPIO_DUPLICATED_FAILURE            = 0x00000025,
    ERC_APSS_ADC_OUT_OF_RANGE_FAILURE           = 0x00000026,
    ERC_APSS_ADC_DUPLICATED_FAILURE             = 0x00000027,

    ERC_STATE_FROM_OBS_TO_STB_FAILURE           = 0x00000028,
    ERC_STATE_FROM_STB_TO_OBS_FAILURE           = 0x00000029,
    ERC_STATE_HEARTBEAT_CFG_FAILURE             = 0x00000080,

    ERC_AMEC_PCAPS_MISMATCH_FAILURE             = 0x0000002A,
    ERC_AMEC_UNDER_PCAP_FAILURE                 = 0x0000002B,

    ERC_AMEC_SLAVE_FAILSAFE_STATE               = 0x0000002C,
    ERC_AMEC_SLAVE_OVS_STATE                    = 0x0000002D,
    ERC_AMEC_SLAVE_POWERCAP                     = 0x0000002E,

    ERC_AMEC_PROC_ERROR_OVER_TEMPERATURE        = 0x0000002F,

    ERC_APLT_INIT_FAILURE                       = 0x00000030,
    ERC_APLT_START_VERSION_MISMATCH             = 0x00000031,
    ERC_APLT_START_CHECKSUM_MISMATCH            = 0x00000032,

    ERC_CMDH_MBOX_REQST_FAILURE                 = 0x00000040,
    ERC_CMDH_INTERNAL_FAILURE                   = 0x00000041,
    ERC_CMDH_THRM_DATA_MISSING                  = 0x00000042,
    ERC_CMDH_IPS_DATA_MISSING                   = 0x00000043,
    ERC_CMDH_INVALID_ATTN_DATA                  = 0x00000044,

    ERC_CHIP_IDS_INVALID                        = 0x00000050,
    ERC_GETSCOM_FAILURE                         = 0x00000051,
    ERC_GETSCOM_TPC_GP0_FAILURE                 = 0x00000052,
    ERC_PNOR_OWNERSHIP_NOT_AVAILABLE            = 0x00000053,

    ERC_HOMER_MAIN_ACCESS_ERROR                 = 0x00000060,
    ERC_HOMER_MAIN_SSX_ERROR                    = 0x00000061,

    ERC_APSS_SCHEDULE_FAILURE                   = 0x00000062,
    ERC_APSS_COMPLETE_FAILURE                   = 0x00000063,

    ERC_PROC_CONTROL_INIT_FAILURE               = 0x00000064,
    ERC_PROC_PSTATE_INSTALL_FAILURE             = 0x00000065,
    ERC_PROC_CORE_DATA_EMPATH_ERROR             = 0x00000066,

    ERC_BCE_REQ_CREATE_READ_FAILURE             = 0x00000070,
    ERC_BCE_REQ_SCHED_READ_FAILURE              = 0x00000071,
    ERC_BCE_REQ_CREATE_INPROG_FAILURE           = 0x00000072,
    ERC_BCE_REQ_SCHED_INPROG_FAILURE            = 0x00000073,
    ERC_BCE_REQ_CREATE_WRITE_FAILURE            = 0x00000074,
    ERC_BCE_REQ_SCHED_WRITE_FAILURE             = 0x00000075,
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
    OCC_IRQ_SETUP                   =  MAIN_COMP_ID | 0x08,
    PMC_HW_ERROR_ISR                =  MAIN_COMP_ID | 0x09,
    GETSCOM_FFDC_MID                =  MAIN_COMP_ID | 0x0a,
    PUTSCOM_FFDC_MID                =  MAIN_COMP_ID | 0x0b,
    HMON_ROUTINE_MID                =  MAIN_COMP_ID | 0x0c,
    AMEC_VERIFY_FREQ_MID            =  MAIN_COMP_ID | 0x0d,
    FIR_DATA_MID                    =  MAIN_COMP_ID | 0x0e,
};

enum occUserDataType
{
    OCC_FULL_ELOG_TYPE              =   0x0000,                 // complete error log data
};

enum occUserDataVersion
{
    OCC_FULL_ELOG_TYPE_VER1         =   0x0001,                 // complete error log data ver 1
};

#endif /* #ifndef _OCC_SERVICE_CODES_H_ */
