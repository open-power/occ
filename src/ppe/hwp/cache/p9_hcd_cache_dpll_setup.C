/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/hwp/cache/p9_hcd_cache_dpll_setup.C $                 */
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
/// @file  p9_hcd_cache_dpll_setup.C
/// @brief Quad DPLL Setup
///
/// *HWP HWP Owner   : David Du       <daviddu@us.ibm.com>
/// *HWP FW Owner    : Sangeetha T S  <sangeet2@in.ibm.com>
/// *HWP Team        : PM
/// *HWP Consumed by : SBE:SGPE
/// *HWP Level       : 1
///
/// Procedure Summary:
///   Initfiles in procedure defined on VBU ENGD wiki  (TODO add link)
///   DPLL tune bits are not dependent on frequency
///   Put DPLL into bypass
///   Set DPLL syncmux sel
///   Set clock controller scan ratio to 1:1 as this is done at refclk speeds
///   Load the EX DPLL scan ring
///   Set clock controller scan ratio to 8:1 for future scans
///   Frequency is controlled by the Quad PPM
///   Actual frequency value for boot is stored into the Quad PPM by
///     p9_hcd_setup_evid.C in istep 2
///   In real cache STOP exit, the frequency value is persistent
///   Enable the DPLL in the correct mode
///   non-dynamic
///   Slew rate established per DPLL team
///   Take the cache glitchless mux out of reset
///     (TODO:  is there still a reset on P9?)
///   Remove DPLL bypass
///   Drop DPLL Tholds
///   Check for DPLL lock
///   Timeout:  200us
///   Switch cache glitchless mux to use the DPLL
///

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fapi2.H>
//#include <common_scom_addresses.H>
//will be replaced with real scom address header file
#include "p9_hcd_cache_dpll_setup.H"

//-----------------------------------------------------------------------------
// Constant Definitions
//-----------------------------------------------------------------------------

//#define	DPLL_LOCK_BIT,                 50
//#define	DPLL_LOCK_MAX_POLL_LOOPS,      0x1600
//#define	DPLL_LOCK_DELAY_CYCLES,        0x10
//#define	STEP_SBE_DPLL_SETUP_LOCK,      1
//#define	STEP_SBE_DPLL_GMUX_SET,        2
//#define	STEP_SBE_DPLL_SETUP_COMPLETE,  3

// For a 2000GHz nest (500MHz Pervasive), 2ns clocks exists
// For a DPLL lock time of >150us from power on, 150000/2 = 75000
// d75000 => 0x124F8   ----> round up to 0x15000

// For a 2400GHz nest (600MHz Pervasive), 1.667ns clocks exists
// For a DPLL lock time of >150us from power on, 150000/1.667 = 89982
// d90000 => 0x15f90   ----> round up to 0x16000

//#define DPLL_LOCK_DELAY 0x10
//#define DPLL_LOCK_LOOP_NUMBER 0x1600

//#define ex_dpll_lock_delay_mult  0x1600
//#define ex_glsmux_post_delay_mult  0x1

//-----------------------------------------------------------------------------
// Procedure: Quad DPLL Setup
//-----------------------------------------------------------------------------

extern "C"
{

fapi2::ReturnCode
p9_hcd_cache_dpll_setup(
    const fapi2::Target<fapi2::TARGET_TYPE_EQ>& i_target)
{

#if 0
    fapi2::buffer<uint64_t> data;

    FAPI_INF("EX Chiplet -> Start DPLL setup" )

    // The setup of P0 happens external to this procedure
    // P0 is set to point to either a single EX core, or to multiple EX
    // cores using multicast
    FAPI_DBG("EX Chiplet dpll_setup P0 -> 0x%02llx" , io_pore.p0.read())

    // Skip DPLL and Glitchless mux setup for Sleep
    // Glitchless mux change for the core (Sleep) done in
    // p9_sbe_ex_chiplet_reset.

    // - ifsleep D0, glm_end
    // >>> IPL/WINKLE

    // - dpll_start:

    // - setp1_mcreadand D0

    FAPI_INF("Set up CPM PARM register for DPLL")
    // = ld      D0, EX_DPLL_CPM_PARM_REG_0x100F0152, P1
    FAPI_TRY(getScom(i_target, EX_DPLL_CPM_PARM_REG_0x100F0152, data));

    // ----- Make sure that ff_bypass is set to "1" -----
    // This is necessary to ensure that the DPLL is in Mode 1.  If not,
    // the lock times will go from ~30us to 3-5ms
    // Set bit 11 of DPLL Mode register  0x150F0152

    FAPI_DBG("Put DPLL into Mode 1 by asserting ff_bypass.")
    FAPI_DBG(">>> Work around  <<<<: Also disabling ping-pong synchronizer" )
    // = ori     D0, D0, (BIT(11)|BIT(32))
    data.setBit<11>().setBit<32>();

    FAPI_DBG("Clear bits 15,16,18-23 and set bit 17 of DPLL reg 0x10F0152")
    // = andi    D0, D0, 0xFFFE00FFFFFFFFFF
    // = ori     D0, D0, BIT(17)
    data.insertFromRight<15,9>(0x40);

    FAPI_DBG("Set slew rate to a modest value")
    // = ori     D0, D0, 0x8000000000000000
    // = std     D0, EX_DPLL_CPM_PARM_REG_0x100F0152, P0
    data.setBit<0>();
    FAPI_TRY(putScom(i_target, EX_DPLL_CPM_PARM_REG_0x100F0152, data));

    // ----- Clear dpllclk_muxsel (syncclk_muxsel) to "0" -----
    FAPI_INF("Reset syncclk_muxsel or dpllclk_muxsel")
    // = sti     GENERIC_GP0_AND_0x00000004, P0, ~BIT(1)
    FAPI_TRY(putScom(i_target, GENERIC_GP0_AND_0x00000004,
                     fapi2:buffer<uint64_t>().flush<1>().clearBit<1>()));

    // = sti     EX_PMGP0_AND_0x100F0101, P0, ~(BIT(11))
    FAPI_TRY(putScom(i_target, EX_PMGP0_AND_0x100F0101,
                     fapi2:buffer<uint64_t>().flush<1>().clearBit<11>()));

    // ----- Take DPLL out of bypass -----
    // Clear bit 5 of EX GP3 Register
    FAPI_INF("EX Chiplet -> Take DPLL out of bypass" )
    // = sti     EX_GP3_AND_0x100F0013, P0, ~(BIT(5))
    FAPI_TRY(putScom(i_target, EX_GP3_AND_0x100F0013,
                     fapi2:buffer<uint64_t>().flush<1>().clearBit<5>()));

    // ----- Drop DPLL tholds  -----
    // Clear bit 3 of EX PMGP0 Register
    FAPI_INF("EX Chiplet -> Drop internal DPLL THOLD" )
    // = sti     EX_PMGP0_AND_0x100F0101, P0, ~(BIT(3))
    FAPI_TRY(putScom(i_target, EX_PMGP0_AND_0x100F0101,
                     fapi2:buffer<uint64_t>().flush<1>().clearBit<3>()));

    // ----- Delay to wait for DPLL to lock  -----
    // TODO:  Determine whether or not we should POLL instead of put delay here.
    // Wait for >150us

    FAPI_INF("Wait for DPLL to lock" )
    // - waits       DPLL_LOCK_LOOP_NUMBER*DPLL_LOCK_DELAY

    // Check for lock
    // - ldandi  D0, EX_PMGP0_0x100F0100, P1, BIT(DPLL_LOCK_BIT)
    // - braz    D0, dpll_nolock

    // ----- Recycle DPLL in and out of bypass -----
    // Clear bit 5 of EX GP3 Register
    FAPI_INF("EX Chiplet -> Recycle DPLL in and out of bypass" )
    // = sti     EX_GP3_OR_0x100F0014, P0, BIT(5)
    FAPI_TRY(putScom(i_target, EX_GP3_OR_0x100F0014,
                     fapi2:buffer<uint64_t>().setBit<5>()));

    // = sti     EX_GP3_AND_0x100F0013, P0, ~(BIT(5))
    FAPI_TRY(putScom(i_target, EX_GP3_AND_0x100F0013,
                     fapi2:buffer<uint64_t>().flush<1>().clearBit<5>()));

    FAPI_INF("EX Chiplet -> EX DPLL is locked" )
    // - updatestep STEP_SBE_DPLL_SETUP_LOCK, D0, P1

    // - glm_start:

    // ----- Set Glitch-Less MUXes for IPL/Winkle case ----
    // For Sleep, the core glitchless mux is change earlier as the DPLL
    // was already up and locked.
    //
    // Set various gl muxes to pass the output of the DPLLs to the clock grid.

    // Read-modify-write (vs sti AND and sti OR) is done so that glitchless
    // mux change field change is atomic (eg 1 store)
    // - setp1_mcreadand D1

    // = ld      D1,EX_PMGP0_0x100F0100,P1
    FAPI_TRY(getScom(i_target, EX_PMGP0_0x100F0100, data));

    // IPL/Winkle - Switch glitchless mux primary source to 001 = DPLL for bits
    // 27:29

    // Note: GLM async reset occured in p9_sbe_ex_chiplet_reset.

    // Set tp_clkglm_sel_dc to "001"  (EX PM GP0 bits 27-29)
    FAPI_INF("EX Chiplet -> Set glitchless mux select for primary chiplet clock source to 001 ")
    // Set tp_clkglm_eco_sel_dc to "0"  (EX PM GP0 bit 30)
    // Set the core glitchless mux to use the primary input (b00).
    // EX PM GP0 bits 32-33)
    FAPI_INF("EX Chiplet -> Set glitchless mux select for core and eco domain to 0 ")
    // = andi    D1, D1, ~(BIT(27)|BIT(28)|BIT(30)|BIT(32)|BIT(33))
    // = ori     D1, D1, (BIT(29))
    data.insertFromRight<27,4>(0x2).inerstFromRight<32,2>(0x0);

    // Store the final result to the hardware
    // = std     D1,EX_PMGP0_0x100F0100,P0
    FAPI_TRY(putScom(i_target, EX_PMGP0_0x100F0100, data));

    // - glm_end:

    // - updatestep STEP_SBE_DPLL_GMUX_SET, D0, P1

    // ----- Drop ff_bypass to enable slewing -----
    // ----- (Change from Mode 1 to mode 2)   -----
    // Clear bit 11 of DPLL Mode register  0x150F0152
    // CMO20131125-Further, drop the pp synchronizer bit32.
    // CMO20131219-Keep the pp sync bit32 asserted to avoid x-leakage(HW276931).
    FAPI_INF("EX Chiplet -> Clear ff_bypass to switch into slew-controlled mode")
    // - setp1_mcreadand D1
    // = ld      D1, EX_DPLL_CPM_PARM_REG_0x100F0152, P1
    // = andi    D1, D1, ~(BIT(11))
    // = std     D1, EX_DPLL_CPM_PARM_REG_0x100F0152, P0
    FAPI_TRY(getScom(i_target, EX_DPLL_CPM_PARM_REG_0x100F0152, data));
    data.clearBit<11>();
    FAPI_TRY(putScom(i_target, EX_DPLL_CPM_PARM_REG_0x100F0152, data));

    // ----- Drop other tholds -----
    // Clear bit 3 of EX PM GP0 Register
    //    FAPI_INF("EX Chiplet -> Drop DPLL thold" )
    //    sti     EX_PMGP0_AND_0x100F0101,P0, ~(BIT(3))

    // - updatestep STEP_SBE_DPLL_SETUP_COMPLETE, D0, P1
    FAPI_INF("EX Chiplet -> DPLL setup completed" )

    // DEBUG only
    // - setp1_mcreadand D1
    // = ld      D0,EX_GP0_0x10000000,P1
    FAPI_TRY(getScom(i_target, EX_GP0_0x10000000, data));

    // - dpll_nolock:
    FAPI_ERR("EX_DPLL -> Failed to lock");
    // - reqhalt RC_SBE_DPLL_SETUP_NOLOCK

    return fapi2::FAPI2_RC_SUCCESS;

    FAPI_CLEANUP();
    return fapi2::FAPI2_RC_PLAT_ERR_SEE_DATA;

#endif

    return fapi2::FAPI2_RC_SUCCESS;

} // Procedure


} // extern C


