/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/hwp/nest/p9_sbe_scominit.C $                          */
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
/// @file  p9_sbe_scominit.C
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


//## auto_generated
#include "p9_sbe_scominit.H"



fapi2::ReturnCode p9_sbe_scominit(const
                  fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP> & i_target)
{
    FAPI_DBG("Entering ...");

    FAPI_DBG("Exiting ...");

    return fapi2::FAPI2_RC_SUCCESS;

}
