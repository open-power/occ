#ifndef __GPE_CONTROL_H__
#define __GPE_CONTROL_H__

// $Id: gpe_control.h,v 1.1.1.1 2013/12/11 20:49:20 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/lib/gpe_control.h,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file gpe_control.h
/// \brief GPE procedures for control

#include "pstates.h"
#include "pgp_config.h"

////////////////////////////////////////////////////////////////////////////
// PcbsPstateRegs
////////////////////////////////////////////////////////////////////////////

#ifndef __ASSEMBLER__

/// Per-core Pstate control registers
///
/// Firmware maintains a vector of PcbsPstateRegs structures - one for
/// each core on the chip - and updates the register fields in place.  The GPE
/// procedure gpe_set_pstates() is run periodically to update the core
/// chiplets from this data structure.  The array can (should) be cleared
/// initially.

typedef struct {

    /// The clipping register
    pcbs_power_management_bounds_reg_t pmbr;

    /// The idle control register
    pcbs_power_management_idle_control_reg_t pmicr;

    /// The Pstate control register
    pcbs_power_management_control_reg_t pmcr;

} PcbsPstateRegs;

#endif  /* __ASSEMBLER__ */

//  Offsets into PcbsPstateRegs

#define PCBSPSTATEREGS_PMBR  0x00  
#define PCBSPSTATEREGS_PMICR 0x08 
#define PCBSPSTATEREGS_PMCR  0x10 

#define SIZEOF_PCBSPSTATEREGS 0x18


#ifndef __ASSEMBLER__

/// Set a chiplet Pmax clipping Pstate
static inline void
set_chiplet_pmax(PcbsPstateRegs *regs, int chiplet, Pstate pmax)
{
    regs[chiplet].pmbr.fields.pmax_clip = pmax;
}

/// Set a chiplet Pmin clipping Pstate
static inline void
set_chiplet_pmin(PcbsPstateRegs *regs, int chiplet, Pstate pmin)
{
    regs[chiplet].pmbr.fields.pmin_clip = pmin;
}

/// Set chiplet Global and Local Pstate requests
static inline void
set_chiplet_pstate(PcbsPstateRegs *regs, int chiplet, 
                   Pstate global, Pstate local)
{
    regs[chiplet].pmcr.fields.global_pstate_req = global;
    regs[chiplet].pmcr.fields.local_pstate_req = local;
}

/// Enable/Disable/Configure chiplet Nap Pstates
static inline void
set_chiplet_nap_pstate(PcbsPstateRegs *regs, int chiplet,
                       Pstate pstate, int enable, int global, int latency)
{
    regs[chiplet].pmicr.fields.nap_pstate_req = pstate;
    regs[chiplet].pmicr.fields.nap_pstate_en  = (enable != 0);
    regs[chiplet].pmicr.fields.nap_global_en  = (global != 0);
    regs[chiplet].pmicr.fields.nap_latency    = latency;
}

/// Enable/Disable/Configure chiplet Sleep Pstates
static inline void
set_chiplet_sleep_pstate(PcbsPstateRegs *regs, int chiplet,
                         Pstate pstate, int enable, int global, int latency)
{
    regs[chiplet].pmicr.fields.sleep_pstate_req = pstate;
    regs[chiplet].pmicr.fields.sleep_pstate_en  = (enable != 0);
    regs[chiplet].pmicr.fields.sleep_global_en  = (global != 0);
    regs[chiplet].pmicr.fields.sleep_latency    = latency;
}

/// Enable/Disable/Configure chiplet Winkle Pstates
static inline void
set_chiplet_winkle_pstate(PcbsPstateRegs *regs, int chiplet,
                          Pstate pstate, int enable, int global, int latency)
{
    regs[chiplet].pmicr.fields.winkle_pstate_req = pstate;
    regs[chiplet].pmicr.fields.winkle_pstate_en  = (enable != 0);
    regs[chiplet].pmicr.fields.winkle_global_en  = (global != 0);
    regs[chiplet].pmicr.fields.winkle_latency    = latency;
}

#endif /* __ASSEMBLER__ */

/// \bug These need to be defined and documented

#define SLEEP_LATENCY_DISABLED    0
#define SLEEP_LATENCY_CLOCKS_OFF  1
#define SLEEP_LATENCY_FAST        2
#define SLEEP_LATENCY_DEEP        3

#define WINKLE_LATENCY_DISABLED    0
#define WINKLE_LATENCY_CLOCKS_OFF  1
#define WINKLE_LATENCY_FAST        2
#define WINKLE_LATENCY_DEEP        3


////////////////////////////////////////////////////////////////////////////
// gpe_set_pstates()
////////////////////////////////////////////////////////////////////////////

#ifndef __ASSEMBLER__

///  Parameters for the GPE procedure gpe_set_pstates()

typedef struct {

    // The chip configuration (for actuation purposes).  Only those core
    // chiplets with bits set in the mask will be actuated.
    ChipConfig config;

    /// This mask, comprised of a logical OR of the GPE_SET_PSTATE_*
    /// macros, controls which register(s) is(are) actuated for each core.
    uint64_t select;

    /// The 32-bit pointer to the array of PcbsPstateRegs holding the register
    /// data, coerced to a 64-bit unsigned.  The real 32-bit pointer must be
    /// the low-order 32 bits of this value.
    uint64_t regs;

} GpeSetPstatesParms;

PoreEntryPoint gpe_set_pstates;

#endif /* __ASSEMBLER__ */

// Parameter offsets for gpe_set_pstates()

#define GPESETPSTATESPARMS_CONFIG                    0x00
#define GPESETPSTATESPARMS_SELECT                    0x08
#define GPESETPSTATESPARMS_REGS                      0x10

// Register/Function select masks for gpe_set_pstates()

#define GPE_SET_PSTATES_PMBR  0x01
#define GPE_SET_PSTATES_PMICR 0x02
#define GPE_SET_PSTATES_PMCR  0x04
#define GPE_SET_PSTATES_SYNC  0x08

#endif  /* __GPE_CONTROL_H__ */
