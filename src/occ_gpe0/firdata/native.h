/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/firdata/native.h $                                */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2017                        */
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

#ifdef __cplusplus
extern "C" {
#endif
#include "pk.h"
#ifdef __cplusplus
}
#endif

#ifdef FIRD_DEBUG
#define TRACDCOMP(frmt,args...) DBG_PRINT(frmt,##args)
#else
#define TRACDCOMP(frmt,args...)
#endif // FIRD_DEBUG

#define TRACDCOMP(frmt,args...)
#define TRACFCOMP(frmt,args...)

#define TRAC_IMP(frmt,args...)  PK_TRACE(frmt,##args)
#define TRAC_ERR(frmt,args...)  PK_TRACE(frmt,##args)
#define TRAC_INF(frmt,args...)  PK_TRACE(frmt,##args)

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

#define KILOBYTE  (1024ul)              /**< 1 KB */
#define MEGABYTE  (1024 * 1024ul)       /**< 1 MB */
#define GIGABYTE  (MEGABYTE * 1024ul)   /**< 1 GB */
#define TERABYTE  (GIGABYTE * 1024ul)   /**< 1 TB */

#define PAGESIZE  (4*KILOBYTE)          /**< 4 KB */

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
void sleep( PkInterval i_nanoseconds );


#endif
