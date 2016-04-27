/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe0/core_data.c $                                    */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2016                        */
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

/// \file core_data.c
/// \brief The GPE program that collect raw data for DTS and EMPATH
///

#include "core_data.h"
#include "p9_config.h"
#include "ppe42_msr.h"
#include "ppe42_scom.h"

uint32_t get_core_data(uint32_t i_core,
                       CoreData* o_data)
{
    uint32_t rc = 0;
    uint32_t size = sizeof(CoreData) / 8;
    uint64_t* ptr = (uint64_t*)o_data;
    uint32_t coreSelect = CHIPLET_CORE_ID(i_core);
    uint32_t quadSelect = CHIPLET_CACHE_ID((i_core / 4));
    //volatile uint64_t* scom_reg = (uint64_t*)(0);
    uint64_t value64 = 0;

    uint32_t i;

    for(i = 0; i < size; ++i)
    {
        ptr[i] = 0;
    }

    // Turn off MCR bit to prevent machine check on error on scom readings bits 1:7
    // rc == 1 resource occupied  (see ppe42_scom.h)   Action: return with rc
    // rc == 2 Core is fenced, offline                 Action: return with rc
    // rc == 3 partial good
    // rc == 4 address error
    // rc == 5 clock error
    // rc == 6 packet error
    // rc == 7 timeout
    // ACTIONS:
    // rc 1,2:- mask the machine check, return rc
    // 3-7 Leave as machine check (or as it was)
    uint32_t org_sem = mfmsr() & MSR_SEM;

    // Clear SIBRC and SIBRCA
    // mask off resource occupied/offline errors - will return these)
    // SIB rc 3-7 will machine check (unless already masked)
    mtmsr((mfmsr() & ~(MSR_SIBRC | MSR_SIBRCA))
          | 0xe0000000);  //MASK SIBRC == 1 | SIBRC == 2

    dts_sensor_result_reg_t dts_scom_data;

    //scom_reg = (uint64_t*)(quadSelect + THERM_DTS_RESULT);
    //dts_scom_data.value = *scom_reg;
    PPE_LVD(quadSelect + THERM_DTS_RESULT, value64);
    dts_scom_data.value = value64;

    // Pick the sensor reading closest to core
    // first two cores - use cache dts0
    // last two cores -  use cache dts1
    if(i_core & 0x00000002)
    {
        o_data->dts.cache.result = dts_scom_data.half_words.reading[1];
    }
    else
    {
        o_data->dts.cache.result = dts_scom_data.half_words.reading[0];
    }

    //scom_reg = (uint64_t*)(coreSelect + THERM_DTS_RESULT);
    //dts_scom_data.value = *scom_reg;
    PPE_LVD(coreSelect + THERM_DTS_RESULT, value64);
    dts_scom_data.value = value64;

    o_data->dts.core[0].result = dts_scom_data.half_words.reading[0];
    o_data->dts.core[1].result = dts_scom_data.half_words.reading[1];

    // Send command to select which emmpath counter to read
    uint64_t empath_scom_data = CORE_RAW_CYCLES;
    //scom_reg = (uint64_t*)(coreSelect + PC_OCC_SPRC);
    //*scom_reg = empath_scom_data;
    PPE_STVD(coreSelect + PC_OCC_SPRC, empath_scom_data)


    // Read counters.
    //  Counter selected auto increments to the next counter after each read.

    //CORE_RAW_CYCLES
    //scom_reg = (uint64_t*)(coreSelect + PC_OCC_SPRD);
    //empath_scom_data = *scom_reg;
    PPE_LVD(coreSelect + PC_OCC_SPRD, empath_scom_data);
    o_data->empath.raw_cycles = (uint32_t)empath_scom_data;

    //CORE_RUN_CYCLES
    //empath_scom_data = *scom_reg;
    PPE_LVD(coreSelect + PC_OCC_SPRD, empath_scom_data);
    o_data->empath.run_cycles = (uint32_t)empath_scom_data;

    //CORE_WORKRATE_BUSY
    //empath_scom_data = *scom_reg;
    PPE_LVD(coreSelect + PC_OCC_SPRD, empath_scom_data);
    o_data->empath.freq_sens_busy = (uint32_t)empath_scom_data;

    //CORE_WORKRATE_FINISH
    //empath_scom_data = *scom_reg;
    PPE_LVD(coreSelect + PC_OCC_SPRD, empath_scom_data);
    o_data->empath.freq_sens_finish = (uint32_t)empath_scom_data;

    //CORE_MEM_HIER_A_LATENCY
    // empath_scom_data = *scom_reg;
    PPE_LVD(coreSelect + PC_OCC_SPRD, empath_scom_data);
    o_data->empath.mem_latency_a = (uint32_t)empath_scom_data;

    //CORE_MEM_HIER_B_LATENCY
    // empath_scom_data = *scom_reg;
    PPE_LVD(coreSelect + PC_OCC_SPRD, empath_scom_data);
    o_data->empath.mem_latency_b = (uint32_t)empath_scom_data;

    //CORE_MEM_HIER_C_ACCESS
    //empath_scom_data = *scom_reg;
    PPE_LVD(coreSelect + PC_OCC_SPRD, empath_scom_data);
    o_data->empath.mem_access_c = (uint32_t)empath_scom_data;

    int thread = 0;

    for( ; thread < EMPATH_CORE_THREADS; ++thread )
    {
        // THREAD_RUN_CYCLES
        //empath_scom_data = *scom_reg;
        PPE_LVD(coreSelect + PC_OCC_SPRD, empath_scom_data);
        o_data->per_thread[thread].run_cycles = (uint32_t)empath_scom_data;

        // THREAD_INST_DISP_UTIL
        //empath_scom_data = *scom_reg;
        PPE_LVD(coreSelect + PC_OCC_SPRD, empath_scom_data);
        o_data->per_thread[thread].dispatch = (uint32_t)empath_scom_data;

        // THREAD_INST_COMP_UTIL
        //empath_scom_data = *scom_reg;
        PPE_LVD(coreSelect + PC_OCC_SPRD, empath_scom_data);
        o_data->per_thread[thread].completion = (uint32_t)empath_scom_data;

        // THREAD_MEM_HEIR_C_ACCESS
        //empath_scom_data = *scom_reg;
        PPE_LVD(coreSelect + PC_OCC_SPRD, empath_scom_data);
        o_data->per_thread[thread].mem_c = (uint32_t)empath_scom_data;
    }

    //IFU_THROTTLE_BLOCK_FETCH
    //empath_scom_data = *scom_reg;
    PPE_LVD(coreSelect + PC_OCC_SPRD, empath_scom_data);
    o_data->throttle.ifu_throttle = (uint32_t)empath_scom_data;

    //IFU_THROTTLE_ACTIVE
    //empath_scom_data = *scom_reg;
    PPE_LVD(coreSelect + PC_OCC_SPRD, empath_scom_data);
    o_data->throttle.ifu_active = (uint32_t)empath_scom_data;

    //VOLT_DROOP_THROTTLE_ACTIVE
    //empath_scom_data = *scom_reg;
    PPE_LVD(coreSelect + PC_OCC_SPRD, empath_scom_data);
    o_data->throttle.v_droop = (uint32_t)empath_scom_data;

    // TOD value
    //scom_reg = (uint64_t*)TOD_VALUE_REG;
    PPE_LVD(TOD_VALUE_REG, empath_scom_data);
    //empath_scom_data = *scom_reg;
    o_data->empath.tod_2mhz = (uint32_t)(empath_scom_data >> 8); //[24..56]

    // STOP_STATE_HIST_OCC_REG
    PPE_LVD(coreSelect + STOP_STATE_HIST_OCC_REG, empath_scom_data);
    o_data->stop_state_hist = empath_scom_data;

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
