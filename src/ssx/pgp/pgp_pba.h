#ifndef __PGP_PBA_H__
#define __PGP_PBA_H__

// $Id: pgp_pba.h,v 1.2 2014/02/03 01:30:35 daviddu Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/pgp/pgp_pba.h,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file pgp_pba.h
/// \brief PBA unit header.  Local and mechanically generated macros.

/// \todo Add Doxygen grouping to constant groups

#include "pba_register_addresses.h"
#include "pba_firmware_registers.h"

#define POWERBUS_CACHE_LINE_SIZE 128
#define LOG_POWERBUS_CACHE_LINE_SIZE 7

/// The PBA OCI region is always either 0 or 3
#define PBA_OCI_REGION 0

// It is assumed the the PBA BAR sets will be assigned according to the
// following scheme.  There are still many open questions concerning PBA
// setup. 

/// The number of PBA Base Address Registers (BARS)
#define PBA_BARS 4

#define PBA_BAR_CHIP    0
#define PBA_BAR_NODE    2
#define PBA_BAR_SYSTEM  3
#define PBA_BAR_CENTAUR 1

#define PBA_BAR_OCC      0    /* OCC image (HOMER) */
#define PBA_BAR_PORE_SLW 2    /* Redundant mapping for SLW offset into HOMER */

// Standard PBA slave assignments, set up by FAPI procedure prior to releasing
// OCC from reset.

/// The number of PBA slaves
#define PBA_SLAVES 4

#define PBA_SLAVE_PORE_GPE 0    /* GPE0/1, but only 1 can access mainstore */
#define PBA_SLAVE_OCC      1    /* 405 I- and D-cache */
#define PBA_SLAVE_PORE_SLW 2    
#define PBA_SLAVE_OCB      3    

/// The maximum number of bytes a PBA block-copy engine can transfer at once
#define PBA_BCE_SIZE_MAX 4096

/// The base-2 log of the minimum PBA translation window size in bytes
#define PBA_LOG_SIZE_MIN 20

/// The base-2 log of the maximum PBA translation window size in bytes
///
/// Note that windows > 2**27 bytes require the extended address.
#define PBA_LOG_SIZE_MAX 41

/// The number of PBA slaves
#define PBA_SLAVES 4

/// The number of PBA read buffers
#define PBA_READ_BUFFERS 6

/// The number of PBA write buffers
#define PBA_WRITE_BUFFERS 2


////////////////////////////////////
// Macros for fields of PBA_SLVCTLn
////////////////////////////////////

// PBA write Ttypes

#define PBA_WRITE_TTYPE_DMA_PR_WR    0x0 /// DMA Partial Write
#define PBA_WRITE_TTYPE_LCO_M        0x1 /// L3 LCO for IPL, Tsize denotes chiplet.
#define PBA_WRITE_TTYPE_ATOMIC_RMW   0x2 /// Atomic operations
#define PBA_WRITE_TTYPE_CACHE_INJECT 0x3 /// Cache inject after IPL
#define PBA_WRITE_TTYPE_CI_PR_W      0x4 /// Cache-inhibited partial write for Centaur putscom().

#define PBA_WRITE_TTYPE_DC PBA_WRITE_TTYPE_DMA_PR_WR // Don't care


// PBA write Tsize is only required for PBA_WRITE_TTYPE_LCO_M (where it
// actually specifies a core chiplet id) and PBA_WRITE_TTYPE_ATOMIC_RMW.

#define PBA_WRITE_TSIZE_CHIPLET(chiplet) (chiplet)

#define PBA_WRITE_TSIZE_ARMW_ADD 0x03
#define PBA_WRITE_TSIZE_ARMW_AND 0x13
#define PBA_WRITE_TSIZE_ARMW_OR  0x23
#define PBA_WRITE_TSIZE_ARMW_XOR 0x33

#define PBA_WRITE_TSIZE_DC 0x0


// PBA write gather timeouts are defined in terms of the number of 'pulses'. A
// pulse occurs every 64 OCI cycles. The timing of the last write of a
// sequence is variable, so the timeout will occur somewhere between (N - 1) *
// 64 and N * 64 OCI cycles.  If write gather timeouts are disabled, the PBA
// holds the data until some condition occurs that causes it to disgorge the
// data. Slaves using cache-inhibited partial write never gather write
// data. Note from spec. : "Write gather timeouts must NOT be disabled if
// multiple masters are enabled to write through the PBA".  The only case
// where write gather timeouts will be disabled is for the IPL-time injection
// of data into the L3 caches. Note that disabling write-gathering timeouts is
// different from disabling write-gathering altogether with the
// SLVCTL.dis_write_gather setting.

#define PBA_WRITE_GATHER_TIMEOUT_DISABLE   0x0
#define PBA_WRITE_GATHER_TIMEOUT_2_PULSES  0x4
#define PBA_WRITE_GATHER_TIMEOUT_4_PULSES  0x5
#define PBA_WRITE_GATHER_TIMEOUT_8_PULSES  0x6
#define PBA_WRITE_GATHER_TIMEOUT_16_PULSES 0x7

/// PBA write gather timeout don't care assignment - see gpe_pba_parms_create()
#define PBA_WRITE_GATHER_TIMEOUT_DC -1


// PBA read Ttype

#define PBA_READ_TTYPE_CL_RD_NC      0x0 /// Cache line read
#define PBA_READ_TTYPE_CI_PR_RD      0x1 /// Cache-inhibited partial read for Centaur getscom().

/// PBA read TTYPE don't care assignment
#define PBA_READ_TTYPE_DC PBA_READ_TTYPE_CL_RD_NC      


// PBA read prefetch options

#define PBA_READ_PREFETCH_AUTO_EARLY  0x0 /// Aggressive prefetch
#define PBA_READ_PREFETCH_NONE        0x1 /// No prefetch
#define PBA_READ_PREFETCH_AUTO_LATE   0x2 /// Non-aggressive prefetch

/// PBA read prefetch don't care assignment - see gpe_pba_parms_create()
#define PBA_READ_PREFETCH_DC -1  


// PBA PowerBus command scope and priority, and PBA defaults

/// Nodal, Local Node
#define POWERBUS_COMMAND_SCOPE_NODAL 0x0 

/// Group, Local 4-chip, (aka, node pump)
#define POWERBUS_COMMAND_SCOPE_GROUP 0x1

/// System,  All units in the system
#define POWERBUS_COMMAND_SCOPE_SYSTEM 0x2

/// RGP, All units in the system (aka, system pump)
#define POWERBUS_COMMAND_SCOPE_RGP 0x3

/// Foreign, All units on the local chip, local SMP, and remote chip (pivot
/// nodes), In P8, only 100 and 101 are valid.
#define POWERBUS_COMMAND_SCOPE_FOREIGN0 0x4 

/// Foreign, All units on the local chip, local SMP, and remote chip (pivot
/// nodes), In P8, only 100 and 101 are valid.
#define POWERBUS_COMMAND_SCOPE_FOREIGN1 0x5


/// Default command scope for BCDE/BCUE transfers
#define PBA_POWERBUS_COMMAND_SCOPE_DEFAULT POWERBUS_COMMAND_SCOPE_NODAL



// Abstract fields of the PBA Slave Reset register used in pba_slave_reset(),
// which checks 'n' for validity.p

#define PBA_SLVRST_SET(n) (4 + (n))
#define PBA_SLVRST_IN_PROG(n) (0x8 >> (n))

/// The default timeout for pba_slave_reset().
///
/// Currently the procedure pba_slave_reset() is thought to be an
/// initialization-only and/or lab-only procedure, so this long polling
/// timeout is not a problem.
#ifndef PBA_SLAVE_RESET_TIMEOUT
#define PBA_SLAVE_RESET_TIMEOUT SSX_MICROSECONDS(100)
#endif 


// PBA Error/Panic codes

#define PBA_SCOM_ERROR1                    0x00722001
#define PBA_SCOM_ERROR2                    0x00722002
#define PBA_SLVRST_TIMED_OUT1              0x00722003
#define PBA_SLVRST_TIMED_OUT2              0x00722004
#define PBA_INVALID_ARGUMENT_BARSET        0x00779005
#define PBA_INVALID_ARGUMENT_RESET         0x00779006
#define PBAX_INVALID_ARGUMENT_CONFIG       0x00779007
#define PBAX_INVALID_ARGUMENT_TARGET       0x00779008
#define PBAX_INVALID_OBJECT                0x00722009

#ifndef __ASSEMBLER__

/// The PBA extended address in the form of a 'firmware register'
///
/// The extended address covers only bits 23:36 of the 50-bit PowerBus address.

typedef union pba_extended_address {

    uint64_t value;
    uint32_t word[2];
    struct {
        uint64_t reserved0        : 23;
        uint64_t extended_address : 14;
        uint64_t reserved1        : 27;
    } fields;
} pba_extended_address_t;


int 
pba_barset_initialize(int idx, uint64_t base, int log_size);

int
_pba_slave_reset(int id, SsxInterval timeout, SsxInterval sleep);

int
pba_slave_reset(int id);


////////////////////////////////////////////////////////////////////////////
// PBAX
////////////////////////////////////////////////////////////////////////////

// PBAX error/panic codes

#define PBAX_SEND_TIMEOUT  0x00722901
#define PBAX_SEND_ERROR    0x00722902
#define PBAX_RECEIVE_ERROR 0x00722903

/// The number of receive queues implemented by PBAX
#define PBAX_QUEUES 2

/// The number of PBAX Node Ids
#define PBAX_NODES 8

/// The number of PBAX Chip Ids (and group Ids)
#define PBAX_CHIPS 8
#define PBAX_GROUPS PBAX_CHIPS

/// The maximum legal PBAX group mask
#define PBAX_GROUP_MASK_MAX 0xff

// PBAX Send Message Scope

#define PBAX_GROUP  1
#define PBAX_SYSTEM 2

// PBAX Send Type

#define PBAX_UNICAST   0
#define PBAX_BROADCAST 1

// Default timeout for pbax_send()

#ifndef PBAX_SEND_DEFAULT_TIMEOUT
#define PBAX_SEND_DEFAULT_TIMEOUT SSX_MICROSECONDS(30)
#endif

/// An abstract target for PBAX send operations
///
/// This structure contains an abstraction of a communication target for PBAX
/// send operations.  An application using PBAX to transmit data first creates
/// an instance of the PbaxTarget for each abstract target using
/// pbax_target_create(), then calls pbax_send() or _pbax_send() with a
/// PbaxTarget and an 8-byte data packet to effect a transmission.
///
/// For applications that use GPE programs to implement PBAX sends, a pointer
/// to this object could also be passed to the GPE program.

typedef struct {

    /// The abstract target
    ///
    /// pbax_target_create() condenses the target parameters into a copy of
    /// the PBAXSNDTX register used to configure the transmission.
    pba_xsndtx_t target;

} PbaxTarget;


int
pbax_target_create(PbaxTarget* target,
                   int type, int scope, int queue, 
                   int node, int chip_or_group);

int
pbax_configure(int master, int node, int chip, int group_mask);

int
_pbax_send(PbaxTarget* target, uint64_t data, SsxInterval timeout);

int
pbax_send(PbaxTarget* target, uint64_t data);


/// Enable the PBAX send mechanism

static inline void
pbax_send_enable(void)
{
    pba_xcfg_t pxc;

    pxc.words.high_order = in32(PBA_XCFG);
    pxc.fields.pbax_en = 1;
    out32(PBA_XCFG, pxc.words.high_order);
    
}


/// Disable the PBAX send mechanism

static inline void
pbax_send_disable(void)
{
    pba_xcfg_t pxc;

    pxc.words.high_order = in32(PBA_XCFG);
    pxc.fields.pbax_en = 0;
    out32(PBA_XCFG, pxc.words.high_order);
    
}


/// Clear the PBAX send error condition

static inline void
pbax_clear_send_error(void)
{
    pba_xcfg_t pxc;

    pxc.words.high_order = in32(PBA_XCFG);
    pxc.fields.snd_reset = 1;
    out32(PBA_XCFG, pxc.words.high_order);
}    


/// Clear the PBAX receive error condition

static inline void
pbax_clear_receive_error(void)
{
    pba_xcfg_t pxc;

    pxc.words.high_order = in32(PBA_XCFG);
    pxc.fields.rcv_reset = 1;
    out32(PBA_XCFG, pxc.words.high_order);
}    

#endif /* __ASSEMBLER__ */

#endif  /* __PGP_PBA_H__ */
