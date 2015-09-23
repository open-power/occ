/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/hwp/cache/p9_hcd_cache_poweron.C $                    */
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
/// @file  p9_hcd_cache_poweron.C
/// @brief Cache Chiplet Power-on
///
// *HWP HWP Owner   : David Young       <davidy@us.ibm.com>
// *HWP FW Owner    : Sangeetha T S     <sangeet2@in.ibm.com>
// *HWP Team        : PM
// *HWP Consumed by : SBE:SGPE
// *HWP Level       : 2
//
// Procedure Summary:
//   Command the cache PFET controller to power-on
//   Check for valid power on completion
//   Polled Timeout:  100us
//

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <fapi2.H>
//#include <common_scom_addresses.H>
//will be replaced with real scom address header file
#include "p9_hcd_cache_poweron.H"

//------------------------------------------------------------------------------
// Constant Definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Procedure: Cache Chiplet Power-on
//------------------------------------------------------------------------------
#define FAPI_CLEANUP() fapi_try_exit:

fapi2::ReturnCode
p9_hcd_cache_poweron(
    const fapi2::Target<fapi2::TARGET_TYPE_EQ>& i_target)
{
    fapi2::ReturnCode l_rc = fapi2::FAPI2_RC_SUCCESS;
    FAPI_EXEC_HWP(l_rc, p9_common_poweronoff, i_target, p9power::POWER_ON);
    return l_rc;
} // Procedure
