/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/hwp/core/p9_hcd_core_initf.C $                        */
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
/// @file  p9_hcd_core_initf.C
/// @brief Core scan init
///
/// *HWP HWP Owner   : David Du      <daviddu@us.ibm.com>
/// *HWP FW Owner    : Reshmi Nair   <resnair5@in.ibm.com>
/// *HWP Team        : PM
/// *HWP Consumed by : SBE:CME
/// *HWP Level       : 1
///
/// Procedure Summary:
///   Initfiles in procedure defined on VBU ENGD wiki (TODO add link)
///   Check for the presence of core FUNC override rings from image;
///   if found, apply;  if not, apply core base FUNC rings from image
///   Note:  FASTINIT ring (eg CMSK ring) is setup at this point to limit the
///   stumps that participate in FUNC ring scanning (this is new for P9).
///   (TODO to make sure the image build support is in place)
///   Note : if in fused mode, both core rings will be initialized to the same
///   values via multicast scans
///

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fapi2.H>
//#include <common_scom_addresses.H>
//will be replaced with real scom address header file
#include "p9_hcd_core_initf.H"

//-----------------------------------------------------------------------------
// Constant Definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Procedure: Core scan init
//-----------------------------------------------------------------------------

extern "C"
{

fapi2::ReturnCode
p9_hcd_core_initf(
    const fapi2::Target<fapi2::TARGET_TYPE_CORE>& i_target)
{

#if 0

    fapi2::buffer<uint64_t> data;

    // - load_ring ex_func_core conditional_override=1
    // - load_ring ex_regf_core conditional_override=1
    // - load_ring ex_fary_l2 conditional_override=1
    // - load_ring ex_lbst_core conditional_override=1
    // - load_ring ex_abfa_core conditional_override=1
    // - load_ring ex_cmsk_core conditional_override=1

    return fapi2::FAPI2_RC_SUCCESS;

    FAPI_CLEANUP();
    return fapi2::FAPI2_RC_PLAT_ERR_SEE_DATA;

#endif

    return fapi2::FAPI2_RC_SUCCESS;

} // Procedure


} // extern C


