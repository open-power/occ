/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/include/registers/pba_register_addresses.h $              */
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
#ifndef __PBA_REGISTER_ADDRESSES_H__
#define __PBA_REGISTER_ADDRESSES_H__

// $Id: pba_register_addresses.h,v 1.1.1.1 2013/12/11 21:03:23 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/pgp/registers/pba_register_addresses.h,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file pba_register_addresses.h
/// \brief Symbolic addresses for the PBA unit

// *** WARNING *** - This file is generated automatically, do not edit.


#define TRUSTEDPIB_BASE 0x02013f00
#define PBA_BARN(n) (PBA_BAR0 + ((PBA_BAR1 - PBA_BAR0) * (n)))
#define PBA_BAR0 0x02013f00
#define PBA_BAR1 0x02013f01
#define PBA_BAR2 0x02013f02
#define PBA_BAR3 0x02013f03
#define PBA_BARMSKN(n) (PBA_BARMSK0 + ((PBA_BARMSK1 - PBA_BARMSK0) * (n)))
#define PBA_BARMSK0 0x02013f04
#define PBA_BARMSK1 0x02013f05
#define PBA_BARMSK2 0x02013f06
#define PBA_BARMSK3 0x02013f07
#define PIB_BASE 0x02010840
#define PBA_FIR 0x02010840
#define PBA_FIR_AND 0x02010841
#define PBA_FIR_OR 0x02010842
#define PBA_FIRMASK 0x02010843
#define PBA_FIRMASK_AND 0x02010844
#define PBA_FIRMASK_OR 0x02010845
#define PBA_FIRACT0 0x02010846
#define PBA_FIRACT1 0x02010847
#define PBA_OCCACT 0x0201084a
#define PBA_CFG 0x0201084b
#define PBA_ERRPT0 0x0201084c
#define PBA_ERRPT1 0x0201084d
#define PBA_ERRPT2 0x0201084e
#define PBA_RBUFVALN(n) (PBA_RBUFVAL0 + ((PBA_RBUFVAL1 - PBA_RBUFVAL0) * (n)))
#define PBA_RBUFVAL0 0x02010850
#define PBA_RBUFVAL1 0x02010851
#define PBA_RBUFVAL2 0x02010852
#define PBA_RBUFVAL3 0x02010853
#define PBA_RBUFVAL4 0x02010854
#define PBA_RBUFVAL5 0x02010855
#define PBA_WBUFVALN(n) (PBA_WBUFVAL0 + ((PBA_WBUFVAL1 - PBA_WBUFVAL0) * (n)))
#define PBA_WBUFVAL0 0x02010858
#define PBA_WBUFVAL1 0x02010859
#define OCI_BASE 0x40020000
#define PBA_MODE 0x40020000
#define PBA_SLVRST 0x40020008
#define PBA_SLVCTLN(n) (PBA_SLVCTL0 + ((PBA_SLVCTL1 - PBA_SLVCTL0) * (n)))
#define PBA_SLVCTL0 0x40020020
#define PBA_SLVCTL1 0x40020028
#define PBA_SLVCTL2 0x40020030
#define PBA_SLVCTL3 0x40020038
#define PBA_BCDE_CTL 0x40020080
#define PBA_BCDE_SET 0x40020088
#define PBA_BCDE_STAT 0x40020090
#define PBA_BCDE_PBADR 0x40020098
#define PBA_BCDE_OCIBAR 0x400200a0
#define PBA_BCUE_CTL 0x400200a8
#define PBA_BCUE_SET 0x400200b0
#define PBA_BCUE_STAT 0x400200b8
#define PBA_BCUE_PBADR 0x400200c0
#define PBA_BCUE_OCIBAR 0x400200c8
#define PBA_PBOCRN(n) (PBA_PBOCR0 + ((PBA_PBOCR1 - PBA_PBOCR0) * (n)))
#define PBA_PBOCR0 0x400200d0
#define PBA_PBOCR1 0x400200d8
#define PBA_PBOCR2 0x400200e0
#define PBA_PBOCR3 0x400200e8
#define PBA_PBOCR4 0x400200f0
#define PBA_PBOCR5 0x400200f8
#define PBA_XSNDTX 0x40020100
#define PBA_XCFG 0x40020108
#define PBA_XSNDSTAT 0x40020110
#define PBA_XSNDDAT 0x40020118
#define PBA_XRCVSTAT 0x40020120
#define PBA_XSHBRN(n) (PBA_XSHBR0 + ((PBA_XSHBR1 - PBA_XSHBR0) * (n)))
#define PBA_XSHBR0 0x40020130
#define PBA_XSHBR1 0x40020150
#define PBA_XSHCSN(n) (PBA_XSHCS0 + ((PBA_XSHCS1 - PBA_XSHCS0) * (n)))
#define PBA_XSHCS0 0x40020138
#define PBA_XSHCS1 0x40020158
#define PBA_XSHINCN(n) (PBA_XSHINC0 + ((PBA_XSHINC1 - PBA_XSHINC0) * (n)))
#define PBA_XSHINC0 0x40020140
#define PBA_XSHINC1 0x40020160

#endif // __PBA_REGISTER_ADDRESSES_H__

