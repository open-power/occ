/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/include/pstate_pgpe_occ_api.h $                           */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2016,2020                        */
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
/// @file  pstates_pgpe_occ_api.h
/// @brief Structures used between PGPE HCode and OCC Firmware
///
// *HWP HW Owner        : Rahul Batra <rbatra@us.ibm.com>
// *HWP HW Owner        : Michael Floyd <mfloyd@us.ibm.com>
// *HWP Team            : PM
// *HWP Level           : 1
// *HWP Consumed by     : PGPE:OCC


#ifndef __PSTATES_PGPE_OCC_API_H__
#define __PSTATES_PGPE_OCC_API_H__

#include <pstates_common.H>

#ifdef __cplusplus
extern "C" {
#endif

#define HCODE_OCC_SHARED_MAGIC_NUMBER_OPS2      0x4F505332 //OPS2

//---------------
// IPC from 405
//---------------
//Note: These are really not used. They are just for documentation purposes
enum MESSAGE_ID_IPI2HI
{
    MSGID_405_INVALID       = 0,
    MSGID_405_START_SUSPEND = 1,
    MSGID_405_CLIPS         = 2,
    MSGID_405_SET_PMCR      = 3,
    MSGID_405_WOF_CONTROL   = 4,
    MSGID_405_WOF_VRT       = 5
};

//
// Return Codes
//
#define PGPE_RC_SUCCESS                         0x01
#define PGPE_WOF_RC_NOT_ENABLED                 0x10
#define PGPE_RC_PSTATES_NOT_STARTED             0x11
#define PGPE_RC_OCC_NOT_PMCR_OWNER              0x14
#define PGPE_RC_PM_COMPLEX_SUSPEND_SAFE_MODE    0x15

#define PGPE_RC_REQ_WHILE_PENDING_ACK           0x21
#define PGPE_RC_NULL_VRT_POINTER                0x22
#define PGPE_RC_INVALID_PMCR_OWNER              0x23

//
// PMCR Owner
//
typedef enum
{
    PMCR_OWNER_HOST         = 0,
    PMCR_OWNER_OCC          = 1,
    PMCR_OWNER_CHAR         = 2
} PMCR_OWNER;


typedef struct ipcmsg_base
{
    uint8_t   rc;
} ipcmsg_base_t;


//
// Start Suspend Actions
//
#define PGPE_ACTION_PSTATE_START   0
#define PGPE_ACTION_PSTATE_STOP    1

typedef struct ipcmsg_start_stop
{
    ipcmsg_base_t   msg_cb;
    uint8_t         action;
    PMCR_OWNER      pmcr_owner;
} ipcmsg_start_stop_t;


typedef struct ipcmsg_clip_update
{
    ipcmsg_base_t   msg_cb;
    uint8_t         ps_val_clip_min;
    uint8_t         ps_val_clip_max;
    uint8_t         pad[1];
} ipcmsg_clip_update_t;


typedef struct ipcmsg_set_pmcr
{
    ipcmsg_base_t   msg_cb;
    uint8_t         pad[7];
    uint64_t        pmcr;       // @todo Why is this a unit64_t?  It was in P9.
} ipcmsg_set_pmcr_t;


//
// WOF Control Actions
//
#define PGPE_ACTION_WOF_ON         1
#define PGPE_ACTION_WOF_OFF        2

typedef struct ipcmsg_wof_control
{
    ipcmsg_base_t   msg_cb;
    uint8_t         action;
    uint8_t         pad;
} ipcmsg_wof_control_t;


typedef struct ipcmsg_wof_vrt
{
    ipcmsg_base_t   msg_cb;
    uint8_t         vratio_mode;         // 0 = variable; 1 = fixed
    uint8_t         fixed_vratio_index;  // if vratio_mode = fixed, index to use
    uint8_t         pad[1];
    VRT_t*          idd_vrt_ptr; // VDD Voltage Ratio Table
    uint32_t        vdd_ceff_ratio; // Used for VDD
    uint32_t        vcs_ceff_ratio; // Used for VCS
} ipcmsg_wof_vrt_t;

// -----------------------------------------------------------------------------
// Start Pstate Table

#define MAX_OCC_PSTATE_TABLE_ENTRIES 256

/// Pstate Table produced by the PGPE for consumption by OCC Firmware
///
/// This structure defines the Pstate Table content
/// -- 16B structure

typedef struct
{
    /// Pstate number
    Pstate_t   pstate;

    /// Assocated Frequency (in MHz)
    uint16_t   frequency_mhz;

    /// Internal VDD voltage ID at the output of the PFET header
    uint8_t    internal_vdd_vid;

} OCCPstateTable_entry_t;

typedef struct
{
    /// Number of Pstate Table entries
    uint32_t                entries;

    /// Internal VDD voltage ID at the output of the PFET header
    OCCPstateTable_entry_t  table[MAX_OCC_PSTATE_TABLE_ENTRIES];

} OCCPstateTable_t;

// End Pstate Table
// -----------------------------------------------------------------------------

typedef struct
{
    union
    {
        uint64_t value;
        struct
        {
            uint32_t high_order;
            uint32_t low_order;
        } words;
        struct
        {
            uint64_t average_pstate             : 8;
            uint64_t average_frequency_pstate   : 8;
            uint64_t wof_clip_pstate            : 8;
            uint64_t average_throttle_idx       : 8;
            uint64_t vratio_inst                : 16;
            uint64_t vratio_avg                 : 16;
        } fields;
    } dw0;
    union
    {
        uint64_t value;
        struct
        {
            uint32_t high_order;
            uint32_t low_order;
        } words;
        struct
        {
            uint64_t idd_avg_ma                 : 16;
            uint64_t ics_avg_ma                 : 16;
            uint64_t idn_avg_ma                 : 16;
            uint64_t iio_avg_ma                 : 16;

        } fields;
    } dw1;
    union
    {
        uint64_t value;
        struct
        {
            uint32_t high_order;
            uint32_t low_order;
        } words;
        struct
        {
            uint64_t vdd_avg_mv                 : 16;
            uint64_t vcs_avg_mv                 : 16;
            uint64_t vdn_avg_mv                 : 16;
            uint64_t vio_avg_mv                 : 16;
        } fields;
    } dw2;
    union
    {
        uint64_t value;
        struct
        {
            uint32_t high_order;
            uint32_t low_order;
        } words;
        struct
        {
            uint64_t ocs_avg_0p01pct            : 16;
            uint64_t reserved                   : 48;
        } fields;
    } dw3;
} pgpe_wof_values_t;

// -----------------------------------------------------------------------------
// Start Error Log Table

/// Maximum number of error log entries available
#define MAX_HCODE_ELOG_ENTRIES 20

/// Index into the array of error log entries
enum elog_entry_index
{
    ELOG_PGPE_CRITICAL      = 0,
    ELOG_PGPE_INFO          = 1,
    ELOG_XGPE_CRITICAL      = 2,
    ELOG_XGPE_INFO          = 3,
    ELOG_QME0_CRITICAL      = 4,
    ELOG_QME0_INFO          = 5,
    ELOG_QME1_CRITICAL      = 6,
    ELOG_QME1_INFO          = 7,
    ELOG_QME2_CRITICAL      = 8,
    ELOG_QME2_INFO          = 9,
    ELOG_QME3_CRITICAL      = 10,
    ELOG_QME3_INFO          = 11,
    ELOG_QME4_CRITICAL      = 12,
    ELOG_QME4_INFO          = 13,
    ELOG_QME5_CRITICAL      = 14,
    ELOG_QME5_INFO          = 15,
    ELOG_QME6_CRITICAL      = 16,
    ELOG_QME6_INFO          = 17,
    ELOG_QME7_CRITICAL      = 18,
    ELOG_QME7_INFO          = 19,
};

/// Structure of an individual error log entry
typedef struct
{
    union
    {
        uint64_t value;
        struct
        {
            uint32_t high_order;
            uint32_t low_order;
        } words;
        struct
        {
            uint64_t errlog_id                  : 8;
            uint64_t errlog_src                 : 8;
            uint64_t errlog_len                 : 16;
            uint64_t errlog_addr                : 32;
        } fields;
    } dw0;
} hcode_elog_entry_t;

/// Full Error Log Table
typedef struct pgpe_error_table
{
    union
    {
        uint64_t value;
        struct
        {
            uint32_t high_order;
            uint32_t low_order;
        } words;
        struct
        {
            uint64_t magic_word                 : 32; //ELTC
            uint64_t total_log_slots            : 8;
            uint64_t reserved                   : 24;
        } fields;
    } dw0;

    /// Array of error log entries (index with enum elog_entry_index)
    hcode_elog_entry_t  elog[MAX_HCODE_ELOG_ENTRIES];
} hcode_error_table_t;

// End Error Log Table
// -----------------------------------------------------------------------------

typedef union
{
    uint64_t value;
    struct
    {
        uint32_t high_order;
        uint32_t low_order;
    } words;
    struct
    {
        uint64_t reserved0                      : 24;
        uint64_t io_index                       : 8;
        uint64_t reserved1                      : 32;
    } fields;
} xgpe_wof_values_t;

typedef union
{
    uint64_t value;
    struct
    {
        uint32_t high_order;
        uint32_t low_order;
    } words;
    struct
    {
        uint64_t sibling_base_frequency         : 16;
        uint64_t reserved0                      : 24;
        uint64_t sibling_pstate                 : 8;
        uint64_t reserved1                      : 32;
    } fields;
} occ_wof_values_t;

typedef union
{
    uint32_t    core_off[32];
    uint32_t    core_vmin[32];
    uint32_t    core_mma_off[32];
    uint32_t    l3_off[32];
} iddq_activity_t;


/// Hcode<>OCC Shared Data Structure
///
/// Shared data between OCC, PGPE and XGPE
typedef struct
{
    /// Magic number + version.  "OPS" || version (nibble)
    uint32_t            magic;

    /// PGPE Beacon
    uint32_t            pgpe_beacon;

    /// OCC Data offset from start of OCC Shared SRAM
    uint16_t            occ_data_offset;

    /// OCC Data Length
    uint16_t            occ_data_length;

    /// PGPE Data offset from start of OCC Shared SRAM
    uint16_t            pgpe_data_offset;

    /// PGPE Data Length
    uint16_t            pgpe_data_length;

    /// XGPE Data offset from start of OCC Shared SRAM
    uint16_t            xgpe_data_offset;

    /// XGPE Data Length
    uint16_t            xgpe_data_length;

    /// IDDQ Data offset from start of OCC Shared SRAM
    uint16_t            iddq_data_offset;

    /// IDDQ Data Length
    uint16_t            iddq_data_length;

    /// Error Log offset from start of OCC Shared SRAM
    uint16_t            error_log_offset;

    /// Pstate Table offset from start of OCC Shared SRAM
    uint16_t            pstate_table_offset;

    uint16_t            reserved;

    ///IDDQ Activity sample depth(number of samples accumulated)
    uint16_t            iddq_activity_sample_depth;

    /// OCC Produced WOF Values
    occ_wof_values_t    occ_wof_values;

    /// XGPE Produced WOF Values
    xgpe_wof_values_t    xgpe_wof_values;

    /// PGPE Produced WOF Values
    pgpe_wof_values_t   pgpe_wof_values;

    /// Hcode Error Log Index
    hcode_error_table_t  errlog_idx;

    /// IDDQ Activity Values created by PGPE
    iddq_activity_t     iddq_activity_values;

    /// Pstate Table for OCC consumption
    OCCPstateTable_t    pstate_table;

} HcodeOCCSharedData_t;

#ifdef __cplusplus
} // end extern C
#endif

#endif    /* __PSTATES_PGPE_OCC_API_H__ */
