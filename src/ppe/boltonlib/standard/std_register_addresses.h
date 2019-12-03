/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/boltonlib/standard/std_register_addresses.h $         */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2019                             */
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
#ifndef __STD_REGISTER_ADDRESSES_H__
#define __STD_REGISTER_ADDRESSES_H__

/// \file std_register_addresses.h
/// \brief Symbolic addresses for a standard PPE



// Define the base address for the external interrupt controller registers
// This can be overridden in the pk_app_cfg.h file
#ifndef STD_EIC_BASE
    #ifdef __PPE_QME
    #define STD_EIC_BASE 0xC0002600
    #else
        #define STD_EIC_BASE 0xC0000000
    #endif
#endif

// Define the base address for the PPE mode registers
// This can be overridden in the pk_app_cfg.h file
#ifndef STD_PMR_BASE
    #ifdef __PPE_QME
    #define STD_PMR_BASE 0xC0002400
    #else
        #define STD_PMR_BASE 0xC0000100
    #endif
#endif

// Note: This list only contains registers that are needed by PK.  If
// an application requires other registers, it should define them
// elsewhere (i.e., cme_register_addresses.h)
#ifdef __PPE_QME
    #define STD_LCL_EISR        (STD_EIC_BASE + 0x0000)
    #define STD_LCL_EISR_OR     (STD_EIC_BASE + 0x0020)
    #define STD_LCL_EISR_CLR    (STD_EIC_BASE + 0x0030)
    #define STD_LCL_EIMR        (STD_EIC_BASE + 0x0040)
    #define STD_LCL_EIMR_OR     (STD_EIC_BASE + 0x0060)
    #define STD_LCL_EIMR_CLR    (STD_EIC_BASE + 0x0070)
    #define STD_LCL_EIPR        (STD_EIC_BASE + 0x0080)
    #define STD_LCL_EIPR_OR     (STD_EIC_BASE + 0x00A0)
    #define STD_LCL_EIPR_CLR    (STD_EIC_BASE + 0x00B0)
    #define STD_LCL_EITR        (STD_EIC_BASE + 0x00C0)
    #define STD_LCL_EITR_OR     (STD_EIC_BASE + 0x00E0)
    #define STD_LCL_EITR_CLR    (STD_EIC_BASE + 0x00F0)
    #define STD_LCL_EISTR       (STD_EIC_BASE + 0x0100)
    #define STD_LCL_EINR        (STD_EIC_BASE + 0x0140)

    #define STD_LCL_TSEL        (STD_PMR_BASE + 0x0000)
    #define STD_LCL_TBR         (STD_PMR_BASE + 0x0080)
#else
    #define STD_LCL_EISR        (STD_EIC_BASE + 0x0000)
    #define STD_LCL_EISR_OR     (STD_EIC_BASE + 0x0010)
    #define STD_LCL_EISR_CLR    (STD_EIC_BASE + 0x0018)
    #define STD_LCL_EIMR        (STD_EIC_BASE + 0x0020)
    #define STD_LCL_EIMR_OR     (STD_EIC_BASE + 0x0030)
    #define STD_LCL_EIMR_CLR    (STD_EIC_BASE + 0x0038)
    #define STD_LCL_EIPR        (STD_EIC_BASE + 0x0040)
    #define STD_LCL_EIPR_OR     (STD_EIC_BASE + 0x0050)
    #define STD_LCL_EIPR_CLR    (STD_EIC_BASE + 0x0058)
    #define STD_LCL_EITR        (STD_EIC_BASE + 0x0060)
    #define STD_LCL_EITR_OR     (STD_EIC_BASE + 0x0070)
    #define STD_LCL_EITR_CLR    (STD_EIC_BASE + 0x0078)
    #define STD_LCL_EISTR       (STD_EIC_BASE + 0x0080)
    #define STD_LCL_EINR        (STD_EIC_BASE + 0x00a0)

    #define STD_LCL_TSEL        (STD_PMR_BASE + 0x0000)
    #define STD_LCL_TBR         (STD_PMR_BASE + 0x0040)
#endif

#endif // __STD_REGISTER_ADDRESSES_H__

