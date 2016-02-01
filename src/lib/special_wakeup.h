#ifndef __SPECIAL_WAKEUP_H__
#define __SPECIAL_WAKEUP_H__

// $Id: special_wakeup.h,v 1.3 2016/01/18 11:34:20 stillgs Exp $
// $Source: /archive/shadow/ekb/.cvsroot/eclipz/chips/p8/working/procedures/lib/special_wakeup.h,v $
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

// SW329314 - added for iVRM initialization handling
int
occ_special_wakeup_noclearcheck(int set,
                   ChipConfigCores cores,
                   int timeout_ms,
                   ChipConfigCores *o_timeouts);


#endif  /* __ASEMBLER__ */
#endif  /* __SPECIAL_WAKEUP_H__ */
