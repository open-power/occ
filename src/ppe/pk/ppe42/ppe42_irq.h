/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/pk/ppe42/ppe42_irq.h $                                */
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
#ifndef __PPE42_IRQ_H__
#define __PPE42_IRQ_H__
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2014
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file ppe42_irq.h
/// \brief PPE42 interrupt handling for PK
///
/// Interrupt handling protocols and interrupt controller programming are
/// inherently non-portable, however PK defines APIs that may be useful among
/// different machines.
///


// Define pseudo-IRQ numbers for PPE42 built-in interrupts.  These numbers
// will appear in bits 16:23 of SPRG0 (__PkKernelContext) when the handlers
// are active

#define PPE42_EXC_MACHINE_CHECK         0x50
#define PPE42_EXC_DATA_STORAGE          0x53
#define PPE42_EXC_INSTRUCTION_STORAGE   0x54
#define PPE42_EXC_ALIGNMENT             0x56
#define PPE42_EXC_PROGRAM               0x57
#define PPE42_IRQ_DEC                   0x58
#define PPE42_IRQ_FIT                   0x59
#define PPE42_IRQ_WATCHDOG              0x5A


//  Unhandled exceptions default to a kernel panic, but the application can
//  override these definition.  Note that the exception area only allocates 32
//  bytes (8 instructions) to an unhandled exception, so any redefinition
//  would most likely be a branch to an application-defined handler.

#ifndef PPE42_MACHINE_CHECK_HANDLER
#define PPE42_MACHINE_CHECK_HANDLER        PK_PANIC(0x0200)
#endif

#ifndef PPE42_DATA_STORAGE_HANDLER
#define PPE42_DATA_STORAGE_HANDLER         PK_PANIC(0x0300)
#endif

#ifndef PPE42_INSTRUCTION_STORAGE_HANDLER
#define PPE42_INSTRUCTION_STORAGE_HANDLER  PK_PANIC(0x0400)
#endif

#ifndef PPE42_ALIGNMENT_HANDLER
#define PPE42_ALIGNMENT_HANDLER            PK_PANIC(0x0600)
#endif


////////////////////////////////////////////////////////////////////////////
//  PK API
////////////////////////////////////////////////////////////////////////////

#ifndef __ASSEMBLER__

/// An IRQ handler takes 2 arguments:
/// \arg \c arg - Private handler data installed by \c ssx_irq_setup() or
///                   \c ssx_irq_handler_set().
/// \arg \c irq     - The IRQ id; to enable a generic handler to manipulate
///                   its own interrupt status .

typedef void (*PkIrqHandler)(void* arg, PkIrqId irq);

/// Declare a subroutine as an IRQ handler

#define PK_IRQ_HANDLER(f) void f(void* arg, PkIrqId irq)

int pk_irq_setup(PkIrqId irq,
                  int      polarity,
                  int      trigger);

int pk_irq_handler_set(PkIrqId      irq,
                        PkIrqHandler handler,
                        void*         arg);

void pk_irq_enable(PkIrqId irq);
void pk_irq_disable(PkIrqId irq);
void pk_irq_statusclear(PkIrqId irq);

PK_IRQ_HANDLER(__ppe42_default_irq_handler);
PK_IRQ_HANDLER(__ppe42_phantom_irq_handler);


int
ppe42_fit_setup(int tcr_fp, PkIrqHandler handler, void* arg);


///  The address of the optional FIT interrupt handler

UNLESS__PPE42_IRQ_CORE_C__(extern)
volatile
PkIrqHandler __ppe42_fit_routine;


/// The private data of the optional FIT interrupt handler

UNLESS__PPE42_IRQ_CORE_C__(extern)
volatile
void* __ppe42_fit_arg;


int
ppe42_watchdog_setup(int tcr_wp, int tcr_wrc, 
                      PkIrqHandler handler, void* arg);


///  The address of the optional Watchdog interrupt handler

UNLESS__PPE42_IRQ_CORE_C__(extern)
volatile
PkIrqHandler __ppe42_watchdog_routine;


/// The private data of the optional Watchdog interrupt handler

UNLESS__PPE42_IRQ_CORE_C__(extern)
volatile
void* __ppe42_watchdog_arg;


int
ppe42_debug_setup(PkIrqHandler handler, void* arg);


///  The address of the optional Debug interrupt handler

UNLESS__PPE42_IRQ_CORE_C__(extern)
volatile
PkIrqHandler __ppe42_debug_routine;


/// The private data of the optional Watchdog interrupt handler

UNLESS__PPE42_IRQ_CORE_C__(extern)
volatile
void* __ppe42_debug_arg;

#endif  /* __ASSEMBLER__ */

//  It's hard to be portable and get all of the definitions and headers in the
//  correct order.  We need to bring in the system IRQ header here.

#ifdef HWMACRO_GPE
#include "gpe_irq.h"
#else
#ifdef HWMACRO_STD
#include "std_irq.h"
#endif
#endif

/// \page ppe42_irq_macros_page PPE42 PK IRQ Assembler Macros
///
///
        
#ifndef __ASSEMBLER__


///  This structure holds the interrupt handler routine addresses and private
///  data.  Assembler code assumes the given structure layout, so any changes
///  to this structure will need to be reflected down into the interrupt
///  dispatch assembler code.

typedef struct {
    PkIrqHandler handler;
    void         *arg;
} Ppe42IrqHandler;


#ifdef STATIC_IRQ_TABLE

#define IRQ_HANDLER(func, arg) \
    {func, arg},

#define IRQ_HANDLER_DEFAULT \
    {__ppe42_default_irq_handler, 0},

#define EXTERNAL_IRQ_TABLE_END \
    {__ppe42_phantom_irq_handler, 0}\
};

#define EXTERNAL_IRQ_TABLE_START \
    Ppe42IrqHandler __ppe42_irq_handlers[EXTERNAL_IRQS + 1] = \
{

#else

#define EXTERNAL_IRQ_TABLE_START

#define IRQ_HANDLER(func, arg)

#define IRQ_HANDLER_DEFAULT

#define EXTERNAL_IRQ_TABLE_END

#endif /*STATIC_IRQ_TABLE*/

/// Interrupt handlers for real (implemented interrupts) plus one for the phantom interrupt handler
extern Ppe42IrqHandler __ppe42_irq_handlers[EXTERNAL_IRQS + 1];


/// The 'phantom interrupt' handler
///
/// A 'phantom' interrupt occurs when the interrupt handling code in the
/// kernel is entered, but no interrupt is found pending in the controller.
/// This is considered a serious bug, as it indictates a short window
/// condition where a level-sensitive interrupt has been asserted and then
/// quickly deasserted before it can be handled.

UNLESS__PPE42_IRQ_CORE_C__(extern)
Ppe42IrqHandler __ppe42_phantom_irq;
    
#endif  /* __ASSEMBLER__ */

#endif  /* __PPE42_IRQ_H__ */
