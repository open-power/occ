/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/pss/pss_service_codes.h $                         */
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

#ifndef _PSS_SERVICE_CODES_H_
#define _PSS_SERVICE_CODES_H_

#include <comp_ids.h>

enum pssModuleId
{
    PSS_MID_APSS_INIT              =  PSS_COMP_ID | 0x00,
    PSS_MID_APSS_START_MEAS        =  PSS_COMP_ID | 0x01,
    PSS_MID_APSS_CONT_MEAS         =  PSS_COMP_ID | 0x02,
    PSS_MID_APSS_COMPLETE_MEAS     =  PSS_COMP_ID | 0x03,
    PSS_MID_DPSS_OVS_IRQ_INIT      =  PSS_COMP_ID | 0x05,
    PSS_MID_DO_APSS_RECOVERY       =  PSS_COMP_ID | 0x07,
    PSS_MID_AVSBUS_READ            =  PSS_COMP_ID | 0x08,
    PSS_MID_APSS_RESET             =  PSS_COMP_ID | 0x09,
};

#endif /* #ifndef _PSS_SERVICE_CODES_H_ */
