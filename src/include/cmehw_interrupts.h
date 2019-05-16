/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/include/cmehw_interrupts.h $                              */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2019                        */
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
#ifndef __CMEHW_INTERRUPTS_H__
#define __CMEHW_INTERRUPTS_H__


////////////////////////////////////////////////////////////////////////////
// IRQ Definition
////////////////////////////////////////////////////////////////////////////

// The CME interrupt controller consists of 1 x 64-bit controller.
//
// All 64 interrupts are or'd together and presented to the PPE as a single
// external interrupt exception.

#define CMEHW_IRQ_DEBUGGER                        0 /* 0x00 */
#define CMEHW_IRQ_DEBUG_TRIGGER                   1 /* 0x01 */
#define CMEHW_IRQ_QUAD_CHECKSTOP                  2 /* 0x02 */
#define CMEHW_IRQ_PVREF_FAIL                      3 /* 0x03 */
#define CMEHW_IRQ_OCC_HEARTBEAT_LOST              4 /* 0x04 */
#define CMEHW_IRQ_CORE_CHECKSTOP                  5 /* 0x05 */
#define CMEHW_IRQ_DROPOUT_DETECT                  6 /* 0x06 */
#define CMEHW_IRQ_SPARE_7                         7 /* 0x07 */
#define CMEHW_IRQ_BCE_BUSY_HIGH                   8 /* 0x08 */
#define CMEHW_IRQ_BCE_TIMEOUT                     9 /* 0x09 */
#define CMEHW_IRQ_DOORBELL3_C0                    10 /* 0x0a */
#define CMEHW_IRQ_DOORBELL3_C1                    11 /* 0x0b */
#define CMEHW_IRQ_PC_INTR_PENDING_C0              12 /* 0x0c */
#define CMEHW_IRQ_PC_INTR_PENDING_C1              13 /* 0x0d */
#define CMEHW_IRQ_SPECIAL_WAKEUP_C0               14 /* 0x0e */
#define CMEHW_IRQ_SPECIAL_WAKEUP_C1               15 /* 0x0f */
#define CMEHW_IRQ_REG_WAKEUP_C0                   16 /* 0x10 */
#define CMEHW_IRQ_REG_WAKEUP_C1                   17 /* 0x11 */
#define CMEHW_IRQ_DOORBELL2_C0                    18 /* 0x12 */
#define CMEHW_IRQ_DOORBELL2_C1                    19 /* 0x13 */
#define CMEHW_IRQ_PC_PM_STATE_ACTIVE_C0           20 /* 0x14 */
#define CMEHW_IRQ_PC_PM_STATE_ACTIVE_C1           21 /* 0x15 */
#define CMEHW_IRQ_L2_PURGE_DONE                   22 /* 0x16 */
#define CMEHW_IRQ_NCU_PURGE_DONE                  23 /* 0x17 */
#define CMEHW_IRQ_CHTM_PURGE_DONE_C0              24 /* 0x18 */
#define CMEHW_IRQ_CHTM_PURGE_DONE_C1              25 /* 0x19 */
#define CMEHW_IRQ_BCE_BUSY_LOW                    26 /* 0x1a */
#define CMEHW_IRQ_FINAL_VDM_DATA0                 27 /* 0x1b */
#define CMEHW_IRQ_FINAL_VDM_DATA1                 28 /* 0x1c */
#define CMEHW_IRQ_COMM_RECVD                      29 /* 0x1d */
#define CMEHW_IRQ_COMM_SEND_ACK                   30 /* 0x1e */
#define CMEHW_IRQ_COMM_SEND_NACK                  31 /* 0x1f */

#define CMEHW_IRQ_SPARE_32                        32 /* 0x20 */
#define CMEHW_IRQ_SPARE_33                        33 /* 0x21 */
#define CMEHW_IRQ_PMCR_UPDATE_C0                  34 /* 0x22 */
#define CMEHW_IRQ_PMCR_UPDATE_C1                  35 /* 0x23 */
#define CMEHW_IRQ_DOORBELL0_C0                    36 /* 0x24 */
#define CMEHW_IRQ_DOORBELL0_C1                    37 /* 0x25 */
#define CMEHW_IRQ_SPARE_38                        38 /* 0x26 */
#define CMEHW_IRQ_SPARE_39                        39 /* 0x27 */
#define CMEHW_IRQ_DOORBELL1_C0                    40 /* 0x28 */
#define CMEHW_IRQ_DOORBELL1_C1                    41 /* 0x29 */
#define CMEHW_IRQ_PECE_INTR_DISABLED_C0           42 /* 0x2a */
#define CMEHW_IRQ_PECE_INTR_DISABLED_C1           43 /* 0x2b */
#define CMEHW_IRQ_RESERVED_44                     44 /* 0x2c */
#define CMEHW_IRQ_RESERVED_45                     45 /* 0x2d */
#define CMEHW_IRQ_RESERVED_46                     46 /* 0x2e */
#define CMEHW_IRQ_RESERVED_47                     47 /* 0x2f */
#define CMEHW_IRQ_RESERVED_48                     48 /* 0x30 */
#define CMEHW_IRQ_RESERVED_49                     49 /* 0x31 */
#define CMEHW_IRQ_RESERVED_50                     50 /* 0x32 */
#define CMEHW_IRQ_RESERVED_51                     51 /* 0x33 */
#define CMEHW_IRQ_RESERVED_52                     52 /* 0x34 */
#define CMEHW_IRQ_RESERVED_53                     53 /* 0x35 */
#define CMEHW_IRQ_RESERVED_54                     54 /* 0x36 */
#define CMEHW_IRQ_RESERVED_55                     55 /* 0x37 */
#define CMEHW_IRQ_RESERVED_56                     56 /* 0x38 */
#define CMEHW_IRQ_RESERVED_57                     57 /* 0x39 */
#define CMEHW_IRQ_RESERVED_58                     58 /* 0x3a */
#define CMEHW_IRQ_RESERVED_59                     59 /* 0x3b */
#define CMEHW_IRQ_RESERVED_60                     60 /* 0x3c */
#define CMEHW_IRQ_RESERVED_61                     61 /* 0x3d */
#define CMEHW_IRQ_RESERVED_62                     62 /* 0x3e */
#define CMEHW_IRQ_RESERVED_63                     63 /* 0x3f */

// Please keep the string definitions up-to-date as they are used for
// reporting in the Simics simulation.

#define CMEHW_IRQ_STRINGS(var)                      \
    const char* var[CMEHW_IRQS] = {                 \
                                                    "CMEHW_IRQ_DEBUGGER",                 \
                                                    "CMEHW_IRQ_DEBUG_TRIGGER",            \
                                                    "CMEHW_IRQ_QUAD_CHECKSTOP",           \
                                                    "CMEHW_IRQ_PVREF_FAIL",               \
                                                    "CMEHW_IRQ_OCC_HEARTBEAT_LOST",       \
                                                    "CMEHW_IRQ_CORE_CHECKSTOP",           \
                                                    "CMEHW_IRQ_DROPOUT_DETECT",           \
                                                    "CMEHW_IRQ_SPARE_7",                  \
                                                    "CMEHW_IRQ_BCE_BUSY_HIGH",            \
                                                    "CMEHW_IRQ_BCE_TIMEOUT",              \
                                                    "CMEHW_IRQ_DOORBELL3_C0",             \
                                                    "CMEHW_IRQ_DOORBELL3_C1",             \
                                                    "CMEHW_IRQ_PC_INTR_PENDING_C0",       \
                                                    "CMEHW_IRQ_PC_INTR_PENDING_C1",       \
                                                    "CMEHW_IRQ_SPECIAL_WAKEUP_C0",        \
                                                    "CMEHW_IRQ_SPECIAL_WAKEUP_C1",        \
                                                    "CMEHW_IRQ_REG_WAKEUP_C0",            \
                                                    "CMEHW_IRQ_REG_WAKEUP_C1",            \
                                                    "CMEHW_IRQ_DOORBELL2_C0",             \
                                                    "CMEHW_IRQ_DOORBELL2_C1",             \
                                                    "CMEHW_IRQ_PC_PM_STATE_ACTIVE_C0",    \
                                                    "CMEHW_IRQ_PC_PM_STATE_ACTIVE_C1",    \
                                                    "CMEHW_IRQ_L2_PURGE_DONE",            \
                                                    "CMEHW_IRQ_NCU_PURGE_DONE",           \
                                                    "CMEHW_IRQ_CHTM_PURGE_DONE_C0",       \
                                                    "CMEHW_IRQ_CHTM_PURGE_DONE_C1",       \
                                                    "CMEHW_IRQ_BCE_BUSY_LOW",             \
                                                    "CMEHW_IRQ_FANAL_VDM_DATA0",          \
                                                    "CMEHW_IRQ_FANAL_VDM_DATA1",          \
                                                    "CMEHW_IRQ_COMM_RECVD",               \
                                                    "CMEHW_IRQ_COMM_SEND_ACK",            \
                                                    "CMEHW_IRQ_COMM_SEND_NACK",           \
                                                    "CMEHW_IRQ_SPARE_32",                 \
                                                    "CMEHW_IRQ_SPARE_33",                 \
                                                    "CMEHW_IRQ_PMCR_UPDATE_C0",           \
                                                    "CMEHW_IRQ_PMCR_UPDATE_C1",           \
                                                    "CMEHW_IRQ_DOORBELL0_C0",             \
                                                    "CMEHW_IRQ_DOORBELL0_C1",             \
                                                    "CMEHW_IRQ_SPARE_38",                 \
                                                    "CMEHW_IRQ_SPARE_39",                 \
                                                    "CMEHW_IRQ_DOORBELL1_C0",             \
                                                    "CMEHW_IRQ_DOORBELL1_C1",             \
                                                    "CMEHW_IRQ_PECE_INTR_DISABLED_C0",    \
                                                    "CMEHW_IRQ_PECE_INTR_DISABLED_C1",    \
                                                    "CMEHW_IRQ_RESERVED_44",              \
                                                    "CMEHW_IRQ_RESERVED_45",              \
                                                    "CMEHW_IRQ_RESERVED_46",              \
                                                    "CMEHW_IRQ_RESERVED_47",              \
                                                    "CMEHW_IRQ_RESERVED_48",              \
                                                    "CMEHW_IRQ_RESERVED_49",              \
                                                    "CMEHW_IRQ_RESERVED_50",              \
                                                    "CMEHW_IRQ_RESERVED_51",              \
                                                    "CMEHW_IRQ_RESERVED_52",              \
                                                    "CMEHW_IRQ_RESERVED_53",              \
                                                    "CMEHW_IRQ_RESERVED_54",              \
                                                    "CMEHW_IRQ_RESERVED_55",              \
                                                    "CMEHW_IRQ_RESERVED_56",              \
                                                    "CMEHW_IRQ_RESERVED_57",              \
                                                    "CMEHW_IRQ_RESERVED_58",              \
                                                    "CMEHW_IRQ_RESERVED_59",              \
                                                    "CMEHW_IRQ_RESERVED_60",              \
                                                    "CMEHW_IRQ_RESERVED_61",              \
                                                    "CMEHW_IRQ_RESERVED_62",              \
                                                    "CMEHW_IRQ_RESERVED_63",              \
                                  };


////////////////////////////////////////////////////////////////////////////
// IRQ Configuration
////////////////////////////////////////////////////////////////////////////

/// Static configuration data for external interrupts:
///
/// IRQ#, TYPE, POLARITY, ENABLE
///
#define APPCFG_EXT_IRQS_CONFIG \
    CMEHW_IRQ_DEBUGGER              STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_DEBUG_TRIGGER         STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_QUAD_CHECKSTOP        STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_PVREF_FAIL            STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_OCC_HEARTBEAT_LOST    STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_CORE_CHECKSTOP        STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_DROPOUT_DETECT        STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_SPARE_7               STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_BCE_BUSY_HIGH         STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_BCE_TIMEOUT           STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_DOORBELL3_C0          STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_DOORBELL3_C1          STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_PC_INTR_PENDING_C0    STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_PC_INTR_PENDING_C1    STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_SPECIAL_WAKEUP_C0     STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_SPECIAL_WAKEUP_C1     STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_REG_WAKEUP_C0         STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_REG_WAKEUP_C1         STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_DOORBELL2_C0          STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_DOORBELL2_C1          STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_PC_PM_STATE_ACTIVE_C0 STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_PC_PM_STATE_ACTIVE_C1 STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_L2_PURGE_DONE         STD_IRQ_TYPE_LEVEL   STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_NCU_PURGE_DONE        STD_IRQ_TYPE_LEVEL   STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_CHTM_PURGE_DONE_C0    STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_CHTM_PURGE_DONE_C1    STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_BCE_BUSY_LOW          STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_FINAL_VDM_DATA0       STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_FINAL_VDM_DATA1       STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_COMM_RECVD            STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_COMM_SEND_ACK         STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_COMM_SEND_NACK        STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_SPARE_32              STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_SPARE_33              STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_PMCR_UPDATE_C0        STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_PMCR_UPDATE_C1        STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_DOORBELL0_C0          STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_DOORBELL0_C1          STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_SPARE_38              STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_SPARE_39              STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_DOORBELL1_C0          STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_DOORBELL1_C1          STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_PECE_INTR_DISABLED_C0 STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_PECE_INTR_DISABLED_C1 STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_RESERVED_44           STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_RESERVED_45           STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_RESERVED_46           STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_RESERVED_47           STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_RESERVED_48           STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_RESERVED_49           STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_RESERVED_50           STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_RESERVED_51           STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_RESERVED_52           STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_RESERVED_53           STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_RESERVED_54           STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_RESERVED_55           STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_RESERVED_56           STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_RESERVED_57           STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_RESERVED_58           STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_RESERVED_59           STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_RESERVED_60           STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_RESERVED_61           STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_RESERVED_62           STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    CMEHW_IRQ_RESERVED_63           STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED


#define APPCFG_IRQ_INVALID_MASK 0
/*
/// This 64 bit mask specifies which of the interrupts are not to be used.
#define APPCFG_IRQ_INVALID_MASK \
(\
   STD_IRQ_MASK64(CMEHW_IRQ_RESERVED_44) |  \
   STD_IRQ_MASK64(CMEHW_IRQ_RESERVED_45) |  \
   STD_IRQ_MASK64(CMEHW_IRQ_RESERVED_46) |  \
   STD_IRQ_MASK64(CMEHW_IRQ_RESERVED_47) |  \
   STD_IRQ_MASK64(CMEHW_IRQ_RESERVED_48) |  \
   STD_IRQ_MASK64(CMEHW_IRQ_RESERVED_49) |  \
   STD_IRQ_MASK64(CMEHW_IRQ_RESERVED_50) |  \
   STD_IRQ_MASK64(CMEHW_IRQ_RESERVED_51) |  \
   STD_IRQ_MASK64(CMEHW_IRQ_RESERVED_52) |  \
   STD_IRQ_MASK64(CMEHW_IRQ_RESERVED_53) |  \
   STD_IRQ_MASK64(CMEHW_IRQ_RESERVED_54) |  \
   STD_IRQ_MASK64(CMEHW_IRQ_RESERVED_55) |  \
   STD_IRQ_MASK64(CMEHW_IRQ_RESERVED_56) |  \
   STD_IRQ_MASK64(CMEHW_IRQ_RESERVED_57) |  \
   STD_IRQ_MASK64(CMEHW_IRQ_RESERVED_58) |  \
   STD_IRQ_MASK64(CMEHW_IRQ_RESERVED_59) |  \
   STD_IRQ_MASK64(CMEHW_IRQ_RESERVED_60) |  \
   STD_IRQ_MASK64(CMEHW_IRQ_RESERVED_61) |  \
   STD_IRQ_MASK64(CMEHW_IRQ_RESERVED_62) |  \
   STD_IRQ_MASK64(CMEHW_IRQ_RESERVED_63))
*/

#endif  /* __CMEHW_INTERRUPTS_H__ */
