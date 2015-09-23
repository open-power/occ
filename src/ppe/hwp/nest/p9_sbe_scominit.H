/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/hwp/nest/p9_sbe_scominit.H $                          */
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
/// @file  p9_sbe_scominit.H
///
/// @brief This procedure contains SCOM based initialization required for
///  fabric configuration & HBI operation
/// *!
/// *!   o Set fabric node/chip ID configuration for all configured
/// *!     chiplets to chip specific values
/// *!   o Establish ADU XSCOM BAR for HBI operation
//------------------------------------------------------------------------------
// *HWP HW Owner         : Girisankar Paulraj <gpaulraj@in.ibm.com>
// *HWP HW Backup Owner  : Joe McGill <jmcgill@us.ibm.com>
// *HWP FW Owner         : Thi N. Tran <thi@us.ibm.com>
// *HWP Team             : Nest
// *HWP Level            : 1
// *HWP Consumed by      : SBE
//------------------------------------------------------------------------------


#ifndef _P9_SBE_SCOMINIT_H_
#define _P9_SBE_SCOMINIT_H_


#include <fapi2.H>


typedef fapi2::ReturnCode (*p9_sbe_scominit_FP_t)(const
        fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP> &);

/// @brief This function does ADU BAR and XSCOM BAR base setup,
/// Set Fabric node/chip ID for  all configured chiplets in nest
/// area and Clearing/setting up FIR register
///
/// @param[in]     i_target   Reference to TARGET_TYPE_PROC_CHIP target
/// @return  FAPI2_RC_SUCCESS if success, else error code.
extern "C"
{
    fapi2::ReturnCode p9_sbe_scominit(const
                      fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP> & i_target);
}

#endif
