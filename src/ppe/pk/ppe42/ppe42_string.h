/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: chips/p9/procedures/ppe/pk/ppe42/ppe42_string.h $             */
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
#ifndef __STRING_H
#define __STRING_H

#include <stdint.h>
typedef uint32_t size_t;

#ifndef NULL
    #ifdef __cplusplus
        #define NULL 0
    #else
        #define NULL ((void*)0)
    #endif
#endif

#ifndef __cplusplus
    typedef int bool;
    #define false 0
    #define true 1
#endif

#ifdef __cplusplus
extern "C"
{
#endif
void* memset(void* s, int c, size_t n) __attribute__ ((weak));
//    void bzero(void *vdest, size_t len);   USE memset
void* memcpy(void* dest, const void* src, size_t num) __attribute__ ((weak));
void* memmove(void* vdest, const void* vsrc, size_t len)  __attribute__ ((weak));
int memcmp(const void* p1, const void* p2, size_t len) __attribute__((weak, pure));
void* memmem(const void* haystack, size_t haystacklen,
             const void* needle, size_t needlelen) __attribute__((weak, pure));

char* strcpy(char* d, const char* s)  __attribute__ ((weak));
char* strncpy(char* d, const char* s, size_t l)  __attribute__ ((weak));
int strcmp(const char* s1, const char* s2) __attribute__((weak, pure));
size_t strlen(const char* s1) __attribute__((weak, pure));
size_t strnlen(const char* s1, size_t n) __attribute__((weak, pure));

char* strcat(char* d, const char* s)  __attribute__ ((weak));
char* strncat(char* d, const char* s, size_t n)  __attribute__ ((weak));

char* strchr(const char* s, int c) __attribute__((weak, pure));


#ifdef __cplusplus
};
#endif

#endif
