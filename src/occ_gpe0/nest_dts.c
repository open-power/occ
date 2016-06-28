/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe0/nest_dts.c $                                     */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2016                             */
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
#include <nest_dts.h>
#include <ppe42_msr.h>
#include <ppe42_scom.h>

uint32_t get_nest_dts(NestDts_t* o_data)
{
    uint64_t value64 = 0;
    uint64_t* ptr = (uint64_t*)o_data;
    uint32_t rc = 0;

    uint32_t nest1Select = CHIPLET_NEST_ID(1);
    uint32_t nest3Select = CHIPLET_NEST_ID(3);

    dts_sensor_result_reg_t dts_scom_data;

    int i;

    for(i = 0; i < sizeof(NestDts_t) / 8; ++i)
    {
        ptr[i] = 0;
    }

    // Turn off MCR bits to prevent machine check on error on scom readings
    // bits 1:7
    uint32_t org_sem = mfmsr() & MSR_SEM;

    // Clear SIBRC and SIBRCA
    // mask off resource occupied/offline errors - will return these)
    // SIB rc 3-7 will machine check (unless already masked)
    mtmsr((mfmsr() & ~(MSR_SIBRC | MSR_SIBRCA))
          | 0xe0000000);  //MASK SIBRC == 1 | SIBRC == 2

    // Get DTS readings
    PPE_LVD(nest1Select + THERM_DTS_RESULT, value64);
    dts_scom_data.value = value64;
    o_data->sensor0.result = dts_scom_data.half_words.reading[0];

    PPE_LVD(nest3Select + THERM_DTS_RESULT, value64);
    dts_scom_data.value = value64;
    o_data->sensor1.result = dts_scom_data.half_words.reading[0];
    o_data->sensor2.result = dts_scom_data.half_words.reading[1];

    // Check rc accumulated - ignore rc == 0
    uint32_t sibrca = (mfmsr() & 0x0000007f);

    if(sibrca)
    {
        // Report most severe error in rc
        rc = 7;
        uint32_t mask = 1;

        for(; mask != 0x00000080; mask <<= 1)
        {
            if( mask & sibrca )
            {
                break;
            }

            --rc;
        }
    }

    // Clear masks SIB masks (MSR_SEM)
    // Clear SIBRC and SIMBRCA
    // Restore any SIB masks that may have been on before.
    mtmsr((mfmsr() & ~(MSR_SEM | MSR_SIBRC | MSR_SIBRCA))
          | (org_sem & MSR_SEM));

    return rc;

}

