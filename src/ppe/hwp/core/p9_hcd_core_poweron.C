/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/hwp/core/p9_hcd_core_poweron.C $                      */
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
/// @file  p9_hcd_core_poweron.C
/// @brief Core Chiplet Power-on
///
// *HWP HWP Owner   : David Young          <davidy@us.ibm.com>
// *HWP FW Owner    : Sangeetha T S     <sangeet2@in.ibm.com>
// *HWP Team        : PM
// *HWP Consumed by : SBE:CME
// *HWP Level       : 2
//
// Procedure Summary:
//   1.Command the core PFET controller to power-on, via putscom to CPPM
//   2.Check for valid power on completion, via getscom from CPPM
//     Polled Timeout:  100us
//

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fapi2.H>
#if 0
#include <common_scom_addresses.H>
//will be replaced with real scom address header file
#endif
#include "p9_hcd_core_poweron.H"

//-----------------------------------------------------------------------------
// Constant Definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Procedure: Core Chiplet Power-on
//-----------------------------------------------------------------------------

#define FAPI_CLEANUP() fapi_try_exit:
#define FAPI_GOTO_EXIT() goto fapi_try_exit;

fapi2::ReturnCode
p9_hcd_core_poweron(
    const fapi2::Target<fapi2::TARGET_TYPE_CORE>& i_target)
{
    fapi2::ReturnCode l_rc = fapi2::FAPI2_RC_SUCCESS;
    FAPI_EXEC_HWP(l_rc, p9_common_poweronoff, i_target, p9power::POWER_ON_VDD);
    return l_rc;
} // Procedure
