/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/powmanlib/gpehw_common.h $                            */
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
#ifndef __GPEHW_COMMON_H__
#define __GPEHW_COMMON_H__

enum GPE_PIG_TYPES
{
    GPE_PIG_PSTATE_PHASE1       = 0,
    GPE_PIG_PSTATE_PHASE2       = 1,
    GPE_PIG_STOP_STATE_CHANGE   = 2,
    GPE_PIG_CME_REQUEST         = 3,
    GPE_PIG_CME_ACK             = 4,
    GPE_PIG_CME_ERROR           = 5,
    GPE_PIG_QUAD_PPM_MSG        = 6,
    GPE_PIG_QUAD_PPM_ERROR      = 7
};


enum GPE_CHIPLET_CONFIGS
{
    CORES_PER_QUAD               = 4,
    MAX_QUADS                    = 8,
    MAX_CORES                    = 32
};

#if !defined(GPE_BUFFER)
#if defined(__PPC405__)
/// GPE data buffer in SRAM(mostly for IPC)
#define GPE_BUFFER(declaration) \
    declaration __attribute__ ((__aligned__(8))) __attribute__ ((section (".noncacheable")))
#else
#define GPE_BUFFER(declaration) \
    declaration __attribute__ ((__aligned__(8))) __attribute__ ((section (".sdata")))
#endif
#endif

#endif  /* __GPEHW_COMMON_H__ */
