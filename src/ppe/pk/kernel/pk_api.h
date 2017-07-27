/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/pk/kernel/pk_api.h $                                  */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2017                        */
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
#ifndef __PK_API_H__
#define __PK_API_H__

/// \file pk_api.h
/// \brief Macros and declarations for the PK API.

//  Basic constants

/// Although the number of threads is defined as a manifest constant,
/// numerous parts of the PK code assume this definition. The number of
/// supported threads _can not_ be changed simply by changing this constant.

#define PK_THREADS 32

#define PK_IDLE_THREAD_PRIORITY PK_THREADS

//  Interrupt API

#define PK_IRQ_POLARITY_ACTIVE_LOW  0
#define PK_IRQ_POLARITY_ACTIVE_HIGH 1

#define PK_IRQ_TRIGGER_LEVEL_SENSITIVE 0
#define PK_IRQ_TRIGGER_EDGE_SENSITIVE  1

// API return codes

#define PK_OK                                 0

/// @see pk_panic_codes.h for valid return/panic codes

/// \defgroup pk_thread_states PK Thread States
///
/// Threads are created in the state PK_THREAD_STATE_SUSPENDED_RUNNABLE.
/// When the thread is mapped it transitions to state PK_THREAD_STATE_MAPPED.
/// A mapped thread is runnable if it appears in the run queue; there is no
/// other flag or status to indicate a runnable thread.  If a blocked thread
/// is suspended it goes into state PK_THREAD_STATE_SUSPENDED_BLOCKED.  For
/// all threads the reason for blockage is detailed in the \a flags field of
/// the thread; See \ref pk_thread_flags.  PK_THREAD_STATE_DELETED and
/// PK_THREAD_STATE_COMPLETED are effectively equivalent but named
/// individually for reporting purposes.
///
/// \note This separation of the thread \a state and \a flags allows the use
/// of an PK semaphore as a thread barrier, as it supports a non-iterative
/// implementation of pk_semaphore_release_all() in which all threads blocked
/// on the semaphore are simultaneously inserted into the run queue with an
/// atomic operation, followed by each individual thread readjusting its flags
/// appropriately once the thread runs again.
///
/// @{

#define PK_THREAD_STATE_SUSPENDED_RUNNABLE 1
#define PK_THREAD_STATE_MAPPED             2
#define PK_THREAD_STATE_SUSPENDED_BLOCKED  3
#define PK_THREAD_STATE_COMPLETED          4
#define PK_THREAD_STATE_DELETED            5

/// @}


/// \defgroup pk_thread_flags PK Thread Flags
///
/// The \a flag field of the thread extends the information contained in the
/// \a state field; See \ref pk_thread_states.  Blocked threads will show
/// PK_THREAD_FLAG_SEMAPHORE_PEND, PK_THREAD_FLAG_TIMER_PEND or both (if
/// blocked on a semaphore with timeout). The flag PK_THREAD_FLAG_TIMED_OUT
/// indicates that a thread timer timed out before the thread became
/// runnable.  Currently only the semaphore-pend-with-timeout code uses this
/// flag.
///
/// Note that a thread can be mapped and runnable (in the run queue) even
/// though PK_THREAD_FLAG_SEMAPHORE_PEND and/or PK_THREAD_FLAG_TIMER_PEND
/// are set. These flags are always cleared by the thread itself, not the code
/// that unblocks the thread. This allows the implementation of the
/// pk_semaphore_release_all() as explained in \ref pk_thread_states.
///
/// @{

#define PK_THREAD_FLAG_SEMAPHORE_PEND 0x1
#define PK_THREAD_FLAG_TIMER_PEND     0x2
#define PK_THREAD_FLAG_TIMED_OUT      0x4

/// @}


//  Critical Sections

/// Enter a critical section, saving the current machine
/// context.

#define pk_critical_section_enter(pctx) \
    pk_interrupt_disable(pctx)

/// Exit a critical section by restoring the previous machine context.

#define pk_critical_section_exit(pctx) \
    pk_machine_context_set(pctx)


/// Execute a statement atomically

#define PK_ATOMIC(stmt) \
    do { \
        PkMachineContext __ctx; \
        pk_critical_section_enter(&__ctx); \
        stmt; \
        pk_critical_section_exit(&__ctx); \
    } while (0)


// Application-overrideable definitions

/// Control whether or not the API functions check for errors.
///
/// This definition can be overriden by the application.

#ifndef PK_ERROR_CHECK_API
    #define PK_ERROR_CHECK_API 1
#endif

/// Control whether API errors cause kernel panics or return negative error
/// codes.
///
/// This selection is only valid if \c PK_ERROR_CHECK_API is defined
/// non-0. This definition can be overriden by the application.

#ifndef PK_ERROR_PANIC
    #define PK_ERROR_PANIC 1
#endif

/// Control whether or not the PK kernel checks key invariants.
///
/// Violations of kernel invariants always cause kernel panics.  This
/// definition can be overriden by the application.

#ifndef PK_ERROR_CHECK_KERNEL
    #define PK_ERROR_CHECK_KERNEL 1
#endif

/// Define the time interval type, which must be an unsigned type of a size
/// less then or equal to the size of \c PkTimebase.  This definition can be
/// overridden by the application.

#ifndef PK_TIME_INTERVAL_TYPE
    #define PK_TIME_INTERVAL_TYPE uint64_t
#endif

/// Provide support for the PkTimer APIs in addition to the default
/// initerrupt APIs.  This definition can be overridden by the application.

#ifndef PK_TIMER_SUPPORT
    #define PK_TIMER_SUPPORT 1
#endif

/// Provide support for the all PK APIs. Thread support requires/implies
/// support for time services and semaphores.  This definition can be
/// overridden by the application.

#ifndef PK_THREAD_SUPPORT
    #define PK_THREAD_SUPPORT 1
#endif

/// Control the level of stack checking.
///
/// This definition can be overriden by the application.
///
/// 0 : No stack prepatterning or checking is made for thread and kernel
/// stacks.
///
/// 1 : Kernel interrupt stacks are prepatterned during
/// \c pk_initialize(). Thread stacks are prepatterned during
/// \c pk_thread_create().
///
/// 2 : (\b Default - Currently Unimplemented) In addition to prepatterning,
/// stack utilization is computed at the exit of context switches and
/// interrupt processing.  The maximum utilization is stored in
/// the thread data structure.  The kernel will panic if stack overflow is
/// detected. Stack utilization is not computed for the idle thread.

#ifndef PK_STACK_CHECK
    #define PK_STACK_CHECK 1
#endif

/// A hook for main()
///
/// This hook macro is expanded in the body of __pk_main() prior to the call
/// of the application main().  The application can redefine this hook macro
/// in (or in headers referred to in) the application header
/// pk_app_cfg.h. The PK_MAIN_HOOK will run on the stack of main().

#ifndef PK_MAIN_HOOK
    #define PK_MAIN_HOOK do {} while (0)
#endif

/// A hook for pk_start_threads()
///
/// This hook macro is expanded in the call-tree of pk_start_threads() before
/// threads are actually started.  The application can redefine this hook
/// macro in (or in headers referred to in) the application header
/// pk_app_cfg.h.
///
/// The PK_START_THREADS_HOOK runs as a pseudo-interrupt handler on the
/// kernel stack, with external interrupts disabled.

#ifndef PK_START_THREADS_HOOK
    #define PK_START_THREADS_HOOK do {} while (0)
#endif

/// The maximum value of the \c PkTimebase type.

#define PK_TIMEBASE_MAX ((PkTimebase)-1)

/// The minimum count the DEC counter can have so as to not overrun PK
///  with DEC interrupts.
#define PK_DEC_MIN  32

/// A special value that specifies that the timebase will not be reset during
/// pk_init().

#define PK_TIMEBASE_CONTINUES PK_TIMEBASE_MAX

/// By convention, a timeout value indicating 'no waiting' in a call of \c
/// pk_semaphore_pend().

#define PK_NO_WAIT 0

/// By convention, a timeout value indicating 'wait forever' in a call of \c
/// pk_semaphore_pend().

#define PK_WAIT_FOREVER ((PkInterval)-1)

/// The PK timebase frequency in Hz
///
/// Earlier version of PK defined the timbase frequency as a preprocessor
/// macro. Now, the timebase frequency is specified as a parameter of the
/// pk_initialize() API. The macro remains defined for backwards
/// compatibility, however all kernel uses of the timebase frequency are now
/// optimized around the timebase parameter.

#define PK_TIMEBASE_FREQUENCY_HZ __pk_timebase_frequency_hz

/// This is the unscaled timebase frequency in Hz.
#ifndef PK_BASE_FREQ_HZ
    #ifdef APPCFG_USE_EXT_TIMEBASE
        #define PK_BASE_FREQ_HZ     (uint32_t)25000000
    #else
        #define PK_BASE_FREQ_HZ     (uint32_t)400000000
    #endif /* APPCFG_USE_EXT_TIMEBASE */
#endif
#define PK_BASE_FREQ_KHZ    (PK_BASE_FREQ_HZ / 1000)
#define PK_BASE_FREQ_MHZ    (PK_BASE_FREQ_HZ / 1000000)

/// Scale a time interval to be _closer_ to what was actually requested
/// base on the actual timebase frequency.
#define PK_INTERVAL_SCALE(interval) ((interval) + ((interval) >> __pk_timebase_rshift))

/// Convert a time in integral seconds to a time interval - overflows are
/// ignored. The application can redefine this macro.

#ifndef PK_SECONDS
    #define PK_SECONDS(s) ((PkInterval)(PK_BASE_FREQ_HZ * (s)))
#endif

/// Convert a time in integral milliseconds to a time interval - overflows are
/// ignored, and a frequency evenly (or closely) divisible by 1000 is
/// assumed. The application can redefine this macro.

#ifndef PK_MILLISECONDS
    #define PK_MILLISECONDS(m) ( (PkInterval)(PK_BASE_FREQ_KHZ * (m)) )
#endif

/// Convert a time in integral microseconds to a time interval - overflows are
/// ignored, and a frequncy evenly (or closely) divisible by 1,000,000 is
/// assumed. The application can redefine this macro.

#ifndef PK_MICROSECONDS
    #define PK_MICROSECONDS(u)  ( (PkInterval)(PK_BASE_FREQ_MHZ * (u)) )
#endif

/// Convert a time in integral nanoseconds to a time interval - overflows are
/// ignored, and a frequeyncy evenly (or closely) divisible by 1,000,000 is
/// assumed. The application can redefine this macro.

#ifndef PK_NANOSECONDS
    #define PK_NANOSECONDS(n) ( (PkInterval)( ( ((PK_BASE_FREQ_MHZ<<10)/1000) * (n) ) >> 10) )
#endif

/// Enable PK application tracing for latency measurments
#ifndef PK_TRACE_PERF_ENABLE
    #define PK_TRACE_PERF_ENABLE 0
#endif

/// Enable PK application tracing (enabled by default)
#ifndef PK_TRACE_ENABLE
    #define PK_TRACE_ENABLE 1
#endif

/// Enable PK ctrl  (enabled by default)
#ifndef PK_TRACE_CTRL_ENABLE
    #define PK_TRACE_CTRL_ENABLE 1
#endif

/// Enable PK crit  (disabled by default)
#ifndef PK_TRACE_CRIT_ENABLE
    #define PK_TRACE_CRIT_ENABLE 0
#endif

/// Enable PK ckpt  (disabled by default)
#ifndef PK_TRACE_CKPT_ENABLE
    #define PK_TRACE_CKPT_ENABLE 0
#endif

/// Enable Debug suppress  (disabled by default)
// a.k.a. enabled means turn off PK_TRACE(), but keep crit trace
#ifndef PK_TRACE_DBG_SUPPRESS
    #define PK_TRACE_DBG_SUPPRESS 0
#endif

/// Enable PK kernel tracing (disabled by default)
#ifndef PK_KERNEL_TRACE_ENABLE
    #define PK_KERNEL_TRACE_ENABLE 0
#endif

/// pk trace disabled implies no tracing at all
//   override any other trace settings
#if !PK_TRACE_ENABLE
    #undef PK_TRACE_DBG_SUPPRESS
    #undef PK_TRACE_CTRL_ENABLE
    #undef PK_TRACE_CRIT_ENABLE
    #undef PK_TRACE_CKPT_ENABLE

    #define PK_TRACE_DBG_SUPPRESS 1
    #define PK_TRACE_CTRL_ENABLE 0
    #define PK_TRACE_CRIT_ENABLE 0
    #define PK_TRACE_CKPT_ENABLE 0
#endif

// PK TRACE enabled implies all default tracing on.
// PK TRACE enabled & PK CKPT DEBUG disabled implies PK CRIT INFO and CTRL ERROR tracing only.
// PK TRACE enabled & PK CKPT DEBUG and CRIT INFO disabled implies PK CTRL ERROR tracing only.
// PK TRACE enabled & PK CRIT INFO disabled && PK DEBUG disabled && PK ERROR disabled implies
//          PK TRACE disabled
#if PK_TRACE_ENABLE &&  PK_TRACE_DBG_SUPPRESS && !PK_TRACE_CRIT_ENABLE && !PK_TRACE_CKPT_ENABLE && !PK_TRACE_CTRL_ENABLE
    #undef PK_TRACE_ENABLE
    #define PK_TRACE_ENABLE 0
#endif


//Application trace macros
#if !PK_TRACE_PERF_ENABLE
    #define PK_TRACE_PERF(...)
#else
    #define PK_TRACE_PERF(...) PKTRACE(__VA_ARGS__)
#endif

#if PK_TRACE_DBG_SUPPRESS
    #define PK_TRACE(...)
    #define PK_TRACE_BIN(str, bufp, buf_size)
#else
    #define PK_TRACE(...) PKTRACE(__VA_ARGS__)
    #define PK_TRACE_BIN(str, bufp, buf_size) PKTRACE_BIN(str, bufp, buf_size)
#endif

#if !PK_TRACE_CTRL_ENABLE
    #define PK_TRACE_ERR(...)
#else
    #define PK_TRACE_ERR(...) PKTRACE(__VA_ARGS__)
#endif

#if !PK_TRACE_CRIT_ENABLE
    #define PK_TRACE_INF(...)
#else
    #define PK_TRACE_INF(...) PKTRACE(__VA_ARGS__)
#endif

#if !PK_TRACE_CKPT_ENABLE
    #define PK_TRACE_DBG(...)
#else
    #define PK_TRACE_DBG(...) PKTRACE(__VA_ARGS__)
#endif

//Kernel trace macros
#if !PK_KERNEL_TRACE_ENABLE
    #define PK_KERN_TRACE(...)
    #define PK_KERN_TRACE_ASM16(...)
#else
    #define PK_KERN_TRACE(...) PK_TRACE(__VA_ARGS__)
    #define PK_KERN_TRACE_ASM16(...) PK_TRACE_ASM16(__VA_ARGS__)
#endif  /* PK_KERNEL_TRACE_ENABLE */


/// Add a string to the trace buffer with an optional register holding a 16bit value
/// WARNING: This calls a c function which may clobber any of the volatile registers
#if (PK_TRACE_SUPPORT && PK_TIMER_SUPPORT)
    #define PK_TRACE_ASM16(...) TRACE_ASM_HELPER16(VARG_COUNT(__VA_ARGS__), __VA_ARGS__)
#else
    #define PK_TRACE_ASM16(...)
#endif /* PK_TRACE_SUPPORT */

/// The following macros are helper macros for tracing.  They should not be called
/// directly.
#define VARG_COUNT_HELPER(_0, _1, _2, _3, _4, _5, _6, _7, N, ...) N
#define VARG_COUNT(...) VARG_COUNT_HELPER(, ##__VA_ARGS__, 7, 6, 5, 4, 3, 2, 1, 0)

#ifdef __ASSEMBLER__
// *INDENT-OFF*
#define TRACE_ASM_HELPER16_CALL(count, ...) TINY_TRACE_ASM ## count (__VA_ARGS__)
#define TRACE_ASM_HELPER16(count, ...) TRACE_ASM_HELPER16_CALL(count, __VA_ARGS__)

#define TINY_TRACE_ASM0()   .error "format string required"
#define TINY_TRACE_ASM1(str) \
    .tiny_trace_asm1 trace_ppe_hash(str, PK_TRACE_HASH_PREFIX)
#define TINY_TRACE_ASM2(str, reg) \
    .tiny_trace_asm2 trace_ppe_hash(str, PK_TRACE_HASH_PREFIX), reg
#define TINY_TRACE_ASM3()   .error "too many parameters"
#define TINY_TRACE_ASM4()   .error "too many parameters"
#define TINY_TRACE_ASM5()   .error "too many parameters"
#define TINY_TRACE_ASM6()   .error "too many parameters"
#define TINY_TRACE_ASM7()   .error "too many parameters"

//Possible enhancement: add support for tracing more than 1 parameter and binary data in assembly

    .global pk_trace_tiny

    .macro  .tiny_trace_asm1 hash16
        lis     %r3, \hash16
        bl      pk_trace_tiny
    .endm

    .macro .tiny_trace_asm2 hash16, parm16
        clrlwi  %r3, \parm16, 16
        oris    %r3, %r3, \hash16
        bl      pk_trace_tiny
    .endm

// *INDENT-ON*
#endif /*__ASSEMBLER__*/



#ifndef __ASSEMBLER__

#include <stddef.h>
#include <stdint.h>

/// The timebase frequency in Hz; A parameter to pk_initialize()
extern uint32_t __pk_timebase_frequency_hz;

extern uint8_t __pk_timebase_rshift;

/// The timebase frequency in KHz
extern uint32_t __pk_timebase_frequency_khz;  //never set or used. Delete?

/// The timebase frequency in Mhz
extern uint32_t __pk_timebase_frequency_mhz;  //never set or used. Delete?


typedef unsigned long int PkAddress;

typedef uint8_t PkThreadState;

typedef uint8_t PkThreadPriority;

typedef uint8_t PkThreadFlags;

typedef uint32_t PkSemaphoreCount;

typedef uint64_t PkTimebase;

typedef PK_TIME_INTERVAL_TYPE PkInterval;

#include "pk_port_types.h"

typedef struct
{

    /// A priority queue of threads pending on the semaphore.
    PkThreadQueue pending_threads;

    /// The current semaphore count.
    PkSemaphoreCount count;

    /// The maximum allowable count - for error checking.
    PkSemaphoreCount max_count;

} PkSemaphore;


/// Compile-time initialize a PkSemaphore structure
///
/// This low-level macro creates a structure initializatin of an PkSemaphore
/// structure. This can be used for example to create compile-time initialized
/// arrays of semaphores.
#define PK_SEMAPHORE_INITIALIZATION(_initial_count, _max_count)        \
    {.pending_threads = 0,                                              \
                        .count = (_initial_count),                                  \
                                 .max_count = (_max_count)}


/// Declare and initialize a semaphore
#define PK_SEMAPHORE(sem, initial_count, max_count)                    \
    PkSemaphore sem = PK_SEMAPHORE_INITIALIZATION(initial_count, max_count)


/// Trace macros for C functions
#define HASH_ARG_COMBO(str, arg) \
    ((((uint32_t)trace_ppe_hash(str, PK_TRACE_HASH_PREFIX)) << 16) | ((uint32_t)(arg) & 0x0000ffff))

#define PKTRACE0(...) pk_trace_tiny() //will fail at compile time

#define PKTRACE1(str) \
    pk_trace_tiny((trace_ppe_hash(str, PK_TRACE_HASH_PREFIX) << 16))

#define PKTRACE2(str, parm0) \
    ((sizeof(parm0) <= 2)? \
     pk_trace_tiny(HASH_ARG_COMBO(str, parm0)): \
     pk_trace_big(HASH_ARG_COMBO(str, 1), ((uint64_t)parm0) << 32, 0))

#define PKTRACE3(str, parm0, parm1) \
    pk_trace_big(HASH_ARG_COMBO(str, 2), ((((uint64_t)parm0) << 32) | parm1), 0)

#define PKTRACE4(str, parm0, parm1, parm2) \
    pk_trace_big(HASH_ARG_COMBO(str, 3), ((((uint64_t)parm0) << 32) | parm1),\
                 ((uint64_t)parm2) << 32 )

#define PKTRACE5(str, parm0, parm1, parm2, parm3) \
    pk_trace_big(HASH_ARG_COMBO(str, 4), ((((uint64_t)parm0) << 32) | parm1),\
                 ((((uint64_t)parm2) << 32) | parm3) )

#define PKTRACE6(...) pk_trace_tiny() //will fail at compile time
#define PKTRACE7(...) pk_trace_tiny() //will fail at compile time

#define PKTRACE_HELPER2(count, ...) PKTRACE ## count (__VA_ARGS__)
#define PKTRACE_HELPER(count, ...) PKTRACE_HELPER2(count, __VA_ARGS__)

#if (PK_TRACE_SUPPORT && PK_TIMER_SUPPORT)
#define PKTRACE(...) PKTRACE_HELPER(VARG_COUNT(__VA_ARGS__), __VA_ARGS__)
#define PKTRACE_BIN(str, bufp, buf_size) \
    pk_trace_binary(((buf_size < 255)? HASH_ARG_COMBO(str, buf_size): HASH_ARG_COMBO(str, 255)), bufp)
#else
#define PKTRACE(...)
#define PKTRACE_BIN(str, bufp, buf_size)
#endif //PK_TRACE_SUPPORT



/// A generic doubly-linked list object
///
/// This object functions both as a sentinel mode for a deque as well as a
/// pointer container for elements in deques.  The PK API assumes that
/// queueable structures will be defined with an PkDeque structure as the
/// initial 'data member' of the structure.  This allows a pointer to a queue
/// element to be cast to a pointer to an PkDeque and vice-versa.

typedef struct PkDeque
{

    /// Pointer to the head or the next element in a deque.
    ///
    /// When an PkDeque is used as the sentinel node for a queue, \a next
    /// points to the head of the queue, and the condition (next == \<self\>)
    /// indicates an empty PkDeque. By convention the condition (\a next ==
    /// 0) is used to indicate that a queue element is not enqueued.
    struct PkDeque* next;

    /// Pointer to the tail or previous element in a deque.
    ///
    /// When a DQueue is used as the sentinel node for a queue, \a previous
    /// points to the tail of the queue.
    struct PkDeque* previous;

} PkDeque;


typedef void (*PkTimerCallback)(void*);

#define PK_TIMER_CALLBACK(callback) void callback(void *)

struct PkTimer;

/// The PK timer object

typedef struct PkTimer
{

    /// The time queue management pointers
    ///
    /// This pointer container is defined as the first element of the
    /// structure to allow the PkTimer to be cast to an PkDeque and
    /// vice-versa.
    PkDeque deque;

    /// The absolute timeout of the timer.
    PkTimebase timeout;

    /// The timer callback
    ///
    /// For PK thread timers used to implement Sleep and semaphore pend
    /// timeouts this field is initialized to __pk_thread_timeout().
    PkTimerCallback callback;

    /// Private data passed to the callback.
    ///
    /// For PK thread timers used to implement Sleep and semaphore pend this
    /// field is initialized to a pointer to the thread.
    void* arg;

} PkTimer;


// Threads

typedef void (*PkThreadRoutine)(void* arg);

#define PK_THREAD_ROUTINE(f) void f(void *arg);

typedef struct
{

    /// Stack pointer saved during context switches.  Assembler code expects
    /// this to always be at address offset 0 from the thread pointer.
    PkAddress saved_stack_pointer;

    /// This is 1 past the last valid byte address of the thread stack.
    /// Assembler code expects this to always be at address offset (sizeof
    /// PkAddress) from the thread pointer.
    PkAddress stack_limit;

    /// This is the original base of the stack.
    /// Assembler code expects this to always be at address offset 2 * (sizeof
    /// PkAddress) from the thread pointer.
    PkAddress stack_base;

    /// If the thread is blocked on a semaphore, then this is the semaphore the
    /// thread is blocked on.
    PkSemaphore* semaphore;

    /// The thread priority.
    PkThreadPriority priority;

    /// The thread state; See \ref pk_thread_states
    PkThreadState state;

    /// Thread flags; See \ref pk_thread_flags
    PkThreadFlags flags;

    /// The timer structure handles Sleep and blocking on a semaphore with
    /// timeout.
    PkTimer timer;

} PkThread;


typedef void (*PkBhHandler)(void*);

#define PK_BH_HANDLER(handler) void handler(void *)

typedef struct
{

    /// The bottom half queue management pointers
    ///
    /// This pointer container is defined as the first element of the
    /// structure to allow the PkBottomHalf to be cast to a PkDeque and
    /// vice-versa.
    PkDeque deque;

    /// The bottom half handler
    PkBhHandler bh_handler;

    /// Private data passed to the handler.
    void* arg;

} PkBottomHalf;


// Initialization APIs

int
pk_initialize(PkAddress  kernel_stack,
              size_t      kernel_stack_size,
              PkTimebase initial_timebase,
              uint32_t    timebase_frequency_hz);

/**
 * Set the timebase frequency.
 * @param[in] The frequency in HZ
 * @return PK_OK
 * @pre  pk_initialize
 * @Note This interface is intended for SBE. The timebase frequency value is
 *       used by PK to calcate timed events. Any existing timeouts,
 *       sleeps, or time based pending semaphores are not recalculated.
 */
int
pk_timebase_freq_set(uint32_t timebase_frequency_hz);

// Timebase APIs

PkTimebase
pk_timebase_get(void);


// Timer APIs

int
pk_timer_create(PkTimer*         timer,
                PkTimerCallback callback,
                void*             arg);


int
pk_timer_schedule(PkTimer*    timer,
                  PkInterval interval);

int
pk_timer_cancel(PkTimer* timer);

int
pk_timer_info_get(PkTimer*    timer,
                  PkTimebase* timeout,
                  int*         active);

// Thread APIs

int
pk_thread_create(PkThread*         thread,
                 PkThreadRoutine  thread_routine,
                 void*              arg,
                 PkAddress        stack,
                 size_t            stack_size,
                 PkThreadPriority priority);

int
pk_start_threads(void);

int
pk_thread_resume(PkThread* thread);

int
pk_thread_suspend(PkThread* thread);

int
pk_thread_delete(PkThread* thread);

int
pk_complete(void);

int
pk_sleep(PkInterval interval);

int
pk_thread_info_get(PkThread*         thread,
                   PkThreadState*    state,
                   PkThreadPriority* priority,
                   int*               runnable);

int
pk_thread_priority_change(PkThread*         thread,
                          PkThreadPriority new_priority,
                          PkThreadPriority* old_priority);

int
pk_thread_at_priority(PkThreadPriority priority,
                      PkThread**         thread);

int
pk_thread_priority_swap(PkThread* thread_a, PkThread* thread_b);


// Semaphore APIs

int
pk_semaphore_create(PkSemaphore*      semaphore,
                    PkSemaphoreCount initial_count,
                    PkSemaphoreCount max_count);

int
pk_semaphore_post(PkSemaphore* semaphore);

int
pk_semaphore_pend(PkSemaphore* semaphore,
                  PkInterval  timeout);

int
pk_semaphore_release_all(PkSemaphore* semaphore);


int
pk_semaphore_info_get(PkSemaphore*      semaphore,
                      PkSemaphoreCount* count,
                      int*               pending);

void
pk_semaphore_post_handler(void* arg,
                          PkIrqId irq);

// Misc. APIs

void
pk_halt() __attribute__ ((noreturn));

// Deque APIs

int
pk_deque_sentinel_create(PkDeque* deque);

#define PK_DEQUE_SENTINEL_INIT(dq_addr) \
    {\
        .next = dq_addr, \
                .previous = dq_addr \
    }

#define PK_DEQUE_SENTINEL_STATIC_CREATE(deque) \
    PkDeque deque = PK_DEQUE_SENTINEL_INIT(&deque)

int
pk_deque_element_create(PkDeque* element);

#define PK_DEQUE_ELEMENT_INIT() \
    {\
        .next = 0, \
                .previous = 0 \
    }

#define PK_DEQUE_ELEMENT_STATIC_CREATE(deque) \
    PkDeque deque = PK_DEQUE_ELEMENT_INIT()

/// Check for an empty PkDeque
///
/// \param deque The sentinel node of a deque
///
/// \retval 0 The PkDeque is not empty
///
/// \retval 1 The PkDeque is empty

static inline int
pk_deque_is_empty(PkDeque* deque)
{
    return (deque == deque->next);
}


/// Check if an PkDeque element is currently enqueued
///
/// \param element Typically the PkDeque object of a queable structure
///
/// \retval 0 The element is not currently enqueued
///
/// \retval 1 The element is currently enqueued

static inline int
pk_deque_is_queued(PkDeque* element)
{
    return (element->next != 0);
}


/// Append an element to the tail of a deque (FIFO order)
///
/// \param deque The sentinel node of a deque
///
/// \param element Typically the PkDeque object of a queable structure
///
/// It is an error to call this API on an element that is already enqueued,
/// but the API does not check for this error.

static inline void
pk_deque_push_back(PkDeque* deque, PkDeque* element)
{
    deque->previous->next = element;
    element->previous = deque->previous;
    element->next = deque;
    deque->previous = element;
}


/// Push an element at the head of a deque (LIFO order)
///
/// \param deque The sentinel node of a deque
///
/// \param element Typically the PkDeque object of a queable structure
///
/// It is an error to call this API on an element that is already enqueued,
/// but the API does not check for this error.

static inline void
pk_deque_push_front(PkDeque* deque, PkDeque* element)
{
    deque->next->previous = element;
    element->next = deque->next;
    element->previous = deque;
    deque->next = element;
}

/// Pop an element from the head of a deque
///
/// \param deque The sentinel node of a deque
///
/// \retval 0 The PkDeque was empty prior to the call
///
/// \retval non-0 A pointer to the previous head of the deque, which has been
/// removed from the deque and marked as no longer queued.

// The cast of 'head' is used to remove the 'volatile' attribute.

static inline PkDeque*
pk_deque_pop_front(PkDeque* deque)
{
    PkDeque* head;

    if (pk_deque_is_empty(deque))
    {
        return 0;
    }
    else
    {
        head = (PkDeque*)(deque->next);
        deque->next = head->next;
        deque->next->previous = deque;
        head->next = 0;
        return head;
    }
}


/// Remove a deque element from any position in the deque
///
/// \param element Typically the PkDeque object of a queable structure
///
/// It is an error to call this API on an element that is not currently
/// enqueued, but the API does not check for this error.

static inline void
pk_deque_delete(PkDeque* element)
{
    element->previous->next = element->next;
    element->next->previous = element->previous;
    element->next = 0;
}

// Bottom Half APIs

extern PkDeque _pk_bh_queue;

static inline void
pk_bh_schedule(PkBottomHalf* bottom_half)
{
    if(!pk_deque_is_queued((PkDeque*)bottom_half))
    {
        pk_deque_push_back(&_pk_bh_queue, (PkDeque*)bottom_half);
    }
}

#define PK_BH_INIT(_handler, _arg) \
    {\
        .deque = PK_DEQUE_ELEMENT_INIT(), \
                 .bh_handler = _handler, \
                               .arg = _arg \
    }

#define PK_BH_STATIC_CREATE(bh_name, handler, arg) \
    PkBottomHalf bh_name  __attribute__((section (".sdata"))) = \
            PK_BH_INIT(handler, arg)



//Trace function prototypes
void pk_trace_tiny(uint32_t i_parm);
void pk_trace_big(uint32_t i_hash_and_count,
                  uint64_t i_parm1, uint64_t i_parm2);
void pk_trace_binary(uint32_t i_hash_and_size, void* bufp);
void pk_trace_set_timebase(PkTimebase timebase);


/// Cast a pointer to another type, in a way that won't cause warnings

#define PK_CAST_POINTER(t, p) ((t)((PkAddress)(p)))

// Static Assert Macro for Compile time assertions.
//   - This macro can be used both inside and outside of a function.
//   - A value of false will cause the ASSERT to produce this error
//   - This will show up on a compile fail as:
//      <file>:<line> error: size of array '_static_assert' is negative
//   - It would be trivial to use the macro to paste a more descriptive
//     array name for each assert, but we will leave it like this for now.
#define PK_STATIC_ASSERT(cond) extern uint8_t _static_assert[(cond) ? 1 : -1]  __attribute__ ((unused))

/// \page pk_errors PK API and Kernel Error Handling
///
/// Error checking in the PK API consumes a significant amount of code space.
/// Approximately 20% of the object code in the PPC405 port is devoted to
/// error checking. Presumably a like amount of time overhead is also added to
/// PK API calls by this checking.
///
/// API error checking can be disabled to save space and time in the kernel.
/// API errors can also be configured to cause kernel panics, allowing
/// applications to be coded without the overhead of error checking but still
/// providing an escape in the event of application errors or (unlikely)
/// hardware failures. The PK default is to check for API errors and kernel
/// invariants, and panic should errors occur.
///
/// PK follows the Unix convention that a successful call of an API returns 0
/// (PK_OK), but returns a negative code in the event of failure, or to
/// provide further information. The error codes are all defined as manifest
/// constants.
///
/// Some negative codes returned by PK APIs are not considered errors.  These
/// conditions are always checked, never cause a panic if they occur, and
/// their interpretation is always left to the application.  See the detailed
/// documentation for each API for lists of error and non-error codes returned
/// by the API.
///
/// There are three configuration options that control error handling in the
/// PK API and kernel:
///
/// \c PK_ERROR_CHECK_API
///
///    \arg \b 0 - No PK API error checking.  APIs that potentially return error
///    codes will always return 0 (PK_OK) instead of an error code.  Those
///    APIs that return negative codes that are not errors (see Table 1.5)
///    always return the negative non-error codes when appropriate.
///
///    \arg \b 1 - (Default) All PK API errors are checked.  The behavior in
///    the event of an error is defined by the configuration option
///    PK_ERROR_PANIC.
///
/// \c PK_ERROR_CHECK_KERNEL
///
///   \arg \b 0 - No kernel invariant error checking is done.
///
///   \arg \b 1 - (Default) Selected kernel invariants are checked.  The overhead
///   for these checks should be  minimal.
///
/// \c PK_ERROR_PANIC
///
///  \arg \b 0 - PK API calls return negative error codes in the event of
///  errors. Note that PK kernel invariants always cause a panic if
///  violations occur.
///
///  \arg \b 1 - (Default) In the event of errors PK APIs invoke PK_PANIC(code),
///  where code is a positive error code.  Kernel invariant checks always
///  cause a panic if violations are detected.

#endif  /* __ASSEMBLER__ */

#endif  /* __PK_API_H__ */
