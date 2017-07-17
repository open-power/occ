/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/proc/proc_data_service_codes.h $                  */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2017                        */
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

#ifndef _PROC_DATA_SERVICE_CODES_H_
#define _PROC_DATA_SERVICE_CODES_H_

#include <comp_ids.h>

enum procModuleId
{
    PROC_TASK_CORE_DATA_MOD         = PROC_COMP_ID | 0x00,
    PROC_CORE_INIT_MOD              = PROC_COMP_ID | 0x01,
    PROC_TASK_FAST_CORE_DATA_MOD    = PROC_COMP_ID | 0x02,
    PROC_GPST_INIT_FAILURE_MOD      = PROC_COMP_ID | 0x03,
    PROC_ENABLE_PSTATES_SMH_MOD     = PROC_COMP_ID | 0x04,
    PROC_PSTATE_KVM_SETUP_MOD       = PROC_COMP_ID | 0x05,
    PROC_TASK_NEST_DTS_MOD          = PROC_COMP_ID | 0x06,
    PROC_NEST_DTS_INIT_MOD          = PROC_COMP_ID | 0x07,
    PROC_24X7_MOD                   = PROC_COMP_ID | 0x08,
    PROC_POP_OPAL_TBL_TO_MEM_MOD    = PROC_COMP_ID | 0x09,
    PROC_CHECK_FOR_OPAL_UPDATES_MOD = PROC_COMP_ID | 0x0A,
};

#endif /* #ifndef _PROC_DATA_SERVICE_CODES_H_ */
