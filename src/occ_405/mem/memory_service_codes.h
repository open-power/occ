/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/mem/memory_service_codes.h $                      */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2019                        */
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

#ifndef _MEMORY_SERVICE_CODES_H
#define _MEMORY_SERVICE_CODES_H

//*************************************************************************
// Includes
//*************************************************************************
#include <comp_ids.h>


enum memModuleId
{
    MEM_MID_TASK_MEMORY_CONTROL    = MEM_COMP_ID | 0x00,
    MEM_MID_MEM_INIT_POWER_CONTROL = MEM_COMP_ID | 0x01,
    MEM_MID_TASK_DATA              = MEM_COMP_ID | 0x02,
    MEM_MID_OCMB_INIT_MOD          = MEM_COMP_ID | 0x03,
};

#endif // _MEMORY_SERVICE_CODES_H
