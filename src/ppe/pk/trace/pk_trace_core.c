/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/pk/trace/pk_trace_core.c $                            */
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

/// \file pk_trace_core.c
/// \brief PK Trace core data and code.
///
/// This file includes the minimal code/data required to do minimal tracing.
/// This includes the periodic timer initialization and the pk_trace_tiny
/// function.  The pk_trace_tiny function is called by the PK_TRACE() macro
/// when there is one or less parameters (not including the format string)
/// and the parameter size is 16 bits or smaller.
///

#include "pk.h"
#include "pk_trace.h"

void pk_trace_timer_callback(void* arg);

#if (PK_TRACE_SUPPORT && PK_TIMER_SUPPORT)

//Static initialization of the trace timer
PkTimer g_pk_trace_timer = {
    .deque = PK_DEQUE_ELEMENT_INIT(),
    .timeout = 0,
    .callback = pk_trace_timer_callback,
    .arg = 0,
};

//Static initialization of the pk trace buffer
PkTraceBuffer g_pk_trace_buf =
{
    .version            = PK_TRACE_VERSION,
    .image_str          = PPE_IMG_STRING,
    .hash_prefix        = PK_TRACE_HASH_PREFIX,
    .partial_trace_hash = trace_ppe_hash("PARTIAL TRACE ENTRY. HASH_ID = %d", PK_TRACE_HASH_PREFIX),
    .size               = PK_TRACE_SZ,
    .max_time_change    = PK_TRACE_MTBT,
    .hz                 = 500000000, //default value. Actual value is set in pk_init.c
    .time_adj64         = 0,
    .state.word64       = 0,
    .cb                 = {0}
};

//Needed for buffer extraction in simics for now
PkTraceBuffer* g_pk_trace_buf_ptr = &g_pk_trace_buf;

// Creates an 8 byte entry in the trace buffer that includes a timestamp,
// a format string hash value and a 16 bit parameter.
//
// i_parm has the hash value combined with the 16 bit parameter
void pk_trace_tiny(uint32_t i_parm)
{
    PkTraceTiny         footer;
    PkTraceState        state;
    uint64_t*           ptr64;
    uint64_t            tb64;
    PkMachineContext    ctx;

    //fill in the footer data
    footer.parms.word32 = i_parm;
    tb64 = pk_timebase_get();
    state.tbu32 = tb64 >> 32;
    footer.time_format.word32 = tb64 & 0x00000000ffffffffull;
    
    footer.time_format.format = PK_TRACE_FORMAT_TINY;

    //The following operations must be done atomically
    pk_critical_section_enter(&ctx);

    //load the current byte count and calculate the address for this
    //entry in the cb
    ptr64 = (uint64_t*)&g_pk_trace_buf.cb[g_pk_trace_buf.state.offset & PK_TRACE_CB_MASK];

    //calculate the offset for the next entry in the cb
    state.offset = g_pk_trace_buf.state.offset + sizeof(PkTraceTiny);

    //update the cb state (tbu and offset)
    g_pk_trace_buf.state.word64 = state.word64;

    //write the data to the circular buffer including the
    //timesamp, string hash, and 16bit parameter
    *ptr64 = footer.word64;

    //exit the critical section
    pk_critical_section_exit(&ctx);
}


// This function is called periodically in order to ensure that the max ticks
// between trace entries is no more than what will fit inside a 32bit value.
void pk_trace_timer_callback(void* arg)
{

    // guarantee at least one trace before the lower 32bit timebase flips
    PK_TRACE("PERIODIC TIMESTAMPING TRACE");

    // restart the timer
    pk_timer_schedule(&g_pk_trace_timer,
                      PK_TRACE_TIMER_PERIOD);
}

// Use this function to synchronize the timebase between multiple PPEs.
// PPE A can send PPE B it's current timebase and then PPE B can set that
// as the current timebase for tracing purposes.  It can also be used
// to set the current time to 0.  This function changes the timebase for
// all entries that are currently in the trace buffer.  Setting the current
// timebase to 0 will cause previous traces to have very large timestamps.
void pk_trace_set_timebase(PkTimebase timebase)
{
    g_pk_trace_buf.time_adj64 = timebase - pk_timebase_get();
}

#endif
