/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/hwpf/plat/include/plat_hw_access.H $                  */
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
/**
 *  @file plat_hw_access.H
 *
 *  @brief Define platform specific calls for PPE Platforms that use the machine
 *  check function of the PPE to deal with errors (eg no explicit return codes
 *  returned)
 */

#ifndef PLATHWACCESS_H_
#define PLATHWACCESS_H_

#include <plat_includes.H>

/// PIB Error Mask

#define PLAT_SET_PIB_ERROR_MASK(_m_mask) \
    {   /*  Read MSR */ \
        uint32_t msr_data = mfmsr(); \
        /* Set SEM field */ \
        msr_data &= ~(BITS(0,8)); \
        msr_data |= (uint32_t)(i_mask << 24); \
        /* Write MSR */ \
        mtmsr(msr_data); \
    };
    
#define PLAT_GET_PIB_ERROR_MASK(_m_mask) \
        uint8_t _m_mask; \
        uint32_t _sem = mfmsr(); \
        _m_mask = (uint8_t)((_sem & MSR_SEM) >> (32-(MSR_SEM_START_BIT + MSR_SEM_LEN)));
        
// Building block PPE instructions
#define PPE_MFMSR(_m_data) \
asm volatile \
    ( \
    "mfmsr %[data] \n" \
    :  [data]"=&r"(*_m_data) \
    :  "[data]"(*_m_data) \
    );

#define PPE_MTMSR(_m_data) \
asm volatile \
    ( \
    "mtmsr %[data] \n" \
    :  [data]"=&r"(*_m_data) \
    :  "[data]"(*_m_data) \
    );


/// GetScom   
#define PLAT_GETSCOM(_m_rc, _m_base, _m_offset, _m_data) \
    PPE_LVDX(_m_base.getAddressOverlay(), (uint32_t)(_m_offset & BITS(40,24)), _m_data)
    
/// PutScom
#define PLAT_PUTSCOM(_m_base, _m_offset, _m_data) \
    PPE_STVDX(_m_rc, _m_base.getAddressOverlay(), (uint32_t)(_m_offset & BITS(40,24)), _m_data)
    
/// ModifyScom
#define PLAT_MODSCOM(_m_base, _m_offset, _m_data, _m_mode) \
    PPE_STVDX(_m_base.getAddressOverlay(), _m_offset, _m_data)


/// GetCFAM  
#define PLAT_GETCFAM(_m_base, _m_offset, _m_data) \
    static_assert( K == TARGET_TYPE_NONE, \
                  "getCfamRegister is not supported by PPE platforms")

/// PutCFAM
#define PLAT_PUTCFAM(_m_base, _m_offset, _m_data) \
     static_assert( K == TARGET_TYPE_NONE, \
                  "putCfamRegister is not supported by PPE platforms")
                  
/// ModifyCFAM
#define PLAT_MODCFAM(_m_base, _m_offset, _m_data, _m_mode) \
     static_assert( K == TARGET_TYPE_NONE, \
                  "modifyCfamRegister is not supported by PPE platforms")

#endif // PLATHWACCESS_H_

