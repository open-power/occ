/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/hwp/core/p9_hcd_core_chiplet_reset.C $                */
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
/// @file  p9_hcd_core_chiplet_reset.C
/// @brief Core Chiplet Reset
///
/// *HWP HWP Owner   : David Du      <daviddu@us.ibm.com>
/// *HWP FW Owner    : Reshmi Nair   <resnair5@in.ibm.com>
/// *HWP Team        : PM
/// *HWP Consumed by : SBE:CME
/// *HWP Level       : 1
///
/// Procedure Summary:
///   Reset chiplet logic
///     (TODO: check with Andreas on the effect of a CME based Endpoint reset
///            relative to the CorePPM path)
///   Scan0 flush entire core chiplet
///

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <fapi2.H>
//#include <common_scom_addresses.H>
//will be replaced with real scom address header file
#include "p9_hcd_core_chiplet_reset.H"

//------------------------------------------------------------------------------
// Constant Definitions
//------------------------------------------------------------------------------

// GP3 Bits
// 1     -  PCB_EP_RESET
// 2     -  GLMMUX Reset
// 3     -  PLL_TEST Enable
// 4     -  PLLRST - PLL Reset
// 5     -  PLL Bypass
// 11    -  D_MODE for Vital
// 13    -  MPW2 for Vital
// 14    -  PMW1 for Vital
// 18    -  FENCE_EN for chiplet
// 22    -  Resonant Clock disable
// 23:24 -  Glitchless Mux Sel
// 25:   -  ?? (set because System Pervasive flow does this)
// Background: system pervasive as the following setting in their tests:
//      7C1623C000000000
// Bits set:
//      1, 2, 3, 4, 5, 11, 13, 14, 18, 22, 23, 24, 25
//#define GP3_INIT_VECTOR (BITS(1,5)|BIT(11)|BIT(13)|BIT(14)|BIT(18)|BIT(22)|BIT(23)|BIT(24)|BIT(25))

// hang counter inits
//#define HANG_P1_INIT            0x0400000000000000
//#define PCB_SL_ERROR_REG_RESET  0xFFFFFFFFFFFFFFFF
//#define STEP_CHIPLET_RESET_1    0x1    // After start of vital clocks
//#define STEP_CHIPLET_RESET_2    0x2    // After fence drop
//#define STEP_CHIPLET_RESET_3    0x3    // Before GPTR flush for IPL/Winkle
//#define STEP_CHIPLET_RESET_4    0x4    // After GPTR flush for IPL/Winkle
//#define STEP_CHIPLET_RESET_5    0x5    // Before Func flush for IPL/Winkle
//#define STEP_CHIPLET_RESET_6    0x6    // After Func flush for IPL/Winkle
//#define STEP_CHIPLET_RESET_7    0x7    // Before GPTR flush for Sleep
//#define STEP_CHIPLET_RESET_8    0x8    // After GPTR flush for Sleep

//------------------------------------------------------------------------------
// Procedure: Core Chiplet Reset
//------------------------------------------------------------------------------

extern "C"
{

fapi2::ReturnCode
p9_hcd_core_chiplet_reset(
    const fapi2::Target<fapi2::TARGET_TYPE_CORE>& i_target)
{

#if 0
    fapi2::buffer<uint64_t> data;

    /////////////////////////////////////////////////////////////////
    // repeat some init steps of chiplet_init
    /////////////////////////////////////////////////////////////////

    // If there is a unused, powered-off EX chiplet which needs to be
    // configured in the following steps to setup the PCB endpoint.

    // Skip the PCB endpoint config steps for sleep so that fences don't
    // get dropped (eg by dropping chiplet_enable (GP3(0)).

    // The following is performed for both IPL/Winkle and Sleep

    // Note:  These are executed for sleep as well as these fences will have
    // already been dropped

    FAPI_DBG("<p9_hcd_core_chiplet_reset>: \
              Remove pervasive ECO fence;");
    // ECO Fence in 22
    // = sti     EX_PMGP0_AND_0x100F0101,P0,~(BIT(22))
    FAPI_TRY(putScom(i_target, EX_PMGP0_AND_0x100F0101,
                     fapi2:buffer<uint64_t>().flush<1>().clearBit<22>()));

    FAPI_DBG("<p9_hcd_core_chiplet_reset>: \
              Remove winkle mode before scan0 on EX chiplets is executed");
    // PM Exit States: WINKLE_EXIT_DROP_ELEC_FENCE

    FAPI_DBG("<p9_hcd_core_chiplet_reset>: \
              Remove logical pervasive/pcbs-pm fence");
    // = sti     EX_PMGP0_AND_0x100F0101,P0,~(BIT(39))
    FAPI_TRY(putScom(i_target, EX_PMGP0_AND_0x100F0101,
                     fapi2:buffer<uint64_t>().flush<1>().clearBit<39>()));

    FAPI_DBG("<p9_hcd_core_chiplet_reset>: \
              Remove PB Winkle Electrical Fence GP3(27)");
    // = sti     EX_GP3_AND_0x100F0013,P0,~(BIT(27))
    FAPI_TRY(putScom(i_target, EX_GP3_AND_0x100F0013,
                     fapi2:buffer<uint64_t>().flush<1>().clearBit<27>()));

    FAPI_DBG("<p9_hcd_core_chiplet_reset>: \
              Configuring chiplet hang counters") ;
    // = sti EX_HANG_P1_0x100F0021,P0,HANG_P1_INIT
    FAPI_TRY(putScom(i_target, EX_HANG_P1_0x100F0021, HANG_P1_INIT));
    // - updatestep STEP_CHIPLET_RESET_2, D0, P1

    //////////////////////////////////////////////////////////////
    // perform scan0 module for pervasive chiplet (GPTR_TIME_REPR)
    //////////////////////////////////////////////////////////////

    // For the Sleep case, the DPLL is running but the core mesh is force to
    // the "constant" or "off" state. In order to flush or scan, the mesh must
    // be reenabled via the Glitchless Mux.

    // Read-modify-write (vs sti AND and sti OR) is done so that glitchless
    // mux change field change is atomic (eg 1 store)
    // - setp1_mcreadand D1
    // = ld      D1,EX_PMGP0_0x100F0100,P1
    FAPI_TRY(fapi2::getScom(i_target, EX_PMGP0_0x100F0100, data));

    // Set the core glitchless mux to use the primary input (b00).
    // Upon Sleep entry, hardware will switch the glitchless mux to 0b10
    // (constant). EX PM GP0 bits 32-33)
    FAPI_INF("EX Chiplet -> Set glitchless mux select for core domain to 00 ")
    // = andi    D1, D1, ~(BIT(32)|BIT(33))

    // Store the final result to the hardware
    // = std     D1,EX_PMGP0_0x100F0100,P0
    data.insertFromRight<32,2>(0x0);
    FAPI_TRY(putScom(i_target,  EX_PMGP0_0x100F0100, data));
    // - updatestep STEP_CHIPLET_RESET_7, D0, P1

       // Hook to bypass in sim while providing a trace
       // - hooki 0, 0xFF03
       // - ifslwcntlbitset P9_SLW_SKIP_FLUSH,1f

     FAPI_DBG("EX Reset: Scan0 Module executed for Sleep: \
               Scan the GPTR/TIME/REP rings");

     // - .rept P9_SCAN0_GPTR_REPEAT
     // - ex_scan0 SCAN_CORE_GPTR_TIME_REP, SCAN_CLK_CORE_ONLY
     // - .endr
     // - 1:
     // - updatestep STEP_CHIPLET_RESET_8, D0, P1

    return fapi2::FAPI2_RC_SUCCESS;

    FAPI_CLEANUP();
    return fapi2::FAPI2_RC_PLAT_ERR_SEE_DATA;

#endif

    return fapi2::FAPI2_RC_SUCCESS;

} // Procedure


} // extern C

