/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/sbe/plat/src/return_code.C $                          */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2012,2015                        */
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

/**
 *  @file return_code.C
 *
 *  @brief Fuctions that process PPE return codes
 */

#include <return_code.H>

namespace fapi2
{
    
    /// @brief Takes a non-zero PIB return code and inssert the value into 
    /// a fapi2::ReturnCode
    /// @param[in]  i_msr         Value read from the PPE MSR
    /// @return fapi::ReturnCode. Built ReturnCode
    ReturnCode&  ReturnCode::insertPIBcode(uint32_t& rhs)
    {
        iv_rc = FAPI2_RC_PLAT_MASK | rhs;
        return iv_rc;
    }

}
