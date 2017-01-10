/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/linkocc.cmd $                                     */
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

// Description

// This linker script creates SRAM images of SSX applications for PgP. This
// script is processed through the C proprocessor to create
// configuration-dependent images.
//
// All sections with different MMU protection properties are 1KB-aligned, even
// when linked in real-addressing mode.
//
// NB: According to *info* manual for ld, it should not be necessary to specify
// the '.' in the section commands, e.g.,
//
//     .data.startup . : { *(.data.startup) } > sram
//
// However without these the sections are not aligned properly, as the linker
// seems to ignore the LC and move the section 'backwards' until it abuts
// (aligned) with the previous one.
//
// Info on PPC binaries:
//     http://devpit.org/wiki/Debugging_PowerPC_ELF_Binaries

// Need to do this so that elf32-powerpc is not modified!
#undef powerpc

#ifndef INITIAL_STACK_SIZE
#define INITIAL_STACK_SIZE 2000
#endif

// Always include occLinkInputFile for GNU builds
//#ifdef OCCMK
INCLUDE occLinkInputFile
//#endif

OUTPUT_FORMAT(elf32-powerpc);

//  Define the beginning of SRAM, the location of the PowerPC exception
//  vectors (must be 64K-aligned) and the location of the boot branch.

//  512 KB SRAM at the top of the 768K SRAM. SRAM starts at 0xfff00000
//  Here we start at 0xfff40000 because the bottom 256K is reserved for
//  -- IPC Common space (0xfff00000 - 0xfff01000)
//  -- GPE0 (0xfff01000 - 0xfff10000)
//  -- GPE1 (0xfff10000 - 0xfff20000)
//  -- GPE2 (0xfff20000 - 0xfff30000)
//  -- GPE3 (0xfff30000 - 0xfff40000)

#define origin            0xfff40000
#define vectors           0xfff40000
#define reset             0xffffffec
#define sram_size         0x00080000
#define sram_available    sram_size

// The SRAM controller aliases the SRAM at 8 x 128MB boundaries to support
// real-mode memory attributes using DCCR, ICCR etc.  Noncacheable access is
// the next-to-last 128MB PPC405 region. Write-though access is the
// next-to-next-to-last 128MB PPC405 region. For our purposes, this means that:
// -- 0xF7F00000 - 0xF7FC0000 is our noncacheable SRAM address space
// -- 0xEFF00000 - 0xEFFC0000 is our writethrough SRAM address space
// -- 0xFFF00000 - 0xFFFC0000 is cached for both reads and writes

#define noncacheable_offset 0x08000000
#define noncacheable_origin (origin - 0x08000000)

#define writethrough_offset 0x10000000
#define writethrough_origin (origin - 0x10000000)

// This is the offset from the ppc405 EVPR where the debug pointers can be
// found.
#define SSX_DEBUG_PTRS_OFFSET   0x820

//  Define SSX kernel text sections to be packed into nooks and crannies of
//  the exception vector area.  An option is provided _not_ to pack, to help
//  better judge the best way to pack. Note that any code eligible for packing
//  is considered 'core' code that will be needed by the application at
//  runtime.  Any header data is _always_ packed into .vectors_0000 however.
//
//  Note that in order to support MMU protection, we can't pack data along
//  with the text.  All of the packed data sections are thus left empty.


// .vectors_0000

#define text_0000 \
*(.vectors_0000)

#define data_0000 main.o(imageHeader)


// .vectors_0100

#define text_0100 \
ppc405_core.o(.text) \
ppc405_irq_core.o(.text)

#define data_0100


// .vectors_0c00

#if SSX_TIMER_SUPPORT
#define text_0c00_conditional
#else
#define text_0c00_conditional
#endif


#define text_0c00 \
text_0c00_conditional \
ppc405_cache_core.o(.text)

#define data_0c00


// .vectors_0f00

#if SSX_TIMER_SUPPORT

#if SSX_THREAD_SUPPORT
#define text_0f00_conditional \
ssx_timer_init.o(.text) \
ssx_timer_core.o(.text) \
ssx_semaphore_core.o(.text)
#endif  /* SSX_THREAD_SUPPORT */

#if !SSX_THREAD_SUPPORT
#define text_0f00_conditional \
ssx_timer_init.o(.text) \
ssx_timer_core.o(.text)
#endif  /* !SSX_THREAD_SUPPORT */

#else  /* SSX_TIMER_SUPPORT */

#define text_0f00_conditional
#endif  /* SSX_TIMER_SUPPORT */

#define text_0f00 \
text_0f00_conditional

#define data_0f00

// .vectors_2000

#if SSX_THREAD_SUPPORT
#define thread_text \
ssx_thread_init.o(.text) \
ssx_thread_core.o(.text) \
ppc405_irq_init.o(.text) \
ppc405_thread_init.o(.text) \
ssx_semaphore_init.o(.text)
#else
#define thread_text
#endif

#if PPC405_MMU_SUPPORT
#define mmu_text \
ppc405_mmu.o(.text)\
ppc405_mmu_asm.o(.text)
#else
#define mmu_text
#endif

#define text_2000 \
occhw_irq_init.o(.text) \
ppc405_cache_init.o(.text) \
ppc405_breakpoint.o(.text) \
occhw_cache.o(.text) \
ssx_stack_init.o(.text) \
thread_text \
mmu_text \
occhw_async.o(.text) \
occhw_async_ocb.o(.text) \
occhw_async_pba.o(.text) \
occhw_scom.o(.text) \
occhw_ocb.o(.text) \
occhw_pba.o(.text) \
occhw_id.o(.text) \
//occhw_centaur.o(.text) \
ppc405_lib_core.o(.text) \
ssx_core.o(.text)

#define data_2000

// .vectors_0000 is always packed with header information

#define pack_0000 text_0000 data_0000
#define nopack_0000

#ifndef NO_PACK_SSX

#define pack_0100 text_0100 data_0100
#define nopack_0100

#define pack_0c00 text_0c00 data_0c00
#define nopack_0c00

#define pack_0f00 text_0f00 data_0f00
#define nopack_0f00

#define pack_2000 text_2000 data_2000
#define nopack_2000

#else

#define pack_0100
#define nopack_0100 text_0100 data_0100

#define pack_0c00
#define nopack_0c00 text_0c00 data_0c00

#define pack_0f00
#define nopack_0f00 text_0f00 data_0f00

#define pack_2000
#define nopack_2000 text_2000 data_2000

#endif

#define init_text \
ssx_init.o(.text) \
ppc405_boot.o(.text) \
ppc405_init.o(.text) \
occhw_init.o(.text)

#ifndef PPC405_MMU_SUPPORT
ASSERT((0), "OCC Application Firmware can not be compiled without \
PPC405_MMU_SUPPORT compile flag")
#endif

// Define memory areas.

MEMORY
{
 sram         : ORIGIN = origin, LENGTH = sram_available
 noncacheable : ORIGIN = noncacheable_origin, LENGTH = sram_available
 writethrough : ORIGIN = writethrough_origin, LENGTH = sram_available
 boot         : ORIGIN = reset,  LENGTH = 20
}

// This symbol is only needed by external debug tools, so add this command
// to ensure that table is pulled in by the linker even if ppc405 code
// never references it.
EXTERN(ssx_debug_ptrs);

// NB: The code that sets up the MMU assumes that the linker script provides a
// standard set of symbols that define the base address and size of each
// expected section. Any section with a non-0 size will be mapped in the MMU
// using protection attributes appropriate for the section.  All sections
// requiring different MMU attributes must be 1KB-aligned.

// NOTE: __START_ADDR__, __READ_ONLY_DATA_LEN__, __WRITEABLE_DATA_ADDR__,
//       __WRITEABLE_DATA_LEN__ are used for the common image header macro

SECTIONS
{
    . = origin;
    . = vectors;

    _MEMORY_ORIGIN = .;
    _MEMORY_SIZE = sram_size;

    __START_ADDR__ = .;

    ////////////////////////////////
    // Text0
    ////////////////////////////////

    // Low-memory kernel code and any other code that would benefit from being
    // resident in lower-latency SRAM

    _TEXT0_SECTION_BASE = .;
    _PPC405_VECTORS_BASE = .;

    .exceptions . : {
         ___vectors = .;
         ppc405_exceptions.o(.vectors_0000)
         pack_0000
         . = ___vectors + 0x0100;
         ppc405_exceptions.o(.vectors_0100)
         . = ___vectors + SSX_DEBUG_PTRS_OFFSET;
         *(.debug_ptrs)
         ppc405_exceptions.o(.irq_exit_traces)
         pack_0100
         . = ___vectors + 0x0c00;
         ppc405_exceptions.o(.vectors_0c00)
         pack_0c00
         . = ___vectors + 0x0f00;
         ppc405_exceptions.o(.vectors_0f00)
         pack_0f00
         . = ___vectors + 0x2000;
         ppc405_exceptions.o(.vectors_2000)
         pack_2000
    } > sram

    // If we're not packing, then place 'core' code immediately after the
    // exception vectors.

   .nopack . : { nopack_0000 nopack_0100 nopack_0c00 nopack_0f00 nopack_2000 } > sram

    . = ALIGN(1024);
    _TEXT0_SECTION_SIZE = . - _TEXT0_SECTION_BASE;

    ////////////////////////////////
    // Noncacheable and Write-through Data
    ////////////////////////////////

    // When running without the MMU we need to carefully arrange things such
    // that the noncacheable and writethrough data is linked at the correct
    // aliased VMA while remaining loaded in contiguous LMA addresses.

#if PPC405_MMU_SUPPORT

#define ALIASED_SECTION(s) s . : {*(s)} > sram

#else

#define ALIASED_SECTION(s) \
    _LMA = . + _lma_offset; \
    s . : AT (_LMA) {*(s)}

#endif

#if !PPC405_MMU_SUPPORT
    . = . - noncacheable_offset;
    _lma_offset = noncacheable_offset;
#endif

     _NONCACHEABLE_RO_SECTION_BASE = .;

    ALIASED_SECTION(.noncacheable_ro)

    . = ALIGN(1024);
    _NONCACHEABLE_RO_SECTION_SIZE = . - _NONCACHEABLE_RO_SECTION_BASE;

    _NONCACHEABLE_SECTION_BASE = .;

    ALIASED_SECTION(.noncacheable)

    . = ALIGN(1024);
    _NONCACHEABLE_SECTION_SIZE = . - _NONCACHEABLE_SECTION_BASE;


#if !PPC405_MMU_SUPPORT
    . = . + noncacheable_offset - writethrough_offset;
    _lma_offset = writethrough_offset;
#endif


    _WRITETHROUGH_SECTION_BASE = .;

    ALIASED_SECTION(.writethrough)

    . = ALIGN(1024);
    _WRITETHROUGH_SECTION_SIZE = . - _WRITETHROUGH_SECTION_BASE;

#if !PPC405_MMU_SUPPORT
    . = . + writethrough_offset;
#endif

    ////////////////////////////////
    // Read-only Data
    ////////////////////////////////

    _RODATA_SECTION_BASE = .;

    // SDA2 constant sections .sdata2 and .sbss2 must be adjacent to each
    // other.  Our SDATA sections are small so we'll use strictly positive
    // offsets.

    _SDA2_BASE_ = .;
   .sdata2 . : { *(.sdata2) } > sram
   .sbss2  . : { *(.sbss2) } > sram

   // The .rodata.vclcommand section contains read-only VclCommandRecord for
   // the benefit of the vcl_console() command interpreter.

   _VCL_COMMAND_SECTION_BASE = .;
   .rodata.vclcommand . : { *(.rodata.vclcommand) } > sram
   _VCL_COMMAND_SECTION_SIZE = . - _VCL_COMMAND_SECTION_BASE;

   // The .rodata.vclthread section contains read-only VclThreadRecord for the
   // benefit of the thread command.

   _VCL_THREAD_SECTION_BASE = .;
   .rodata.vclthread . : { *(.rodata.vclthread) } > sram
   _VCL_THREAD_SECTION_SIZE = . - _VCL_THREAD_SECTION_BASE;

   // The .rodata.vclpackage section contains read-only char* pointers for the
   // benefit of the package command.

   _VCL_PACKAGE_SECTION_BASE = .;
   .rodata.vclpackage . : { *(.rodata.vclpackage) } > sram
   _VCL_PACKAGE_SECTION_SIZE = . - _VCL_PACKAGE_SECTION_BASE;

   // Other read-only data.

   .buildname . : { *(.buildname) } > sram
   .rodata . : { *(.rodata*) *(.got2) } > sram

    . = ALIGN(1024);
    _RODATA_SECTION_SIZE = . - _RODATA_SECTION_BASE;
    __READ_ONLY_DATA_LEN__ = . - __START_ADDR__;


    ////////////////////////////////
    // Text1
    ////////////////////////////////

    // The default text section

    _TEXT1_SECTION_BASE = .;

    // Initialization text.  If we ever do a scheme to get rid of
    // initialization text then this will have to be moved if we're also doing
    // MMU protection.

    .itext . : { init_text } > sram

    // Other text
    // It's not clear why boot.S is generating empty .glink,.iplt
    .glink . : { *(.glink) } > sram
    .otext . : { *(.text) *(.text.startup)} > sram

     __CTOR_LIST__ = .;
    .ctors . : { *(.ctors) } > sram
     __CTOR_END__ = .;

    . = ALIGN(1024);
    _TEXT1_SECTION_SIZE = . - _TEXT1_SECTION_BASE;

    ////////////////////////////////
    // Read-write Data
    ////////////////////////////////

    _DATA_SECTION_BASE = .;
    __WRITEABLE_DATA_ADDR__ = .;

    // SDA sections .sdata and .sbss must be adjacent to each
    // other.  Our SDATA sections are small so we'll use strictly positive
    // offsets.

    _SDA_BASE_ = .;
    .sdata  . : { *(.sdata)  } > sram
    // Make sbss section 128 bytes aligned as linker is complaining while
    // compiling product applets that use global variables from the occ
    // application. OCC application is compiled with SDA data enabled and
    // applets are compiled with SDA sections not enabled.
    .sbss   . : { *(.sbss) . = ALIGN(128);  } > sram

    // Other read-write data
    // It's not clear why boot.S is generating empty .glink,.iplt

    .rela   . : { *(.rela*) *(.glink*) *(.iplt*) }  > sram
    .rwdata . : { *(.data) *(.bss) *(COMMON) . = ALIGN(128);  } > sram

    // Initialization-only data.  This includes the stack of main, the data
    // structures declared by INITCALL, and any other data areas that can be
    // reclaimed to the heap after initialization.
    //
    // NB: If we ever do reclaim this space, we need to modify the concept of
    // executable free space.

    _INIT_ONLY_DATA_BASE = .;

    // Put the stack right after the 405 main application and before the trace buffers
    _SSX_INITIAL_STACK_LIMIT = .;
    . = . + INITIAL_STACK_SIZE;
    _SSX_INITIAL_STACK = . - 1;

    _INITCALL_SECTION_BASE = .;
    .data.initcall . : { *(.data.initcall) } > sram
    _INITCALL_SECTION_SIZE = . - _INITCALL_SECTION_BASE;

    .data.startup . : { *(.data.startup) } > sram

    _INIT_ONLY_DATA_SIZE = . - _INIT_ONLY_DATA_BASE;

    ////////////////////////////////
    // Free Space
    ////////////////////////////////

    // If the configuration allows executing from free space - i.e.,
    // malloc()-ing a buffer and loading and executing code from it - then the
    // free space is separated and aligned so that it can be marked executable.
    // Otherwise it is simply read/write like the normal data sections.

#ifndef EXECUTABLE_FREE_SPACE
#define EXECUTABLE_FREE_SPACE 0
#endif

#if PPC405_MMU_SUPPORT && EXECUTABLE_FREE_SPACE
    . = ALIGN(1024);
#endif

    // The free space available to the program starts here.  This space does
    // not include the initial stack used by the boot loader and main().  The
    // initial stack space is considered part of the free 'section' for MMU
    // purposes.  Free space is always 8-byte aligned.
    //
    // Note that there is no data after _SSX_FREE_START.  When binary images
    // are created they can be padded to _SSX_FREE_START to guarantee
    // that .bss and COMMON data are zeroed, and that the images contain an
    // even multiple of 8 bytes (required for HW loaders).

     . = ALIGN(8);
    _EX_FREE_SECTION_BASE = .;
    _SSX_FREE_START = .;

#if EXECUTABLE_FREE_SPACE
    _DATA_SECTION_SIZE = . - _DATA_SECTION_BASE;
    __WRITEABLE_DATA_LEN__ = . - __WRITEABLE_DATA_ADDR__ ;
    _EX_FREE_SECTION_SIZE = _PING_PONG_BUFFER_BASE - _EX_FREE_SECTION_BASE;
#else
    _DATA_SECTION_SIZE = _PING_PONG_BUFFER_BASE - _DATA_SECTION_BASE;
    __WRITEABLE_DATA_LEN__ = _PING_PONG_BUFFER_BASE - __WRITEABLE_DATA_ADDR__ ;
    _EX_FREE_SECTION_SIZE = 0;
#endif

    _SSX_FREE_END   = _PING_PONG_BUFFER_BASE - 1;


     ////////////////////////////////
    // Ping/Pong Buffer Section
    //
    // Contains two 256-byte buffers used to tell the PGPE which vfrt to use
    //
    ////////////////////////////////
    __CUR_COUNTER__ = .;
    _PING_PONG_BUFFER_BASE = 0xfffb3d00;
    _PING_BUFFER_BASE = 0xfffb3d00;
    _PING_BUFFER_SIZE = 0x100;
    _PONG_BUFFER_BASE = 0xfffb3e00;
    _PONG_BUFFER_SIZE = 0x100;
    . = _PING_BUFFER_BASE;
#if !PPC405_MMU_SUPPORT
    . = . - writethrough_offset;
    _LMA = . + writethrough_offset;
    .vfrt_ping_buffer . : AT(_LMA) {*(vfrt_ping_buffer) . = ALIGN(_PING_BUFFER_SIZE);}
    _LMA = . + writethrough_offset;
    .vfrt_pong_buffer . : AT(_LMA) {*(vfrt_pong_buffer) . = ALIGN(_PONG_BUFFER_SIZE);}
    . = . + writethrough_offset;
#else
    .vfrt_ping_buffer . : {*(vfrt_ping_buffer) . = ALIGN(_PING_BUFFER_SIZE);} > sram
    .vfrt_pong_buffer . : {*(vfrt_pong_buffer) . = ALIGN(_PONG_BUFFER_SIZE);} > sram
#endif
    . = __CUR_COUNTER__;



    ////////////////////////////////
    // Global Data Section
    //
    // Contains pointers to important Global variables
    //
    ////////////////////////////////
    __CUR_COUNTER__ = .;
    _GLOBAL_DATA_BASE = 0xfffb3f00;
    _GLOBAL_DATA_SIZE = 0x100;
    . = _GLOBAL_DATA_BASE;
#if !PPC405_MMU_SUPPORT
    . = . - writethrough_offset;
    _LMA = . + writethrough_offset;
    .global_data . : AT(_LMA) {*(global_data) . = ALIGN(_GLOBAL_DATA_SIZE);}
    . = . + writethrough_offset;
#else
    .global_data . : {*(global_data) . = ALIGN(_GLOBAL_DATA_SIZE);} > sram
#endif
    . = __CUR_COUNTER__;


    ////////////////////////////////
    // Trace Buffers
    //
    // NOTE: If these addresses change, TMGT/HTMGT will require
    //       changes as well, to collect trace data in the event
    //       the OCC/405 dies unexpectedly.
    ////////////////////////////////
    __CUR_COUNTER__ = .;
    _ERR_TRACE_BUFFER_BASE = 0xfffb4000;
    _TRACE_BUFFERS_START_BASE = 0xfffb4000;
    _ERR_TRACE_BUFFER_SIZE = 0x2000;
    _INF_TRACE_BUFFER_BASE = 0xfffb6000;
    _INF_TRACE_BUFFER_SIZE = 0x2000;
    _IMP_TRACE_BUFFER_BASE = 0xfffb8000;
    _IMP_TRACE_BUFFER_SIZE = 0x2000;
    . = _ERR_TRACE_BUFFER_BASE;
#if !PPC405_MMU_SUPPORT
    . = . - writethrough_offset;
    _LMA = . + writethrough_offset;
    .err_trac . : AT (_LMA) {*(err_trac) . = ALIGN(_ERR_TRACE_BUFFER_SIZE);}
    _LMA = . + writethrough_offset;
    .inf_trac . : AT (_LMA) {*(inf_trac) . = ALIGN(_INF_TRACE_BUFFER_SIZE);}
    _LMA = . + writethrough_offset;
    .imp_trac . : AT (_LMA) {*(imp_trac) . = ALIGN(_IMP_TRACE_BUFFER_SIZE);}
    . = . + writethrough_offset;
#else
    .err_trac . : {*(err_trac) . = ALIGN(_ERR_TRACE_BUFFER_SIZE);} > sram
    .inf_trac . : {*(inf_trac) . = ALIGN(_INF_TRACE_BUFFER_SIZE);} > sram
    .imp_trac . : {*(imp_trac) . = ALIGN(_IMP_TRACE_BUFFER_SIZE);} > sram
#endif
    . = __CUR_COUNTER__;

    ////////////////////////////////
    // FIR data heap section
    ////////////////////////////////
    __CUR_COUNTER__ = .;
    _FIR_HEAP_SECTION_BASE = 0xfffba000;
    _FIR_HEAP_SECTION_SIZE = 0x3000;
    . = _FIR_HEAP_SECTION_BASE;

#if !PPC405_MMU_SUPPORT
    . = . - writethrough_offset;
    _LMA = . + writethrough_offset;
    .firHeap . : AT (_LMA) {*(firHeap) . = ALIGN(1024);}
    . = . + writethrough_offset;
#else
    .firHeap . : {*(firHeap) . = ALIGN(1024);} > sram
#endif

    . = __CUR_COUNTER__;

    ////////////////////////////////
    // FIR data parms section
    ////////////////////////////////
    __CUR_COUNTER__ = .;
    _FIR_PARMS_SECTION_BASE = 0xfffbd000;
    _FIR_PARMS_SECTION_SIZE = 0x1000;
    . = _FIR_PARMS_SECTION_BASE;

#if !PPC405_MMU_SUPPORT
    . = . - noncacheable_offset;
    _LMA = . + noncacheable_offset;
    .firParms . : AT (_LMA) {*(firParms) . = ALIGN(1024);}
    . = . + noncacheable_offset;
#else
    .firParms . : {*(firParms) . = ALIGN(1024);} > sram
#endif

    . = __CUR_COUNTER__;

    ////////////////////////////////
    // FSP Command Buffer
    ////////////////////////////////
    __CUR_COUNTER__ = .;

    _LINEAR_WR_WINDOW_SECTION_BASE = 0xfffbe000;
    _LINEAR_WR_WINDOW_SECTION_SIZE = 0x1000;
    _LINEAR_RD_WINDOW_SECTION_BASE = 0xfffbf000;     // Update FFDC_BUFFER_ADDR if changed
    _LINEAR_RD_WINDOW_SECTION_SIZE = 0x1000;
    . = _LINEAR_WR_WINDOW_SECTION_BASE;
#if !PPC405_MMU_SUPPORT
    . = . - noncacheable_offset;
    _LMA = . + noncacheable_offset;
    .linear_wr . : AT (_LMA) {*(linear_wr) . = ALIGN(_LINEAR_WR_WINDOW_SECTION_SIZE);}
#else
    .linear_wr . : {*(linear_wr) . = ALIGN(_LINEAR_WR_WINDOW_SECTION_SIZE);} > sram
#endif

#if !PPC405_MMU_SUPPORT
    . = . + noncacheable_offset - writethrough_offset;
    _LMA = . + writethrough_offset;
    .linear_rd . : AT (_LMA) {*(linear_rd) . = ALIGN(_LINEAR_RD_WINDOW_SECTION_SIZE);}
#else
    .linear_rd . : {*(linear_rd) . = ALIGN(_LINEAR_RD_WINDOW_SECTION_SIZE);} > sram
#endif

#if !PPC405_MMU_SUPPORT
    . = . + writethrough_offset;
#endif

    . = __CUR_COUNTER__;

    ////////////////////////////////
    //  TODO:     Previously, we were able to reclaim this space
    //            by loading applets over init data. The init data
    //            takes up around 6K. It would be good to figure 
    //            out what to do with it to regain the space.
    ////////////////////////////////
    //__CUR_COUNTER__ = .;
    //INIT_SECTION_BASE = 0xfffbf000;
    //. = INIT_SECTION_BASE;

    // Section aligned to 128 to make occ main application image 128 bytes
    // aligned which is requirement for applet manager when traversing through
    // all the image headers
    //initSection . :  { *(initSection) init_text . = ALIGN(128);} > sram

    //. = __CUR_COUNTER__;

    //////////////////////////////
    // End Of Memory
    //////////////////////////////

    _PPC405_END_OF_MEMORY = 0;
}


