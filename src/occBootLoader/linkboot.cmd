// IBM_PROLOG_BEGIN_TAG
// This is an automatically generated prolog.
//
// $Source: src/occBootLoader/linkboot.cmd$
//
// OpenPOWER OnChipController Project
//
// Contributors Listed Below - COPYRIGHT 2011,2014
// [+] Google Inc.
// [+] International Business Machines Corp.
//
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

// Linker script for the OCC Firmware boot loader.
//
// The image is 4 byte aligned.

#ifdef OCCMK
INCLUDE occLinkInputFile
#endif

#define BOOT_IMAGE_START_ADDR   0x00000000
#define BOOT_VECTORS            0x00000000
#define BOOT_VECTORS_SIZE       0x00000740
#define BOOT_BUILDNAME_ADDR     (BOOT_IMAGE_START_ADDR + BOOT_VECTORS_SIZE)
#define WRITE_DATA_SEC_ADDR     0x80000000
#define BYTE_ALIGN 128
#define pack_0000 bootMain.o(imageHeader)

MEMORY
{
    writeableMem : ORIGIN = WRITE_DATA_SEC_ADDR, LENGTH = 0x4000
}

SECTIONS
{
    . = BOOT_IMAGE_START_ADDR;
    . = BOOT_VECTORS;

    __START_ADDR__ = .;

    ////////////////////////////////
    // start read-only section
    ////////////////////////////////

    ////////////////////////////////
    // exception/vector section
    ////////////////////////////////
    .exceptions . : {
        ___vectors = .;
        bootInit.o(.vectors_0000)
        pack_0000
        . = ___vectors + 0x0100;
        bootInit.o(.vectors_0100)
        . = ___vectors + 0x0200;
        bootInit.o(.vectors_0200)
        . = ___vectors + 0x0300;
        bootInit.o(.vectors_0300)
        . = ___vectors + 0x0400;
        bootInit.o(.vectors_0400)
        . = ___vectors + 0x0500;
        bootInit.o(.vectors_0500)
        . = ___vectors + 0x0600;
        bootInit.o(.vectors_0600)
        . = ___vectors + 0x0700;
        bootInit.o(.vectors_0700)
        }

    ////////////////////////////////
    // buildname section 4 byte aligned
    ////////////////////////////////
    . = BOOT_BUILDNAME_ADDR;
    .buildname . : { *(.buildname) }

    ////////////////////////////////
    // text section 4 byte aligned, follows buildname section
    ////////////////////////////////
    .text . : { *(.text) *(.text.*) . = ALIGN(BYTE_ALIGN); }

    ////////////////////////////////
    // SDA2 section 4 byte aligned
    ////////////////////////////////
    _SDA2_BASE_ = .;
    .sdata2 . : { *(.sdata2) . = ALIGN(BYTE_ALIGN); }
    .sbss2 . : { *(.sbss2) . = ALIGN(BYTE_ALIGN); }

    .rodata . : { *(.rodata*) *(.got2) . = ALIGN(BYTE_ALIGN); }

    __READ_ONLY_DATA_LEN__ = . - BOOT_IMAGE_START_ADDR;
    __WRITEABLE_ADDR__ = .;

    ////////////////////////////////
    // start writeable section, has different virtual and loadable memory addresses
    ////////////////////////////////
    __WRITEABLE_DATA_ADDR__ = WRITE_DATA_SEC_ADDR;
    __CURADDR__ = WRITE_DATA_SEC_ADDR;

    ////////////////////////////////
    // read-write section
    ////////////////////////////////
    .rela __CURADDR__ : AT(__WRITEABLE_ADDR__) { *(.rela*) . = ALIGN(BYTE_ALIGN); } > writeableMem
    __CURADDR__ = __CURADDR__ + SIZEOF(.rela);
    .rwdata __CURADDR__ : AT(__WRITEABLE_ADDR__ + SIZEOF(.rela)) { *(.data) *(.bss) *(COMMON) . = ALIGN(BYTE_ALIGN); } > writeableMem
    __CURADDR__ = __CURADDR__ + SIZEOF(.rwdata);

    ////////////////////////////////
    // SDA section
    ////////////////////////////////
    _SDA_BASE_ = __CURADDR__;
    .sdata __CURADDR__ : AT(__WRITEABLE_ADDR__ + SIZEOF(.rela) + SIZEOF(.rwdata)) { *(.sdata) . = ALIGN(BYTE_ALIGN); } > writeableMem
    __CURADDR__ = __CURADDR__ + SIZEOF(.sdata);
    .sbss (__CURADDR__) : AT(__WRITEABLE_ADDR__ + SIZEOF(.rela) + SIZEOF(.rwdata) + SIZEOF(.sdata)) { *(.sbss) . = ALIGN(BYTE_ALIGN); } > writeableMem
    __CURADDR__ = __CURADDR__ + SIZEOF(.sbss);

    ////////////////////////////////
    // writeable section length
    ////////////////////////////////
   __WRITEABLE_DATA_LEN__ = (__WRITEABLE_ADDR__ + SIZEOF(.sdata) + SIZEOF(.rela) + SIZEOF(.rwdata)) - __WRITEABLE_ADDR__;

    /DISCARD/ : {
       *(.eh_frame)
    }

}
