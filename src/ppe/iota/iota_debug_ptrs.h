/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/iota/iota_debug_ptrs.h $                              */
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
#if !defined(__IOTA_DEBUG_PTRS_H__)
#define __IOTA_DEBUG_PTRS_H__
/**
 * @file iota_debug_ptrs.h
 * @brief Structure for a table of pointers to debug data
 */

#define IOTA_DEBUG_PTRS_VERSION  1

typedef struct
{
    // The size and version of this structure
    unsigned short   debug_ptrs_size;
    unsigned short   debug_ptrs_version;

    // Trace buffer location and size
    void*           debug_trace_ptr;
    unsigned long   debug_trace_size;


} iota_debug_ptrs_t;

#endif
