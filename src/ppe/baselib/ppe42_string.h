/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/baselib/ppe42_string.h $                              */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2019                             */
/* [+] International Business Machines Corp.                              */
/*                                                                        */
/*                                                                        */
/* Licensed under the Apache License, Version 2.0 (the "License");        */
/* you may not use this file except in compliance with the License.       */
/* You may obtain a copy of the License at                                */
/*                                                                        */
/*     http://www.apache.org/licenses/LICENSE-2.0                         */
/*                                                                        */
/* Unless required by applicable law or agreed to in writing, software    */
/* distributed under the License is distributed on an "AS IS" BASIS,      */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or        */
/* implied. See the License for the specific language governing           */
/* permissions and limitations under the License.                         */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */
#ifndef __STRING_H
#define __STRING_H

#include <stdint.h>
#include <stddef.h>

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
