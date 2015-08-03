/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/hwp/core/p9_hcd_core_scominit.C $                     */
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
/// @file  p9_hcd_core_scominit.C
/// @brief Core SCOM Inits
///
/// *HWP HWP Owner   : David Du      <daviddu@us.ibm.com>
/// *HWP FW Owner    : Reshmi Nair   <resnair5@in.ibm.com>
/// *HWP Team        : PM
/// *HWP Consumed by : SBE:CME
/// *HWP Level       : 1
///
/// Procedure Summary:
/// Apply any coded SCOM initialization to core
///

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fapi2.H>
//#include <common_scom_addresses.H>
//will be replaced with real scom address header file
#include "p9_hcd_core_scominit.H"


//-----------------------------------------------------------------------------
// Constant Definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Procedure: Core SCOM Inits
//-----------------------------------------------------------------------------

extern "C"
{

fapi2::ReturnCode
p9_hcd_core_scominit(
    const fapi2::Target<fapi2::TARGET_TYPE_CORE>& i_target)
{

#if 0
    fapi2::buffer<uint64_t> data;

    ///////////////////
    // Core
    ///////////////////

    // Force the hardware to think we're in special wakeup so the SCOMs will
    // succeed to the core due to the init state of Nap.  This does NOT
    // influence the PM state machines; only the wire indicating special
    // wake-up using the override in PMGP1
    // Bit 6 enables/disables override; bit 8 controls the Special Wake-up
    // = sti EX_PMGP1_OR_0x100F0105, P0, BIT(6) | BIT(8)
    FAPI_TRY(putScom(i_target, EX_PMGP1_OR_0x100F0105,
                     fapi2:buffer<uint64_t>().insertFromRight<6,3>(0x5)));
    // - setp1_mcreadand D0
#if 0
    // Disable the AISS to allow the override
    // - ld      D0, EX_OHA_MODE_REG_RWx1002000D, P1
    // - andi    D0, D0, ~(BIT(1))
    // - std     D0, EX_OHA_MODE_REG_RWx1002000D, P0
    // Drop PSCOM fence to allow SCOM and set pm_wake-up to PC to accepts
    // RAMs (SCOMs actually) in the IPL "Nap" state
    // - ld      D0, EX_OHA_AISS_IO_REG_0x10020014, P1
    // - ori     D0, D0, (BIT(15))
    // - andi    D0, D0, ~(BIT(21))
    // - std     D0, EX_OHA_AISS_IO_REG_0x10020014, P0
#endif
    //CMO-> Should prob reenable AISS here. But carefully...

    // These are dropped in p9_sbe_ex_host_runtime_scom.S

    ///////////////////
    // Clock Controller
    ///////////////////

    // Set the OPCG_PAD_VALUE to be fast enough to not allow overrun by the
    // OHA in for Deep Sleep Exit.  Set for 32 cycles (2 x 16) -> 0b010
    FAPI_INF("Setup OPCG_PAD_VALUE for Deep Sleep scanning ...")
    // - ld      D1, EX_OPCG_CNTL2_0x10030004, P1
    // - andi    D1, D1, ~(BITS(49,3))
    // - ori     D1, D1,  BIT(50)
    // - std     D1, EX_OPCG_CNTL2_0x10030004, P0
    FAPI_TRY(getScom(i_target, EX_OPCG_CNTL2_0x10030004, data));
    data.insertFromRight<49,3>(0x2);
    FAPI_TRY(putScom(i_target, EX_OPCG_CNTL2_0x10030004, data));

    ///////////////////
    // L2
    ///////////////////

    // set L2 inits to force single member mode if required
    FAPI_DBG("Configuring L2 single member mode ...");
    // - l2_single_member

    // set L2 inits to disable L3 if required
    FAPI_DBG("Configuring L3 disable ...");
    // - l3_setup L3_SETUP_ACTION_DISABLE, L3_SETUP_UNIT_L2

    ///////////////////
    // DTS
    ///////////////////

    // As this routine get runs for IPL, Winkle and Sleep, all Digital
    // Thermal Sensor setup is done here.
    // For the case of Sleep where the L3 DTS is still active, the
    // initialization is redone anyway as, while this operation is going on,
    // the atomic lock prevents other entities (eg OCC) from accessing it.
    // This keep the flows the same.

    // - setp1_mcreadand D0
    FAPI_INF("Initialize DTS function ...")

    // Enable DTS sampling - bit 5
    // Sample Pulse Count - bits(6:9)  set to a small number for sim
    // Enable loop 1 DTSs  (20:22); loop 2 DTSs (24)
    // = ld      D1, EX_THERM_MODE_REG_0x1005000F, P1
    // = ori     D1, D1, (BIT(5)|BITS(6, 4)|BITS(20,3)|BIT(24))
    // = std     D1, EX_THERM_MODE_REG_0x1005000F, P0
    FAPI_TRY(getScom(i_target, EX_THERM_MODE_REG_0x1005000F, data));
    data.insertFromRight<5,5>(0x1F).insertFromRight<20,3>(0xF).setBit<24>();
    FAPI_TRY(putScom(i_target, EX_THERM_MODE_REG_0x1005000F, data));

    return fapi2::FAPI2_RC_SUCCESS;

    FAPI_CLEANUP();
    return fapi2::FAPI2_RC_PLAT_ERR_SEE_DATA;

#endif

    return fapi2::FAPI2_RC_SUCCESS;

} // Procedure


} // extern C

