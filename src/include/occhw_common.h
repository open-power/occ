/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/include/occhw_common.h $                                  */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2016                        */
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
#ifndef __OCCHW_COMMON_H__
#define __OCCHW_COMMON_H__

/// \file occhw_common.h
/// \brief Common header for SSX and PMX versions of OCCHW
///
/// This header is maintained as part of the SSX port for OCCHW, but needs to be
/// physically present in the PMX area to allow dropping PMX code as a whole
/// to other teams.

// -*- WARNING: This file is maintained as part of SSX.  Do not edit in -*-
// -*- the PMX area as your edits will be lost.                         -*-

#include "occhw_interrupts.h"
#include "occhw_irq_config.h"

#define EXTERNAL_IRQS OCCHW_IRQS

#ifndef __ASSEMBLER__
#include <stdint.h>
extern unsigned int g_ocb_timer_divider; //grm
#endif

////////////////////////////////////////////////////////////////////////////
// Configuration
////////////////////////////////////////////////////////////////////////////

#define OCCHW_NCORES            16
#define OCCHW_NCORE_PARTITIONS   4
#define OCCHW_NMCS               8
#define OCCHW_NCENTAUR           8
#define OCCHW_NTHREADS           8
#define OCCHW_NDTSCPM            4

#ifndef PROCESSOR_EC_LEVEL
#define MURANO_DD10 1
#else
#define MURANO_DD10 0
#endif

#ifndef SIMICS_ENVIRONMENT
#define SIMICS_ENVIRONMENT 0
#endif

/// OCC instance ID's that can be read from the PPE42 PIR and used in IPC operations.
/// NOTE: The PPC instance ID is not associated with a register and was assigned to be
///       four as it was most convenient to do so in the code.
#define OCCHW_MAX_INSTANCES 5

#define OCCHW_INST_ID_GPE0 0
#define OCCHW_INST_ID_GPE1 1
#define OCCHW_INST_ID_GPE2 2
#define OCCHW_INST_ID_GPE3 3
#define OCCHW_INST_ID_PPC  4
#define OCCHW_INST_ID_MAX (OCCHW_MAX_INSTANCES - 1)
#define OCCHW_INST_ID_MAX_GPE 3

/// Fail to compile if APPCFG_OCC_INSTANCE_ID is not defined somewhere or is out of range
#ifndef APPCFG_OCC_INSTANCE_ID
#error "APPCFG_OCC_INSTANCE_ID must be defined by the application"
#else
#if ((APPCFG_OCC_INSTANCE_ID > OCCHW_INST_ID_MAX) || (APPCFG_OCC_INSTANCE_ID < 0))
#warning "APPCFG_OCC_INSTANCE_ID is out of range"
#endif
#endif
#define OCCHW_INST_ID_SELF APPCFG_OCC_INSTANCE_ID

////////////////////////////////////////////////////////////////////////////
// Clocking
////////////////////////////////////////////////////////////////////////////
//
// The SSX timebase is driven by the pervasive clock, which is nest / 4.  This
// will typically be 600MHz, but may be 500MHz for power-constrained system
// designs. 

/// The pervasive hang timer divider used for the OCB timer
///
/// This is supposed to yield an approximately 1us timer, however for MURANO
/// DD10 we need to use an approximate 64us timer

#if MURANO_DD10
#define OCB_TIMER_DIVIDER_DEFAULT (64 * 512)
#else
#define OCB_TIMER_DIVIDER_DEFAULT 512
#endif

/// This is set to the above default at compile time but may be updated
/// at run time. grm
//#define OCB_TIMER_DIVIDER g_ocb_timer_divider
#define OCB_TIMER_DIVIDER OCB_TIMER_DIVIDER_DEFAULT

/// The OCB timer frequency
#define OCB_TIMER_FREQUENCY_HZ (SSX_TIMEBASE_FREQUENCY_HZ / OCB_TIMER_DIVIDER)

/// The pervasive hang timer divider used for the PMC (same as OCB timer)
#define PMC_TIMER_DIVIDER OCB_TIMER_DIVIDER

/// The PMC hang pulse frequency
#define PMC_HANG_PULSE_FREQUENCY_HZ \
    (SSX_TIMEBASE_FREQUENCY_HZ / PMC_TIMER_DIVIDER)

/// The pervasive hang timer divider for PCBS 'fast' timers
///
/// This timer yeilds an approximate 100ns pulse with a 2.4 GHz pervasive clock
#define PCBS_FAST_TIMER_DIVIDER 64

/// The pervasive hang timer divider for PCBS 'slow' timers
///
/// This timer yeilds an approximate 1us pulse with a 2.4 GHz pervasive clock
#define PCBS_SLOW_TIMER_DIVIDER 512

/// The PCBS slow divider frequency
#define PCBS_SLOW_HANG_PULSE_FREQUENCY_HZ \
    (SSX_TIMEBASE_FREQUENCY_HZ / PCBS_SLOW_TIMER_DIVIDER)

/// The PCBS occ heartbeat pulse is predivided in hardware by 64
#define PCBS_HEARTBEAT_DIVIDER \
    (PCBS_SLOW_TIMER_DIVIDER * 64)

/// The PCBS heartbeat pulse frequency
#define PCBS_HEARTBEAT_PULSE_FREQUENCY_HZ \
    (SSX_TIMEBASE_FREQUENCY_HZ / PCBS_HEARTBEAT_DIVIDER)



////////////////////////////////////////////////////////////////////////////
// OCI
////////////////////////////////////////////////////////////////////////////

// OCI Master Id assigments - required for PBA slave programming.  These Ids
// also appear as bits 12:15 of the OCI register space addresses of the OCI
// registers for each device that contains OCI-addressable registers (GPE,
// PMC, PBA, SLW and OCB).

#define OCI_TRANSPORT_DELAY    10

#define OCI_MASTER_ID_PHONY    -1
#define OCI_MASTER_ID_GPE0     0
#define OCI_MASTER_ID_GPE1     1
#define OCI_MASTER_ID_GPE2     2
#define OCI_MASTER_ID_GPE3     3
#define OCI_MASTER_ID_PBA      4
#define OCI_MASTER_ID_OCC_ICU  5
#define OCI_MASTER_ID_OCB      6
#define OCI_MASTER_ID_OCC_DCU  7


////////////////////////////////////////////////////////////////////////////
// PIB
////////////////////////////////////////////////////////////////////////////

#define PIB_TRANSPORT_DELAY     50

#define PIB_MASTER_ID_PHONY     -1
#define PIB_MASTER_ID_FSI2PIB   0x2
#define PIB_MASTER_ID_FSI_SHIFT 0x3
#define PIB_MASTER_ID_TOD       0x4
#define PIB_MASTER_ID_PMC       0x6
#define PIB_MASTER_ID_PORE_GPE  0x7
#define PIB_MASTER_ID_PORE_SLW  0x8
#define PIB_MASTER_ID_ADU       0x9
#define PIB_MASTER_ID_MEMS0     0xA
#define PIB_MASTER_ID_I2C_SLAVE 0xD
#define PIB_MASTER_ID_PORE_SBE  0xE


////////////////////////////////////////////////////////////////////////////
// OCB
////////////////////////////////////////////////////////////////////////////

/// The base address of the OCI control register space
#define OCI_REGISTER_SPACE_BASE 0xC0000000

/// The base address of the entire PIB port mapped by the OCB.  The
/// OCB-contained PIB registers are based at OCB_PIB_BASE.
#define OCB_PIB_SLAVE_BASE 0x00060000

/// The size of the OCI control register address space
///
/// There are at most 8 slaves, each of which maps 2**16 bytes of register
/// address space.
#define OCI_REGISTER_SPACE_SIZE POW2_32(19)

/// This macro converts an OCI register space address into a PIB address as
/// seen through the OCB direct bridge.
#define OCI2PIB(addr) ((((addr) & 0x0007ffff) >> 3) + OCB_PIB_SLAVE_BASE)


// OCB communication channel constants

#define OCB_INDIRECT_CHANNELS 4

#define OCB_RW_READ  0
#define OCB_RW_WRITE 1

#define OCB_STREAM_MODE_DISABLED 0
#define OCB_STREAM_MODE_ENABLED  1

#define OCB_STREAM_TYPE_LINEAR   0
#define OCB_STREAM_TYPE_CIRCULAR 1

#define OCB_INTR_ACTION_FULL      0
#define OCB_INTR_ACTION_NOT_FULL  1
#define OCB_INTR_ACTION_EMPTY     2
#define OCB_INTR_ACTION_NOT_EMPTY 3


////////////////////////////////////////////////////////////////////////////
// PMC
////////////////////////////////////////////////////////////////////////////
/*
#ifndef __ASSEMBLER__

/// A Pstate type
///
/// Pstates are signed, but our register access macros operate on unsigned
/// values.  To avoid bugs, Pstate register fields should always be extracted
/// to a variable of type Pstate.  If the size of Pstate variables ever
/// changes we will have to revisit this convention.
typedef uint8_t Pstate;

/// A DPLL frequency code
///
/// DPLL frequency codes moved from 8 to 9 bits going from P7 to P8
typedef uint16_t DpllCode;

/// A VRM11 VID code
typedef uint8_t Vid11;

#endif  // __ASSEMBLER__ 

/// The minimum Pstate
#define PSTATE_MIN -128

/// The maximum Pstate
#define PSTATE_MAX 127

/// The minimum \e legal DPLL frequency code
///
/// This is ~1GHz with a 33.3MHz tick frequency.
#define DPLL_MIN 0x01e

/// The maximum DPLL frequency code
#define DPLL_MAX 0x1ff

/// The minimum \a legal (non-power-off) VRM11 VID code
#define VID11_MIN 0x02

/// The maximum \a legal (non-power-off) VRM11 VID code
#define VID11_MAX 0xfd
*/

////////////////////////////////////////////////////////////////////////////
// PCB
////////////////////////////////////////////////////////////////////////////

/// Convert a core chiplet 0 SCOM address to the equivalent address for any
/// other core chiplet.
///
/// Note that it is unusual to address core chiplet SCOMs directly.  Normally
/// this is done as part of a GPE program where the program iterates over core
/// chiplets, using the chiplet-0 address + a programmable offset held in a
/// chiplet address register.  Therefore the only address macro defined is the
/// chiplet-0 address. This macro is used for the rare cases of explicit
/// getscom()/ putscom() to a particular chiplet.

#define CORE_CHIPLET_ADDRESS(addr, core) ((addr) + ((core) << 24))

// PCB/PMC interrupt packet constants

#define PCB_INTERRUPT_PACKET_PERVASIVE        0x0
#define PCB_INTERRUPT_PACKET_POWER_MANAGEMENT 0x7

#define PMC_INTERRUPT_TYPE_PSTATE  0x0
#define PMC_INTERRUPT_TYPE_IDLE    0x1
#define PMC_INTERRUPT_TYPE_NOTIFY  0x2

#define PMC_INTERRUPT_SLEEP_EXIT   0x2
#define PMC_INTERRUPT_WINKLE_EXIT  0x3
#define PMC_INTERRUPT_SLEEP_ENTRY  0x6
#define PMC_INTERRUPT_WINKLE_ENTRY 0x7

// Reserved #define PMC_INTERRUPT_NAP_EXIT   0x1
// Reserved #define PMC_INTERRUPT_DOZE_ENTRY 0x4
// Reserved #define PMC_INTERRUPT_NAP_ENTRY  0x5

#define PMC_INTERRUPT_PMAX_SYNC  0x1
#define PMC_INTERRUPT_GPSA_ACK   0x2
#define PMC_INTERRUPT_DPLL_ERROR 0x3 // Spec says "placeholder"

// PCB Error codes

#define PCB_ERROR_NONE              0
#define PCB_ERROR_RESOURCE_OCCUPIED 1
#define PCB_ERROR_CHIPLET_OFFLINE   2
#define PCB_ERROR_PARTIAL_GOOD      3
#define PCB_ERROR_ADDRESS_ERROR     4
#define PCB_ERROR_CLOCK_ERROR       5
#define PCB_ERROR_PACKET_ERROR      6
#define PCB_ERROR_TIMEOUT           7

// PCB Multicast modes

#define PCB_MULTICAST_OR      0
#define PCB_MULTICAST_AND     1
#define PCB_MULTICAST_SELECT  2
#define PCB_MULTICAST_COMPARE 4
#define PCB_MULTICAST_WRITE   5

/// \defgroup pcb_multicast_groups PCB Multicast Groups
///
/// Technically the multicast groups are programmable; This is the multicast
/// grouping established by proc_sbe_chiplet_init().
///
/// - Group 0 : All functional chiplets (PRV PB XBUS ABUS PCIE TPCEX)
/// - Group 1 : All functional EX chiplets (no cores)
/// - Group 2 : All functional EX chiplets (core only)
/// - Group 3 : All functional chiplets except pervasive (PRV)
///
/// @{

#define MC_GROUP_ALL         0
#define MC_GROUP_EX          1
#define MC_GROUP_EX_CORE     2
#define MC_GROUP_ALL_BUT_PRV 3

/// @}


/// Convert any SCOM address to a multicast address
#define MC_ADDRESS(address, group, mode) \
    (((address) & 0x00ffffff) | ((0x40 | ((mode) << 3) | (group)) << 24))



////////////////////////////////////////////////////////////////////////////
// PBA
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////
// Macros for fields of PBA_MODECTL
////////////////////////////////////

/// The 64KB OCI HTM marker space is enabled by default at 0x40070000
///
/// See the comments for occhw_trace.h

#define PBA_OCI_MARKER_BASE 0x40070000


// SSX Kernel reserved trace addresses, see occhw_trace.h.

#define SSX_TRACE_CRITICAL_IRQ_ENTRY_BASE           0xf000
#define SSX_TRACE_CRITICAL_IRQ_EXIT_BASE            0xf100
#define SSX_TRACE_NONCRITICAL_IRQ_ENTRY_BASE        0xf200
#define SSX_TRACE_NONCRITICAL_IRQ_EXIT_BASE         0xf300
#define SSX_TRACE_THREAD_SWITCH_BASE                0xf400
#define SSX_TRACE_THREAD_SLEEP_BASE                 0xf500
#define SSX_TRACE_THREAD_WAKEUP_BASE                0xf600
#define SSX_TRACE_THREAD_SEMAPHORE_PEND_BASE        0xf700
#define SSX_TRACE_THREAD_SEMAPHORE_POST_BASE        0xf800
#define SSX_TRACE_THREAD_SEMAPHORE_TIMEOUT_BASE     0xf900
#define SSX_TRACE_THREAD_SUSPENDED_BASE             0xfa00
#define SSX_TRACE_THREAD_DELETED_BASE               0xfb00
#define SSX_TRACE_THREAD_COMPLETED_BASE             0xfc00
#define SSX_TRACE_THREAD_MAPPED_RUNNABLE_BASE       0xfd00
#define SSX_TRACE_THREAD_MAPPED_SEMAPHORE_PEND_BASE 0xfe00
#define SSX_TRACE_THREAD_MAPPED_SLEEPING_BASE       0xff00


// Please keep the string definitions up to date as they are used for
// reporting in the Simics simulation.

#define SSX_TRACE_STRINGS(var)                  \
    const char* var[16] = {                     \
        "Critical IRQ Entry             ",      \
        "Critical IRQ Exit              ",      \
        "Noncritical IRQ Entry          ",      \
        "Noncritical IRQ Exit           ",      \
        "Thread Switch                  ",      \
        "Thread Blocked : Sleep         ",      \
        "Thread Unblocked : Wakeup      ",      \
        "Thread Blocked : Semaphore     ",      \
        "Thread Unblocked : Semaphore   ",      \
        "Thread Unblocked : Sem. Timeout",      \
        "Thread Suspended               ",      \
        "Thread Deleted                 ",      \
        "Thread Completed               ",      \
        "Thread Mapped Runnable         ",      \
        "Thread Mapped Semaphore Pend.  ",      \
        "Thread Mapped Sleeping         ",      \
    };


// PBA transaction sizes for the block copy engines

#define PBA_BCE_OCI_TRANSACTION_32_BYTES 0
#define PBA_BCE_OCI_TRANSACTION_64_BYTES 1
#define PBA_BCE_OCI_TRANSACTION_8_BYTES  2


// PBAX communication channel constants

#define PBAX_CHANNELS 2

#define PBAX_INTR_ACTION_FULL      0
#define PBAX_INTR_ACTION_NOT_FULL  1
#define PBAX_INTR_ACTION_EMPTY     2
#define PBAX_INTR_ACTION_NOT_EMPTY 3


// PBA Write Buffer fields

#define PBA_WBUFVALN_STATUS_EMPTY     0x01
#define PBA_WBUFVALN_STATUS_GATHERING 0x02
#define PBA_WBUFVALN_STATUS_WAIT      0x04
#define PBA_WBUFVALN_STATUS_WRITING   0x08
#define PBA_WBUFVALN_STATUS_CRESPERR  0x10


////////////////////////////////////////////////////////////////////////////
// VRM
////////////////////////////////////////////////////////////////////////////

// These are the command types recognized by the VRMs

#define VRM_WRITE_VOLTAGE  0x0
#define VRM_READ_STATE     0xc
#define VRM_READ_VOLTAGE   0x3

// Voltage rail designations for the read voltage command
#define VRM_RD_VDD_RAIL    0x0
#define VRM_RD_VCS_RAIL    0x1


////////////////////////////////////////////////////////////////////////////
// OHA
////////////////////////////////////////////////////////////////////////////

// Power proxy trace record idle state encodings.  These encodings are unique
// to the Power proxy trace record.

#define PPT_IDLE_NON_IDLE     0x0
#define PPT_IDLE_NAP          0x1
#define PPT_IDLE_LIGHT_SLEEP  0x2
#define PPT_IDLE_FAST_SLEEP   0x3
#define PPT_IDLE_DEEP_SLEEP   0x4
#define PPT_IDLE_LIGHT_WINKLE 0x5
#define PPT_IDLE_FAST_WINKLE  0x6
#define PPT_IDLE_DEEP_WINKLE  0x7


////////////////////////////////////////////////////////////////////////////
// PC
////////////////////////////////////////////////////////////////////////////

// SPRC numbers for PC counters.  The low-order 3 bits are always defined as
// 0. The address can also be modified by OR-ing in 0x400 to indicate
// auto-increment addressing.  Note that the frequency-sensitivity counters
// are called "workrate" counters in the hardware documentation.
//
// Notes on the throttle counters:
//
// SPRN_IFU_THROTTLE_COUNTER
//     Cycles the IFU throttle was actually blocking fetch
//
//     <= if_pc_didt_throttle_blocked
//
// SPRN_ISU_THROTTLE_COUNTER
//     Cycles that ISU throttle was active and modeably IFU throttle request
//     was not
//
//     <= sd_pc_uthrottle_active AND 
//        (NOT scom_isuonly_count_mode OR NOT trigger_didt_throttle)
//
// SPRN_IFU_ACTIVE_COUNTER
//     Cycles that IFU throttle active input is asserted
//
//     <= if_pc_didt_throttle_active


/// \note The OCC SPRC/SPRD hardware has a bug that makes it such that the OCC
/// SPRC increments whenever the OCC SPRD is accessed, regardless of the
/// setting of the SPRN_PC_AUTOINCREMENT bit. This bug won't be fixed.

#define SPRN_CORE_INSTRUCTION_DISPATCH         0x200
#define SPRN_CORE_INSTRUCTION_COMPLETE         0x208
#define SPRN_CORE_FREQUENCY_SENSITIVITY_BUSY   0x210
#define SPRN_CORE_FREQUENCY_SENSITIVITY_FINISH 0x218
#define SPRN_CORE_RUN_CYCLE                    0x220
#define SPRN_CORE_RAW_CYCLE                    0x228
#define SPRN_CORE_MEM_HIER_A                   0x230
#define SPRN_CORE_MEM_HIER_B                   0x238
#define SPRN_CORE_MEM_C_LPAR(p)                (0x240 + (8 * (p)))
#define SPRN_WEIGHTED_INSTRUCTION_PROCESSING   0x260
#define SPRN_WEIGHTED_GPR_REGFILE_ACCESS       0x268
#define SPRN_WEIGHTED_VRF_REGFILE_ACCESS       0x270
#define SPRN_WEIGHTED_FLOATING_POINT_ISSUE     0x278
#define SPRN_WEIGHTED_CACHE_READ               0x280
#define SPRN_WEIGHTED_CACHE_WRITE              0x288
#define SPRN_WEIGHTED_ISSUE                    0x290
#define SPRN_WEIGHTED_CACHE_ACCESS             0x298
#define SPRN_WEIGHTED_VSU_ISSUE                0x2a0
#define SPRN_WEIGHTED_FXU_ISSUE                0x2a8

#define SPRN_THREAD_RUN_CYCLES(t)              (0x2b0 + (0x20 * (t)))
#define SPRN_THREAD_INSTRUCTION_COMPLETE(t)    (0x2b8 + (0x20 * (t)))
#define SPRN_THREAD_MEM_HIER_A(t)              (0x2c0 + (0x20 * (t)))
#define SPRN_THREAD_MEM_HIER_B(t)              (0x2c8 + (0x20 * (t)))

#define SPRN_IFU_THROTTLE_COUNTER              0x3b0
#define SPRN_ISU_THROTTLE_COUNTER              0x3b8
#define SPRN_IFU_ACTIVE_COUNTER                0x3c0

#define SPRN_PC_AUTOINCREMENT                  0x400


////////////////////////////////////////////////////////////////////////////
// Centaur
////////////////////////////////////////////////////////////////////////////

// DIMM sensor status codes

/// The next sampling period began before this sensor was read or the master
/// enable is off, or the individual sensor is disabled. If the subsequent
/// read completes on time, this will return to valid reading. Sensor data may
/// be accurate, but stale.  If due to a stall, the StallError FIR will be
/// set.
#define DIMM_SENSOR_STATUS_STALLED 0

/// The sensor data was not returned correctly either due to parity
/// error or PIB bus error code. Will return to valid if the next PIB
/// access to this sensor is valid, but a FIR will be set; Refer to FIR
/// for exact error. Sensor data should not be considered valid while
/// this code is present.
#define DIMM_SENSOR_STATUS_ERROR 1

/// Sensor data is valid, and has been valid since the last time this
/// register was read.
#define DIMM_SENSOR_STATUS_VALID_OLD 2

/// Sensor data is valid and has not yet been read by a SCOM. The status code
/// return to DIMM_SENSOR_STATUS_VALID_OLD after this register is read.
#define DIMM_SENSOR_STATUS_VALID_NEW 3


/// OCCHW common panic codes
#define OCCHW_INSTANCE_MISMATCH     0x00622400
#define OCCHW_IRQ_ROUTING_ERROR     0x00622401
#define OCCHW_XIR_INVALID_POINTER   0x00622402
#define OCCHW_XIR_INVALID_GPE       0x00622403


#endif  /* __OCCHW_COMMON_H__ */
