/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/pgpe_shared.h $                                   */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2016                        */
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

#define PGPE_HEADER_ADDR             0xFFF20180 // 0xfff20000 + 0x180
#define PGPE_HEADER_SZ               96         // Size of PGPE Image header

// Offset addresses of PGPE Header parameters (relative to start address)
#define PGPE_SHARED_SRAM_ADDR_OFFSET 0x0c
#define PGPE_SHARED_SRAM_SZ_OFFSET   0x14
#define PGPE_BEACON_ADDR_OFFSET      0x48

// PGPE Image Header Parameter addresses

//Shared OCC-PGPE SRAM parameters
#define PGPE_SHARED_SRAM_ADDR_PTR    (PGPE_HEADER_ADDR + PGPE_SHARED_SRAM_ADDR_OFFSET)
#define PGPE_SHARED_SRAM_SZ_PTR      (PGPE_HEADER_ADDR + PGPE_SHARED_SRAM_SZ_OFFSET)

// A pointer to PGPE Beacon Address
#define PGPE_BEACON_ADDR_PTR         (PGPE_HEADER_ADDR + PGPE_BEACON_ADDR_OFFSET)

