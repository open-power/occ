/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/ssx/ssx_init.c $                                      */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2014,2016                        */
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
/// \file ssx_init.c
/// \brief SSX initialization
///
/// The entry points in this file are initialization routines - they are never
/// needed after SSX initialization and their code space could be reclaimed by
/// the application after initialization if required.

#include "ssx.h"

uint32_t __ssx_timebase_frequency_hz;
uint32_t __ssx_timebase_frequency_khz;
uint32_t __ssx_timebase_frequency_mhz;


/// Initialize SSX.  
///
/// \param noncritical_stack A stack area for noncritical interrupt handlers.
///
/// \param noncritical_stack_size The size (in bytes) of the stack area for
/// noncritical interrupt handlers. 
///
/// \param critical_stack A stack area for critical interrupt handlers.
///
/// \param critical_stack_size The size (in bytes) of the stack area for
/// critical interrupt handlers. 
///
/// \param initial_timebase The initial value of the SSX timebase.  If this
/// argument is given as the special value \c SSX_TIMEBASE_CONTINUE, then the
/// timebase is not reset.
///
/// \param timebase_frequency_hz The frequency of the SSX timebase in Hz.
///
/// This routine \e must be called before any other SSX / routines, and \e
/// should be called before any interrupts are enabled.
///
/// Return values other than SSX_OK (0) are errors; see \ref ssx_errors
///
/// \retval 0 Successful completion
///
/// \retval -SSX_INVALID_ARGUMENT_INIT A stack pointer is 0 or is given 
/// a 0 size.
///
/// \retval -SSX_STACK_OVERFLOW One or both stacks are not large enough to
/// support a minimum context save in the event of an interrupt.

// Note that SSX does not rely on any static initialization of dynamic
// variables. In debugging sessions using RAM-resident SSX images it is
// assumed that the processor may be reset at any time, so we always need to
// reset everything at initialization.

int
ssx_initialize(SsxAddress  noncritical_stack,
               size_t      noncritical_stack_size,
               SsxAddress  critical_stack,
               size_t      critical_stack_size,
               SsxTimebase initial_timebase,
               uint32_t    timebase_frequency_hz)
{
    int rc;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF((noncritical_stack == 0) ||
                     (noncritical_stack_size == 0) ||
                     (critical_stack == 0) ||
                     (critical_stack_size == 0),
                     SSX_INVALID_ARGUMENT_INIT);
    }

    if (initial_timebase != SSX_TIMEBASE_CONTINUES) {
        __ssx_timebase_set(initial_timebase);
    }

    __ssx_timebase_frequency_hz = timebase_frequency_hz;
    __ssx_timebase_frequency_khz = timebase_frequency_hz / 1000;
    __ssx_timebase_frequency_mhz = timebase_frequency_hz / 1000000;

    __ssx_thread_machine_context_default = SSX_THREAD_MACHINE_CONTEXT_DEFAULT;

    rc = __ssx_stack_init(&noncritical_stack, &noncritical_stack_size);
    if (rc) {
        return rc;
    }

    __ssx_noncritical_stack = noncritical_stack;
    __ssx_noncritical_stack_size = noncritical_stack_size;

    rc = __ssx_stack_init(&critical_stack, &critical_stack_size);
    if (rc) {
        return rc;
    }

    __ssx_critical_stack = critical_stack;
    __ssx_critical_stack_size = critical_stack_size;

#if SSX_TIMER_SUPPORT

    // Initialize the time queue sentinel as a circular queue, set the next
    // timeout and clear the cursor.

    ssx_deque_sentinel_create((SsxDeque*)&__ssx_time_queue);
    __ssx_time_queue.cursor = 0;
    __ssx_time_queue.next_timeout = SSX_TIMEBASE_MAX;

#endif  /* SSX_TIMER_SUPPORT */

#if SSX_THREAD_SUPPORT
    
    // Clear the priority map. The final entry [SSX_THREADS] is for the idle
    // thread.

    int i;
    for (i = 0; i <= SSX_THREADS; i++) {
        __ssx_priority_map[i] = 0;
    }

    // Initialize the thread scheduler

    __ssx_thread_queue_clear(&__ssx_run_queue);
    __ssx_current_thread = 0;   
    __ssx_next_thread    = 0;
    __ssx_delayed_switch = 0;

#endif  /* SSX_THREAD_SUPPORT */

   return SSX_OK;
}


/// Call the application main()
///
/// __ssx_main() is called from the bootloader.  It's only purpose is to
/// provide a place for the SSX_MAIN_HOOK to be called before main() is
/// called.

void
__ssx_main(int argc, char **argv)
{
    SSX_MAIN_HOOK;

    int main(int argc, char **argv);
    main(argc, argv);
}






        
        
        
