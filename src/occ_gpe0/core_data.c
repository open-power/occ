/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe0/core_data.c $                                    */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2019                        */
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
#include "eq_config.h"
#include "ppe42_msr.h"
#include "ppe42_scom.h"
#include "pk.h"

// Retry for CoreDataEmpath counts.
// Work-around for HW539674
uint32_t empath_retry(uint32_t core_base_address, uint32_t counter_offset, uint64_t * value)
{
    uint32_t rc = 0;
    int retry_count = 0;
    for(;retry_count < MAX_SCOMD_RETRY; ++retry_count)
    {
        uint64_t scom_data = (uint64_t)counter_offset;
        rc = putscom(core_base_address, PC_OCC_SPRC, scom_data);
        if (rc)
        {
            break;
        }

        scom_data = 0;
        rc = getscom(core_base_address, PC_OCC_SPRD, &scom_data);
        if (rc)
        {
            break;
        }
        if(scom_data != 0ull)
        {
            rc = 0;
            *value = scom_data;
            break;
        }
        rc = EMPATH_COUNTER_ZERO;
    }

    if (EMPATH_COUNTER_ZERO == rc)
    {
        PK_TRACE("EMPATH count zero for offset 0x%x",counter_offset);
        rc = 0;
    }
    return rc;
}

uint32_t get_core_data(uint32_t i_core,
                       CoreData* o_data)
{
    uint32_t rc = 0;
    uint32_t size = sizeof(CoreData) / sizeof(uint64_t);
    uint64_t* core_data64 = (uint64_t*)o_data;
    uint32_t quadSelect = CHIPLET_QUAD_BASE((i_core / CORES_PER_QUAD));
    uint32_t dtsCoreSelect = DTS_CHIPLET_CORE_OFFSET(i_core % CORES_PER_QUAD) + quadSelect;
    uint32_t raceTrackSelect = quadSelect + EQ_DTS_RACETRACK_OFFSET;

    uint32_t i;

    for(i = 0; i < size; ++i)
    {
        core_data64[i] = 0;
    }

    // Turn off MCR bit to prevent machine check on error on scom readings bits 1:7
    // rc == 1 resource occupied  (see ppe42_scom.h)
    // rc == 2 Core is fenced, offline
    // rc == 3 partial good
    // rc == 4 address error (Can be caused by other device using bus)
    // rc == 5 clock error
    // rc == 6 packet error
    // rc == 7 timeout
    uint32_t org_sem = mfmsr() & MSR_SEM;

    // Clear SIBRC and SIBRCA
    // mask off SIB errors as machine checks, return rc instead
    mtmsr((mfmsr() & ~(MSR_SIBRC | MSR_SIBRCA)) | MSR_SEM);

    // ==============
    // DTS
    // ==============
    do
    {
        dts_sensor_result_reg_t dts_scom_data;

        rc = getscom(raceTrackSelect,THERM_DTS_RESULT, &(dts_scom_data.value));
        if (rc)
        {
            break;
        }

        // Store the Racetrace DTS reading
        o_data->dts.racetrack.result = dts_scom_data.half_words.reading[0];

        rc = getscom(dtsCoreSelect, THERM_DTS_RESULT, &(dts_scom_data.value));
        if (rc)
        {
            break;
        }

        o_data->dts.core[0].result = dts_scom_data.half_words.reading[0];
        o_data->dts.core[1].result = dts_scom_data.half_words.reading[1];
        o_data->dts.cache.result = dts_scom_data.half_words.reading[2];


        // =============
        // EMPATH
        // =============
        // Send command to select which emmpath counter to read
        uint32_t empath_offset = CORE_RAW_CYCLES;
        uint64_t empath_scom_data = (uint64_t)empath_offset;

        uint32_t empathSelect = quadSelect + CORE_REGION(i_core);
        rc = putscom(empathSelect, PC_OCC_SPRC, empath_scom_data);
        if (rc)
        {
            break;
        }

        // Read counters.
        //  Counter selected auto increments to the next counter after each read.

        //CORE_RAW_CYCLES
        rc = getscom(empathSelect, PC_OCC_SPRD, &empath_scom_data);
        if (rc)
        {
            break;
        }
        if(empath_scom_data == 0ull)
        {
            rc = empath_retry(empathSelect, empath_offset, &empath_scom_data);
            if (rc)
            {
                break;
            }
        }
        o_data->empath.raw_cycles = (uint32_t)empath_scom_data;

        //CORE_RUN_CYCLES
        empath_offset += 8;
        rc = getscom(empathSelect, PC_OCC_SPRD, &empath_scom_data);
        if (rc)
        {
            break;
        }
        if(empath_scom_data == 0ull)
        {
            rc = empath_retry(empathSelect, empath_offset, &empath_scom_data);
            if (rc)
            {
                break;
            }
        }
        o_data->empath.run_cycles = (uint32_t)empath_scom_data;

        // Core instruction Complete Utilization Counter
        empath_offset += 8;
        rc = getscom(empathSelect, PC_OCC_SPRD, &empath_scom_data);
        if (rc)
        {
            break;
        }
        if(empath_scom_data == 0ull)
        {
            rc = empath_retry(empathSelect, empath_offset, &empath_scom_data);
            if (rc)
            {
                break;
            }
        }
        o_data->empath.complete = (uint32_t)empath_scom_data;

        //CORE_WORKRATE_BUSY
        empath_offset += 8;
        rc = getscom(empathSelect, PC_OCC_SPRD,&empath_scom_data);
        if (rc)
        {
            break;
        }
        if(empath_scom_data == 0ull)
        {
            rc = empath_retry(empathSelect, empath_offset, &empath_scom_data);
            if (rc)
            {
                break;
            }
        }
        o_data->empath.freq_sens_busy = (uint32_t)empath_scom_data;

        //CORE_WORKRATE_FINISH
        empath_offset += 8;
        rc = getscom(empathSelect, PC_OCC_SPRD,&empath_scom_data);
        if (rc)
        {
            break;
        }
        if(empath_scom_data == 0ull)
        {
            rc = empath_retry(empathSelect, empath_offset, &empath_scom_data);
            if (rc)
            {
                break;
            }
        }
        o_data->empath.freq_sens_finish = (uint32_t)empath_scom_data;

        //CORE_MEM_HIER_A_LATENCY
        empath_offset += 8;
        rc = getscom(empathSelect, PC_OCC_SPRD,&empath_scom_data);
        if (rc)
        {
            break;
        }
        if(empath_scom_data == 0ull)
        {
            rc = empath_retry(empathSelect, empath_offset, &empath_scom_data);
            if (rc)
            {
                break;
            }
        }
        o_data->empath.mem_latency_a = (uint32_t)empath_scom_data;

        //CORE_MEM_HIER_B_LATENCY
        empath_offset += 8;
        rc = getscom(empathSelect, PC_OCC_SPRD,&empath_scom_data);
        if (rc)
        {
            break;
        }
        if(empath_scom_data == 0ull)
        {
            rc = empath_retry(empathSelect, empath_offset, &empath_scom_data);
            if (rc)
            {
                break;
            }
        }
        o_data->empath.mem_latency_b = (uint32_t)empath_scom_data;

        //CORE_MEM_HIER_C_ACCESS
        empath_offset += 8;
        rc = getscom(empathSelect, PC_OCC_SPRD,&empath_scom_data);
        if (rc)
        {
            break;
        }
        if(empath_scom_data == 0ull)
        {
            rc = empath_retry(empathSelect, empath_offset, &empath_scom_data);
            if (rc)
            {
                break;
            }
        }
        o_data->empath.mem_access_c = (uint32_t)empath_scom_data;

        //IFU_THROTTLE_BLOCK_FETCH
        rc = getscom(empathSelect, PC_OCC_SPRD,&empath_scom_data);
        if (rc)
        {
            break;
        }
        o_data->throttle.ifu_throttle = (uint32_t)empath_scom_data;

        //IFU_THROTTLE_ACTIVE
        rc = getscom(empathSelect, PC_OCC_SPRD,&empath_scom_data);
        if (rc)
        {
            break;
        }
        o_data->throttle.ifu_active = (uint32_t)empath_scom_data;

        // Power Proxy
        rc = getscom(empathSelect, PC_OCC_SPRD,&empath_scom_data);
        if (rc)
        {
            break;
        }
        o_data->throttle.pwr_proxy = (uint32_t)empath_scom_data;

        // Fine Throttle Blocked
        rc = getscom(empathSelect, PC_OCC_SPRD,&empath_scom_data);
        if (rc)
        {
            break;
        }
        o_data->throttle.fine_throttle = (uint32_t)empath_scom_data;

        // Harsh Throttle Active
        rc = getscom(empathSelect, PC_OCC_SPRD,&empath_scom_data);
        if (rc)
        {
            break;
        }
        o_data->throttle.harsh_throttle = (uint32_t)empath_scom_data;

        // Small Droop Present
        rc = getscom(empathSelect, PC_OCC_SPRD,&empath_scom_data);
        if (rc)
        {
            break;
        }
        o_data->droop.v_droop_small = (uint32_t)empath_scom_data;

        // Large Droop Present
        rc = getscom(empathSelect, PC_OCC_SPRD,&empath_scom_data);
        if (rc)
        {
            break;
        }
        o_data->droop.v_droop_large = (uint32_t)empath_scom_data;

        // MMA Activer
        rc = getscom(empathSelect, PC_OCC_SPRD,&empath_scom_data);
        if (rc)
        {
            break;
        }
        o_data->droop.mma_active = (uint32_t)empath_scom_data;

        // TOD value
        rc = getscom_abs(TOD_VALUE_REG,&empath_scom_data);
        if (rc)
        {
            break;
        }
        o_data->tod_2mhz = (uint32_t)(empath_scom_data >> 8); //[24..56]

        // STOP_STATE_HIST_OCC_REG TODO 213673 P10 exist?
        //rc = getscom(empathSelect, STOP_STATE_HIST_OCC_REG, &empath_scom_data);
        //if (rc)
        //{
        //    break;
        //}
        //o_data->stop_state_hist = empath_scom_data;

        o_data->empathValid = EMPATH_VALID;
    } while(0);

    // Clear masks SIB masks (MSR_SEM)
    // Clear SIBRC and SIMBRCA
    // Restore any SIB masks that may have been on before.
    mtmsr((mfmsr() & ~(MSR_SEM | MSR_SIBRC | MSR_SIBRCA))
          | (org_sem & MSR_SEM));

    return rc;
}

uint32_t get_core_droop_sensors(uint32_t i_core,
                                DdsData* o_data)
{
    //DPLL_ECHAR offset 0x01060058(PAU) 0x01060158(NEST)
    //[1:3] DATA (000b,100b,110b, 111b are valid, otherwise discard)
    //[5:7] MIN_DATA
    //[9:11] MAX_DATA

    uint32_t rc = 0;
    uint32_t coreSelect = CHIPLET_QUAD_BASE((i_core / CORES_PER_QUAD)) +
                          CORE_REGION(i_core);

    cpms_sdsr_t sdsr;

    o_data->value = 0;

    rc = getscom(coreSelect, CPMS_SDSR, &(sdsr.value));

    if(rc == 0)
    {
        uint32_t data = sdsr.fields.data;
        o_data->fields.dds_reading = data;
        if(data <= 24)
        {
            o_data->fields.dds_valid = 1;
        }
        o_data->fields.dds_min  = sdsr.fields.data_min;
        o_data->fields.dds_min_valid = 1;

        data = sdsr.fields.data_max;
        o_data->fields.dds_max  = data;
        if(data <= 24)
        {
            o_data->fields.dds_max_valid = 1;
        }
    }
    return rc;
}
