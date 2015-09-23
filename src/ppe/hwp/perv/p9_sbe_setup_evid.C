/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/hwp/perv/p9_sbe_setup_evid.C $                        */
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
/// @file  p9_sbe_setup_evid.C
/// @brief Setup External Voltage IDs and Boot Frequency
///
// *HW Owner    : Greg Still <stillgs@us.ibm.com>
// *FW Owner    : Sangeetha T S <sangeet2@in.ibm.com>
// *Team        : PM
// *Consumed by : SBE
// *Level       : 1
///
/// @verbatim
/// Procedure Summary:
///   - Use Attributes to send VDD, VCS via the AVS bus to VRMs
///   - Use Attributes to adjust the VDN and send via I2C to VRM
///   - Read core frequency ATTR and write to the Quad PPM
/// @endverbatim

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fapi2.H>
#include "p9_sbe_setup_evid.H"

//-----------------------------------------------------------------------------
// Procedure
//-----------------------------------------------------------------------------

fapi2::ReturnCode
p9_sbe_setup_evid(const fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP>& i_target)
{

    //fapi2::ReturnCode l_rc = fapi2::FAPI2_RC_SUCCESS;

    // Substep indicators

    // commented out in Level 1 to not have "unused variable" warnings
    // until the SBE substep management "macro" or "call" is defined.

    // const uint32_t STEP_SBE_EVID_START              = 0x1;
    // const uint32_t STEP_SBE_EVID_CONFIG             = 0x2;
    // const uint32_t STEP_SBE_EVID_WRITE_VDN          = 0x3;
    // const uint32_t STEP_SBE_EVID_POLL_VDN_STATUS    = 0x4;
    // const uint32_t STEP_SBE_EVID_WRITE_VDD          = 0x5;
    // const uint32_t STEP_SBE_EVID_POLL_VDD_STATUS    = 0x6;
    // const uint32_t STEP_SBE_EVID_WRITE_VCS          = 0x7;
    // const uint32_t STEP_SBE_EVID_POLL_VCS_STATUS    = 0x8;
    // const uint32_t STEP_SBE_EVID_TIMEOUT            = 0x9;
    // const uint32_t STEP_SBE_EVID_BOOT_FREQ          = 0xA;
    // const uint32_t STEP_SBE_EVID_COMPLETE           = 0xB;

// The inclusion of the following will cause a "label 'fapi_try_exit' defined but not used"
// compile error in Cronus.  This will be uncommented when FAPI_TRY functions are added
// during the real procedure development.  However, this is NOT needed for Level 1.
//fapi_try_exit:
    return fapi2::current_err;

} // Procedure

