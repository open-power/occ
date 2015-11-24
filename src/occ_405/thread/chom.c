/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/thread/chom.c $                                       */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2015                        */
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
uint8_t  g_chom_pwr_modes[OCC_INTERNAL_MODE_MAX_NUM]; // Nominal, SPS, DPS, DPS-MP, FFO
// force immediate chom log flag
uint8_t  g_chom_force;
// chom data log
ChomLogData_t   g_chom_log;
ChomLogData_t * g_chom = &g_chom_log;

// Make sure that the size of chom log is less than 3kB
// otherwise cause error on the compile.
STATIC_ASSERT( sizeof(ChomLogData_t) > CHOM_LOG_DATA_MAX );

// Chom Sensors Table
//   Some of the chom sensors need multiple mini-sensor to calculate
//   the max, summstion of temperature or bandwidth
//   mark those mini-sensor "NULL" and will be updated
//   from "chom_update_sensors()"

const uint16_t * g_chom_sensor_table[CHOM_NUM_OF_SENSORS] =
{   // Node total power (DC)
    &g_amec_sys.sys.pwr250us.sample,
    // Socket power
    &g_amec_sys.proc[0].pwr250us.sample,
    &G_dcom_slv_outbox_rx[1].pwr250usp0,
    &G_dcom_slv_outbox_rx[2].pwr250usp0,
    &G_dcom_slv_outbox_rx[3].pwr250usp0,
    &G_dcom_slv_outbox_rx[4].pwr250usp0,
    &G_dcom_slv_outbox_rx[5].pwr250usp0,
    &G_dcom_slv_outbox_rx[6].pwr250usp0,
    &G_dcom_slv_outbox_rx[7].pwr250usp0,
    // Memory power
    &G_dcom_slv_outbox_rx[0].pwr250usmemp0,
    &G_dcom_slv_outbox_rx[1].pwr250usmemp0,
    &G_dcom_slv_outbox_rx[2].pwr250usmemp0,
    &G_dcom_slv_outbox_rx[3].pwr250usmemp0,
    &G_dcom_slv_outbox_rx[4].pwr250usmemp0,
    &G_dcom_slv_outbox_rx[5].pwr250usmemp0,
    &G_dcom_slv_outbox_rx[6].pwr250usmemp0,
    &G_dcom_slv_outbox_rx[7].pwr250usmemp0,
    // Fan power
    &g_amec_sys.fan.pwr250usfan.sample,
    // Processor frequency
    &G_dcom_slv_outbox_rx[0].freqa2msp0,
    &G_dcom_slv_outbox_rx[1].freqa2msp0,
    &G_dcom_slv_outbox_rx[2].freqa2msp0,
    &G_dcom_slv_outbox_rx[3].freqa2msp0,
    &G_dcom_slv_outbox_rx[4].freqa2msp0,
    &G_dcom_slv_outbox_rx[5].freqa2msp0,
    &G_dcom_slv_outbox_rx[6].freqa2msp0,
    &G_dcom_slv_outbox_rx[7].freqa2msp0,
    // Processor utilization sensor
    &G_dcom_slv_outbox_rx[0].util2msp0,
    &G_dcom_slv_outbox_rx[1].util2msp0,
    &G_dcom_slv_outbox_rx[2].util2msp0,
    &G_dcom_slv_outbox_rx[3].util2msp0,
    &G_dcom_slv_outbox_rx[4].util2msp0,
    &G_dcom_slv_outbox_rx[5].util2msp0,
    &G_dcom_slv_outbox_rx[6].util2msp0,
    &G_dcom_slv_outbox_rx[7].util2msp0,
    // Max Core temperature for all processors in the node
    NULL,
    // Max Centaur temperature for all Centaurs in the node
    NULL,
    // Max Dimm temperature for all Dimms in the node
    NULL,
    // Instructions per second sensor
    NULL,
    // Memory bandwidth for process memory controller
    // P0M0 ~ P0M7
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    // P1M0 ~ P1M7
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    // P2M0 ~ P2M7
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    // P3M0 ~ P3M7
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    // P4M0 ~ P4M7
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    // P5M0 ~ P5M7
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    // P6M0 ~ P6M7
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    // P7M0 ~ P7M7
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
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
    uint16_t l_max_core_temp = 0;
    uint16_t l_max_cent_temp = 0;
    uint16_t l_max_dimm_temp = 0;
    uint16_t l_mips = 0;
    uint16_t l_mem_rw = 0;
    uint16_t l_sample = 0;
    uint16_t i = 0, j = 0, k = 0;

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
            case OCC_MODE_NOMINAL:
                g_chom_pwr_modes[OCC_INTERNAL_MODE_NOM] = 1;
                break;

            case OCC_MODE_PWRSAVE:
                g_chom_pwr_modes[OCC_INTERNAL_MODE_SPS] = 1;
                break;

            case OCC_MODE_DYN_POWER_SAVE:
                g_chom_pwr_modes[OCC_INTERNAL_MODE_DPS] = 1;
                break;

            case OCC_MODE_DYN_POWER_SAVE_FP:
                g_chom_pwr_modes[OCC_INTERNAL_MODE_DPS_MP] = 1;
                break;

            case OCC_MODE_FFO:
                g_chom_pwr_modes[OCC_INTERNAL_MODE_FFO] = 1;
                break;

            default:
                TRAC_INFO("chom_update_sensors: Cannot record chom data for mode 0x%02X",
                          g_chom->sensorData[0].pwrMode.mode);
                break;
        }
    }

    // update number of samples
    g_chom->sensorData[0].pwrMode.numOfSamples++;

    // update chom sensors which has multiple mini-sensor source
    for (i = 0 ; i<MAX_OCCS ; i++)
    {
        if (G_dcom_slv_outbox_rx[i].temp4msp0peak > l_max_core_temp)
        {
            l_max_core_temp = G_dcom_slv_outbox_rx[i].temp4msp0peak;
        }

        if (G_dcom_slv_outbox_rx[i].temp2mscent > l_max_cent_temp)
        {
            l_max_cent_temp = G_dcom_slv_outbox_rx[i].temp2mscent;
        }

        if (G_dcom_slv_outbox_rx[i].temp2msdimm > l_max_dimm_temp)
        {
            l_max_dimm_temp = G_dcom_slv_outbox_rx[i].temp2msdimm;
        }
    }
    g_chom->sensorData[0].sensor[CHOMTEMPPROC].sample = l_max_core_temp;
    g_chom->sensorData[0].sensor[CHOMTEMPCENT].sample = l_max_cent_temp;
    g_chom->sensorData[0].sensor[CHOMTEMPDIMM].sample = l_max_dimm_temp;

    // update MIPS
    k = 0;
    for (i=0 ; i<MAX_OCCS ; i++)
    {
        if (0 != G_dcom_slv_outbox_rx[i].ips2msp0)
        {
            l_mips += G_dcom_slv_outbox_rx[i].ips2msp0;
            k++;
        }
    }
    if (k != 0)
    {
        g_chom->sensorData[0].sensor[CHOMIPS].sample = (l_mips/k);
    }

    // update memory bandwidth
    k = CHOMBWP0M0;
    for (i=0 ; i<MAX_OCCS ; i++)
    {
        for (j=0 ; j<MAX_NUM_MEM_CONTROLLERS ; j++)
        {
            // TODO: Need to take a snapshot of the "max" field for that sensor and
            // redefine the mini-sensor information that we pass back to the Master
            l_mem_rw = G_dcom_slv_outbox_rx[i].mrd2msp0mx[j]+G_dcom_slv_outbox_rx[i].mwr2msp0mx[j];
            g_chom->sensorData[0].sensor[k].sample = l_mem_rw;
            k++;
        }
    }

    // loop through all sensors and update data from mini-sensors
    for (i = 0 ; i<CHOM_NUM_OF_SENSORS ; i++)
    {   // update sample, min, max, average sensor data
        if (NULL != g_chom_sensor_table[i])
        {
            //Report all sensors listed in g_chom_sensor_table, unless
            //we have a Murano (DCM), then we only report the power from the
            //OCC_DCM_MASTER occ. The DCM occ pairs (OCC_DCM_MASTER,OCC_DCM_SLAVE) are
            //((0,1),(2,3),(4,5),(6,7)).
            if( ((i >= CHOMPWRS0) && (i <= CHOMPWRS7)) &&
                (CFAM_CHIP_TYPE_MURANO == cfam_chip_type()) &&
                (((i-CHOMPWRS0) % 2) == 1) )
            {
                //Do nothing
            }
            else
            {
                // directly mapping to mini-sensor
                g_chom->sensorData[0].sensor[i].sample = *g_chom_sensor_table[i];
            }
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
}


// Function Specification
//
// Name: chom_collect_cpi_data
//
// Description: collect CPI data
//
// End Function Specification
void chom_collect_cpi_data(void)
{
    // TODO
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

    // update total time
    g_chom->nodeData.totalTime = g_chom_gen_periodic_log_timer;

    // update the number of different power modes
    for (i=0 ; i<OCC_INTERNAL_MODE_MAX_NUM ; i++)
    {
        if (g_chom_pwr_modes[i] == 1)
        {
            g_chom->nodeData.modeInLog++;
        }
    }

    // collect CPI data
    chom_collect_cpi_data();

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

    // only update chom sensors on an active OCC
    if (IS_OCC_STATE_ACTIVE())
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
