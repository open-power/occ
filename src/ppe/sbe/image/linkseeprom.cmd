/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/sbe/image/linkseeprom.cmd $                           */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015                             */
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
// Need to do this so that elf32-powerpc is not modified!
#undef powerpc

#ifndef INITIAL_STACK_SIZE
#define INITIAL_STACK_SIZE 256
#endif

OUTPUT_FORMAT(elf32-powerpc);

SECTIONS
{
    . = 0xfff00000;
    _seeprom_origin = . - 0;
    ////////////////////////////////
    // Header
    ////////////////////////////////
   . = ALIGN(1); _header_origin = .; _header_offset = . - _seeprom_origin; .header . : { *(.header) } _header_size = . - _header_origin;

    ////////////////////////////////
    // FIXED
    ////////////////////////////////
    . = ALIGN(512); _fixed_origin = .; _fixed_offset = . - _seeprom_origin; .fixed . : { *(.fixed) }  _fixed_size = . - _fixed_origin;

   ////////////////////////////////
    // FIXED_TOC
    ////////////////////////////////
    . = ALIGN(8); _fixed_toc_origin = .; _fixed_toc_offset = . - _seeprom_origin; .fixed_toc . : { *(.fixed_toc) } _fixed_toc_size = . - _fixed_toc_origin;

    ////////////////////////////////
    // LOADER_TEXT
    ////////////////////////////////
    . = ALIGN(4); _loader_text_origin = .; _loader_text_offset = . - _seeprom_origin; .loader_text . : { *(.loader_text) } _loader_text_size = . - _loader_text_origin;

    ////////////////////////////////
    // LOADER_DATA
    ////////////////////////////////
    . = ALIGN(8); _loader_data_origin = .; _loader_data_offset = . - _seeprom_origin; .loader_data . : { *(.loader_data) } _loader_data_size = . - _loader_data_origin;

    ////////////////////////////////
    // TEXT
    ////////////////////////////////
    . = ALIGN(4); _text_origin = .; _text_offset = . - _seeprom_origin; .text . : {. = ALIGN(512); *(.vectors) *(.text) *(.eh_frame) } _text_size = . - _text_origin;

    ////////////////////////////////
    // DATA
    ////////////////////////////////
    . = ALIGN(8); _data_origin = .; _data_offset = . - _seeprom_origin; .data . : { *(.data) *(.comment) *(.rodata*)} _data_size = . - _data_origin;

   ////////////////////////////////
    // TOC
    ////////////////////////////////
    . = ALIGN(4); _toc_origin = .; _toc_offset = . - _seeprom_origin; .toc . : { *(.toc) } _toc_size = . - _toc_origin;

   ////////////////////////////////
    // STRING
    ////////////////////////////////
    . = ALIGN(1); _strings_origin = .; _strings_offset = . - _seeprom_origin; .strings . : { *(.strings) } _strings_size = . - _strings_origin;

//    . = ALIGN(8);
//    _seeprom_size = . - _seeprom_origin;
//    _seeprom_end = . - 0;


    _RODATA_SECTION_BASE = .;


    _SDA2_BASE_ = .;

    _RODATA_SECTION_SIZE = . - _RODATA_SECTION_BASE;

    // SDA sections .sdata and .sbss must be adjacent to each
    // other.  Our SDATA sections are small so we'll use strictly positive
    // offsets. 

    _SDA_BASE_ = .;

    .sbss   . : { *(.sbss)   }
    .sdata   . : { *(.sdata)  }


   _PK_INITIAL_STACK_LIMIT = .;
   . = . + INITIAL_STACK_SIZE;
   _PK_INITIAL_STACK = . - 1;


    . = ALIGN(8);
    _seeprom_size = . - _seeprom_origin;
    _seeprom_end = . - 0;


}
