/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/trace/ssx_trace_core.c $                              */
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

/// \file ssx_trace_core.c
/// \brief SSX Trace core data and code.
///
/// This file includes the minimal code/data required to do minimal tracing.
/// This includes the periodic timer initialization and the ssx_trace_tiny
/// function.  The ssx_trace_tiny function is called by the SSX_TRACE() macro
/// when there is one or less parameters (not including the format string)
/// and the parameter size is 16 bits or smaller.
///

#include "ssx.h"
#include "ssx_trace.h"

void ssx_trace_timer_callback(void* arg);

#if (SSX_TRACE_SUPPORT && SSX_TIMER_SUPPORT)

//Static initialization of the trace timer
SsxTimer g_ssx_trace_timer = {
    .deque = SSX_DEQUE_ELEMENT_INIT(),
    .timeout = 0,
    .callback = ssx_trace_timer_callback,
    .arg = 0,
    .options = SSX_TIMER_CALLBACK_PREEMPTIBLE,
};

//Static initialization of the ssx trace buffer
SsxTraceBuffer g_ssx_trace_buf =
{
    .version            = SSX_TRACE_VERSION,
    .image_str          = PPC_IMG_STRING,
    .hash_prefix        = SSX_TRACE_HASH_PREFIX,
    .partial_trace_hash = trace_ppe_hash("PARTIAL TRACE ENTRY. HASH_ID = %d", SSX_TRACE_HASH_PREFIX),
    .size               = SSX_TRACE_SZ,
    .max_time_change    = SSX_TRACE_MTBT,
    .hz                 = 500000000, //default value. Actual value is set in ssx_init.c
    .time_adj64         = 0,
    .state.word64       = 0,
    .cb                 = {0}
};

//Needed for buffer extraction in simics for now
SsxTraceBuffer* g_ssx_trace_buf_ptr = &g_ssx_trace_buf;

// Creates an 8 byte entry in the trace buffer that includes a timestamp,
// a format string hash value and a 16 bit parameter.
//
// i_parm has the hash value combined with the 16 bit parameter
void ssx_trace_tiny(uint32_t i_parm)
{
    SsxTraceTiny        footer;
    SsxTraceState       state;
    uint64_t*           ptr64;
    uint64_t            tb64;
    SsxMachineContext   ctx;

    //fill in the footer data
    footer.parms.word32 = i_parm;
    tb64 = ssx_ext_timebase_get();
    state.tbu32 = tb64 >> 32;
    footer.time_format.word32 = tb64 & 0x00000000ffffffffull;
    
    footer.time_format.format = SSX_TRACE_FORMAT_TINY;

    //The following operations must be done atomically
    ssx_critical_section_enter(SSX_NONCRITICAL, &ctx);

    //load the current byte count and calculate the address for this
    //entry in the cb
    ptr64 = (uint64_t*)&g_ssx_trace_buf.cb[g_ssx_trace_buf.state.offset & SSX_TRACE_CB_MASK];

    //calculate the offset for the next entry in the cb
    state.offset = g_ssx_trace_buf.state.offset + sizeof(SsxTraceTiny);

    //update the cb state (tbu and offset)
    g_ssx_trace_buf.state.word64 = state.word64;

    //write the data to the circular buffer including the
    //timesamp, string hash, and 16bit parameter
    *ptr64 = footer.word64;

    //exit the critical section
    ssx_critical_section_exit(&ctx);
}


// This function is called periodically in order to ensure that the max ticks
// between trace entries is no more than what will fit inside a 32bit value.
void ssx_trace_timer_callback(void* arg)
{

    // guarantee at least one trace before the lower 32bit timebase flips
    SSX_TRACE("PERIODIC TIMESTAMPING TRACE");

    // restart the timer
    ssx_timer_schedule(&g_ssx_trace_timer,
                      SSX_TRACE_TIMER_PERIOD,
                      0);
}

// Use this function to synchronize the timebase between multiple Processors.
// proc A can send proc B it's current timebase and then proc B can set that
// as the current timebase for tracing purposes.  It can also be used
// to set the current time to 0.  This function changes the timebase for
// all entries that are currently in the trace buffer.  Setting the current
// timebase to 0 will cause previous traces to have very large timestamps.
void ssx_trace_set_timebase(SsxTimebase timebase)
{
    g_ssx_trace_buf.time_adj64 = timebase - ssx_ext_timebase_get();
}

void ssx_trace_init(uint32_t    timebase_frequency_hz,
                    SsxTimebase initial_timebase)
{
    //set the trace timebase HZ (used by parsing tools)
    g_ssx_trace_buf.hz = timebase_frequency_hz;

    if(initial_timebase != SSX_TIMEBASE_CONTINUES)
    {
        //Set the timebase adjustment.  The external timebase
        //is not adjustable so we use a software adjustment instead.
        //Typically, this should only be used by the first processor to come
        //up in order to set the timebase to 0.  Other processors
        //will want to synchronize with the first processor's timebase.
        ssx_trace_set_timebase(initial_timebase);
    }

    // Schedule the timer that puts a timestamp in the trace buffer
    // periodically.  This allows us to use 32bit timestamps.
    ssx_timer_schedule(&g_ssx_trace_timer,
                       SSX_TRACE_TIMER_PERIOD,
                       0);
}

////Needed for easy cache flush of trace buffer
size_t g_ssx_trace_buf_size=sizeof(g_ssx_trace_buf);


#endif
