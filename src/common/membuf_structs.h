/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/common/membuf_structs.h $                                 */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2016,2018                        */
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

/* This header file is used by both occ_405 and occ_gpe1.                 */
/* Contains common structures and globals.                                */
#if !defined(_MEMBUF_STRUCTS_H)
#define _MEMBUF_STRUCTS_H

#include "stdint.h"
#include "gpe_export.h"
#include "membuf_configuration.h"

// IPC message payload for call to gpe_membuf_init()
typedef struct
{
    GpeErrorStruct error;
    MemBufConfiguration_t * membufConfiguration;
    uint32_t mem_type;

} MemBufConfigParms_t;

// IPC message payload for call to gpe_membuf_scom()
typedef struct
{
    GpeErrorStruct error;

    /**
     * Input: The SCOM list
     * This is a pointer to an array of scomList_t objects
     * describing the sequence of commands to execute.
     */
    scomList_t* scomList;

    /**
     * Input: The number of entries in the scomList.
     * @note It is considered an error if \a entries is 0, under the
     * assumption that the caller must have neglected to initialize the
     * structure.
     */
    int     entries;

} MemBufScomParms_t;

// IPC message payload for call to gpe_membuf_data()
typedef struct
{
    GpeErrorStruct error;

    /**
     * The index (0 .. NMEMBUF - 1) of the MemBuf whose sensor cache
     * data to collect, or -1 to bypass collection.
     */
    int collect;

    /**
     * Send the OCC_TOUCH cmd to the OCMB to reset the OCMB deadman timer
     * if not 0.
     */
    int touch;

    /**
     * Pointer to data collected.  Needs to be set if collect != -1
     * otherwise it's not used.
     */
    uint64_t * data;
} MemBufGetMemDataParms_t;


#endif
