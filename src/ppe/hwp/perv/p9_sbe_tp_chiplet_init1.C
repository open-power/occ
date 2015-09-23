/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/hwp/perv/p9_sbe_tp_chiplet_init1.C $                  */
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
/// @file  p9_sbe_tp_chiplet_init1.C
///
/// @brief Initial steps of PIB AND PCB
//------------------------------------------------------------------------------
// *HWP HWP Owner        : Abhishek Agarwal <abagarw8@in.ibm.com>
// *HWP HWP Backup Owner : Srinivas V Naga <srinivan@in.ibm.com>
// *HWP FW Owner         : sunil kumar <skumar8j@in.ibm.com>
// *HWP Team             : Perv
// *HWP Level            : 2
// *HWP Consumed by      : SBE
//------------------------------------------------------------------------------


//## auto_generated
#include "p9_sbe_tp_chiplet_init1.H"

#include "perv_scom_addresses.H"


fapi2::ReturnCode p9_sbe_tp_chiplet_init1(const
        fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP> & i_target_chip)
{
    fapi2::buffer<uint64_t> l_data64;
    FAPI_DBG("Entering ...");

    //Setting ROOT_CTRL0 register value
    FAPI_TRY(fapi2::getScom(i_target_chip, PERV_ROOT_CTRL0_SCOM, l_data64));
    l_data64.clearBit<15>();  //PIB.ROOT_CTRL0.VDD2VIO_LVL_FENCE_DC = 0
    FAPI_TRY(fapi2::putScom(i_target_chip, PERV_ROOT_CTRL0_SCOM, l_data64));

    FAPI_INF("Release PCB Reset");
    //Setting ROOT_CTRL0 register value
    FAPI_TRY(fapi2::getScom(i_target_chip, PERV_ROOT_CTRL0_SCOM, l_data64));
    l_data64.clearBit<30>();  //PIB.ROOT_CTRL0.PCB_RESET_DC = 0
    FAPI_TRY(fapi2::putScom(i_target_chip, PERV_ROOT_CTRL0_SCOM, l_data64));

    FAPI_INF("Set Chiplet Enable");
    //Setting PERV_CTRL0 register value
    FAPI_TRY(fapi2::getScom(i_target_chip, PERV_PERV_CTRL0_SCOM, l_data64));
    l_data64.setBit<0>();  //PIB.PERV_CTRL0.TP_CHIPLET_EN_DC = 1
    FAPI_TRY(fapi2::putScom(i_target_chip, PERV_PERV_CTRL0_SCOM, l_data64));

    FAPI_INF("Drop TP Chiplet Fence Enable");
    //Setting PERV_CTRL0 register value
    FAPI_TRY(fapi2::getScom(i_target_chip, PERV_PERV_CTRL0_SCOM, l_data64));
    l_data64.clearBit<18>();  //PIB.PERV_CTRL0.TP_FENCE_EN_DC = 0
    FAPI_TRY(fapi2::putScom(i_target_chip, PERV_PERV_CTRL0_SCOM, l_data64));

    FAPI_INF("Drop Global Endpoint reset");
    //Setting ROOT_CTRL0 register value
    FAPI_TRY(fapi2::getScom(i_target_chip, PERV_ROOT_CTRL0_SCOM, l_data64));
    l_data64.clearBit<31>();  //PIB.ROOT_CTRL0.GLOBAL_EP_RESET_DC = 0
    FAPI_TRY(fapi2::putScom(i_target_chip, PERV_ROOT_CTRL0_SCOM, l_data64));
    FAPI_INF("Switching PIB trace bus to SBE tracing");

    FAPI_DBG("Exiting ...");

fapi_try_exit:
    return fapi2::current_err;

}
