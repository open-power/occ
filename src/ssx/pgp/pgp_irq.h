/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/pgp/pgp_irq.h $                                       */
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
#ifndef __PGP_IRQ_H__
#define __PGP_IRQ_H__

/// \file pgp_irq.h
/// \brief PPC405-PgP Interrupt handling for SSX
///
/// The PgP interrupt controller supports a maximum of 64 interrupts, split
/// into 2 x 32-bit non-cascaded interrupt controllers with simple OR
/// combining of the interrupt signals.
///
/// The PGP interrupt controller allows interrupt status to be set directly by
/// software, as well as providing a mode that causes an enabled pending
/// interrupt to trigger an Unconditional Debug Event. The PGP interrupt
/// controller contains a 'mask' register, unlike other 405 interrupt
/// controllers that have an 'enable' register.  The PgP mask and status
/// registers also have atomic AND/OR function so that it is never necessary
/// to enter a critical section to enable/disable/clear interrupts and
/// interrupt status.

#include "pgp_common.h"
#include "ocb_register_addresses.h"

#ifndef __ASSEMBLER__

/// Enable an interrupt by clearing the mask bit.  

UNLESS__PPC405_IRQ_CORE_C__(extern)
inline void
ssx_irq_enable(SsxIrqId irq)
{
    out32(OCB_OIMR_AND(irq), ~PGP_IRQ_MASK32(irq));
}


/// Disable an interrupt by setting the mask bit.

UNLESS__PPC405_IRQ_CORE_C__(extern)
inline void
ssx_irq_disable(SsxIrqId irq)
{
    out32(OCB_OIMR_OR(irq), PGP_IRQ_MASK32(irq));
}


/// Clear interrupt status with an AND mask.  Only meaningful for
/// edge-triggered interrupts.

UNLESS__PPC405_IRQ_CORE_C__(extern)
inline void
ssx_irq_status_clear(SsxIrqId irq)
{
    out32(OCB_OISR_AND(irq), ~PGP_IRQ_MASK32(irq));
}


/// Get IRQ status as a 0 or non-0 integer

UNLESS__PPC405_IRQ_CORE_C__(extern)
inline int
ssx_irq_status_get(SsxIrqId irq)
{
    return (in32(OCB_OISR(irq)) & PGP_IRQ_MASK32(irq)) != 0;
}


/// Set or clear interrupt status explicitly.

UNLESS__PPC405_IRQ_CORE_C__(extern)
inline void
ssx_irq_status_set(SsxIrqId irq, int value)
{
    if (value) {
        out32(OCB_OISR_OR(irq), PGP_IRQ_MASK32(irq));
    } else {
        out32(OCB_OISR_AND(irq), ~PGP_IRQ_MASK32(irq));
    }
}


void
ssx_irq_debug_set(SsxIrqId irq, int value);

#endif  /* __ASSEMBLER__ */

/// \page pgp_irq_macros PgP IRQ API Assembler Macros
///
/// These macros encapsulate the SSX API for the PgP interrupt
/// controller. These macros require 2 scratch registers in addition to the \c
/// irq parameter register passed into the handler from SSX interrupt
/// dispatch. These macros also modift CR0.
///
/// \arg \c rirq A register that holds the \c irq parameter passed to
///              the handler from SSX interrupt dispatch.  This register is not
///              modified. 
/// \arg \c rmask A scratch register - At the end of macro execution this
///               register contains the 32-bit mask form of the irq.
///
/// \arg \c raddr A scratch register - At the end of macro execution this
///               register holds the address of the interrupt 
///               controller facility that implements the action.
///
/// \arg \c imm An immediate (0/non-0) value for certain macros.
///
/// Forms:
/// 
/// \b _ssx_irq_enable \a rirq, \a rmask, \a raddr - Enable an \c irq. \n
/// \b _ssx_irq_disable \a rirq, \a rmask, \a raddr - Disable an \c irq. \n
/// \b _ssx_irq_status_clear \a rirq, \a rmask, \a raddr - Clear \c irq
///                          interrupt status. \n
/// \b _ssx_irq_status_set \a rirq, \a rmask, \a raddr, \a imm - Set \c irq status
///                        with an immediate (0/non-0) value. \n
///
/// \todo Once the logic design is locked down, revisit whether these macros
/// (and C-code versions) can be implemented without branching.  This could be
/// done in theory by converting bit 26 into the byte offset between addresses
/// in interupt controller 0 and interrupt controller 1 - assuming the
/// distances are all the same power-of-two.
///
/// \cond

// IRQ numbers are in the range 0..63.  IRQs are converted to the 32-bit
// residue used to compute the mask.  CR0 is set as a test of IRQ > 32 - the
// register \c raddr is used as scratch for these computations.  Hopefully the
// local labels 888 and 999 are unique enough.

// Register names must be compared as strings - e.g., %r0 is not 
// a symbol, it is converted to "0" by the assembler.

#ifdef __ASSEMBLER__

        .macro  .two_unique, ra, rb
        .ifnc   \ra, \rb
        .exitm
        .endif
        .error  "Both register arguments must be unique"
        .endm


        .macro  .three_unique, ra, rb, rc
        .ifnc   \ra, \rb
        .ifnc   \rb, \rc
        .ifnc   \ra, \rc
        .exitm
        .endif
        .endif
        .endif
        .error  "All three register arguments must be unique"
        .endm


        .macro  _pgp_irq_or_mask, rirq:req, rmask:req
        .two_unique \rirq, \rmask
        lis     \rmask, 0x8000
        srw     \rmask, \rmask, \rirq
        .endm

        .macro  _pgp_irq_and_mask, rirq:req, rmask:req
        .two_unique \rirq, \rmask
        _pgp_irq_or_mask \rirq, \rmask
        not     \rmask, \rmask
        .endm


        .macro  _ssx_irq_enable, rirq:req, rmask:req, raddr:req
        .three_unique \rirq, \rmask, \raddr
        
        andi.   \raddr, \rirq, 0x20
        clrlwi  \raddr, \rirq, 27
        _pgp_irq_and_mask \raddr, \rmask
        bne-    888f
        _stwi   \rmask, \raddr, OCB_OIMR0_AND
        b       999f
888:    
        _stwi   \rmask, \raddr, OCB_OIMR1_AND
999:            
        eieio
        .endm


        .macro  _ssx_irq_disable, rirq:req, rmask:req, raddr:req
        .three_unique \rirq, \rmask, \raddr
        
        andi.   \raddr, \rirq, 0x20
        clrlwi  \raddr, \rirq, 27
        _pgp_irq_or_mask \raddr, \rmask
        bne-    888f
        _stwi   \rmask, \raddr, OCB_OIMR0_OR
        b       999f
888:    
        _stwi   \rmask, \raddr, OCB_OIMR1_OR
999:    
        eieio
        .endm


        .macro  _ssx_irq_status_clear, rirq:req, rmask:req, raddr:req
        .three_unique \rirq, \rmask, \raddr
        
        andi.   \raddr, \rirq, 0x20
        clrlwi  \raddr, \rirq, 27
        _pgp_irq_and_mask \raddr, \rmask
        bne-    888f
        _stwi   \rmask, \raddr, OCB_OISR0_AND
        b       999f
888:
        _stwi   \rmask, \raddr, OCB_OISR1_AND
999:            
        eieio
        .endm


        .macro  _ssx_irq_status_set, rirq:req, rmask:req, raddr:req, imm:req
        .three_unique \rirq, \rmask, \raddr

        andi.   \raddr, \rirq, 0x20
        clrlwi  \raddr, \rirq, 27
        
        .if     \imm
        _pgp_irq_or_mask \raddr, \rmask
        bne-    888f
        _stwi   \rmask, \raddr, OCB_OISR0_OR
        b       999f
888:    
        _stwi   \rmask, \raddr, OCB_OISR1_OR

        .else
        
        _pgp_irq_and_mask \raddr, \rmask
        bne-    888f
        _stwi   \rmask, \raddr, OCB_OISR0_AND
        b       999f
888:
        _stwi   \rmask, \raddr, OCB_OISR1_AND
        .endif

999:    
        eieio
        .endm

#endif  /* __ASSEMBLER__ */

/// \endcond

#endif /* __PGP_IRQ_H__ */
