// $Id: ppc405_mmu.c,v 1.1.1.1 2013/12/11 21:03:27 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/ppc405/ppc405_mmu.c,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file ppc405_mmu.c
/// \brief Functions related to the PPC405 MMU and its use in SSX.
///
/// SSX currently only supports using the PPC405 MMU for instruction and data
/// area translation and protection for the global SSX application and kernel.
/// No support for demand paging, user vs. kernel protection or
/// thread-specific protection is provided.
///
/// Instead, the ppc405_mmu_map() API is provided.  This API sets up TLB
/// entries that provide address translation and protection for a region of
/// memory.  TLB entries are "locked", that is, once a TLB entry is defined it
/// is permanently defined.  SSX makes use of the variable page sizes provided
/// by the PPC405 to protect regions using the minimum number of TLB entries.
/// The minimum page size is 1K, and all regions to be protected must be 1K
/// aligned and have sizes that are multiples of 1K.
///
/// The ppc405_mmu_map() API optionally returns a Ppc405MmuMap descriptor.
/// This descriptor can be later used as the argument to ppc405_mmu_unmap() to
/// unmap the region.
///
/// The overall SSX configuration option is PPC405_MMU_SUPPORT, with
/// suboptions PPC405_IR_SUPPORT and PPC405_DR_SUPPORT.  The IR (instruction
/// relocate) and DR (data relocate) bits of the MSR must be set to enable
/// instruction and data translation/protection respectively. In SSX this is
/// handled by the definition of a macro PPC405_RELOCATION_MODE that contains
/// the IR and/or DR bits as configured.  This macro is OR'ed with the default
/// PPC405 SSX_THREAD_MACHINE_CONTEXT_DEFAULT. If the application defines its
/// own SSX_THREAD_MACHINE_CONTEXT_DEFAULT then the application will have to
/// take care of ensuring that the correct IR/DR settings go into the default.
///
/// During interrupts and context switches the relocation mode is
/// re-established before any loads or stores take place which provides
/// complete protection for interrupt handlers. Note the
/// assumption/requirement that all kernel, interrupt and thread code will be
/// run under the PPC405_RELOCATION_MODE.

#include "ssx.h"
#include "ppc405_mmu.h"

// A map of free TLB entries.
//
// It's handy that the PPC405 TLB has 64 entries.  Thus we can use a 64-bit
// bit mask to represent free entries.  The next free entry is quickly found
// using cntlz64().
uint64_t __ppc405_tlb_free = 0;

/// Reset the PPC405 simple MMU translation/protection scheme
///
/// This API invalidates the TLB, clears the zone protection register, and
/// otherwise resets the SSX simple translation/protcetion scheme for the
/// PPC405.  The application must not be running in a translated mode when
/// this API is invoked.
///
/// \retval 0 Success
///
/// \retval -PPC405_MMU_ILLEGAL_CONTEXT The API was called with translation
/// enabled.

int
ppc405_mmu_reset()
{
    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF(mfmsr() & (MSR_IR | MSR_DR),
                     PPC405_MMU_ILLEGAL_CONTEXT);
    }

    tlbia();
    mtspr(SPRN_PID, 0);
    mtspr(SPRN_ZPR, 0);
    __ppc405_tlb_free = 0xffffffffffffffffull;
    isync();

    return 0;
}


/// Define a memory region for MMU protection purposes
///
/// \param effective_address The effective (virtual) base address of the
/// region. This address must be at least 1KB aligned.
///
/// \param real_address The real base address of the region.  This address
/// must be at least 1KB aligned.
///
/// \param size The size of the region in bytes.  The size must be a multiple
/// of 1KB. A \a size of 0 is legal and creates no entries.
///
/// \param tlbhi_flags An OR combination of PPC405_TLBHI_* bits (excluding the
/// V-bit). This parameter is unlikely to be non-0 as it contains only the
/// little-endian (E) and U0 flags.
///
/// - TLBHI_E : Little Endian
/// - TLBHI_U0 : U0 mode access
///
/// \param tlblo_flags An OR combination of PPC405_TLBLO_* bits.  This
/// parameter defines WR and EX protection, as well as the 'WIMG' bits.
///
/// - TLBLO_EX : Executable code
/// - TLBLO_WR : Writable data
/// - TLBLO_W  : Write-through mode
/// - TLBLO_I  : Cache-inhibited
/// - TLBLO_M  : Memory Coherent (Implemented but Ignored)
/// - TLBLO_G  : Guarded
///
/// \param map If non-0, then a Ppc405TlbMap for the region is returned.
/// This map can be later used as an argument to ppc405_mmu_unmap() to unmap
/// the region.
///
/// This API creates fixed TLB entries that provide virtual-to-real address
/// translation and protection using a minimum number of TLB entries.  The
/// number of TLB entries is fixed, so there is no guarantee in general that
/// any particular memory map is feasible. In general it is helpful to make
/// sure that the effective and real memory ranges have similar alignment,
/// otherwise the algorithm will be forced to use small page sizes.
///
/// The caller is responsible for cache-correctness of this API.  If necessary
/// the caller should flush or invalidate memory areas whose protection
/// attributes have changed prior to and/or after invoking this API.
///
/// Note the the simple translation/protection protocol supported by this SSX
/// API does not support the "zone selection" field of the PPC405 TLB
/// entry. In SSX the PID is always 0.
///
/// If SSX_ERROR_CHECK_API is configured, the API checks each new TLB entry to
/// ensure that it does not duplicate an existing entry.  The check only
/// covers duplicated effective addresses (which are not supported by the
/// hardware), not the real addresses.
///
/// \retval 0 Success
///
/// \retval -PPC405_MMU_INVALID_ARGUMENT Can be signalled by numerous errors
/// including improperly aigned memory regions, region size not a multiple of
/// 1KB, and illegal flags.
///
/// \retval -PPC405_TOO_MANY_TLB_ENTRIES There are not enough TLB entries left
/// to completely map the region. The state of the TLB at this point may be
/// inconsistent.
///
/// \retval -PPC405_DUPLICATE_TLB_ENTRY The requested mapping would duplicate
/// all or part of a currently existing TLB entry.  Duplicate entries are not
/// supported in the 405 core and will cause a TLB miss if an address in a
/// duplicated range is accessed.

int
ppc405_mmu_map(SsxAddress effective_address, 
               SsxAddress real_address,
               size_t size, int tlbhi_flags, int tlblo_flags,
               Ppc405MmuMap *map)
{
    size_t log_page_size;
    size_t page_size = 0;
    Ppc405Tlbhi tlbhi;
    Ppc405Tlblo tlblo;
    Ppc405MmuMap local_map = 0;
    int tlb_entry;
    uint64_t bit;
    SsxMachineContext ctx;

    if (SSX_ERROR_CHECK_API) {
        uint64_t allocated;
        SsxAddress this_effective_address;
        int entry, overlap;

        // Check alignment, wrapping and legal flags.

        SSX_ERROR_IF((effective_address % PPC405_PAGE_SIZE_MIN)           ||
                     (real_address % PPC405_PAGE_SIZE_MIN)                ||
                     (size % PPC405_PAGE_SIZE_MIN)                        ||
                     ((effective_address + size - 1) < effective_address) ||
                     (tlbhi_flags & ~TLBHI_LEGAL_FLAGS)                   ||
                     (tlblo_flags & ~TLBLO_LEGAL_FLAGS),
                     PPC405_MMU_INVALID_ARGUMENT);

        // The check for duplicate entries needs to be done iteratively since
        // we don't use a fixed page size.  Since this API will probably only
        // be called during initialization or from thread contexts, and since
        // the TLB size is small, this overhead is not considered too onerous.
        // For complete correctness this check would need to be done in its
        // entirity in an SSX_CRITICAL critical section. In order to reduce
        // SSX_CRITICAL interrupt latency we simply check against the TLB
        // entries that were allocated at the time of the API call.  This code
        // may not protect against multiple threads simultaneously creating
        // mappings that duplicate each other (a super-bug), but it should
        // protect against bugs in a single thread's updating of the TLB.

        if (size != 0) {

            // See if the requested effective address is already mapped in the
            // TLB

            overlap = tlbsx(effective_address, &entry);

            // Iteratively check the other overlap condition, which is the
            // case that the effective address of any TLB entry is in the
            // range of the new request.

            allocated = ~__ppc405_tlb_free;
            while (!overlap && (allocated != 0)) {

                entry = cntlz64(allocated);
                allocated &= ~((uint64_t)1 << (63 - entry));

                tlbhi.value = tlbrehi(entry);
                if (tlbhi.fields.v) {

                    this_effective_address = 
                        tlbhi.fields.epn << PPC405_LOG_PAGE_SIZE_MIN;

                
                    // See if the first byte of this entry is inside the
                    // requested effective address range.  NB: use actual
                    // address ranges (addr + size - 1) to compute overlap to
                    // avoid overflow.

                    overlap |= 
                        (this_effective_address >= 
                         effective_address) &&
                        (this_effective_address <=
                         (effective_address + size - 1));
                }
            }
            SSX_ERROR_IF(overlap, PPC405_DUPLICATE_TLB_ENTRY);
        }
    }

    // NB: PPC405 page sizes go from 1K to 16M by factors of 4. This is a
    // 'greedy' algorithm that packs the region into the fewest number of
    // pages, by using the largest possible (aligned) page size for the
    // remaining memory area.

    while (size != 0) {

        ssx_critical_section_enter(SSX_CRITICAL, &ctx);

        if (SSX_ERROR_CHECK_API) {
            SSX_ERROR_IF_CRITICAL(__ppc405_tlb_free == 0, 
                                  PPC405_TOO_MANY_TLB_ENTRIES,
                                  &ctx);
        }

        tlb_entry = cntlz64(__ppc405_tlb_free);
        bit = 0x8000000000000000ull >> tlb_entry;
        __ppc405_tlb_free &= ~bit;
        local_map |= bit;

        ssx_critical_section_exit(&ctx);

        log_page_size = PPC405_LOG_PAGE_SIZE_MAX;
        do {
            page_size = POW2_32(log_page_size);
            if ((size >= page_size) && 
                ((effective_address & (page_size - 1)) == 0) &&
                ((real_address & (page_size - 1)) == 0)) {
                break;
            } else {
                log_page_size -= 2;
            }
        } while (1);

        size -= page_size;

        // Create and install the TLB entries.  The installation is done in a
        // critical section to avoid any chance of another entity seeing an
        // inconsistent TLB.

        tlbhi.value = tlbhi_flags;
        tlbhi.fields.epn = effective_address >> PPC405_LOG_PAGE_SIZE_MIN;
        tlbhi.fields.size = (log_page_size - PPC405_LOG_PAGE_SIZE_MIN) / 2;
        tlbhi.fields.v = 1;

        tlblo.value = tlblo_flags;
        tlblo.fields.rpn = real_address >> PPC405_LOG_PAGE_SIZE_MIN;

        ssx_critical_section_enter(SSX_CRITICAL, &ctx);

        tlbwelo(tlb_entry, tlblo.value);
        tlbwehi(tlb_entry, tlbhi.value);
        isync();

        ssx_critical_section_exit(&ctx);

        effective_address += page_size;
        real_address += page_size;
    }

    if (map) {
        *map = local_map;
    }

    return 0;
}


/// Un-define a memory region for MMU protection purposes
///
/// \param map A pointer to a Ppc405MmuMap object created by ppc405_mmu_map()
/// when the memory region was mapped.  This map is used to invalidate the TLB
/// entries associated with the map, then the map itself is invalidated.
///
/// The caller is responsible for cache-correctness of this API.  If necessary
/// the caller should flush or invalidate memory areas whose protection
/// attributes have changed prior to and/or after invoking this API.
///
/// \retval 0 Success
///
/// \retval -PPC405_MMU_INVALID_ARGUMENT The \a map pointer is null (0).

int
ppc405_mmu_unmap(Ppc405MmuMap *map)
{
    int tlb_entry;
    uint64_t bit;
    SsxMachineContext ctx;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF(map == 0, PPC405_MMU_INVALID_ARGUMENT);
    }

    while ((tlb_entry = cntlz64(*map)) != 64) {

        bit = 0x8000000000000000ull >> tlb_entry;
        *map &= ~bit;
        tlbwehi(tlb_entry, 0);
        isync();

        ssx_critical_section_enter(SSX_CRITICAL, &ctx);
        
        __ppc405_tlb_free |= bit;

        ssx_critical_section_exit(&ctx);
    }
    
    return 0;
}


/// Produce a dump of the TLB
///
/// \param[in] i_stream The output stream for the dump
///
/// \param[in] i_map An optional pointer.  If NULL (0) then a full report is
/// printed.  If non-null then only the entries recorded in the \a i_map are
/// printed.

void
ppc405_mmu_report(FILE* i_stream, Ppc405MmuMap* i_map)
{
    size_t i;
    Ppc405Tlbhi tlbhi;
    Ppc405Tlblo tlblo;
    uint32_t size, eff_lo, eff_hi, real_lo, real_hi;
    const char *size_string[] = {
        "  1K", "  4K", " 16K", " 64K", "256K", "  1M", "  4M", " 16M"
    };
    Ppc405MmuMap map;

    fprintf(i_stream, "------------------------------------------------------------------------------\n");
    if (i_map == 0) {
        fprintf(i_stream, "--                 PPC405 MMU : Full Report                                 --\n");
    } else {
        fprintf(i_stream, "--                 PPC405 MMU : Partial Report                              --\n");
    }
    fprintf(i_stream, "------------------------------------------------------------------------------\n");
    fprintf(i_stream, "--  #       Effective               Real         Size   EX/WR   WIMG  Other --\n");
    fprintf(i_stream, "------------------------------------------------------------------------------\n");
    
    if (i_map == 0) {
        map =  __ppc405_tlb_free; 
    } else {
        map = ~*i_map;
    }

    for (i = 0; i < PPC405_TLB_ENTRIES; i++, map <<= 1) {

        if (map & 0x8000000000000000ull) {
            continue;
        }

        tlbhi.value = tlbrehi(i);
        tlblo.value = tlbrelo(i);

        if (tlbhi.fields.v) {

            size = 
                POW2_32(PPC405_LOG_PAGE_SIZE_MIN) << (2 * tlbhi.fields.size);

            eff_lo = tlbhi.fields.epn << PPC405_LOG_PAGE_SIZE_MIN;
            eff_hi = eff_lo + size - 1;

            real_lo = tlblo.fields.rpn << PPC405_LOG_PAGE_SIZE_MIN;
            real_hi = real_lo + size - 1;

            fprintf(i_stream, "-- %2d : %08x:%08x -> %08x:%08x : %s : %s %s : %s%s%s%s : %s%s  --\n",
                   i, 
                   eff_lo, eff_hi,
                   real_lo, real_hi,
                   size_string[tlbhi.fields.size],
                   tlblo.fields.ex ? "EX" : "  ",
                   tlblo.fields.wr ? "WR" : "  ",
                   tlblo.fields.w ? "W" : " ",
                   tlblo.fields.i ? "I" : " ",
                   tlblo.fields.m ? "M" : " ",
                   tlblo.fields.g ? "G" : " ",
                   tlbhi.fields.e ? "E" : " ",
                   tlbhi.fields.u0 ? "U0" : "  ");
        } else {
            fprintf(i_stream, "-- %2d : ENTRY NOT VALID\n", i);
        }
    }

    fprintf(i_stream, "------------------------------------------------------------------------------\n");
}


/// Perform a memcpy() without address translation (protection)
///
/// It sometimes arises that "read-only" data needs to be initialized at
/// run-time. This can be accomplished in general by temporarily disabling
/// translation (protection) while the "read-only" data is altered.  Another
/// option is to use the memcpy_real() API to copy an image of the data from
/// writable memory to memory marked read-only by the MMU.
///
/// The memcpy_real() function copies \a n bytes from memory area \a src to
/// memory area \a dest, with translation disabled. The memory areas should
/// not overlap. The memcpy_real() function returns a pointer to dest.
///
/// This is a general-purpose API that makes no assumption about the
/// cacheability of the data, and can also be used to move code from data
/// areas to text areas as the I-cache is always invalidated after the copy.
/// The algorithm is as follows:
///
/// - Flush the \a dest data from the D-cache
/// - Disable translation
/// - memcpy() the \a src to the \a dest
/// - Flush the \a dest data from the D-cache
/// - Invalidate the I-cache
/// - Re-enable translation (if it had been previously enabled)
///
/// \note Any synchronization access required for \a dest or \a src is the
/// responsibility of the caller.
void*
memcpy_real(void* dest, const void* src, size_t n)
{
    uint32_t msr;

    dcache_flush(dest, n);

    msr = mfmsr();
    mtmsr(msr & ~(MSR_IR | MSR_DR));
    sync();                     /* HW239446! */

    memcpy(dest, src, n);

    dcache_flush(dest, n);

    icache_invalidate_all();

    mtmsr(msr);
    sync();                       /* HW239446! */

    return dest;
}

    

    
