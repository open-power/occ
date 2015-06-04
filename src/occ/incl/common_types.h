/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/incl/common_types.h $                                 */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2015                        */
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

#ifndef _COMMON_TYPES_H
#define _COMMON_TYPES_H

#include <stdint.h>

#ifdef USE_SSX_APP_CFG_H
#include <ssx_app_cfg.h>
#endif

typedef uint32_t      UINT32;
typedef int32_t       INT32;
typedef uint8_t       UCHAR;
typedef uint8_t       UINT8;
typedef int8_t        INT8;
typedef uint16_t      UINT16;
typedef int16_t       INT16;
typedef char          CHAR;
typedef unsigned int  UINT;
typedef unsigned long ULONG;
typedef int           INT;
typedef void          VOID;

// Skip this typedef in x86 environment
#ifndef OCC_X86_PARSER
typedef uint8_t       bool;
#endif

// Definition of FALSE and TRUE
#if !defined(FALSE) && !defined(TRUE)
typedef enum
{
    FALSE = 0,
    TRUE
} BOOLEAN;
#endif

#define true 1
#define false 0

#ifndef NULL
#define NULL                 (VOID *) 0
#endif

#define MAIN_APP_ID         "Main App Image\0"

#define SRAM_REPAIR_RESERVE_SZ  64
#define IMAGE_ID_STR_SZ         16
#define RESERVED_SZ              8
#define TRAP_INST               0x7FE00004
#define ID_NUM_INVALID          0xFFFF
#define KILOBYTE                1024

// Magic number set for applet headers
#define APLT_MAGIC_NUMBER {0x1A,0x2B,0x3C,0x4D, 0x5E,0x6F,0x7A,0x8B, 0x9C,0xAD,0xAE,0x9F, 0x8A,0x7B,0x6C,0x5D, 0x4E,0x3F,0x2A,0x1B,\
                           0x1A,0x2B,0x3C,0x4D, 0x5E,0x6F,0x7A,0x8B, 0x9C,0xAD,0xAE,0x9F, 0x8A,0x7B,0x6C,0x5D, 0x4E,0x3F,0x2A,0x1B,\
                           0x1A,0x2B,0x3C,0x4D, 0x5E,0x6F,0x7A,0x8B, 0x9C,0xAD,0xAE,0x9F, 0x8A,0x7B,0x6C,0x5D, 0x4E,0x3F,0x2A,0x1B, 0x12,0x34,0xAB,0xCD}
#define SRAM_HEADER_HACK   0x48000042

//Struct containing various OCC flags
struct OCC_FLAGS
{
    uint8_t ipl_time_flag : 1;
};


#ifndef __ASSEMBLER__

// Structure for the common image header
struct image_header
{
    // Overload sram_repair_reserved for magic applet number
    // Note: unit64_t's don't compile when used w/IMAGE_HEADER macro.
    union
    {
        struct
        {
            uint32_t magic_1;  // 0x1A2B3C4D
            uint32_t magic_2;  // 0x5E6F7A8B
            uint32_t magic_3;  // 0x9CADAE9F
            uint32_t magic_4;  // 0x8A7B6C5D
            uint32_t magic_5;  // 0x4E3F2A1B
            uint32_t magic_6;  // 0x1A2B3C4D
            uint32_t magic_7;  // 0x5E6F7A8B
            uint32_t magic_8;  // 0x9CADAE9F
            uint32_t magic_9;  // 0x8A7B6C5D
            uint32_t magic_a;  // 0x4E3F2A1B
            uint32_t magic_b;  // 0x1A2B3C4D
            uint32_t magic_c;  // 0x5E6F7A8B
            uint32_t magic_d;  // 0x9CADAE9F
            uint32_t magic_e;  // 0x8A7B6C5D
            uint32_t magic_f;  // 0x4E3F2A1B
            uint32_t magic_10; // 0x1234ABCD
        };
        uint8_t sram_repair_reserved[SRAM_REPAIR_RESERVE_SZ]; //reserved for HW use
    };

    uint32_t ep_branch_inst;            // entry point branch instruction for bootloader
    uint32_t halt_inst;                 // halt instruction
    uint32_t image_size;                // image size including header
    uint32_t start_addr;                // image start address including header
    uint32_t readonly_size;             // readonly image size
    uint32_t boot_writeable_addr;       // boot writeable address
    uint32_t boot_writeable_size;       // boot writeable size
    uint32_t zero_data_addr;            // zero data address
    uint32_t zero_data_size;            // zero data size
    uint32_t ep_addr;                   // entry point to the image
    uint32_t checksum;                  // checksum of the image including header
    uint32_t version;                   // image version
    char image_id_str[IMAGE_ID_STR_SZ]; // image id string
    uint16_t aplt_id;                   // type: enum OCC_APLT
    union
    {
        uint16_t flags;
        struct OCC_FLAGS flag_bits;
    } occ_flags;                        // bit field for occ flags
    uint32_t nest_frequency;            // nest frequency
    uint8_t reserved[RESERVED_SZ];      // reserved for future use
} __attribute__ ((__packed__));

typedef struct image_header imageHdr_t;

extern uint32_t __READ_ONLY_DATA_LEN__;
extern uint32_t __WRITEABLE_DATA_ADDR__;
extern uint32_t __WRITEABLE_DATA_LEN__;
extern uint32_t __START_ADDR__;


// Macro for creating common image header
// NOTE: ep_branch_inst is defaulted to trap instruction. Script to fix
// header will change it to branch to address in ep_addr field.
#define IMAGE_HEADER(nameStr, epAddr, IdStr, IdNum) \
const volatile imageHdr_t nameStr  __attribute__((section("imageHeader")))= \
{ \
    /* sram_repair_reserved values should match APLT_MAGIC_NUMBER for an applet; 0 otherwise */ \
    { \
        { \
            .magic_1 = (IdNum == ID_NUM_INVALID)?0:0x1A2B3C4D, \
            .magic_2 = (IdNum == ID_NUM_INVALID)?0:0x5E6F7A8B, \
            .magic_3 = (IdNum == ID_NUM_INVALID)?0:0x9CADAE9F, \
            .magic_4 = (IdNum == ID_NUM_INVALID)?0:0x8A7B6C5D, \
            .magic_5 = (IdNum == ID_NUM_INVALID)? SRAM_HEADER_HACK:0x4E3F2A1B, \
            .magic_6 = (IdNum == ID_NUM_INVALID)?0:0x1A2B3C4D, \
            .magic_7 = (IdNum == ID_NUM_INVALID)?0:0x5E6F7A8B, \
            .magic_8 = (IdNum == ID_NUM_INVALID)?0:0x9CADAE9F, \
            .magic_9 = (IdNum == ID_NUM_INVALID)?0:0x8A7B6C5D, \
            .magic_a = (IdNum == ID_NUM_INVALID)?0:0x4E3F2A1B, \
            .magic_b = (IdNum == ID_NUM_INVALID)?0:0x1A2B3C4D, \
            .magic_c = (IdNum == ID_NUM_INVALID)?0:0x5E6F7A8B, \
            .magic_d = (IdNum == ID_NUM_INVALID)?0:0x9CADAE9F, \
            .magic_e = (IdNum == ID_NUM_INVALID)?0:0x8A7B6C5D, \
            .magic_f = (IdNum == ID_NUM_INVALID)?0:0x4E3F2A1B, \
            .magic_10 = (IdNum == ID_NUM_INVALID)?0:0x1234ABCD, \
        } \
    }, \
    TRAP_INST,                          /* ep_branch_Inst */ \
    TRAP_INST,                          /* halt_inst */ \
    0,                                  /* image_size (filled in later by imageHdrScript) */ \
    (uint32_t)&__START_ADDR__,          /* start_addr */ \
    (uint32_t)&__READ_ONLY_DATA_LEN__,  /* readonly_size */ \
    (uint32_t)&__WRITEABLE_DATA_ADDR__, /* boot_writeable_addr */ \
    (uint32_t)&__WRITEABLE_DATA_LEN__,  /* boot_writeable_size */ \
    0,                                  /* zero_data_addr (currently unused) */ \
    0,                                  /* zero data_size (currently unused) */ \
    (uint32_t)&epAddr,                  /* ep_addr */ \
    0,                                  /* checksum (calculated later by imsageHdrScript) */ \
    0,                                  /* version (filled in later by imageHdrScript) */ \
    IdStr,                              /* image_id_str */ \
    (uint16_t)IdNum,                    /* aplt_id */ \
    0,                                  /* occ flags bit field */ \
    0,                                  /* nest_frequency */ \
    {0}                                 /* reserved */ \
};

#endif  /* __ASSEMBLER__ */

#endif //_COMMON_TYPES_H
