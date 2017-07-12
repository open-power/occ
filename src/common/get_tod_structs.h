/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/common/get_tod_structs.h $                                */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2017,2017                        */
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

/* This header file is used by both occ_405 and occ_gpe0.                 */
/* Contains common structures and defines.                                */

#ifndef _GET_TOD_STRUCTS_H
#define _GET_TOD_STRUCTS_H

#include <stdint.h>             // For uint*_t
#include <gpe_export.h>         // For GpeErrorStruct

/**
 * Struct containing the error state and arguments for the IPC function
 * IPC_ST_GET_TOD_FUNCID.
 */
typedef struct __attribute__ ((packed))
{
    GpeErrorStruct error;
    uint64_t       tod;
} gpe_get_tod_args_t;

/**
 * Value used when the actual TOD value is unknown.
 */
#define TOD_VALUE_UNKNOWN 0xFFFFFFFFFFFFFFFFull

#endif // _GET_TOD_STRUCTS_H
