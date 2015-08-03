/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/pk/ppe42/ppe42.h $                                    */
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
#ifndef __PPE42_H__
#define __PPE42_H__
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2014
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file ppe42.h
/// \brief PPE42 port header for PK

// Macros to define where declared code is actually compiled

#ifdef __PPE42_C__
#define IF__PPE42_CORE_C__(x) x
#define UNLESS__PPE42_CORE_C__(x) 
#else
#define IF__PPE42_CORE_C__(x)
#define UNLESS__PPE42_CORE_C__(x) x
#endif

#ifdef __PPE42_IRQ_CORE_C__
#define IF__PPE42_IRQ_CORE_C__(x) x
#define UNLESS__PPE42_IRQ_CORE_C__(x) 
#else
#define IF__PPE42_IRQ_CORE_C__(x)
#define UNLESS__PPE42_IRQ_CORE_C__(x) x
#endif

#ifdef HWMACRO_GPE
#include "gpe.h"
#elif defined(HWMACRO_STD)
#include "std.h"
#elif defined(HWMACRO_PPE)
#include "ppe.h"
#else
#error "Macro Type not specified.  Are you building from the correct directory?"
#endif


#include "ppe42_asm.h"
#include "ppe42_gcc.h"
#include "ppe42_spr.h"
#include "ppe42_msr.h"


///start

/// The synchronization macros defined here all create a compiler
/// memory barrier that will cause GCC to flush/invalidate all memory data
/// held in registers before the macro. This is consistent with other systems,
/// e.g., the PowerPC Linux kernel, and is the safest way to define these
/// macros.


// Condition register fields

#define CR_LT(n) (0x80000000u >> (4 * (n)))
#define CR_GT(n) (0x40000000u >> (4 * (n)))
#define CR_EQ(n) (0x20000000u >> (4 * (n)))
#define CR_SO(n) (0x10000000u >> (4 * (n)))


#ifndef __ASSEMBLER__

#include "stdint.h"

/// ssize_t is defined explictly rather than bringing in all of <unistd.h>
#ifndef __ssize_t_defined
#define __ssize_t_defined
typedef int ssize_t;
#endif

/// A memory barrier
#define barrier() asm volatile ("" : : : "memory")

/// Ensure In-order Execution of Input/Output
#define eieio() asm volatile ("sync" : : : "memory")

/// Memory barrier
#define sync() asm volatile ("sync" : : : "memory")

/// Instruction barrier
#define isync() asm volatile ("sync" : : : "memory")

/// CouNT Leading Zeros Word
#define cntlzw(x) \
({uint32_t __x = (x); \
  uint32_t __lzw; \
  asm volatile ("cntlzw %0, %1" : "=r" (__lzw) : "r" (__x)); \
  __lzw;})

/// CouNT Leading Zeros : uint32_t
static inline int
cntlz32(uint32_t x) {
    return cntlzw(x);
}

/// CouNT Leading Zeros : uint64_t
static inline int
cntlz64(uint64_t x) {
    if (x > 0xffffffff) {
        return cntlz32(x >> 32);
    } else {
        return 32 + cntlz32(x);
    }
}


/// 32-bit population count
static inline int
popcount32(uint32_t x)
{
    return __builtin_popcount(x);
}
    

/// 64-bit population count
static inline int
popcount64(uint64_t x)
{
    return __builtin_popcountll(x);
}


// NB: Normally we wouldn't like to force coercion inside a macro because it
// can mask programming errors, but for the MMIO macros the addresses are
// typically manifest constants or 32-bit unsigned integer expressions so we
// embed the coercion to avoid warnings.

/// 8-bit MMIO Write
#define out8(addr, data) \
do {*(volatile uint8_t *)(addr) = (data);} while(0)

/// 8-bit MMIO Read
#define in8(addr) \
({uint8_t __data = *(volatile uint8_t *)(addr); __data;})

/// 16-bit MMIO Write
#define out16(addr, data) \
do {*(volatile uint16_t *)(addr) = (data);} while(0)

/// 16-bit MMIO Read
#define in16(addr) \
({uint16_t __data = *(volatile uint16_t *)(addr); __data;})

/// 32-bit MMIO Write
#define out32(addr, data) \
do {*(volatile uint32_t *)(addr) = (data);} while(0)

/// 32-bit MMIO Read
#define in32(addr) \
({uint32_t __data = *(volatile uint32_t *)(addr); __data;})

#ifdef HWMACRO_GPE

/// 64-bit MMIO Write
#define out64(addr, data) \
    do { \
        uint64_t __data = (data);                               \
        volatile uint32_t *__addr_hi = (uint32_t *)(addr);      \
        volatile uint32_t *__addr_lo = __addr_hi + 1;           \
        *__addr_hi = (__data >> 32);                            \
        *__addr_lo = (__data & 0xffffffff);                     \
    } while(0)

#else /* standard PPE's require a 64 bit write */

/// 64-bit MMIO Write
#define out64(addr, data) \
{\
    uint64_t __d = (data); \
    uint32_t* __a = (uint32_t*)(addr); \
    asm volatile \
    (\
        "stvd %1, %0 \n" \
        : "=o"(*__a) \
        : "r"(__d) \
    ); \
}

#endif /* HWMACRO_GPE */

#ifdef HWMACRO_GPE
/// 64-bit MMIO Read
#define in64(addr) \
    ({                                                     \
        uint64_t __data;                                   \
        volatile uint32_t *__addr_hi = (uint32_t *)(addr); \
        volatile uint32_t *__addr_lo = __addr_hi + 1;      \
        __data = *__addr_hi;                               \
        __data = (__data << 32) | *__addr_lo;              \
        __data;})

#else /* Standard PPE's require a 64 bit read */

#define in64(addr) \
({\
    uint64_t __d; \
    uint32_t* __a = (uint32_t*)(addr); \
    asm volatile \
    (\
        "lvd %0, %1 \n" \
        :"=r"(__d) \
        :"o"(*__a) \
    ); \
    __d; \
})
    
#endif  /* HWMACRO_GPE */

#endif  /* __ASSEMBLER__ */

#include "ppe42_irq.h"

#ifndef __ASSEMBLER__

/// Store revision information as a (global) string constant
#define REVISION_STRING(symbol, rev) const char* symbol = rev;

#else // __ASSEMBLER__

/// Store revision information as a global string constant
        .macro  .revision_string, symbol:req, rev:req
        .pushsection .rodata
        .balign 4
        .global \symbol
\symbol\():
        .asciz  "\rev"
        .balign 4
        .popsection
        .endm

#endif  // __ASSEMBLER__



#include "ppe42_context.h"

// PPE42 stack characteristics for PK.  The pre-pattern pattern is selected
// to be easily recognizable yet be an illegal instruction.

#define PK_STACK_DIRECTION     -1
#define PK_STACK_PRE_DECREMENT 1
#define PK_STACK_ALIGNMENT     8
#define PK_STACK_TYPE          unsigned int
#define PK_STACK_PATTERN       0x03abcdef

// Kernel data structure offsets for assembler code

#define PK_THREAD_OFFSET_SAVED_STACK_POINTER 0
#define PK_THREAD_OFFSET_STACK_LIMIT         4
#define PK_THREAD_OFFSET_STACK_BASE          8

// PK boot loader panic codes

#define PPE42_BOOT_VECTORS_NOT_ALIGNED 0x00405001

// Interrupt handler panic codes

#define PPE42_DEFAULT_IRQ_HANDLER     0x00405010
#define PPE42_DEFAULT_SPECIAL_HANDLER 0x00405011
#define PPE42_PHANTOM_INTERRUPT       0x00405012
#define PPE42_PROGRAM_HALT            0x00405013


// Exception handling invariant panic codes

#define PPE42_IRQ_FULL_EXIT_INVARIANT 0x00405020
#define PPE42_IRQ_FAST2FULL_INVARIANT 0x00405021


// API error panic codes


// Application-overrideable definitions

/// The default thread machine context has MSR[CE], MSR[EE] and MSR[ME] set,
/// and all other MSR bits cleared.
///
/// The default definition allows external and machine check exceptions.  This
/// definition can be overriden by the application.  

#ifndef PK_THREAD_MACHINE_CONTEXT_DEFAULT
#define PK_THREAD_MACHINE_CONTEXT_DEFAULT \
    (MSR_UIE | MSR_EE | MSR_ME)

#endif


#ifndef __ASSEMBLER__

/// The PK kernel default panic sequence for C code
///
/// By default a kernel panic from C code forces external debug mode then
/// generates a \c trap instruction followed by the error code.  The \a code
/// argument must be a compile-time integer immediate. This definition can be
/// overriden by the application.
///
/// The OCC may be running in internal debug mode for various reasons, and
/// TRAP-ing in internal debug mode would lead to an infinite loop in the
/// default Program Interrupt handler - which itself would be a TRAP (since
/// that's the default implementation of PK_PANIC().  Therefore by default
/// the panic is implemented as a special code sequence that forces the core
/// into external debug mode before issuing a TRAP which will halt the core.
/// To preserve the state we use the special global variables
/// __pk_panic_save_dbcr0 and __pk_panic_save_r3 defined in ppe42_core.c.
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

#ifndef PK_PANIC
 
/*#define PK_PANIC(code)                                                 \
    do {                                                                \
        barrier();                                                      \
        asm volatile ("stw     %r3, __pk_panic_save_r3@sda21(0)");     \
        asm volatile ("mfdbcr0 %r3");                                   \
        asm volatile ("stw     %r3, __pk_panic_save_dbcr0@sda21(0)");  \
        asm volatile ("lwz     %r3, __pk_panic_dbcr0@sda21(0)");       \
        asm volatile ("mtdbcr0 %r3");                                   \
        asm volatile ("isync");                                         \
        asm volatile ("lwz     %r3, __pk_panic_save_r3@sda21(0)");     \
        asm volatile ("rlwimi  1,1,0,0,0");                             \
        asm volatile ("trap");                                          \
        asm volatile (".long %0" : : "i" (code));                       \
    } while (0)
*/
#define PK_PANIC(code)                                                 \
    do {                                                                \
        barrier();                                                      \
        asm volatile ("b .");                                          \
    } while (0)

// These variables are used by the PK_PANIC() definition above to save and
// restore state. __pk_panic_dbcr0 is the value loaded into DBCR0 to force
// traps to halt the OCC and freeze the timers.

//#ifdef __PPE42_CORE_C__
//uint32_t __pk_panic_save_r3;
//uint32_t __pk_panic_save_dbcr0;
//uint32_t __pk_panic_dbcr0 = DBCR0_EDM | DBCR0_TDE | DBCR0_FT;
//#endif

#endif // PK_PANIC

/// This is the Simics 'magic breakpoint' instruction.
///
/// Note that this form does not include a memory barrier, as doing so might
/// change the semantics of the program.  There is an alternative form
/// SIMICS_MAGIC_BREAKPOINT_BARRIER that does include a barrier.

//#define SIMICS_MAGIC_BREAKPOINT asm volatile ("rlwimi 0,0,0,0,0")

/// This is the Simics 'magic breakpoint' instruction including a memory
/// barrier. 
///
/// Note that the memory barrier guarantees that all variables held in
/// registers are flushed to memory before the breakpoint, however this might
/// change the semantics of the program.  There is an alternative form of
/// SIMICS_MAGIC_BREAKPOINT that does not include a barrier.  If the idea is
/// to use the breakpoint for tracing code execution in Simics, the barrier
/// form may be preferred so that variable values will be visible in memory.

/*#define SIMICS_MAGIC_BREAKPOINT_BARRIER \
             asm volatile ("rlwimi 0,0,0,0,0" : : : "memory")
*/

#else // __ASSEMBLER__

/// This is the Simics 'magic breakpoint' instruction.  An assembler macro
/// form is also provided for use within macros.

//#define SIMICS_MAGIC_BREAKPOINT rlwimi 0,0,0,0,0

//        .macro _simics_magic_breakpoint
//        rlwimi  0,0,0,0,0
//        .endm

/// The PK kernel panic default panic sequence for assembler code
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

#ifndef PK_PANIC
        
#define PK_PANIC(code) _pk_panic code

        .macro  _pk_panic, code
        b .
        .endm

#endif // PK_PANIC

#endif // __ASSEMBLER__


//  Application-overridible definitions for the PK boot loader

/// In order to enable the default kernel panic (a trap) to halt the machine,
/// the Debug Control Register 0 (DBCR0) is initialized in externel debug
/// mode, with the Trap Debug Event enabled so that the trap will not cause a
/// program exception, and the FT bit set so that the timers will freeze.
/// This definition can be overridden by the application.
///
/// NB: It is expected that a reliable production system will redefine all of
/// the 'panic' macros and the default DBCR0 setup.

#ifndef PPE42_DBCR_INITIAL
#define PPE42_DBCR_INITIAL DBCR_TRAP
#endif

/// This is the value of the MSR used during initialization.  Once PK threads
/// are started (with \c pk_start_threads()), all machine contexts derive
/// from the default thread context \c
/// PK_THREAD_MACHINE_CONTEXT_DEFAULT. This definition can be overriden by
/// the application.
///
/// The default is to enable machine checks only.

#ifndef PPE42_MSR_INITIAL
#define PPE42_MSR_INITIAL MSR_ME
#endif

/// The \a argc argument passed to \c main(). This definition can be overriden
/// by the application.

#ifndef PPE42_ARGC_INITIAL
#define PPE42_ARGC_INITIAL 0
#endif

/// The \a argv argument passed to \c main(). This definition can be overriden
/// by the application.

#ifndef PPE42_ARGV_INITIAL
#define PPE42_ARGV_INITIAL 0
#endif

/// Optionally trap the reset for the debugger, which means that the PPE42
/// will simply spin at the symbol \c __reset_trap after a chip reset. Set R0
/// to a non-zero value in the debugger to continue execution. This definition
/// can be overriden by the application.

#ifndef PPE42_RESET_TRAP
#define PPE42_RESET_TRAP 0
#endif

// PPE42 doesn't support the system call instruction so we use this illegal
// instruction to force a program exception which then emulates the system call
// instruction.
#define PPE42_SC_INST   0x44000002

#ifndef __ASSEMBLER__

/// The PPE42 PK machine context is simply the MSR, a 32-bit integer.

typedef uint32_t PkMachineContext;

/// Disable interrupts and return the current
/// context.  
///
/// \param context A pointer to an PkMachineContext, this is the context that
///                existed before interrupts were disabled. Typically this
///                context is restored at the end of a critical section.
///
/// Return values other then PK_OK (0) are errors; see \ref pk_errors
///
/// \retval 0 Successful completion
///
/// \retval -PK_INVALID_ARGUMENT_INTERRUPT An illegal priority was specified.

UNLESS__PPE42_CORE_C__(extern)
inline int
pk_interrupt_disable(PkMachineContext *context)
{
    *context = mfmsr();

    wrteei(0);

    return PK_OK;
}

/// Set the machine context.
///
/// \param context A pointer to an PkMachineContext
///
/// Return values other then PK_OK (0) are errors; see \ref pk_errors
///
/// \retval 0 Successful completion
///
/// \retval -PK_INVALID_ARGUMENT_CONTEXT_SET A null pointer was provided as 
/// the \a context argument or an illegal machine context was specified.

UNLESS__PPE42_CORE_C__(extern)
inline int
pk_machine_context_set(PkMachineContext *context)
{
    if (PK_ERROR_CHECK_API) {
        PK_ERROR_IF(context == 0, PK_INVALID_ARGUMENT_CONTEXT_SET);
    }

    mtmsr(*context);

    return PK_OK;
}

/// Get the machine context.
///
/// \param context A pointer to an PkMachineContext.
///
/// Return values other then PK_OK (0) are errors; see \ref pk_errors
///
/// \retval 0 Successful completion
///
/// \retval -PK_INVALID_ARGUMENT_CONTEXT_GET A null pointer was provided as 
/// the \a context argument.

UNLESS__PPE42_CORE_C__(extern)
inline int
pk_machine_context_get(PkMachineContext *context)
{
    if (PK_ERROR_CHECK_API) {
        PK_ERROR_IF(context == 0, PK_INVALID_ARGUMENT_CONTEXT_GET);
    }

    *context = mfmsr();

    return PK_OK;
}

/// The PK kernel thread context switch - PPE42 uses the 'sc' illegal instruction
/// to force a program exception to occur. 
#define __pk_switch() asm volatile (".long 0x44000002")


/// In the PowerPC EABI all initial stack frames require 8 bytes - the 4 bytes
/// at the SP are zeroed to indicate the end of the stack, and the 4 bytes
/// behind the SP are for the initial subroutine's LR.

static inline void
__pk_stack_create_initial_frame(PkAddress *stack, size_t *size) \
{
    *stack -= 8;
    *size -= 8; 
    *((PK_STACK_TYPE *)(*stack)) = 0;
}

/// The PK Kernel Context for PPE42 
///
/// The PK portable kernel does not define how the kernel keeps track of
/// whether PK is running, interrupt levels, and other debug
/// information. Instead it defines an API that the port must provide to the
/// portable kernel.
///
/// In the PPE42 port, the kernel context is maintained in SPRG0.  This
/// 32-bit value is treated as 6 distinct fields as indicated in the structure
/// definition.
typedef union {

    uint32_t value;

    struct {

        /// A flag indicating that PK is in thread mode after a call of
        /// pk_start_threads(). 
        unsigned thread_mode : 1;

        /// If this field is non-zero then PK is processing an interrupt
        /// and the \c irq field will contain the PkIrqId of the interrupt
        /// that kicked off interrupt processing.
        unsigned processing_interrupt : 1;

        /// The priority of the currently running thread.  In an interrupt
        /// context, this is the priority of the thread that was interrupted.
        unsigned thread_priority : 6;

        /// This bit tracks whether the current context can be discarded or
        /// if the context must be saved.  If the processor takes an interrupt
        /// and this bit is set, then the current context will be discarded.
        /// This bit is set at the end of handling an interrupt and prior
        /// to entering the wait enabled state.
        unsigned discard_ctx : 1;

        /// The PkIrqId of the currently running (or last run) handler.  If
        /// \c processing_interrupt is set, then this is the
        /// PkIrqId of the IRQ that is currently executing.
        unsigned irq : 7;

        /// Each PPE application will define (or not) the interpretation of
        /// this field.  Since SPRG0 is saved and restored during during thread
        /// context switches, this field can be used to record the progress of
        /// individual threads.  The kernel and/or application will provide
        /// APIs or macros to read and write this field.
        unsigned app_specific : 16;

    } fields;

} __PkKernelContext;

// These APIs are provided for applications to get and set the app_specific
// field of the kernel context which is held in sprg0.

static inline uint16_t ppe42_app_ctx_get(void)
{
    __PkKernelContext __ctx;
    __ctx.value = mfspr(SPRN_SPRG0);
    return __ctx.fields.app_specific;
}

static inline void ppe42_app_ctx_set(uint16_t app_ctx)
{
    PkMachineContext    mctx;
    __PkKernelContext   __ctx;
    mctx = mfmsr();
    wrteei(0);
    __ctx.value = mfspr(SPRN_SPRG0);
    __ctx.fields.app_specific = app_ctx;
    mtspr(SPRN_SPRG0, __ctx.value);
    mtmsr(mctx);
}

// These APIs are provided to the PK portable kernel by the port.

/// PK threads have been started by a call of pk_start_threads().

#define __pk_kernel_mode_thread() \
    ({ \
        __PkKernelContext __ctx; \
        __ctx.value = mfspr(SPRN_SPRG0); \
        __ctx.fields.thread_mode;})


///  PK is executing in a thread context (not an interrupt handler).

#define __pk_kernel_context_thread() \
    ({ \
        __PkKernelContext __ctx; \
        __ctx.value = mfspr(SPRN_SPRG0); \
        __ctx.fields.thread_mode && !__ctx.fields.processing_interrupt;})


///  PK is executing an interrupt handler of any priority.

#define __pk_kernel_context_any_interrupt() \
    ({ \
        __PkKernelContext __ctx; \
        __ctx.value = mfspr(SPRN_SPRG0); \
        __ctx.fields.processing_interrupt;})


// PK requires the port to define the type PkThreadQueue, which is a
// priority queue (where 0 is the highest priority).  This queue must be able
// to handle PK_THREADS + 1 priorities (the last for the idle thread) The
// port must also define methods for clearing, insertion, deletion and min
// (with assumed legal priorities).  The min operation returns PK_THREADS if
// the queue is empty (or a queue could be initialized with that entry always
// present - PK code never tries to delete the idle thread from a thread
// queue).
//
// These queues are used both for the run queue and the pending queue
// associated with every semaphore. 
//
// On PPE42 with 32 threads (implied), this is a job for a uint32_t and
// cntlzw(). 

static inline void
__pk_thread_queue_clear(volatile PkThreadQueue *queue)
{
    *queue = 0;
}

static inline void
__pk_thread_queue_insert(volatile PkThreadQueue *queue, PkThreadPriority priority)
{
    *queue |= (0x80000000u >> priority);
}

static inline void
__pk_thread_queue_delete(volatile PkThreadQueue *queue, PkThreadPriority priority)
{
    *queue &= ~(0x80000000u >> priority);
}

static inline PkThreadPriority
__pk_thread_queue_min(volatile PkThreadQueue *queue)
{
    return cntlzw(*queue);
}

static inline int
__pk_thread_queue_member(volatile PkThreadQueue *queue, PkThreadPriority priority)
{
    return ((*queue >> (31 - priority)) & 1);
}

static inline void
__pk_thread_queue_union(volatile PkThreadQueue *queue0, 
                         volatile PkThreadQueue *queue1)
{
    *queue0 |= *queue1;
}

static inline int
__pk_thread_queue_count(volatile PkThreadQueue* queue) 
{
    return __builtin_popcount(*queue);
}


/// This macro is used to call __pk_start_threads() using the kernel stack,
/// in a critical section.

#define __pk_call_pk_start_threads()                          \
    do {                                                        \
        PkMachineContext ctx;                                  \
        pk_critical_section_enter(&ctx);      \
        asm volatile ("mr 1, %0; mtlr %1; blrl" : :             \
                      "r" (__pk_kernel_stack),            \
                      "r" (__pk_start_threads));               \
        PK_PANIC(PK_START_THREADS_RETURNED);                  \
    } while (0)


#endif  /* __ASSEMBLER__ */

/// The __PkKernelContext 'thread_mode' bit as a flag

#define PPE42_THREAD_MODE   0x8000
#define PPE42_PROC_IRQ      0x4000
#define PPE42_DISCARD_CTX   0x0080

#define PPE42_THREAD_MODE_BIT   0
#define PPE42_PROC_IRQ_BIT      1
#define PPE42_DISCARD_CTX_BIT   8

#ifndef __ASSEMBLER__

/// Code breakpoints for PPE42
///
/// This macro inserts a special PPE42-only breakpoint into the object code
/// at the place the macro invocation appears.  This facility is designed for
/// VBU/VPO procedure debugging. This type of breakpoint may not be required
/// on real hardware as we will then have the full power of RISCWatch, gdb,
/// etc. Once inserted into the code, code breakpoints can be enabled or
/// disabled by manipulating the global variable _code_breakpoint_enable,
/// which defaults to 1.
///
/// The code breakpoint is implemented as a setup routine and a teardown
/// routine, executed in an critical section.  The actual break
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
/// that the CODE_BREAKPOINT is ignored by the Simics PPE42 model as it does
/// not model debug events.

//#if defined(SIMICS_ENVIRONMENT) && (SIMICS_ENVIRONMENT != 0) 
/*#define CODE_BREAKPOINT                         \
    do {                                        \
        if (_code_breakpoint_enable) {          \
            SIMICS_MAGIC_BREAKPOINT;            \
        }                                       \
    } while (0)
#else
#define CODE_BREAKPOINT                                         \
    do {                                                        \
        if (_code_breakpoint_enable) {                          \
            PkMachineContext __ctx;                            \
            pk_critical_section_enter(&__ctx);   \
            _code_breakpoint_prologue();                        \
            _code_breakpoint_epilogue();                        \
            pk_critical_section_exit(&__ctx);                  \
        }                                                       \
    } while (0)
#endif
*/

//void
//_code_breakpoint_prologue(void);

//void
//_code_breakpoint_epilogue(void);

//extern uint32_t _code_breakpoint_enable;

#endif // __ASSEMBLER__


#endif /* __PPE42_H__ */
