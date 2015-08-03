/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/pk/kernel/pk_kernel.h $                               */
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
#ifndef __PK_KERNEL_H__
#define __PK_KERNEL_H__
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2014
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file pk_kernel.h
/// \brief PK portable kernel (non-API) data and data structures
///
/// \todo In theory, as long as the critical section entry/exit macros use GCC
/// memory barriers, we should be able to eliminate all of the 'volatile'
/// declarations in PK code.  These have been added to the port, so
/// we should try it.

#ifdef __PK_CORE_C__
#define IF__PK_CORE_C__(x) x
#define UNLESS__PK_CORE_C__(x) 
#else
#define IF__PK_CORE_C__(x)
#define UNLESS__PK_CORE_C__(x) x
#endif

#if PK_MINIMIZE_KERNEL_CODE_SPACE
#define IF_PK_MINIMIZE_KERNEL_CODE_SPACE(x) x
#define UNLESS_PK_MINIMIZE_KERNEL_CODE_SPACE(x) 
#else
#define IF_PK_MINIMIZE_KERNEL_CODE_SPACE(x)
#define UNLESS_PK_MINIMIZE_KERNEL_CODE_SPACE(x) x
#endif


#ifndef __ASSEMBLER__

/// This is the stack pointer saved when switching from a thread context to an
/// interrupt context.

UNLESS__PK_CORE_C__(extern)
volatile
PkAddress __pk_saved_sp;

/// The kernel stack; constant once defined by the call of
/// pk_initialize(). 

UNLESS__PK_CORE_C__(extern)
volatile
PkAddress __pk_kernel_stack;

/// This is the run queue - the queue of mapped runnable tasks.
UNLESS__PK_CORE_C__(extern)
volatile
PkThreadQueue __pk_run_queue;

/// This flag is set by \c __pk_schedule() if a new highest-priority thread
/// becomes runnable during an interrupt handler.  The context switch will
/// take place at the end of interrupt processing, and the
/// interrupt handling code will clear the flag. 

UNLESS__PK_CORE_C__(extern)
volatile
int __pk_delayed_switch;

/// The currently running thread, or NULL (0) to indicate the idle thread
///
/// \a __pk_current_thread holds a pointer to the currently executing
/// thread.  This pointer will be NULL (0) under the following conditions:
///
/// - After pk_initialize() but prior to pk_start_threads()
///
/// - After pk_start_threads(), when no threads are runnable.  In this case
/// the NULL (0) value indicates that the PK idle thread is 'running'. 
///
/// - After pk_start_threads(), when the current (non-idle) thread has
/// completed or been deleted. 
/// 
/// If \a __pk_current_thread == 0 then there is no requirement to save any
/// register state on a context switch, either because the PK idle thread has
/// no permanent context, or because any thread context on the kernel stack is
/// associated with a deleted thread.
///
/// If \a __pk_current_thread != 0 then \a __pk_current_thread is a pointer
/// to the currently executing thread.  In an interrupt handler \a
/// pk_current_thread is a pointer to the thread whose context is saved on
/// the kernel stack.
UNLESS__PK_CORE_C__(extern)
volatile
PkThread* __pk_current_thread;

/// The thread to switch to during the next context switch, or NULL (0).
///
/// \a __pk_next_thread is computed by __pk_schedule().  \a
/// __pk_next_thread holds a pointer to the thread to switch to at the next
/// context switch.  In a thread context the switch happens immediately if \a
/// __pk_next_thread == 0 or \a __pk_next_thread != \a __pk_current_thread.
/// In an interrupt context the check happens at the end of processing all
/// interrupts.
///
/// \a __pk_next_thread may be NULL (0) under the following
/// conditions: 
///
/// - After pk_initialize() but prior to pk_start_threads(), assuming no
/// threads have been made runnable.
///
/// - After pk_start_threads(), when no threads are runnable.  In this case
/// the NULL (0) value indicates that the PK idle thread is the next thread
/// to 'run'. 
/// 
/// If \a __pk_next_thread == 0 then there is no requirement to restore
/// any register state on a context switch, because the PK idle thread has
/// no permanent context.
///
/// If \a __pk_next_thread != 0 then \a __pk_next_thread is a pointer
/// to the thread whose context will be restored at the next context switch.
UNLESS__PK_CORE_C__(extern)
volatile
PkThread* __pk_next_thread;

/// The priority of \a __pk_next_thread
///
/// If \a __pk_next_thread == 0, the \a __pk_next_priority == PK_THREADS.
UNLESS__PK_CORE_C__(extern)
volatile
PkThreadPriority __pk_next_priority;

/// This variable holds the default thread machine context for newly created
/// threads. The idle thread also uses this context. This variable is normally
/// constant after the call of \c pk_initialize().

UNLESS__PK_CORE_C__(extern)
volatile
PkMachineContext __pk_thread_machine_context_default;


/// The size of the kernel stack (bytes).

UNLESS__PK_CORE_C__(extern)
volatile
size_t __pk_kernel_stack_size;

/// This table maps priorities to threads, and contains PK_THREADS + 1
/// entries. The final entry is for the idle thread and will always be null
/// after initizlization.

UNLESS__PK_CORE_C__(extern)
volatile
PkThread* __pk_priority_map[PK_THREADS + 1];

/// The PK time queue structure
///
/// This structure is defined for use by the kernel, however applications
/// could also use this structure to define their own time queues.

typedef struct {

    /// A sentinel node for the time queue.
    ///
    /// The time queue is an PkDeque managed as a FIFO queue for queue
    /// management purpose, although events time out in time order.
    ///
    /// This pointer container is defined as the first element of the
    /// structure to allow the PkTimeQueue to be cast to an PkDeque.
    PkDeque queue;

    /// The next timeout in absolute time.
    PkTimebase next_timeout;

    /// A pointer to allow preemption of time queue processing
    ///
    /// If non-0, then this is the next timer in the time queue to handle, or
    /// a pointer to the \a queue object indicating no more timers to handle.
    ///
    /// \a cursor != 0 implies that time queue handler is in the midst of
    /// processing the time queue, but has enabled interrupt preemption for
    /// processing a timer handler. This means that 1) if the timer pointed to
    /// by \a cursor is deleted then the cursor must be assigned to the
    /// next timer in the queue; and 2) if a new timer is scheduled then
    /// activating the next timeout will be handled by the timer handler.
    PkDeque* cursor;

} PkTimeQueue;

UNLESS__PK_CORE_C__(extern)
PkTimeQueue __pk_time_queue;

/// Return a pointer to the PkThread object of the currently running thread,
/// or NULL (0) if PK is idle or has not been started.
///
/// In this API the current thread is not volatile - it will never change
/// inside application code - thus the 'volatile' is cast away. The PK kernel
/// does not (must not) use this API.

UNLESS__PK_CORE_C__(extern)
inline PkThread *
pk_current(void)
{
    return (PkThread *)__pk_current_thread;
}

/// Schedule the next timeout in a machine-specific way.

void
__pk_schedule_hardware_timeout(PkTimebase timeout);

/// The thread timeout handler. Portable.

PK_TIMER_CALLBACK(__pk_thread_timeout);

/// Generic stack initialization. Portable.

int
__pk_stack_init(PkAddress *stack,
                 size_t     *size);

/// Machine-specific thread context initialization.

void 
__pk_thread_context_initialize(PkThread        *thread, 
                                PkThreadRoutine thread_routine, 
                                void             *arg);

/// Machine specific resumption of __pk_next_thread at __pk_next_priority
/// without saving the current context.
void
__pk_next_thread_resume(void);

/// Schedule a timer in the time queue. Portable.
void
__pk_timer_schedule(PkTimer *timer);

/// Remove a timer from the time queue. Portable.
int
__pk_timer_cancel(PkTimer *timer);

void
__pk_schedule(void);


// Call the application main(). Portable.

void
__pk_main(int argc, char **argv);

#endif  /* __ASSEMBLER__ */

#endif /* __PK_KERNEL_H__ */
