/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/iota/iota_debug_ptrs.c $                              */
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
#include "iota_app_cfg.h"
#include "iota_trace.h"
#include "iota_debug_ptrs.h"

iota_debug_ptrs_t iota_debug_ptrs __attribute__((section(".debug_ptrs"))) =
{
    .debug_ptrs_size            = sizeof(iota_debug_ptrs_t),
    .debug_ptrs_version         = IOTA_DEBUG_PTRS_VERSION,

#if PK_TRACE_SUPPORT
    .debug_trace_ptr            = &g_pk_trace_buf,
    .debug_trace_size           = sizeof(g_pk_trace_buf),
#else
    .debug_trace_ptr            = 0,
    .debug_trace_size           = 0,
#endif /* PK_TRACE_SUPPORT */

};
