/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe0/nest_dts.c $                                     */
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

#include <nest_dts.h>
#include <ppe42_msr.h>
#include <ppe42_scom.h>

uint32_t nest_dts_scom[NEST_DTS_COUNT] =
{
    0x02050000, // N0 THERM
    0x03050000, // N1 THERM
    0x10050000, // East South PAU Therm
    0x11050000, // East North PAU Therm
    0x12050000, // West South PAU Therm
    0x13050000, // West North PAU Therm
};

uint32_t get_nest_dts(NestDts_t* o_data)
{
    uint32_t rc = 0;
    uint64_t* clr_ptr = (uint64_t*)o_data;
    int i;

    for(i = 0; i < sizeof(NestDts_t) / 8; ++i)
    {
        clr_ptr[i] = 0;
    }

    // Turn off MCR bits to prevent machine check on error on scom readings
    // bits 1:7
    uint32_t org_sem = mfmsr() & MSR_SEM;

    // Clear SIBRC and SIBRCA
    // mask off SIB errors as machine checks, return rc instead
    mtmsr((mfmsr() & ~(MSR_SIBRC | MSR_SIBRCA)) | MSR_SEM);

    for( i = 0; i < NEST_DTS_COUNT; ++i)
    {
        uint64_t value64 = 0;
        uint32_t address = nest_dts_scom[i];
        dts_sensor_result_reg_t dts_scom_data;

        // Get DTS readings
        PPE_LVD(address, value64);
        dts_scom_data.value = value64;
        o_data->sensor[i].result = dts_scom_data.half_words.reading[0];
    }

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

int encode2response(uint32_t v)
{
    switch(v)
    {
        case 0:  return 0; break;
        case 4:  return 1; break;
        case 6:  return 2; break;
        case 7:  return 3; break;
        default: return -1; break;
    };
    return -1;
}

uint32_t get_nest_droop_sensors(NestDdsData * o_ddsData)
{
    uint32_t rc = 0;
    dpll_echar_reg_t dpll_echar;

    o_ddsData->value = 0;

    rc = getscom_abs(NEST_DPLL_ECHAR,&(dpll_echar.value));
    if(rc == 0)
    {
        int response = encode2response(dpll_echar.fields.encode);
        if(response > 0)
        {
            o_ddsData->fields.dds_reading = response;
            o_ddsData->fields.dds_valid = 1;
        }

        response = encode2response(dpll_echar.fields.encode_min);
        if(response > 0)
        {
            o_ddsData->fields.dds_min = response;
            o_ddsData->fields.dds_min_valid = 1;
        }

        response = encode2response(dpll_echar.fields.encode_max);
        if(response > 0)
        {
            o_ddsData->fields.dds_max = response;
            o_ddsData->fields.dds_max_valid = 1;
        }
    }
    return rc;
}
