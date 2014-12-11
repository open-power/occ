#ifndef __STRTOX_H__
#define __STRTOX_H__

// $Id: strtox.h,v 1.2 2014/02/03 01:30:25 daviddu Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/lib/strtox.h,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file strtox.h 
/// \brief Underlying and extended APIs that support strtoX macros
///
/// See the Doxygen comments of the file strtox.c for descriptions of the
/// facilities provided by this header.

#ifndef __ASSEMBLER__

#include <limits.h>

// Error codes

#define STRTOX_NO_CONVERSION_EMPTY     0x00787901
#define STRTOX_NO_CONVERSION_PARSE     0x00787902
#define STRTOX_INVALID_ARGUMENT        0x00787903
#define STRTOX_INVALID_ARGUMENT_STRTOL 0x00787904
#define STRTOX_UNDERFLOW_STRTOL1       0x00787905
#define STRTOX_UNDERFLOW_STRTOL2       0x00787906
#define STRTOX_UNDERFLOW_STRTOLL1      0x00787907
#define STRTOX_UNDERFLOW_STRTOLL2      0x00787908
#define STRTOX_OVERFLOW_STRTOL1        0x00787909
#define STRTOX_OVERFLOW_STRTOL2        0x0078790a
#define STRTOX_OVERFLOW_STRTOLL1       0x0078790b
#define STRTOX_OVERFLOW_STRTOLL2       0x0078790c
#define STRTOX_OVERFLOW_STRTOUL        0x0078790d
#define STRTOX_OVERFLOW_STRTOULL       0x0078790e

// Earlier GCC configurations (ppcnf-mcp5-gcc) are not configured to define
// these standard constants, which exist in the include tree under various
// switches and configuration settings (from <limits.h>). They are defined by
// default in later standard cross builds however (GCC 4.5, 4.6). However we
// always assume that (long long) is a 64-bit type. It's likely that this is
// the only place these constant will be used (as they are defined as the
// values for under/overflow of strtoX() conversions), however it may be
// necessary in the future to move these #defines somewhere else.

#ifndef LLONG_MIN
# define LLONG_MIN (0x8000000000000000ll)
#endif

#ifndef LLONG_MAX
# define LLONG_MAX (0x7fffffffffffffffll)
#endif

#ifndef ULLONG_MAX
# define ULLONG_MAX (0xffffffffffffffffull)
#endif

int
_strtol(const char* str, char** endptr, int base, long* value);

int
_strtoul(const char* str, char** endptr, int base, unsigned long* value);

int
_strtoll(const char* str, char** endptr, int base, long long* value);

int
_strtoull(const char* str, char** endptr, int base, unsigned long long* value);


// The way the sizeof(long) is discovered by default depends on which version
// of gcc/cpp we're using as these macros are predefined by cpp.

#if (__SIZEOF_LONG__ == 4) || (__LONG_MAX__ == 2147483647L)

/// See documentation for the file strtox.c
static inline int
strtoi32(const char* str, char** endptr, int base, int32_t* value)
{
    long int value_l;
    int rc;

    rc = _strtol(str, endptr, base, &value_l);
    *value = value_l;
    return rc;
}

/// See documentation for the file strtox.c
static inline int
strtou32(const char* str, char** endptr, int base, uint32_t* value)
{
    unsigned long int value_ul;
    int rc;

    rc = _strtoul(str, endptr, base, &value_ul);
    *value = value_ul;
    return rc;
}

#else 

#error "No port of strtox.h yet for systems with sizeof(long) != 4"

#endif

// It is assumed that long long is always 64 bits; There is no standard macro
// for this size constant

/// See documentation for the file strtox.c
static inline int
strtoi64(const char* str, char** endptr, int base, int64_t* value)
{
    return _strtoll(str, endptr, base, value);
}

/// See documentation for the file strtox.c
static inline int
strtou64(const char* str, char** endptr, int base, uint64_t* value)
{
    return _strtoull(str, endptr, base, value);
}

#endif // __ASSEMBLER__

#endif // __STRTOX_H__
