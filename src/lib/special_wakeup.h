#ifndef __SPECIAL_WAKEUP_H__
#define __SPECIAL_WAKEUP_H__

// $Id: special_wakeup.h,v 1.2 2014/02/03 01:30:25 daviddu Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/lib/special_wakeup.h,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file special_wakeup.h
/// \brief Container for special wakeup related procedures

#ifndef __ASSEMBLER__

#include <stdint.h>
#include "pgp_config.h"
#include "ssx.h"

#define SPWU_INVALID_ARGUMENT 0x00779801

extern uint32_t G_special_wakeup_count[PGP_NCORES];

int
occ_special_wakeup(int set,
                   ChipConfigCores cores,
                   int timeout_ms,
                   ChipConfigCores *o_timeouts);

#endif  /* __ASEMBLER__ */
#endif  /* __SPECIAL_WAKEUP_H__ */
