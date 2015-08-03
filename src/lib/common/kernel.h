/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/common/kernel.h $                                     */
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
#ifndef __KERNEL_H__
#define __KERNEL_H__
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2015
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file kernel.h
/// \brief Kernel agnostic macros that allow the same code to work with
///        different kernels.
///
///        Programmers can include this instead of pk.h or ssx.h
///       
///
#ifndef __ASSEMBLER__
#ifdef __SSX__

/// ----------------------- Use SSX kernel interfaces --------------------------

#include "ssx.h"

//Types
#define KERN_SEMAPHORE              SsxSemaphore
#define KERN_SEMAPHORE_COUNT        SsxSemaphoreCount
#define KERN_DEQUE                  SsxDeque
#define KERN_THREAD                 SsxThread
#define KERN_THREAD_PRIORITY        SsxThreadPriority
#define KERN_THREAD_STATE           SsxThreadState
#define KERN_THREAD_ROUTINE         SsxThreadRoutine
#define KERN_THREAD_FLAGS           SsxThreadFlags
#define KERN_ADDRESS                SsxAddress
#define KERN_TIMER                  SsxTimer
#define KERN_TIMER_CALLBACK         SsxTimerCallback
#define KERN_INTERVAL               SsxInterval
#define KERN_TIMEBASE               SsxTimebase
#define KERN_IRQ_ID                 SsxIrqId
#define KERN_MACHINE_CONTEXT        SsxMachineContext

//Constants
#define KERN_SEMAPHORE_PEND_TIMED_OUT   SSX_SEMAPHORE_PEND_TIMED_OUT
#define KERN_SEMAPHORE_PEND_NO_WAIT     SSX_SEMAPHORE_PEND_NO_WAIT
#define KERN_NONCRITICAL                SSX_NONCRITICAL
#define KERN_CRITICAL                   SSX_CRITICAL
#define KERN_SUPERCRITICAL              SSX_SUPERCRITICAL
#define KERN_ERROR_CHECK_API            SSX_ERROR_CHECK_API
#define KERN_NO_WAIT                    SSX_NO_WAIT
#define KERN_WAIT_FOREVER               SSX_WAIT_FOREVER

//Functions
#define KERN_SECONDS(s)         SSX_SECONDS(s)
#define KERN_MILLISECONDS(m)    SSX_MILLISECONDS(m) 
#define KERN_MICROSECONDS(u)    SSX_MICROSECONDS(u)
#define KERN_NANOSECONDS(n)     SSX_NANOSECONDS(n) 

#define KERN_TIMEBASE_GET()     ssx_timebase_get()
#define KERN_TIMEBASE_SET(tb)   ssx_timebase_set(tb)

#define KERN_INTERRUPT_PREEMPTION_ENABLE()  ssx_interrupt_preemption_enable()
#define KERN_INTERRUPT_PREEMPTION_DISABLE() ssx_interrupt_preemption_disable()

#define KERN_TIMER_CREATE(timer, callback, arg) \
    ssx_timer_create(timer, callback, arg)
#define KERN_TIMER_CREATE_NONPREEMPTIBLE(timer, callback, arg)  \
    ssx_timer_create_nonpreemptible(timer, callback, arg)
#define KERN_TIMER_SCHEDULE_ABSOLUTE(timer, time, period) \
    ssx_timer_schedule_absolute(timer, time, period)
#define KERN_TIMER_SCHEDULE(timer, interval, period) \
    ssx_timer_schedule(timer, interval, period)
#define KERN_TIMER_CANCEL(timer) \
    ssx_timer_cancel(timer)
#define KERN_TIMER_INFO_GET(timer, timeout, active) \
    ssx_timer_info_get(timer, timeout, active)

#define KERN_THREAD_CREATE(thread, thread_routine, arg, stack, stack_size, priority) \
    ssx_thread_create(thread, thread_routine, arg, stack, stack_size, priority)
#define KERN_THREAD_INFO_GET(thread, state, priority, runnable) \
    ssx_thread_info_get(thread, state, priority, runnable)
#define KERN_THREAD_PRIORTY_CHANGE(thread, new_priority, old_priority) \
    ssx_thread_priority_change(thread, new_priority, old_priority)
#define KERN_THREAD_AT_PRIORITY(priority, thread) \
    ssx_thread_at_priority(priority, thread)
#define KERN_THREAD_PRIORITY_SWAP(thread_a, thread_b) \
    ssx_thread_priority_swap(thread_a, thread_b)

#define KERN_START_THREADS()            ssx_start_threads()
#define KERN_THREAD_RESUME(thread)      ssx_thread_resume(thread)
#define KERN_THREAD_SUSPEND(thread)     ssx_thread_suspend(thread)
#define KERN_THREAD_DELETE(thread)      ssx_thread_delete(thread)
#define KERN_COMPLETE()                 ssx_complete()
#define KERN_SLEEP_ABSOLUTE(time)       ssx_sleep_absolute(time)
#define KERN_SLEEP(interval)            ssx_sleep(interval)

#define KERN_SEMAPHORE_CREATE(semaphore, initial_count, max_count) \
    ssx_semaphore_create(semaphore, initial_count,max_count)
#define KERN_SEMAPHORE_STATIC_CREATE(sem, initial_count, max_count) \
    SSX_SEMAPHORE(sem, initial_count, max_count)
#define KERN_SEMAPHORE_INITIALIZATION(_initial_count, _max_count) \
    SSX_SEMAPHORE_INITIALIZATION(_initial_count, _max_count)
#define KERN_SEMAPHORE_INFO_GET(semaphore, count, pending) \
    ssx_semaphore_info_get(semaphore, count, pending)

#define KERN_SEMAPHORE_POST(semaphore) \
    ssx_semaphore_post(semaphore)
#define KERN_SEMAPHORE_PEND(semaphore, timeout) \
    ssx_semaphore_pend(semaphore, timeout)
#define KERN_SEMAPHORE_RELEASE_ALL(semaphore) \
    ssx_semaphore_release_all(semaphore)
#define KERN_SEMAPHORE_POST_HANDLER(arg, irq, priority) \
    ssx_semaphore_post_handler(arg, irq, priority)


#define KERN_HALT() \
    ssx_halt()
#define KERN_PANIC(code) \
    SSX_PANIC(code)

#define KERN_DEQUE_SENTINEL_CREATE(deque) \
    ssx_deque_sentinel_create(deque)
#define KERN_DEQUE_SENTINEL_STATIC_CREATE(deque) \
    SSX_DEQUE_SENTINEL_STATIC_CREATE(deque)
#define KERN_DEQUE_SENTINEL_INIT(dq_addr) \
    SSX_DEQUE_SENTINEL_INIT(dq_addr)
#define KERN_DEQUE_ELEMENT_CREATE(element) \
    ssx_deque_element_create(element)
#define KERN_DEQUE_ELEMENT_STATIC_CREATE(deque) \
    SSX_DEQUE_ELEMENT_STATIC_CREATE(deque)
#define KERN_DEQUE_ELEMENT_INIT() \
    SSX_DEQUE_ELEMENT_INIT()
#define KERN_DEQUE_IS_EMPTY(deque) \
    ssx_deque_is_empty(deque)
#define KERN_DEQUE_IS_QUEUED(element) \
    ssx_deque_is_queued(element)
#define KERN_DEQUE_PUSH_BACK(deque, element) \
    ssx_deque_push_back(deque, element)
#define KERN_DEQUE_PUSH_FRONT(deque, element) \
    ssx_deque_push_front(deque, element)
#define KERN_DEQUE_POP_FRONT(deque) \
    ssx_deque_pop_front(deque)
#define KERN_DEQUE_DELETE(element) \
    ssx_deque_delete(element)

#define KERN_IRQ_HANDLER(f) \
    SSX_IRQ_HANDLER(f)
#define KERN_IRQ_SETUP(irq, polarity, trigger) \
    ssx_irq_setup(irq, polarity, trigger)
#define KERN_IRQ_HANDLER_SET(irq, handler, arg, priority) \
    ssx_irq_handler_set(irq, handler, arg, priority)
#define KERN_IRQ_ENABLE(irq) \
    ssx_irq_enable(irq)
#define KERN_IRQ_DISABLE(irq) \
    ssx_irq_disable(irq)
#define KERN_IRQ_STATUS_CLEAR(irq) \
    ssx_irq_status_clear(irq)
#define KERN_IRQ_STATUS_SET(irq, value) \
    ssx_irq_status_set(irq, value)
#define KERN_IRQ_FAST2FULL(fast_handler, full_handler) \
    SSX_IRQ_FAST2FULL(fast_handler, full_handler)

#define KERN_CRITICAL_SECTION_ENTER(priority, pctx) \
    ssx_critical_section_enter(priority, pctx)
#define KERN_CRITICAL_SECTION_EXIT(pctx) \
    ssx_critical_section_exit(pctx)
#define KERN_CONTEXT_CRITICAL_INTERRUPT() \
    __ssx_kernel_context_critical_interrupt()

#define KERN_ERROR_IF(condition, code) SSX_ERROR_IF(condition, code)

#define KERN_CAST_POINTER(t, p) SSX_CAST_POINTER(t, p)

#define KERN_STATIC_ASSERT(cond) SSX_STATIC_ASSERT(cond)

#elif defined(__PK__)

/// ----------------------- Use PK kernel interfaces --------------------------

#include "pk.h"

//Types
#define KERN_SEMAPHORE              PkSemaphore
#define KERN_SEMAPHORE_COUNT        PkSemaphoreCount
#define KERN_DEQUE                  PkDeque
#define KERN_THREAD                 PkThread
#define KERN_THREAD_PRIORITY        PkThreadPriority
#define KERN_THREAD_STATE           PkThreadState
#define KERN_THREAD_ROUTINE         PkThreadRoutine
#define KERN_THREAD_FLAGS           PkThreadFlags
#define KERN_ADDRESS                PkAddress
#define KERN_TIMER                  PkTimer
#define KERN_TIMER_CALLBACK         PkTimerCallback
#define KERN_INTERVAL               PkInterval
#define KERN_TIMEBASE               PkTimebase
#define KERN_IRQ_ID                 PkIrqId
#define KERN_MACHINE_CONTEXT        PkMachineContext

//Constants
#define KERN_SEMAPHORE_PEND_TIMED_OUT   PK_SEMAPHORE_PEND_TIMED_OUT
#define KERN_SEMAPHORE_PEND_NO_WAIT     PK_SEMAPHORE_PEND_NO_WAIT
#define KERN_NONCRITICAL                0
#define KERN_CRITICAL                   0
#define KERN_SUPERCRITICAL              0
#define KERN_ERROR_CHECK_API            PK_ERROR_CHECK_API
#define KERN_NO_WAIT                    PK_NO_WAIT
#define KERN_WAIT_FOREVER               PK_WAIT_FOREVER

//Functions
#define KERN_SECONDS(s)         PK_SECONDS(s)
#define KERN_MILLISECONDS(m)    PK_MILLISECONDS(m) 
#define KERN_MICROSECONDS(u)    PK_MICROSECONDS(u)
#define KERN_NANOSECONDS(n)     PK_NANOSECONDS(n) 

#define KERN_TIMEBASE_GET()     pk_timebase_get()
#define KERN_TIMEBASE_SET(tb)   pk_timebase_set(tb)

#define KERN_INTERRUPT_PREEMPTION_ENABLE()  pk_interrupt_preemption_enable()
#define KERN_INTERRUPT_PREEMPTION_DISABLE() pk_interrupt_preemption_disable()

#define KERN_TIMER_CREATE(timer, callback, arg) \
    pk_timer_create(timer, callback, arg)
#define KERN_TIMER_CREATE_NONPREEMPTIBLE(timer, callback, arg)  \
    pk_timer_create_nonpreemptible(timer, callback, arg)
#define KERN_TIMER_SCHEDULE_ABSOLUTE(timer, time, period) \
    pk_timer_schedule_absolute(timer, time, period)
#define KERN_TIMER_SCHEDULE(timer, interval, period) \
    pk_timer_schedule(timer, interval, period)
#define KERN_TIMER_CANCEL(timer) \
    pk_timer_cancel(timer)
#define KERN_TIMER_INFO_GET(timer, timeout, active) \
    pk_timer_info_get(timer, timeout, active)

#define KERN_THREAD_CREATE(thread, thread_routine, arg, stack, stack_size, priority) \
    pk_thread_create(thread, thread_routine, arg, stack, stack_size, priority)
#define KERN_THREAD_INFO_GET(thread, state, priority, runnable) \
    pk_thread_info_get(thread, state, priority, runnable)
#define KERN_THREAD_PRIORTY_CHANGE(thread, new_priority, old_priority) \
    pk_thread_priority_change(thread, new_priority, old_priority)
#define KERN_THREAD_AT_PRIORITY(priority, thread) \
    pk_thread_at_priority(priority, thread)
#define KERN_THREAD_PRIORITY_SWAP(thread_a, thread_b) \
    pk_thread_priority_swap(thread_a, thread_b)

#define KERN_START_THREADS()            pk_start_threads()
#define KERN_THREAD_RESUME(thread)      pk_thread_resume(thread)
#define KERN_THREAD_SUSPEND(thread)     pk_thread_suspend(thread)
#define KERN_THREAD_DELETE(thread)      pk_thread_delete(thread)
#define KERN_COMPLETE()                 pk_complete()
#define KERN_SLEEP_ABSOLUTE(time)       pk_sleep_absolute(time)
#define KERN_SLEEP(interval)            pk_sleep(interval)

#define KERN_SEMAPHORE_CREATE(semaphore, initial_count, max_count) \
    pk_semaphore_create(semaphore, initial_count,max_count)
#define KERN_SEMAPHORE_STATIC_CREATE(sem, initial_count, max_count) \
    PK_SEMAPHORE(sem, initial_count, max_count)
#define KERN_SEMAPHORE_INITIALIZATION(_initial_count, _max_count) \
    PK_SEMAPHORE_INITIALIZATION(_initial_count, _max_count)
#define KERN_SEMAPHORE_INFO_GET(semaphore, count, pending) \
    pk_semaphore_info_get(semaphore, count, pending)

#define KERN_SEMAPHORE_POST(semaphore) \
    pk_semaphore_post(semaphore)
#define KERN_SEMAPHORE_PEND(semaphore, timeout) \
    pk_semaphore_pend(semaphore, timeout)
#define KERN_SEMAPHORE_RELEASE_ALL(semaphore) \
    pk_semaphore_release_all(semaphore)
#define KERN_SEMAPHORE_POST_HANDLER(arg, irq, priority) \
    pk_semaphore_post_handler(arg, irq, priority)


#define KERN_HALT() \
    pk_halt()
#define KERN_PANIC(code) \
    PK_PANIC(code)

#define KERN_DEQUE_SENTINEL_CREATE(deque) \
    pk_deque_sentinel_create(deque)
#define KERN_DEQUE_SENTINEL_STATIC_CREATE(deque) \
    PK_DEQUE_SENTINEL_STATIC_CREATE(deque)
#define KERN_DEQUE_SENTINEL_INIT(dq_addr) \
    PK_DEQUE_SENTINEL_INIT(dq_addr)
#define KERN_DEQUE_ELEMENT_CREATE(element) \
    pk_deque_element_create(element)
#define KERN_DEQUE_ELEMENT_STATIC_CREATE(deque) \
    PK_DEQUE_ELEMENT_STATIC_CREATE(deque)
#define KERN_DEQUE_ELEMENT_INIT() \
    PK_DEQUE_ELEMENT_INIT()
#define KERN_DEQUE_IS_EMPTY(deque) \
    pk_deque_is_empty(deque)
#define KERN_DEQUE_IS_QUEUED(element) \
    pk_deque_is_queued(element)
#define KERN_DEQUE_PUSH_BACK(deque, element) \
    pk_deque_push_back(deque, element)
#define KERN_DEQUE_PUSH_FRONT(deque, element) \
    pk_deque_push_front(deque, element)
#define KERN_DEQUE_POP_FRONT(deque) \
    pk_deque_pop_front(deque)
#define KERN_DEQUE_DELETE(element) \
    pk_deque_delete(element)

#define KERN_IRQ_HANDLER(f) \
    PK_IRQ_HANDLER(f)
#define KERN_IRQ_SETUP(irq, polarity, trigger) \
    pk_irq_setup(irq, polarity, trigger)
#define KERN_IRQ_HANDLER_SET(irq, handler, arg, priority) \
    pk_irq_handler_set(irq, handler, arg)
#define KERN_IRQ_ENABLE(irq) \
    pk_irq_enable(irq)
#define KERN_IRQ_DISABLE(irq) \
    pk_irq_disable(irq)
#define KERN_IRQ_STATUS_CLEAR(irq) \
    pk_irq_status_clear(irq)
#define KERN_IRQ_STATUS_SET(irq, value) \
    pk_irq_status_set(irq, value)
#define KERN_IRQ_FAST2FULL(fast_handler, full_handler) \
    PK_IRQ_FAST2FULL(fast_handler, full_handler)

#define KERN_CRITICAL_SECTION_ENTER(priority, pctx) \
    pk_critical_section_enter(pctx)
#define KERN_CRITICAL_SECTION_EXIT(pctx) \
    pk_critical_section_exit(pctx)
#define KERN_CONTEXT_CRITICAL_INTERRUPT() \
    (0)

#define KERN_ERROR_IF(condition, code) PK_ERROR_IF(condition, code)

#define KERN_CAST_POINTER(t, p) PK_CAST_POINTER(t, p)

#define KERN_STATIC_ASSERT(cond) PK_STATIC_ASSERT(cond)

#else

/// ----------------------- Kernel type not defined --------------------------

#error "Kernel type must be defined in img_defs.mk"

#endif /*__SSX__*/

#endif /*__ASSEMBLER__*/

#endif /* __KERNEL_H__ */
