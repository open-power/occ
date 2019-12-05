/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/include/nest_dts.h $                                      */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2016,2019                        */
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
#if !defined(__NEST_DTS_H__)
#define __NEST_DTS_H__

#include <eq_config.h>

#define NEST_DTS_COUNT 6

#define N0_DTS 0
#define N1_DTS 1
#define SE_PAU_DTS 2
#define NE_PAU_DTS 3
#define SW_PAU_DTS 4
#define NW_PAU_DTS 5

// Make struct size a multiple of 8 bytes for performance.
typedef struct
{
    sensor_result_t sensor[NEST_DTS_COUNT];
    uint32_t  reserved;
} NestDts_t;

uint32_t get_nest_dts(NestDts_t* o_data);
#endif

