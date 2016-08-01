/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/trace/ssx_trace_big.c $                               */
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

/// \file ssx_trace_big.c
/// \brief SSX Trace function that supports up to four 32-bit parameters
///
/// The ssx_trace_big function is only called (via some macro magic) if the
/// caller passes in a single parameter (not including the format string)
/// that is larger than 16 bits to the SSX_TRACE(...) macro.
///

#include "ssx.h"
#include "ssx_trace.h"

#if (SSX_TRACE_SUPPORT && SSX_TIMER_SUPPORT)
void ssx_trace_big(uint32_t i_hash_and_count,
                   uint64_t i_parm1, uint64_t i_parm2)
{
    SsxTraceBig         footer;
    SsxTraceBig*        footer_ptr;
    SsxTraceState       state;
    uint64_t*           ptr64;
    uint64_t            tb64;
    SsxMachineContext   ctx;
    uint32_t            parm_size;
    uint32_t            cur_offset;
    uint32_t            footer_offset;

    //fill in the footer data
    tb64 = ssx_ext_timebase_get();
    footer.parms.word32 = i_hash_and_count; //this has the parm count and hash
    state.tbu32 = tb64 >> 32;
    footer.time_format.word32 = tb64 & 0x00000000ffffffffull;
    footer.time_format.format = SSX_TRACE_FORMAT_BIG;

    //round up to 8 byte boundary
    if(footer.parms.num_parms <= 2)
    {
        parm_size = 8;
    }
    else
    {
        parm_size = 16;
    }

    //*****The following operations must be done atomically*****
    ssx_critical_section_enter(SSX_NONCRITICAL, &ctx);

    //load in the offset in the cb for the entry we are adding
    cur_offset = g_ssx_trace_buf.state.offset;

    //Find the offset for the footer (at the end of the entry)
    footer_offset = cur_offset + parm_size;

    //calculate the address of the footer
    ptr64 = (uint64_t*)&g_ssx_trace_buf.cb[footer_offset & SSX_TRACE_CB_MASK];

    //calculate the offset for the next entry in the cb
    state.offset = footer_offset + sizeof(SsxTraceBig);

    //update the cb state (tbu and offset)
    g_ssx_trace_buf.state.word64 = state.word64;

    //write the data to the circular buffer including the
    //timesamp, string hash, and 16bit parameter
    *ptr64 = footer.word64;

    //*******************exit the critical section***************
    ssx_critical_section_exit(&ctx);


    //write parm values to the circular buffer
    footer_ptr = (SsxTraceBig*)ptr64;
    ptr64 = (uint64_t*)&g_ssx_trace_buf.cb[cur_offset & SSX_TRACE_CB_MASK];
    *ptr64 = i_parm1;

    if(parm_size > 8)
    {
        ptr64 = (uint64_t*)&g_ssx_trace_buf.cb[(cur_offset + 8) & SSX_TRACE_CB_MASK];
        *ptr64 = i_parm2;
    }

    //Mark the trace entry update as being completed
    footer_ptr->parms.complete = 1;

}

#endif


