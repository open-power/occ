/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/pk/std/std_irq.h $                                    */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2016                        */
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
#ifndef __STD_IRQ_H__
#define __STD_IRQ_H__

//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2014
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file occhw_irq.h
/// \brief Standard PPE Externnal Interrupt handling for PK
///
/// The standard PPE interrupt controller supports a maximum of 64 interrupts with
/// simple OR combining of the interrupt signals.
///
/// The standard PPE interrupt controller allows interrupt status to be set directly by
/// software.  It contains a 'mask' register, unlike most 405 interrupt
/// controllers that have an 'enable' register.  The standard PPE mask and status
/// registers also have atomic CLR/OR function so that it is never necessary
/// to enter a critical section to enable/disable/clear interrupts and
/// interrupt status.

#include "std_common.h"
#include "std_register_addresses.h"
#include "ppe42.h"

#ifndef __ASSEMBLER__

/// Enable an interrupt by clearing the mask bit.

UNLESS__PPE42_IRQ_CORE_C__(extern)
inline void
pk_irq_enable(PkIrqId irq)
{
    out64(STD_LCL_EIMR_CLR, STD_IRQ_MASK64(irq));
}

/// Enable a vector of interrupts by clearing the mask bits.

UNLESS__PPE42_IRQ_CORE_C__(extern)
inline void
pk_irq_vec_enable(uint64_t irq_vec_mask)
{
    out64(STD_LCL_EIMR_CLR, irq_vec_mask);
}

/// Disable an interrupt by setting the mask bit.

UNLESS__PPE42_IRQ_CORE_C__(extern)
inline void
pk_irq_disable(PkIrqId irq)
{
    out64(STD_LCL_EIMR_OR, STD_IRQ_MASK64(irq));
}

/// Disable a vector of interrupts by setting the mask bits.

UNLESS__PPE42_IRQ_CORE_C__(extern)
inline void
pk_irq_vec_disable(uint64_t irq_vec_mask)
{
    out64(STD_LCL_EIMR_OR, irq_vec_mask);
}


/// Clear interrupt status with an CLR mask.  Only meaningful for
/// edge-triggered interrupts.

UNLESS__PPE42_IRQ_CORE_C__(extern)
inline void
pk_irq_status_clear(PkIrqId irq)
{
    out64(STD_LCL_EISR_CLR, STD_IRQ_MASK64(irq));
}


/// Clear a vector of interrupts status with an CLR mask.  Only meaningful for
/// edge-triggered interrupts.

UNLESS__PPE42_IRQ_CORE_C__(extern)
inline void
pk_irq_vec_status_clear(uint64_t irq_vec_mask)
{
    out64(STD_LCL_EISR_CLR, irq_vec_mask);
}

/// Get IRQ status as a 0 or non-0 integer

UNLESS__PPE42_IRQ_CORE_C__(extern)
inline int
pk_irq_status_get(PkIrqId irq)
{
    return (in64(STD_LCL_EISR) & STD_IRQ_MASK64(irq)) != 0;
}


/// Set or clear interrupt status explicitly.

UNLESS__PPE42_IRQ_CORE_C__(extern)
inline void
pk_irq_status_set(PkIrqId irq, int value)
{
    if (value)
    {
        out64(STD_LCL_EISR_OR, STD_IRQ_MASK64(irq));
    }
    else
    {
        out64(STD_LCL_EISR_CLR, STD_IRQ_MASK64(irq));
    }
}


#endif  /* __ASSEMBLER__ */

#endif /* __STD_IRQ_H__ */
