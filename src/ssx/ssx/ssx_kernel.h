#ifndef __SSX_KERNEL_H__
#define __SSX_KERNEL_H__

// $Id: ssx_kernel.h,v 1.1.1.1 2013/12/11 21:03:27 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/ssx/ssx_kernel.h,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file ssx_kernel.h
/// \brief SSX portable kernel (non-API) data and data structures
///
/// \todo In theory, as long as the critical section entry/exit macros use GCC
/// memory barriers, we should be able to eliminate all of the 'volatile'
/// declarations in SSX code.  These have been added to the PPC405 port, so
/// we should try it.

#ifdef __SSX_CORE_C__
#define IF__SSX_CORE_C__(x) x
#define UNLESS__SSX_CORE_C__(x) 
#else
#define IF__SSX_CORE_C__(x)
#define UNLESS__SSX_CORE_C__(x) x
#endif

#if SSX_MINIMIZE_KERNEL_CODE_SPACE
#define IF_SSX_MINIMIZE_KERNEL_CODE_SPACE(x) x
#define UNLESS_SSX_MINIMIZE_KERNEL_CODE_SPACE(x) 
#else
#define IF_SSX_MINIMIZE_KERNEL_CODE_SPACE(x)
#define UNLESS_SSX_MINIMIZE_KERNEL_CODE_SPACE(x) x
#endif


#ifndef __ASSEMBLER__

/// This is the stack pointer saved when switching from a thread or
/// non-critical interrupt context to a full-mode critical interrupt context.

UNLESS__SSX_CORE_C__(extern) 
volatile 
SsxAddress __ssx_saved_sp_critical;

/// The critical interrupt stack; constant once defined by the call of
/// ssx_initialize(). 

UNLESS__SSX_CORE_C__(extern)
volatile 
SsxAddress __ssx_critical_stack;

/// This is the stack pointer saved when switching from a thread context to a
/// full-mode non-critical interrupt context.

UNLESS__SSX_CORE_C__(extern)
volatile
SsxAddress __ssx_saved_sp_noncritical;

/// The non-critical interrupt stack; constant once defined by the call of
/// ssx_initialize(). 

UNLESS__SSX_CORE_C__(extern)
volatile
SsxAddress __ssx_noncritical_stack;

/// This is the run queue - the queue of mapped runnable tasks.
UNLESS__SSX_CORE_C__(extern)
volatile
SsxThreadQueue __ssx_run_queue;

/// This flag is set by \c __ssx_schedule() if a new highest-priority thread
/// becomes runnable during an interrupt handler.  The context switch will
/// take place at the end of non-critical interrupt processing, and the
/// interrupt handling code will clear the flag. 

UNLESS__SSX_CORE_C__(extern)
volatile
int __ssx_delayed_switch;

/// The currently running thread, or NULL (0) to indicate the idle thread
///
/// \a __ssx_current_thread holds a pointer to the currently executing
/// thread.  This pointer will be NULL (0) under the following conditions:
///
/// - After ssx_initialize() but prior to ssx_start_threads()
///
/// - After ssx_start_threads(), when no threads are runnable.  In this case
/// the NULL (0) value indicates that the SSX idle thread is 'running'. 
///
/// - After ssx_start_threads(), when the current (non-idle) thread has
/// completed or been deleted. 
/// 
/// If \a __ssx_current_thread == 0 then there is no requirement to save any
/// register state on a context switch, either because the SSX idle thread has
/// no permanent context, or because any thread context on the kernel stack is
/// associated with a deleted thread.
///
/// If \a __ssx_current_thread != 0 then \a __ssx_current_thread is a pointer
/// to the currently executing thread.  In an interrupt handler \a
/// ssx_current_thread is a pointer to the thread whose context is saved on
/// the kernel stack.
UNLESS__SSX_CORE_C__(extern)
volatile
SsxThread* __ssx_current_thread;

/// The thread to switch to during the next context switch, or NULL (0).
///
/// \a __ssx_next_thread is computed by __ssx_schedule().  \a
/// __ssx_next_thread holds a pointer to the thread to switch to at the next
/// context switch.  In a thread context the switch happens immediately if \a
/// __ssx_next_thread == 0 or \a __ssx_next_thread != \a __ssx_current_thread.
/// In an interrupt context the check happens at the end of processing all
/// SSX_NONCRITICAL interrupts.
///
/// \a __ssx_next_thread may be NULL (0) under the following
/// conditions: 
///
/// - After ssx_initialize() but prior to ssx_start_threads(), assuming no
/// threads have been made runnable.
///
/// - After ssx_start_threads(), when no threads are runnable.  In this case
/// the NULL (0) value indicates that the SSX idle thread is the next thread
/// to 'run'. 
/// 
/// If \a __ssx_next_thread == 0 then there is no requirement to restore
/// any register state on a context switch, because the SSX idle thread has
/// no permanent context.
///
/// If \a __ssx_next_thread != 0 then \a __ssx_next_thread is a pointer
/// to the thread whose context will be restored at the next context switch.
UNLESS__SSX_CORE_C__(extern)
volatile
SsxThread* __ssx_next_thread;

/// The priority of \a __ssx_next_thread
///
/// If \a __ssx_next_thread == 0, the \a __ssx_next_priority == SSX_THREADS.
UNLESS__SSX_CORE_C__(extern)
volatile
SsxThreadPriority __ssx_next_priority;

/// This variable holds the default thread machine context for newly created
/// threads. The idle thread also uses this context. This variable is normally
/// constant after the call of \c ssx_initialize().

UNLESS__SSX_CORE_C__(extern)
volatile
SsxMachineContext __ssx_thread_machine_context_default;


/// The size of the noncritical stack (bytes).

UNLESS__SSX_CORE_C__(extern)
volatile
size_t __ssx_noncritical_stack_size;

/// The size of the critical stack (bytes).

UNLESS__SSX_CORE_C__(extern)
volatile
size_t __ssx_critical_stack_size;

/// This table maps priorities to threads, and contains SSX_THREADS + 1
/// entries. The final entry is for the idle thread and will always be null
/// after initizlization.

UNLESS__SSX_CORE_C__(extern)
volatile
SsxThread* __ssx_priority_map[SSX_THREADS + 1];

/// The SSX time queue structure
///
/// This structure is defined for use by the kernel, however applications
/// could also use this structure to define their own time queues.

typedef struct {

    /// A sentinel node for the time queue.
    ///
    /// The time queue is an SsxDeque managed as a FIFO queue for queue
    /// management purpose, although events time out in time order.
    ///
    /// This pointer container is defined as the first element of the
    /// structure to allow the SsxTimeQueue to be cast to an SsxDeque.
    SsxDeque queue;

    /// The next timeout in absolute time.
    SsxTimebase next_timeout;

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
    SsxDeque* cursor;

} SsxTimeQueue;

UNLESS__SSX_CORE_C__(extern)
SsxTimeQueue __ssx_time_queue;

/// Return a pointer to the SsxThread object of the currently running thread,
/// or NULL (0) if SSX is idle or has not been started.
///
/// In this API the current thread is not volatile - it will never change
/// inside application code - thus the 'volatile' is cast away. The SSX kernel
/// does not (must not) use this API.

UNLESS__SSX_CORE_C__(extern)
inline SsxThread *
ssx_current(void)
{
    return (SsxThread *)__ssx_current_thread;
}


/// Set the timebase.  This is only called at initialization. Machine
/// specific. 

void
__ssx_timebase_set(SsxTimebase t);

/// Schedule the next timeout in a machine-specific way.

void
__ssx_schedule_hardware_timeout(SsxTimebase timeout);

/// Cancel the next timeout in a machine-specific way.

void
__ssx_cancel_hardware_timeout(void);

/// The thread timeout handler. Portable.

SSX_TIMER_CALLBACK(__ssx_thread_timeout);

/// Generic stack initialization. Portable.

int
__ssx_stack_init(SsxAddress *stack,
                 size_t     *size);

/// Machine-specific thread context initialization.

void 
__ssx_thread_context_initialize(SsxThread        *thread, 
                                SsxThreadRoutine thread_routine, 
                                void             *arg);

/// Machine specific resumption of __ssx_next_thread at __ssx_next_priority
/// without saving the current context.
void
__ssx_next_thread_resume(void);

/// Schedule a timer in the time queue. Portable.
void
__ssx_timer_schedule(SsxTimer *timer);

/// Remove a timer from the time queue. Portable.
int
__ssx_timer_cancel(SsxTimer *timer);

void
__ssx_schedule(void);


// Call the application main(). Portable.

void
__ssx_main(int argc, char **argv);

#endif  /* __ASSEMBLER__ */

#endif /* __SSX_KERNEL_H__ */
