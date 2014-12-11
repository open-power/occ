// $Id: ctype.c,v 1.1.1.1 2013/12/11 20:49:20 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/lib/ctype.c,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file ctype.c
/// \brief Replacement for <ctype.h> functions
///
/// This file contains entry point equivalents for the "ctype.h" macros.
/// These would only ever be used by assembler programs, therefore it's likely
/// that the object file will never be linked into an image.

#define __CTYPE_C__
#include "ctype.h"
#undef  __CTYPE_C__




