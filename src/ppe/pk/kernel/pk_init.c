/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/pk/kernel/pk_init.c $                                 */
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
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2014
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file pk_init.c
/// \brief PK initialization
///
/// The entry points in this file are initialization routines - they are never
/// needed after PK initialization and their code space could be reclaimed by
/// the application after initialization if required.

#include "pk.h"
#include "pk_trace.h"

uint32_t __pk_timebase_frequency_hz;

/// The timebase frequency is passed into PK during initialization.  It cannot
/// be set statically because there is a requirement to support a frequency
/// that can change from one IPL to the next.  On the 405, scaling of time
/// intervals is accomplished by doing a 32x32 bit multiplication which is
/// supported by the ppc405 instruction set.  PPE42 does not support 32x32 bit
/// multiplication directly and some applications can not afford to use a
/// function call to emulate the operation.  Instead we scale the time
/// interval by shifting the value X bits to the right and adding it to itself.
/// This can scale the value by 2, 1.5, 1.25, 1.125, etc.
///
/// This is the right shift value.
/// NOTE: shifting by 0 gives a 2x scale factor, shifting by 32 gives a 1x
/// scale factor.
uint8_t  __pk_timebase_rshift = 32;

void pk_set_timebase_rshift(uint32_t timebase_freq_hz)
{
    //Use 1.0 scale if less than halfway between 1.0 and 1.25
    if(timebase_freq_hz <= (PK_BASE_FREQ_HZ + (PK_BASE_FREQ_HZ >> 3)))
    {
        __pk_timebase_rshift = 32;
    }

    //use 1.25 scale if less than halfway between 1.25 and 1.5
    else if(timebase_freq_hz <= (PK_BASE_FREQ_HZ + (PK_BASE_FREQ_HZ >> 3) + (PK_BASE_FREQ_HZ >> 2)))
    {
        __pk_timebase_rshift = 2;
    }
    //use 1.5 scale if less than halfway between 1.5 and 2.0
    else if(timebase_freq_hz <= (PK_BASE_FREQ_HZ + (PK_BASE_FREQ_HZ >> 2) + (PK_BASE_FREQ_HZ >> 1)))
    {
        __pk_timebase_rshift = 1;
    }
    //use 2.0 scale if greater than 1.5
    else
    {
        __pk_timebase_rshift = 0;
    }
}

/// Initialize PK.  
///
/// \param kernel_stack A stack area for interrupt and bottom-half handlers.
///
/// \param kernel_stack_size The size (in bytes) of the stack area for
/// interrupt and bottom-half handlers. 
///
/// \param initial_timebase The initial value of the PK timebase.
/// If the argument is given as the special value \c PK_TIMEBASE_CONTINUES, then the
/// timebase is not reset.
///
/// \param timebase_frequency_hz The frequency of the PK timebase in Hz.
///
/// This routine \e must be called before any other PK / routines, and \e
/// should be called before any interrupts are enabled.
///
/// Return values other than PK_OK (0) are errors; see \ref pk_errors
///
/// \retval 0 Successful completion
///
/// \retval -PK_INVALID_ARGUMENT_INIT A stack pointer is 0 or is given 
/// a 0 size.
///
/// \retval -PK_STACK_OVERFLOW One or both stacks are not large enough to
/// support a minimum context save in the event of an interrupt.

// Note that PK does not rely on any static initialization of dynamic
// variables. In debugging sessions using RAM-resident PK images it is
// assumed that the processor may be reset at any time, so we always need to
// reset everything at initialization.

int
pk_initialize(PkAddress     kernel_stack,
               size_t       kernel_stack_size,
               PkTimebase   initial_timebase,
               uint32_t     timebase_frequency_hz)
{
    int rc;

    if (PK_ERROR_CHECK_API)
    {
        PK_ERROR_IF((kernel_stack == 0) ||
                    (kernel_stack_size == 0),
                    PK_INVALID_ARGUMENT_INIT);
    }

    __pk_timebase_frequency_hz = timebase_frequency_hz;

    __pk_thread_machine_context_default = PK_THREAD_MACHINE_CONTEXT_DEFAULT;

    //set the shift adjustment to get us closer to the true
    //timebase frequency (versus what was hardcoded)
    pk_set_timebase_rshift(timebase_frequency_hz);

    rc = __pk_stack_init(&kernel_stack, &kernel_stack_size);

    if (rc)
    {
        return rc;
    }

    __pk_kernel_stack = kernel_stack;
    __pk_kernel_stack_size = kernel_stack_size;

#if PK_TIMER_SUPPORT

    // Initialize the time queue sentinel as a circular queue, set the next
    // timeout and clear the cursor.

    pk_deque_sentinel_create((PkDeque*)&__pk_time_queue);
    __pk_time_queue.cursor = 0;
    __pk_time_queue.next_timeout = PK_TIMEBASE_MAX;

#if PK_TRACE_SUPPORT
extern PkTimer       g_pk_trace_timer;
extern PkTraceBuffer g_pk_trace_buf;

    //set the trace timebase HZ
    g_pk_trace_buf.hz = timebase_frequency_hz;

    if(initial_timebase != PK_TIMEBASE_CONTINUES)
    {
        //set the timebase ajdustment for trace synchronization
        pk_trace_set_timebase(initial_timebase);
    }

    // Schedule the timer that puts a 64bit timestamp in the trace buffer
    // periodically.  This allows us to use 32bit timestamps.
    pk_timer_schedule(&g_pk_trace_timer,
                      PK_TRACE_TIMER_PERIOD);

#endif  /* PK_TRACE_SUPPORT */

#endif  /* PK_TIMER_SUPPORT */

#if PK_THREAD_SUPPORT

    // Clear the priority map. The final entry [PK_THREADS] is for the idle
    // thread.

    int i;

    for (i = 0; i <= PK_THREADS; i++)
    {
        __pk_priority_map[i] = 0;
    }

    // Initialize the thread scheduler

    __pk_thread_queue_clear(&__pk_run_queue);
    __pk_current_thread = 0;
    __pk_next_thread    = 0;
    __pk_delayed_switch = 0;

#endif  /* PK_THREAD_SUPPORT */

    return PK_OK;
}


// Set the timebase frequency.
int
pk_timebase_freq_set(uint32_t timebase_frequency_hz)
{
    __pk_timebase_frequency_hz = timebase_frequency_hz;
    pk_set_timebase_rshift(timebase_frequency_hz);

#if PK_TRACE_SUPPORT
    g_pk_trace_buf.hz = timebase_frequency_hz;
#endif
    // Does the initial_timebase need to be reset?
    return PK_OK;
}


/// Call the application main()
///
/// __pk_main() is called from the bootloader.  It's only purpose is to
/// provide a place for the PK_MAIN_HOOK to be called before main() is
/// called.

void
__pk_main(int argc, char** argv)
{
    PK_MAIN_HOOK;

    int main(int argc, char** argv);
    main(argc, argv);
}






        
        
        
