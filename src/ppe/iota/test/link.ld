/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/iota/test/link.ld $                                   */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2019                             */
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
OUTPUT_FORMAT(elf32-powerpc)

#define  __LINKERSCRIPT__
#include "iota_lnk_cfg.h"

#if ((SRAM_START % 512) != 0)
#error SRAM_START must be 512 byte aligned
#endif

MEMORY
{
    sram : ORIGIN = SRAM_START, LENGTH = SRAM_LENGTH
}

// The linker will discard any symbols and sections it deems are unused.
// Declare as EXTERN any symbols only used externally to ensure the linker
// keeps them even though APP code never references them.
EXTERN(iota_debug_ptrs);
EXTERN(g_cme_magic_word);

SECTIONS
{
    . = SRAM_START;
    _VECTOR_START = .;

    .vectors _VECTOR_START  : { *(.vectors) } > sram

    /*=====================================
     * IMAGE HEADER
     ======================================*/
    _IMG_HEADER = _VECTOR_START +  PPE_HEADER_IMAGE_OFFSET;
    PPE_HEADER_IMAGE_NAME _IMG_HEADER : { *(PPE_HEADER_IMAGE_NAME) } > sram

    /*====================================
     * Debug pointers table
     ====================================*/
    _DEBUG_PTRS_START = _VECTOR_START + PPE_DEBUG_PTRS_OFFSET;
    .debug_ptrs _DEBUG_PTRS_START : { *(.debug_ptrs) } > sram

    _DUMP_PTR_PSTATE_START = _DEBUG_PTRS_START + PPE_DEBUG_PTRS_SIZE;
    .dump_ptr_pstate _DUMP_PTR_PSTATE_START : { *(.dump_ptr_pstate) } > sram

    _DUMP_PTR_STOP_START = _DUMP_PTR_PSTATE_START + PPE_DUMP_PTR_PSTATE_SIZE;
    .dump_ptr_stop _DUMP_PTR_STOP_START : { *(.dump_ptr_stop) } > sram

    . = ALIGN(16);
    /*======================================
     * Code
     ======================================*/
    .text    . : { *(.text*)    } > sram


    /*======================================
     * Read only data
     *====================================*/
    .rodata . : { *(.rodata*) *(.got2) } > sram

    /*===================================================
     * Small data area 2. Must be 8 byte aligned
     * .sdata2 and .sbss2 should be next to each other.
     * .sdata2 is intended for read-only data (const).
     * .sbss2 is for zero initailized or non-initialized
     * const data, but the POWER EABI allow it to be written.
     * This area is referenced by an offset to gpr2
     ====================================================*/
    . = ALIGN(8);
    _SDA2_BASE_ = . ;
    .sdata2  . : { *(.sdata2*) } > sram
    .sbss2   . : { *(.sbss2*)  } > sram

    /*================================================
     * The Small data area. Must by 8 byte aligned
     * .sdata and .sbss should be next to each other
     * This area is referenced by an offset to gpr13.
     * The compiler will place any static/global data
     * type 8 bytes or less in length in these sections
     * .sdata is for initialized data.
     * .sbss is for uninitialized or zero initialized data
     * Any data object can be forced into this secion
     * using __attribute__((section(".sdata")))
     * @note
     * This would be good place to put iota kernel data
     * such that multiple ppe engines could share the
     * same kernel, but have their own context (not complete)
     ================================================*/
    . = ALIGN(8);
    _SDA_BASE_ = . ;
    .sdata   . : {  *(.sdata.ppe42_64bit_timebase)
                    *(.sdata.g_current_prty_level)
                    *(.sdata.g_iota_curr_machine_state_ptr)
                    *(.sdata.g_iota_fit_handler)
                    *(.sdata.g_iota_dec_handler)
                    *(.sdata.g_iota_idle_task_list)
                    *(.sdata.g_iota_task_list)
                    *(.sdata*)
                 } > sram

    .sbss    . : {  *(.sbss.sbss.g_ext_irq_vector)
                    *(.sbss.g_eimr_override)
                    *(.sbss*)  } > sram

    /*=====================================================
     * Large data area.
     * data types > 8 bytes go here by default.
     * The linker will throw out sections at the end of
     * an image that only contain zeros, so use KEEP()
     =====================================================*/
    . = ALIGN(8);
    .rwdata . : { KEEP(*(.data.g_iota_execution_stack))
                  KEEP(*(.data.g_iota_execution_stack_end))
                  KEEP(*(.data*))
                  KEEP(*(.bss*))
                  FILL(0xA55A);  /* mark end of image     */
                  . = ALIGN(16); /* keeps the linker from */
                                 /* truncating the image  */
              } > sram

    _IMAGE_END = .;
    _IMAGE_SIZE = . - SRAM_START;

    .rela     : { *(.rela*) }
    .eh_frame : { *(.eh_frame) }
    .iplt     : { *(.iplt) }

    /DISCARD/ : {
        *(.comment)
        *(.gnu.attributes)
        *(.dtors)
        *(.interp)
    }

}
