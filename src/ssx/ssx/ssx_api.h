/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/ssx/ssx_api.h $                                       */
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
#ifndef __SSX_API_H__
#define __SSX_API_H__

/// \file ssx_api.h
/// \brief Macros and declarations for the SSX API.

//  Basic constants

/// Although the number of threads is defined as a manifest constant,
/// numerous parts of the SSX code assume this definition. The number of
/// supported threads _can not_ be changed simply by changing this constant.

#define SSX_THREADS 32

#define SSX_IDLE_THREAD_PRIORITY SSX_THREADS

//  Interrupt API

#define SSX_NONCRITICAL   0
#define SSX_CRITICAL      1
#define SSX_SUPERCRITICAL 2

#define SSX_IRQ_POLARITY_ACTIVE_LOW  0
#define SSX_IRQ_POLARITY_ACTIVE_HIGH 1

#define SSX_IRQ_TRIGGER_LEVEL_SENSITIVE 0
#define SSX_IRQ_TRIGGER_EDGE_SENSITIVE  1

// API return codes

#define SSX_OK                                 0
#define SSX_ILLEGAL_CONTEXT_CRITICAL_INTERRUPT 0x00779001
#define SSX_ILLEGAL_CONTEXT_THREAD_CONTEXT     0x00779002
#define SSX_ILLEGAL_CONTEXT_INTERRUPT_CONTEXT  0x00779003
#define SSX_ILLEGAL_CONTEXT_THREAD             0x00779004
#define SSX_ILLEGAL_CONTEXT_TIMER              0x00779005
#define SSX_ILLEGAL_CONTEXT_PPC405_CACHE       0x00779006
#define SSX_INVALID_THREAD_AT_RESUME1          0x00779007
#define SSX_INVALID_THREAD_AT_RESUME2          0x00779008
#define SSX_INVALID_THREAD_AT_SUSPEND1         0x00779009
#define SSX_INVALID_THREAD_AT_SUSPEND2         0x0077900a
#define SSX_INVALID_THREAD_AT_DELETE           0x0077900b
#define SSX_INVALID_THREAD_AT_INFO             0x0077900c
#define SSX_INVALID_THREAD_AT_CHANGE           0x0077900d
#define SSX_INVALID_THREAD_AT_SWAP1            0x0077900e
#define SSX_INVALID_THREAD_AT_SWAP2            0x0077900f
#define SSX_INVALID_THREAD_AT_CREATE           0x00779010
#define SSX_INVALID_SEMAPHORE_AT_POST          0x00779011
#define SSX_INVALID_SEMAPHORE_AT_PEND          0x00779012
#define SSX_INVALID_SEMAPHORE_AT_RELEASE       0x00779013
#define SSX_INVALID_SEMAPHORE_AT_INFO          0x00779014
#define SSX_INVALID_SEMAPHORE_AT_CREATE        0x00779015
#define SSX_INVALID_TIMER_AT_SCHEDULE          0x00779016
#define SSX_INVALID_TIMER_AT_CANCEL            0x00779017
#define SSX_INVALID_TIMER_AT_INFO              0x00779018
#define SSX_INVALID_TIMER_AT_CREATE            0x00779019
#define SSX_INVALID_ARGUMENT_IRQ_SETUP         0x0077901a
#define SSX_INVALID_ARGUMENT_IRQ_HANDLER       0x0077901b
#define SSX_INVALID_ARGUMENT_INTERRUPT         0x00779024
#define SSX_INVALID_ARGUMENT_CONTEXT_SET       0x00779025
#define SSX_INVALID_ARGUMENT_CONTEXT_GET       0x00779026
#define SSX_INVALID_ARGUMENT_PPC405_FIT        0x00779027
#define SSX_INVALID_ARGUMENT_PPC405_WATCHDOG   0x00779028
#define SSX_INVALID_ARGUMENT_INIT              0x00779029
#define SSX_INVALID_ARGUMENT_SEMAPHORE         0x0077902a
#define SSX_INVALID_ARGUMENT_THREAD_CHANGE     0x0077902b
#define SSX_INVALID_ARGUMENT_THREAD_PRIORITY   0x0077902c
#define SSX_INVALID_ARGUMENT_THREAD1           0x0077902d
#define SSX_INVALID_ARGUMENT_THREAD2           0x0077902e
#define SSX_INVALID_ARGUMENT_THREAD3           0x0077902f
#define SSX_STACK_OVERFLOW                     0x00779030
#define SSX_TIMER_ACTIVE                       0x00779031
#define SSX_TIMER_NOT_ACTIVE                   0x00779032
#define SSX_PRIORITY_IN_USE_AT_RESUME          0x00779033
#define SSX_PRIORITY_IN_USE_AT_CHANGE          0x00779034
#define SSX_PRIORITY_IN_USE_AT_SWAP            0x00779035
#define SSX_SEMAPHORE_OVERFLOW                 0x00779036
#define SSX_SEMAPHORE_PEND_NO_WAIT             0x00779037
#define SSX_SEMAPHORE_PEND_TIMED_OUT           0x00779038
#define SSX_SEMAPHORE_PEND_WOULD_BLOCK         0x00779039
#define SSX_INVALID_DEQUE_SENTINEL             0x0077903a
#define SSX_INVALID_DEQUE_ELEMENT              0x0077903b
#define SSX_INVALID_OBJECT                     0x0077903c

// Kernel panics

#define SSX_NO_TIMER_SUPPORT                   0x0077903d
#define SSX_START_THREADS_RETURNED             0x0077903e
#define SSX_UNIMPLEMENTED                      0x0077903f
#define SSX_SCHEDULING_INVARIANT               0x00779040
#define SSX_TIMER_HANDLER_INVARIANT            0x00779041
#define SSX_THREAD_TIMEOUT_STATE               0x00779045


/// \defgroup ssx_thread_states SSX Thread States
///
/// Threads are created in the state SSX_THREAD_STATE_SUSPENDED_RUNNABLE.
/// When the thread is mapped it transitions to state SSX_THREAD_STATE_MAPPED.
/// A mapped thread is runnable if it appears in the run queue; there is no
/// other flag or status to indicate a runnable thread.  If a blocked thread
/// is suspended it goes into state SSX_THREAD_STATE_SUSPENDED_BLOCKED.  For
/// all threads the reason for blockage is detailed in the \a flags field of
/// the thread; See \ref ssx_thread_flags.  SSX_THREAD_STATE_DELETED and
/// SSX_THREAD_STATE_COMPLETED are effectively equivalent but named
/// individually for reporting purposes.
///
/// \note This separation of the thread \a state and \a flags allows the use
/// of an SSX semaphore as a thread barrier, as it supports a non-iterative
/// implementation of ssx_semaphore_release_all() in which all threads blocked
/// on the semaphore are simultaneously inserted into the run queue with an
/// atomic operation, followed by each individual thread readjusting its flags
/// appropriately once the thread runs again.
///
/// @{

#define SSX_THREAD_STATE_SUSPENDED_RUNNABLE 1
#define SSX_THREAD_STATE_MAPPED             2
#define SSX_THREAD_STATE_SUSPENDED_BLOCKED  3
#define SSX_THREAD_STATE_COMPLETED          4
#define SSX_THREAD_STATE_DELETED            5

/// @}


/// \defgroup ssx_thread_flags SSX Thread Flags
///
/// The \a flag field of the thread extends the information contained in the
/// \a state field; See \ref ssx_thread_states.  Blocked threads will show
/// SSX_THREAD_FLAG_SEMAPHORE_PEND, SSX_THREAD_FLAG_TIMER_PEND or both (if
/// blocked on a semaphore with timeout). The flag SSX_THREAD_FLAG_TIMED_OUT
/// indicates that a thread timer timed out before the thread became
/// runnable.  Currently only the semaphore-pend-with-timeout code uses this
/// flag.
///
/// Note that a thread can be mapped and runnable (in the run queue) even
/// though SSX_THREAD_FLAG_SEMAPHORE_PEND and/or SSX_THREAD_FLAG_TIMER_PEND
/// are set. These flags are always cleared by the thread itself, not the code
/// that unblocks the thread. This allows the implementation of the
/// ssx_semaphore_release_all() as explained in \ref ssx_thread_states.
///
/// @{

#define SSX_THREAD_FLAG_SEMAPHORE_PEND 0x1
#define SSX_THREAD_FLAG_TIMER_PEND     0x2
#define SSX_THREAD_FLAG_TIMED_OUT      0x4

/// @}


//  Critical Sections

/// Enter a critical section of a given priority, saving the current machine
/// context.

#define ssx_critical_section_enter(priority, pctx) \
    ssx_interrupt_disable(priority, pctx)

/// Exit a critical section by restoring the previous machine context.

#define ssx_critical_section_exit(pctx) \
    ssx_machine_context_set(pctx)


/// Execute a statement atomically, in a particular interrupt priority
/// context.

#define SSX_ATOMIC(priority, stmt) \
    do { \
        SsxMachineContext __ctx; \
        ssx_critical_section_enter((priority), &__ctx); \
        stmt; \
        ssx_critical_section_exit(&__ctx); \
    } while (0)


// Application-overrideable definitions

/// Control whether or not the API functions check for errors.
///
/// This definition can be overriden by the application.

#ifndef SSX_ERROR_CHECK_API
    #define SSX_ERROR_CHECK_API 1
#endif

/// Control whether API errors cause kernel panics or return negative error
/// codes.
///
/// This selection is only valid if \c SSX_ERROR_CHECK_API is defined
/// non-0. This definition can be overriden by the application.

#ifndef SSX_ERROR_PANIC
    #define SSX_ERROR_PANIC 1
#endif

/// Control whether or not the SSX kernel checks key invariants.
///
/// Violations of kernel invariants always cause kernel panics.  This
/// definition can be overriden by the application.

#ifndef SSX_ERROR_CHECK_KERNEL
    #define SSX_ERROR_CHECK_KERNEL 1
#endif

/// Define the time interval type, which must be an unsigned type of a size
/// less then or equal to the size of \c SsxTimebase.  This definition can be
/// overridden by the application.

#ifndef SSX_TIME_INTERVAL_TYPE
    #define SSX_TIME_INTERVAL_TYPE uint64_t
#endif

/// Provide support for the SsxTimer APIs in addition to the default
/// initerrupt APIs.  This definition can be overridden by the application.

#ifndef SSX_TIMER_SUPPORT
    #define SSX_TIMER_SUPPORT 1
#endif

/// Provide support for the all SSX APIs. Thread support requires/implies
/// support for time services and semaphores.  This definition can be
/// overridden by the application.

#ifndef SSX_THREAD_SUPPORT
    #define SSX_THREAD_SUPPORT 1
#endif

/// Control the level of stack checking.
///
/// This definition can be overriden by the application.
///
/// 0 : No stack prepatterning or checking is made for thread and kernel
/// stacks.
///
/// 1 : Kernel interrupt stacks are prepatterned during
/// \c ssx_initialize(). Thread stacks are prepatterned during
/// \c ssx_thread_create().
///
/// 2 : (\b Default - Currently Unimplemented) In addition to prepatterning,
/// stack utilization is computed at the exit of context switches and
/// noncritical interrupt processing.  The maximum utilization is stored in
/// the thread data structure.  The kernel will panic if stack overflow is
/// detected. Stack utilization is not computed for the idle thread.

#ifndef SSX_STACK_CHECK
    #define SSX_STACK_CHECK 1
#endif

/// A hook for main()
///
/// This hook macro is expanded in the body of __ssx_main() prior to the call
/// of the application main().  The application can redefine this hook macro
/// in (or in headers referred to in) the application header
/// ssx_app_cfg.h. The SSX_MAIN_HOOK will run on the stack of main().

#ifndef SSX_MAIN_HOOK
    #define SSX_MAIN_HOOK do {} while (0)
#endif

/// A hook for ssx_start_threads()
///
/// This hook macro is expanded in the call-tree of ssx_start_threads() before
/// threads are actually started.  The application can redefine this hook
/// macro in (or in headers referred to in) the application header
/// ssx_app_cfg.h.
///
/// The SSX_START_THREADS_HOOK runs as a pseudo-interrupt handler on the
/// noncritical interrupt stack, with noncritical interrupts disabled.

#ifndef SSX_START_THREADS_HOOK
    #define SSX_START_THREADS_HOOK do {} while (0)
#endif

/// The maximum value of the \c SsxTimebase type.

#define SSX_TIMEBASE_MAX ((SsxTimebase)-1)

/// A special value that specifies that the timebase will not be reset during
/// ssx_init().

#define SSX_TIMEBASE_CONTINUES SSX_TIMEBASE_MAX

/// By convention, a timeout value indicating 'no waiting' in a call of \c
/// ssx_semaphore_pend().

#define SSX_NO_WAIT 0

/// By convention, a timeout value indicating 'wait forever' in a call of \c
/// ssx_semaphore_pend().

#define SSX_WAIT_FOREVER ((SsxInterval)-1)

/// The SSX timebase frequency in Hz
///
/// Earlier version of SSX defined the timbase frequency as a preprocessor
/// macro. Now, the timebase frequency is specified as a parameter of the
/// ssx_initialize() API. The macro remains defined for backwards
/// compatibility, however all kernel uses of the timebase frequency are now
/// optimized around the timebase parameter.

#define SSX_TIMEBASE_FREQUENCY_HZ __ssx_timebase_frequency_hz

/// Convert a time in integral seconds to a time interval - overflows are
/// ignored. The application can redefine this macro.

#ifndef SSX_SECONDS
    #define SSX_SECONDS(s) ((SsxInterval)(__ssx_timebase_frequency_hz * (SsxInterval)(s)))
#endif

/// Convert a time in integral milliseconds to a time interval - overflows are
/// ignored, and a frequency evenly (or closely) divisible by 1000 is
/// assumed. The application can redefine this macro.

#ifndef SSX_MILLISECONDS
    #define SSX_MILLISECONDS(m) ((SsxInterval)(__ssx_timebase_frequency_khz * (SsxInterval)(m)))
#endif

/// Convert a time in integral microseconds to a time interval - overflows are
/// ignored, and a frequncy evenly (or closely) divisible by 1,000,000 is
/// assumed. The application can redefine this macro.

#ifndef SSX_MICROSECONDS
    #define SSX_MICROSECONDS(u) ((SsxInterval)(__ssx_timebase_frequency_mhz * (SsxInterval)(u)))
#endif

/// Convert a time in integral nanoseconds to a time interval - overflows are
/// ignored, and a frequeyncy evenly (or closely) divisible by 1,000,000 is
/// assumed. The application can redefine this macro.

#ifndef SSX_NANOSECONDS
#define SSX_NANOSECONDS(n) \
    ((SsxInterval)((__ssx_timebase_frequency_mhz  * (SsxInterval)(n)) / 1000))
#endif


/// Enable SSX application tracing (enabled by default)
#ifndef SSX_TRACE_ENABLE
    #define SSX_TRACE_ENABLE 1
#endif

/// Enable SSX kernel tracing (disabled by default)
#ifndef SSX_KERNEL_TRACE_ENABLE
    #define SSX_KERNEL_TRACE_ENABLE 0
#endif

#if !SSX_TRACE_ENABLE
    #define SSX_TRACE(...)
    #define SSX_TRACE_BIN(str, bufp, buf_size)
#else
    #define SSX_TRACE(...) SSXTRACE(__VA_ARGS__)
    #define SSX_TRACE_BIN(str, bufp, buf_size) SSXTRACE_BIN(str, bufp, buf_size)
#endif

//Kernel trace macros
#if !SSX_KERNEL_TRACE_ENABLE
    #define SSX_KERN_TRACE(...)
    #define SSX_KERN_TRACE_ASM16(...)
#else
    #define SSX_KERN_TRACE(...) SSX_TRACE(__VA_ARGS__)
    #define SSX_KERN_TRACE_ASM16(...) SSX_TRACE_ASM16(__VA_ARGS__)
#endif  /* SSX_KERNEL_TRACE_ENABLE */


/// Add a string to the trace buffer with an optional register holding a 16bit value
/// WARNING: This calls a c function which may clobber any of the volatile registers
#if (SSX_TRACE_SUPPORT && SSX_TIMER_SUPPORT)
    #define SSX_TRACE_ASM16(...) TRACE_ASM_HELPER16(VARG_COUNT(__VA_ARGS__), __VA_ARGS__)
#else
    #define SSX_TRACE_ASM16(...)
#endif /* SSX_TRACE_SUPPORT */

/// The following macros are helper macros for tracing.  They should not be called
/// directly.
#define VARG_COUNT_HELPER(_0, _1, _2, _3, _4, _5, _6, _7, N, ...) N
#define VARG_COUNT(...) VARG_COUNT_HELPER(, ##__VA_ARGS__, 7, 6, 5, 4, 3, 2, 1, 0)

#ifdef __ASSEMBLER__
#define TRACE_ASM_HELPER16_CALL(count, ...) TINY_TRACE_ASM ## count (__VA_ARGS__)
#define TRACE_ASM_HELPER16(count, ...) TRACE_ASM_HELPER16_CALL(count, __VA_ARGS__)

#define TINY_TRACE_ASM0()   .error "format string required"
#define TINY_TRACE_ASM1(str) \
    .tiny_trace_asm1 trace_ppe_hash(str, SSX_TRACE_HASH_PREFIX)
#define TINY_TRACE_ASM2(str, reg) \
    .tiny_trace_asm2 trace_ppe_hash(str, SSX_TRACE_HASH_PREFIX), reg
#define TINY_TRACE_ASM3()   .error "too many parameters"
#define TINY_TRACE_ASM4()   .error "too many parameters"
#define TINY_TRACE_ASM5()   .error "too many parameters"
#define TINY_TRACE_ASM6()   .error "too many parameters"
#define TINY_TRACE_ASM7()   .error "too many parameters"

//TODO: add support for tracing more than 1 parameter and binary data in assembly
// *INDENT-OFF*

    .global ssx_trace_tiny

    .macro  .tiny_trace_asm1 hash16
        lis     %r3, \hash16
        bl      ssx_trace_tiny
    .endm

    .macro .tiny_trace_asm2 hash16, parm16
        clrlwi  %r3, \parm16, 16
        oris    %r3, %r3, \hash16
        bl      ssx_trace_tiny
    .endm

// *INDENT-ON*        
#endif /*__ASSEMBLER__*/



#ifndef __ASSEMBLER__

#include <stddef.h>
#include <stdint.h>

/// The timebase frequency in Hz; A parameter to ssx_initialize()
extern uint32_t __ssx_timebase_frequency_hz;

/// The timebase frequency in KHz
extern uint32_t __ssx_timebase_frequency_khz;

/// The timebase frequency in Mhz
extern uint32_t __ssx_timebase_frequency_mhz;


typedef unsigned long int SsxAddress;

typedef uint8_t SsxThreadState;

typedef uint8_t SsxThreadPriority;

typedef uint8_t SsxThreadFlags;

typedef uint32_t SsxSemaphoreCount;

typedef uint64_t SsxTimebase;

typedef SSX_TIME_INTERVAL_TYPE SsxInterval;

#include "ssx_port_types.h"

typedef struct
{

    /// A priority queue of threads pending on the semaphore.
    SsxThreadQueue pending_threads;

    /// The current semaphore count.
    SsxSemaphoreCount count;

    /// The maximum allowable count - for error checking.
    SsxSemaphoreCount max_count;

} SsxSemaphore;


/// Compile-time initialize an SsxSemaphore structure
///
/// This low-level macro creates a structure initializatin of an SsxSemaphore
/// structure. This can be used for example to create compile-time initialized
/// arrays of semaphores.
#define SSX_SEMAPHORE_INITIALIZATION(_initial_count, _max_count)        \
    {.pending_threads = 0,                                              \
                        .count = (_initial_count),                                  \
                                 .max_count = (_max_count)}


/// Declare and initialize a semaphore
#define SSX_SEMAPHORE(sem, initial_count, max_count)                    \
    SsxSemaphore sem = SSX_SEMAPHORE_INITIALIZATION(initial_count, max_count)

/// Trace macros for C functions
#define HASH_ARG_COMBO(str, arg) \
    ((((uint32_t)trace_ppe_hash(str, SSX_TRACE_HASH_PREFIX)) << 16) | ((uint32_t)(arg) & 0x0000ffff))

#define SSXTRACE0(...) ssx_trace_tiny() //will fail at compile time

#define SSXTRACE1(str) \
    ssx_trace_tiny((trace_ppe_hash(str, SSX_TRACE_HASH_PREFIX) << 16))

#define SSXTRACE2(str, parm0) \
    ((sizeof(parm0) <= 2)? \
     ssx_trace_tiny(HASH_ARG_COMBO(str, parm0)): \
     ssx_trace_big(HASH_ARG_COMBO(str, 1), ((uint64_t)parm0) << 32, 0))

#define SSXTRACE3(str, parm0, parm1) \
    ssx_trace_big(HASH_ARG_COMBO(str, 2), ((((uint64_t)parm0) << 32) | parm1), 0)

#define SSXTRACE4(str, parm0, parm1, parm2) \
    ssx_trace_big(HASH_ARG_COMBO(str, 3), ((((uint64_t)parm0) << 32) | parm1),\
                  ((uint64_t)parm2) << 32 )

#define SSXTRACE5(str, parm0, parm1, parm2, parm3) \
    ssx_trace_big(HASH_ARG_COMBO(str, 4), ((((uint64_t)parm0) << 32) | parm1),\
                  ((((uint64_t)parm2) << 32) | parm3) )

#define SSXTRACE6(...) ssx_trace_tiny() //will fail at compile time
#define SSXTRACE7(...) ssx_trace_tiny() //will fail at compile time

#define SSXTRACE_HELPER2(count, ...) SSXTRACE ## count (__VA_ARGS__)
#define SSXTRACE_HELPER(count, ...) SSXTRACE_HELPER2(count, __VA_ARGS__)

#if (SSX_TRACE_SUPPORT && SSX_TIMER_SUPPORT)
#define SSXTRACE(...) SSXTRACE_HELPER(VARG_COUNT(__VA_ARGS__), __VA_ARGS__)
#define SSXTRACE_BIN(str, bufp, buf_size) \
    ssx_trace_binary(((buf_size < 255)? HASH_ARG_COMBO(str, buf_size): HASH_ARG_COMBO(str, 255)), bufp)
#else
#define SSXTRACE(...)
#define SSXTRACE_BIN(str, bufp, buf_size)
#endif //SSX_TRACE_SUPPORT

//Needed for easy cache flush of trace buffer
//Note, in order to use this macro you must declare g_ssx_trace_buf[_size]
//  as extern SsxTraceBuffer [and size_t] variables in the .c file as well
//  as include ssx_trace.h.
#if (SSX_TRACE_ENABLE && SSX_TRACE_SUPPORT && SSX_TIMER_SUPPORT)
    #define SSX_FLUSH_TRACE_BUF() dcache_flush(&g_ssx_trace_buf, g_ssx_trace_buf_size)
#else
    #define SSX_FLUSH_TRACE_BUF()
#endif

//Making sure we can still compile application codes if SSX tracing for some
//  reason isn't enabled.
#if (SSX_TRACE_ENABLE && SSX_TRACE_SUPPORT && SSX_TIMER_SUPPORT)
    #define SSX_TRACE_INIT(freqhz, time0) ssx_trace_init(freqhz, time0)
#else
    #define SSX_TRACE_INIT(freqhz, time0)
#endif

/// A generic doubly-linked list object
///
/// This object functions both as a sentinel mode for a deque as well as a
/// pointer container for elements in deques.  The SSX API assumes that
/// queueable structures will be defined with an SsxDeque structure as the
/// initial 'data member' of the structure.  This allows a pointer to a queue
/// element to be cast to a pointer to an SsxDeque and vice-versa.

typedef struct SsxDeque
{

    /// Pointer to the head or the next element in a deque.
    ///
    /// When an SsxDeque is used as the sentinel node for a queue, \a next
    /// points to the head of the queue, and the condition (next == \<self\>)
    /// indicates an empty SsxDeque. By convention the condition (\a next ==
    /// 0) is used to indicate that a queue element is not enqueued.
    struct SsxDeque* next;

    /// Pointer to the tail or previous element in a deque.
    ///
    /// When a DQueue is used as the sentinel node for a queue, \a previous
    /// points to the tail of the queue.
    struct SsxDeque* previous;

} SsxDeque;


typedef void (*SsxTimerCallback)(void*);

#define SSX_TIMER_CALLBACK(callback) void callback(void *)

struct SsxTimer;

/// The SSX timer object

typedef struct SsxTimer
{

    /// The time queue management pointers
    ///
    /// This pointer container is defined as the first element of the
    /// structure to allow the SsxTimer to be cast to an SsxDeque and
    /// vice-versa.
    SsxDeque deque;

    /// The absolute timeout of the timer.
    SsxTimebase timeout;

    /// The timer period
    ///
    /// If not 0, then this is a periodic timer and it will be automatically
    /// rescheduled in absolute time from the previous timeout.
    SsxInterval period;

    /// The timer callback
    ///
    /// For SSX thread timers used to implement Sleep and semaphore pend
    /// timeouts this field is initialized to __ssx_thread_timeout().
    SsxTimerCallback callback;

    /// Private data passed to the callback.
    ///
    /// For SSX thread timers used to implement Sleep and semaphore pend this
    /// field is initialized to a pointer to the thread.
    void* arg;

    /// Options for timer processing; See \ref ssx_timer_options
    uint8_t options;

} SsxTimer;

/// \defgroup ssx_timer_options SSX Timer Options
/// @{

/// Allow interrupt preemption during the callback
///
/// This is the normal mode for SsxTimer objects scheduled by SSX kernal
/// mechanisms.  The timer callbacks effectively run as if inside a
/// highest-priority thread, allowing other interrupts to preempt them.
#define SSX_TIMER_CALLBACK_PREEMPTIBLE 0x1

/// @}


// Threads

typedef void (*SsxThreadRoutine)(void* arg);

#define SSX_THREAD_ROUTINE(f) void f(void *arg);

typedef struct
{

    /// Stack pointer saved during context switches.  Assembler code expects
    /// this to always be at address offset 0 from the thread pointer.
    SsxAddress saved_stack_pointer;

    /// This is 1 past the last valid byte address of the thread stack.
    /// Assembler code expects this to always be at address offset (sizeof
    /// SsxAddress) from the thread pointer.
    SsxAddress stack_limit;

    /// This is the original base of the stack.
    /// Assembler code expects this to always be at address offset 2 * (sizeof
    /// SsxAddress) from the thread pointer.
    SsxAddress stack_base;

    /// If the thread is blocked on a semaphore, then this is the semaphore the
    /// thread is blocked on.
    SsxSemaphore* semaphore;

    /// The thread priority.
    SsxThreadPriority priority;

    /// The thread state; See \ref ssx_thread_states
    SsxThreadState state;

    /// Thread flags; See \ref ssx_thread_flags
    SsxThreadFlags flags;

    /// The timer structure handles Sleep and blocking on a semaphore with
    /// timeout.
    SsxTimer timer;

} SsxThread;


// Initialization APIs

int
ssx_initialize(SsxAddress  noncritical_stack,
               size_t      noncritical_stack_size,
               SsxAddress  critical_stack,
               size_t      critical_stack_size,
               SsxTimebase initial_timebase,
               uint32_t    timebase_frequency_hz);


// Timebase APIs

SsxTimebase
ssx_timebase_get(void);

#if APPCFG_USE_EXT_TIMEBASE_FOR_TRACE
// Retrieve an external timebase
SsxTimebase
ssx_ext_timebase_get(void);
#else
static inline SsxTimebase
ssx_ext_timebase_get(void)
{
    return ssx_timebase_get();
}
#endif /* APPCFG_USE_EXT_TIMEBASE_FOR_TRACE */

void
ssx_timebase_set(SsxTimebase timebase);

// Interrupt preemption APIs

int
ssx_interrupt_preemption_enable(void);

int
ssx_interrupt_preemption_disable(void);

// Timer APIs

int
ssx_timer_create(SsxTimer*         timer,
                 SsxTimerCallback callback,
                 void*             arg);

int
ssx_timer_create_nonpreemptible(SsxTimer*         timer,
                                SsxTimerCallback callback,
                                void*             arg);

int
ssx_timer_schedule_absolute(SsxTimer*    timer,
                            SsxTimebase time,
                            SsxInterval period);

int
ssx_timer_schedule(SsxTimer*    timer,
                   SsxInterval interval,
                   SsxInterval period);

int
ssx_timer_cancel(SsxTimer* timer);

int
ssx_timer_info_get(SsxTimer*    timer,
                   SsxTimebase* timeout,
                   int*         active);

// Thread APIs

int
ssx_thread_create(SsxThread*         thread,
                  SsxThreadRoutine  thread_routine,
                  void*              arg,
                  SsxAddress        stack,
                  size_t            stack_size,
                  SsxThreadPriority priority);

int
ssx_start_threads(void);

int
ssx_thread_resume(SsxThread* thread);

int
ssx_thread_suspend(SsxThread* thread);

int
ssx_thread_delete(SsxThread* thread);

int
ssx_complete(void);

int
ssx_sleep_absolute(SsxTimebase time);

int
ssx_sleep(SsxInterval interval);

int
ssx_thread_info_get(SsxThread*         thread,
                    SsxThreadState*    state,
                    SsxThreadPriority* priority,
                    int*               runnable);

int
ssx_thread_priority_change(SsxThread*         thread,
                           SsxThreadPriority new_priority,
                           SsxThreadPriority* old_priority);

int
ssx_thread_at_priority(SsxThreadPriority priority,
                       SsxThread**         thread);

int
ssx_thread_priority_swap(SsxThread* thread_a, SsxThread* thread_b);


// Semaphore APIs

int
ssx_semaphore_create(SsxSemaphore*      semaphore,
                     SsxSemaphoreCount initial_count,
                     SsxSemaphoreCount max_count);

int
ssx_semaphore_post(SsxSemaphore* semaphore);

int
ssx_semaphore_pend(SsxSemaphore* semaphore,
                   SsxInterval  timeout);

int
ssx_semaphore_release_all(SsxSemaphore* semaphore);


int
ssx_semaphore_info_get(SsxSemaphore*      semaphore,
                       SsxSemaphoreCount* count,
                       int*               pending);

void
ssx_semaphore_post_handler(void* arg,
                           SsxIrqId irq,
                           int priority);

// Misc. APIs

void
ssx_halt() __attribute__ ((noreturn));

// Deque APIs

int
ssx_deque_sentinel_create(SsxDeque* deque);

#define SSX_DEQUE_SENTINEL_INIT(dq_addr) \
    {\
        .next = dq_addr, \
                .previous = dq_addr \
    }

#define SSX_DEQUE_SENTINEL_STATIC_CREATE(deque) \
    SsxDeque deque = SSX_DEQUE_SENTINEL_INIT(&deque)

int
ssx_deque_element_create(SsxDeque* element);

#define SSX_DEQUE_ELEMENT_INIT() \
    {\
        .next = 0, \
                .previous = 0 \
    }

#define SSX_DEQUE_ELEMENT_STATIC_CREATE(deque) \
    SsxDeque deque = SSX_DEQUE_ELEMENT_INIT()


/// Check for an empty SsxDeque
///
/// \param deque The sentinel node of a deque
///
/// \retval 0 The SsxDeque is not empty
///
/// \retval 1 The SsxDeque is empty

static inline int
ssx_deque_is_empty(SsxDeque* deque)
{
    return (deque == deque->next);
}


/// Check if an SsxDeque element is currently enqueued
///
/// \param element Typically the SsxDeque object of a queable structure
///
/// \retval 0 The element is not currently enqueued
///
/// \retval 1 The element is currently enqueued

static inline int
ssx_deque_is_queued(SsxDeque* element)
{
    return (element->next != 0);
}


/// Append an element to the tail of a deque (FIFO order)
///
/// \param deque The sentinel node of a deque
///
/// \param element Typically the SsxDeque object of a queable structure
///
/// It is an error to call this API on an element that is already enqueued,
/// but the API does not check for this error.

static inline void
ssx_deque_push_back(SsxDeque* deque, SsxDeque* element)
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
/// \param element Typically the SsxDeque object of a queable structure
///
/// It is an error to call this API on an element that is already enqueued,
/// but the API does not check for this error.

static inline void
ssx_deque_push_front(SsxDeque* deque, SsxDeque* element)
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
/// \retval 0 The SsxDeque was empty prior to the call
///
/// \retval non-0 A pointer to the previous head of the deque, which has been
/// removed from the deque and marked as no longer queued.

// The cast of 'head' is used to remove the 'volatile' attribute.

static inline SsxDeque*
ssx_deque_pop_front(SsxDeque* deque)
{
    SsxDeque* head;

    if (ssx_deque_is_empty(deque))
    {
        return 0;
    }
    else
    {
        head = (SsxDeque*)(deque->next);
        deque->next = head->next;
        deque->next->previous = deque;
        head->next = 0;
        return head;
    }
}


/// Remove a deque element from any position in the deque
///
/// \param element Typically the SsxDeque object of a queable structure
///
/// It is an error to call this API on an element that is not currently
/// enqueued, but the API does not check for this error.

static inline void
ssx_deque_delete(SsxDeque* element)
{
    element->previous->next = element->next;
    element->next->previous = element->previous;
    element->next = 0;
}


//Trace function prototypes
void ssx_trace_tiny(uint32_t i_parm);
void ssx_trace_big(uint32_t i_hash_and_count,
                   uint64_t i_parm1, uint64_t i_parm2);
void ssx_trace_binary(uint32_t i_hash_and_size, void* bufp);
void ssx_trace_set_timebase(SsxTimebase timebase);
void ssx_trace_init(uint32_t timebase_frequency_hz,
                    SsxTimebase initial_timebase);



/// Cast a pointer to another type, in a way that won't cause warnings

#define SSX_CAST_POINTER(t, p) ((t)((SsxAddress)(p)))

// Static Assert Macro for Compile time assertions.
//   - This macro can be used both inside and outside of a function.
//   - A value of false will cause the ASSERT to produce this error
//   - This will show up on a compile fail as:
//      <file>:<line> error: size of array '_static_assert' is negative
//   - It would be trivial to use the macro to paste a more descriptive
//     array name for each assert, but we will leave it like this for now.
#define SSX_STATIC_ASSERT(cond) extern uint8_t _static_assert[(cond) ? 1 : -1]  __attribute__ ((unused))

/// \page ssx_errors SSX API and Kernel Error Handling
///
/// Error checking in the SSX API consumes a significant amount of code space.
/// Approximately 20% of the object code in the PPC405 port is devoted to
/// error checking. Presumably a like amount of time overhead is also added to
/// SSX API calls by this checking.
///
/// API error checking can be disabled to save space and time in the kernel.
/// API errors can also be configured to cause kernel panics, allowing
/// applications to be coded without the overhead of error checking but still
/// providing an escape in the event of application errors or (unlikely)
/// hardware failures. The SSX default is to check for API errors and kernel
/// invariants, and panic should errors occur.
///
/// SSX follows the Unix convention that a successful call of an API returns 0
/// (SSX_OK), but returns a negative code in the event of failure, or to
/// provide further information. The error codes are all defined as manifest
/// constants.
///
/// Some negative codes returned by SSX APIs are not considered errors.  These
/// conditions are always checked, never cause a panic if they occur, and
/// their interpretation is always left to the application.  See the detailed
/// documentation for each API for lists of error and non-error codes returned
/// by the API.
///
/// There are three configuration options that control error handling in the
/// SSX API and kernel:
///
/// \c SSX_ERROR_CHECK_API
///
///    \arg \b 0 - No SSX API error checking.  APIs that potentially return error
///    codes will always return 0 (SSX_OK) instead of an error code.  Those
///    APIs that return negative codes that are not errors (see Table 1.5)
///    always return the negative non-error codes when appropriate.
///
///    \arg \b 1 - (Default) All SSX API errors are checked.  The behavior in
///    the event of an error is defined by the configuration option
///    SSX_ERROR_PANIC.
///
/// \c SSX_ERROR_CHECK_KERNEL
///
///   \arg \b 0 - No kernel invariant error checking is done.
///
///   \arg \b 1 - (Default) Selected kernel invariants are checked.  The overhead
///   for these checks should be  minimal.
///
/// \c SSX_ERROR_PANIC
///
///  \arg \b 0 - SSX API calls return negative error codes in the event of
///  errors. Note that SSX kernel invariants always cause a panic if
///  violations occur.
///
///  \arg \b 1 - (Default) In the event of errors SSX APIs invoke SSX_PANIC(code),
///  where code is a positive error code.  Kernel invariant checks always
///  cause a panic if violations are detected.

#endif  /* __ASSEMBLER__ */

#endif  /* __SSX_API_H__ */
