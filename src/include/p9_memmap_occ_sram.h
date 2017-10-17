/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/include/p9_memmap_occ_sram.h $                            */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2017                             */
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
#if !defined(__P9_MEMMAP_OCC_SRAM_H__)
#define __P9_MEMMAP_OCC_SRAM_H__

#define OCC_SRAM_BASE_ADDR  0xFFF00000

#define GPE_DEBUG_PTRS_OFFSET 0x180
#define PGPE_DEBUG_PTRS_OFFSET 0x200
#define SGPE_DEBUG_PTRS_OFFSET 0x200

#define PK_TRACE_PTR_OFFSET  0x04
#define PK_TRACE_SIZE_OFFSET 0x08

#define OCC_SRAM_IPC_REGION_SIZE (4 * 1024)
#define OCC_SRAM_GPE0_REGION_SIZE (60 * 1024)
#define OCC_SRAM_GPE1_REGION_SIZE (64 * 1024)

#define GPE0_SRAM_BASE_ADDR OCC_SRAM_BASE_ADDR + OCC_SRAM_IPC_REGION_SIZE
#define GPE1_SRAM_BASE_ADDR GPE0_SRAM_BASE_ADDR + OCC_SRAM_GPE0_REGION_SIZE

#endif
