/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/pk/ppe42/ppe42_scom.c $                               */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2016                        */
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

/// \file   ppe42_scom.c
/// \brief  Lowest level PK SCOM definitions.
///
/// Currently these SCOM functions are only optimized for functionality, not
/// speed. Speed optimization will be done when we have full compiler support
/// for the low-level stvd and lvd SCOM OPs.
///
/// A FAPI-lite SCOM can call these PK SCOM functions.
///
/// Comment:
/// - No need to poll for SCOM completion, nor return error code of SCOM fails.
///   A SCOM fail will cause the GPE to hang if configured to do so. But do we
///   necessarily have to do this?  Wouldn't a gentle recovery from a SCOM fail
///   be preferred?

#include "pk.h"
#include "ppe42_scom.h"
#include "ppe42_msr.h"


uint32_t putscom_abs(const uint32_t i_address, uint64_t i_data)
{

    // Perform the Store Virtual Double instruction
    PPE_STVD(i_address, i_data);

    // Get the MSR[SIBRC] as the return code   
    uint32_t rc = mfmsr();
    rc = ((rc & MSR_SIBRC) >> (32-(MSR_SIBRC_START_BIT + MSR_SIBRC_LEN)));
    return (rc);
    
}

uint32_t _putscom( uint32_t i_chiplet_id, uint32_t i_address, uint64_t i_data)
{

    // Perform the Store Virtual Double Index instruction
    PPE_STVDX(i_chiplet_id, i_address, i_data);

    // Get the MSR[SIBRC] as the return code   
    uint32_t rc = mfmsr();
    rc = ((rc & MSR_SIBRC) >> (32-(MSR_SIBRC_START_BIT + MSR_SIBRC_LEN)));
    return (rc);

}

uint32_t getscom_abs( const uint32_t i_address, uint64_t *o_data)
{
    uint64_t temp;
    // Perform the Load Virtual Double instruction
    PPE_LVD(i_address, temp);
    PPE_STVD(o_data, temp);

    // Get the MSR[SIBRC] as the return code   
    uint32_t rc = mfmsr();
    rc = ((rc & MSR_SIBRC) >> (32-(MSR_SIBRC_START_BIT + MSR_SIBRC_LEN)));
    return (rc);
}


uint32_t _getscom( const uint32_t i_chiplet_id, const uint32_t i_address, uint64_t *o_data)
{
    uint64_t temp;
    // Perform the Load Virtual Double Index instruction
    PPE_LVDX(i_chiplet_id, i_address, temp);
    PPE_STVD(o_data, temp);

    // Get the MSR[SIBRC] as the return code   
    uint32_t rc = mfmsr();
    rc = ((rc & MSR_SIBRC) >> (32-(MSR_SIBRC_START_BIT + MSR_SIBRC_LEN)));
    return (rc);

}
