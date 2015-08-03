/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/hwp/cache/p9_hcd_cache_chiplet_reset.C $              */
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
/// @file  p9_hcd_cache_chiplet_reset.C
/// @brief Cache Chiplet Reset
///
/// *HWP HWP Owner   : David Du       <daviddu@us.ibm.com>
/// *HWP FW Owner    : Sangeetha T S  <sangeet2@in.ibm.com>
/// *HWP Team        : PM
/// *HWP Consumed by : SBE:SGPE
/// *HWP Level       : 1
///
/// Procedure Summary:
///   Reset quad chiplet logic
///   Scan0 flush entire cache chiplet
///

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <fapi2.H>
//#include <common_scom_addresses.H>
//will be replaced with real scom address header file
#include "p9_hcd_cache_chiplet_reset.H"

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
// Procedure: Cache Chiplet Reset
//------------------------------------------------------------------------------

extern "C"
{

fapi2::ReturnCode
p9_hcd_cache_chiplet_reset(
    const fapi2::Target<fapi2::TARGET_TYPE_EQ>& i_target)
{

#if 0
    uint32_t                loop;

    /////////////////////////////////////////////////////////////////
    // repeat some init steps of chiplet_init
    /////////////////////////////////////////////////////////////////

    // If there is a unused, powered-off EX chiplet which needs to be
    // configured in the following steps to setup the PCB endpoint.

    // Skip the PCB endpoint config steps for sleep so that fences don't
    // get dropped (eg by dropping chiplet_enable (GP3(0)).

    FAPI_INF("<p9_hcd_cache_chiplet_reset>: \
              Repeat dedicated pervasive init steps for EX ");

    FAPI_DBG("<p9_hcd_cache_chiplet_reset>: \
              Reset GP3 for EX chiplet, step needed for hotplug");
    // = sti    GENERIC_GP3_0x000F0012,P0,GP3_INIT_VECTOR
    FAPI_TRY(putScom(i_target, GENERIC_GP3_0x000F0012,
                     fapi2:buffer<uint64_t>(23452345)));

    FAPI_DBG("<p9_hcd_cache_chiplet_reset>: \
              Drop aync reset to Glitchless Mux");
    // = sti     GENERIC_GP3_AND_0x000F0013,P0,~BIT(2)
    FAPI_TRY(putScom(i_target, GENERIC_GP3_AND_0x000F0013,
                     fapi2:buffer<uint64_t>().flush<1>().clearBit<2>()));

    // 19:21 PM_PI_DECODE needs to be 010 for functional operation(set bit 20)
    FAPI_DBG("<p9_hcd_cache_chiplet_reset>: \
              Put DPLL in functional mode");
    // = sti     GENERIC_GP3_OR_0x000F0014,P0, BIT(20)
    FAPI_TRY(putScom(i_target, GENERIC_GP3_OR_0x000F0014,
                     fapi2::buffer<uint64_t>().setBit<20>()));

    // - updatestep STEP_CHIPLET_RESET_1, D0, P1

    FAPI_DBG("<p9_hcd_cache_chiplet_reset>: \
              Release endpoint reset for PCB");
    // = sti     GENERIC_GP3_AND_0x000F0013,P0,~BIT(1)
    FAPI_TRY(putScom(i_target, GENERIC_GP3_AND_0x000F0013,
                     fapi2:buffer<uint64_t>().flush<1>().clearBit<1>()));

    FAPI_DBG("<p9_hcd_cache_chiplet_reset>: \
              Partial good setting");
    // = sti     GENERIC_GP3_OR_0x000F0014,P0,BIT(0)
    FAPI_TRY(putScom(i_target, GENERIC_GP3_OR_0x000F0014,
                     fapi2::buffer<uint64_t>().setBit<0>()));

    FAPI_DBG("<p9_hcd_cache_chiplet_reset>: \
              PCB slave error reg reset");
    // = sti     MASTER_PCB_ERR_0x000F001F,P0,PCB_SL_ERROR_REG_RESET
    FAPI_TRY(putScom(i_target, MASTER_PCB_ERR_0x000F001F,
                     fapi2:buffer<uint64_t>(643564)));

    FAPI_DBG("Use timer mode for DPLL lock when enabled");
    // = sti     EX_PMGP0_OR_0x100F0102, P0, BIT(7)
    FAPI_TRY(putScom(i_target, EX_PMGP0_OR_0x100F0102,
                     fapi2::buffer<uint64_t>().setBit<7>()));

    // Set the DPLL Timer value for waiting on DPLL HOLDs in 35:36
    // 00 = 1024 cycles  <----
    // 01 = 512 cycles
    // 10 = 256 cycles
    // 11 = 128 cycles
    FAPI_DBG("Set the timer value for waiting on DPLL THOLDs");
    // = sti     EX_PMGP0_AND_0x100F0101,P0,~(BIT(35)|BIT(36))
    FAPI_TRY(putScom(i_target, EX_PMGP0_AND_0x100F0101,
             fapi2:buffer<uint64_t>().flush<1>().insertFromRight<35,2>(0)));

    // Only perform the disablement of PCBS-PM for the IPL work-around when
    // doing IPL (eg skip if winkle; this whole section is skipped for sleep)
    // >>> IPL
    // FAPI_DBG("Disable the PCBS-PM as part of winkle enablement");
    // = sti     EX_PMGP0_OR_0x100F0102, P0, BIT(0)
    FAPI_TRY(putScom(i_target, EX_PMGP0_OR_0x100F0102,
                     fapi2:buffer<uint64_t>().setBit<0>()));

    // The following is performed for both IPL/Winkle and Sleep

    // Note:  These are executed for sleep as well as these fences will have
    // already been dropped

    FAPI_DBG("<p9_hcd_cache_chiplet_reset>: \
              Remove pervasive ECO fence;");
    // ECO Fence in 22
    // = sti     EX_PMGP0_AND_0x100F0101,P0,~(BIT(22))
    FAPI_TRY(putScom(i_target, EX_PMGP0_AND_0x100F0101,
                     fapi2:buffer<uint64_t>().flush<1>().clearBit<22>()));

    FAPI_DBG("<p9_hcd_cache_chiplet_reset>: \
              Remove winkle mode before scan0 on EX chiplets is executed");
    // PM Exit States: WINKLE_EXIT_DROP_ELEC_FENCE

    FAPI_DBG("<p9_hcd_cache_chiplet_reset>: \
              Remove logical pervasive/pcbs-pm fence");
    // = sti     EX_PMGP0_AND_0x100F0101,P0,~(BIT(39))
    FAPI_TRY(putScom(i_target, EX_PMGP0_AND_0x100F0101,
                     fapi2:buffer<uint64_t>().flush<1>().clearBit<39>()));

    FAPI_DBG("<p9_hcd_cache_chiplet_reset>: \
              Remove PB Winkle Electrical Fence GP3(27)");
    // = sti     EX_GP3_AND_0x100F0013,P0,~(BIT(27))
    FAPI_TRY(putScom(i_target, EX_GP3_AND_0x100F0013,
                     fapi2:buffer<uint64_t>().flush<1>().clearBit<27>()));

    FAPI_DBG("<p9_hcd_cache_chiplet_reset>: \
              Configuring chiplet hang counters") ;
    // = sti EX_HANG_P1_0x100F0021,P0,HANG_P1_INIT
    data = 34654;  // HANG_P1_INIT
    FAPI_TRY(putScom(i_target, EX_HANG_P1_0x100F0021,
                     fapi2:buffer<uint64_t>(34654)));
    // - updatestep STEP_CHIPLET_RESET_2, D0, P1

    //////////////////////////////////////////////////////////////
    // perform scan0 module for pervasive chiplet (GPTR_TIME_REPR)
    //////////////////////////////////////////////////////////////

    // >>> IPL/Winkle scan flush - core and EX

       // Hook to bypass in sim while providing a trace
       // - hooki 0, 0xFF01
       // - ifslwcntlbitset P9_SLW_SKIP_FLUSH,1f

    // \todo WORKAROUND UNTIL LOGIC CHANGES
    // Drop the core2cache and cache2core fences to allow for L2 scanning
    FAPI_DBG("<p9_hcd_cache_chiplet_reset>: \
              Remove pervasive ECO fence;");
    // = sti     EX_PMGP0_AND_0x100F0101,P0,~(BIT(20) | BIT(21))
    FAPI_TRY(putScom(i_target, EX_PMGP0_AND_0x100F0101,
             fapi2:buffer<uint64_t>().flush<1>().insertFromRight<20,2>(0)));

    FAPI_DBG("EX Reset: Scan0 Module executed: \
              Scan the GPTR/TIME/REP rings");
    // - updatestep STEP_CHIPLET_RESET_3, D0, P1

    // = .rept P9_SCAN0_GPTR_REPEAT
    // = ex_scan0 SCAN_GPTR_TIME_REP, SCAN_CLK_ALL
    // = .endr
    for(loop=0;loop<P9_SCAN0_GPTR_REPEAT;loop++)
    {
        ex_scan0(SCAN_GPTR_TIME_REP, SCAN_CLK_ALL);
    }

    // - updatestep STEP_CHIPLET_RESET_4, D0, P1

       // - 1:
       // Hook to bypass in sim while providing a trace
       // - hooki 0, 0xFF02
       // - ifslwcntlbitset P9_SLW_SKIP_FLUSH,1f

    FAPI_DBG("EX Reset: Scan0 Module executed: \
              Scan the all but GPTR/TIME/REP rings");
    // - updatestep STEP_CHIPLET_RESET_5, D0, P1

    // Each scan0 will rotate the ring 8191 latches (2**13 - 1) and the
    // the longest ring is defined by P9_SCAN0_FUNC_REPEAT.  When the design
    // ALWAYS has all stumps less than 8191, the repeat (eg .rept) can be
    // removed.
    // Implementation note:  this is not done in a loop (or included in the
    // ex_scan0_module itself) as the D0 and D1 registers are used in
    // ex_scan0_module and there is no convenient place to temporarily store
    // the 2-64b values values.  Argueably, PIBMEM could be used for this
    // but was not utilized.
    // = .rept P9_SCAN0_FUNC_REPEAT
    // = ex_scan0 SCAN_ALL_BUT_GPTRTIMEREP, SCAN_CLK_ALL
    // = .endr
    for(loop=0;loop<P9_SCAN0_FUNC_REPEAT;loop++)
    {
        ex_scan0(SCAN_ALL_BUT_GPTRTIMEREP, SCAN_CLK_ALL);
    }

    // - updatestep STEP_CHIPLET_RESET_6, D0, P1

    // Ensure CC interrupts are disabled before starting clock domains and dpll
    // But only do it for Murano-DD1.x!
    // - lpcs    P1, STBY_CHIPLET_0x00000000
    // - ldandi  D0, PCBMS_DEVICE_ID_0x000F000F, P1, BITS(0, 32)
    // - andi    D0, D0, CFAM_CHIP_ID_CHIP_MAJOR_MASK
    // - cmpibrane   D0, 1f, CFAM_CHIP_ID_MURANO_1

    // - mr     A0, P0
    // - mr     D1, ETR
    // - ls     P0, CHIPLET0
    // - ls     CTR, MAX_CORES
    // - bra end_core_loop

    // - begin_core_loop:
    // Inspect presence of current ex chiplet
    // - andi    D0, D1, BIT(32)
    // - braz    D0, continue_core
    // Disable CC interrupts.
    // - ld      D0, EX_SYNC_CONFIG_0x10030000, P0
    // - ori     D0, D0, BIT(6)
    // - std     D0, EX_SYNC_CONFIG_0x10030000, P0

    // - continue_core:
    // - adds    P0, P0, 1  //Increment ex chiplet
    // - rols    D1, D1, 1  //Shift ex chiplet mask left

    // - end_core_loop:
    // - loop begin_core_loop
    // - mr      P0, A0
    // - 1:

    return fapi2::FAPI2_RC_SUCCESS;

    FAPI_CLEANUP();
    return fapi2::FAPI2_RC_PLAT_ERR_SEE_DATA;

#endif

    return fapi2::FAPI2_RC_SUCCESS;

} // Procedure


} // extern C

