/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/include/registers/pba_register_addresses.h $              */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2019                        */
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

/// \file pba_register_addresses.h
/// \brief Symbolic addresses for the PBA unit

// *** WARNING *** - This file is generated automatically, do not edit.


#define PBA_OCI_BASE 0xC0040000
#define PBA_MODE 0xC0040000
#define PBA_SLVRST 0xC0040008
#define PBA_SLVCTLN(n) (PBA_SLVCTL0 + ((PBA_SLVCTL1 - PBA_SLVCTL0) * (n)))
#define PBA_SLVCTL0 0xC0040020
#define PBA_SLVCTL1 0xC0040028
#define PBA_SLVCTL2 0xC0040030
#define PBA_SLVCTL3 0xC0040038
#define PBA_BCDE_CTL 0xC0040080
#define PBA_BCDE_SET 0xC0040088
#define PBA_BCDE_STAT 0xC0040090
#define PBA_BCDE_DR 0xC0040098
#define PBA_BCDE_OCIBAR 0xC00400a0
#define PBA_BCUE_CTL 0xC00400a8
#define PBA_BCUE_SET 0xC00400b0
#define PBA_BCUE_STAT 0xC00400b8
#define PBA_BCUE_DR 0xC00400c0
#define PBA_BCUE_OCIBAR 0xC00400c8
#define PBA_OCRN(n) (PBA_OCR0 + ((PBA_OCR1 - PBA_OCR0) * (n)))
#define PBA_OCR0 0xC00400d0
#define PBA_OCR1 0xC00400d8
#define PBA_OCR2 0xC00400e0
#define PBA_OCR3 0xC00400e8
#define PBA_XSNDTX 0xC0040100
#define PBA_XCFG 0xC0040108
#define PBA_XSNDSTAT 0xC0040110
#define PBA_XSNDDAT 0xC0040118
#define PBA_XRCVSTAT 0xC0040120
#define PBA_XSHBRN(n) (PBA_XSHBR0 + ((PBA_XSHBR1 - PBA_XSHBR0) * (n)))
#define PBA_XSHBR0 0xC0040130
#define PBA_XSHBR1 0xC0040150
#define PBA_XSHCSN(n) (PBA_XSHCS0 + ((PBA_XSHCS1 - PBA_XSHCS0) * (n)))
#define PBA_XSHCS0 0xC0040138
#define PBA_XSHCS1 0xC0040158
#define PBA_XSHINCN(n) (PBA_XSHINC0 + ((PBA_XSHINC1 - PBA_XSHINC0) * (n)))
#define PBA_XSHINC0 0xC0040140
#define PBA_XSHINC1 0xC0040160
#define PBA_XISNDTX 0xC0040180
#define PBA_XICFG 0xC0040188
#define PBA_XISNDSTAT 0xC0040190
#define PBA_XISNDDAT 0xC0040198
#define PBA_XIRCVSTAT 0xC00401a0
#define PBA_XISHBRN(n) (PBA_XISHBR0 + ((PBA_XISHBR1 - PBA_XISHBR0) * (n)))
#define PBA_XISHBR0 0xC00401b0
#define PBA_XISHBR1 0xC00401d0
#define PBA_XISHCSN(n) (PBA_XISHCS0 + ((PBA_XISHCS1 - PBA_XISHCS0) * (n)))
#define PBA_XISHCS0 0xC00401b8
#define PBA_XISHCS1 0xC00401d8
#define PBA_XISHINCN(n) (PBA_XISHINC0 + ((PBA_XISHINC1 - PBA_XISHINC0) * (n)))
#define PBA_XISHINC0 0xC00401c0
#define PBA_XISHINC1 0xC00401e0
#define PBA_PBAF_BASE 0x03011dc0
#define PBA_PBAF_FIR 0x03011dc0
#define PBA_PBAF_FIR_AND 0x03011dc1
#define PBA_PBAF_FIR_OR 0x03011dc2
#define PBA_PBAF_FIRMASK 0x03011dc3
#define PBA_PBAF_FIRMASK_AND 0x03011dc4
#define PBA_PBAF_FIRMASK_OR 0x03011dc5
#define PBA_PBAF_FIRACT0 0x03011dc6
#define PBA_PBAF_FIRACT1 0x03011dc7
#define PBA_PBAF_OCCACT 0x03011dca
#define PBA_PBAF_OCFG 0x03011dcb
#define PBA_PBAF_ERRRPT0 0x03011dcc
#define PBA_PBAF_ERRRPT1 0x03011dcd
#define PBA_PBAF_ERRRPT2 0x03011dce
#define PBA_PIB_BASE 0x01010cc0
#define PBA_FIR 0x01010cc0
#define PBA_FIR_AND 0x01010cc1
#define PBA_FIR_OR 0x01010cc2
#define PBA_FIRMASK 0x01010cc3
#define PBA_FIRMASK_AND 0x01010cc4
#define PBA_FIRMASK_OR 0x01010cc5
#define PBA_FIRACT0 0x01010cc6
#define PBA_FIRACT1 0x01010cc7
#define PBA_OCCACT 0x01010cca
#define PBA_OCFG 0x01010ccb
#define PBA_ERRRPT0 0x01010ccc
#define PBA_ERRRPT1 0x01010ccd
#define PBA_ERRRPT2 0x01010cce
#define PBA_RBUFVALN(n) (PBA_RBUFVAL0 + ((PBA_RBUFVAL1 - PBA_RBUFVAL0) * (n)))
#define PBA_RBUFVAL0 0x01010cd0
#define PBA_RBUFVAL1 0x01010cd1
#define PBA_RBUFVAL2 0x01010cd2
#define PBA_RBUFVAL3 0x01010cd3
#define PBA_RBUFVAL4 0x01010cd4
#define PBA_RBUFVAL5 0x01010cd5
#define PBA_WBUFVALN(n) (PBA_WBUFVAL0 + ((PBA_WBUFVAL1 - PBA_WBUFVAL0) * (n)))
#define PBA_WBUFVAL0 0x01010cd8
#define PBA_WBUFVAL1 0x01010cd9
#define PBA_BARN(n) (PBA_BAR0 + ((PBA_BAR1 - PBA_BAR0) * (n)))
#define PBA_BAR0 0x01010cda
#define PBA_BAR1 0x01010cdb
#define PBA_BAR2 0x01010cdc
#define PBA_BAR3 0x01010cdd
#define PBA_BARMSKN(n) (PBA_BARMSK0 + ((PBA_BARMSK1 - PBA_BARMSK0) * (n)))
#define PBA_BARMSK0 0x01010cde
#define PBA_BARMSK1 0x01010cdf
#define PBA_BARMSK2 0x01010ce0
#define PBA_BARMSK3 0x01010ce1
#define PBA_PBTXTN(n) (PBA_PBTXT0 + ((PBA_PBTXT1 - PBA_PBTXT0) * (n)))
#define PBA_PBTXT0 0x01010ce2
#define PBA_PBTXT1 0x01010ce3
#define PBA_PBTXT2 0x01010ce4
#define PBA_PBTXT3 0x01010ce5





#endif // __PBA_REGISTER_ADDRESSES_H__

