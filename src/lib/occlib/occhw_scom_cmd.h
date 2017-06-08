/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/occlib/occhw_scom_cmd.h $                             */
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
#ifndef __OCCHW_SCOM_CMD_H__
#define __OCCHW_SCOM_CMD_H__

/// \file occhw_scom_cmd.h
/// \brief Defines the shared scom command control block
///
/// This file contains definitions that are used by both ppc405 and GPE
/// instances inside the OCC.  

// The most significant bit of scom addresses is always 0, so we use this
// bit to tell the SCOM proxy server (GPE) whether a getscom or putscom is
// needed.
#define OCCHW_SCOM_READ_MASK        0x80000000

// This is a status value that the ppc405 will write to the status
// word prior to starting a new request.  This value can be used to
// detect that the GPE has not yet written status for the request.
// Note: The GPE simply copies it's MSR value to the status field
// when it completes.  This value is not a valid MSR value.
#define OCCHW_SCOM_PENDING          0x0000EF00

// Define the structure offsets for use in assembly code on the GPE
#define OCCHW_SCOM_STATUS_OFFSET    0
#define OCCHW_SCOM_ADDR_OFFSET      4
#define OCCHW_SCOM_DATA_OFFSET      8

#ifndef __ASSEMBLER__
typedef union
{
    uint32_t    status32;
    struct
    {
        uint32_t rsvd0  :  1;
        uint32_t mask   :  7;
        uint32_t is0    :  1;
        uint32_t sibrc  :  3;
        uint32_t lp     :  1;
        uint32_t we     :  1;
        uint32_t is1    :  1;
        uint32_t uie    :  1;
        uint32_t ee     :  1;
        uint32_t rsvd1  :  2;
        uint32_t me     :  1;
        uint32_t rsvd2  :  3;
        uint32_t ipe    :  1;
        uint32_t sibrca :  8;
    };
} occhw_scom_status_t;

typedef struct
{
    //MSR is saved here
    volatile occhw_scom_status_t    scom_status;

    //msg is used to communicate read/!write
    volatile uint32_t               scom_addr;

    //data for putscom or from getscom
    volatile uint64_t               scom_data;
} occhw_scom_cmd_t;

#endif /*__ASSEMBLER__*/

#endif  /* __OCCHW_SCOM_CMD_H__ */
