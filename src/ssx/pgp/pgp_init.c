// $Id: pgp_init.c,v 1.2 2014/03/14 16:34:34 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/pgp/pgp_init.c,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file pgp_init.c
/// \brief SSX initialization for PgP
///
/// The entry points in this routine are used during initialization.  This
/// code space can be deallocated and reassigned after application
/// initialization if required.

#include "ssx.h"
#include "pgp_vrm.h"
#include "simics_stdio.h"
#include "string_stream.h"

#if USE_RTX_IO
// This file is not avilable to OCC FW builds
#include "rtx_stdio.h"
#endif

// We need to make sure that the PLB arbiter is set up correctly to obtain
// highest performance in the PgP environment, and that PLB error reporting is
// appropriate.

// The PLB arbiter is configured to support fair arbitration of equal-priority
// requests, however we don't set priorities here. The default settings have
// been found to be acceptible so far. We do enable arbiter pipelining however.

// We do set the "plbarb_lockerr" bit so that the PLB arbiter will trap and
// hold the first PLB timeout address.

static void
plb_arbiter_setup()
{
    plb_pacr_t pacr;
    ocb_ocichsw_t oo;

    pacr.value = 0;
    pacr.fields.ppm = 1;        /* Fair arbitration */
    pacr.fields.hbu = 1;        /* High bus utilization */
    pacr.fields.rdp = 1;        /* 2-deep read pipelining */
    pacr.fields.wrp = 1;        /* 2-deep write pipelining */
    mtdcr(PLB_PACR, pacr.value);

    oo.value = in32(OCB_OCICHSW);
    oo.fields.plbarb_lockerr = 1;
    out32(OCB_OCICHSW, oo.value);
}


#if PPC405_MMU_SUPPORT

#include "ppc405_mmu.h"

// MMU regions
//
// The linker script provides a standard set of symbols that define the base
// address and size of each expected section. Any section with a non-0 size
// will be mapped in the MMU using protection attributes appropriate for the
// section.  All sections requiring different MMU attributes must be
// 1KB-aligned.  The OCI control register space is fixed and also mapped by
// the same mechanism.
//
// By default, caching is enabled for all sections other than the sections
// explicitly cache-inhibited.  Configuration options are provided to disable
// caching of text, data and both.  Note that sections that (may) contain code
// and data will be marked cache-inhibited if either text or data is globally
// configured as cache-inhibited.  Also note that "writethrough" can only be
// defined on cacheable data sections.

#ifdef CACHE_INHIBIT_ALL
#define CACHE_INHIBIT_TEXT 1
#define CACHE_INHIBIT_DATA 1
#endif

#if CACHE_INHIBIT_TEXT
#define TEXT_CACHEABILITY_FLAG TLBLO_I
#else
#define TEXT_CACHEABILITY_FLAG 0
#endif

#if CACHE_INHIBIT_DATA
#define DATA_CACHEABILITY_FLAG TLBLO_I
#define WRITETHROUGH_FLAG 0
#else
#define DATA_CACHEABILITY_FLAG 0
#define WRITETHROUGH_FLAG TLBLO_W
#endif


// This structure contains all of the fields necessary to create a MMU
// mapping.

typedef struct {
    SsxAddress base;
    size_t size;
    uint32_t tlbhi_flags;
    uint32_t tlblo_flags;
    Ppc405MmuMap* map;
} MmuRegion;

// The section table along with (default) MMU characteristics.  Global
// Ppc405MmuMap variables are defined for certain sections so that those
// mappings may be later modified.

Ppc405MmuMap G_ex_free_mmu_map;
Ppc405MmuMap G_applet0_mmu_map;
Ppc405MmuMap G_applet1_mmu_map;

static const MmuRegion mmu_regions[] = {

    {(SsxAddress)&_TEXT0_SECTION_BASE,
     (size_t)&_TEXT0_SECTION_SIZE,
     0, TEXT_CACHEABILITY_FLAG | TLBLO_EX, 0} ,

    {(SsxAddress)&_TEXT1_SECTION_BASE,
     (size_t)&_TEXT1_SECTION_SIZE,
     0, TEXT_CACHEABILITY_FLAG | TLBLO_EX, 0} ,

    {(SsxAddress)&_RODATA_SECTION_BASE,
     (size_t)&_RODATA_SECTION_SIZE,
     0, DATA_CACHEABILITY_FLAG, 0} ,

    {(SsxAddress)&_NONCACHEABLE_RO_SECTION_BASE,
     (size_t)&_NONCACHEABLE_RO_SECTION_SIZE,
     0, TLBLO_I, 0} ,

    {(SsxAddress)&_NONCACHEABLE_SECTION_BASE,
     (size_t)&_NONCACHEABLE_SECTION_SIZE,
     0, TLBLO_I | TLBLO_WR, 0} ,

    {(SsxAddress)&_WRITETHROUGH_SECTION_BASE,
     (size_t)&_WRITETHROUGH_SECTION_SIZE,
     0, DATA_CACHEABILITY_FLAG | WRITETHROUGH_FLAG | TLBLO_WR, 0} ,

    {(SsxAddress)&_DATA_SECTION_BASE,
     (size_t)&_DATA_SECTION_SIZE,
     0, DATA_CACHEABILITY_FLAG | TLBLO_WR, 0} ,

    {(SsxAddress)&_EX_FREE_SECTION_BASE,
     (size_t)&_EX_FREE_SECTION_SIZE,
     0, DATA_CACHEABILITY_FLAG | TEXT_CACHEABILITY_FLAG | TLBLO_EX | TLBLO_WR,
     &G_ex_free_mmu_map},

    {(SsxAddress)&_APPLET0_SECTION_BASE,
     (size_t)&_APPLET0_SECTION_SIZE,
     0, DATA_CACHEABILITY_FLAG | TEXT_CACHEABILITY_FLAG | TLBLO_WR | TLBLO_EX,
     &G_applet0_mmu_map},

    {(SsxAddress)&_APPLET1_SECTION_BASE,
     (size_t)&_APPLET1_SECTION_SIZE,
     0, DATA_CACHEABILITY_FLAG | TEXT_CACHEABILITY_FLAG | TLBLO_WR | TLBLO_EX,
     &G_applet1_mmu_map},

    {(SsxAddress)OCI_REGISTER_SPACE_BASE,
     (size_t)OCI_REGISTER_SPACE_SIZE,
     0, TLBLO_WR | TLBLO_I | TLBLO_G, 0} ,
};

/// PgP MMU setup
///
/// Run down the mmu_regions[] array and map all regions with non-0 sizes.
/// These are direct maps, setting the effective address to the physical
/// address.  Once the MMU is set up MMU protection is enabled.
///
/// Any OCC mappings of PBA space will have to be done elsewhere, as these
/// memory areas are controlled by pHyp, and the product firmware has no plans
/// to access main memory from the OCC.

static void
pgp_mmu_setup()
{
    int i, regions;

    ppc405_mmu_reset();

    regions = sizeof(mmu_regions) / sizeof(MmuRegion);
    for (i = 0; i < regions; i++) {
        if (mmu_regions[i].size != 0) {
            ppc405_mmu_map(mmu_regions[i].base,
                           mmu_regions[i].base,
                           mmu_regions[i].size,
                           mmu_regions[i].tlbhi_flags,
                           mmu_regions[i].tlblo_flags,
                           mmu_regions[i].map);
        }
    }

    ppc405_mmu_start();
}

#endif  /* PPC405_MMU_SUPPORT */


// I/O Initialization
//
// Initialize the SSX/Simics/Verification Serial I/O channels.  This is done
// early in the initialization to allow initialization code to use printk().
// If the application does not select one of the I/O methods then 'ssxout'
// defaults to the NULL stream and 'stdin', 'stdout' and 'stderr' are
// undefined.

#if USE_TRACE_IO || USE_EPM_IO

WrappingStream G_ssxout
SECTION_ATTRIBUTE(".noncacheable") = {{0}};

uint8_t G_ssxout_buffer[SSXOUT_TRACE_BUFFER_SIZE]
SECTION_ATTRIBUTE(".noncacheable") = {0};

#endif  // USE_TRACE_IO || USE_EPM_IO

static void
io_setup()
{
    //NB: These I/O options are listed in priority order - multiple options may
    //be selected.
#if USE_TRACE_IO

    // If the application chooses to use trace buffer output, the application
    // must define SSXOUT_TRACE_BUFFER_SIZE, and all output streams are merged
    // into a single trace buffer which locks low-level file operations in an
    // SSX_CRITICAL critical secton.

    /// \todo Split trace I/O mode into multiple streams

    wrapping_stream_create(&G_ssxout, &G_ssxout_buffer,
                           SSXOUT_TRACE_BUFFER_SIZE,
                           SSX_FILE_OP_LOCK_CRITICAL);

    stdout = (FILE *)(&G_ssxout);
    stderr = (FILE *)(&G_ssxout);
    ssxout = (FILE *)(&G_ssxout);

#elif USE_EPM_IO

    linear_stream_create(&G_ssxout, &G_ssxout_buffer,
                        SSXOUT_TRACE_BUFFER_SIZE,
                        SSX_FILE_OP_LOCK_CRITICAL);

    stdout = (FILE *)(&G_ssxout);
    stderr = (FILE *)(&G_ssxout);
    ssxout = (FILE *)(&G_ssxout);

#elif USE_RTX_IO

    rtx_stdin_create(&rtx_stdin);
    rtx_stdout_create(&rtx_stdout);
    rtx_stderr_create(&rtx_stderr);

    stdin = (FILE *)(&rtx_stdin);
    stdout = (FILE *)(&rtx_stdout);
    stderr = (FILE *)(&rtx_stderr);
    ssxout = (FILE *)(&rtx_stdout);

    printf("Initialize the RTX stdio.\n");
    printf("RTX stdin is not implemented.\n");

#elif USE_SIMICS_IO

    simics_stdin_create(&simics_stdin);
    simics_stdout_create(&simics_stdout);
    simics_stderr_create(&simics_stderr);

    stdin = (FILE *)(&simics_stdin);
    stdout = (FILE *)(&simics_stdout);
    stderr = (FILE *)(&simics_stderr);
    ssxout = (FILE *)(&simics_stdout);

#endif // I/O Configuration
}


/// PgP environment initial setup.
///
/// This is setup common to all PgP applications.  This setup takes place
/// during boot, before main() is called.

void
__pgp_setup()
{
    // All OCB interrupts are masked.  The SSX/PPC405 Boot code masks PPC405
    // PIT, FIT, and Watchdog interrupts.  All interrupts are also initially
    // set up as noncritical, non-debugged, edge-triggered, active-high, and
    // their status is cleared.  This clarifies IPL debugging as it eliminates
    // spurious "asserted" interrupts until the firmware comes in and actually
    // sets up the interrupt.

    out32(OCB_OIMR0, 0xffffffff);  /* Masked */
    out32(OCB_OIMR1, 0xffffffff);
    out32(OCB_OITR0, 0xffffffff);  /* Edge */
    out32(OCB_OITR1, 0xffffffff);
    out32(OCB_OIEPR0, 0xffffffff); /* Active High */
    out32(OCB_OIEPR1, 0xffffffff);
    out32(OCB_OCIR0, 0);           /* Noncritical */
    out32(OCB_OCIR1, 0);
    out32(OCB_OISR0_AND, 0);       /* Clear Status */
    out32(OCB_OISR1_AND, 0);
    out32(OCB_OUDER0, 0);          /* No Unconditional Debug Event */
    out32(OCB_OUDER1, 0);
    out32(OCB_ODHER0, 0);          /* No Debug Halt Event */
    out32(OCB_ODHER1, 0);

    // Setup requires SCOM, which requires a timeout. Therefore we need to set
    // up a default timebase frequency, which may be overridden during
    // ssx_initialize(). 

    __ssx_timebase_frequency_hz = 600000000;
    __ssx_timebase_frequency_khz = 600000;
    __ssx_timebase_frequency_mhz = 600;

    // Set up I/O.  This is done early in the initialization so that
    // initialization drivers can use printk().

    io_setup();

    // Cache the device identification and chip configuration
    _pgp_get_ids();
    _pgp_get_chip_configuration();

    // Set up the PLB arbiter

    plb_arbiter_setup();

    // If the MMU is enabled the base image MMU programming is set up, and the
    // MMU is activated.

#if PPC405_MMU_SUPPORT
    pgp_mmu_setup();
#endif

    // The PgP Async drivers are initialized.

    async_initialize();
}
