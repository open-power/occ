#ifndef __PGP_PORE_H__
#define __PGP_PORE_H__

// $Id: pgp_pore.h,v 1.1.1.1 2013/12/11 21:03:22 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/pgp/pgp_pore.h,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file pgp_pore.h
/// \brief PORE unit header.  Local and mechanically generated macros.

#include "pore_register_addresses.h"
#include "pore_firmware_registers.h"

/// The parameter for GPE-protocol triggering is the low-order word of the
/// EXE_TRIGGER register
#define PORE_EXE_PARAMETER_OFFSET (PORE_EXE_TRIGGER_OFFSET + 4)

/// The PORE OCI address space descriptor 
#define PORE_ADDRESS_SPACE_OCI 0x8000

/// The PORE BRAI opcode
#define PORE_BRAI 0xa2000000

/// The PORE BRAD D0 opcode
#define PORE_BRAD_D0 0x38500000


#ifndef __ASSEMBLER__

/// The putative type of PORE program entry points - to make GCC happy
typedef void *(PoreEntryPoint)(void);

#endif  /* __ASSEMBLER__ */

#endif  /* __PGP_PORE_H__ */
