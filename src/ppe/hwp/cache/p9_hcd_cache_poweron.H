/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/hwp/cache/p9_hcd_cache_poweron.H $                    */
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
/// @file  p9_hcd_cache_poweron.H
/// @brief Cache Chiplet Power-on

//
// *HWP HWP Owner   : David Young       <davidy@us.ibm.com>
// *HWP FW Owner    : Sangeetha T S  <sangeet2@in.ibm.com>
// *HWP Team        : PM
// *HWP Level       : 2
// *HWP Consumed by : SBE:SGPE
//

#ifndef __P9_HCD_CACHE_POWERON_H__
#define __P9_HCD_CACHE_POWERON_H__

#include "p9_common_poweronoff.H"

/// @typedef p9_hcd_cache_poweron_FP_t
/// function pointer typedef definition for HWP call support
typedef fapi2::ReturnCode (*p9_hcd_cache_poweron_FP_t) (
    const fapi2::Target<fapi2::TARGET_TYPE_EQ>&);

extern "C"
{
/// @brief Cache Chiplet Power-on
/// @param [in] i_target    TARGET_TYPE_EQ target
/// @param [in] i_operation ENUM(ON,OFF)
///
/// @attr
/// @attritem ATTR_PFET_*
///
/// @retval FAPI2_RC_SUCCESS if success, else error code

fapi2::ReturnCode
p9_hcd_cache_poweron(
    const fapi2::Target<fapi2::TARGET_TYPE_EQ>& i_target);
}
#endif  // __P9_HCD_CACHE_POWERON_H__
