/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/hwp/cache/p9_hcd_cache_arrayinit.H $                  */
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
/// @file  p9_hcd_cache_arrayinit.H
/// @brief EX Initialize arrays
///
/// *HWP HWP Owner   : David Du       <daviddu@us.ibm.com>
/// *HWP FW Owner    : Sangeetha T S  <sangeet2@in.ibm.com>
/// *HWP Team        : PM
/// *HWP Consumed by : SBE:SGPE
/// *HWP Level       : 1
///

#ifndef __P9_HCD_CACHE_ARRAYINIT_H__
#define __P9_HCD_CACHE_ARRAYINIT_H__

extern "C"
{

/// @typedef p9_hcd_cache_arrayinit_FP_t
/// function pointer typedef definition for HWP call support
typedef fapi2::ReturnCode (*p9_hcd_cache_arrayinit_FP_t) (
    const fapi2::Target<fapi2::TARGET_TYPE_EX>&);

/// @brief EX Initialize arrays
///
/// @param [in] i_target TARGET_TYPE_EX target
///
/// @attr
/// @attritem NONE
///
/// @retval FAPI_RC_SUCCESS
fapi2::ReturnCode
p9_hcd_cache_arrayinit(
    const fapi2::Target<fapi2::TARGET_TYPE_EX>& i_target);


} // extern C

#endif  // __P9_HCD_CACHE_ARRAYINIT_H__
