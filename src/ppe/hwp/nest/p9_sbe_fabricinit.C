/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/hwp/nest/p9_sbe_fabricinit.C $                        */
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
/// @file p9_sbe_fabricinit.C
/// @brief Initialize island-mode fabric configuration (FAPI2)
///
/// @author Joe McGill <jmcgill@us.ibm.com>
/// @author Christy Graves <clgraves@us.ibm.com>
///

//
// *HWP HWP Owner: Joe McGill <jmcgill@us.ibm.com>
// *HWP FW Owner: Thi Tran <thi@us.ibm.com>
// *HWP Team: Nest
// *HWP Level: 3
// *HWP Consumed by: SBE
//

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <p9_sbe_fabricinit.H>


//------------------------------------------------------------------------------
// Constant definitions
//------------------------------------------------------------------------------

// ADU SCOM register address definitions
// TODO: these are currently incorrect in the FigTree/generated SCOM address header
//       including locally defined address constants here for testing purposes
const uint64_t PU_ALTD_CMD_REG = 0x00090001;
const uint64_t PU_ALTD_STATUS_REG = 0x00090003;
const uint64_t PU_SND_MODE_REG = 0x00090021;
// FBC SCOM register address definitions
// TODO: these are currently not present in the generated SCOM adddress header
//       including locally defined address constants here for testing purposes
const uint64_t PU_FBC_MODE_REG = 0x05011C0A;

// ADU delay/polling constants
const uint64_t FABRICINIT_DELAY_HW_NS = 1000; // 1us
const uint64_t FABRICINIT_DELAY_SIM_CYCLES = 200;

// ADU Command Register field/bit definitions
const uint32_t ALTD_CMD_START_OP_BIT = 2;
const uint32_t ALTD_CMD_CLEAR_STATUS_BIT = 3;
const uint32_t ALTD_CMD_RESET_FSM_BIT = 4;
const uint32_t ALTD_CMD_ADDRESS_ONLY_BIT = 6;
const uint32_t ALTD_CMD_LOCK_BIT = 11;
const uint32_t ALTD_CMD_DROP_PRIORITY_BIT = 20;
const uint32_t ALTD_CMD_OVERWRITE_PBINIT_BIT = 22;
const uint32_t ALTD_CMD_TTYPE_START_BIT = 25;
const uint32_t ALTD_CMD_TTYPE_END_BIT = 31;
const uint32_t ALTD_CMD_TSIZE_START_BIT = 32;
const uint32_t ALTD_CMD_TSIZE_END_BIT = 39;

const uint32_t ALTD_CMD_TTYPE_NUM_BITS = (ALTD_CMD_TTYPE_END_BIT-ALTD_CMD_TTYPE_START_BIT+1);
const uint32_t ALTD_CMD_TSIZE_NUM_BITS = (ALTD_CMD_TSIZE_END_BIT-ALTD_CMD_TSIZE_START_BIT+1);

const uint32_t ALTD_CMD_TTYPE_PBOP_EN_ALL = 0x3F;
const uint32_t ALTD_CMD_TSIZE_PBOP_EN_ALL = 0x0B;

// ADU Status Register field/bit definitions
const uint32_t ALTD_STATUS_ADDR_DONE_BIT = 2;
const uint32_t ALTD_STATUS_PBINIT_MISSING_BIT = 18;
const uint32_t ALTD_STATUS_CRESP_START_BIT = 59;
const uint32_t ALTD_STATUS_CRESP_END_BIT = 63;

const uint32_t ALTD_STATUS_CRESP_NUM_BITS = (ALTD_STATUS_CRESP_END_BIT-ALTD_STATUS_CRESP_START_BIT+1);

const uint32_t ALTD_STATUS_CRESP_ACK_DONE = 0x04;

// ADU PMisc Register field/bit definitions
const uint32_t ALTD_SND_MODE_FBC_STOP_BIT = 22;

// FBC Mode Register field/bit definitions
const uint32_t PU_FBC_MODE_PB_INITIALIZED_BIT = 0;


//------------------------------------------------------------------------------
// Function definitions
//------------------------------------------------------------------------------


fapi2::ReturnCode
p9_sbe_fabricinit(const fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP> & i_target)
{
    FAPI_INF("Start");

    fapi2::buffer<uint64_t> l_cmd_data;
    fapi2::buffer<uint64_t> l_pmisc_mode_data;
    fapi2::buffer<uint64_t> l_status_data_act;
    fapi2::buffer<uint64_t> l_status_data_exp;
    fapi2::buffer<uint64_t> l_fbc_mode_data;

    // check state of fabric pervasive stop control signal
    // if set, this would prohibit all fabric commands from being broadcast
    FAPI_TRY(fapi2::getScom(i_target, PU_SND_MODE_REG, l_pmisc_mode_data),
             "Error reading ADU PMisc Mode register");
    FAPI_ASSERT(!l_pmisc_mode_data.getBit<ALTD_SND_MODE_FBC_STOP_BIT>(),
                fapi2::P9_SBE_FABRICINIT_FBC_STOPPED_ERR().set_TARGET(i_target),
                "Pervasive stop control is asserted, so fabricinit will not run!");

    // write ADU Command Register to attempt lock acquisition
    // hold lock until finished with sequence
    FAPI_DBG("Lock and reset ADU ...");
    l_cmd_data.setBit<ALTD_CMD_LOCK_BIT>();
    FAPI_TRY(fapi2::putScom(i_target, PU_ALTD_CMD_REG, l_cmd_data),
             "Error writing ADU Command Register to acquire lock");

    // clear ADU status/reset state machine
    l_cmd_data.setBit<ALTD_CMD_CLEAR_STATUS_BIT>()
              .setBit<ALTD_CMD_RESET_FSM_BIT>();
    FAPI_TRY(fapi2::putScom(i_target, PU_ALTD_CMD_REG, l_cmd_data),
             "Error writing ADU Command Register to clear status and reset state machine");

    // launch init command
    FAPI_DBG("Launching fabric init command via ADU ...");
    l_cmd_data.setBit<ALTD_CMD_START_OP_BIT>()
              .clearBit<ALTD_CMD_CLEAR_STATUS_BIT>()
              .clearBit<ALTD_CMD_RESET_FSM_BIT>()
              .setBit<ALTD_CMD_ADDRESS_ONLY_BIT>()
              .setBit<ALTD_CMD_DROP_PRIORITY_BIT>()
              .setBit<ALTD_CMD_OVERWRITE_PBINIT_BIT>();
    l_cmd_data.insertFromRight<ALTD_CMD_TTYPE_START_BIT,ALTD_CMD_TTYPE_NUM_BITS>(ALTD_CMD_TTYPE_PBOP_EN_ALL);
    l_cmd_data.insertFromRight<ALTD_CMD_TSIZE_START_BIT,ALTD_CMD_TSIZE_NUM_BITS>(ALTD_CMD_TSIZE_PBOP_EN_ALL);
    FAPI_TRY(fapi2::putScom(i_target, PU_ALTD_CMD_REG, l_cmd_data),
             "Error writing ADU Command Register to launch init operation");

    // delay prior to checking for completion
    FAPI_TRY(fapi2::delay(FABRICINIT_DELAY_HW_NS, FABRICINIT_DELAY_SIM_CYCLES),
             "Error from delay");

    // read ADU Status Register and check for expected pattern
    FAPI_DBG("Checking status of ADU operation ...");
    FAPI_TRY(fapi2::getScom(i_target, PU_ALTD_STATUS_REG, l_status_data_act),
             "Error polling ADU Status Register");

    l_status_data_exp.setBit<ALTD_STATUS_ADDR_DONE_BIT>();
    l_status_data_exp.insertFromRight<ALTD_STATUS_CRESP_START_BIT,ALTD_STATUS_CRESP_NUM_BITS>(ALTD_STATUS_CRESP_ACK_DONE);

    FAPI_ASSERT(l_status_data_exp == l_status_data_act,
                fapi2::P9_SBE_FABRICINIT_FAILED_ERR().set_TARGET(i_target),
                "Fabric init failed, or mismatch in expected ADU status!");

    // clear ADU Command Register to release lock
    FAPI_DBG("Success! Releasing ADU lock ...");
    l_cmd_data = 0;
    FAPI_TRY(fapi2::putScom(i_target, PU_ALTD_CMD_REG, l_cmd_data),
             "Error writing ADU Command Register to release lock");

    // confirm that fabric was successfully initialized
    FAPI_DBG("Checking status of FBC init ...");
    FAPI_TRY(fapi2::getScom(i_target, PU_FBC_MODE_REG, l_fbc_mode_data),
             "Error reading FBC Mode Register");
    FAPI_ASSERT(l_fbc_mode_data.getBit<PU_FBC_MODE_PB_INITIALIZED_BIT>(),
                fapi2::P9_SBE_FABRICINIT_NO_INIT_ERR().set_TARGET(i_target),
                "ADU command succeded, but fabric was not initialized!");

fapi_try_exit:
    FAPI_INF("End");
    return fapi2::current_err;
}
