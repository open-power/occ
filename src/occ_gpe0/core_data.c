/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe0/core_data.c $                                    */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2017                        */
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
#include "cme_register_addresses.h"
#include "pk.h"

#define CME_VDSR_BASE (CME_SCOM_VDSR & 0x00ffffff)

// Global variables
uint32_t g_vdm_cache_large_droop_count[MAX_NUM_QUADS]__attribute__((section (".sbss")));
uint32_t g_vdm_core_small_droop_count[MAX_NUM_CORES]__attribute__((section (".sbss")));

uint32_t get_core_data(uint32_t i_core,
                       CoreData* o_data)
{
    uint32_t rc = 0;
    uint32_t size = sizeof(CoreData) / sizeof(uint64_t);
    uint64_t* ptr = (uint64_t*)o_data;
    uint32_t coreSelect = CHIPLET_CORE_ID(i_core);
    uint32_t quadSelect = CHIPLET_CACHE_ID((i_core / CORES_PER_QUAD));
    uint64_t value64 = 0;

    uint32_t i,idx;

    for(i = 0; i < size; ++i)
    {
        ptr[i] = 0;
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

        rc = getscom(quadSelect,THERM_DTS_RESULT, &(dts_scom_data.value));
        if (rc)
        {
            break;
        }

        // Store the quad DTS readings
        o_data->dts.cache[0].result = dts_scom_data.half_words.reading[0];
        o_data->dts.cache[1].result = dts_scom_data.half_words.reading[1];

        rc = getscom(coreSelect, THERM_DTS_RESULT, &(dts_scom_data.value));
        if (rc)
        {
            break;
        }

        o_data->dts.core[0].result = dts_scom_data.half_words.reading[0];
        o_data->dts.core[1].result = dts_scom_data.half_words.reading[1];

        // =============
        // DROOP
        // =============
        // Read Droop events. Event bit == 0 indicates event occurred.
        // Side effect of read: event bits are reset to 1 (no event) in hw
        // Only quad large droop and core small drop events are of interest
        rc = getscom(quadSelect, CME_VDSR_BASE, &value64);
        if (rc)
        {
            // DROOP events are not critial. Leave event counts as zero
            // and continue.
            PK_TRACE("Could not read droop events! rc = %d",rc);
            rc = 0;
        }
        else // update droop event counts
        {

            if((value64 & CACHE_VDM_LARGE_DROOP) == 0)
            {
                ++g_vdm_cache_large_droop_count[i_core / CORES_PER_QUAD];
            }

            idx = (i_core / CORES_PER_QUAD) * CORES_PER_QUAD;

            if((value64 & CORE0_VDM_SMALL_DROOP) == 0)
            {
                ++g_vdm_core_small_droop_count[idx];
            }

            if((value64 & CORE1_VDM_SMALL_DROOP) == 0)
            {
                ++g_vdm_core_small_droop_count[idx+1];
            }

            if((value64 & CORE2_VDM_SMALL_DROOP) == 0)
            {
                ++g_vdm_core_small_droop_count[idx+2];
            }

            if((value64 & CORE3_VDM_SMALL_DROOP) == 0)
            {
                ++g_vdm_core_small_droop_count[idx+3];
            }

            // return the event status for the requested core and
            // corresponding quad.
            // Clear the counter for only the droop events returned.
            if(g_vdm_cache_large_droop_count[i_core / CORES_PER_QUAD] != 0)
            {
                o_data->droop.cache_large_event = 1;
                g_vdm_cache_large_droop_count[i_core / CORES_PER_QUAD] = 0;
            }

            if(g_vdm_core_small_droop_count[i_core] != 0)
            {
                o_data->droop.core_small_event = 1;
                g_vdm_core_small_droop_count[i_core] = 0;
            }
        }

        // =============
        // EMPATH
        // =============
        // Send command to select which emmpath counter to read
        do
        {
            uint64_t empath_scom_data = CORE_RAW_CYCLES;
            rc = putscom(coreSelect, PC_OCC_SPRC, empath_scom_data);
            if (rc)
            {
                break;
            }

            // Read counters.
            //  Counter selected auto increments to the next counter after each read.

            //CORE_RAW_CYCLES
            rc = getscom(coreSelect, PC_OCC_SPRD, &empath_scom_data);
            if (rc)
            {
                break;
            }
            o_data->empath.raw_cycles = (uint32_t)empath_scom_data;

            //CORE_RUN_CYCLES
            rc = getscom(coreSelect, PC_OCC_SPRD, &empath_scom_data);
            if (rc)
            {
                break;
            }
            o_data->empath.run_cycles = (uint32_t)empath_scom_data;

            //CORE_WORKRATE_BUSY
            rc = getscom(coreSelect, PC_OCC_SPRD,&empath_scom_data);
            if (rc)
            {
                break;
            }
            o_data->empath.freq_sens_busy = (uint32_t)empath_scom_data;

            //CORE_WORKRATE_FINISH
            rc = getscom(coreSelect, PC_OCC_SPRD,&empath_scom_data);
            if (rc)
            {
                break;
            }
            o_data->empath.freq_sens_finish = (uint32_t)empath_scom_data;

            //CORE_MEM_HIER_A_LATENCY
            rc = getscom(coreSelect, PC_OCC_SPRD,&empath_scom_data);
            if (rc)
            {
                break;
            }
            o_data->empath.mem_latency_a = (uint32_t)empath_scom_data;

            //CORE_MEM_HIER_B_LATENCY
            rc = getscom(coreSelect, PC_OCC_SPRD,&empath_scom_data);
            if (rc)
            {
                break;
            }
            o_data->empath.mem_latency_b = (uint32_t)empath_scom_data;

            //CORE_MEM_HIER_C_ACCESS
            rc = getscom(coreSelect, PC_OCC_SPRD,&empath_scom_data);
            if (rc)
            {
                break;
            }
            o_data->empath.mem_access_c = (uint32_t)empath_scom_data;

            int thread = 0;

            for( ; thread < EMPATH_CORE_THREADS; ++thread )
            {
                // THREAD_RUN_CYCLES
                rc = getscom(coreSelect, PC_OCC_SPRD,&empath_scom_data);
                if (rc)
                {
                    break;
                }
                o_data->per_thread[thread].run_cycles = (uint32_t)empath_scom_data;

                // THREAD_INST_DISP_UTIL
                rc = getscom(coreSelect, PC_OCC_SPRD,&empath_scom_data);
                if (rc)
                {
                    break;
                }
                o_data->per_thread[thread].dispatch = (uint32_t)empath_scom_data;

                // THREAD_INST_COMP_UTIL
                rc = getscom(coreSelect, PC_OCC_SPRD,&empath_scom_data);
                if (rc)
                {
                    break;
                }
                o_data->per_thread[thread].completion = (uint32_t)empath_scom_data;

                // THREAD_MEM_HEIR_C_ACCESS
                rc = getscom(coreSelect, PC_OCC_SPRD,&empath_scom_data);
                if (rc)
                {
                    break;
                }
                o_data->per_thread[thread].mem_c = (uint32_t)empath_scom_data;
            }
            if (rc)
            {
                break;
            }

            //IFU_THROTTLE_BLOCK_FETCH
            rc = getscom(coreSelect, PC_OCC_SPRD,&empath_scom_data);
            if (rc)
            {
                break;
            }
            o_data->throttle.ifu_throttle = (uint32_t)empath_scom_data;

            //IFU_THROTTLE_ACTIVE
            rc = getscom(coreSelect, PC_OCC_SPRD,&empath_scom_data);
            if (rc)
            {
                break;
            }
            o_data->throttle.ifu_active = (uint32_t)empath_scom_data;

            //VOLT_DROOP_THROTTLE_ACTIVE
            rc = getscom(coreSelect, PC_OCC_SPRD,&empath_scom_data);
            if (rc)
            {
                break;
            }
            o_data->throttle.v_droop = (uint32_t)empath_scom_data;

            // TOD value
            rc = getscom_abs(TOD_VALUE_REG,&empath_scom_data);
            if (rc)
            {
                break;
            }
            o_data->empath.tod_2mhz = (uint32_t)(empath_scom_data >> 8); //[24..56]

            // STOP_STATE_HIST_OCC_REG
            rc = getscom(coreSelect, STOP_STATE_HIST_OCC_REG, &empath_scom_data);
            if (rc)
            {
                break;
            }
            o_data->stop_state_hist = empath_scom_data;

            o_data->empathValid = EMPATH_VALID;
        } while(0); // EMPATH

        if (rc)
        {
            // Work-around for HW problem. See SW407201
            // If ADDRESS_ERROR then perform a SCOM write of all zeros to
            // 2n010800 where n is the core number. Ignore ADDRESS_ERROR
            // returned.  EMPATH_VALID will be left unset to indicate the
            // EMPATH data is not valid, however, return SUCCESS to indicate
            // the DTS data is good.
            if(rc == SIBRC_ADDRESS_ERROR)
            {
                uint64_t zeros = 0;
                putscom(coreSelect,WORKAROUND_SCOM_ADDRESS, zeros);
                rc = 0;
            }
        }

    } while(0);

    // Clear masks SIB masks (MSR_SEM)
    // Clear SIBRC and SIMBRCA
    // Restore any SIB masks that may have been on before.
    mtmsr((mfmsr() & ~(MSR_SEM | MSR_SIBRC | MSR_SIBRCA))
          | (org_sem & MSR_SEM));

    return rc;
}
