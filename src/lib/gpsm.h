/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/gpsm.h $                                              */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2014,2016                        */
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
#ifndef __GPSM_H__
#define __GPSM_H__
/// \file gpsm.h
/// \brief PgP Global Pstate Machine (Mechanism)

#include "ssx.h"
#include "gpe_control.h"
#include "pgp_async.h"
#include "pstates.h"

// GPSM modes

#define GPSM_MODE_HW          1
#define GPSM_MODE_FW_AUCTION  2
#define GPSM_MODE_FW          3

// Misc./Error/Panic codes

#define GPSM_INVALID_OBJECT                0x00477601
#define GPSM_INVALID_ARGUMENT_GPST_INSTALL 0x00477602
#define GPSM_INVALID_ARGUMENT_LPST_INSTALL 0x00477603
#define GPSM_INVALID_ARGUMENT_RCLK_INSTALL 0x00477604
#define GPSM_INVALID_ARGUMENT_EPSS         0x00477605
#define GPSM_ILLEGAL_MODE_HW_QUIESCE       0x00477606
#define GPSM_ILLEGAL_MODE_BGA              0x00477607
#define GPSM_ILLEGAL_MODE_GPST_INSTALL     0x00477608
#define GPSM_ILLEGAL_MODE_LPST_INSTALL     0x00477609
#define GPSM_ILLEGAL_MODE_RCLK_INSTALL     0x0047760a
#define GPSM_ILLEGAL_MODE_GPSM_INIT        0x0047760b
#define GPSM_ILLEGAL_MODE_EPSM             0x0047760c
#define GPSM_ILLEGAL_MODE_EPSS             0x0047760d
#define GPSM_SYNC_ERROR                    0x0047760e
#define GPSM_PSTATE_CLIPPED                0x0047760f
#define GPSM_BUG                           0x00477610
#define GPSM_CONFIGURATION_ERROR           0x00477611
#define GPSM_ERROR_BREAK                   0x00477612
#define GPSM_INVALID_MAGIC                 0x00477613
#define GPSM_IVRM_CALIBRATION_TIMEOUT      0x00477614
#define GPSM_IVRM_GROSS_OR_FINE            0x00477615
#define GPSM_PSTATE_ENABLED                0x00477616
#define GPSM_BABYSTEPPER_SYNC_TIMEOUT      0x00477617
#ifndef __ASSEMBLER__

// Lab/VBU/VPO debugging

#if 0
#include "trace.h"
#define TRACE_GPSM(i_code) trace_tbl_bbbb(1, i_code, 0, 0, 0);
#define TRACE_GPSM_B(i_code, i_b0) trace_tbl_bbbb(1, i_code, i_b0, 0, 0);
#define TRACE_GPSM_H(i_code, i_h0) trace_tbl_bbh(1, i_code, 0, i_h0);
#else
#define TRACE_GPSM(i_code)
#define TRACE_GPSM_B(i_code, i_b0)
#define TRACE_GPSM_H(i_code, i_h0)
#endif


/// Information required by an SCM or a DCM master to be passed from
/// gpsm_enable_pstates_master() to gpsm_enable_pstates_slave().

typedef struct {

    /// Indicates whether or not gpsm_enable_pstates_slave() should move the
    /// voltage. 
    ///
    /// If 0, it means that the master has already moved the voltage and only
    /// the frequency needs to move.  If 1, voltage is moved after frequency
    /// moves. 
    int move_voltage;

    /// The current and target external voltage settings as VRM11 VID codes.
    Vid11 currentVdd, currentVcs, targetVdd, targetVcs;

} GpsmEnablePstatesMasterInfo;


/// A GpsmAuctionProcedure is any function of no arguments that returns a
/// Pstate

typedef Pstate (*GpsmAuctionProcedure)();

extern SsxSemaphore G_gpsm_protocol_semaphore;

extern uint8_t G_gpsm_initialized;

// APIs defined in gpsm_init.c

int
gpsm_gpst_install(GlobalPstateTable* o_gpst,
                  const GlobalPstateTable* i_source);

int
gpsm_lpsa_install(const LocalPstateArray* i_lpsa,
                  const PstateOptions* i_options);

int
gpsm_resclk_install(const ResonantClockingSetup* i_resclk,
                    const GlobalPstateTable* i_gpst,
                    const PstateOptions* i_options);

int
gpsm_initialize(const PstateSuperStructure* i_pss, 
                GlobalPstateTable* o_gpst);

int
gpsm_enable_pstates_master(GpsmEnablePstatesMasterInfo* o_info,
                           Pstate* o_voltage_pstate,
                           Pstate* o_frequency_pstate);

int
gpsm_enable_pstates_slave(const GpsmEnablePstatesMasterInfo* i_info,
                          const Pstate i_voltage_pstate,
                          const Pstate i_frequency_pstate);

// APIs defined in gpsm.c

int
gpsm_quiesced_p(void);

int
gpsm_hw_mode_p(void);

int
gpsm_fw_auction_mode_p(void);

int
gpsm_fw_mode_p(void);

int
gpsm_dcm_mode_p(void);

int
gpsm_dcm_master_p(void);

int
gpsm_dcm_slave_p(void);

GlobalPstateTable*
gpsm_gpst();

int
gpsm_quiesce(void);

int
_gpsm_hw_quiesce(void);

int
gpsm_fw_mode(void);

int
gpsm_fw_auction_mode(void);

int
gpsm_hw_mode(void);

Pstate
gpsm_default_auction(void);

void
gpsm_get_global_bids(Pstate* o_bids);

void
gpsm_set_global_bids(const Pstate* i_bids);

void
_gpsm_broadcast_global_actual(const Pstate i_pstate, 
                              const gpst_entry_t i_entry);

int
gpsm_broadcast_global_actual(const GlobalPstateTable *i_gpst, 
                             const Pstate i_pstate, 
                             const int i_bias);

int
gpsm_set_pstate(const Pstate i_pstate);

int
gpsm_hold_auction(const GpsmAuctionProcedure i_procedure);

#endif  /* __ASSEMBLER__ */

#endif  /* __GPSM_H__ */
