/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/thread/chom.c $                                   */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2021                        */
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

#include <occ_common.h>
#include <comp_ids.h>
#include <sensor.h>
#include <trac.h>
#include <chom.h>
#include <mode.h>
#include <dcom.h>
#include <amec_sys.h>
#include <occ_service_codes.h>
#include <thread_service_codes.h>

extern amec_sys_t g_amec_sys;

#define CHOM_SENSOR_DATA(index,data) [index] = data

// chom timer
uint32_t g_chom_gen_periodic_log_timer;

// track which power mode has been during the polling period
uint8_t  g_chom_pwr_modes[NUM_CHOM_MODES];

// force immediate chom log flag
uint8_t  g_chom_force;

// indicates that chom data has been reset
bool    g_chom_reset = FALSE;

// chom data log
ChomLogData_t   g_chom_log;
ChomLogData_t * g_chom = &g_chom_log;

// Make sure that the size of chom log is less than allowed
// otherwise cause error on the compile.
STATIC_ASSERT( sizeof(ChomLogData_t) > CHOM_LOG_DATA_MAX );

// Chom Sensors Table
//   Some of the chom sensors need multiple mini-sensor to calculate
//   the max, summation of temperature or bandwidth
//   mark those mini-sensor "NULL" and will be updated
//   from "chom_update_sensors()"

const uint16_t * g_chom_sensor_table[CHOM_NUM_OF_SENSORS] =
{   // Node total power (DC)
    &g_amec_sys.sys.pwrsys.sample,
    // APSS sensors 1 per channel (16 total)
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    // Processor frequency
    &G_dcom_slv_outbox_rx[0].freqa,
    &G_dcom_slv_outbox_rx[1].freqa,
    &G_dcom_slv_outbox_rx[2].freqa,
    &G_dcom_slv_outbox_rx[3].freqa,
    // Processor utilization sensor
    &G_dcom_slv_outbox_rx[0].util,
    &G_dcom_slv_outbox_rx[1].util,
    &G_dcom_slv_outbox_rx[2].util,
    &G_dcom_slv_outbox_rx[3].util,
    // Processor temperature sensors
    &G_dcom_slv_outbox_rx[0].tempprocthermal,
    &G_dcom_slv_outbox_rx[1].tempprocthermal,
    &G_dcom_slv_outbox_rx[2].tempprocthermal,
    &G_dcom_slv_outbox_rx[3].tempprocthermal,
    // Processor IO temperature sensors
    &G_dcom_slv_outbox_rx[0].tempprociothermal,
    &G_dcom_slv_outbox_rx[1].tempprociothermal,
    &G_dcom_slv_outbox_rx[2].tempprociothermal,
    &G_dcom_slv_outbox_rx[3].tempprociothermal,
    // Membuf temperature sensors
    &G_dcom_slv_outbox_rx[0].tempmembufthrm,
    &G_dcom_slv_outbox_rx[1].tempmembufthrm,
    &G_dcom_slv_outbox_rx[2].tempmembufthrm,
    &G_dcom_slv_outbox_rx[3].tempmembufthrm,
    // DIMM temperature sensors
    &G_dcom_slv_outbox_rx[0].tempdimmthrm,
    &G_dcom_slv_outbox_rx[1].tempdimmthrm,
    &G_dcom_slv_outbox_rx[2].tempdimmthrm,
    &G_dcom_slv_outbox_rx[3].tempdimmthrm,
    // temperature sensors for mem controller + DRAM
    &G_dcom_slv_outbox_rx[0].tempmcdimmthrm,
    &G_dcom_slv_outbox_rx[1].tempmcdimmthrm,
    &G_dcom_slv_outbox_rx[2].tempmcdimmthrm,
    &G_dcom_slv_outbox_rx[3].tempmcdimmthrm,
    // temperature sensors for PMIC
    &G_dcom_slv_outbox_rx[0].temppmicthrm,
    &G_dcom_slv_outbox_rx[1].temppmicthrm,
    &G_dcom_slv_outbox_rx[2].temppmicthrm,
    &G_dcom_slv_outbox_rx[3].temppmicthrm,
    // temperature sensors for external mem controller sensor
    &G_dcom_slv_outbox_rx[0].tempmcextthrm,
    &G_dcom_slv_outbox_rx[1].tempmcextthrm,
    &G_dcom_slv_outbox_rx[2].tempmcextthrm,
    &G_dcom_slv_outbox_rx[3].tempmcextthrm,
    // VRM VDD temperatures
    &G_dcom_slv_outbox_rx[0].tempvdd,
    &G_dcom_slv_outbox_rx[1].tempvdd,
    &G_dcom_slv_outbox_rx[2].tempvdd,
    &G_dcom_slv_outbox_rx[3].tempvdd,
    // Instructions per second sensor
    NULL,
    // Memory bandwidth for process memory controller
    // P0M0 ~ P0M15
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    // P1M0 ~ P1M15
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    // P2M0 ~ P2M15
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    // P3M0 ~ P3M15
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    // WOF related data
    &G_dcom_slv_outbox_rx[0].ddsAvg,
    &G_dcom_slv_outbox_rx[1].ddsAvg,
    &G_dcom_slv_outbox_rx[2].ddsAvg,
    &G_dcom_slv_outbox_rx[3].ddsAvg,
    &G_dcom_slv_outbox_rx[0].ddsMin,
    &G_dcom_slv_outbox_rx[1].ddsMin,
    &G_dcom_slv_outbox_rx[2].ddsMin,
    &G_dcom_slv_outbox_rx[3].ddsMin,
    &G_dcom_slv_outbox_rx[0].curVdd,
    &G_dcom_slv_outbox_rx[1].curVdd,
    &G_dcom_slv_outbox_rx[2].curVdd,
    &G_dcom_slv_outbox_rx[3].curVdd,
    &G_dcom_slv_outbox_rx[0].ceffRatioVdd,
    &G_dcom_slv_outbox_rx[1].ceffRatioVdd,
    &G_dcom_slv_outbox_rx[2].ceffRatioVdd,
    &G_dcom_slv_outbox_rx[3].ceffRatioVdd,
    &G_dcom_slv_outbox_rx[0].uvAvg,
    &G_dcom_slv_outbox_rx[1].uvAvg,
    &G_dcom_slv_outbox_rx[2].uvAvg,
    &G_dcom_slv_outbox_rx[3].uvAvg,
    &G_dcom_slv_outbox_rx[0].ovAvg,
    &G_dcom_slv_outbox_rx[1].ovAvg,
    &G_dcom_slv_outbox_rx[2].ovAvg,
    &G_dcom_slv_outbox_rx[3].ovAvg,
};


// Function Specification
//
// Name: chom_data_init
//
// Description: Init chom data
//
// End Function Specification
void chom_data_init()
{
    // chom data reset
    chom_data_reset();
    // update eyecatcher
    memcpy(&g_chom->nodeData.eyecatcher, "CHOM", 0x04);
    // update chom version
    g_chom->nodeData.version = CHOM_VERSION;
    // update num of sensors
    g_chom->nodeData.numSensors = CHOM_NUM_OF_SENSORS;
}


// Function Specification
//
// Name: chom_data_reset
//
// Description: Reset chom data
//
// End Function Specification
void chom_data_reset()
{
    // reset chom data log
    memset(g_chom, 0x00, sizeof(*g_chom));
    // reset chom modes in log
    memset(g_chom_pwr_modes, 0x00, sizeof(g_chom_pwr_modes));
    // reset chom timer
    g_chom_gen_periodic_log_timer = 0;
    // reset flag
    g_chom_force = FALSE;
    // reset static data
    g_chom_reset = TRUE;
}


// Function Specification
//
// Name: chom_update_sensors
//
// Description: Update chom sensor data
//
// End Function Specification
void chom_update_sensors()
{
    uint32_t l_mips = 0;
    uint16_t l_mem_rw = 0;
    uint16_t l_sample = 0;

    static uint32_t L_memBWNumSamples[NUM_CHOM_MODES][MAX_NUM_MEM_CONTROLLERS] = {{0}};

    // Use FMAX as default
    static uint32_t * L_curNumSamplePtr = L_memBWNumSamples[CHOM_MODE_FMAX];

    if(TRUE == g_chom_reset)
    {
        memset(L_memBWNumSamples, 0, sizeof(L_memBWNumSamples));
        g_chom_reset = FALSE;
    }

    uint16_t i = 0, j = 0;

    // Is the current mode different than previous poll
    if (g_chom->nodeData.curPwrMode != CURRENT_MODE())
    {
        // update current power mode
        g_chom->nodeData.curPwrMode = CURRENT_MODE();
        // copy the current mode data to previous mode
        memcpy(&g_chom->sensorData[1], &g_chom->sensorData[0], sizeof(ChomSensorData_t));
        // reset all fields in the current chom sensors
        memset(&g_chom->sensorData[0], 0x00, sizeof(ChomSensorData_t));
        // update power saving mode of sensor data
        g_chom->sensorData[0].pwrMode.mode = g_chom->nodeData.curPwrMode;

        // initial min value of chom sensors
        for (i=0 ; i<CHOM_NUM_OF_SENSORS ; i++)
        {
            g_chom->sensorData[0].sensor[i].sampleMin = 0xffff;
        }

        // update the number of power modes during the polling period
        switch (g_chom->nodeData.curPwrMode)
        {
            case OCC_MODE_DISABLED:
                g_chom_pwr_modes[CHOM_MODE_DISABLED] = 1;
                L_curNumSamplePtr = L_memBWNumSamples[CHOM_MODE_DISABLED];
                break;

            case OCC_MODE_PWRSAVE:
                g_chom_pwr_modes[CHOM_MODE_SPS] = 1;
                L_curNumSamplePtr = L_memBWNumSamples[CHOM_MODE_SPS];
                break;

            case OCC_MODE_STATIC_FREQ_POINT:
                g_chom_pwr_modes[CHOM_MODE_SFP] = 1;
                L_curNumSamplePtr = L_memBWNumSamples[CHOM_MODE_SFP];
                break;

            case OCC_MODE_FFO:
                g_chom_pwr_modes[CHOM_MODE_FFO] = 1;
                L_curNumSamplePtr = L_memBWNumSamples[CHOM_MODE_FFO];
                break;

            case OCC_MODE_DYN_PERF:
                g_chom_pwr_modes[CHOM_MODE_DYN_PERF] = 1;
                L_curNumSamplePtr = L_memBWNumSamples[CHOM_MODE_DYN_PERF];
                break;

            case OCC_MODE_MAX_PERF:
                g_chom_pwr_modes[CHOM_MODE_MAX_PERF] = 1;
                L_curNumSamplePtr = L_memBWNumSamples[CHOM_MODE_MAX_PERF];
                break;

            case OCC_MODE_FMAX:
                g_chom_pwr_modes[CHOM_MODE_FMAX] = 1;
                L_curNumSamplePtr = L_memBWNumSamples[CHOM_MODE_FMAX];
                break;

            default:
                TRAC_INFO("chom_update_sensors: Cannot record chom data for mode 0x%02X",
                          g_chom->sensorData[0].pwrMode.mode);
                L_curNumSamplePtr = L_memBWNumSamples[CHOM_MODE_FMAX];
                break;
        }
    }

    // update number of samples
    g_chom->sensorData[0].pwrMode.numOfSamples++;

    // update APSS
    uint16_t l_apss_idx = CHOMPWRAPSSCH0;
    uint16_t l_current_channel = 0;
    for( i = 0; i < MAX_APSS_ADC_CHANNELS; i++ )
    {
        // Transfer data from AMEC sensor to CHOM sensor
        l_current_channel = AMECSENSOR_ARRAY_PTR( PWRAPSSCH00, i )->sample;
       // TRAC_INFO("channel %d reading: %d", i, l_current_channel);
        g_chom->sensorData[0].sensor[l_apss_idx].sample = l_current_channel;

        // Send the corresponding function ID in the node data
        g_chom->nodeData.channelFuncIds[i] =
                         G_apss_ch_to_function[i];
        l_apss_idx++;
    }


    // update MIPS
    uint16_t l_mips_count = 0;
    uint16_t l_mem_idx = CHOMBWP0M0;

    // Loop through OCCs updating chom sensors
    for ( i = 0; i < CHOM_MAX_OCCS; i++ )
    {
        // count MIPS
        if ( 0 != G_dcom_slv_outbox_rx[i].ips4msp0 )
        {
            l_mips += G_dcom_slv_outbox_rx[i].ips4msp0;
            l_mips_count++;
        }

        // update memory bandwidth
        for ( j = 0; j < MAX_NUM_MEM_CONTROLLERS; j++)
        {
            l_mem_rw = G_dcom_slv_outbox_rx[i].mrd[j] +
                       G_dcom_slv_outbox_rx[i].mwr[j];

            // If l_mem_rw == 0, do not add to sensor
            if(l_mem_rw != 0)
            {
                g_chom->sensorData[0].sensor[l_mem_idx].sample = l_mem_rw;
                L_curNumSamplePtr[j]++;

                // Calculate the averages/min/max for the memory bandwidth sensors
                l_sample = g_chom->sensorData[0].sensor[l_mem_idx].sample;

                if (g_chom->sensorData[0].sensor[l_mem_idx].sampleMin > l_sample)
                {
                    g_chom->sensorData[0].sensor[l_mem_idx].sampleMin = l_sample;
                }
                if (g_chom->sensorData[0].sensor[l_mem_idx].sampleMax < l_sample)
                {
                    g_chom->sensorData[0].sensor[l_mem_idx].sampleMax = l_sample;
                }

                g_chom->sensorData[0].sensor[l_mem_idx].accumulator += l_sample;
                g_chom->sensorData[0].sensor[l_mem_idx].average =
                                 (g_chom->sensorData[0].sensor[l_mem_idx].accumulator /
                                  L_curNumSamplePtr[j]);
            }
            l_mem_idx++;
        }
    }

    // Update MIPS
    if (l_mips_count != 0)
    {
        g_chom->sensorData[0].sensor[CHOMIPS].sample = (l_mips/l_mips_count);
    }

    // loop through all sensors and update data from mini-sensors
    for (i = 0 ; i<CHOM_NUM_OF_SENSORS ; i++)
    {
        // Skip memory bandwidth controllers since handled above
        if( i == CHOMBWP0M0 )
        {
            i += (MAX_NUM_MEMORY_SENSORS-1);
            continue;
        }

        // update sample, min, max, average sensor data
        if (NULL != g_chom_sensor_table[i])
        {
            // directly mapping to mini-sensor
            g_chom->sensorData[0].sensor[i].sample = *g_chom_sensor_table[i];
        }

        l_sample = g_chom->sensorData[0].sensor[i].sample;

        if (g_chom->sensorData[0].sensor[i].sampleMin > l_sample)
        {
            g_chom->sensorData[0].sensor[i].sampleMin = l_sample;
        }
        if (g_chom->sensorData[0].sensor[i].sampleMax < l_sample)
        {
            g_chom->sensorData[0].sensor[i].sampleMax = l_sample;
        }
        g_chom->sensorData[0].sensor[i].accumulator += l_sample;
        g_chom->sensorData[0].sensor[i].average = (g_chom->sensorData[0].sensor[i].accumulator/
                                                   g_chom->sensorData[0].pwrMode.numOfSamples);
    }

    // Collect the error history data and fclip history
    int proc_idx = 0, errh_idx = 0, slv_idx = 0, entry_idx = 0;

    // get the master proc index
    uint8_t master_id = G_pbax_id.chip_id;

    // Iterate through procs adding node data
    for( proc_idx = 0; proc_idx < CHOM_MAX_OCCS; proc_idx++ )
    {
        g_chom->nodeData.ddsMinCore[proc_idx] = G_dcom_slv_outbox_rx[proc_idx].ddsMinCore;

        // If we are on the master proc, skip ERRH and Fclip since it is already
        // present in the call home log
        if( proc_idx == master_id )
        {
            continue;
        }
        else
        {
            // Add Fclip history
            g_chom->nodeData.fClipHist[slv_idx] =
                G_dcom_slv_outbox_rx[proc_idx].fClipHist;

            // Iterate through each proc's error history counts
            for( errh_idx = 0; errh_idx < DCOM_MAX_ERRH_ENTRIES; errh_idx++)
            {
                // If the entry id is 0, we have reached the end of error history
                // counts for this proc
                // If entry_idx is 4, we have reached our limit of entries to collect
                if((G_dcom_slv_outbox_rx[proc_idx].errhCount[errh_idx].error_id == 0) ||
                   (entry_idx >= CHOM_MAX_ERRH_ENTRIES))
                {
                    break;
                }
                else
                {
                    // Add the error history to the chom data
                    g_chom->nodeData.errhCounts[slv_idx][entry_idx] =
                        G_dcom_slv_outbox_rx[proc_idx].errhCount[errh_idx];
                    entry_idx++;
                }
            }

            // next slave
            slv_idx++;
            entry_idx = 0;
        }
    }
}



// Function Specification
//
// Name: chom_gen_periodic_log
//
// Description: Create call home data logs
//
// End Function Specification
void chom_gen_periodic_log()
{
    uint8_t        i                = 0;
    errlHndl_t     l_errlHndl       = NULL;

    TRAC_INFO("Enter chom_gen_periodic_log");
    TRAC_INFO("chom size = %d", sizeof(*g_chom));
    // update total time
    g_chom->nodeData.totalTime = g_chom_gen_periodic_log_timer;

    // update the number of different power modes
    for (i=0 ; i<NUM_CHOM_MODES ; i++)
    {
        if (g_chom_pwr_modes[i] == 1)
        {
            g_chom->nodeData.modeInLog++;
        }
    }

    /* @
     * @errortype
     * @moduleid    THRD_MID_GEN_CALLHOME_LOG
     * @reasoncode  GEN_CALLHOME_LOG
     * @userdata1   0
     * @userdata2   0
     * @userdata4   OCC_NO_EXTENDED_RC
     * @devdesc     Generating periodic call-home log, which contains critical power/thermal/performance data since the last log was generated (or the last OCC reset)
     */
    l_errlHndl = createErrl(
            THRD_MID_GEN_CALLHOME_LOG,    // modId
            GEN_CALLHOME_LOG,             // reasoncode
            OCC_NO_EXTENDED_RC,           // Extended reason code
            ERRL_SEV_CALLHOME_DATA,       // Severity; will be changed to informational in errl.c
            NULL,                         // Trace Buf
            0,                            // Trace Size
            0,                            // userdata1
            0                             // userdata2
            );

    // add chom data
    addUsrDtlsToErrl(l_errlHndl,
                     (uint8_t*)g_chom,
                     sizeof(*g_chom),
                     ERRL_STRUCT_VERSION_1,
                     ERRL_USR_DTL_CALLHOME_DATA);

    // commit error log
    if (l_errlHndl != NULL)
    {
        commitErrl(&l_errlHndl);
    }
}


// Function Specification
//
// Name: chom_force_gen_log()
//
// Description: immediate chom log
//
// End Function Specification
void chom_force_gen_log()
{
    g_chom_force = TRUE;
}


// Function Specification
//
// Name: chom_main
//
// Description: Call home data routine
//
// End Function Specification
void chom_main()
{
    MAIN_DBG("CHOM routine processing...");

    // check if this is the first time we run
    if (memcmp(&g_chom->nodeData.eyecatcher, "CHOM", 0x04) != 0)
    {
        chom_data_init();
    }

    // only update chom sensors on active, observing, or characterizing OCCs
    if (IS_OCC_STATE_ACTIVE()           ||
        IS_OCC_STATE_OBSERVATION()      ||
        IS_OCC_STATE_CHARACTERIZATION())
    {
        chom_update_sensors();
    }

    // increase chom timer
    g_chom_gen_periodic_log_timer++;

    // check if the timer expired
    if ((g_chom_gen_periodic_log_timer >= CHOM_GEN_LOG_PERIODIC_TIME) ||
        (TRUE == g_chom_force))
    {
        chom_gen_periodic_log();
        chom_data_reset();
    }
}
