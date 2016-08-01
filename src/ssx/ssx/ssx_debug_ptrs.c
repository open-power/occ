/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/ssx/ssx_debug_ptrs.c $                                */
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

/// \file ssx_debug_ptrs.c
/// \brief Defines a table of pointers to important kernel debug data.
///
/// This table is placed in a special section named .debug_ptrs which can be
/// placed at a well-known memory location for tools to find.
///

#include "ssx.h"
#include "ssx_trace.h"
#include "ssx_debug_ptrs.h"


extern SsxTimebase ppc405_64bit_ext_timebase;

#if SSX_TRACE_SUPPORT
    extern SsxTraceBuffer g_ssx_trace_buf;
#endif

ssx_debug_ptrs_t ssx_debug_ptrs SECTION_ATTRIBUTE(".debug_ptrs") =
{
    .debug_ptrs_size            = sizeof(ssx_debug_ptrs),
    .debug_ptrs_version         = SSX_DEBUG_PTRS_VERSION,

#if SSX_TRACE_SUPPORT
    .debug_trace_ptr            = &g_ssx_trace_buf,
    .debug_trace_size           = sizeof(g_ssx_trace_buf),
#else
    .debug_trace_ptr            = 0,
    .debug_trace_size           = 0,
#endif /* SSX_TRACE_SUPPORT */

#if SSX_THREAD_SUPPORT
    .debug_thread_table_ptr     = &__ssx_priority_map,
    .debug_thread_table_size    = sizeof(__ssx_priority_map),
    .debug_thread_runq_ptr      = (void*)& __ssx_run_queue,
    .debug_thread_runq_size     = sizeof(__ssx_run_queue),
#else
    .debug_thread_table_ptr     = 0,
    .debug_thread_table_size    = 0,
    .debug_thread_runq_ptr      = 0,
    .debug_thread_runq_size     = 0,
#endif /* SSX_THREAD_SUPPORT */

    .debug_timebase_ptr         = &ppc405_64bit_ext_timebase,
    .debug_timebase_size        = sizeof(ppc405_64bit_ext_timebase),

};

