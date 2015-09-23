/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/hwp/perv/p9_sbe_setup_evid.H $                        */
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
/// @file  p9_sbe_setup_evid.H
/// @brief Setup External Voltage IDs and Boot Frequency
///
/// *HW Owner    : Greg Still <stillgs@us.ibm.com>
/// *FW Owner    : Sangeetha T S <sangeet2@in.ibm.com>
/// *Team        : PM
/// *Consumed by : SBE
/// *Level       : 1
///

#ifndef __P9_SBE_SETUP_EVID_H__
#define __P9_SBE_SETUP_EVID_H__

extern "C"
{

/// @typedef p9_sbe_setup_evid_FP_t
/// function pointer typedef definition for HWP call support
    typedef fapi2::ReturnCode (*p9_sbe_setup_evid_FP_t) (
        const fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP> &);

/// @brief Read attributes containing this part's boot voltages (VDD, VCS and VDN)
///        and set these voltage using the AVSBUS interface (VDD, VCS) an I2C (VDN).
///        Also reads a differnt attribute containing the boot frequency and set that
///        into each configured EQ chiplet.
/// @param [in] i_target TARGET_TYPE_PROC_CHIP
/// @attr
/// @attritem   ATTR_BOOT_FREQ          uint16_t - 9 bit frequency multiplier of the refclk right justified
/// @attritem   ATTR_VCS_BOOT_VOLTAGE   uint16_t - 1mV grandularity setting for the VCS rail
/// @attritem   ATTR_VDD_BOOT_VOLTAGE   uint16_t - 1mV grandularity setting for the VDD rail
/// @attritem   ATTR_VDN_BOOT_VOLTAGE   uint16_t - 1mV grandularity setting for the VDN rail
///
/// @retval FAPI_RC_SUCCESS
    fapi2::ReturnCode
    p9_sbe_setup_evid(const fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP>& i_target);


} // extern C

#endif  // __P9_SBE_SETUP_EVID_H__
