/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/hwp/core/p9_hcd_core_arrayinit.C $                    */
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
/// @file  p9_hcd_core_arrayinit.C
/// @brief Core Initialize arrays
///
/// *HWP HWP Owner   : David Du      <daviddu@us.ibm.com>
/// *HWP FW Owner    : Reshmi Nair   <resnair5@in.ibm.com>
/// *HWP Team        : PM
/// *HWP Consumed by : SBE:CME
/// *HWP Level       : 1
///
/// Procedure Summary:
///   Use ABIST engine to zero out all arrays
///   Upon completion, scan0 flush all rings except Vital,Repair,GPTR,and TIME
///

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <fapi2.H>
//#include <common_scom_addresses.H>
//will be replaced with real scom address header file
#include "p9_hcd_core_arrayinit.H"

//------------------------------------------------------------------------------
// Constant Definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Procedure: Core Initialize arrays
//------------------------------------------------------------------------------

extern "C"
{

fapi2::ReturnCode
p9_hcd_core_arrayinit(
    const fapi2::Target<fapi2::TARGET_TYPE_CORE>& i_target)

{

#if 0
    fapi2::buffer<uint64_t> data;

    // Procedure Prereq :    P0 is pointing to the targeted EX chiplet
    // submodules:
    // seeprom_array_init_module
    // ex_scan0

    FAPI_INF("<p9_hcd_core_arrayinit> : \
              *** Array Init and Scan0 Cleanup for EX Chiplets ***");

    // SBE Address Base Register Setups
    // Setup PRV_BASE_ADDR1; points to selected EX chiplet
    // - mr      P1, P0
    FAPI_INF("<p9_hcd_core_arrayinit> : \
              Copy selected EX info from P0 to P1");

    // Step 1: Array Init for selected EX chiplet
    // ARRAY INIT module -> see p9_sbe_tp_array_init.S
    //
    // At entry:
    //
    //    P1 : The chiplet ID/Multicast Group
    //    D1 : Clock Regions for Array Init
    //
    // At exit:
    //
    //    P0, D0, D1, CTR  : destroyed
    //    P1, A0, A1       : maintained
    //
    FAPI_INF("<p9_hcd_core_arrayinit> : \
              Calling Array Init Subroutine");

    // >>> Sleep
    // - li      D1, SCAN_CLK_CORE_ONLY

    // Execute the array init
    // - bsr     seeprom_array_init_module

    // Restore P0 with selected EX chiplet info
    // - mr      P0, P1
    FAPI_INF("<p9_hcd_core_arrayinit> : \
              Copy selected EX info back from P1 to P0");

    // Step 2: Scan0 for selected EX chiplet except PRV, GPTR, TIME and DPLL
    FAPI_INF("<p9_hcd_core_arrayinit> : \
              Calling Scan0 Subroutine");

    // taken from p9_sbe_ex_chiplet_init

    // >>> Sleep scan flush - core only

       // Hook to bypass in Sim
       // - hooki 0, 0xFF04
       // - ifslwcntlbitset P9_SLW_SKIP_FLUSH,2f

    FAPI_DBG("EX ArrayInit: Scan0 Module executed: \
              Scan all core chains except GPTR and TIME");

    // Each scan0 will rotate the ring 8191 latches (2**13 - 1) and the
    // the longest ring is defined by P9_SCAN0_FUNC_REPEAT.  When the design
    // ALWAYS has all stumps less than 8191, the repeat (eg .rept) can be
    // removed.
    // Implementation note:  this is not done in a loop (or included in the
    // ex_scan0_module itself) as the D0 and D1 registers are used in
    // ex_scan0_module and there is no convenient place to temporaily store
    // the 2-64b values values. Argueably, PIBMEM could be used for this
    // but was not utilized.

    // - .rept P9_SCAN0_FUNC_REPEAT
    // - ex_scan0 SCAN_CORE_ALL_BUT_GPTRTIMEREP, SCAN_CLK_CORE_ONLY
    // - .endr
    // - 2:

    FAPI_INF("<p9_hcd_core_arrayinit> : \
             *** End of Procedure ***");

    return fapi2::FAPI2_RC_SUCCESS;

clean_up:
    return fapi2::FAPI2_RC_PLAT_ERR_SEE_DATA;

#endif

    return fapi2::FAPI2_RC_SUCCESS;

} // Procedure


} // extern C


