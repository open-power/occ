/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/pk/kernel/pk_thread_init.c $                          */
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

/// \file pk_thread_init.c
/// \brief PK thread API initialization routines
///
/// The entry points in this file are routines that are typically used during
/// initialization, and their code space could be deallocated and recovered if
/// no longer needed by the application after initialization.

#include "pk.h"

/// Create (initialize) a thread
///
/// \param thread A pointer to an PkThread structure to initialize
///
/// \param thread_routine The subroutine that implements the thread
///
/// \param arg Private data to be passed as the argument to the thread
/// routine when it begins execution
///
/// \param stack The stack space of the thread
///
/// \param stack_size The size of the stack in bytes
///
/// \param priority The initial priority of the thread
///
/// The \a thread argument must be a pointer to an uninitialized or completed
/// or deleted thread.  This \c PkThread structure \em is the thread, so this
/// memory area must not be modified by the application until the thread
/// completes or is deleted.  PK can not tell if an PkThread structure is
/// currently in use as a thread control block.pk_thread_create() will
/// silently overwrite an PkThread structure that is currently in use.
///
/// The stack area must be large enough to hold the dynamic stack requirements
/// of the entry point routine, and all subroutines and functions that might
/// be invoked on any path from the entry point.  The stack must also always
/// be able to hold the thread context in the event the thread is preempted,
/// plus other critical context.  PK aligns stack areas in machine-specific
/// ways, so that the actual stack area may reduced in size slightly if it is
/// not already aligned.
///
/// Threads are created runnable but unmapped.  A newly created thread will
/// not be eligible to run until a call of pk_thread_resume() targets the
/// thread.
///
/// Return values other than PK_OK (0) are errors; see \ref pk_errors
///
/// \retval 0 Successful completion
///
/// \retval -PK_INVALID_THREAD_AT_CREATE The \a thread is a null (0) pointer.
/// 
/// \retval -PK_INVALID_ARGUMENT_THREAD1 the \a thread_routine is null (0)
///
/// \retval -PK_INVALID_ARGUMENT_THREAD2 the \a priority is invalid, 
///
/// \retval -PK_INVALID_ARGUMENT_THREAD3 the stack area wraps around 
/// the end of memory.
///
/// \retval -PK_STACK_OVERFLOW The stack area at thread creation is smaller
/// than the minimum safe size.

int
pk_thread_create(PkThread         *thread,
                  PkThreadRoutine  thread_routine,
                  void              *arg,
                  PkAddress        stack,
                  size_t            stack_size,
                  PkThreadPriority priority)
{
    int rc;

    if (PK_ERROR_CHECK_API) {
        PK_ERROR_IF(thread == 0, PK_INVALID_THREAD_AT_CREATE);
        PK_ERROR_IF((thread_routine == 0) ||
                     (priority >= PK_THREADS),
                     PK_INVALID_ARGUMENT_THREAD1);
    }

    rc = __pk_stack_init(&stack, &stack_size);
    if (rc) {
        return rc;
    }

    thread->saved_stack_pointer = stack;
    thread->stack_base = stack;

    if (PK_STACK_DIRECTION < 0) {

        thread->stack_limit = stack - stack_size;
        if (PK_ERROR_CHECK_API) {
            PK_ERROR_IF(thread->stack_limit > thread->stack_base,
                         PK_INVALID_ARGUMENT_THREAD2);
        }

    } else {

        thread->stack_limit = stack + stack_size;
        if (PK_ERROR_CHECK_API) {
            PK_ERROR_IF(thread->stack_limit < thread->stack_base,
                         PK_INVALID_ARGUMENT_THREAD3);
        }
    }

    thread->semaphore = 0;
    thread->priority = priority;
    thread->state = PK_THREAD_STATE_SUSPENDED_RUNNABLE;
    thread->flags = 0;

    pk_timer_create(&(thread->timer), 
                    __pk_thread_timeout, 
                    (void *)thread);

    __pk_thread_context_initialize(thread, thread_routine, arg);

    return rc;
}
    

    

    

    

        

    
        


