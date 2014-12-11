/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occBootLoader/bootMain.h $                                */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* COPYRIGHT International Business Machines Corp. 2011,2014              */
/* [+] Google Inc.                                                        */
/* [+] International Business Machines Corp.                              */
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

#ifndef _bootMain_h
#define _bootMain_h

//*************************************************************************
// Includes
//*************************************************************************
#ifndef __ASSEMBLER__
#include <common_types.h> // defines imageHdr_t and other types
#endif  /* __ASSEMBLER__ */

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************
#define MACHINE_CHECK_ENABLE        0x00001000
#define DATA_CACHE_BLOCK_ENABLE     0x00008000
#define DATA_CACHE_BLOCK_ADDR       0x80000000
#define DATA_CACHE_SIZE             (16 * 1024)
#define CACHE_LINE_SIZE             32
#define DATA_CACHE_LINES            (DATA_CACHE_SIZE/CACHE_LINE_SIZE)

// Data cache address + 8K
#define STACK_POINTER_ADDR          (DATA_CACHE_BLOCK_ADDR + 0x2000)

#ifndef __ASSEMBLER__

typedef enum CHKPOINT
{
    BOOT_TEST_SRAM_CHKPOINT =       0x00000001,
    BOOT_LOAD_IMAGE_CHKPOINT =      0x00000002,
    BOOT_CALCULTE_CHKSUM_CHKPOINT=  0x00000003,
    BOOT_GET_NEST_FREQ_CHKPOINT =   0x00000004,
    BOOT_SSX_BOOT_CALL_CHKPOINT =   0x00000005,
    BOOT_SSX_RETURNED_CHKPOINT =    0x00000006
}CHKPOINT;

#endif  /* __ASSEMBLER__ */

#define SRAM_TEST_START_ADDRESS     0xFFF80040
#define SRAM_START_ADDRESS          0xFFF80000
#define SRAM_TEST_END_ADDRESS       0xFFFFFBFF
#define SRAM_END_ADDRESS            0xFFFFFFFF
#define SRAM_TEST_BIT_PATTERN       0xA5A5A5A5
#define EYE_CATCHER_ADDRESS         0x1234ABCD

#define BOOT_LOADER_ID              "OCC Boot Image\0"

// Define to write val to SPRG0 register
#define WRITE_TO_SPRG0(val) \
    ({__asm__ __volatile__ ("mtsprg0 %0;" ::"r"(val));})

#define WRITE_TO_SPRG1_AND_HALT(rc) \
    ({__asm__ __volatile__ ("mtsprg1 %0;" "tw 31,0,0;": : "r" (rc));})

//*************************************************************************
// Structures
//*************************************************************************

//*************************************************************************
// Globals
//*************************************************************************

//*************************************************************************
// Function Prototypes
//*************************************************************************

//*************************************************************************
// Functions
//*************************************************************************

#endif // _bootMain_h

