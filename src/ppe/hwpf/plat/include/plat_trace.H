/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/hwpf/plat/include/plat_trace.H $                      */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2015                        */
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
 *  @file plat_trace.H
 *  @brief Defines the FAPI2 trace macros.
 *
 *  Note that platform code must provide the implementation.
 *
 *  FAPI has provided a default implementation. Platform code must
 *  provide an alternate implementation if needed.
 */

#ifndef FAPI2_PLATTRACE_H_
#define FAPI2_PLATTRACE_H_

#include <stdio.h>
#include <stdint.h>

// @todo  update these headers with extern "C" in a future commit
//        or not and leave this just as it is.
extern "C" 
{
#include "pk.h"
#include <pk_trace.h>
#include "trac_interface.h"
}

// Why not a #define, why is this in the fapi2 namespace?
// To prevent problems with Cronus and the fapi1 definitions.
namespace fapi2
{
    static const uint32_t MAX_ECMD_STRING_LEN = 64;
};

// Information traces (go into fast trace buffer that can wrap often)
#define FAPI_TRACE(_id_, _fmt_, _args_...)                              \
    PK_TRACE(_fmt_, ##_args_);


/*  The following is a desirous trace entry but the second line has a
    compilation issue that is unresolved
    
#define FAPI_TRACE(_id_, _fmt_, _args_...)                              \
    PK_TRACE("%s: %s:%d ", _id_, __FUNCTION__, __LINE__);               \
    PK_TRACE(_fmt_, ##_args_);
*/

#define FAPI_INF(_fmt_, _args_...) FAPI_TRACE("inf", _fmt_, ##_args_)

// Important traces (go into slow trace buffer that should not wrap often)
#define FAPI_IMP(_fmt_, _args_...) FAPI_TRACE("imp", _fmt_, ##_args_)

// Error traces (go into slow trace buffer that should not wrap often)
#define FAPI_ERR(_fmt_, _args_...) FAPI_TRACE("err", _fmt_, ##_args_)

// Debug traces (go into fast trace buffer that can wrap often)
#define FAPI_DBG(_fmt_, _args_...) FAPI_TRACE("dbg", _fmt_, ##_args_)

// Scan traces
#define FAPI_SCAN(_fmt_, _args_...) FAPI_TRACE("scan", _fmt_, ##_args_)

#define FAPI_MFG(_fmt_, _args_...) FAPI_TRACE("mfg", _fmt_, ##_args_)

#endif // FAPI2_PLATTRACE_H_
