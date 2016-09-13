/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/include/occhw_pba_common.h $                              */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2016                             */
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
#ifndef __OCCHW_PBA_COMMON_H__
#define __OCCHW_PBA_COMMON_H__

#include "pba_register_addresses.h"
#include "pba_firmware_registers.h"

// Abstract fields of the PBA Slave Reset register used in pba_slave_reset(),
// which checks 'n' for validity.p

#define PBA_SLVRST_SET(n) (4 + (n))
#define PBA_SLVRST_IN_PROG(n) (0x8 >> (n))

/// The default timeout for pba_slave_reset().
///
/// Currently the procedure pba_slave_reset() is thought to be an
/// initialization-only and/or lab-only procedure, so this long polling
/// timeout is not a problem.
#ifndef PBA_SLAVE_RESET_TIMEOUT
    #define PBA_SLAVE_RESET_TIMEOUT SSX_MICROSECONDS(100)
#endif

////////////////////////////////////
// Macros for fields of PBA_SLVCTLn
////////////////////////////////////

// PBA write Ttypes

#define PBA_WRITE_TTYPE_DMA_PR_WR    0x0 /// DMA Partial Write
#define PBA_WRITE_TTYPE_LCO_M        0x1 /// L3 LCO for IPL, Tsize denotes chiplet.
#define PBA_WRITE_TTYPE_ATOMIC_RMW   0x2 /// Atomic operations
#define PBA_WRITE_TTYPE_CACHE_INJECT 0x3 /// Cache inject after IPL
#define PBA_WRITE_TTYPE_CI_PR_W      0x4 /// Cache-inhibited partial write for Centaur putscom().

#define PBA_WRITE_TTYPE_DC PBA_WRITE_TTYPE_DMA_PR_WR // Don't care


// PBA write Tsize is only required for PBA_WRITE_TTYPE_LCO_M (where it
// actually specifies a core chiplet id) and PBA_WRITE_TTYPE_ATOMIC_RMW.

#define PBA_WRITE_TSIZE_CHIPLET(chiplet) (chiplet)

#define PBA_WRITE_TSIZE_ARMW_ADD 0x02
#define PBA_WRITE_TSIZE_ARMW_AND 0x22
#define PBA_WRITE_TSIZE_ARMW_OR  0x42
#define PBA_WRITE_TSIZE_ARMW_XOR 0x62

#define PBA_WRITE_TSIZE_DC 0x0


// PBA write gather timeouts are defined in terms of the number of 'pulses'. A
// pulse occurs every 64 OCI cycles. The timing of the last write of a
// sequence is variable, so the timeout will occur somewhere between (N - 1) *
// 64 and N * 64 OCI cycles.  If write gather timeouts are disabled, the PBA
// holds the data until some condition occurs that causes it to disgorge the
// data. Slaves using cache-inhibited partial write never gather write
// data. Note from spec. : "Write gather timeouts must NOT be disabled if
// multiple masters are enabled to write through the PBA".  The only case
// where write gather timeouts will be disabled is for the IPL-time injection
// of data into the L3 caches. Note that disabling write-gathering timeouts is
// different from disabling write-gathering altogether with the
// SLVCTL.dis_write_gather setting.

#define PBA_WRITE_GATHER_TIMEOUT_DISABLE   0x0
#define PBA_WRITE_GATHER_TIMEOUT_2_PULSES  0x4
#define PBA_WRITE_GATHER_TIMEOUT_4_PULSES  0x5
#define PBA_WRITE_GATHER_TIMEOUT_8_PULSES  0x6
#define PBA_WRITE_GATHER_TIMEOUT_16_PULSES 0x7

/// PBA write gather timeout don't care assignment - see gpe_pba_parms_create()
#define PBA_WRITE_GATHER_TIMEOUT_DC -1


// PBA read Ttype

#define PBA_READ_TTYPE_CL_RD_NC      0x0 /// Cache line read
#define PBA_READ_TTYPE_CI_PR_RD      0x1 /// Cache-inhibited partial read for Centaur getscom().

/// PBA read TTYPE don't care assignment
#define PBA_READ_TTYPE_DC PBA_READ_TTYPE_CL_RD_NC


// PBA read prefetch options

#define PBA_READ_PREFETCH_AUTO_EARLY  0x0 /// Aggressive prefetch
#define PBA_READ_PREFETCH_NONE        0x1 /// No prefetch
#define PBA_READ_PREFETCH_AUTO_LATE   0x2 /// Non-aggressive prefetch

/// PBA read prefetch don't care assignment - see gpe_pba_parms_create()
#define PBA_READ_PREFETCH_DC -1

#endif
