/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/common.h $                                        */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2020                        */
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
#ifndef _common_h
#define _common_h

#include <rtls.h>

// Allow Trace Masks
#define ALLOW_PMCR_TRACE           0x0001
#define ALLOW_CLIP_TRACE           0x0002
#define ALLOW_VRT_TRACE            0x0004
#define ALLOW_OPAL_TRACE           0x0008
#define ALLOW_MEM_TRACE            0x0010
#define ALLOW_AVSBUS_TRACE         0x0020
#define ALLOW_WOF_OCS_TRACE        0x0040
#define ALLOW_CEFF_RATIO_VDD_TRACE 0x0080
#define ALLOW_PCAP_TRACE           0x0100
#define ALLOW_24x7_TRACE           0x0200
#define ALLOW_DCOM_TRACE           0x0400
#define ALLOW_PBAX_TRACE           0x0800
#define ALLOW_AMEC_PSTATE_TRACE    0x1000
#define ALLOW_CEFF_RATIO_VCS_TRACE 0x2000

// Start of SRAM memory
#define SRAM_START_ADDRESS_405      0xFFF40000

// OCB_OCCMISC has 3 bits reserved for OCC (ext_intr_reason)
// Reasons why the OCCMISC external interrupt was triggered
typedef enum
{
    // defined as individual bits (since multiple interrupts could be queued)
    INTR_REASON_HTMGT_SERVICE_REQUIRED  = 0x04,
    INTR_REASON_I2C_OWNERSHIP_CHANGE    = 0x02,
    INTR_REASON_OPAL_SHARED_MEM_CHANGE  = 0x01
} ext_intr_reason_t;


// OCB_OCCFLG0 and OCB_OCCFLG1 is defined by the OCC (instead of ocb_firmware_registers.h)
// OCC currently uses OCCFLG0 with the following structure:
typedef union ocb_occflg {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t reserved_gpe          : 16;
    uint32_t i2c_engine1_lock_host : 1;
    uint32_t i2c_engine1_lock_occ  : 1;
    uint32_t i2c_engine2_lock_host : 1;
    uint32_t i2c_engine2_lock_occ  : 1;
    uint32_t i2c_engine3_lock_host : 1;
    uint32_t i2c_engine3_lock_occ  : 1;
    uint32_t gpu0_reset_status     : 1;
    uint32_t gpu1_reset_status     : 1;
    uint32_t gpu2_reset_status     : 1;
    uint32_t ocs_dirty             : 1; // bit 25 OverCurrent sensor dirty
    uint32_t ocs_dirty_type        : 1; // bit 26 OverCurrent sensor dirty type
    uint32_t pm_reset_suppress     : 1;
    uint32_t occ_started           : 1;
    uint32_t undefined             : 2;
    uint32_t request_occ_safe      : 1; // bit 31 currently not checked
#else
    uint32_t request_occ_safe      : 1;
    uint32_t undefined             : 2;
    uint32_t occ_started           : 1;
    uint32_t pm_reset_suppress     : 1;
    uint32_t ocs_dirty_type        : 1;
    uint32_t ocs_dirty             : 1;
    uint32_t gpu2_reset_status     : 1;
    uint32_t gpu1_reset_status     : 1;
    uint32_t gpu0_reset_status     : 1;
    uint32_t i2c_engine3_lock_occ  : 1;
    uint32_t i2c_engine3_lock_host : 1;
    uint32_t i2c_engine2_lock_occ  : 1;
    uint32_t i2c_engine2_lock_host : 1;
    uint32_t i2c_engine1_lock_occ  : 1;
    uint32_t i2c_engine1_lock_host : 1;
    uint32_t reserved_gpe          : 16;
#endif // _BIG_ENDIAN
    } fields;
} ocb_occflg_t;


// Miscellaneous checks to be done by 405
// - check for pending host notifications
// - check for checkstop
void task_misc_405_checks(task_t *i_self);

// Trigger interrupt to the host with the specified reason (OCCMISC/core_ext_intr)
// Returns true if notification was sent, false if interrupt already outstanding
bool notify_host(const ext_intr_reason_t i_reason);

// Returns true if PGPE error should be ignored
bool ignore_pgpe_error(void);

#endif // _common_h
