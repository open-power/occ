/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/occlib/occhw_xir_dump.c $                             */
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

/// \file occhw_xir_dump.c
/// \brief Implementation of the occhw_xir_dump function
#include "kernel.h"
#include "occhw_common.h"
#include "occhw_xir_dump.h"

int32_t occhw_xir_dump(uint32_t inst_id, occhw_xir_dump_t* xir_dump)
{
    int rc;
    do
    {
        if(!xir_dump)
        {
            rc = OCCHW_XIR_INVALID_POINTER;
            break;
        }

        if(inst_id > OCCHW_INST_ID_MAX_GPE)
        {
            rc = OCCHW_XIR_INVALID_GPE;
            break;
        }

        //TODO: dump the XIR regs once the addresses are available
        rc = 0;

    }
    while(0);

    return rc;
}
