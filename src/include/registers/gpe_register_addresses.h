/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/include/registers/gpe_register_addresses.h $              */
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
#ifndef __GPE_REGISTER_ADDRESSES_H__
#define __GPE_REGISTER_ADDRESSES_H__

/// \file gpe_register_addresses.h
/// \brief Symbolic addresses for the GPE unit

// *** WARNING *** - This file is generated automatically, do not edit.

// The addresses defined in this file are all for GPE0.  This macro will convert for specified GPE.
#define OCI_ADDR(_base_addr, _gpe_instance) (_base_addr + (0x10000 * (_gpe_instance)))

#define GPE_OCI_BASE 0xC0000000
#define GPE_OCB_GPETSEL 0xc0000000
#define GPE_OCB_GPEIVPR 0xc0000008
#define GPE_OCB_GPEDBG 0xc0000010
#define GPE_OCB_GPE0STR 0xc0000018
#define GPE_OCB_GPEMACR 0xc0000020
#define GPE_OCB_GPESWPR0 0xc0000028
#define GPE_OCB_GPESWPR1 0xc0000030
#define GPE_OCB_GPEXIXCR 0xc0000080
#define GPE_OCB_GPEXIRAMRA 0xc0000088
#define GPE_OCB_GPEXIRAMGA 0xc0000090
#define GPE_OCB_GPEXIRAMDBG 0xc0000098
#define GPE_OCB_GPEXIRAMEDR 0xc00000a0
#define GPE_OCB_GPEXIDBGPRO 0xc00000a8
#define GPE_OCB_GPEXISIB 0xc00000b0
#define GPE_OCB_GPEXIMEM 0xc00000b8
#define GPE_OCB_GPEXISGB 0xc00000c0
#define GPE_OCB_GPEXIICAC 0xc00000c8
#define GPE_OCB_GPEXIDCAC 0xc00000d0
#define GPE_OCB_GPEXIDBGINF 0xc00000f8
#define GPE_OCB_GPEOXIXCR 0xc0000100
#define GPE_OCB_GPEXIXSR 0xc0000108
#define GPE_OCB_GPEXISPRG0 0xc0000110
#define GPE_OCB_GPEXIEDR 0xc0000118
#define GPE_OCB_GPEXIIR 0xc0000120
#define GPE_OCB_GPEXIIAR 0xc0000128
#define GPE_OCB_GPEXISIBU 0xc0000130
#define GPE_OCB_GPEXISIBL 0xc0000138
#define GPE_OCB_GPEXIMEMU 0xc0000140
#define GPE_OCB_GPEXIMEML 0xc0000148
#define GPE_OCB_GPEXISGBU 0xc0000150
#define GPE_OCB_GPEXISGBL 0xc0000158
#define GPE_OCB_GPEXIICACU 0xc0000160
#define GPE_OCB_GPEXIICACL 0xc0000168
#define GPE_OCB_GPEXIDCACU 0xc0000170
#define GPE_OCB_GPEXIDCACL 0xc0000178
#define GPE_OCB_GPEXISRR0 0xc0000180
#define GPE_OCB_GPEXILR 0xc0000188
#define GPE_OCB_GPEXICTR 0xc0000190
#define GPE_OCB_GPEXIGPR0 0xc0000200
#define GPE_OCB_GPEXIGPR1 0xc0000208
#define GPE_OCB_GPEXIGPR2 0xc0000210
#define GPE_OCB_GPEXIGPR3 0xc0000218
#define GPE_OCB_GPEXIGPR4 0xc0000220
#define GPE_OCB_GPEXIGPR5 0xc0000228
#define GPE_OCB_GPEXIGPR6 0xc0000230
#define GPE_OCB_GPEXIGPR7 0xc0000238
#define GPE_OCB_GPEXIGPR8 0xc0000240
#define GPE_OCB_GPEXIGPR9 0xc0000248
#define GPE_OCB_GPEXIGPR10 0xc0000250
#define GPE_OCB_GPEXIGPR13 0xc0000258
#define GPE_OCB_GPEXIGPR28 0xc0000260
#define GPE_OCB_GPEXIGPR29 0xc0000268
#define GPE_OCB_GPEXIGPR30 0xc0000270
#define GPE_OCB_GPEXIGPR31 0xc0000278
#define GPE_OCB_GPEXIVDR0 0xc0000400
#define GPE_OCB_GPEXIVDR2 0xc0000408
#define GPE_OCB_GPEXIVDR4 0xc0000410
#define GPE_OCB_GPEXIVDR6 0xc0000418
#define GPE_OCB_GPEXIVDR8 0xc0000420
#define GPE_OCB_GPEXIVDRX 0xc0000428
#define GPE_OCB_GPEXIVDR28 0xc0000430
#define GPE_OCB_GPEXIVDR30 0xc0000438
#define GPE_OCB_GPE1STR 0xc0010018
#define GPE_OCB_GPE2STR 0xc0020018
#define GPE_OCB_GPE3STR 0xc0030018

#endif // __GPE_REGISTER_ADDRESSES_H__

