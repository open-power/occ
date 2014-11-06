// $Id: strtox.c,v 1.2 2014/02/03 01:30:25 daviddu Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/lib/strtox.c,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file strtox.c
/// \brief Implementation of strtol(), strtoul(), strtoll() and strtoull()
///
/// <b> Standard String Conversion Routines </b>
///
/// This file contains implementaions of strtol(), strtoul(), strtoll() and
/// strtoull(). These APIs are all called as 
///
/// \code
/// strtoX(const char* str, char** endptr, int base)
/// \endcode
///
/// where X is
///
/// - l : Convert to a long integer
/// - ul : Convert to an unsigned long integer
/// - ll : Convert to a long long integer
/// - ull : Convert to an unsigned long long integer
///
/// \param str The string to convert
///
/// \param endptr If non-null, will be set to a pointer to the portion of the
/// string following the convertable portion. If no conversion is performed
/// then the original \a str is returned here.
///
/// \param base Either 0 to indicate that the base should be derived from
/// radix markers in the string, or a number in the range 2 to 36 inclusive.
///
/// The APIs convert the initial portion of the string pointed to by \a str to
/// an integer, which is either a long integer (strtol), an unsigned long
/// (strtoul()), a long long (strtoll), or an unsigned long long
/// (strtoull). First, the APIs decompose the input string into three parts: 
/// 
/// - An initial, possibly empty, sequence of white-space characters (as
/// specified by isspace())
/// 
/// - A subject sequence interpreted as an integer represented in some radix
/// determined by the value of \a base
/// 
/// - A final string of one or more unrecognized characters, including the
/// terminating null byte of the input string.
/// 
/// The APIs then attempt to convert the subject sequence to an integer of the
/// required type and returns the result.
/// 
/// If the value of \a base is 0, the expected form of the subject sequence is
/// that of a decimal constant, octal constant, or hexadecimal constant, any
/// of which may be preceded by a '+' or '-' sign. A decimal constant begins
/// with a non-zero digit, and consists of a sequence of decimal digits. An
/// octal constant consists of the prefix '0' optionally followed by a
/// sequence of the digits '0' to '7' only. A hexadecimal constant consists of
/// the prefix 0x or 0X followed by a sequence of the decimal digits and
/// letters 'a' (or 'A' ) to 'f' (or 'F' ) with values 10 to 15 respectively.
/// 
/// If the value of \a base is between 2 and 36, the expected form of the
/// subject sequence is a sequence of letters and digits representing an
/// integer with the radix specified by base, optionally preceded by a '+' or
/// '-' sign. The letters from 'a' (or 'A' ) to 'z' (or 'Z' ) inclusive are
/// ascribed the values 10 to 35; only letters whose ascribed values are less
/// than that of base are permitted. If the value of base is 16, the
/// characters 0x or 0X may optionally precede the sequence of letters and
/// digits, following the sign if present.
/// 
/// The subject sequence is defined as the longest initial subsequence of the
/// input string, starting with the first non-white-space character that is of
/// the expected form. The subject sequence contains no characters if the
/// input string is empty or consists entirely of white-space characters, or if
/// the first non-white-space character is other than a sign or a permissible
/// letter or digit.
/// 
/// If the subject sequence has the expected form and the value of base is 0,
/// the sequence of characters starting with the first digit will be
/// interpreted as an integer constant. If the subject sequence has the
/// expected form and the value of base is between 2 and 36, it will be used
/// as the base for conversion, ascribing to each letter its value as given
/// above. If the subject sequence begins with a minus sign, the value
/// resulting from the conversion will be negated. A pointer to the final
/// string will be stored in the object pointed to by \a endptr, provided that
/// \a endptr is not a null pointer.
/// 
/// If the subject sequence is empty or does not have the expected form, no
/// conversion is performed; the value of \a str is stored in the object
/// pointed to by \a endptr, provided that \a endptr is not a null pointer.
///
/// Note that the unsigned APIs silently convert signed representations into
/// the equivalent unsigned number.
/// 
/// Since 0, (L)LONG_MIN and (U)(L)LONG_MAX are returned on error and are
/// also valid returns on success, there is no way for an SSX application to
/// determine whether the conversion succeeded or failed (since SSX does not
/// support \a errno). For this reason it is recommended that SSX-only
/// applications use the underlying APIs _strtol(), _strtoul(), _strtoll() and
/// _strtoull(), or even better the extended APIs strtoi32(), strtou32(),
/// strtoi64() or strtou64() discussed further below. 
/// 
/// Upon successful completion, strtoX() returns the converted
/// value, if any. If no conversion could be performed or there was an error
/// in the base specification, 0 is returned.
/// 
/// If the correct value is outside the range of representable values,
/// (L)LONG_MIN or (U)(L)LONG_MAX will be returned (according to the sign
/// and type of the value).
///
/// Note: This specification is adapted from IEEE Std. 10003.1, 2003 Edition
///
///
/// <b> Underlying APIs </b>
///
/// The APIs underlying the standard APIs are all called as
///
/// \code
/// int _strtoX(const char* str, char** endptr, int radix, <type>* value)
/// \endcode
///
/// where X is
///
/// - l : Convert to a long integer
/// - ul : Convert to an unsigned long integer
/// - ll : Convert to a long long integer
/// - ull : Convert to an unsigned long long integer
///
/// \param str The string to convert
///
/// \param endptr If non-null, will be set to a pointer to the portion of the
/// string following the convertable portion. If no conversion is performed
/// then the original \a str is returned here.
///
/// \param base Either 0 to indicate that the base should be derived from
/// radix markers in the string, or a number in the range 2 to 36 inclusive.
///
/// \param value The converted value, returned as the return value of the
/// standard API.
///
/// The return value of the underlying APIs is one of the following
///
/// \retval 0 Success
///
/// \retval -STRTOX_NO_CONVERSION_EMPTY No conversion was performed because the
/// string was effectively empty.
///
/// \retval -STRTOX_NO_CONVERSION_PARSE No conversion was performed because the
/// string did not parse as an integer.
///
/// \retval -STRTOX_INVALID_ARGUMENT No conversion was performed because the
/// \a base specification was not valid.
///
/// \retval -STRTOX_UNDERFLOW_STRTOL1  Conversion resulted in underflow
///
/// \retval -STRTOX_UNDERFLOW_STRTOL2  Conversion resulted in underflow
///
/// \retval -STRTOX_UNDERFLOW_STRTOLL1 Conversion resulted in underflow
///
/// \retval -STRTOX_UNDERFLOW_STRTOLL2 Conversion resulted in underflow
///
/// \retval -STRTOX_OVERFLOW_STRTOL1   Conversion resulted in overflow
///
/// \retval -STRTOX_OVERFLOW_STRTOL2   Conversion resulted in overflow
///
/// \retval -STRTOX_OVERFLOW_STRTOLL1  Conversion resulted in overflow
///
/// \retval -STRTOX_OVERFLOW_STRTOLL2  Conversion resulted in overflow
///
///
/// <b> Extended APIs </b>
///
/// The extended APIs are the preferred way to do portable integer
/// conversion. These APIs are all called as
///
/// \code
/// int strtoX(const char* str, char** endptr, int radix, <type>* value)
/// \endcode
///
/// where X is
///
/// - i32 : Convert to an int32_t
/// - u32 : Convert to a uint32_t
/// - i64 : Convert to an int64_t
/// - u64 : Convert to a uint64_t
///
/// \param str The string to convert
///
/// \param endptr If non-null, will be set to a pointer to the portion of the
/// string following the convertable portion. If no conversion is performed
/// then the original \a str is returned here.
///
/// \param base Either 0 to indicate that the base should be derived from
/// radix markers in the string, or a number in the range 2 to 36 inclusive.
///
/// \param value The converted value
///
/// The return value of the underlying APIs is one of the following
///
/// \retval 0 Success
///
/// \retval -STRTOX_NO_CONVERSION_EMPTY No conversion was performed because the
/// string was effectively empty.
///
/// \retval -STRTOX_NO_CONVERSION_PARSE No conversion was performed because the
/// string did not parse as an integer.
///
/// \retval -STRTOX_INVALID_ARGUMENT No conversion was performed because the
/// \a base specification was not valid.
///
/// \retval -STRTOX_UNDERFLOW_STRTOL1  Conversion resulted in underflow
///
/// \retval -STRTOX_UNDERFLOW_STRTOL2  Conversion resulted in underflow
///
/// \retval -STRTOX_UNDERFLOW_STRTOLL1 Conversion resulted in underflow
///
/// \retval -STRTOX_UNDERFLOW_STRTOLL2 Conversion resulted in underflow
///
/// \retval -STRTOX_OVERFLOW_STRTOL1   Conversion resulted in overflow
///
/// \retval -STRTOX_OVERFLOW_STRTOL2   Conversion resulted in overflow
///
/// \retval -STRTOX_OVERFLOW_STRTOLL1  Conversion resulted in overflow
///
/// \retval -STRTOX_OVERFLOW_STRTOLL2  Conversion resulted in overflow
///

#include "ssx.h"
#include "ctype.h"
#include "libssx.h"
#include "strtox.h"


// Skip whitespace

static const char *
skip_whitespace(const char *s)
{
    while (isspace(*s)) {
        s++;
    }
    return s;
}

// Pick up a +/- sign.  This is a predicate returning 1 if the value is
// negated. 

static int
sign(const char** s)
{
    if (**s == '+') {
        (*s)++;
        return 0;
    } else if (**s == '-') {
        (*s)++;
        return 1;
    } else {
        return 0;
    }
}
        

// Look for a radix mark (0, 0[xX]). The string pointer is advanced if it is a
// hex mark (0[xX]), but not for a simple '0' which could be either the start
// of an octal constant or simply the number 0. The return value is either 8,
// 10 or 16.

static int
radix_mark(const char** s) 
{
    const char* p = *s;

    if (p[0] == '0') {
        if ((p[1] == 'x') || (p[1] == 'X')) {
            *s += 2;
            return 16;
        } else {
            return 8;
        }
    } else {
        return 10;
    }
}


// Parse a character as a radix-base digit.  Return the value of the digit or
// -1 if it is not a legal digit for the radix.

static int
parse_digit(char c, int radix)
{
    if (isdigit(c)) {
        if ((c - '0') < radix) {
            return c - '0';
        } else {
            return -1;
        }
    } else if (radix <= 10) {
        return -1;
    } else {
        if (islower(c)) {
            if ((c - 'a') < (radix - 10)) {
                return c - 'a' + 10;
            } else {
                return -1;
            }
        } else if (isupper(c)) {
            if ((c - 'A') < (radix - 10)) {
                return c - 'A' + 10;
            } else {
                return -1;
            }
        } else {
            return -1;
        }
    }
}


// The most basic API is strtox(), which converts a string to an unsigned long
// long. All of the base APIs are written in terms of this.  This is legal due
// to the fact that conversion is defined to continue even in the event of
// overflow. This API may return the codes STRTOX_NO_CONVERSION_EMPTY, 
// STRTOX_NO_CONVERSION_PARSE or STRTOX_INVALID_ARGUMENT, 
// which the standard APIs always convert to a 0
// return value.  Otherwise the flags 'overflow' and 'negative' are used by
// the base APIs to determine how to handle special cases.

static int
strtox(const char *str, char **endptr, int base, 
        unsigned long long* value, int* negative, int* overflow)
{
    const char* s;
    unsigned long long new;
    int rc, radix, digit;


    do {

        s = str;
        *value = 0;
        *negative = 0;
        *overflow = 0;

        // Initial error checks

        if ((base != 0) && ((base < 2) || (base > 36))) {
            rc = STRTOX_INVALID_ARGUMENT;
            break;
        }

        // Skip whitespace

        s = skip_whitespace(s);
        if (*s == '\0') {
            rc = STRTOX_NO_CONVERSION_EMPTY;
            break;
        }

        // Process a +/- sign.  Only one is allowed.

        *negative = sign(&s);

        // Look for a radix mark.  Note that if base == 16 this will cause the
        // skip of a leading 0 in the string not followed by [xX], but that's
        // OK because it doesn't change the result of the conversion.

        if (base == 0) {
            radix = radix_mark(&s);
        } else {
            radix = base;
            if (radix == 16) {
                radix_mark(&s);
            }
        }

        // Parse.  Note that once overflow is detected we continue to parse
        // (but ignore the data).

        rc = STRTOX_NO_CONVERSION_PARSE;

        while ((digit = parse_digit(*s, radix)) >= 0) {
            s++;
            if (!*overflow) {
                rc = 0;
                new = (*value * radix) + digit;
                if (new < *value) {
                    *overflow = 1;
                } else {
                    *value = new;
                }
            }
        }
    } while(0);

    if (endptr) {
        if (rc == 0) {
            *endptr = (char*)s;
        } else {
            *endptr = (char*)str;
        }
    }

    return rc;
}
     

/// See documentation for the file strtox.c
int
_strtol(const char* str, char** endptr, int base, long* value)
{
    int rc, negative, overflow;
    unsigned long long value_ull;

    rc = strtox(str, endptr, base, &value_ull, &negative, &overflow);
    if (rc) {
        *value = 0;
    } else {
        if (overflow || (value_ull != (unsigned long)value_ull)) {
            if (negative) {
                rc = STRTOX_UNDERFLOW_STRTOL1;
                *value = LONG_MIN;
            } else {
                rc = STRTOX_OVERFLOW_STRTOL1;
                *value = LONG_MAX;
            }
        } else if (negative) {
            if (value_ull > ((unsigned long long)LONG_MAX + 1ull)) {
                rc = STRTOX_UNDERFLOW_STRTOL2;
                *value = LONG_MIN;
            } else {
                *value = ~value_ull + 1;
            }
        } else if (value_ull > (unsigned long long)LONG_MAX) {
            rc = STRTOX_OVERFLOW_STRTOL2;
            *value = LONG_MAX;
        } else {
            *value = value_ull;
        }
    }
    return rc;
}


/// See documentation for the file strtox.c
int
_strtoll(const char* str, char** endptr, int base, long long* value)
{
    int rc, negative, overflow;
    unsigned long long value_ull;

    rc = strtox(str, endptr, base, &value_ull, &negative, &overflow);
    if (rc) {
        *value = 0;
    } else {
        if (overflow) {
            if (negative) {
                rc = STRTOX_UNDERFLOW_STRTOLL1;
                *value = LLONG_MIN;
            } else {
                rc = STRTOX_OVERFLOW_STRTOLL1;
                *value = LLONG_MAX;
            }
        } else if (negative) {
            if (value_ull > ((unsigned long long)LLONG_MAX + 1ull)) {
                rc = STRTOX_UNDERFLOW_STRTOLL2;
                *value = LLONG_MIN;
            } else {
                *value = ~value_ull + 1;
            }
        } else if (value_ull > (unsigned long long)LLONG_MAX) {
            rc = STRTOX_OVERFLOW_STRTOLL2;
            *value = LLONG_MAX;
        } else {
            *value = value_ull;
        }
    }
    return rc;
}


/// See documentation for the file strtox.c
int
_strtoul(const char* str, char** endptr, int base, unsigned long* value)
{
    int rc, negative, overflow;
    unsigned long long value_ull;

    rc = strtox(str, endptr, base, &value_ull, &negative, &overflow);
    if (rc) {
        *value = 0;
    } else {
        if (overflow || (value_ull != (unsigned long)value_ull)) {
            rc = STRTOX_OVERFLOW_STRTOUL;
            *value = ULONG_MAX;
        } else {
            *value = value_ull;
            if (negative) {
                *value = ~*value + 1;
            }
        }
    }
    return rc;
}

/// See documentation for the file strtox.c
int
_strtoull(const char* str, char** endptr, int base, unsigned long long* value)
{
    int rc, negative, overflow;

    rc = strtox(str, endptr, base, value, &negative, &overflow);
    if (rc) {
        *value = 0;
    } else {
        if (overflow) {
            rc = STRTOX_OVERFLOW_STRTOULL;
            *value = ULLONG_MAX;
        } else {
            if (negative) {
                *value = ~*value + 1;
            }
        }
    }
    return rc;
}


/// See documentation for the file strtox.c
long int
strtol(const char* str, char** endptr, int base)
{
    long int value;
    
    _strtol(str, endptr, base, &value);
    return value;
}


/// See documentation for the file strtox.c
long long int
strtoll(const char* str, char** endptr, int base)
{
    long long int value;
    
    _strtoll(str, endptr, base, &value);
    return value;
}


/// See documentation for the file strtox.c
unsigned long int
strtoul(const char* str, char** endptr, int base)
{
    unsigned long int value;
    
    _strtoul(str, endptr, base, &value);
    return value;
}


/// See documentation for the file strtox.c
unsigned long long int
strtoull(const char* str, char** endptr, int base)
{
    unsigned long long int value;
    
    _strtoull(str, endptr, base, &value);
    return value;
}


#if (__GNUC__ < 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ <= 1))

/// Internal version of strtol()
///
/// ppcnf-mcp5 (GCC 4.1) requires that the entry point __strtol_internal() be
/// present at certain optimization levels. This is equivalent to strtol()
/// except that it takes an extra argument that must be == 0. The \a group
/// parameter is supposed to control locale-specific thousands grouping.

long int
__strtol_internal(const char* str, char** endptr, int base, int group)
{
    if (group != 0) {
        SSX_PANIC(STRTOX_INVALID_ARGUMENT_STRTOL);
    }
    return strtol(str, endptr, base);
}

#endif

