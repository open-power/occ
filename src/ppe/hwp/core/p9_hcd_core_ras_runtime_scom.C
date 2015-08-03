/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/hwp/core/p9_hcd_core_ras_runtime_scom.C $             */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015                             */
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
///
/// @file  p9_hcd_core_ras_runtime_scom.C
/// @brief FSP/Host run-time SCOMS
///
/// *HWP HWP Owner   : David Du      <daviddu@us.ibm.com>
/// *HWP FW Owner    : Reshmi Nair   <resnair5@in.ibm.com>
/// *HWP Team        : PM
/// *HWP Consumed by : SBE:CME
/// *HWP Level       : 1
///
/// Procedure Summary:
///   Run-time updates from FSP based PRD, etc that are put on the core image
///   by STOP API calls
///   Dynamically built pointer where a NULL is checked before execution
///   If NULL (the SBE case), return
///   Else call the function at the pointer;
///   pointer is filled in by STOP image build
///   Run-time updates from Host code that are put on the core image by
///   STOP API calls
///     Restore Hypervisor, Host PRD, etc. SCOMs
///

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fapi2.H>
//#include <common_scom_addresses.H>
//will be replaced with real scom address header file
#include "p9_hcd_core_ras_runtime_scom.H"

//-----------------------------------------------------------------------------
// Constant Definitions
//-----------------------------------------------------------------------------
#define host_runtime_scom 0

//-----------------------------------------------------------------------------
// Procedure: FSP/Host run-time SCOMS
//-----------------------------------------------------------------------------

extern "C"
{

fapi2::ReturnCode
p9_hcd_core_ras_runtime_scom(
    const fapi2::Target<fapi2::TARGET_TYPE_CORE>& i_target)
{

#if 0
    fapi2::buffer<uint64_t> data;

    // Run the SCOM sequence if the SCOM procedure is defined
    // -   la      A0, sp_runtime_scom
    // -   ld      D0, 0, A0
    // -   braz    D0, 1f
    //FAPI_INF("Launching SP Runtime SCOM routine")
    // -   bsrd    D0
    // -   1:
    //

    // Run the SCOM sequence if the SCOM procedure is defined.
    // -   la      A0, host_runtime_scom
    // -   ld      D1, 0, A0
    // -   braz    D1, 1f

    // Prep P1
    // -   setp1_mcreadand D0
#if 0
    // Disable the AISS to allow the override
    // -   ld      D0, EX_OHA_MODE_REG_RWx1002000D, P1
    // -   andi    D0, D0, ~(BIT(1))
    // - std     D0, EX_OHA_MODE_REG_RWx1002000D, P0
    // Drop PSCOM fence to allow SCOM and set pm_wake-up to PC to accepts
    // RAMs (SCOMs actually) in the IPL "Nap" state
    // -   ld      D0, EX_OHA_AISS_IO_REG_0x10020014, P1
    // -   ori     D0, D0, (BIT(15))
    // -   andi    D0, D0, ~(BIT(21))
    // -   std     D0, EX_OHA_AISS_IO_REG_0x10020014, P0
#endif
    // Branch to sub_slw_runtime_scom()
    FAPI_INF("Launching Host Runtime SCOM routine")
    // -   bsrd    D1

    // Prep P1
    // -   setp1_mcreadand D0
#if 0
    // Clear regular wake-up and restore PSCOM fence in OHA
    // These were established in p9_sbe_ex_scominit.S
    // -   ld      D0, EX_OHA_AISS_IO_REG_0x10020014, P1
    // -   andi    D0, D0, ~(BIT(15))
    // -   ori     D0, D0, BIT(21)
    // -   std     D0, EX_OHA_AISS_IO_REG_0x10020014, P0
    // Enable the AISS to allow further operation
    // -   ld      D0, EX_OHA_MODE_REG_RWx1002000D, P1
    // -   ori     D0, D0, (BIT(1))
    // -   std     D0, EX_OHA_MODE_REG_RWx1002000D, P0
#endif
    // -   bra     2f
    // -   1:
    // To accomodate IPL flow, where sub_slw_runtime_scom() is skipped
    // - setp1_mcreadand D0
#if 0
    // Clear regular wake-up and restore PSCOM fence in OHA
    // These were established in p9_sbe_ex_scominit.S
    // -   ld      D0, EX_OHA_MODE_REG_RWx1002000D, P1
    // -   andi    D0, D0, ~BIT(1)
    // -   std     D0, EX_OHA_MODE_REG_RWx1002000D, P0
    // -   ld      D0, EX_OHA_AISS_IO_REG_0x10020014, P1
    // -   andi    D0, D0, ~(BIT(15))
    // -   ori     D0, D0, BIT(21)
    // -   std     D0, EX_OHA_AISS_IO_REG_0x10020014, P0
    // Enable the AISS to allow further operation
    // -   ld      D0, EX_OHA_MODE_REG_RWx1002000D, P1
    // -   ori     D0, D0, (BIT(1))
    // -   std     D0, EX_OHA_MODE_REG_RWx1002000D, P0
#endif
    // -   2:

    // If using cv_multicast, we need to set the magic istep number here
    // -   la      A0, p9_sbe_select_ex_control
    // -   ldandi  D0, 0, A0, P9_CONTROL_INIT_ALL_EX
    // -   braz    D0, 3f
    FAPI_DBG("Setting istep num to magic number because cv_multicast is set")
    // -   lpcs    P1, MBOX_SBEVITAL_0x0005001C
    // -   sti     MBOX_SBEVITAL_0x0005001C, P1, (P9_SBE_EX_RAS_RUNTIME_SCOM_MAGIC_ISTEP_NUM << (4+32))
    // -   3:

    return fapi2::FAPI2_RC_SUCCESS;

    FAPI_CLEANUP();
    return fapi2::FAPI2_RC_PLAT_ERR_SEE_DATA;

#endif

    return fapi2::FAPI2_RC_SUCCESS;

} // Procedure


} // extern C


