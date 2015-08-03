/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/hwp/cache/p9_hcd_cache_gptr_time_initf.C $            */
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
/// @file  p9_hcd_cache_gptr_time_initf.C
/// @brief Load GPTR and Time for EX non-core
///
/// *HWP HWP Owner   : David Du       <daviddu@us.ibm.com>
/// *HWP FW Owner    : Sangeetha T S  <sangeet2@in.ibm.com>
/// *HWP Team        : PM
/// *HWP Consumed by : SBE:SGPE
/// *HWP Level       : 1
///
/// Procedure Summary:
///   Initfiles in procedure defined on VBU ENGD wiki (TODO add link)
///     to produce #G VPD contents
///   Check for the presence of core override GPTR ring from image
///     (this is new fvor P9)
///   if found, apply;  if not, apply core GPTR from image
///     Check for the presence of core override TIME ring from image;
///   if found, apply;  if not, apply core base TIME from image
///

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <fapi2.H>
//#include <common_scom_addresses.H>
//will be replaced with real scom address header file
#include "p9_hcd_cache_gptr_time_initf.H"

//------------------------------------------------------------------------------
// Constant Definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Procedure: Load GPTR and Time for EX non-core
//------------------------------------------------------------------------------

extern "C"
{

fapi2::ReturnCode
p9_hcd_cache_gptr_time_initf(
    const fapi2::Target<fapi2::TARGET_TYPE_EQ>& i_target)
{

#if 0

    // Set EX scan ratio to 1:1 as EX is still at refclock
    FAPI_INF("<p9_sbe_ex_gptr_time_initf> : Set EX scan ratio to 1:1 ...");
    // = sti     EX_OPCG_CNTL0_0x10030002, P0, 0x0
    FAPI_TRY(fapi2::putScom(i_target, EX_OPCG_CNTL0_0x10030002, 0x0));

    // scan ring content shared among all chiplets
    FAPI_DBG("Scanning EX GPTR rings...")
    // - load_ring ex_gptr_perv skipoverride=1
    // - load_ring ex_gptr_dpll skipoverride=1
    // - load_ring ex_gptr_l3 skipoverride=1
    // - load_ring ex_gptr_l3refr skipoverride=1

    // scan chiplet specific ring content
    FAPI_DBG("Scanning EX TIME rings...")
    // - load_ring_vec_ex ex_time_eco


    // Set EX scan ratio back to 8:1
    FAPI_INF("<p9_sbe_ex_gptr_time_initf> : Set EX scan ratio to 8:1 ...");
    // Inputs: A1 and P0 and D0, destroys D0 & D1
    // -   .pibmem_port (PORE_SPACE_PIBMEM & 0xf)
    // -    lpcs    P1, PIBMEM0_0x00080000
    // -    ld      D0, ex_scan_ratio_override, P1
    // -    bsr     set_scan_ratio_d0

    return fapi2::FAPI2_RC_SUCCESS;

    FAPI_CLEANUP();
    return fapi2::FAPI2_RC_PLAT_ERR_SEE_DATA;

#endif

    return fapi2::FAPI2_RC_SUCCESS;

} // Procedure


} // extern C


