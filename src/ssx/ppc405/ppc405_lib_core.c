// $Id: ppc405_lib_core.c,v 1.2 2014/06/26 13:00:11 cmolsen Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/ppc405/ppc405_lib_core.c,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file ppc405_lib_core.c
/// \brief PPC405-specific library procedures 
///
/// The routines in this file are not part of SSX per se, but are included
/// with SSX because they may be considered OS services.
/// 
/// These are core routines that will present in all PPC405 SSX applications.

#include "ssx.h"

/// Cause a PPC405 core reset by an action on DBCR0

void
ppc405_core_reset()
{
    or_spr(SPRN_DBCR0, DBCR0_RST_CORE);
}

/// Cause a PPC405 chip reset by an action on DBCR0

void
ppc405_chip_reset()
{
    or_spr(SPRN_DBCR0, DBCR0_RST_CHIP);
}

/// Cause a PPC405 system reset by an action on DBCR0

void
ppc405_system_reset()
{
    or_spr(SPRN_DBCR0, DBCR0_RST_SYSTEM);
}

