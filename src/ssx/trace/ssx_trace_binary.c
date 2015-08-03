/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/trace/ssx_trace_binary.c $                            */
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
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2014
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file ssx_trace_binary.c
/// \brief SSX Trace function for dumping memory contents
///
/// The ssx_trace_binary function is called by the SSX_TRACE_BINARY() macro.
///


#include "ssx.h"
#include "ssx_trace.h"

#if (SSX_TRACE_SUPPORT && SSX_TIMER_SUPPORT)
void ssx_trace_binary(uint32_t i_hash_and_size, void* bufp)
{
    SsxTraceBinary          footer;
    SsxTraceBinary*         footer_ptr;
    SsxTraceState           state;
    uint64_t*               ptr64;
    uint64_t                tb64;
    SsxMachineContext       ctx;
    uint32_t                data_size;
    uint32_t                cb_offset;
    uint32_t                footer_offset;
    uint8_t*                dest;
    uint8_t*                src;
    uint32_t                index;

    //fill in the footer data
    tb64 = ssx_ext_timebase_get();
    footer.parms.word32 = i_hash_and_size; //this has the size and hash
    state.tbu32 = tb64 >> 32;
    footer.time_format.word32 = tb64 & 0x00000000ffffffffull;
    footer.time_format.format = SSX_TRACE_FORMAT_BINARY;

    //round up to 8 byte boundary
    data_size = (footer.parms.num_bytes + 7) & ~0x00000007ul;

    //limit data size
    if(data_size > SSX_TRACE_CLIPPED_BINARY_SZ)
    {
        data_size = SSX_TRACE_CLIPPED_BINARY_SZ;
    }

    //*****The following operations must be done atomically*****
    ssx_critical_section_enter(SSX_NONCRITICAL, &ctx);

    //load in the offset in the cb for the entry we are adding
    cb_offset = g_ssx_trace_buf.state.offset;

    //Find the offset for the footer (at the end of the entry)
    footer_offset = cb_offset + data_size;

    //calculate the address of the footer
    ptr64 = (uint64_t*)&g_ssx_trace_buf.cb[footer_offset & SSX_TRACE_CB_MASK];

    //calculate the offset for the next entry in the cb
    state.offset = footer_offset + sizeof(SsxTraceBinary);

    //update the cb state (tbu and offset)
    g_ssx_trace_buf.state.word64 = state.word64;

    //write the footer data to the circular buffer including the
    //timesamp, string hash and data size
    *ptr64 = footer.word64;

    //*******************exit the critical section***************
    ssx_critical_section_exit(&ctx);

    //write data to the circular buffer
    for(src = bufp, index = 0;
        index < data_size;
        index++)
    {
        dest = &g_ssx_trace_buf.cb[(cb_offset + index) & SSX_TRACE_CB_MASK];
        *dest = *(src++);
    }

    //Mark the trace entry update as being completed
    footer_ptr = (SsxTraceBinary*)ptr64;
    footer_ptr->parms.complete = 1;

}

#endif


