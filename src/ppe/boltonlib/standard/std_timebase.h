/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/boltonlib/standard/std_timebase.h $                   */
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
#ifndef __STD_TIMEBASE_H__
#define __STD_TIMEBASE_H__

/// \file std_timebase.h
/// \brief support for using the standard PPE 32 bit timebase register
///
/// Each standard PPE has it's own timebase register that runs at a constant
/// frequency.

#include "ppe42_mmio.h"
#include "std_register_addresses.h"

#ifndef __ASSEMBLER__

#ifndef APPCFG_USE_EXT_TIMEBASE
static inline
uint32_t pk_timebase32_get(void)
{
    return (uint32_t)((in64(STD_LCL_TBR)) >> 32);
}

#else
//assembly function is defined in ppe42_timebase.S
uint32_t pk_timebase32_get(void);

#endif  /* APPCFG_USE_EXT_TIMEBASE */

#else

.macro _pk_timebase32_get rT, rA
lis \rA, STD_LCL_TBR@ha
lvd \rT, STD_LCL_TBR@l(\rA)
.endm

#endif  /* __ASSEMBLER__ */

#endif /* __STD_TIMEBASE_H__ */
