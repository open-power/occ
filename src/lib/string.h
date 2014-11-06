#ifndef __STRING_H__
#define __STRING_H__

// $Id: string.h,v 1.1.1.1 2013/12/11 20:49:20 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/lib/string.h,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file string.h
/// \brief Replacement for <string.h>
///
/// The SSX library does not implement the entire <string.h> function.
/// However the real reason for this header was the finding that under certain
/// optimization modes, we were geting errors from the default <string.h>
/// supplied with the MPC environment.  So we created this replacement that
/// only calls out what is implemented, exactly as it is implemented for SSX.

#ifndef __ASSEMBLER__

#include <stddef.h>

// APIs inmplemented by string.c

size_t 
strlen(const char *s);

int
strcmp(const char* s1, const char* s2);

int 
strncmp(const char* s1, const char* s2, size_t n);

int
strcasecmp(const char* s1, const char* s2);

int 
strncasecmp(const char* s1, const char* s2, size_t n);

char *
strcpy(char *dest, const char *src);

char *
strncpy(char *dest, const char *src, size_t n);

void *
memcpy(void *dest, const void *src, size_t n);

void *
memset(void *s, int c, size_t n);

int
memcmp(const void* s1, const void* s2, size_t n);

// APIs implemented by strdup.c

char *
strdup(const char* s);

#endif  /* __ASSEMBLER__ */

#endif  /* __STRING_H__ */
