/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: chips/p9/procedures/ppe/pk/ppe42/ppe42_string.c $             */
/*                                                                        */
/* IBM CONFIDENTIAL                                                       */
/*                                                                        */
/* EKB Project                                                            */
/*                                                                        */
/* COPYRIGHT 2016                                                         */
/* [+] International Business Machines Corp.                              */
/*                                                                        */
/*                                                                        */
/* The source code for this program is not published or otherwise         */
/* divested of its trade secrets, irrespective of what has been           */
/* deposited with the U.S. Copyright Office.                              */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */

// Note: this code does not compile under the ppc2ppe backend.
// It emits illegal ppe42 asm instructions.
// __PPE42__ is set by the ppe42 compiler
#ifdef __PPE42__

#include <ppe42_string.h>
#ifdef __cplusplus
extern "C"
{
#endif
void* memset(void* vdest, int ch, size_t len)
{
    uint32_t addr = (uint32_t)vdest;

    while(len && (addr & 0x7)) // not 8 byte aligned
    {
        uint8_t* p = (uint8_t*)addr;
        *p = ch;
        ++addr;
        --len;
    }

    if(len >= sizeof(uint64_t))
    {
        uint64_t lch = ch & 0xff;
        lch |= lch << 8;
        lch |= lch << 16;
        lch |= lch << 32;

        while(len >= sizeof(uint64_t))
        {
            uint64_t* p = (uint64_t*)addr;
            *p = lch;
            len -= sizeof(uint64_t);
            addr += sizeof(uint64_t);
        }
    }

    while(len)
    {
        uint8_t* p = (uint8_t*)addr;
        *p = ch;
        ++addr;
        --len;
    }

    return vdest;
}


void* memcpy(void* vdest, const void* vsrc, size_t len)
{

    // Loop, copying 4 bytes
    long* ldest = (long*)vdest;
    const long* lsrc = (const long*)vsrc;

    while (len >= sizeof(long))
    {
        *ldest++ = *lsrc++;
        len -= sizeof(long);
    }

    // Loop, copying 1 byte
    char* cdest = (char*)ldest;
    const char* csrc = (const char*)lsrc;
    size_t i = 0;

    for (; i < len; ++i)
    {
        cdest[i] = csrc[i];
    }

    return vdest;
}

void* memmove(void* vdest, const void* vsrc, size_t len)
{
    // Copy first-to-last
    if (vdest <= vsrc)
    {
        return memcpy(vdest, vsrc, len);
    }

    // Copy last-to-first (TO_DO: optimize)
    char* dest = (char*)(vdest);
    const char* src = (const char*)(vsrc);
    size_t i = len;

    for (; i > 0;)
    {
        --i;
        dest[i] = src[i];
    }

    return vdest;
}

int memcmp(const void* p1, const void* p2, size_t len)
{
    const char* c1 = (const char*)(p1);
    const char* c2 = (const char*)(p2);

    size_t i = 0;

    for (; i < len; ++i)
    {
        long n = (long)(c1[i]) - (long)(c2[i]);

        if (n != 0)
        {
            return n;
        }
    }

    return 0;
}

void* memmem(const void* haystack, size_t haystacklen,
             const void* needle, size_t needlelen)
{
    const void* result = NULL;

    if (haystacklen >= needlelen)
    {
        const char* c_haystack = (const char*)(haystack);
        const char* c_needle = (const char*)(needle);
        bool match = false;

        size_t i = 0;

        for (; i <= (haystacklen - needlelen); i++)
        {
            match = true;

            size_t j = 0;

            for (; j < needlelen; j++)
            {
                if (*(c_haystack + i + j) != *(c_needle + j))
                {
                    match = false;
                    break;
                }
            }

            if (match)
            {
                result = (c_haystack + i);
                break;
            }
        }
    }

    return (void*)(result);
}


char* strcpy(char* d, const char* s)
{
    char* d1 = d;

    do
    {
        *d1 = *s;

        if (*s == '\0')
        {
            return d;
        }

        d1++;
        s++;
    }
    while(1);
}

char* strncpy(char* d, const char* s, size_t l)
{
    char* d1 = d;
    size_t len = 0;

    do
    {
        if (len++ >= l)
        {
            break;
        }

        *d1 = *s;

        if (*s == '\0')
        {
            break;
        }

        d1++;
        s++;
    }
    while(1);

    // pad the remainder
    while( len < l )
    {
        d1[len++] = '\0';
    }

    return d;
}

int strcmp(const char* a, const char* b)
{
    while((*a != '\0') && (*b != '\0'))
    {
        if (*a == *b)
        {
            a++;
            b++;
        }
        else
        {
            return (*a > *b) ? 1 : -1;
        }
    }

    if (*a == *b)
    {
        return 0;
    }

    if (*a == '\0')
    {
        return -1;
    }
    else
    {
        return 1;
    }
}

size_t strlen(const char* a)
{
    size_t length = 0;

    while(*a++)
    {
        length++;
    }

    return length;
}

size_t strnlen(const char* s, size_t n)
{
    size_t length = 0;

    while((length < n) && (*s++))
    {
        length++;
    }

    return length;
}

char* strcat(char* d, const char* s)
{
    char* _d = d;

    while(*_d)
    {
        _d++;
    }

    while(*s)
    {
        *_d = *s;
        _d++;
        s++;
    }

    *_d = '\0';

    return d;
}

char* strncat(char* d, const char* s, size_t n)
{
    char* _d = d;

    while(*_d)
    {
        _d++;
    }

    while((*s) && (0 != n))
    {
        *_d = *s;
        _d++;
        s++;
        n--;
    }

    *_d = '\0';

    return d;
}


char* strchr(const char* s, int c)
{
    while((*s != '\0') && (*s != c))
    {
        s++;
    }

    if (*s == c)
    {
        return (char*)s;
    }

    return NULL;
}
#ifdef __cplusplus
};
#endif
#endif
