/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/occhw/occhw_init.c $                                  */
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
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2014
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file occhw_init.c
/// \brief SSX initialization for OCCHW
///
/// The entry points in this routine are used during initialization.  This
/// code space can be deallocated and reassigned after application
/// initialization if required.

#include "ssx.h"
#include "occhw_async.h"

//#include "occhw_vrm.h"
#include "simics_stdio.h"
#include "string_stream.h"

#if USE_RTX_IO
// This file is not avilable to OCC FW builds
#include "rtx_stdio.h"
#endif

// We need to make sure that the PLB arbiter is set up correctly to obtain
// highest performance in the OCCHW environment, and that PLB error reporting is
// appropriate.

// The PLB arbiter is configured to support fair arbitration of equal-priority
// requests, however we don't set priorities here. The default settings have
// been found to be acceptible so far. We do enable arbiter pipelining however.

// We do set the "plbarb_lockerr" bit so that the PLB arbiter will trap and
// hold the first PLB timeout address.

static void
plb_arbiter_setup()
{
    //TODO: enable this once OCB support is present in simics
#if 0
    ocb_oacr_t oacr;
    ocb_ocichsw_t oo;

    oacr.value = 0;
    oacr.fields.oci_priority_mode = 1;          /* Fair arbitration */
    oacr.fields.oci_hi_bus_mode = 1;            /* High bus utilization */
    oacr.fields.oci_read_pipeline_control = 1;  /* 2-deep read pipelining */
    oacr.fields.oci_write_pipeline_control = 1; /* 2-deep write pipelining */
    mtdcr(OCB_OACR, oacr.value);

    oo.value = in32(OCB_OCICHSW);
    oo.fields.plbarb_lockerr = 1;
    out32(OCB_OCICHSW, oo.value);
#endif
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

/// OCCHW MMU setup
///
/// Run down the mmu_regions[] array and map all regions with non-0 sizes.
/// These are direct maps, setting the effective address to the physical
/// address.  Once the MMU is set up MMU protection is enabled.
///
/// Any OCC mappings of PBA space will have to be done elsewhere, as these
/// memory areas are controlled by pHyp, and the product firmware has no plans
/// to access main memory from the OCC.

static void
occhw_mmu_setup()
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

    printf("Initialize the Simics stdio.\n");

#endif // I/O Configuration
}

/// OCCHW environment initial setup.
///
/// This is setup common to all OCCHW applications.  This setup takes place
/// during boot, before main() is called.

void
__occhw_setup()
{
    uint64_t oirrA;
    uint64_t oirrB;
    uint64_t oirrC;
    uint64_t owned_actual;
    uint64_t reverse_polarity;

#if (APPCFG_OCC_INSTANCE_ID == OCCHW_IRQ_ROUTE_OWNER)
    //If this instance is the owner of the interrupt routting registers
    //then write the routing registers for all OCC interrupts.
    //This instance must be the first instance to run within the OCC
    //This will be done while all external interrupts are masked.
    out32(OCB_OIMR0_OR, 0xffffffff);
    out32(OCB_OIMR1_OR, 0xffffffff);
    out32(OCB_OIRR0A, (uint32_t)(g_ext_irqs_routeA >> 32));
    out32(OCB_OIRR1A, (uint32_t)g_ext_irqs_routeA);
    out32(OCB_OIRR0B, (uint32_t)(g_ext_irqs_routeB >> 32));
    out32(OCB_OIRR1B, (uint32_t)g_ext_irqs_routeB);
    out32(OCB_OIRR0C, (uint32_t)(g_ext_irqs_routeC >> 32));
    out32(OCB_OIRR1C, (uint32_t)g_ext_irqs_routeC);

    //Note: all interrupts are left in the masked state at this point
#endif

    //Determine from the routing registers which irqs are owned by this instance
    //NOTE: If a bit is not set in the routeA register, it is not owned by a GPE

    oirrA = ((uint64_t)in32(OCB_OIRR0A)) << 32;
    oirrA |= in32(OCB_OIRR1A);
    oirrB = ((uint64_t)in32(OCB_OIRR0B)) << 32;
    oirrB |= in32(OCB_OIRR1B);
    oirrC = ((uint64_t)in32(OCB_OIRR0C)) << 32;
    oirrC |= in32(OCB_OIRR1C);

    //All interrupts owned by the 405 will not have a bit set in routeA
    owned_actual = ~oirrA;

    //Panic if we don't own the irqs we were expecting
    //NOTE: we don't panic if we are given more IRQ's than expected
    if((owned_actual & g_ext_irqs_owned) != g_ext_irqs_owned)
    {
        //IRQ's were not routed to us correctly.
        SSX_PANIC(OCCHW_IRQ_ROUTING_ERROR);
    }

    //Mask all external interrupts owned by this instance
    //(even the ones given to us that we weren't expecting)
    out32(OCB_OIMR0_OR, (uint32_t)(owned_actual >> 32));
    out32(OCB_OIMR1_OR, (uint32_t)owned_actual);

    //Set the interrupt type for all interrupts owned by this instance
    out32(OCB_OITR0_CLR, (uint32_t)(g_ext_irqs_owned >> 32));
    out32(OCB_OITR1_CLR, (uint32_t)g_ext_irqs_owned);
    out32(OCB_OITR0_OR, (uint32_t)(g_ext_irqs_type >> 32));
    out32(OCB_OITR1_OR, (uint32_t)g_ext_irqs_type);

    //Set the interrupt polarity for all interrupts owned by this instance
    out32(OCB_OIEPR0_CLR, (uint32_t)(g_ext_irqs_owned >> 32));
    out32(OCB_OIEPR1_CLR, (uint32_t)g_ext_irqs_owned);
    out32(OCB_OIEPR0_OR, (uint32_t)(g_ext_irqs_polarity >> 32));
    out32(OCB_OIEPR1_OR, (uint32_t)g_ext_irqs_polarity);

    //clear the status of all external interrupts owned by this instance
    out32(OCB_OISR0_CLR, ((uint32_t)(g_ext_irqs_owned >> 32)));
    out32(OCB_OISR1_CLR, ((uint32_t)g_ext_irqs_owned));

    //set the status for interrupts that have reverse polarity
    reverse_polarity = ~g_ext_irqs_polarity & g_ext_irqs_owned;
    out32(OCB_OISR0_OR, ((uint32_t)(reverse_polarity >> 32)));
    out32(OCB_OISR1_OR, ((uint32_t)reverse_polarity));

    //Unmask the interrupts owned by this instance that are to be enabled by default
    out32(OCB_OIMR0_CLR, (uint32_t)(g_ext_irqs_enable >> 32));
    out32(OCB_OIMR1_CLR, (uint32_t)g_ext_irqs_enable);


    // Setup requires SCOM, which requires a timeout. Therefore we need to set
    // up a default timebase frequency, which may be overridden during
    // ssx_initialize(). 

    __ssx_timebase_frequency_hz = 600000000;
    __ssx_timebase_frequency_khz = 600000;
    __ssx_timebase_frequency_mhz = 600;

    // Set up I/O.  This is done early in the initialization so that
    // initialization drivers can use printk().

    io_setup();

    // TODO: enable once chip id support is present
#if 0
    // Cache the device identification and chip configuration
    _occhw_get_ids();
    _occhw_get_chip_configuration();
#endif

    // Set up the PLB arbiter

    plb_arbiter_setup();

    // If the MMU is enabled the base image MMU programming is set up, and the
    // MMU is activated.

#if PPC405_MMU_SUPPORT
    occhw_mmu_setup();
#endif

    // The Async drivers are initialized.
    async_initialize();
}
