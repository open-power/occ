// $Id: linkssx.cmd,v 1.2 2014/03/14 16:33:45 bcbrock Exp $

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

OUTPUT_FORMAT(elf32-powerpc);

//  Define the beginning of SRAM, the location of the PowerPC exception
//  vectors (must be 64K-aligned) and the location of the boot branch. 

//  512 KB SRAM at the top of the 32-bit address space

#define origin            0xfff80000
#define vectors           0xfff80000
#define reset             0xffffffec
#define sram_available    (reset - origin)
#define sram_size         0x00080000

// The SRAM controller aliases the SRAM at 8 x 128MB boundaries to support
// real-mode memory attributes using DCCR, ICCR etc.  Noncacheable access is
// the next-to-last 128MB PPC405 region. Write-though access is the
// next-to-next-to-last 128MB PPC405 region

#define noncacheable_offset 0x08000000
#define noncacheable_origin (origin - 0x08000000)

#define writethrough_offset 0x10000000
#define writethrough_origin (origin - 0x10000000)

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

#define data_0000


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
pgp_irq_init.o(.text) \
ppc405_cache_init.o(.text) \
ppc405_breakpoint.o(.text) \
pgp_cache.o(.text) \
ssx_stack_init.o(.text) \
thread_text \
mmu_text \
pgp_async.o(.text) \
pgp_async_pore.o(.text) \
pgp_async_ocb.o(.text) \
pgp_async_pba.o(.text) \
pgp_pmc.o(.text) \
pgp_ocb.o(.text) \
pgp_pba.o(.text) \
pgp_id.o(.text) \
pgp_centaur.o(.text) \
ppc405_lib_core.o(.text) \
ssx_core.o(.text) \

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
pgp_init.o(.text)       

// Define memory areas.

MEMORY
{
 sram         : ORIGIN = origin, LENGTH = sram_available
 noncacheable : ORIGIN = noncacheable_origin, LENGTH = sram_available
 writethrough : ORIGIN = writethrough_origin, LENGTH = sram_available
 boot         : ORIGIN = reset,  LENGTH = 20
}

// NB: The code that sets up the MMU assumes that the linker script provides a
// standard set of symbols that define the base address and size of each
// expected section. Any section with a non-0 size will be mapped in the MMU
// using protection attributes appropriate for the section.  All sections
// requiring different MMU attributes must be 1KB-aligned.

SECTIONS
{       
    . = origin;
    . = vectors;

    _MEMORY_ORIGIN = .;
    _MEMORY_SIZE = sram_size;
    
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

    // Non-cacheable and write-through data is placed in low memory to
    // improve latency.  PORE-private text and data is also placed here. PORE
    // text and data are segregated to enable relocated PORE disassembly of
    //.text.pore. PORE text is read-only to OCC, however PORE data is writable
    // by OCC to allow shared data structures (e.g., PTS).

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
    ALIASED_SECTION(.text.pore)

    . = ALIGN(1024);
    _NONCACHEABLE_RO_SECTION_SIZE = . - _NONCACHEABLE_RO_SECTION_BASE;
    

    _NONCACHEABLE_SECTION_BASE = .;   

    ALIASED_SECTION(.noncacheable)
    ALIASED_SECTION(.data.pore)

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

    // Accesses of read-only data may or may not benefit from being in fast
    // SRAM - we'll give it the benefit of the doubt.

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

   .rodata . : { *(.rodata*) *(.got2) } > sram

    . = ALIGN(1024);
    _RODATA_SECTION_SIZE = . - _RODATA_SECTION_BASE;

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

   .otext . : { *(.text) *(.text.startup)} > sram
   .glink . : { *(.glink) } > sram

    . = ALIGN(1024);
    _TEXT1_SECTION_SIZE = . - _TEXT1_SECTION_BASE;

    ////////////////////////////////
    // Read-write Data
    ////////////////////////////////

    _DATA_SECTION_BASE = .;

    // SDA sections .sdata and .sbss must be adjacent to each
    // other.  Our SDATA sections are small so we'll use strictly positive
    // offsets. 

    _SDA_BASE_ = .;
    .sdata  . : { *(.sdata)  } > sram
    .sbss   . : { *(.sbss)   } > sram

    // Other read-write data
    // It's not clear why boot.S is generating empty .glink,.iplt

   .rela   . : { *(.rela*) } > sram
   .rwdata . : { *(.data) *(.bss) } > sram
   .iplt . : { *(.iplt) } > sram


   // Initialization-only data.  This includes the stack of main, the data
   // structures declared by INITCALL, and any other data areas that can be
   // reclaimed to the heap after initialization. 
   //
   // NB: If we ever do reclaim this space, we need to modify the concept of
   // executable free space.

   _INIT_ONLY_DATA_BASE = .;

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
    _EX_FREE_SECTION_SIZE = 0 - _EX_FREE_SECTION_BASE;
#else
    _DATA_SECTION_SIZE = 0 - _DATA_SECTION_BASE;     
    _EX_FREE_SECTION_SIZE = 0;
#endif

    ////////////////////////////////
    // Applet areas
    ////////////////////////////////

    // These symbols are currently unused, but required to be defined.
    
    _APPLET0_SECTION_BASE = 0;
    _APPLET0_SECTION_SIZE = 0;
    _APPLET1_SECTION_BASE = 0;
    _APPLET1_SECTION_SIZE = 0;

    // The final 16 bytes of memory are reserved for the hardware boot branch

    _SSX_FREE_END = reset - 1;
}

