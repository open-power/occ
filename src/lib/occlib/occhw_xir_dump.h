/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/occlib/occhw_xir_dump.h $                             */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2016                        */
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
#ifndef __OCCHW_XIR_DUMP_H__
#define __OCCHW_XIR_DUMP_H__

/// \file occhw_xir_dump.h
/// \brief header for the occhw_xir_dump function
///

#ifndef __ASSEMBLER__
#include "stdint.h"

/// Structure for dumping XIR data for a GPE
typedef struct
{
    uint32_t    xsr;
    uint32_t    sprg0;
    uint32_t    edr;
    uint32_t    ir;
    uint32_t    iar;
    uint32_t    sib_upper;
    uint32_t    sib_lower;
} occhw_xir_dump_t;

///////////////////////////////////////////////////////////////////////////////
/// Dump the XIR registers for a GPE engine
///
/// \param inst_id The instance ID of the target GPE.
///
/// \param xir_dump Pointer to a occhw_xir_dump_t structure.
///
/// Possible return codes are:
///
/// \retval 0 XIR registers were successfully dumped
///
/// \retval OCCHW_XIR_INVALID_GPE \a inst_id is not for a valid GPE instance.
///
/// \retval OCCHW_XIR_INVALID_POINTER \a xir_dump is NULL
///
int32_t occhw_xir_dump(uint32_t inst_id, occhw_xir_dump_t* xir_dump);

#endif /*__ASSEMBLER__*/
#endif /*__OCCHW_XIR_DUMP_H__*/
