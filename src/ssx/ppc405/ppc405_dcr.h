#ifndef __PPC405_DCR_H__
#define __PPC405_DCR_H__

// $Id: ppc405_dcr.h,v 1.1.1.1 2013/12/11 21:03:27 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/ppc405/ppc405_dcr.h,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file ppc405_dcr.h
/// \brief Everything related to PPC405-specific DCRs
///
/// DCRs are chip-specific.  This file only defines DCR access methods; DCR
/// numbers will be defined by chip-specific headers.

/// Move From DCR
///
///  Note that \a dcrn must be a compile-time constant.

#define mfdcr(dcrn)                                             \
   ({uint32_t __value;                                          \
   asm volatile ("mfdcr %0, %1" : "=r" (__value) : "i" (dcrn)); \
   __value;})    


/// Move to DCR
///
///  Note that \a dcrn must be a compile-time constant.

#define mtdcr(dcrn, value)                                        \
   ({uint32_t __value = (value);                                  \
     asm volatile ("mtdcr %0, %1" : : "i" (dcrn), "r" (__value)); \
   })
    

/// Read-Modify-Write a DCR with OR (Set DCR bits)
///
///  Note that \a dcrn must be a compile-time constant. This operation is only
///  guaranteed atomic in a critical section.

#define or_dcr(dcrn, x) \
    mtdcr(dcrn, mfdcr(dcrn) | (x))


/// Read-Modify-Write a DCR with AND complement (Clear DCR bits)
///
///  Note that \a dcrn must be a compile-time constant.  This operation is only
///  guaranteed atomic in a critical section.

#define andc_dcr(dcrn, x) \
    mtdcr(dcrn, mfdcr(dcrn) & ~(x))

#endif /* __PPC405_DCR_H__ */
