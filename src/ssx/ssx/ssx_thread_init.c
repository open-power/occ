// $Id: ssx_thread_init.c,v 1.2 2014/02/03 01:30:44 daviddu Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/ssx/ssx_thread_init.c,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file ssx_thread_init.c
/// \brief SSX thread API initialization routines
///
/// The entry points in this file are routines that are typically used during
/// initialization, and their code space could be deallocated and recovered if
/// no longer needed by the application after initialization.

#include "ssx.h"

/// Create (initialize) a thread
///
/// \param thread A pointer to an SsxThread structure to initialize
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
/// or deleted thread.  This \c SsxThread structure \em is the thread, so this
/// memory area must not be modified by the application until the thread
/// completes or is deleted.  SSX can not tell if an SsxThread structure is
/// currently in use as a thread control block.ssx_thread_create() will
/// silently overwrite an SsxThread structure that is currently in use.
///
/// The stack area must be large enough to hold the dynamic stack requirements
/// of the entry point routine, and all subroutines and functions that might
/// be invoked on any path from the entry point.  The stack must also always
/// be able to hold the thread context in the event the thread is preempted,
/// plus other critical context.  SSX aligns stack areas in machine-specific
/// ways, so that the actual stack area may reduced in size slightly if it is
/// not already aligned.
///
/// Threads are created runnable but unmapped.  A newly created thread will
/// not be eligible to run until a call of ssx_thread_resume() targets the
/// thread.
///
/// Return values other than SSX_OK (0) are errors; see \ref ssx_errors
///
/// \retval 0 Successful completion
///
/// \retval -SSX_INVALID_THREAD_AT_CREATE The \a thread is a null (0) pointer.
/// 
/// \retval -SSX_ILLEGAL_CONTEXT The API was called from a critical interrupt
/// context. 
///
/// \retval -SSX_INVALID_ARGUMENT_THREAD1 the \a thread_routine is null (0)
///
/// \retval -SSX_INVALID_ARGUMENT_THREAD2 the \a priority is invalid, 
///
/// \retval -SSX_INVALID_ARGUMENT_THREAD3 the stack area wraps around 
/// the end of memory.
///
/// \retval -SSX_STACK_OVERFLOW The stack area at thread creation is smaller
/// than the minimum safe size.

int
ssx_thread_create(SsxThread         *thread,
                  SsxThreadRoutine  thread_routine,
                  void              *arg,
                  SsxAddress        stack,
                  size_t            stack_size,
                  SsxThreadPriority priority)
{
    int rc;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF_CRITICAL_INTERRUPT_CONTEXT();
        SSX_ERROR_IF(thread == 0, SSX_INVALID_THREAD_AT_CREATE);
        SSX_ERROR_IF((thread_routine == 0) ||
                     (priority >= SSX_THREADS),
                     SSX_INVALID_ARGUMENT_THREAD1);
    }

    rc = __ssx_stack_init(&stack, &stack_size);
    if (rc) {
        return rc;
    }

    thread->saved_stack_pointer = stack;
    thread->stack_base = stack;

    if (SSX_STACK_DIRECTION < 0) {

        thread->stack_limit = stack - stack_size;
        if (SSX_ERROR_CHECK_API) {
            SSX_ERROR_IF(thread->stack_limit > thread->stack_base,
                         SSX_INVALID_ARGUMENT_THREAD2);
        }

    } else {

        thread->stack_limit = stack + stack_size;
        if (SSX_ERROR_CHECK_API) {
            SSX_ERROR_IF(thread->stack_limit < thread->stack_base,
                         SSX_INVALID_ARGUMENT_THREAD3);
        }
    }

    thread->semaphore = 0;
    thread->priority = priority;
    thread->state = SSX_THREAD_STATE_SUSPENDED_RUNNABLE;
    thread->flags = 0;

    ssx_timer_create_nonpreemptible(&(thread->timer), 
                                    __ssx_thread_timeout, 
                                    (void *)thread);

    __ssx_thread_context_initialize(thread, thread_routine, arg);

    return rc;
}
    

    

    

    

        

    
        


