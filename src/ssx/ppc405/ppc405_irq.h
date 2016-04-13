/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/ppc405/ppc405_irq.h $                                 */
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
#ifndef __PPC405_IRQ_H__
#define __PPC405_IRQ_H__

/// \file ppc405_irq.h
/// \brief PPC405 interrupt handling for SSX
///
/// Interrupt handling protocols and interrupt controller programming are
/// inherently non-portable, however SSX defines APIs that may be useful among
/// different machines.
///
/// The interrupt controllers in PPC405 ASICS and PGP allow interrupts to be
/// programmed as critical or non-critical, with programmable polarity and
/// edge or level sensitivity.


// Define pseudo-IRQ numbers for PPC405 built-in interrupts.  These numbers
// will appear in bits 16:23 of USPRG0 (__SsxKernelContext) when the handlers
// are active, and are also passed as the second argument of the handlers when
// they are invoked.

#define PPC405_IRQ_PIT      0x80
#define PPC405_IRQ_FIT      0x81
#define PPC405_IRQ_WATCHDOG 0x82
#define PPC405_IRQ_DEBUG    0x83

// These are suggested values to use for the IRQ in the __SsxKernelContext if
// the application defines handlers for any of the 'unhandled exceptions'.

#define PPC405_EXC_MACHINE_CHECK         0x90
#define PPC405_EXC_DATA_STORAGE          0x91
#define PPC405_EXC_INSTRUCTION_STORAGE   0x92
#define PPC405_EXC_ALIGNMENT             0x93
#define PPC405_EXC_PROGRAM               0x94
#define PPC405_EXC_FPU_UNAVAILABLE       0x95
#define PPC405_EXC_APU_UNAVAILABLE       0x96
#define PPC405_EXC_DATA_TLB_MISS         0x97
#define PPC405_EXC_INSTRUCTION_TLB_MISS  0x98


//  Unhandled exceptions default to a kernel panic, but the application can
//  override these definition.  Note that the exception area only allocates 32
//  bytes (8 instructions) to an unhandled exception, so any redefinition
//  would most likely be a branch to an application-defined handler.

#ifndef PPC405_MACHINE_CHECK_HANDLER
#define PPC405_MACHINE_CHECK_HANDLER        SSX_PANIC(0x0200)
#endif

#ifndef PPC405_DATA_STORAGE_HANDLER
#define PPC405_DATA_STORAGE_HANDLER         SSX_PANIC(0x0300)
#endif

#ifndef PPC405_INSTRUCTION_STORAGE_HANDLER
#define PPC405_INSTRUCTION_STORAGE_HANDLER  SSX_PANIC(0x0400)
#endif

#ifndef PPC405_ALIGNMENT_HANDLER
#define PPC405_ALIGNMENT_HANDLER            SSX_PANIC(0x0600)
#endif

#ifndef PPC405_PROGRAM_HANDLER
#define PPC405_PROGRAM_HANDLER              SSX_PANIC(0x0700)
#endif

#ifndef PPC405_FPU_UNAVAILABLE_HANDLER
#define PPC405_FPU_UNAVAILABLE_HANDLER      SSX_PANIC(0x0800)
#endif

#ifndef PPC405_APU_UNAVAILABLE_HANDLER
#define PPC405_APU_UNAVAILABLE_HANDLER      SSX_PANIC(0x0f20)
#endif

#ifndef PPC405_DATA_TLB_MISS_HANDLER
#define PPC405_DATA_TLB_MISS_HANDLER        SSX_PANIC(0x1100)
#endif

#ifndef PPC405_INSTRUCTION_TLB_MISS_HANDLER
#define PPC405_INSTRUCTION_TLB_MISS_HANDLER SSX_PANIC(0x1200)
#endif


////////////////////////////////////////////////////////////////////////////
//  SSX API
////////////////////////////////////////////////////////////////////////////

#ifndef __ASSEMBLER__

/// An IRQ handler takes 3 arguments:
/// \arg \c arg - Private handler data installed by \c ssx_irq_setup() or
///                   \c ssx_irq_handler_set().
/// \arg \c irq     - The IRQ id; to enable a generic handler to manipulate
///                   its own interrupt status .
/// \arg \c priority - One of the values \c SSX_CRITICAL or \c
///                    SSX_NONCRITICAL; to enable a generic handler to choose
///                    a behavior appropriate for the interrupt priority. 

typedef void (*SsxIrqHandler)(void*    arg,
                              SsxIrqId irq,
                              int      priority);

/// Declare a subroutine as an IRQ handler

#define SSX_IRQ_HANDLER(f) void f(void *arg, SsxIrqId irq, int priority)

int ssx_irq_setup(SsxIrqId irq,
                  int      polarity,
                  int      trigger);

int ssx_irq_handler_set(SsxIrqId      irq,
                        SsxIrqHandler handler,
                        void*         arg,
                        int           priority);

void ssx_irq_enable(SsxIrqId irq);
void ssx_irq_disable(SsxIrqId irq);
void ssx_irq_statusclear(SsxIrqId irq);

SSX_IRQ_HANDLER(__ppc405_default_irq_handler);
SSX_IRQ_HANDLER(__ppc405_phantom_irq_handler);


int
ppc405_fit_setup(int tcr_fp, SsxIrqHandler handler, void* arg);


///  The address of the optional FIT interrupt handler

UNLESS__PPC405_IRQ_CORE_C__(extern)
volatile
SsxIrqHandler __ppc405_fit_routine;


/// The private data of the optional FIT interrupt handler

UNLESS__PPC405_IRQ_CORE_C__(extern)
volatile
void* __ppc405_fit_arg;


int
ppc405_watchdog_setup(int tcr_wp, int tcr_wrc, 
                      SsxIrqHandler handler, void* arg);


///  The address of the optional Watchdog interrupt handler

UNLESS__PPC405_IRQ_CORE_C__(extern)
volatile
SsxIrqHandler __ppc405_watchdog_routine;


/// The private data of the optional Watchdog interrupt handler

UNLESS__PPC405_IRQ_CORE_C__(extern)
volatile
void* __ppc405_watchdog_arg;


int
ppc405_debug_setup(SsxIrqHandler handler, void* arg);


///  The address of the optional Debug interrupt handler

UNLESS__PPC405_IRQ_CORE_C__(extern)
volatile
SsxIrqHandler __ppc405_debug_routine;


/// The private data of the optional Watchdog interrupt handler

UNLESS__PPC405_IRQ_CORE_C__(extern)
volatile
void* __ppc405_debug_arg;


// Note: Why SSX_IRQ_FAST2FULL (below) is implemented so strangely.
//
// I am adamant that I want to have a a macro in the 'C' environment to create
// these bridge functions. However the limitations of the C preprocessor and
// the intelligence of the GCC 'asm' facility consipre against a
// straightforward solution.  The only way that I was able to find to get
// naked assembly code into the output stream is to use 'asm' with simple
// strings - I couldn't make it work with any kind of argument, as 'asm' would
// reinterpret the arguments and resulting assembler code in various ways.  
//
// There is another alternative that I tried wherby I created a subroutine
// call and then filled in the subroutine body with 'asm' code.  However, the
// subroutine wrapper that GCC creates only works for PowerPC fast-mode
// handlers if GCC is invoked with optimization, which ensures that the
// wrapper doesn't touch the stack pointer or other registers. True, we'll
// always use optimization, but I did not want to have to make this
// requirement for using this macro.

///  This macro creates a 'bridge' handler that converts the initial fast-mode
///  IRQ dispatch into a call of a full-mode IRQ handler. The full-mode
///  handler is defined by the user (presumably as a \c C subroutine) and has
///  the same prototype (type SsxIrqHandler) as the fast handler.
///
///  \param fast_handler This will be the global function name of the fast
///                      IRQ handler created by this macro. This is the symbol
///                      that should be passed in as the \a handler argument
///                      of \c ssx_irq_setup() and \c ssx_irq_handler_set().
///
///  \param full_handler This is the name of the user-defined full-mode
///                      handler which is invoked through this bridge.
///
/// \e BUG \e ALERT : Beware of passing the \c full_handler to IRQ setup
/// APIs. This won't be caught by the compiler (because the \c full_handler
/// has the correct prototype) and will lead to nasty bugs.  Always pass in
/// the \c fast_handler symbol to IRQ setup APIS.
///
/// The code stream injected into the GCC assembler output in response to
///
/// SSX_IRQ_FAST2FULL(fast_handler, full_handler)
/// 
/// is (comments added for clarification) :
///
/// \code
/// .text 
/// .global fast_handler 
/// .align 5                   # Hard-coded PPC405 cache-line alignment
/// fast_handler = .           # Can't macro expand LABEL: - this is equivalent 
/// bl __ssx_irq_fast2full     # The fast-mode to full-mode conversion sequence 
/// bl full_handler 
/// b  __ssx_irq_full_mode_exit 
/// \endcode
///
/// The macro also declares the prototype of the fast handler:
///
/// \code
/// SSX_IRQ_HANDLER(fast_handler);
/// \endcode
///

#define SSX_IRQ_FAST2FULL(fast_handler, full_handler) \
    SSX_IRQ_HANDLER(fast_handler); \
    __SSX_IRQ_FAST2FULL(.global fast_handler, fast_handler = ., bl full_handler)

#define __SSX_IRQ_FAST2FULL(global, label, call) \
asm(".text"); \
asm(#global); \
asm(".align 5"); \
asm(#label); \
asm("bl __ssx_irq_fast2full"); \
asm(#call); \
asm("b __ssx_irq_full_mode_exit");

#endif  /* __ASSEMBLER__ */

//  It's hard to be portable and get all of the definitions and headers in the
//  correct order.  We need to bring in the system IRQ header here.

#ifdef CHIP_PGP
#include "pgp_irq.h"
#endif

/// \page ppc405_irq_macros_page PPC405 SSX IRQ Assembler Macros
///
///
/// \section fast2full_asm Fast-Mode to Full-Mode Handler Conversion
///
/// This macro produces the calling sequence required to convert a
/// fast-mode interrupt handler to a full-mode interrupt handler. The
/// full-mode handler is implemented by another subroutine.  The
/// requirements for invoking this macro are:
///
/// \li The stack pointer and stack must be exactly as they were when the
/// fast-mode handler was entered.
///
/// \li No changes have been made to the MSR - the interrupt level must
///     remain disabled.
///
/// \li The handler owns the fast context and has not modified the other
///     register context.  The conversion process will not modify any
///     register in the fast context (other than the LR used for
///     subroutine linkage).
///
/// The final condition above means that the \a full_handler will
/// begin with the fast-mode context exactly as it was (save for LR)
/// at conversion, including the contents of GPR3-7 (the first 5
/// PowerPC ABI paramater passing registers) and the entire CR.
///
/// Forms:
///
/// \c _ssx_irq_fast2full \a full_handler
/// \cond

#ifdef __ASSEMBLER__

        .macro  _ssx_irq_fast2full full_handler
        bl      __ssx_irq_fast2full
        bl      \full_handler
        b       __ssx_irq_full_mode_exit
        .endm     

#endif  /* __ASSEMBLER__ */

/// \endcond
        
#ifndef __ASSEMBLER__


///  This structure holds the interrupt handler routine addresses and private
///  data.  Assembler code assumes the given structure layout, so any changes
///  to this structure will need to be reflected down into the interrupt
///  dispatch assembler code.

typedef struct {
    SsxIrqHandler handler;
    void          *arg;
} Ppc405IrqHandler;


/// Interrupt handlers for real (implemented interrupts)

UNLESS__PPC405_IRQ_CORE_C__(extern)
Ppc405IrqHandler __ppc405_irq_handlers[PPC405_IRQS];


/// The 'phantom interrupt' handler
///
/// A 'phantom' interrupt occurs when the interrupt handling code in the
/// kernel is entered, but no interrupt is found pending in the controller.
/// This is considered a serious bug, as it indictates a short window
/// condition where a level-sensitive interrupt has been asserted and then
/// quickly deasserted before it can be handled.

UNLESS__PPC405_IRQ_CORE_C__(extern)
Ppc405IrqHandler __ppc405_phantom_irq;
    
#endif  /* __ASSEMBLER__ */

#endif  /* __PPC405_IRQ_H__ */
