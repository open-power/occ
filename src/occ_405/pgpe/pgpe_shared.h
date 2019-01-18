/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/pgpe/pgpe_shared.h $                              */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2018                        */
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

#define PPMR_MAGIC_NUMBER_10         0x50504D525F312E30ull   // "PPMR_1.0"
#define OPPB_MAGIC_NUMBER_10         0x4F43435050423030ull   // "OCCPPB00"
#define PGPE_MAGIC_NUMBER_10         0x504750455F312E30ull   // "PGPE_1.0"

// magic number for OCC-PGPE Shared Sram
#define OPS_MAGIC_NUMBER_P9          0x4F505330ull   // "OPS0"
#define OPS_MAGIC_NUMBER_P9_PRIME    0x4F505331ull   // "OPS1"
#define OPS_MAGIC_NUMBER_P10         0x4F505332ull   // "OPS2"


// Address and offset to find PGPE PK trace
#define PGPE_DEBUG_PTRS_ADDR           0xFFF20200   // 0xfff20000 + 0x200
#define PGPE_DEBUG_TRACE_ADDR_OFFSET   0x04
#define PGPE_DEBUG_TRACE_SIZE_OFFSET   0x08

// Offset addresses of PGPE Header parameters (relative to start address)
#define PGPE_SHARED_SRAM_ADDR_OFFSET           0x0c
#define PGPE_SHARED_SRAM_LEN_OFFSET            0x14
#define PGPE_GENERATED_PSTATE_TBL_ADDR_OFFSET  0x38 // generated table address is offset from beginning of HOMER
#define PGPE_GENERATED_PSTATE_TBL_SZ_OFFSET    0x3C
#define PGPE_OCC_PSTATE_TBL_ADDR_OFFSET        0x40 // OCC table address is a SRAM address
#define PGPE_OCC_PSTATE_TBL_SZ_OFFSET          0x44
#define PGPE_BEACON_ADDR_OFFSET                0x48
#define PGPE_ACTUAL_QUAD_STATUS_ADDR_OFFSET    0x4c
#define PGPE_WOF_STATE_ADDR_OFFSET             0x50
#define PGPE_REQUESTED_ACTIVE_QUAD_ADDR_OFFSET 0x54
#define PGPE_WOF_TBLS_ADDR_OFFSET              0x58
#define PGPE_WOF_TBLS_LEN_OFFSET               0x5C
#define PGPE_PRODUCED_WOF_VALUES_ADDR_OFFSET   0x78

// This size must be a multiple of 128
typedef struct __attribute__ ((packed))
{
    uint64_t  magic_number;
    uint32_t  bc_offset;
    uint32_t  reserved;
    uint32_t  bl_offset;
    uint32_t  bl_length;
    uint32_t  build_date;
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


typedef struct __attribute__ ((packed))
{
    uint32_t shared_sram_addr;
    uint32_t shared_sram_length;
    uint32_t generated_pstate_table_homer_offset;
    uint32_t generated_pstate_table_length;
    uint32_t occ_pstate_table_sram_addr;
    uint32_t occ_pstate_table_length;
    uint32_t beacon_sram_addr;
    uint32_t actual_quad_status_sram_addr;
    uint32_t wof_state_address;
    uint32_t requested_active_quad_sram_addr;
    uint32_t wof_tables_addr;
    uint32_t wof_tables_length;
    uint32_t pgpe_produced_wof_values_addr;
} pgpe_header_data_t;

extern pgpe_header_data_t G_pgpe_header;



