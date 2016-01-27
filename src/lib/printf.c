// $Id: printf.c,v 1.1.1.1 2013/12/11 20:49:20 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/lib/printf.c,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file printf.c
/// \brief Clean-room implementation of printf() functions for SSX I/O
///
/// For licensing reasons we are required to create our own version of the
/// printf() family of functions.  This implementation was created without
/// reference to or inclusion of any licensed or copyrighted code.
///
/// The functions defined in this file have prototypes, behavior and return
/// values as defined by C language standards.  In the event of an error a
/// negative value is returned, generally corresponding to a standard Unix
/// 'errno' code. Note that SSX does not support either an application- or
/// per-thread 'errno', so the only record of any error is the \a error field
/// of the stream. Also note that SSX may be configured to cause a panic if an
/// error is detected rather than returning an error code.
///
/// This implementation defines a limited but useful subset of the C standard
/// for format control. This implementation includes the following:
///
/// - \b c, \b d, \b i, \b n, \b p, \b s, \b u, \b x, and \b X conversion
/// specifiers, as well as '%%' to output a single '%'
///
/// - \b #, \b 0, \b ' ' and \b + flag characters
///
/// - Decimal field width specifiers including * (but indirect field widths
/// must be positive as left-justification is not supported)
///
/// - Decimal precision specifiers (currently only apply to %s formats, may be
/// indirect using *)
///
/// - \b l, \b ll and \b z length modifiers
///
/// \b Notes:
///
/// \a If a \c p conversion specifier is used without any flags (\c '%p'), the
/// \c p conversion is interptered as if it were \c 0x%08lx for 32-bit address
/// machines and \c 0x%016llx for 64-bit address machines. The GCC builtin
/// format checker gives warnings about '0' flag characters for \c p
/// conversion specifiers, so there is otherwise no 'un-warned' way to get
/// this preferred (by some) format of pointer values. If you do include
/// explicit flags (e.g., \c %30p) they will be processed as expected.
///
/// Similar to how printf() behaves on an X86-Linux machine, a null pointer
/// will print as "(null)" with the %s format (unless the precision specifier
/// precludes it) and "(nil)" with the %p format.
///
/// Note that calling formatted I/O functions on non-blocking streams may fail
/// with the -EAGAIN error, and there is no clean way to restart these
/// calls. Calling formatted (or any) I/O functions on blocking streams from
/// interrupt contexts in SSX is also likely to fail intermittently since
/// interrupt contexts can not block in SSX.
///
/// \todo I'd really like to implement the '-' flag for
/// left-justification. Implementing the precision specifer for integers
/// should be done for completeness.

#include "ssx.h"
#include "ctype.h"
#include "ssx_io.h"

// Formatting options

#define OPTION_ALTERNATE   0x0001
#define OPTION_PAD_ZERO    0x0002
#define OPTION_PLUS_SIGN   0x0004
#define OPTION_FIELD_WIDTH 0x0008
#define OPTION_PRECISION   0x0010
#define OPTION_LONG        0x0020
#define OPTION_LONG_LONG   0x0040
#define OPTION_SIZE_T      0x0080
#define OPTION_UPPERCASE   0x0100
#define OPTION_HEX         0x0200
#define OPTION_SPACE       0x0400


// Generate padding if required, returning the total number of pad characters
// output or a negative error code. The 'nchars' argument is the number of
// non-pad characters to be output by the caller.

#define PAD_SIZE 8
static const char zeros[PAD_SIZE]  = {'0', '0', '0', '0', '0', '0', '0', '0'};
static const char blanks[PAD_SIZE] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};

static ssize_t
pad(FILE *stream, size_t nchars, int options, size_t width)
{
    const char *padchars;
    size_t chars, written;
    int rc;

    if (!(options & OPTION_FIELD_WIDTH) || (nchars >= width)) {
        return 0;
    }
    chars = width - nchars;
    if (options & OPTION_PAD_ZERO) {
        padchars = zeros;
    } else {
        padchars = blanks;
    }
    while (chars) {
        rc = swrite(stream, (void *)padchars, MIN(chars, PAD_SIZE), &written);
        if (rc < 0) return rc;
        chars -= written;
    }
    return width - nchars;
}


// Format a character

static ssize_t
format_char(FILE *stream, unsigned char c, int options, size_t width)
{
    ssize_t padchars, nchars;
    int rc;

    padchars = pad(stream, 1, options, width);
    if (padchars < 0) return padchars;
    nchars = padchars + 1;
    rc = swrite(stream, (void *)(&c), 1, 0);
    if (rc < 0) return rc;
    return nchars;
}


// Format a string
//
// If the string is the NULL pointer then normally "(null)" is printed
// unless the precision is < 6, in which case the empty string is printed.
// The specification leaves it as undefined what happens if a string requests
// 0 padding; Here we always pad with blanks (although GCC/PowerPC catches
// this as an error).

static ssize_t
format_string(FILE *stream, const char *s, int options, 
              size_t width, size_t precision)
{
    size_t len;
    ssize_t padchars, nchars;
    int rc;

    if (s == 0) {
        if ((options & OPTION_PRECISION) && (precision < 6)) {
            s = "";
        } else {
            s = "(null)";
        }
    }

    len = strlen(s);
    if (options & OPTION_PRECISION) {
        len = MIN(len, precision);
    }

    options &= ~OPTION_PAD_ZERO;
    padchars = pad(stream, len, options, width);
    if (padchars < 0) return padchars;
    nchars = padchars + len;
    rc = swrite(stream, (void *)s, len, 0);
    if (rc < 0) return rc;
    return nchars;
}


// Format an integer - signed and unsigned.  A 64-bit integer (assumed to be
// the longest we'll see) has 20 decimal digits.  An extra space is reserved
// for the sign.  If zero-padding is specified, the sign will be output
// separately.

static const char lower[16] = {
  '0', '1', '2', '3', '4', '5', '6', '7', 
  '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

static const char upper[16] = {
  '0', '1', '2', '3', '4', '5', '6', '7', 
  '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

static ssize_t
format_int(FILE *stream, long long int lli, int options, size_t width)
{
    char digits[21];
    int rc, k, ndigits, negative, positive, i; 
    ssize_t output;

    negative = (lli < 0);
    positive = (lli > 0);

    // Unpack the integer to characters.  The code is optimized for 32-bit
    // machines where 64-bit division is not built in.  The first part of the
    // loop handles integers requiring a 64-bit divide, the second loop
    // handles 32-bit integers.

    if (lli == 0) {
        digits[20] = '0';
        k = 20;
    } else if (negative) {
        for (k = 21;
             lli != (int)lli;
             digits[--k] = lower[-(lli % 10)], lli = lli / 10);
        for (i = (int)lli;
             i != 0;
             digits[--k] = lower[-(i % 10)], i = i / 10);
    } else {
        for (k = 21;
             lli != (int)lli;
             digits[--k] = lower[lli % 10], lli = lli / 10);
        for (i = (int)lli;
             i != 0;
             digits[--k] = lower[i % 10], i = i / 10);
    }

    ndigits = 21 - k;

    // Handle other options and output

    output = 0;
    if (options & OPTION_PAD_ZERO) {

        if (negative) {
            rc = swrite(stream, "-", 1, 0);
            if (rc < 0) return rc;
            output++;
        } else if (positive) {
            if (options & OPTION_PLUS_SIGN) {
                rc = swrite(stream, "+", 1, 0);
                if (rc < 0) return rc;
                output++;
            } else if (options & OPTION_SPACE) {
                rc = swrite(stream, " ", 1, 0);
                if (rc < 0) return rc;
                output++;
            }
        }
        rc = pad(stream, ndigits + output, options, width);
        if (rc < 0) return rc;
        output += rc;
        rc = swrite(stream, &(digits[k]), ndigits, 0);
        if (rc < 0) return rc;
        output += ndigits;

    } else {

        if (negative) {
            digits[--k] = '-';
            ndigits++;
        } else if (positive) {
            if (options & OPTION_PLUS_SIGN) {
                digits[--k] = '+';
                ndigits++;
            } else if (options & OPTION_SPACE) {
                digits[--k] = ' ';
                ndigits++;
            }
        }
        rc = pad(stream, ndigits, options, width);
        if (rc < 0) return rc;
        output += rc;
        rc = swrite(stream, &(digits[k]), ndigits, 0);
        if (rc < 0) return rc;
        output += ndigits;
    }

    return output;
}

        
static ssize_t
format_unsigned(FILE *stream, unsigned long long ull, int options, size_t width)
{
    char digits[21], *alternate;
    const char *xchars;
    int rc, k, ndigits, zero;
    unsigned u;
    ssize_t output;

    zero = (ull == 0);

    // Determine hex case and alternate string

    alternate = 0;
    if (options & OPTION_HEX) {
        if (options & OPTION_UPPERCASE) {
            xchars = upper;
            if (options & OPTION_ALTERNATE) {
                alternate = "0X";
            }
        } else {
            xchars = lower;
            if (options & OPTION_ALTERNATE) {
                alternate = "0x";
            }
        }
    } else {
        xchars = lower;
    }

    // Unpack the unsigned integer to characters.  The Hex conversions are
    // easier since they can be done with shift and mask rather than
    // divison. The code is optimized for a 32-bit machine where 64-bit
    // division is not built-in. 

    if (zero) {
        digits[20] = '0';
        k = 20;
    } else if (options & OPTION_HEX) {
        for (k = 21;
             ull != (unsigned)ull;
             digits[--k] = xchars[ull & 0xf], ull = ull >> 4);
        for (u = (unsigned)ull;
             u != 0;
             digits[--k] = xchars[u & 0xf], u = u >> 4);
    } else {
        for (k = 21;
             ull != (unsigned)ull;
             digits[--k] = xchars[ull % 10], ull = ull / 10);
        for (u = (unsigned)ull;
             u != 0;
             digits[--k] = xchars[u % 10], u = u / 10);
    }

    ndigits = 21 - k;

    // Handle other options and output

    output = 0;
    if (options & OPTION_PAD_ZERO) {

        if (!zero && alternate) {
            rc = swrite(stream, (void *)alternate, 2, 0);
            if (rc < 0) return rc;
            output += 2;
        }
        rc = pad(stream, ndigits + output, options, width);
        if (rc < 0) return rc;
        output += rc;
        rc = swrite(stream, &(digits[k]), ndigits, 0);
        if (rc < 0) return rc;
        output += ndigits;

    } else {

        if (!zero && alternate) {
            output += 2;
        }
        rc = pad(stream, ndigits + output, options, width);
        if (rc < 0) return rc;
        output += rc;
        if (!zero && alternate) {
            rc = swrite(stream, alternate, 2, 0);
            if (rc < 0) return rc;
            output += 2;
        }
        rc = swrite(stream, &(digits[k]), ndigits, 0);
        if (rc < 0) return rc;
        output += ndigits;
    }

    return output;
}

        
int
vfprintf(FILE *stream, const char *format, va_list argp)
{
    const char *fmt, *scan;
    int rc, total_chars, options, done;
    size_t width, precision;

    int arg_i, *arg_pi;
    long int arg_li;
    long long int  arg_lli;
    ssize_t arg_zi;
    unsigned arg_u;
    unsigned long arg_lu;
    unsigned long long arg_llu;
    size_t arg_zu;
    char *arg_s;

    total_chars = 0;
    
    fmt = format;
    while (*fmt) {

        // Scan until '%' or the end of the format, then output the text.

        scan = fmt;
        while (*scan && (*scan != '%')) {
            scan++;
        }
        if (scan != fmt) {
            rc = swrite(stream, fmt, scan - fmt, 0);
            if (rc < 0) return rc;
            total_chars += scan - fmt;
        }
        fmt = scan;
        if (!*fmt) {
            return total_chars;
        }
        fmt++;

        // We got a '%'. Check for %% and %n.

        switch (*fmt) {
        case '\0':
            SSX_IO_ERROR(stream, EINVAL);
            break;
        case '%':
            rc = swrite(stream, "%", 1, 0);
            if (rc < 0) return rc;
            total_chars++;
            fmt++;
            continue;
        case 'n':
            arg_pi = va_arg(argp, int *);
            *arg_pi = total_chars;
            fmt++;
            continue;
        }

        // Collect padding options, if any.  Left justification is not
        // implemeted. 

        options = 0;
        done = 0;
        do {
            switch (*fmt) {
            case '\0':
                SSX_IO_ERROR(stream, EINVAL);
                break;
            case '#':
                options |= OPTION_ALTERNATE;
                break;
            case '0':
                options |= OPTION_PAD_ZERO;
                break;
            case '+':
                options |= OPTION_PLUS_SIGN;
                break;
            case ' ':
                options |= OPTION_SPACE;
                break;
            case '-':
                SSX_IO_ERROR(stream, EINVAL); // Left just. not impl.
                break;
            default:
                done = 1;
                break;
            }
            if (!done) {
                fmt++;
            }
        } while (!done);

        // Collect the field width, if specified. A negative precision
        // specified as an argument indicates left justification (not
        // implemented). 

        width = 0; 
        if (isdigit(*fmt)) {       
            options |= OPTION_FIELD_WIDTH;
            for (; isdigit(*fmt); fmt++) {
                width = (width * 10) + (*fmt - '0');
            }
        } else if (*fmt == '*') {
            fmt++;
            options |= OPTION_FIELD_WIDTH;
            arg_i = va_arg(argp, int);
            if (arg_i < 0) {
                SSX_IO_ERROR(stream, EINVAL); // Left just. not impl.
            }
            width = arg_i;
        }

        // Collect the precision, if specified.  By standard specification an
        // empty or negative precision is interpreted as 0.

        precision = 0;
        if (*fmt == '.') {
            fmt++;
            options |= OPTION_PRECISION;
            if (isdigit(*fmt)) {
                for(; isdigit(*fmt); fmt++) {
                    precision = (precision * 10) + (*fmt - '0');
                } 
            } else if (*fmt == '*') {
                fmt++;
                arg_i = va_arg(argp, int);                    
                if (arg_i < 0) {
                    arg_i = 0;
                }
                precision = arg_i;
            }
        }

        // Collect length modifiers. 

        done = 0;
        do {
            switch (*fmt) {
            case '\0':
                SSX_IO_ERROR(stream, EINVAL);
                break;
            case 'l':
                if (options & OPTION_LONG) {
                    options &= ~OPTION_LONG;
                    options |= OPTION_LONG_LONG;
                } else if (options & OPTION_LONG_LONG) {
                    SSX_IO_ERROR(stream, EINVAL);
                } else {
                    options |= OPTION_LONG;
                }
                if (options & OPTION_SIZE_T) {
                    SSX_IO_ERROR(stream, EINVAL);
                }
                break;
            case 'z':
                if ((options & OPTION_LONG) || (options & OPTION_LONG_LONG)) {
                    SSX_IO_ERROR(stream, EINVAL);
                }
                options |= OPTION_SIZE_T;
                break;
            default:
                done = 1;
                break;
            }
            if (!done) {
                fmt++;
            }
        } while (!done);

        // Use the conversion specifier to format the next argument

        switch (*fmt) {

        case 'c':
            arg_i = va_arg(argp, int);
            rc = format_char(stream, (unsigned char)arg_i, options, width);
            if (rc < 0) return rc;
            total_chars++;
            break;
                
        case 'd':
        case 'i':
            if (options & OPTION_LONG) {
                arg_li = va_arg(argp, long int);
                rc = format_int(stream, (long long int)arg_li, options, 
                                width);
            } else if (options & OPTION_LONG_LONG) {
                arg_lli = va_arg(argp, long long int);
                rc = format_int(stream, (long long int)arg_lli, options, 
                                width);
            } else if (options & OPTION_SIZE_T) {
                arg_zi = va_arg(argp, ssize_t);
                rc = format_int(stream, (long long int)arg_zi, options, 
                                width);
            } else {
                arg_i = va_arg(argp, int);
                rc = format_int(stream, (long long int)arg_i, options, 
                                width);
            }
            if (rc < 0) return rc;
            total_chars += rc;
            break;

        case 'p':
            arg_lu = va_arg(argp, unsigned long);
            options |= (OPTION_ALTERNATE | OPTION_HEX);
            if (!(options & OPTION_PAD_ZERO) && 
                !(options & OPTION_FIELD_WIDTH)) {
                options |= (OPTION_PAD_ZERO | OPTION_FIELD_WIDTH);
                width = (2 * sizeof(unsigned long)) + 2; /* 0x........ */
            }
            if (arg_lu == 0) {
                options &= ~OPTION_PRECISION;
                rc = format_string(stream, "(nil)", options, width, precision);
            } else {
                rc = format_unsigned(stream, (unsigned long long)arg_lu, 
                                     options, width);
            }
            if (rc < 0) return rc;
            total_chars += rc;
            break;

        case 's':
            arg_s = va_arg(argp, char *);
            rc = format_string(stream, arg_s, options, width, precision);
            if (rc < 0) return rc;
            total_chars += rc;
            break;

        case 'X':
            options |= OPTION_UPPERCASE;
        case 'x':
            options |= OPTION_HEX;
        case 'u':
            if (options & OPTION_LONG) {
                arg_lu = va_arg(argp, unsigned long);
                rc = format_unsigned(stream, (unsigned long long)arg_lu, 
                                     options, width);
            } else if (options & OPTION_LONG_LONG) {
                arg_llu = va_arg(argp, unsigned long long);
                rc = format_unsigned(stream, (unsigned long long)arg_llu, 
                                     options, width);
            } else if (options & OPTION_SIZE_T) {
                arg_zu = va_arg(argp, size_t);
                rc = format_unsigned(stream, (unsigned long long)arg_zu, 
                                     options, width);
            } else {
                arg_u = va_arg(argp, unsigned);
                rc = format_unsigned(stream, (unsigned long long )arg_u, 
                                     options, width);
            }
            if (rc < 0) return rc;
            total_chars += rc;
            break;
            
        default:
            SSX_IO_ERROR(stream, EINVAL);
            break;
        }

        fmt++;
    }

    return total_chars;
}


int
vprintf(const char *format, va_list argp)
{
    return vfprintf(stdout, format, argp);
}


int
fprintf(FILE *stream, const char *format, ...)
{
    va_list argp;
    int rc;

    va_start(argp, format);
    rc = vfprintf(stream, format, argp);
    va_end(argp);
    return rc;
}


int
printf(const char *format, ...)
{
    va_list argp;
    int rc;

    va_start(argp, format);
    rc = vfprintf(stdout, format, argp);
    va_end(argp);
    return rc;
}
    

int
printk(const char *format, ...)
{
    va_list argp;
    int rc;

    va_start(argp, format);
    rc = vfprintf(ssxout, format, argp);
    va_end(argp);
    return rc;
}
    
