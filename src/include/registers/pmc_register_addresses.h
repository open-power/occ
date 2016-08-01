/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/include/registers/pmc_register_addresses.h $              */
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
#ifndef __PMC_REGISTER_ADDRESSES_H__
#define __PMC_REGISTER_ADDRESSES_H__

/// \file pmc_register_addresses.h
/// \brief Symbolic addresses for the PMC unit

// *** WARNING *** - This file is generated automatically, do not edit.


#define PMC_OCI_BASE 0x40010000
#define PMC_MODE_REG 0x40010000
#define PMC_HARDWARE_AUCTION_PSTATE_REG 0x40010008
#define PMC_PSTATE_MONITOR_AND_CTRL_REG 0x40010010
#define PMC_RAIL_BOUNDS_REGISTER 0x40010018
#define PMC_GLOBAL_PSTATE_BOUNDS_REG 0x40010020
#define PMC_PARAMETER_REG0 0x40010028
#define PMC_PARAMETER_REG1 0x40010030
#define PMC_EFF_GLOBAL_ACTUAL_VOLTAGE_REG 0x40010038
#define PMC_GLOBAL_ACTUAL_VOLTAGE_REG 0x40010040
#define PMC_STATUS_REG 0x40010048
#define PMC_PHASE_ENABLE_REG 0x40010050
#define PMC_UNDERVOLTING_REG 0x40010060
#define PMC_CORE_DECONFIGURATION_REG 0x40010068
#define PMC_INTCHP_CTRL_REG1 0x40010080
#define PMC_INTCHP_CTRL_REG2 0x40010088
#define PMC_INTCHP_CTRL_REG4 0x40010090
#define PMC_INTCHP_STATUS_REG 0x40010098
#define PMC_INTCHP_COMMAND_REG 0x400100a0
#define PMC_INTCHP_MSG_WDATA 0x400100a8
#define PMC_INTCHP_MSG_RDATA 0x400100b0
#define PMC_INTCHP_PSTATE_REG 0x400100b8
#define PMC_INTCHP_GLOBACK_REG 0x400100c0
#define PMC_FSMSTATE_STATUS_REG 0x40010100
#define PMC_TRACE_MODE_REG 0x40010180
#define PMC_SPIV_CTRL_REG0A 0x40010200
#define PMC_SPIV_CTRL_REG0B 0x40010208
#define PMC_SPIV_CTRL_REG1 0x40010210
#define PMC_SPIV_CTRL_REG2 0x40010218
#define PMC_SPIV_CTRL_REG3 0x40010220
#define PMC_SPIV_CTRL_REG4 0x40010228
#define PMC_SPIV_STATUS_REG 0x40010230
#define PMC_SPIV_COMMAND_REG 0x40010238
#define PMC_O2S_CTRL_REG0A 0x40010280
#define PMC_O2S_CTRL_REG0B 0x40010288
#define PMC_O2S_CTRL_REG1 0x40010290
#define PMC_O2S_CTRL_REG2 0x40010298
#define PMC_O2S_CTRL_REG4 0x400102a8
#define PMC_O2S_STATUS_REG 0x400102b0
#define PMC_O2S_COMMAND_REG 0x400102b8
#define PMC_O2S_WDATA_REG 0x400102c0
#define PMC_O2S_RDATA_REG 0x400102c8
#define PMC_O2P_ADDR_REG 0x40010300
#define PMC_O2P_CTRL_STATUS_REG 0x40010308
#define PMC_O2P_SEND_DATA_HI_REG 0x40010310
#define PMC_O2P_SEND_DATA_LO_REG 0x40010318
#define PMC_O2P_RECV_DATA_HI_REG 0x40010320
#define PMC_O2P_RECV_DATA_LO_REG 0x40010328
#define PMC_OCC_HEARTBEAT_REG 0x40010330
#define PMC_ERROR_INT_MASK_HI_REG 0x40010338
#define PMC_ERROR_INT_MASK_LO_REG 0x40010340
#define PMC_IDLE_SUSPEND_MASK_REG 0x40010348
#define PMC_PEND_IDLE_REQ_REG0 0x40010400
#define PMC_PEND_IDLE_REQ_REG1 0x40010408
#define PMC_PEND_IDLE_REQ_REG2 0x40010410
#define PMC_PEND_IDLE_REQ_REG3 0x40010418
#define PMC_SLEEP_INT_REQ_VEC_REG0 0x40010420
#define PMC_SLEEP_INT_REQ_VEC_REG1 0x40010428
#define PMC_SLEEP_INT_REQ_VEC_REG2 0x40010430
#define PMC_SLEEP_INT_REQ_VEC_REG3 0x40010438
#define PMC_WINKLE_INT_REQ_VEC_REG0 0x40010440
#define PMC_WINKLE_INT_REQ_VEC_REG1 0x40010448
#define PMC_WINKLE_INT_REQ_VEC_REG2 0x40010450
#define PMC_WINKLE_INT_REQ_VEC_REG3 0x40010458
#define PMC_NAP_INT_REQ_VEC_REG0 0x40010460
#define PMC_NAP_INT_REQ_VEC_REG1 0x40010468
#define PMC_PORE_REQ_REG0 0x40010470
#define PMC_PORE_REQ_REG1 0x40010478
#define PMC_PORE_REQ_STAT_REG 0x40010480
#define PMC_PORE_REQ_TOUT_TH_REG 0x40010488
#define PMC_DEEP_EXIT_MASK_REG 0x40010490
#define PMC_DEEP_EXIT_MASK_REG_AND 0x40010500
#define PMC_DEEP_EXIT_MASK_REG_OR 0x40010508
#define PMC_CORE_PSTATE_REG0 0x400104a0
#define PMC_CORE_PSTATE_REG1 0x400104a8
#define PMC_CORE_PSTATE_REG2 0x400104b0
#define PMC_CORE_PSTATE_REG3 0x400104b8
#define PMC_CORE_POWER_DONATION_REG 0x400104c0
#define PMC_PMAX_SYNC_COLLECTION_REG 0x400104c8
#define PMC_PMAX_SYNC_COLLECTION_MASK_REG 0x400104d0
#define PMC_GPSA_ACK_COLLECTION_REG 0x400104d8
#define PMC_GPSA_ACK_COLLECTION_MASK_REG 0x400104e0
#define PMC_PORE_SCRATCH_REG0 0x400104e8
#define PMC_PORE_SCRATCH_REG1 0x400104f0
#define PMC_DEEP_IDLE_EXIT_REG 0x400104f8
#define PMC_DEEP_STATUS_REG 0x40010510
#define PMC_PIB_BASE 0x01010840
#define PMC_LFIR_ERR_REG 0x01010840
#define PMC_LFIR_ERR_REG_AND 0x01010841
#define PMC_LFIR_ERR_REG_OR 0x01010842
#define PMC_LFIR_ERR_MASK_REG 0x01010843
#define PMC_LFIR_ERR_MASK_REG_AND 0x01010844
#define PMC_LFIR_ERR_MASK_REG_OR 0x01010845
#define PMC_LFIR_ACTION0_REG 0x01010846
#define PMC_LFIR_ACTION1_REG 0x01010847
#define PMC_LFIR_WOF_REG 0x01010848

#endif // __PMC_REGISTER_ADDRESSES_H__

