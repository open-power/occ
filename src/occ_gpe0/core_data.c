/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe0/core_data.c $                                    */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015                             */
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
#include "core_data.h"
#include "ppe42_scom.h"
#include "p9_config.h"

uint32_t get_core_data(uint32_t i_core,
                       CoreData* o_data)
{
    uint32_t rc = 0;
    uint32_t size = sizeof(CoreData) / 8;
    uint64_t* ptr = (uint64_t*)o_data;
    uint32_t i = 0;
    for(i = 0; i < size; ++i)
    {
        ptr[i] = 0;
    }

    /*   TODO RTC 141391 - No simics support to test this yet.
    uint32_t coreSelect = CHIPLET_CORE_ID(i_core);
    uint32_t quadSelect = CHIPLET_CACHE_ID((i_core / 4));
    dts_sensor_result_reg_t scom_data;

    rc = getscom(quadSelect,THERM_DTS_RESULT, &(scom_data.value));
    if(!rc)
    {
        // Pick the sensor reading closest to core TODO check this!
        // first two cores - use cache dts0
        // last two cores -  use cache dts1
        if(i_core & 0x00000002)
        {
            o_data->dts.cache.result = scom_data.half_words.reading[1];
        }
        else
        {
            o_data->dts.cache.result = scom_data.half_words.reading[0];
        }

        rc = getscom(coreSelect,THERM_DTS_RESULT, &(scom_data.value));
        if(!rc)
        {
            o_data->dts.core[0].result = scom_data.half_words.reading[0];
            o_data->dts.core[1].result = scom_data.half_words.reading[1];
        }
    }
    */
    return rc;
}
