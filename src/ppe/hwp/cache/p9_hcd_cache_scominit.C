/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/hwp/cache/p9_hcd_cache_scominit.C $                   */
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
/// @file  p9_hcd_cache_scominit.C
/// @brief  Cache Customization SCOMs
///
/// *HWP HWP Owner   : David Du       <daviddu@us.ibm.com>
/// *HWP FW Owner    : Sangeetha T S  <sangeet2@in.ibm.com>
/// *HWP Team        : PM
/// *HWP Consumed by : SBE:SGPE
/// *HWP Level       : 1
///
/// Procedure Summary:
///   Apply any SCOM initialization to the cache
///   Stop L3 configuration mode
///   Configure Trace Stop on Xstop
///   DTS Initialization sequense
///

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <fapi2.H>
//#include <common_scom_addresses.H>
//will be replaced with real scom address header file
#include "p9_hcd_cache_scominit.H"

//------------------------------------------------------------------------------
// Constant Definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Procedure: Cache Customization SCOMs
//------------------------------------------------------------------------------

extern "C"
{

fapi2::ReturnCode
p9_hcd_cache_scominit(
    const fapi2::Target<fapi2::TARGET_TYPE_EX>& i_target)
{

#if 0
    fapi2::buffer<uint64_t> data;

    ///////
    // NCU
    ///////

    ///////
    // L3
    ///////

    FAPI_DBG("Configuring L3 disable");
    // - l3_setup L3_SETUP_ACTION_DISABLE, L3_SETUP_UNIT_L3

    ///////
    // OHA
    ///////

    FAPI_DBG("Enable OHA to accept idle operations \
              by removing idle state override");
    // - setp1_mcreadand D1
    // = ld      D0, EX_OHA_MODE_REG_RWx1002000D, P1
    FAPI_TRY(getScom(i_target, EX_OHA_MODE_REG_RWx1002000D, data));

    //FAPI_DBG("Read OHA_MODE value:    0x%16llx", io_pore.d0.read());
    // = andi    D0, D0, ~BIT(6)
    data.clearBit<6>();

    //FAPI_DBG("Updated OHA_MODE value: 0x%16llx", io_pore.d0.read());
    // = std     D0, EX_OHA_MODE_REG_RWx1002000D, P0
    FAPI_TRY(putScom(i_target, EX_OHA_MODE_REG_RWx1002000D, data));

    // set trace stop on checkstop
    // Get the ECID to apply trace setup to only Murano DD2+ / Venice
    // - lpcs    P1, STBY_CHIPLET_0x00000000
    // - ldandi  D0, PCBMS_DEVICE_ID_0x000F000F, P1, (CFAM_CHIP_ID_CHIP_MASK | CFAM_CHIP_ID_MAJOR_EC_MASK)
    // - cmpibraeq   D0, 1f, (CFAM_CHIP_ID_MURANO | CFAM_CHIP_ID_MAJOR_EC_1 )

    FAPI_DBG("Configuring EX chiplet trace arrays \
              to stop on checkstop/recoverable errors")
    // = sti     GENERIC_DBG_MODE_REG_0x000107C0, P0, BIT(7) | BIT(8)
    FAPI_TRY(putScom(i_target, GENERIC_DBG_MODE_REG_0x000107C0,
                     fapi2:buffer<uint64_t>().insertFromRight<7,2>(0x3)));

    // = sti     GENERIC_DBG_TRACE_REG2_0x000107CB, P0, BIT(17)
    FAPI_TRY(putScom(i_target, GENERIC_DBG_TRACE_REG2_0x000107CB,
                     fapi2:buffer<uint64_t>().setBit<17>()));
    // - 1:

    return fapi2::FAPI2_RC_SUCCESS;

    FAPI_CLEANUP();
    return fapi2::FAPI2_RC_PLAT_ERR_SEE_DATA;

#endif

    return fapi2::FAPI2_RC_SUCCESS;

} // Procedure


} // extern C



