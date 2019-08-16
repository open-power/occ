/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/boltonlib/vanilla/ppe_timebase.h $                    */
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
#ifndef __PPE_TIMEBASE_H__
#define __PPE_TIMEBASE_H__

/// \file ppe_timebase.h
/// \brief support for using the OCB 32 bit timebase register
///

#include "pk.h"

//The timebase register will never be supported in the base ppe model
#ifdef APPCFG_USE_EXT_TIMEBASE
    #error "External timebase is not supported on the PPE standalone model"
#endif /* APPCFG_USE_EXT_TIMEBASE */

#ifndef __ASSEMBLER__

    //assembly function is defined in ppe42_timebase.S
    uint32_t pk_timebase32_get(void);

#endif  /* __ASSEMBLER__ */

#endif /* __PPE_TIMEBASE_H__ */
