/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/pgpe/pgpe_service_codes.h $                       */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2019                        */
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

#ifndef _PGPE_SERVICE_CODES_H_
#define _PGPE_SERVICE_CODES_H_

#include <comp_ids.h>

enum pgpeModuleId
{
    PGPE_INIT_CLIPS_MOD              = PGPE_COMP_ID | 0x00,
    PGPE_INIT_PMCR_MOD               = PGPE_COMP_ID | 0x01,
    PGPE_INIT_START_SUSPEND_MOD      = PGPE_COMP_ID | 0x02,
    PGPE_INIT_WOF_CONTROL_MOD        = PGPE_COMP_ID | 0x03,
    PGPE_INIT_WOF_VRT_MOD            = PGPE_COMP_ID | 0x04,
    PGPE_CLIP_UPDATE_MOD             = PGPE_COMP_ID | 0x05,
    PGPE_START_SUSPEND_MOD           = PGPE_COMP_ID | 0x06,
    PGPE_PMCR_SET_MOD                = PGPE_COMP_ID | 0x07,
    PGPE_SET_CLIP_BLOCKING_MOD       = PGPE_COMP_ID | 0x09,
};


#endif /* #ifndef _PGPE_SERVICE_CODES_H_ */
