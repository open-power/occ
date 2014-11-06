// $Id: gpe_pba.c,v 1.1.1.1 2013/12/11 20:49:20 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/lib/gpe_pba.c,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file gpe.c
/// \brief Generic PORE-GPE support procesures (outside of the kernel
/// drivers).

#include "ssx.h"
#include "gpe.h"
#include "gpe_pba.h"

/// Create/initialize a GpePbaParms structure
///
/// \param parms A unused or uninitialized GpePbaParms structure
///
/// \param slave The PBA slave port to program.  For production code this will
/// normally be PBA_SLAVE_PORE_GPE, however for test/verification code it
/// could be any PBA slave.
///
/// \param write_ttype One of PBA_WRITE_TTYPE_* (see pgp_pba.h).  Use
/// PBA_WRITE_TTYPE_DC (don't care) if the GPE application does not do writes.
///
/// \param write_tsize One of PBA_WRITE_TSIZE_* (see pgp_pba.h).  The \a
/// write_tsize is only relevant for \a write_ttype == PBA_WRITE_TTYPE_LCO_M,
/// where the macro PBA_WRITE_TSIZE_CHIPLET() is used to specify the target
/// chiplet, and \a write_ttype == PBA_WRITE_TTYPE_ATOMIC_RMW, where the \a
/// write_tsize specifies the atomic operation. Otherwise use
/// PBA_WRITE_TTYPE_DC (don't care).
///
/// \param read_ttype One of PBA_READ_TTYPE_* (see pgp_pba.h).  Normally this
/// will be PBA_READ_TTYPE_CL_READ_NC (or PBA_READ_TTYPE_DC if you don't
/// care.) 
///
/// \param flags Two flags are provided that override default
/// behavior. GPE_PBA_PARMS_READ_INVALIDATE specifies read buffer invalidation
/// after every read.  This is always selected when the read Ttype is a
/// cache-inhibited partial read, but may be optionally specified for test
/// purposes. 
///
/// Similarly, GPE_PBA_PARMS_DISABLE_WRITE_GATHER specifies that write
/// gathering for write Ttype DMA partial write is disabled. Note that
/// GPE_PBA_PARMS_DISABLE_WRITE_GATHER disables write gathering in the sense
/// that writes pass through the PBA immediately without being buffered. This
/// is different from using the PBA_WRITE_GATHER_TIMEOUT_DISABLE option to \a
/// write_gather_timeout, which specifies that writes are gathered until an
/// entire line is filled.
///
/// This API initializes the GpePbaParms structure used by every GPE program
/// that accesses mainstore via PBA. It creates an image of a PBA_SLVCTL
/// register to be applied under a mask.  
///
/// \note Read buffer invalidation is always enforced for cache-inhibited
/// partial reads. This also forces prefetching to be disabled for the
/// slave. Our procedures currently do not support save/restore of prefetch
/// controls as different tasks reprogram the PBA Slave.  Thus any access of a
/// shared slave that is also used to do CI_PR_RD will have prefetching
/// disabled. 
///
/// \retval 0 Success
///
/// \retval -GPE_INVALID_OBJECT The \a parms pointer is NULL (0) or othewise
/// invalid. 
///
/// \retval -GPE_INVALID_ARGUMENT One of the arguments is invalid in some way.

int
gpe_pba_parms_create(GpePbaParms *parms,
                     int slave,
                     int write_ttype,
                     int write_tsize,
                     int read_ttype)
{
    pba_slvctln_t *slvctl, *mask;
    pba_slvrst_t* slvrst;
    pba_slvrst_t* slvrst_in_progress;
    uint64_t all1 = 0xffffffffffffffffull;


    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF((parms == 0), GPE_INVALID_OBJECT);
        SSX_ERROR_IF((slave < 0) ||
                     (slave >= PBA_SLAVES),
                     GPE_INVALID_ARGUMENT);
    }

    parms->slave_id = slave;

    slvctl = &(parms->slvctl);
    mask = &(parms->mask);
    slvrst = &(parms->slvrst);
    slvrst_in_progress = &(parms->slvrst_in_progress);

    parms->slvctl_address = PBA_SLVCTLN(slave);

    slvrst->value = 0;
    slvrst->fields.set = PBA_SLVRST_SET(slave);

    slvrst_in_progress->value = 0;
    slvrst_in_progress->fields.in_prog = PBA_SLVRST_IN_PROG(slave);

    slvctl->value = 0;
    mask->value = 0;

    slvctl->fields.enable = 1;
    mask->fields.enable = all1;

    slvctl->fields.write_ttype = write_ttype;
    mask->fields.write_ttype = all1;
    
    slvctl->fields.write_tsize = write_tsize;
    mask->fields.write_tsize = all1;

    slvctl->fields.read_ttype = read_ttype;
    mask->fields.read_ttype = all1;

    if (read_ttype == PBA_READ_TTYPE_CI_PR_RD) {

        slvctl->fields.buf_invalidate_ctl = 1;
        mask->fields.buf_invalidate_ctl = all1;

        slvctl->fields.read_prefetch_ctl = PBA_READ_PREFETCH_NONE;
        mask->fields.read_prefetch_ctl = all1;

    } else {

        slvctl->fields.buf_invalidate_ctl = 0;
        mask->fields.buf_invalidate_ctl = all1;
    }

    mask->value = ~(mask->value);

    return 0;
}
        
    
    
          
    

    
                     
    
                     
