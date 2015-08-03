/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/hwp/cache/p9_hcd_cache_occ_runtime_scom.C $           */
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
/// @file  p9_hcd_cache_occ_runtime_scom.C
/// @brief EX OCC runtime scoms
///
/// *HWP HWP Owner   : David Du       <daviddu@us.ibm.com>
/// *HWP FW Owner    : Sangeetha T S  <sangeet2@in.ibm.com>
/// *HWP Team        : PM
/// *HWP Consumed by : SBE:SGPE
/// *HWP Level       : 1
///
/// Procedure Summary:
///   Run-time updates from OCC code that are put  somewhere TBD
///     (TODO . revisit with OCC FW team)
///   OCC FW sets up value in the TBD SCOM section
///   This was not leverage in P8 with the demise of CPMs
///   Placeholder at this point
///

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <fapi2.H>
//#include <common_scom_addresses.H>
//will be replaced with real scom address header file
#include "p9_hcd_cache_occ_runtime_scom.H"

//------------------------------------------------------------------------------
// Constant Definitions
//------------------------------------------------------------------------------
#define host_runtime_scom 0

//------------------------------------------------------------------------------
// Procedure: EX OCC runtime SCOMS
//------------------------------------------------------------------------------

extern "C"
{

fapi2::ReturnCode
p9_hcd_cache_occ_runtime_scom(
    const fapi2::Target<fapi2::TARGET_TYPE_EX>& i_target)
{

#if 0
    fapi2::buffer<uint64_t> data;

    // Run the SCOM sequence if the SCOM procedure is defined
    // -   la      A0, occ_runtime_scom
    // -   ld      D0, 0, A0
    // -   braz    D0, 1f
    FAPI_INF("Launching OCC Runtime SCOM routine")
    // -   bsrd    D0
    // -   1:

    return fapi2::FAPI2_RC_SUCCESS;

    FAPI_CLEANUP();
    return fapi2::FAPI2_RC_PLAT_ERR_SEE_DATA;

#endif

    return fapi2::FAPI2_RC_SUCCESS;

} // Procedure


} // extern C


