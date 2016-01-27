#ifndef __CTYPE_H__
#define __CTYPE_H__

// $Id: ctype.h,v 1.1.1.1 2013/12/11 20:49:20 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/lib/ctype.h,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file ctype.h
/// \brief Replacement for <ctype.h>
///
/// The Gnu <ctype.h> requires some locale information to be defined.  We
/// avoid this overhead and implement the simple functions, simply assuming
/// standard 8-bit ASCII. The standard requires that these be defined as entry
/// points, but may be defined as macros, therefore all macros defined by
/// ctype.h are also be replicated in \c ctype.c

// The reference for which characters are included in each set was:
//
//     http://www.cplusplus.com/reference/clibrary/cctype/
//
// Note that no code was copied from the above or any other published
// description of the ctype.h functionality.

// To keep space to a minimum we encode the 8 common types directly into
// an 8-bit mask. Other types take a little longer to compute.

#define _CTYPE_CNTRL  0x01
#define _CTYPE_SPACE  0x02
#define _CTYPE_PRINT  0x04
#define _CTYPE_PUNCT  0x08
#define _CTYPE_UPPER  0x10
#define _CTYPE_LOWER  0x20
#define _CTYPE_DIGIT  0x40
#define _CTYPE_XDIGIT 0x80

#ifndef __ASSEMBLER__

#include "stdint.h"

/// \bug <ctype.h> can not include <stdio.h> to get the definition of
/// EOF. This is because it causes conflicts withe the pore_inline* code which
/// is portable (and ported) to Linux and PHYP. We need to go back through the
/// way the includes are done in SSX and this library and fix this. We should
/// have redefined <stdio.h> rather than creating a new "ssx_io.h"

#define _CTYPE_EOF -1

// Note that in all of the type macros, 'c' is an unsigned char.

extern const uint8_t _ctype[256];

#define _CTYPE_ISLOWER(c) (_ctype[c] & _CTYPE_LOWER)

#define _CTYPE_ISUPPER(c) (_ctype[c] & _CTYPE_UPPER)

#define _CTYPE_ISALPHA(c) (_CTYPE_ISUPPER(c) || _CTYPE_ISLOWER(c))

#define _CTYPE_ISDIGIT(c) (_ctype[c] & _CTYPE_DIGIT)

#define _CTYPE_ISALNUM(c) (_CTYPE_ISALPHA(c) || _CTYPE_ISDIGIT(c))

#define _CTYPE_ISXDIGIT(c) (_ctype[c] & _CTYPE_XDIGIT)

#define _CTYPE_ISCNTRL(c) (_ctype[c] & _CTYPE_CNTRL)

#define _CTYPE_ISSPACE(c) (_ctype[c] & _CTYPE_SPACE)

#define _CTYPE_ISPRINT(c) (_ctype[c] & _CTYPE_PRINT)

#define _CTYPE_ISGRAPH(c) (_CTYPE_ISPRINT(c) && ((c) != 0x20))

#define _CTYPE_ISPUNCT(c) (_ctype[c] & _CTYPE_PUNCT)

#define _CTYPE_TOUPPER(c) (islower(c) ? ((c) + ('A' - 'a')) : (c))

#define _CTYPE_TOLOWER(c) (isupper(c) ? ((c) - ('A' - 'a')) : (c))

// When #include'ed into ctype.c, the non-inline forms of the functions are
// created.  
//
// Note that the specification requires that 'c' "must have the value of an
// unsigned char or EOF". The specification also stipulates that "The values
// returned are non-zero if the character c falls into the tested class, and a
// zero value if not."

#define _CTYPE_PREDICATE(predicate, def)                \
    static inline int predicate(int c) {         \
        return ((c == _CTYPE_EOF) ?                     \
                0 : _CTYPE_##def((unsigned char)c));    \
    }

#define _CTYPE_FUNCTION(function, def)                          \
    static inline int function(int c) {                  \
        return ((c == _CTYPE_EOF) ?                             \
                _CTYPE_EOF : _CTYPE_##def((unsigned char)c));   \
    }

_CTYPE_PREDICATE(islower,  ISLOWER)
_CTYPE_PREDICATE(isupper,  ISUPPER)
_CTYPE_PREDICATE(isalpha,  ISALPHA)
_CTYPE_PREDICATE(isdigit,  ISDIGIT)
_CTYPE_PREDICATE(isalnum,  ISALNUM)
_CTYPE_PREDICATE(isxdigit, ISXDIGIT)
_CTYPE_PREDICATE(iscntrl,  ISCNTRL)
_CTYPE_PREDICATE(isspace,  ISSPACE)
_CTYPE_PREDICATE(isprint,  ISPRINT)
_CTYPE_PREDICATE(isgraph,  ISGRAPH)
_CTYPE_PREDICATE(ispunct,  ISPUNCT)

_CTYPE_FUNCTION(tolower, TOLOWER)
_CTYPE_FUNCTION(toupper, TOUPPER)

#endif // __ASSEMBLER__

#endif  /* __CTYPE_H__ */
