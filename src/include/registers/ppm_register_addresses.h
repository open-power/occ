/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/include/registers/ppm_register_addresses.h $              */
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
#ifndef __PPM_REGISTER_ADDRESSES_H__
#define __PPM_REGISTER_ADDRESSES_H__

/// \file ppm_register_addresses.h
/// \brief Symbolic addresses for the PPM unit

// *** WARNING *** - This file is generated automatically, do not edit.


#define PPM_PIB_BASE 0x000F0000
#define PPM_GPMMR 0x000f0100
#define PPM_GPMMR_CLR 0x000f0101
#define PPM_GPMMR_OR 0x000f0102
#define PPM_SPWKUP_OTR 0x000f010a
#define PPM_SPWKUP_FSP 0x000f010b
#define PPM_SPWKUP_OCC 0x000f010c
#define PPM_SPWKUP_HYP 0x000f010d
#define PPM_SSHSRC 0x000f0110
#define PPM_SSHFSP 0x000f0111
#define PPM_SSHOCC 0x000f0112
#define PPM_SSHOTR 0x000f0113
#define PPM_SSHHYP 0x000f0114
#define PPM_PFCS 0x000f0118
#define PPM_PFCS_CLR 0x000f0119
#define PPM_PFCS_OR 0x000f011a
#define PPM_PFDLY 0x000f011b
#define PPM_PFSNS 0x000f011c
#define PPM_PFOFF 0x000f011d
#define PPM_SCRATCH0 0x000f011e
#define PPM_SCRATCH1 0x000f011f
#define PPM_CGCR 0x000f0164
#define PPM_PIG 0x000f0180
#define PPM_IVRMCR 0x000f01b0
#define PPM_IVRMCR_CLR 0x000f01b1
#define PPM_IVRMCR_OR 0x000f01b2
#define PPM_IVRMST 0x000f01b3
#define PPM_IVRMDVR 0x000f01b4
#define PPM_IVRMAVR 0x000f01b5
#define PPM_VDMCR 0x000f01b8
#define PPM_VDMCR_CLR 0x000f01b9
#define PPM_VDMCR_OR 0x000f01ba

// Hand added
#define PPM_ERROR 0x000f0121

#endif // __PPM_REGISTER_ADDRESSES_H__

