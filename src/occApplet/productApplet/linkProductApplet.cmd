// IBM_PROLOG_BEGIN_TAG
// This is an automatically generated prolog.
//
// $Source: src/occApplet/productApplet/linkProductApplet.cmd $
//
// OpenPOWER OnChipController Project
//
// Contributors Listed Below - COPYRIGHT 2011,2014
// [+] Google Inc.
// [+] International Business Machines Corp.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied. See the License for the specific language governing
// permissions and limitations under the License.
//
// IBM_PROLOG_END_TAG

// This linker script for the product applet.

#define APP_IMAGE_SRAM_START_ADDR   0xFFFF8000
#define WORD_ALIGN 4
#define BYTE_ALIGN 1024
#ifdef OCCMK
INCLUDE occLinkInputFile
#endif

MEMORY
{
   AppMem : ORIGIN = APP_IMAGE_SRAM_START_ADDR, LENGTH = 0x4000
}

SECTIONS
{
    . = APP_IMAGE_SRAM_START_ADDR;

    __START_ADDR__ = .;

    ////////////////////////////////
    // start read-only section
    ////////////////////////////////
    imageHeader . : { *(imageHeader) } > AppMem

    ////////////////////////////////
    // text section 1024 byte aligned
    ////////////////////////////////
    .text . : { *(.text) . = ALIGN(1024);} > AppMem
    // NOTE: rodata section needs to be 1k aligned as it is used for setting
    // mmu permission druing applet execution
    .rodata . : { *(.rodata) *(.got2) *(.rodata.str1.4) *(.rodata.str1.1). = ALIGN(BYTE_ALIGN); } > AppMem
    __READ_ONLY_DATA_LEN__ = . - APP_IMAGE_SRAM_START_ADDR ;

    ////////////////////////////////
    // start writeable section
    ////////////////////////////////
    __WRITEABLE_DATA_ADDR__ = .;

    ////////////////////////////////
    // read-write section  1024 byte aligned
    ////////////////////////////////
   .rela   . :  { *(.rela*) . = ALIGN(WORD_ALIGN);} > AppMem
   // NOTE: rwdata section needs to be 1024 bytes aligned for setting mmu
   // permission, so that applet total size is 1024 bytes aligned.
   // It is needed for doing DMA copy of the applet.
   .rwdata . :  { *(.data) *(.bss) *(COMMON) . = ALIGN(BYTE_ALIGN);} > AppMem

   __WRITEABLE_DATA_LEN__ = . - __WRITEABLE_DATA_ADDR__;

}
