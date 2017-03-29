/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/pk/gpe/gpe_timebase.h $                               */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2017                        */
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
#ifndef __GPE_TIMEBASE_H__
#define __GPE_TIMEBASE_H__

/// \file gpe_timebase.h
/// \brief support for using the OCB 32 bit timebase register
///
/// The OCB timebase register is a 32 bit count-up register that is shared
/// by all GPE's in the OCC complex.

#include "pk.h"

#ifndef __ASSEMBLER__

#ifdef APPCFG_USE_EXT_TIMEBASE
static inline
uint32_t pk_timebase32_get(void)
{
    return in32(OCB_OTBR);
}

#else
//assembly function is defined in ppe42_timebase.S
uint32_t pk_timebase32_get(void);

#endif /* APPCFG_USE_EXT_TIMEBASE */

#else

.macro _pk_timebase32_get rT, rA
_lwzi \rT, \rA, OCB_OTBR
.endm
#endif  /* __ASSEMBLER__ */

#endif /* __GPE_TIMEBASE_H__ */
