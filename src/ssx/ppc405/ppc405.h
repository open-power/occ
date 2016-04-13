/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/ppc405/ppc405.h $                                     */
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
#ifndef __PPC405_H__
#define __PPC405_H__

/// \file ppc405.h
/// \brief PowerPC 405 port header for SSX

//  The 405 has a 32-byte line and 2-way set associative caches.  The cache
//  configuration varies by chip/ASIC. 
//
// Regarding the DCACHE_TAG_MASK, used by dcache_flush_all: The IBM
// documentation on the D-cache tag sizes doesn't make any sense to me - it
// claims the tag size is constant regardless of the size of the cache.
// However the Xilinx documentation for their 405 core (which has the same
// 16KB cache as PgP) is consistent with the way the DCACHE_TAG_MASK is
// defined here.

#define CACHE_LINE_SIZE     32
#define LOG_CACHE_LINE_SIZE 5

#define ICACHE_WAYS         2
#define DCACHE_WAYS         2

#define LOG_ICACHE_WAYS     1
#define LOG_DCACHE_WAYS     1

#define ICACHE_LINES (ICACHE_SIZE / CACHE_LINE_SIZE)
#define DCACHE_LINES (DCACHE_SIZE / CACHE_LINE_SIZE)

#define DCACHE_TAG_MASK \
    ((0xffffffff << (LOG_DCACHE_SIZE - LOG_DCACHE_WAYS)) & 0xffffffff)

#ifdef CHIP_PGP

#define ICACHE_SIZE  (16 * 1024)
#define DCACHE_SIZE  (16 * 1024)

#define LOG_ICACHE_SIZE 14
#define LOG_DCACHE_SIZE 14

#else
#error "Please define the cache configuration of the processor"
#endif

// Macros to define where declared code is actually compiled

#ifdef __PPC405_C__
#define IF__PPC405_CORE_C__(x) x
#define UNLESS__PPC405_CORE_C__(x) 
#else
#define IF__PPC405_CORE_C__(x)
#define UNLESS__PPC405_CORE_C__(x) x
#endif

#ifdef __PPC405_IRQ_CORE_C__
#define IF__PPC405_IRQ_CORE_C__(x) x
#define UNLESS__PPC405_IRQ_CORE_C__(x) 
#else
#define IF__PPC405_IRQ_CORE_C__(x)
#define UNLESS__PPC405_IRQ_CORE_C__(x) x
#endif


#ifdef CHIP_PGP
#include "pgp.h"
#endif

#include "ppc32.h"
#include "ppc405_dcr.h"
#include "ppc405_spr.h"
#include "ppc405_msr.h"
#include "ppc405_irq.h"
#include "ppc405_cache.h"

#if PPC405_MMU_SUPPORT

#include "ppc405_mmu.h"

#ifndef PPC405_IR_SUPPORT
#define PPC405_IR_SUPPORT 1
#endif

#ifndef PPC405_DR_SUPPORT
#define PPC405_DR_SUPPORT 1
#endif

#define PPC405_RELOCATION_MODE \
    ((PPC405_IR_SUPPORT * MSR_IR) | (PPC405_DR_SUPPORT * MSR_DR))

#ifndef __ASSEMBLER__
void*
memcpy_real(void* dest, const void* src, size_t n);
#endif

#else   /* PPC405_MMU_SUPPORT */

#define PPC405_RELOCATION_MODE 0

#ifndef __ASSEMBLER__
static inline void*
memcpy_real(void* dest, const void* src, size_t n)
{
    memcpy(dest, src, n);
    icache_invalidate_all();
    return dest;
}
#endif

#endif  /* PPC405_MMU_SUPPORT */

/// By default, in MMU mode free space is read/write only, just like normal
/// data. Saome applications may need to execute from free space however, and
/// can override this default.

#ifndef EXECUTABLE_FREE_SPACE
#define EXECUTABLE_FREE_SPACE 0
#endif

#include "ppc405_context.h"

// PPC405 stack characteristics for SSX.  The pre-pattern pattern is selected
// to be easily recognizable yet be an illegal instruction.

#define SSX_STACK_DIRECTION     -1
#define SSX_STACK_PRE_DECREMENT 1
#define SSX_STACK_ALIGNMENT     8
#define SSX_STACK_TYPE          unsigned int
#define SSX_STACK_PATTERN       0x03abcdef

// Kernel data structure offsets for assembler code

#define SSX_THREAD_OFFSET_SAVED_STACK_POINTER 0
#define SSX_THREAD_OFFSET_STACK_LIMIT         4
#define SSX_THREAD_OFFSET_STACK_BASE          8

// SSX boot loader panic codes

#define PPC405_BOOT_CCR0_MODIFY_FAILED  0x00405000
#define PPC405_BOOT_VECTORS_NOT_ALIGNED 0x00405001

// Interrupt handler panic codes

#define PPC405_DEFAULT_IRQ_HANDLER     0x00405010
#define PPC405_DEFAULT_SPECIAL_HANDLER 0x00405011
#define PPC405_PHANTOM_INTERRUPT       0x00405012
#define PPC405_PROGRAM_HALT            0x00405013


// Exception handling invariant panic codes

#define PPC405_IRQ_FULL_EXIT_INVARIANT 0x00405020
#define PPC405_IRQ_FAST2FULL_INVARIANT 0x00405021


// API error panic codes

#define PPC405_CACHE_ALIGNMENT 0x00405030

// Application-overrideable definitions

/// The default thread machine context has MSR[CE], MSR[EE] and MSR[ME] set,
/// and all other MSR bits cleared.
///
/// The default definition allows critical, non-critical and machine check
/// exceptions. Debug interrupts are not enabled by default.  This definition
/// can be overriden by the application.  If MMU protection is enabled then
/// the IR/DR bits are also modeably set.

#ifndef SSX_THREAD_MACHINE_CONTEXT_DEFAULT
#define SSX_THREAD_MACHINE_CONTEXT_DEFAULT \
    (MSR_CE | MSR_EE | MSR_ME | PPC405_RELOCATION_MODE)
#endif


#ifndef __ASSEMBLER__

/// The SSX kernel default panic sequence for C code
///
/// By default a kernel panic from C code forces external debug mode then
/// generates a \c trap instruction followed by the error code.  The \a code
/// argument must be a compile-time integer immediate. This definition can be
/// overriden by the application.
///
/// The OCC may be running in internal debug mode for various reasons, and
/// TRAP-ing in internal debug mode would lead to an infinite loop in the
/// default Program Interrupt handler - which itself would be a TRAP (since
/// that's the default implementation of SSX_PANIC().  Therefore by default
/// the panic is implemented as a special code sequence that forces the core
/// into external debug mode before issuing a TRAP which will halt the core.
/// To preserve the state we use the special global variables
/// __ssx_panic_save_dbcr0 and __ssx_panic_save_r3 defined in ppc405_core.c.
/// The original value of DBCR0 is destroyed, but can be recovered from the
/// global.  In the end %r3 is reloaded from temporary storage and will be
/// unchanged at the halt.
///
/// Note that there is a small chance that an interrupt will fire and
/// interrupt this code before the halt - in general there is no way around
/// this.
///
/// The Simics environment does not model Debug events correctly. It executes
/// the TRAP as an illegal instruction and branches to the Program Interrupt
/// handler, destroying the contents of SRR0 and SRR1. Therefore we always
/// insert a special Simics magic breakpoint (which is an effective NOP)
/// before the hardware trap.  The special-form magic instruction is
/// recognized by our Simics support scripts which decode the kernel state and
/// try to help the user interpret what happened based on the TRAP code.

#ifndef SSX_PANIC
 
#define SSX_PANIC(code)                                                 \
    do {                                                                \
        barrier();                                                      \
        asm volatile ("stw     %r3, __ssx_panic_save_r3@sda21(0)");     \
        asm volatile ("mfdbcr0 %r3");                                   \
        asm volatile ("stw     %r3, __ssx_panic_save_dbcr0@sda21(0)");  \
        asm volatile ("lwz     %r3, __ssx_panic_dbcr0@sda21(0)");       \
        asm volatile ("mtdbcr0 %r3");                                   \
        asm volatile ("isync");                                         \
        asm volatile ("lwz     %r3, __ssx_panic_save_r3@sda21(0)");     \
        asm volatile ("rlwimi  1,1,0,0,0");                             \
        asm volatile ("trap");                                          \
        asm volatile (".long %0" : : "i" (code));                       \
    } while (0)

// These variables are used by the SSX_PANIC() definition above to save and
// restore state. __ssx_panic_dbcr0 is the value loaded into DBCR0 to force
// traps to halt the OCC and freeze the timers.

#ifdef __PPC405_CORE_C__
uint32_t __ssx_panic_save_r3;
uint32_t __ssx_panic_save_dbcr0;
uint32_t __ssx_panic_dbcr0 = DBCR0_EDM | DBCR0_TDE | DBCR0_FT;
#endif

#endif // SSX_PANIC

/// This is the Simics 'magic breakpoint' instruction.
///
/// Note that this form does not include a memory barrier, as doing so might
/// change the semantics of the program.  There is an alternative form
/// SIMICS_MAGIC_BREAKPOINT_BARRIER that does include a barrier.

#define SIMICS_MAGIC_BREAKPOINT asm volatile ("rlwimi 0,0,0,0,0")

/// This is the Simics 'magic breakpoint' instruction including a memory
/// barrier. 
///
/// Note that the memory barrier guarantees that all variables held in
/// registers are flushed to memory before the breakpoint, however this might
/// change the semantics of the program.  There is an alternative form of
/// SIMICS_MAGIC_BREAKPOINT that does not include a barrier.  If the idea is
/// to use the breakpoint for tracing code execution in Simics, the barrier
/// form may be preferred so that variable values will be visible in memory.

#define SIMICS_MAGIC_BREAKPOINT_BARRIER \
             asm volatile ("rlwimi 0,0,0,0,0" : : : "memory")


#else // __ASSEMBLER__

/// This is the Simics 'magic breakpoint' instruction.  An assembler macro
/// form is also provided for use within macros.

#define SIMICS_MAGIC_BREAKPOINT rlwimi 0,0,0,0,0

        .macro _simics_magic_breakpoint
        rlwimi  0,0,0,0,0
        .endm

/// The SSX kernel panic default panic sequence for assembler code
///
/// By default a kernel panic from assembler forces external debug mode then
/// generates a \c trap instruction followed by the error code.  The \a code
/// argument must be a compile-time integer immediate. This definition can be
/// overriden by the application.
///
/// See the comments for the non-ASSEMBLER version for further details.  Note
/// that the code space reserved for exception handlers is only 8
/// instructions, so in the assembler context we don't save DBCR0 as doing so
/// would require 10.

#ifndef SSX_PANIC
        
#define SSX_PANIC(code) _ssx_panic code

        .macro  _ssx_panic, code
        _stwsd  %r3, __ssx_panic_save_r3
        _lwzsd  %r3, __ssx_panic_dbcr0
        mtdbcr0 %r3
        isync
        _lwzsd  %r3, __ssx_panic_save_r3
        rlwimi  1,1,0,0,0
        trap
        .long \code
        .endm

#endif // SSX_PANIC

#endif // __ASSEMBLER__


//  Application-overridible definitions for the SSX boot loader

/// In order to enable the default kernel panic (a trap) to halt the machine,
/// the Debug Control Register 0 (DBCR0) is initialized in externel debug
/// mode, with the Trap Debug Event enabled so that the trap will not cause a
/// program exception, and the FT bit set so that the timers will freeze.
/// This definition can be overridden by the application.
///
/// NB: It is expected that a reliable production system will redefine all of
/// the 'panic' macros and the default DBCR0 setup.

#ifndef PPC405_DBCR0_INITIAL
#define PPC405_DBCR0_INITIAL (DBCR0_EDM | DBCR0_TDE | DBCR0_FT)
#endif

/// This is the value of the MSR used during initialization.  Once SSX threads
/// are started (with \c ssx_start_threads()), all machine contexts derive
/// from the default thread context \c
/// SSX_THREAD_MACHINE_CONTEXT_DEFAULT. This definition can be overriden by
/// the application.
///
/// The default is to enable machine checks only.

#ifndef PPC405_MSR_INITIAL
#define PPC405_MSR_INITIAL MSR_ME
#endif

/// This is the initial value of Cache Control Register 0 (CCR0).  This
/// definition can be overridden by the application.
///
/// The default sets the CCR0 to give priority to DCU and ICU operations.  The
/// user should consider setting other options in this register that affect
/// performance, e.g., ICU prefetching. Other options can be set at run time
/// with the API \c ppc405_ccr0_modify().

#ifndef PPC405_CCR0_INITIAL
#define PPC405_CCR0_INITIAL (CCR0_DPP1 | CCR0_IPP0 | CCR0_IPP1)
#endif

/// The \a argc argument passed to \c main(). This definition can be overriden
/// by the application.

#ifndef PPC405_ARGC_INITIAL
#define PPC405_ARGC_INITIAL 0
#endif

/// The \a argv argument passed to \c main(). This definition can be overriden
/// by the application.

#ifndef PPC405_ARGV_INITIAL
#define PPC405_ARGV_INITIAL 0
#endif

/// Optionally trap the reset for the debugger, which means that the PPC405
/// will simply spin at the symbol \c __reset_trap after a chip reset. Set R0
/// to a non-zero value in the debugger to continue execution. This definition
/// can be overriden by the application.

#ifndef PPC405_RESET_TRAP
#define PPC405_RESET_TRAP 0
#endif

#ifndef __ASSEMBLER__

/// The PPC405 SSX machine context is simply the MSR, a 32-bit integer.

typedef uint32_t SsxMachineContext;

/// Disable interrupts at the given priority level, and return the current
/// context.  
///
/// \param priority The interrupt priority level to disable, either \c
/// SSX_NONCRITICAL, \c SSX_CRITICAL or \c SSX_SUPERCRITICAL.  For best
/// efficiency, the \a priority parameter should be a manifest constant.
///
/// \param context A pointer to an SsxMachineContext, this is the context that
///                existed before interrupts were disabled. Typically this
///                context is restored at the end of a critical section.
///
/// The PPC405 supports a 'super-critical' context in which every possible
/// maskable exception is disabled.
///
/// Return values other then SSX_OK (0) are errors; see \ref ssx_errors
///
/// \retval 0 Successful completion
///
/// \retval -SSX_INVALID_ARGUMENT_INTERRUPT An illegal priority was specified.

UNLESS__PPC405_CORE_C__(extern)
inline int
ssx_interrupt_disable(int priority, SsxMachineContext *context)
{
    *context = mfmsr();

    if (priority == SSX_NONCRITICAL) {

        wrteei(0);

    } else if (priority == SSX_CRITICAL) {

        mtmsr(*context & ~(MSR_EE | MSR_CE));

    } else if (priority == SSX_SUPERCRITICAL) {

        mtmsr(*context & ~(MSR_APE | MSR_WE | MSR_CE | MSR_EE | MSR_ME | 
                           MSR_DWE | MSR_DE));

    } else if (SSX_ERROR_CHECK_API) {
        SSX_ERROR(SSX_INVALID_ARGUMENT_INTERRUPT);
    }

    return SSX_OK;
}


/// Set the machine context.
///
/// \param context A pointer to an SsxMachineContext
///
/// Return values other then SSX_OK (0) are errors; see \ref ssx_errors
///
/// \retval 0 Successful completion
///
/// \retval -SSX_INVALID_ARGUMENT_CONTEXT_SET A null pointer was provided as 
/// the \a context argument or an illegal machine context was specified.

UNLESS__PPC405_CORE_C__(extern)
inline int
ssx_machine_context_set(SsxMachineContext *context)
{
    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF(context == 0, SSX_INVALID_ARGUMENT_CONTEXT_SET);
    }

    mtmsr(*context);

    return SSX_OK;
}


/// Get the machine context.
///
/// \param context A pointer to an SsxMachineContext.
///
/// Return values other then SSX_OK (0) are errors; see \ref ssx_errors
///
/// \retval 0 Successful completion
///
/// \retval -SSX_INVALID_ARGUMENT_CONTEXT_GET A null pointer was provided as 
/// the \a context argument.

UNLESS__PPC405_CORE_C__(extern)
inline int
ssx_machine_context_get(SsxMachineContext *context)
{
    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF(context == 0, SSX_INVALID_ARGUMENT_CONTEXT_GET);
    }

    *context = mfmsr();

    return SSX_OK;
}


/// The SSX kernel thread context switch - PPC405 uses the system call
/// exception. 

#define __ssx_switch() asm volatile ("sc")


/// In the PowerPC EABI all initial stack frames require 8 bytes - the 4 bytes
/// at the SP are zeroed to indicate the end of the stack, and the 4 bytes
/// behind the SP are for the initial subroutine's LR.

static inline void
__ssx_stack_create_initial_frame(SsxAddress *stack, size_t *size) \
{
    *stack -= 8;
    *size -= 8; 
    *((SSX_STACK_TYPE *)(*stack)) = 0;
}


/// The SSX Kernel Context for PPC405 
///
/// The SSX portable kernel does not define how the kernel keeps track of
/// whether SSX is running, interrupt levels, and other debug
/// information. Instead it defines an API that the port must provide to the
/// portable kernel.
///
/// In the PPC405 port, the kernel context is maintained in USPRG0.  This
/// 32-bit value is treated as 5 distinct fields as indicated in the structure
/// definition. For certain tests it's also helpful to look at the two
/// interrupt counters as a single 0/non-0 field.

typedef union {

    uint32_t value;

    struct {

        /// The critical interrupt nesting level.  If this field is non-zero,
        /// then interrupt priority and preemption rules guarantee that a
        /// critical interrupt handler is running, and the \c irq field will
        /// contain the SsxIrqId of the currently active critical interrupt.
        unsigned critical_interrupts : 8;

        /// The non-critical interrupt nesting level. If this field is
        /// non-zero and the \c critical_interrupts field is 0, then interrupt
        /// priority and preemption rules guarantee that a noncritical
        /// interrupt handler is running, and the \c irq field will contain
        /// the SsxIrqId of the currently active noncritical interrupt.
        unsigned noncritical_interrupts : 8;

        /// The SsxIrqId of the currently running (or last run) handler.  If
        /// either of the interrupt nesting levels are non-0, then this is the
        /// SsxIrqId of the IRQ that is currently executing.
        unsigned irq : 8;

        /// A flag indicating that SSX is in thread mode after a call of
        /// ssx_start_threads(). 
        unsigned thread_mode : 1;

        /// The priority of the currently running thread.  In an interrupt
        /// context, this is the priority of the thread that was interrupted.
        unsigned thread_priority : 7;

    } fields;

    struct {

        /// Used as a 0/non-0 flag for interrupt context.
        unsigned interrupt_context : 16;

        /// Ignore
        unsigned ignore : 16;

    } merged_fields;

} __SsxKernelContext;


// These APIs are provided to the SSX portable kernel by the port.

/// SSX threads have been started by a call of ssx_start_threads().

#define __ssx_kernel_mode_thread() \
    ({ \
        __SsxKernelContext __ctx; \
        __ctx.value = mfspr(SPRN_USPRG0); \
        __ctx.fields.thread_mode;})


///  SSX is executing in a thread context (not an interrupt handler).

#define __ssx_kernel_context_thread() \
    ({ \
        __SsxKernelContext __ctx; \
        __ctx.value = mfspr(SPRN_USPRG0); \
        __ctx.fields.thread_mode && !__ctx.merged_fields.interrupt_context;})


///  SSX is executing an interrupt handler of any priority.

#define __ssx_kernel_context_any_interrupt() \
    ({ \
        __SsxKernelContext __ctx; \
        __ctx.value = mfspr(SPRN_USPRG0); \
        __ctx.merged_fields.interrupt_context;})


/// SSX is executing a critical interrupt handler.

#define __ssx_kernel_context_critical_interrupt() \
    ({ \
        __SsxKernelContext __ctx; \
        __ctx.value = mfspr(SPRN_USPRG0); \
        __ctx.fields.critical_interrupts;})


/// SSX is executing a non-critical interrupt handler.

#define __ssx_kernel_context_noncritical_interrupt() \
    ({ \
        __SsxKernelContext __ctx; \
        __ctx.value = mfspr(SPRN_USPRG0); \
        __ctx.fields.noncritical_interrupts &&  \
            !__ctx.fields.critical_interrupts;})


/// Return the noncritical interrupt nesting level

#define __ssx_noncritical_level() \
    ({ \
        __SsxKernelContext __ctx; \
        __ctx.value = mfspr(SPRN_USPRG0); \
        __ctx.fields.noncritical_interrupts; })

        
/// Return the critical interrupt nesting level

#define __ssx_critical_level() \
    ({ \
        __SsxKernelContext __ctx; \
        __ctx.value = mfspr(SPRN_USPRG0); \
        __ctx.fields.critical_interrupts; })
        

// SSX requires the port to define the type SsxThreadQueue, which is a
// priority queue (where 0 is the highest priority).  This queue must be able
// to handle SSX_THREADS + 1 priorities (the last for the idle thread) The
// port must also define methods for clearing, insertion, deletion and min
// (with assumed legal priorities).  The min operation returns SSX_THREADS if
// the queue is empty (or a queue could be initialized with that entry always
// present - SSX code never tries to delete the idle thread from a thread
// queue).
//
// These queues are used both for the run queue and the pending queue
// associated with every semaphore. 
//
// On PPC405 with 32 threads (implied), this is a job for a uint32_t and
// cntlzw(). 

static inline void
__ssx_thread_queue_clear(volatile SsxThreadQueue *queue)
{
    *queue = 0;
}

static inline void
__ssx_thread_queue_insert(volatile SsxThreadQueue *queue, SsxThreadPriority priority)
{
    *queue |= (0x80000000u >> priority);
}

static inline void
__ssx_thread_queue_delete(volatile SsxThreadQueue *queue, SsxThreadPriority priority)
{
    *queue &= ~(0x80000000u >> priority);
}

static inline SsxThreadPriority
__ssx_thread_queue_min(volatile SsxThreadQueue *queue)
{
    return cntlzw(*queue);
}

static inline int
__ssx_thread_queue_member(volatile SsxThreadQueue *queue, SsxThreadPriority priority)
{
    return ((*queue >> (31 - priority)) & 1);
}

static inline void
__ssx_thread_queue_union(volatile SsxThreadQueue *queue0, 
                         volatile SsxThreadQueue *queue1)
{
    *queue0 |= *queue1;
}

static inline int
__ssx_thread_queue_count(volatile SsxThreadQueue* queue) 
{
    return __builtin_popcount(*queue);
}


/// This macro is used to call __ssx_start_threads() using the kernel stack,
/// in a SSX_NONCRITICAL critical section.

#define __ssx_call_ssx_start_threads()                          \
    do {                                                        \
        SsxMachineContext ctx;                                  \
        ssx_critical_section_enter(SSX_NONCRITICAL, &ctx);      \
        asm volatile ("mr 1, %0; mtlr %1; blrl" : :             \
                      "r" (__ssx_noncritical_stack),            \
                      "r" (__ssx_start_threads));               \
        SSX_PANIC(SSX_START_THREADS_RETURNED);                  \
    } while (0)


#endif  /* __ASSEMBLER__ */

/// The __SsxKernelContext 'thread_mode' bit as a flag

#define PPC405_THREAD_MODE 0x80


#ifndef __ASSEMBLER__

/// Code breakpoints for PPC405
///
/// This macro inserts a special PPC405-only breakpoint into the object code
/// at the place the macro invocation appears.  This facility is designed for
/// VBU/VPO procedure debugging. This type of breakpoint may not be required
/// on real hardware as we will then have the full power of RISCWatch, gdb,
/// etc. Once inserted into the code, code breakpoints can be enabled or
/// disabled by manipulating the global variable _code_breakpoint_enable,
/// which defaults to 1.
///
/// The code breakpoint is implemented as a setup routine and a teardown
/// routine, executed in an SSX_CRITICAL critical section.  The actual break
/// will occur at the address of the call of the teardown routine, in the
/// context of the calling code.  The setup routine saves the state of DBCR0/1
/// and IAC4, then programs the DBCR for an external debug mode, IAC4
/// breakpoint.  The IAC4 breakpoint is set for the address of the call of the
/// teardown routine.  The teardown routine simply restores the state of the
/// debug registers that existed before the code breakpoint.
///
/// Once hit, restarting from the break requires clearing IAC4 and restarting
/// instructions:
///
/// \code 
///
/// putspr pu.occ iac4 0
/// cipinstruct pu.occ start
///
/// \endcode
/// 
/// The above restart processes is also encapsulated as the p8_tclEcmd
/// procedure 'unbreakOcc'.
///
/// In code built for the Simics environment (i.e., with the preprocessor
/// macro SIMICS_ENVIRONMENT=1) this macro simply expands into
/// SIMICS_MAGIC_BREAKPOINT, and simulation can be continued from the break as
/// normal. This Simics magic breakpoint is also under the control of
/// _code_breakpoint_enable.  In code not built with SIMICS_ENVIROMENT=1, note
/// that the CODE_BREAKPOINT is ignored by the Simics PPC405 model as it does
/// not model debug events.

#if defined(SIMICS_ENVIRONMENT) && (SIMICS_ENVIRONMENT != 0) 
#define CODE_BREAKPOINT                         \
    do {                                        \
        if (_code_breakpoint_enable) {          \
            SIMICS_MAGIC_BREAKPOINT;            \
        }                                       \
    } while (0)
#else
#define CODE_BREAKPOINT                                         \
    do {                                                        \
        if (_code_breakpoint_enable) {                          \
            SsxMachineContext __ctx;                            \
            ssx_critical_section_enter(SSX_CRITICAL, &__ctx);   \
            _code_breakpoint_prologue();                        \
            _code_breakpoint_epilogue();                        \
            ssx_critical_section_exit(&__ctx);                  \
        }                                                       \
    } while (0)
#endif

void
_code_breakpoint_prologue(void);

void
_code_breakpoint_epilogue(void);

extern uint32_t _code_breakpoint_enable;

#endif // __ASSEMBLER__


#endif /* __PPC405_H__ */
