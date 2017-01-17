/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/wof/wof_service_codes.h $                         */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2017                             */
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
#ifndef _WOF_SERVICE_CODES_H_
#define _WOF_SERVICE_CODES_H_

#include <comp_ids.h>

enum wofModuleId
{
    WOF_MAIN                            =  WOF_COMP_ID | 0x01,
    SEND_VFRT_TO_PGPE                   =  WOF_COMP_ID | 0x02,
    COPY_VFRT_TO_SRAM                   =  WOF_COMP_ID | 0x03,
    WOF_VFRT_CALLBACK                   =  WOF_COMP_ID | 0x04,
};





#endif /* #ifndef _WOF_SERVICE_CODES_H_ */
