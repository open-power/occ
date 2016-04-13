/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/ppc405/ppc405_irq_core.c $                            */
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
/// \file ppc405_irq_core.c
/// \brief Core IRQ routines required of any PPC405 configuration of SSX
///
///  This file is mostly only a placeholder - where 'extern inline' API
///  functions and 'extern' variables are realized.  A couple of default
///  handlers are also installed here.  The entry points in this file are
///  considered 'core' routines that will always be present at runtime in any
///  SSX application.

#define __PPC405_IRQ_CORE_C__

#include "ssx.h"

/// This function is installed by default for interrupts not explicitly set up
/// by the application.  These interrupts should never fire.

void 
__ppc405_default_irq_handler(void     *arg,
                             SsxIrqId irq,
                             int      critical)
{
    SSX_PANIC(PPC405_DEFAULT_IRQ_HANDLER);
}


/// This function is installed by default to handle the case that the
/// interrupt dispatch code is entered in response to an external critical or
/// non-critical interrupt, but no interrupt is found pending in the interrupt
/// controller.  This should never happen, as it would indicate that a
/// 'glitch' occurred on the external noncritical or critical interrupt input
/// to the PPC405 core.

void __ppc405_phantom_irq_handler(void     *arg,
                                  SsxIrqId irq,
                                  int      critical)
{
    SSX_PANIC(PPC405_PHANTOM_INTERRUPT);
}
    

#undef __PPC405_IRQ_CORE_C__
