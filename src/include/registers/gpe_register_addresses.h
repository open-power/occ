/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/include/registers/gpe_register_addresses.h $              */
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
#ifndef __GPE_REGISTER_ADDRESSES_H__
#define __GPE_REGISTER_ADDRESSES_H__

/// \file gpe_register_addresses.h
/// \brief Symbolic addresses for the GPE unit

// *** WARNING *** - This file is generated automatically, do not edit.


#define GPE_OCI_BASE 0xC0000000
#define GPE_GPENTSEL(n) (GPE_GPE0TSEL + ((GPE_GPE1TSEL - GPE_GPE0TSEL) * (n)))
#define GPE_GPE0TSEL 0xc0000000
#define GPE_GPE1TSEL 0xc0010000
#define GPE_GPE2TSEL 0xc0020000
#define GPE_GPE3TSEL 0xc0030000
#define GPE_GPENIVPR(n) (GPE_GPE0IVPR + ((GPE_GPE1IVPR - GPE_GPE0IVPR) * (n)))
#define GPE_GPE0IVPR 0xc0000008
#define GPE_GPE1IVPR 0xc0010008
#define GPE_GPE2IVPR 0xc0020008
#define GPE_GPE3IVPR 0xc0030008
#define GPE_GPENDBG(n) (GPE_GPE0DBG + ((GPE_GPE1DBG - GPE_GPE0DBG) * (n)))
#define GPE_GPE0DBG 0xc0000010
#define GPE_GPE1DBG 0xc0010010
#define GPE_GPE2DBG 0xc0020010
#define GPE_GPE3DBG 0xc0030010
#define GPE_GPENSTR(n) (GPE_GPE0STR + ((GPE_GPE1STR - GPE_GPE0STR) * (n)))
#define GPE_GPE0STR 0xc0000018
#define GPE_GPE1STR 0xc0010018
#define GPE_GPE2STR 0xc0020018
#define GPE_GPE3STR 0xc0030018
#define GPE_GPENMACR(n) (GPE_GPE0MACR + ((GPE_GPE1MACR - GPE_GPE0MACR) * (n)))
#define GPE_GPE0MACR 0xc0000020
#define GPE_GPE1MACR 0xc0010020
#define GPE_GPE2MACR 0xc0020020
#define GPE_GPE3MACR 0xc0030020
#define GPE_GPENXIXCR(n) (GPE_GPE0XIXCR + ((GPE_GPE1XIXCR - GPE_GPE0XIXCR) * (n)))
#define GPE_GPE0XIXCR 0xc0000080
#define GPE_GPE1XIXCR 0xc0010080
#define GPE_GPE2XIXCR 0xc0020080
#define GPE_GPE3XIXCR 0xc0030080
#define GPE_GPENXIRAMRA(n) (GPE_GPE0XIRAMRA + ((GPE_GPE1XIRAMRA - GPE_GPE0XIRAMRA) * (n)))
#define GPE_GPE0XIRAMRA 0xc0000088
#define GPE_GPE1XIRAMRA 0xc0010088
#define GPE_GPE2XIRAMRA 0xc0020088
#define GPE_GPE3XIRAMRA 0xc0030088
#define GPE_GPENXIRAMGA(n) (GPE_GPE0XIRAMGA + ((GPE_GPE1XIRAMGA - GPE_GPE0XIRAMGA) * (n)))
#define GPE_GPE0XIRAMGA 0xc0000090
#define GPE_GPE1XIRAMGA 0xc0010090
#define GPE_GPE2XIRAMGA 0xc0020090
#define GPE_GPE3XIRAMGA 0xc0030090
#define GPE_GPENXIRAMDBG(n) (GPE_GPE0XIRAMDBG + ((GPE_GPE1XIRAMDBG - GPE_GPE0XIRAMDBG) * (n)))
#define GPE_GPE0XIRAMDBG 0xc0000098
#define GPE_GPE1XIRAMDBG 0xc0010098
#define GPE_GPE2XIRAMDBG 0xc0020098
#define GPE_GPE3XIRAMDBG 0xc0030098
#define GPE_GPENXIRAMEDR(n) (GPE_GPE0XIRAMEDR + ((GPE_GPE1XIRAMEDR - GPE_GPE0XIRAMEDR) * (n)))
#define GPE_GPE0XIRAMEDR 0xc00000a0
#define GPE_GPE1XIRAMEDR 0xc00100a0
#define GPE_GPE2XIRAMEDR 0xc00200a0
#define GPE_GPE3XIRAMEDR 0xc00300a0
#define GPE_GPENXIDBGPRO(n) (GPE_GPE0XIDBGPRO + ((GPE_GPE1XIDBGPRO - GPE_GPE0XIDBGPRO) * (n)))
#define GPE_GPE0XIDBGPRO 0xc00000a8
#define GPE_GPE1XIDBGPRO 0xc00100a8
#define GPE_GPE2XIDBGPRO 0xc00200a8
#define GPE_GPE3XIDBGPRO 0xc00300a8
#define GPE_GPENXISIB(n) (GPE_GPE0XISIB + ((GPE_GPE1XISIB - GPE_GPE0XISIB) * (n)))
#define GPE_GPE0XISIB 0xc00000b0
#define GPE_GPE1XISIB 0xc00100b0
#define GPE_GPE2XISIB 0xc00200b0
#define GPE_GPE3XISIB 0xc00300b0
#define GPE_GPENXIMEM(n) (GPE_GPE0XIMEM + ((GPE_GPE1XIMEM - GPE_GPE0XIMEM) * (n)))
#define GPE_GPE0XIMEM 0xc00000b8
#define GPE_GPE1XIMEM 0xc00100b8
#define GPE_GPE2XIMEM 0xc00200b8
#define GPE_GPE3XIMEM 0xc00300b8
#define GPE_GPENXISGB(n) (GPE_GPE0XISGB + ((GPE_GPE1XISGB - GPE_GPE0XISGB) * (n)))
#define GPE_GPE0XISGB 0xc00000c0
#define GPE_GPE1XISGB 0xc00100c0
#define GPE_GPE2XISGB 0xc00200c0
#define GPE_GPE3XISGB 0xc00300c0
#define GPE_GPENXIICAC(n) (GPE_GPE0XIICAC + ((GPE_GPE1XIICAC - GPE_GPE0XIICAC) * (n)))
#define GPE_GPE0XIICAC 0xc00000c8
#define GPE_GPE1XIICAC 0xc00100c8
#define GPE_GPE2XIICAC 0xc00200c8
#define GPE_GPE3XIICAC 0xc00300c8
#define GPE_GPENXIDCAC(n) (GPE_GPE0XIDCAC + ((GPE_GPE1XIDCAC - GPE_GPE0XIDCAC) * (n)))
#define GPE_GPE0XIDCAC 0xc00000d0
#define GPE_GPE1XIDCAC 0xc00100d0
#define GPE_GPE2XIDCAC 0xc00200d0
#define GPE_GPE3XIDCAC 0xc00300d0
#define GPE_GPENOXIXCR(n) (GPE_GPE0OXIXCR + ((GPE_GPE1OXIXCR - GPE_GPE0OXIXCR) * (n)))
#define GPE_GPE0OXIXCR 0xc0000100
#define GPE_GPE1OXIXCR 0xc0010100
#define GPE_GPE2OXIXCR 0xc0020100
#define GPE_GPE3OXIXCR 0xc0030100
#define GPE_GPENXIXSR(n) (GPE_GPE0XIXSR + ((GPE_GPE1XIXSR - GPE_GPE0XIXSR) * (n)))
#define GPE_GPE0XIXSR 0xc0000108
#define GPE_GPE1XIXSR 0xc0010108
#define GPE_GPE2XIXSR 0xc0020108
#define GPE_GPE3XIXSR 0xc0030108
#define GPE_GPENXISPRG0(n) (GPE_GPE0XISPRG0 + ((GPE_GPE1XISPRG0 - GPE_GPE0XISPRG0) * (n)))
#define GPE_GPE0XISPRG0 0xc0000110
#define GPE_GPE1XISPRG0 0xc0010110
#define GPE_GPE2XISPRG0 0xc0020110
#define GPE_GPE3XISPRG0 0xc0030110
#define GPE_GPENXIEDR(n) (GPE_GPE0XIEDR + ((GPE_GPE1XIEDR - GPE_GPE0XIEDR) * (n)))
#define GPE_GPE0XIEDR 0xc0000118
#define GPE_GPE1XIEDR 0xc0010118
#define GPE_GPE2XIEDR 0xc0020118
#define GPE_GPE3XIEDR 0xc0030118
#define GPE_GPENXIIR(n) (GPE_GPE0XIIR + ((GPE_GPE1XIIR - GPE_GPE0XIIR) * (n)))
#define GPE_GPE0XIIR 0xc0000120
#define GPE_GPE1XIIR 0xc0010120
#define GPE_GPE2XIIR 0xc0020120
#define GPE_GPE3XIIR 0xc0030120
#define GPE_GPENXIIAR(n) (GPE_GPE0XIIAR + ((GPE_GPE1XIIAR - GPE_GPE0XIIAR) * (n)))
#define GPE_GPE0XIIAR 0xc0000128
#define GPE_GPE1XIIAR 0xc0010128
#define GPE_GPE2XIIAR 0xc0020128
#define GPE_GPE3XIIAR 0xc0030128
#define GPE_GPENXISIBU(n) (GPE_GPE0XISIBU + ((GPE_GPE1XISIBU - GPE_GPE0XISIBU) * (n)))
#define GPE_GPE0XISIBU 0xc0000130
#define GPE_GPE1XISIBU 0xc0010130
#define GPE_GPE2XISIBU 0xc0020130
#define GPE_GPE3XISIBU 0xc0030130
#define GPE_GPENXISIBL(n) (GPE_GPE0XISIBL + ((GPE_GPE1XISIBL - GPE_GPE0XISIBL) * (n)))
#define GPE_GPE0XISIBL 0xc0000138
#define GPE_GPE1XISIBL 0xc0010138
#define GPE_GPE2XISIBL 0xc0020138
#define GPE_GPE3XISIBL 0xc0030138
#define GPE_GPENXIMEMU(n) (GPE_GPE0XIMEMU + ((GPE_GPE1XIMEMU - GPE_GPE0XIMEMU) * (n)))
#define GPE_GPE0XIMEMU 0xc0000140
#define GPE_GPE1XIMEMU 0xc0010140
#define GPE_GPE2XIMEMU 0xc0020140
#define GPE_GPE3XIMEMU 0xc0030140
#define GPE_GPENXIMEML(n) (GPE_GPE0XIMEML + ((GPE_GPE1XIMEML - GPE_GPE0XIMEML) * (n)))
#define GPE_GPE0XIMEML 0xc0000148
#define GPE_GPE1XIMEML 0xc0010148
#define GPE_GPE2XIMEML 0xc0020148
#define GPE_GPE3XIMEML 0xc0030148
#define GPE_GPENXISGBU(n) (GPE_GPE0XISGBU + ((GPE_GPE1XISGBU - GPE_GPE0XISGBU) * (n)))
#define GPE_GPE0XISGBU 0xc0000150
#define GPE_GPE1XISGBU 0xc0010150
#define GPE_GPE2XISGBU 0xc0020150
#define GPE_GPE3XISGBU 0xc0030150
#define GPE_GPENXISGBL(n) (GPE_GPE0XISGBL + ((GPE_GPE1XISGBL - GPE_GPE0XISGBL) * (n)))
#define GPE_GPE0XISGBL 0xc0000158
#define GPE_GPE1XISGBL 0xc0010158
#define GPE_GPE2XISGBL 0xc0020158
#define GPE_GPE3XISGBL 0xc0030158
#define GPE_GPENXIICACU(n) (GPE_GPE0XIICACU + ((GPE_GPE1XIICACU - GPE_GPE0XIICACU) * (n)))
#define GPE_GPE0XIICACU 0xc0000160
#define GPE_GPE1XIICACU 0xc0010160
#define GPE_GPE2XIICACU 0xc0020160
#define GPE_GPE3XIICACU 0xc0030160
#define GPE_GPENXIICACL(n) (GPE_GPE0XIICACL + ((GPE_GPE1XIICACL - GPE_GPE0XIICACL) * (n)))
#define GPE_GPE0XIICACL 0xc0000168
#define GPE_GPE1XIICACL 0xc0010168
#define GPE_GPE2XIICACL 0xc0020168
#define GPE_GPE3XIICACL 0xc0030168
#define GPE_GPENXIDCACU(n) (GPE_GPE0XIDCACU + ((GPE_GPE1XIDCACU - GPE_GPE0XIDCACU) * (n)))
#define GPE_GPE0XIDCACU 0xc0000170
#define GPE_GPE1XIDCACU 0xc0010170
#define GPE_GPE2XIDCACU 0xc0020170
#define GPE_GPE3XIDCACU 0xc0030170
#define GPE_GPENXIDCACL(n) (GPE_GPE0XIDCACL + ((GPE_GPE1XIDCACL - GPE_GPE0XIDCACL) * (n)))
#define GPE_GPE0XIDCACL 0xc0000178
#define GPE_GPE1XIDCACL 0xc0010178
#define GPE_GPE2XIDCACL 0xc0020178
#define GPE_GPE3XIDCACL 0xc0030178

#endif // __GPE_REGISTER_ADDRESSES_H__

