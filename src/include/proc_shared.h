/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/include/proc_shared.h $                                   */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2016                        */
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

#ifndef _PROC_SHARED_H
#define _PROC_SHARED_H

#include "core_data.h"
#include "nest_dts.h"
#include "gpe_export.h"

// Paramaters for gpe_get_core_data()
typedef struct ipc_core_data_parms
{
    GpeErrorStruct   error;
    CoreData*  data;
    uint32_t   core_num;
} ipc_core_data_parms_t;

typedef struct ipc_nest_dts_parms
{
    GpeErrorStruct  error;
    NestDts_t       data;
} ipc_nest_dts_parms_t;

typedef struct ipc_quad_data_parms
{
    GpeErrorStruct   error;
    QuadData*  data;
    uint32_t   quad_mask;
} ipc_quad_data_parms_t;

#endif // _PROC_SHARED_H
