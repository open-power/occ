/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/firdata/native.H $                                    */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015                             */
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

/* Native functions provided by OCC code */
#ifndef _NATIVE_H
#define _NATIVE_H

#include <common_types.h>
#include <trac.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "ssx.h"
#ifdef __cplusplus
}
#endif

#ifndef NO_TRAC_STRINGS

#ifdef FIRD_DEBUG
#define TRACDCOMP(frmt,args...) DBG_PRINT(frmt,##args)
#else
#define TRACDCOMP(frmt,args...)
#endif // FIRD_DEBUG

#define TRACFCOMP(frmt,args...) TRACE(g_trac_inf,INFO_MRK frmt,##args)

#else // NO_TRAC_STRINGS

#define TRACDCOMP(frmt,args...)
#define TRACFCOMP(frmt,args...)

#endif // NO_TRAC_STRINGS

typedef uint32_t errorHndl_t;

#define ENTER_MRK
#define NO_ERROR 0

/* Return a number >= input that is aligned up to the next 4-byte boundary */
#define ALIGN_4(u) (((u) + 0x3ull) & ~0x3ull)

#define NS_PER_SEC (1000000000ull)

#undef be64toh
#undef htobe64
#define be64toh(x) (x)
#define htobe64(x) (x)

#if defined(KILOBYTE)
#undef KILOBYTE
#endif
#define KILOBYTE  (1024ul)              /**< 1 KB */
#define MEGABYTE  (1024 * 1024ul)       /**< 1 MB */
#define GIGABYTE  (MEGABYTE * 1024ul)   /**< 1 GB */
#define TERABYTE  (GIGABYTE * 1024ul)   /**< 1 TB */

#define PAGESIZE  (4*KILOBYTE)          /**< 4 KB */
#define PAGE_SIZE PAGESIZE

#undef SUCCESS
#define SUCCESS 0

#undef FAIL
#define FAIL -1

/*================================================ */

/* XSCOM Read */
int32_t xscom_read( uint32_t i_address, uint64_t * o_data );

/* XSCOM Write */
int32_t xscom_write( uint32_t i_address, uint64_t i_data );

/* Sleep */
void sleep( SsxInterval i_nanoseconds );


#endif
