/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/gpu/gpu_service_codes.h $                         */
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

#ifndef _GPU_SERVICE_CODES_H_
#define _GPU_SERVICE_CODES_H_

#include <comp_ids.h>

enum gpuModuleId
{
    GPU_MID_INIT                      =  GPU_COMP_ID | 0x00,
    GPU_MID_GPU_SM                    =  GPU_COMP_ID | 0x01,
    GPU_MID_MARK_GPU_FAILED           =  GPU_COMP_ID | 0x02,
    GPU_MID_GPU_SCHED_REQ             =  GPU_COMP_ID | 0x03,
    GPU_MID_GPU_SCHED_RSP             =  GPU_COMP_ID | 0x04,
    GPU_MID_GPU_RESET_SM              =  GPU_COMP_ID | 0x05,
    GPU_MID_GPU_READ_TEMP             =  GPU_COMP_ID | 0x06,
    GPU_MID_GPU_READ_MEM_TEMP         =  GPU_COMP_ID | 0x07,
    GPU_MID_GPU_READ_MEM_TEMP_CAPABLE =  GPU_COMP_ID | 0x08,
    GPU_MID_GPU_CHECK_DRIVER_LOADED   =  GPU_COMP_ID | 0x09,
    GPU_MID_GPU_READ_PWR_LIMIT        =  GPU_COMP_ID | 0x0A,
    GPU_MID_GPU_SET_PWR_LIMIT         =  GPU_COMP_ID | 0x0B,
};

#endif /* #ifndef _GPU_SERVICE_CODES_H_ */
