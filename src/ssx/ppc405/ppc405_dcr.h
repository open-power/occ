/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/ppc405/ppc405_dcr.h $                                 */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2014,2015                        */
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
#ifndef __PPC405_DCR_H__
#define __PPC405_DCR_H__

//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2014
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
