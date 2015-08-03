/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/hwp/cache/p9_hcd_cache_chiplet_init.C $               */
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
/// @file  p9_hcd_cache_chiplet_init.C
/// @brief EX Flush/Initialize
///
/// *HWP HWP Owner   : David Du       <daviddu@us.ibm.com>
/// *HWP FW Owner    : Sangeetha T S  <sangeet2@in.ibm.com>
/// *HWP Team        : PM
/// *HWP Consumed by : SBE:SGPE
/// *HWP Level       : 1
///
/// Procedure Summary:
///   Scan0 flush all configured chiplet rings except Vital, GPTR, TIME and DPLL
///

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fapi2.H>
//#include <common_scom_addresses.H>
//will be replaced with real scom address header file
#include "p9_hcd_cache_chiplet_init.H"

//-----------------------------------------------------------------------------
// Constant Definitions
//-----------------------------------------------------------------------------
//#define SIM_PLL
//#define SIM_SPEEDUP
////#define SCAN0_DISABLE
//#define STEP_CHIPLET_INIT_0   0x0     // Resetting DPLL
//#define STEP_CHIPLET_INIT_1   0x1     // Core+ECO glmux switch (IPL/Winkle)
//#define STEP_CHIPLET_INIT_2   0x2     // Core glmux switch (Sleep)
//#define STEP_CHIPLET_INIT_3   0x3     // Before Func flush for IPL/Winkle
//#define STEP_CHIPLET_INIT_4   0x4     // After Func flush for IPL/Winkle
//#define STEP_CHIPLET_INIT_5   0x5     // After Func flush for IPL/Winkle
//#define STEP_CHIPLET_INIT_6   0x6     // Before Core GPTR flush for Sleep
//#define STEP_CHIPLET_INIT_7   0x7     // After Core GPTR flush for Sleep
//#define STEP_CHIPLET_INIT_8   0x8     // Before Core Func flush for Sleep
//#define STEP_CHIPLET_INIT_9   0x9     // After Core Func flush for Sleep
//#define STEP_CHIPLET_INIT_A   0xA     // Before Core Func flush for Sleep
//#define STEP_CHIPLET_INIT_B   0xB     // After Core Func flush for Sleep
//#define PORE_REFCLK_CYCLES    1       // \todo need real value for hdw
//#define DPLL_LOCK_DELAY       8192*PORE_REFCLK_CYCLES

//-----------------------------------------------------------------------------
// Procedure: EX Flush/Initialize
//-----------------------------------------------------------------------------

extern "C"
{

fapi2::ReturnCode
p9_hcd_cache_chiplet_init(
    const fapi2::Target<fapi2::TARGET_TYPE_EQ>& i_target)
{

#if 0
    fapi2::buffer<uint64_t> data;
    uint32_t                loop;

    // Procedure Prereq:
    // p9_sbe_ex_chiplet_reset, p9_sbe_ex_dpll_initf, p9_sbe_ex_pll_initf

    FAPI_INF("<p9_hcd_cache_chiplet_init>: Entering procedure");

    // Look for PSCOM error on any chip, fail if we find one
    // scan0 flush all configured chiplet rings except EX DPLL
    // call ex_scan0_module(  )

    // >>> IPL/Winkle scan flush - all except vital

       // Hook to bypass in Sim
       // - hooki 0, 0xFF02
       // - ifslwcntlbitset P9_SLW_SKIP_FLUSH,1f

    FAPI_DBG("EX Init: Scan0 Module executed: \
              Scan all except vital, DPL, GPTR, and TIME scan chains");
    // - updatestep STEP_CHIPLET_INIT_6, D0, P1

    // Each scan0 will rotate the ring 8191 latches (2**13 - 1) and the
    // the longest ring is defined by P9_SCAN0_FUNC_REPEAT.  When the design
    // ALWAYS has all stumps less than 8191, the repeat (eg .rept) can be
    // removed.
    // Implementation note:  this is not done in a loop (or included in the
    // ex_scan0_module itself) as the D0 and D1 registers are used in
    // ex_scan0_module and there is no convenient place to temporaily store
    // the 2-64b values values. Argueably, PIBMEM could be used for this
    // but was not utilized.
    // = .rept P9_SCAN0_FUNC_REPEAT
    // = ex_scan0 SCAN_ALL_BUT_VITALDPLLGPTRTIME, SCAN_CLK_ALLEXDPLL
    // = .endr
    for(loop=0;loop<P9_SCAN0_FUNC_REPEAT;loop++)
    {
        ex_scan0(SCAN_ALL_BUT_VITALDPLLGPTRTIME,SCAN_CLK_ALLEXDPLL);
    }
    // - updatestep STEP_CHIPLET_INIT_7, D0, P1
    // - 1:

    FAPI_INF("<p9_hcd_cache_chiplet_init>: Exiting procedure");

    return fapi2::FAPI2_RC_SUCCESS;

    FAPI_CLEANUP();
    return fapi2::FAPI2_RC_PLAT_ERR_SEE_DATA;

#endif

    return fapi2::FAPI2_RC_SUCCESS;

} // Procedure


} // extern C




