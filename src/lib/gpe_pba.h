#ifndef __GPE_PBA_H__
#define __GPE_PBA_H__

// $Id: gpe_pba.h,v 1.1.1.1 2013/12/11 20:49:20 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/lib/gpe_pba.h,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file gpe_pba.h
/// \brief PBA subroutines for PORE-GPE procedures

// Error/Panic codes

#define GPE_INVALID_OBJECT   0x00473001
#define GPE_INVALID_ARGUMENT 0x00473002


#ifndef __ASSEMBLER__

/// Encapsulated PBA setup for GPE programs
///
/// All GPE programs that access mainstore via PBA utilize a common parameter
/// structure that encapsulates the required setup.  This is required due to
/// the unusual architecture of the PBA which interprets OCI addresses and the
/// associated PowerBus transaction type based on which OCI \e master issued
/// the OCI transaction, not based on the OCI \a address (or PowerBus
/// address).  The final complication is that each OCI master is assigned a
/// unique PBA "slave port", so any reprogramming must be done on the PBA
/// registers associated with the particular port assigned to the particular
/// PORE engine.
///
/// In product code the PBA slave port assignment for the PORE-GPE engines
/// will be fixed; however this structure assumes the most general case and
/// allows for an arbitraray and dynamic assignment, and even allows mainstore
/// programs to run on the PORE-SLW. However the procedure that creates this
/// structure (gpe_pba_parms_create()) must know which engine/port will run
/// the program in order to set up the parameters.
///
/// The PBA software interface is not friendly for dynamic programming of the
/// PBA slave setup, especially from the PORE. The slave setup is modifed by a
/// read-modify-write under mask. gpa_pba_parms_create() does not allow
/// specification of which read buffers, prefetch modes or write timeouts to
/// use as these have global implications. Only the values that affect the
/// particular mode can be programmed there (but of course can be later
/// overridden if required).
///
/// Note that there is an assumption that PORE engines have exclusive access
/// to their PBA ports.  All GPE procedures that access the PowerBus follow a
/// protocol that makes no assumptions about how the PBA is set up - they set
/// up the PBA for their own use, then leave it to subsequent procedures to
/// re-setup the PBA as necessary for subsequent use. Also note that only one
/// GPE thread can be designated to run programs that access the PBA,
/// as they share an OCI master ID, and hence a PBA slave port.
///
/// Another - perhaps obvious - complication has to do with PBA slave reset. A
/// PORE engine executing from main memory can not modify the PBA slave read
/// parameter setup, as this would corrupt the instruction stream.  Currently
/// GPE procedures that need to modify the PBA read parameter setup execute
/// from SRAM, and the SLW executing from main memeory never changes its read
/// setup.
///
/// This structure is read-only to the GPE routines that access it.

typedef struct {

    /// The 32-bit OCI address of the PBA_SLVCTLn register to set up
    uint64_t slvctl_address;

    /// An image of the relevant parts of the PBA_SLVCTLn register in effect
    /// for this procedure
    pba_slvctln_t slvctl;

    /// The mask in effect for this update of the PBA_SLVCTL
    pba_slvctln_t mask;

    /// The value to write to the PBA_SLVRST register to reset the slave
    pba_slvrst_t slvrst;

    /// The bit to AND-poll to check for slave reset in progress
    pba_slvrst_t slvrst_in_progress;

    /// The slave id (0 - 3)
    uint64_t slave_id;

} GpePbaParms;

int
gpe_pba_parms_create(GpePbaParms *parms,
                     int slave,
                     int write_ttype,
                     int write_tsize,
                     int read_ttype);

#endif /* __ASSEMBLER__ */

// Parameter offset for GpePbaParms

#define GPEPBAPARMS_SLVCTL_ADDRESS     0x00
#define GPEPBAPARMS_SLVCTL             0x08
#define GPEPBAPARMS_MASK               0x10
#define GPEPBAPARMS_SLVRST             0x18
#define GPEPBAPARMS_SLVRST_IN_PROGRESS 0x20
#define GPEPBAPARMS_SLAVE_ID           0x28        

#define SIZEOF_GPEPBAPARMS 0x30


// Flags for gpe_pba_parms_setup()

#define GPE_PBA_PARMS_READ_INVALIDATE      0x01
#define GPE_PBA_PARMS_DISABLE_WRITE_GATHER 0x02

#endif  /* __GPE_H__ */
