/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/pgp/pgp.h $                                           */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2014,2016                        */
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
#ifndef __PGP_H__
#define __PGP_H__

/// \file pgp.h
/// \brief The PgP environment for SSX.

// This is a 'circular' reference in SSX, but included here to simplify PGAS
// programming. 

#ifndef CHIP_PGP
#define CHIP_PGP
#include "ppc405.h"
#endif  

// Can't include this here due to ordering issues.  It's included in
// ppc405_irq.h.
// #include "pgp_irq.h"

// Required for MMU Map declarations
#include "ppc405_mmu.h"

#include "pgp_common.h"
#include "pgp_core.h"
#include "pgp_trace.h"
#include "pgp_ocb.h"
#include "pgp_pba.h"
#include "pgp_pore.h"
#include "pgp_pmc.h"
#include "pgp_sramctl.h"
#include "pgp_vrm.h"
#include "pgp_id.h"
#include "pgp_centaur.h"

#include "pcbs_register_addresses.h"
#include "pcbs_firmware_registers.h"

#include "tod_register_addresses.h"
#include "tod_firmware_registers.h"

#include "plb_arbiter_register_addresses.h"
#include "plb_arbiter_firmware_registers.h"

#include "mcs_register_addresses.h"
#include "mcs_firmware_registers.h"

#include "centaur_firmware_registers.h"
#include "centaur_register_addresses.h"

#include "tpc_register_addresses.h"
#include "tpc_firmware_registers.h"

#include "oha_register_addresses.h"
#include "oha_firmware_registers.h"


// Include other driver headers

#include "pgp_async.h"

/// \defgroup memory_map Real-mode memory map setup for SRAM-resident applications
///
/// Below are the interpretations of the default settings of the real-mode
/// memory control registers.  All of the settings can be overridden as
/// compile switches for special test purposes.
///
/// The final 128MB of memory (the SRAM) is mapped both I- and
/// D-cacheable. The other 7 high-memory regions aliased by the SRAM remain
/// uncacheable, however SRAM alias region 29 is marked write-through.
/// 
/// Low memory addresses (direct-map Mainstore via PBA) are I-cacheable
/// but not D-cacheable to improve predicatability of execution times.
/// However, we should not execute from mainstore after initialization.
///
/// OCI register space (Segment '0b01') is marked guarded and non-cacheable.
///
/// All memory is big-endian.
///
/// @{

#ifndef PPC405_ICCR_INITIAL
#define PPC405_ICCR_INITIAL 0xff000001
#endif

#ifndef PPC405_DCCR_INITIAL
#define PPC405_DCCR_INITIAL 0x00000001
#endif

#ifndef PPC405_DCWR_INITIAL
#define PPC405_DCWR_INITIAL 0x00000004
#endif

#ifndef PPC405_SGR_INITIAL
#define PPC405_SGR_INITIAL  0x00ff0000
#endif

#ifndef PPC405_SU0R_INITIAL
#define PPC405_SU0R_INITIAL 0x00000000
#endif

#ifndef PPC405_SLER_INITIAL
#define PPC405_SLER_INITIAL 0x00000000
#endif

/// @}


/// PgP always runs from a memory image

#define SSX_RUN_FROM_MEMORY 1

/// This is the initial value of Cache Control Register 0 (CCR0) for PgP.
/// This definition can be overridden by the application.
///
/// The default setting:
///
/// - Enables parity checking in the caches and TLBs.  The parity mode is not
///   modified and defaults to 'imprecise'.
///
/// - Enables ICU prefetching for cacheable regions (Subject to final
///   performance evaluation).  Non-cacheable regions are not prefetched.
///
/// - Gives highest PLB priority to ICU fetches.  This setting can be
///   overriden by scan-only dials in the PgP design which force a fixed
///   priority on the ICU.
///
/// - Sets priority bit 1 to '1' for DCU operations.  The DCU sets priority
///   bit 0 automatically.  This setting can also be overridden by scan-only
///   dials that force a fixed priority on the DCU
///
/// Other options can be set at run time with the API \c ppc405_ccr0_modify().

#ifndef PPC405_CCR0_INITIAL
#define PPC405_CCR0_INITIAL                     \
    (CCR0_DPE | CCR0_IPE | CCR0_TPE |           \
     CCR0_PFC |                                 \
     CCR0_IPP0 | CCR0_IPP1 |                    \
     CCR0_DPP1)
#endif


#ifndef __ASSEMBLER__

/// \page noncacheable_support Non-cacheable modes for PgP
///
/// In order to support evaluation of cache management strategies on
/// performance, DMA buffers read/written by DMA devices can be declared as
/// NONCACHEABLE or NONCACHEABLE_RO, and DMA buffers read by DMA devices can
/// be declared as WRITETHROUGH.  However the code still does an explicit
/// FLUSH of these buffers before activating DMA devices. The configuration
/// option NONCACHEABLE_SUPPORT determines how NONCACHEABLE, NONCACAHEABLE_RO,
/// WRITETHROUGH and FLUSH are defined.
///
/// When noncacheable support is configured, the linker will link the
/// noncacheable and writethrough sections at a fixed offset from cacheable
/// address, depending on where SSX is loaded. Non-cacheable read-only
/// sections are enforced only if PPC405_MMU_SUPPORT is also configured.
/// Writethrogh sections are assumed to be read-write.
///
/// PGP_HIGH_MEMORY_LOAD
///
///     cacheable    : 0xfff8000 - 0xffffffff
///     noncacheable : 0xf7f8000 - 0xf7ffffff [cacheable - 128MB]
///     writethrough : 0xeff8000 - 0xefffffff [cacheable - 256MB]


#ifndef NONCACHEABLE_SUPPORT
#define NONCACHEABLE_SUPPORT 0
#endif


/// Declare an aligned data structure in a noncacheable RO section
///
/// This macro declares an aligned data structure in a noncacheable read-only
/// section.  The linker requires that data allocated in non-default sections
/// be initialized - so an initialization declaration for at least one element
/// of the data structure is required.  Use a value of 8 as the default
/// alignment.
///
/// See \ref noncacheable_support

#if NONCACHEABLE_SUPPORT

#define NONCACHEABLE_RO(declaration, alignment, initialization) \
    declaration __attribute__ \
    ((section (".noncacheable_ro"), aligned (alignment))) = initialization

#else

#define NONCACHEABLE_RO(declaration, alignment, initialization) \
    declaration __attribute__ \
    ((aligned (alignment))) = initialization

#endif  /* NONCACHEABLE_SUPPORT */


/// Declare an aligned data structure in a noncacheable RW section
///
/// This macro declares an aligned data structure in a noncacheable read-write
/// section.  The linker requires that data allocated in non-default sections
/// be initialized - so an initialization declaration for at least one element
/// of the data structure is required.  Use a value of 8 as the default
/// alignment.
///
/// See \ref noncacheable_support

#if NONCACHEABLE_SUPPORT

#define NONCACHEABLE(declaration, alignment, initialization) \
    declaration __attribute__ \
    ((section (".noncacheable"), aligned (alignment))) = initialization

#else

#define NONCACHEABLE(declaration, alignment, initialization) \
    declaration __attribute__ \
    ((aligned (alignment))) = initialization

#endif  /* NONCACHEABLE_SUPPORT */


/// Declare an aligned data structure in a write-through section
///
/// This macro declares an aligned data structure in a write-throgh section.
/// The linker requires that data allocated in non-default sections be
/// initialized - so an initialization declaration for at least one element of
/// the data structure is required.  Use a value of 8 as the default
/// alignment.
///
/// See \ref noncacheable_support

#if NONCACHEABLE_SUPPORT

#define WRITETHROUGH(declaration, alignment, initialization) \
    declaration __attribute__ \
    ((section (".writethrough"), aligned (alignment))) = initialization

#else

#define WRITETHROUGH(declaration, alignment, initialization) \
    declaration __attribute__ \
    ((aligned (alignment))) = initialization

#endif  /* NONCACHEABLE_SUPPORT */


/// Flush/invalidate a region of memory

#if NONCACHEABLE_SUPPORT
#define FLUSH(p, n) do {} while (0)
#define INVALIDATE(p, n) do {} while (0)
#else
#define FLUSH(p, n) do {dcache_flush((p), (n));} while (0)
#define INVALIDATE(p, n) do {dcache_invalidate((p), (n));} while (0)
#endif  /* NONCACHEABLE_SUPPORT */


/// The type of linker symbols
///
/// C++ and current GCC versions do not allow us to declare (C++) or take the
/// address of (C) a symbol of type void, which was an acceptable way to
/// declare linker symbols in earlier versions of GCC.  However if we declare
/// them of type 'char' or another integral type, the compiler will try to
/// make references to this 'data' appear to be in the small data areas (since
/// we're compiling with the PPC EABI), which causes the link to fail since
/// the symbols are actually defined in many different sections. The solution
/// is to declare them to be external references of a bogus type,
/// SsxLinkerSymbol, which is too large (9 bytes) to be stored in the small
/// data area.
///
/// This type definition is considered a required definition for a port of
/// SSX.

typedef struct {
    char bogus[9];
} SsxLinkerSymbol;

// Symbols defined by linkssx.cmd, used during MMU setup, byte-pool setup, and
// other purposes.

extern SsxLinkerSymbol _MEMORY_ORIGIN;
extern SsxLinkerSymbol _MEMORY_SIZE;
extern SsxLinkerSymbol _TEXT0_SECTION_BASE;
extern SsxLinkerSymbol _TEXT0_SECTION_SIZE;
extern SsxLinkerSymbol _TEXT1_SECTION_BASE;
extern SsxLinkerSymbol _TEXT1_SECTION_SIZE;
extern SsxLinkerSymbol _RODATA_SECTION_BASE;
extern SsxLinkerSymbol _RODATA_SECTION_SIZE;
extern SsxLinkerSymbol _NONCACHEABLE_RO_SECTION_BASE;
extern SsxLinkerSymbol _NONCACHEABLE_RO_SECTION_SIZE;
extern SsxLinkerSymbol _NONCACHEABLE_SECTION_BASE;
extern SsxLinkerSymbol _NONCACHEABLE_SECTION_SIZE;
extern SsxLinkerSymbol _WRITETHROUGH_SECTION_BASE;
extern SsxLinkerSymbol _WRITETHROUGH_SECTION_SIZE;
extern SsxLinkerSymbol _DATA_SECTION_BASE;
extern SsxLinkerSymbol _DATA_SECTION_SIZE;
extern SsxLinkerSymbol _EX_FREE_SECTION_BASE;
extern SsxLinkerSymbol _EX_FREE_SECTION_SIZE;
extern SsxLinkerSymbol _APPLET0_SECTION_BASE;
extern SsxLinkerSymbol _APPLET0_SECTION_SIZE;
extern SsxLinkerSymbol _APPLET1_SECTION_BASE;
extern SsxLinkerSymbol _APPLET1_SECTION_SIZE;
extern SsxLinkerSymbol _SSX_FREE_START;
extern SsxLinkerSymbol _SSX_FREE_END;

// Global MMU maps to allow remapping certain regions

extern Ppc405MmuMap G_ex_free_mmu_map;
extern Ppc405MmuMap G_applet0_mmu_map;
extern Ppc405MmuMap G_applet1_mmu_map;

#endif  /* __ASSEMBLER__ */

// PgP defines a private version of dcache_flush_all() that uses the undefined
// OCI space at 0x80000000; See dcache_flush_all() in pgp_cache.S.

#define USE_GENERIC_DCACHE_FLUSH_ALL 0
#define PGP_FLUSH_ZERO_ADDRESS       0x80000000
#define PGP_FLUSH_ZERO_DCCR          0x00008000

#endif  /* __PGP_H__ */
