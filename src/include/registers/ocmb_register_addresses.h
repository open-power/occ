/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/include/registers/ocmb_register_addresses.h $             */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2017                        */
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
#ifndef __OCMB_REGISTER_ADDRESSES_H__
#define __OCMB_REGISTER_ADDRESSES_H__


#define MMIO_MERRCTL  0x080108ea
#define MMIO_MFIR     0x08010870
#define MMIO_MFIR_AND 0x08010871
#define MMIO_OCTHERM  0x08010851
#define MMIO_D0THERM  0x08010852
#define MMIO_D1THERM  0x08010853



// N/M Throtling Control
#define OCMB_MBA_FARB3Q 0x08011418

#define OCMB_IB_SENSOR_CACHE_ADDR (0x40084200 >> 3)
#define OCMB_IB_BAR_B_BIT (0x0000000080000000ull)

#endif
