/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/hwp/perv/p9_sbe_startclock_chiplets.H $               */
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
//------------------------------------------------------------------------------
/// @file  p9_sbe_startclock_chiplets.H
///
/// @brief Start clock procedure for XBUS, OBUS, PCIe
//------------------------------------------------------------------------------
// *HWP HWP Owner        : Abhishek Agarwal <abagarw8@in.ibm.com>
// *HWP HWP Backup Owner : Srinivas V Naga <srinivan@in.ibm.com>
// *HWP FW Owner         : sunil kumar <skumar8j@in.ibm.com>
// *HWP Team             : Perv
// *HWP Level            : 1
// *HWP Consumed by      : SBE
//------------------------------------------------------------------------------


#ifndef _P9_SBE_STARTCLOCK_CHIPLETS_H_
#define _P9_SBE_STARTCLOCK_CHIPLETS_H_


#include <fapi2.H>


typedef fapi2::ReturnCode (*p9_sbe_startclock_chiplets_FP_t)(
    const fapi2::Target<fapi2::TARGET_TYPE_PERV> &);

/// @brief Start Xbus, Obus, PCIe clocks
/// Start clocks on configured chiplets for all chips (master and slaves)
///
/// @param[in]     i_target_chiplets   Reference to TARGET_TYPE_PERV target
/// @return  FAPI2_RC_SUCCESS if success, else error code.
extern "C"
{
    fapi2::ReturnCode p9_sbe_startclock_chiplets(const
            fapi2::Target<fapi2::TARGET_TYPE_PERV> & i_target_chiplets);
}

#endif
