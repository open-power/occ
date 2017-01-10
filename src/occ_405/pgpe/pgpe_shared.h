/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/pgpe/pgpe_shared.h $                              */
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

#define PGPE_HEADER_ADDR             0xFFF20180 // 0xfff20000 + 0x180
#define PGPE_HEADER_SZ               96         // Size of PGPE Image header

#define PGPE_MAGIC_NUMBER            0x5849502050475045ull   // "XIP PGPE"

// Offset addresses of PGPE Header parameters (relative to start address)
#define PGPE_SHARED_SRAM_ADDR_OFFSET 0x0c
#define PGPE_SHARED_SRAM_SZ_OFFSET   0x14
#define PGPE_BEACON_ADDR_OFFSET      0x48
#define PGPE_ACTIVE_QUAD_ADDR_OFFSET 0x4c
#define PGPE_WOF_TBLS_ADDR_OFFSET    0x50
#define PGPE_WOF_TBLS_LEN_OFFSET     0x54

// Offset addresses of OCC-PGPE Shared SRAM pointers (relative to Shared SRAM ptr)
#define PGPE_SHARED_REQUESTED_ACTIVE_QUAD_UPDATE_OFFSET 0x18

// PGPE Image Header Parameter addresses

//Shared OCC-PGPE SRAM parameters
#define PGPE_SHARED_SRAM_ADDR_PTR    (PGPE_HEADER_ADDR + PGPE_SHARED_SRAM_ADDR_OFFSET)
#define PGPE_SHARED_SRAM_SZ_PTR      (PGPE_HEADER_ADDR + PGPE_SHARED_SRAM_SZ_OFFSET)




// A pointer to PGPE Beacon Address
#define PGPE_BEACON_ADDR_PTR         (PGPE_HEADER_ADDR + PGPE_BEACON_ADDR_OFFSET)

// Pointers to data needed by WOF
#define PGPE_ACTIVE_QUAD_ADDR_PTR    (PGPE_HEADER_ADDR + PGPE_ACTIVE_QUAD_ADDR_OFFSET)
#define PGPE_WOF_TBLS_ADDR_PTR       (PGPE_HEADER_ADDR + PGPE_WOF_TBLS_ADDR_OFFSET)
#define PGPE_WOF_TBLS_LEN_PTR        (PGPE_HEADER_ADDR + PGPE_WOF_TBLS_LEN_OFFSET)
#define PGPE_SHARED_REQUESTED_ACTIVE_QUAD_UPDATE_PTR \
                                     (PGPE_SHARED_SRAM_ADDR_PTR + \
                                      PGPE_SHARED_REQUESTED_ACTIVE_QUAD_UPDATE_OFFSET)


// PMMR (Pstates PM region) in HOMMR
#define PPMR_OPPM_ADDR_OFFSET    0x40     //offset of the OCC Pstates Parameter Block address in the PPMR header
#define PPMR_OPPM_SZ_OFFSET      0x44     //offset of the OCC Pstates Parameter Block size in the PPMR header


// This size must be a multiple of 128
typedef struct __attribute__ ((packed))
{
    uint64_t  magic_number;
    uint32_t  bc_offset;
    uint32_t  resserved;
    uint32_t  bl_offset;
    uint32_t  bl_length;
    uint32_t  build_date;;
    uint32_t  version;
    uint8_t   resvd_flag[8];
    uint32_t  pgpe_hcode_offset;
    uint32_t  pgpe_hcode_length;
    uint32_t  gppb_offset;
    uint32_t  gppb_length;
    uint32_t  lppb_offset;
    uint32_t  lppb_length;
    uint32_t  oppb_offset;
    uint32_t  oppb_length;
    uint32_t  pstables_offset;
    uint32_t  pstables_length;
    uint8_t   pad[48];
} ppmr_header_t __attribute__ ((aligned (128)));
