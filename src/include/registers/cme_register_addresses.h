/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/include/registers/cme_register_addresses.h $              */
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
#ifndef __CME_REGISTER_ADDRESSES_H__
#define __CME_REGISTER_ADDRESSES_H__

// $Id$
// $Source$
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2015
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file cme_register_addresses.h
/// \brief Symbolic addresses for the CME unit

// *** WARNING *** - This file is generated automatically, do not edit.


#define CME_FIRPIB_BASE 0x10012000
#define CME_SCOM_LFIR 0x10012000
#define CME_SCOM_LFIR_AND 0x10012001
#define CME_SCOM_LFIR_OR 0x10012002
#define CME_SCOM_LFIRMASK 0x10012003
#define CME_SCOM_LFIRMASK_AND 0x10012004
#define CME_SCOM_LFIRMASK_OR 0x10012005
#define CME_SCOM_LFIRACT0 0x10012006
#define CME_SCOM_LFIRACT1 0x10012007
#define CME_PIB_BASE 0x10012000
#define CME_SCOM_CSCR 0x1001200a
#define CME_SCOM_CSCR_CLR 0x1001200b
#define CME_SCOM_CSCR_OR 0x1001200c
#define CME_SCOM_CSAR 0x1001200d
#define CME_SCOM_CSDR 0x1001200e
#define CME_SCOM_BCECSR 0x1001200f
#define CME_SCOM_BCEBAR0 0x10012010
#define CME_SCOM_BCEBAR1 0x10012011
#define CME_SCOM_QFMR 0x10012012
#define CME_SCOM_AFSR 0x10012013
#define CME_SCOM_AFTR 0x10012014
#define CME_SCOM_VTSR0 0x10012015
#define CME_SCOM_VTSR1 0x10012016
#define CME_SCOM_VDSR 0x10012017
#define CME_SCOM_EIIR 0x10012019
#define CME_SCOM_FWMR 0x1001201a
#define CME_SCOM_FWMR_CLR 0x1001201b
#define CME_SCOM_FWMR_OR 0x1001201c
#define CME_SCOM_SICR 0x1001201d
#define CME_SCOM_SICR_CLR 0x1001201e
#define CME_SCOM_SICR_OR 0x1001201f
#define CME_SCOM_FLAGS 0x10012020
#define CME_SCOM_FLAGS_CLR 0x10012021
#define CME_SCOM_FLAGS_OR 0x10012022
#define CME_SCOM_SRTCH0 0x10012023
#define CME_SCOM_SRTCH1 0x10012024
#define CME_SCOM_EISR 0x10012025
#define CME_SCOM_EIMR 0x10012026
#define CME_SCOM_EIPR 0x10012027
#define CME_SCOM_EITR 0x10012028
#define CME_SCOM_EISTR 0x10012029
#define CME_SCOM_EINR 0x1001202a
#define CME_SCOM_SISR 0x1001202b
#define CME_SCOM_ICRR 0x1001202c
#define CME_SCOM_XIXCR 0x10012030
#define CME_SCOM_XIRAMRA 0x10012031
#define CME_SCOM_XIRAMGA 0x10012032
#define CME_SCOM_XIRAMDBG 0x10012033
#define CME_SCOM_XIRAMEDR 0x10012034
#define CME_SCOM_XIDBGPRO 0x10012035
#define CME_SCOM_XISIB 0x10012036
#define CME_SCOM_XIMEM 0x10012037
#define CME_SCOM_CMEXISGB 0x10012038
#define CME_SCOM_XIICAC 0x10012039
#define CME_SCOM_XIPCBQ0 0x1001203a
#define CME_SCOM_XIPCBQ1 0x1001203b
#define CME_SCOM_XIPCBMD0 0x1001203c
#define CME_SCOM_XIPCBMD1 0x1001203d
#define CME_SCOM_XIPCBMI0 0x1001203e
#define CME_SCOM_XIPCBMI1 0x1001203f
#define CME_SCOM_PMSRS0 0x10012040
#define CME_SCOM_PMSRS1 0x10012041
#define CME_SCOM_PMCRS0 0x10012042
#define CME_SCOM_PMCRS1 0x10012043
#define CME_SCOM_PSCRS00 0x10012044
#define CME_SCOM_PSCRS10 0x10012045
#define CME_SCOM_PSCRS01 0x10012048
#define CME_SCOM_PSCRS11 0x10012049
#define CME_SCOM_PSCRS02 0x1001204c
#define CME_SCOM_PSCRS12 0x1001204d
#define CME_SCOM_PSCRS03 0x10012050
#define CME_SCOM_PSCRS13 0x10012051
#define CME_LOCAL_BASE 0xC0000000
#define CME_LCL_EISR 0xc0000000
#define CME_LCL_EISR_OR 0xc0000010
#define CME_LCL_EISR_CLR 0xc0000018
#define CME_LCL_EIMR 0xc0000020
#define CME_LCL_EIMR_OR 0xc0000030
#define CME_LCL_EIMR_CLR 0xc0000038
#define CME_LCL_EIPR 0xc0000040
#define CME_LCL_EIPR_OR 0xc0000050
#define CME_LCL_EIPR_CLR 0xc0000058
#define CME_LCL_EITR 0xc0000060
#define CME_LCL_EITR_OR 0xc0000070
#define CME_LCL_EITR_CLR 0xc0000078
#define CME_LCL_EISTR 0xc0000080
#define CME_LCL_EINR 0xc00000a0
#define CME_LCL_TSEL 0xc0000100
#define CME_LCL_DBG 0xc0000120
#define CME_LCL_DBG_OR 0xc0000130
#define CME_LCL_DBG_CLR 0xc0000138
#define CME_LCL_TBR 0xc0000140
#define CME_LCL_AFSR 0xc0000160
#define CME_LCL_AFTR 0xc0000180
#define CME_LCL_LMCR 0xc00001a0
#define CME_LCL_BCECSR 0xc00001f0
#define CME_LCL_PMSRS0 0xc0000200
#define CME_LCL_PMSRS1 0xc0000220
#define CME_LCL_PMCRS0 0xc0000240
#define CME_LCL_PMCRS1 0xc0000260
#define CME_LCL_PECESR0 0xc0000280
#define CME_LCL_PECESR1 0xc00002a0
#define CME_LCL_PSCRS00 0xc0000300
#define CME_LCL_PSCRS10 0xc0000320
#define CME_LCL_PSCRS20 0xc0000340
#define CME_LCL_PSCRS30 0xc0000360
#define CME_LCL_PSCRS01 0xc0000380
#define CME_LCL_PSCRS11 0xc00003a0
#define CME_LCL_PSCRS21 0xc00003c0
#define CME_LCL_PSCRS31 0xc00003e0
#define CME_LCL_FLAGS 0xc0000400
#define CME_LCL_FLAGS_OR 0xc0000410
#define CME_LCL_FLAGS_CLR 0xc0000418
#define CME_LCL_SRTCH0 0xc0000420
#define CME_LCL_SRTCH1 0xc0000440
#define CME_LCL_SICR 0xc0000500
#define CME_LCL_SICR_OR 0xc0000510
#define CME_LCL_SICR_CLR 0xc0000518
#define CME_LCL_SISR 0xc0000520
#define CME_LCL_XIPCBMD0 0xc0000580
#define CME_LCL_XIPCBMD1 0xc00005a0
#define CME_LCL_XIPCBMI0 0xc00005c0
#define CME_LCL_XIPCBMI1 0xc00005e0
#define CME_LCL_VTSR0 0xc0000600
#define CME_LCL_VTSR1 0xc0000620
#define CME_LCL_VDSR 0xc0000640
#define CME_LCL_ICCR 0xc0000700
#define CME_LCL_ICCR_OR 0xc0000710
#define CME_LCL_ICCR_CLR 0xc0000718
#define CME_LCL_ICSR 0xc0000720
#define CME_LCL_ICRR 0xc0000740

#endif // __CME_REGISTER_ADDRESSES_H__

