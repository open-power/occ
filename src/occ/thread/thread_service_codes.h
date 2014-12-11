/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/thread/thread_service_codes.h $                       */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2014                        */
/* [+] Google Inc.                                                        */
/* [+] International Business Machines Corp.                              */
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

//  NOTE: The thread component will not actually generate errors
//  using the thread component.  This is due to the fact that  will
//  use trace so therefore we can induce non-ending recursion if trace
//  were to use ERRL.

//  A trace failure is not a system critical failure which is why we
//  can get away with it not reporting errors.

#ifndef _THREAD_SERVICE_CODES_H_
#define _THREAD_SERVICE_CODES_H_

#include <comp_ids.h>

enum thrdModuleId
{
    THRD_MID_INIT_THREAD_SCHDLR     =  THRD_COMP_ID | 0x00,
    THRD_MID_THREAD_SWAP_CALLBACK   =  THRD_COMP_ID | 0x01,
    THRD_MID_GEN_CALLHOME_LOG       =  THRD_COMP_ID | 0x02,
    THRD_THERMAL_VRM_FAN_CONTROL    =  THRD_COMP_ID | 0x03,
    THRD_THERMAL_MAIN               =  THRD_COMP_ID | 0x04,
};

#endif /* #ifndef _THREAD_SERVICE_CODES_H_ */
