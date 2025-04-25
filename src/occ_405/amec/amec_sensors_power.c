/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/amec/amec_sensors_power.c $                       */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2025                        */
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


/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <occ_common.h>
#include <ssx.h>
#include <errl.h>               // Error logging
#include "sensor.h"
#include "rtls.h"
#include "occ_sys_config.h"
#include "occ_service_codes.h"  // for SSX_GENERIC_FAILURE
#include "dcom.h"
#include "proc_data.h"
#include "amec_smh.h"
#include "amec_slave_smh.h"
#include <trac.h>
#include "amec_sys.h"
#include "sensor_enum.h"
#include "amec_service_codes.h"
#include <amec_sensors_power.h>
#include "amec_oversub.h"
#include "avsbus.h"
#include <pstates_occ.H>
#include <wof.h>
#include <cmdh_fsp_cmds.h>      // For G_apss_ch_to_function
//*************************************************************************/
// Defines/Enums
//*************************************************************************/

// For Acceleration Factor (AF) calculation
#define NUM_TICKS_AF_CALC 10000   // 5s in 500us ticks
#define NUM_SAMPLES_VSENSE_AVG 16 // 8ms same time scale TEMPPROCTHRM updated
#define MIN_AFT 2
#define MAX_AFT 43
#define MIN_XV 93
#define NUM_XV 18
#define MAX_XV (MIN_XV + NUM_XV - 1) // 110
uint16_t G_AFv_lookup[NUM_XV] = { 1,      // AFv for MIN_XV
                                  2,
                                  2,
                                  4,
                                  6,
                                  9,
                                  13,
                                  20,
                                  31,
                                  46,
                                  69,
                                  103,
                                  154,
                                  228,
                                  337,
                                  495,
                                  726,
                                  1061 }; // AFv for MAX_XV

/******************************************************************************/
/* Globals                                                                    */
/******************************************************************************/
// This holds the converted ADC Reads
uint32_t G_lastValidAdcValue[MAX_APSS_ADC_CHANNELS] = {0};

// Indicates if we have determined GPU presence
bool G_gpu_config_done = FALSE;


// Bitmap of GPUs present
uint32_t G_first_proc_gpu_config = 0;
uint32_t G_first_sys_gpu_config = 0;
uint32_t G_first_num_gpus_sys = 0;
uint32_t G_curr_proc_gpu_config = 0;
uint32_t G_curr_sys_gpu_config = 0;
uint32_t G_curr_num_gpus_sys = 0;

// There are only MAX_APSS_ADC_CHANNELS channels.  Therefore if the channel value
// is greater then the MAX, then there was no channel associated with the function id.
#define ADC_CONVERTED_VALUE(i_chan) \
    ((i_chan < MAX_APSS_ADC_CHANNELS) ? G_lastValidAdcValue[i_chan] : 0)

extern bool    G_gpu_monitoring_allowed;
extern uint8_t G_occ_interrupt_type;
extern PWR_READING_TYPE  G_pwr_reading_type;
extern bool    G_apss_present;
extern OCCPstateParmBlock_t G_oppb;
extern task_t G_task_table[TASK_END];

extern uint16_t G_configured_mbas;
extern uint8_t G_injected_epow_asserted;
extern amec_sys_t g_amec_sys;

#define VRM_EFFICIENCY_PCT 85
#define CONVERT_VRM_POWER_TO_OUTPUT(input) ((input) * VRM_EFFICIENCY_PCT / 100)

//*************************************************************************/
// Code
//*************************************************************************/

// Function Specification
//
// Name: amec_sensor_from_apss_adc
//
// Description: Calculates sensor from raw ADC value
//
// Thread: RealTime Loop
//
// End Function Specification
uint32_t amec_value_from_apss_adc(uint8_t i_chan)
{
    uint16_t l_raw = 0;
    uint32_t l_temp = 0;
    uint32_t l_gain = 0;
    uint32_t l_offset = 0;

    if(i_chan != SYSCFG_INVALID_ADC_CHAN)
    {
        /*
         * The APSS value is in mV or mA depending on the channel and the raw
         * reading from the APSS must be decoded using the following info:
         *
         * APSS LSB: Vref/4096
         * Full scale: (Vref - LSB) = 0xFFF
         * Midscale: Vref/2 = 0x800
         * Midscale - 1 LSB: Vref/2 - 1 LSB, 0x7FF
         * Zero: 0V, 0x000
         * Any voltage at or above Vref will will result in an ADC channel value
         * of 0xFFF
         *
         * Our APSS has Vref pinned to 2.048 V
         * LSB: 0.5 mV (minimum ADC resolution)
         * Full scale: 2047.5 mV (0xFFF)
         * Midscale: 1024 mV (0x800)
         * Zero: 0V, 0x000
         *
         * To get the right mV reading from the raw APSS data all we need to do
         * is divide the raw 12 bit ADC code by 2.  The same logic applies if
         * the channel is measuring power in mA.
         *
         * If there is an offset it will bein mV or mA depending upon the sensor
         * and it needs to be added or subtracted from the raw value depending
         * on it's signedness.  Negative offsets are stored in 2's complement
         * form.
         *
         * The gain values will be multiplied by 1000 in TMGT before being sent
         * in the sysconfig packet.  Raw gain from the MRW values are in A/V and
         * TMGT will multiply this by 1000 before sending in the sysconfig
         * packet to preserve precision.  That makes the gain units mA/V.
         *
         * To apply the gain multiply the sysconfig value against the converted
         * APSS voltage.
         *
         * Applying the gain to the converted APSS data gives (mV . mA)/V so we
         * divide by 1000 to reduce the result to mA.  This is the unit that
         * is returned to the caller. For example:
         * raw APSS value: 0x800
         * converted value: raw/2 = 1024 mV
         * gain from MRW: 10.00 A/V
         * Converted gain: 10,000 mA/V
         * gain adjusted output: 1024mV * 10,000 mA/V = 10,240,000 (mV . mA)/V
         * Reduced value: adjusted/1000 = 10,240 mA
         *
         * Note that in the case of the remote ground and voltage sense the gain
         * values are in V/V so the returned value is actually in mVs.
         *
         * Max returnable value is 4,294,967,295 mA or approx. 4.3 MA
         */

        // Get ADC Mapping calibration info for this entity.
        l_gain = G_sysConfigData.apss_cal[i_chan].gain;
        l_offset = G_sysConfigData.apss_cal[i_chan].offset;

        // Read Raw Value in mA (divide masked channel data by 2)
        l_raw = (G_dcom_slv_inbox_rx.adc[i_chan] & APSS_12BIT_ADC_MASK)/2;
        // Apply offset and gain
        // Add offset if Raw Value is not zero
        if (l_raw != 0)
        {
            if (l_offset & 0x80000000)
            {
                // Negative offset
                l_raw -= (~l_offset + 1);
            }
            else
            {
                l_raw += l_offset;
            }
        }
        //Check to see if l_raw is negative.  If so, set raw to 0
        if (l_raw & 0x8000)
        {
            l_raw = 0;
        }

        l_temp = ((uint32_t)l_raw * l_gain);
        // Reduce value back to mA or mV
        l_temp /= 1000;
    }

    AMEC_DBG("APSS ADC info: chan=%d, raw=0x%04x, offset=%d, gain=%d calibrated output=%d\n",
             i_chan, l_raw, l_offset, l_gain, l_temp);

    return l_temp;
}

#define ADCMULT_TO_UNITS 1000000
#define ADCMULT_ROUND ADCMULT_TO_UNITS/2
#define ROUND_POWER(value) ((((uint64_t)value) + ADCMULT_ROUND) / ADCMULT_TO_UNITS)
#define NUM_DEBUG_TRACE 4
#define DEBUG_HIGH_READ 900

void calculateProcPower(const uint64_t l_bulk_voltage)
{
    // Determine DCM# (and partner chip)
    uint8_t module = G_pbax_id.chip_id;
    uint8_t partnerChip = G_pbax_id.chip_id;

    bool isSCM = false;
    if (G_sysConfigData.apss_adc_map.sense_12v == 14)
    {
        // McKinley has 12V Sense on Channel 14
        isSCM = true;
    }
    else
    {
        // DCM or ioSCM?

        // TODO: Determine if Blue Ridge is DCM or ioSCM

        // Get chip number for module partner
        module >>= 1;
        if (G_pbax_id.chip_id % 2 == 0)
        {
            ++partnerChip;
        }
        else
        {
            --partnerChip;
        }
    }

    // Get the AVSBUS data (from PGPE)
    uint32_t avsVddChipPower = 0;
    uint32_t avsVcsChipPower = 0;
    uint32_t avsVdnModulePower = 0;
    sensor_t *l_sensor = getSensorByGsid(PWRVDD);   // VDD per CHIP
    if (l_sensor)
        avsVddChipPower = l_sensor->sample;

    l_sensor = getSensorByGsid(PWRVCS);             // VCS per CHIP
    if (l_sensor)
        avsVcsChipPower = l_sensor->sample;

    l_sensor = getSensorByGsid(PWRVDN);             // VDN is per MODULE
    if (l_sensor)
        avsVdnModulePower = l_sensor->sample;

    // Read applicable APSS power data
    uint32_t vcsiopciePower = 0;
    uint16_t vioChipPower = 0;
    uint16_t vpcieChipPower = 0;
    //uint16_t avddPower = 0;
    uint8_t l_channel = 0;
    for (l_channel = 0; l_channel < MAX_APSS_ADC_CHANNELS; l_channel++)
    {
        const uint8_t function_id = G_apss_ch_to_function[l_channel];
        if (((function_id == ADC_VCS_VIO_VPCIE_PROC_0) && (module == 0)) ||
            ((function_id == ADC_VCS_VIO_VPCIE_PROC_1) && (module == 1)) ||
            ((function_id == ADC_VCS_VIO_VPCIE_PROC_2) && (module == 2)) ||
            ((function_id == ADC_VCS_VIO_VPCIE_PROC_3) && (module == 3)))
        {
            // Blue Ridge / Balcones / McKinley
            // APSS reading contains VCS, VIO & VPCIE for each module
            vcsiopciePower = ROUND_POWER(ADC_CONVERTED_VALUE(l_channel) * l_bulk_voltage);
            if (!isSCM)
            {
                // Split modeule power between both chips
                vcsiopciePower /= 2;
            }
        }
        else if (((function_id == ADC_VIO_CURRENT_DCM0) && (module == 0)) ||
                 ((function_id == ADC_VIO_CURRENT_DCM1) && (module == 1)) ||
                 ((function_id == ADC_VIO_CURRENT_DCM2) && (module == 2)) ||
                 ((function_id == ADC_VIO_CURRENT_DCM3) && (module == 3)))
        {
            // Fuji only (VIO per module)

            // TODO: do we want to weight the chips differently?  70:30, etc

            vioChipPower = ROUND_POWER(ADC_CONVERTED_VALUE(l_channel) * l_bulk_voltage) / 2;
        }
        else if (((function_id == ADC_VPCIE_CURRENT_DCM0) && (module == 0)) ||
                 ((function_id == ADC_VPCIE_CURRENT_DCM1) && (module == 1)) ||
                 ((function_id == ADC_VPCIE_CURRENT_DCM2) && (module == 2)) ||
                 ((function_id == ADC_VPCIE_CURRENT_DCM3) && (module == 3)))
        {
            // Fuji only (PCIE per module)

            // TODO: do we want to weight the chips differently?  70:30, etc

            vpcieChipPower = ROUND_POWER(ADC_CONVERTED_VALUE(l_channel) * l_bulk_voltage) / 2;
        }
        else if (function_id == ADC_AVDD_CURRENT_TOTAL)
        {
            // Fuji only (AVDD is shared between ALL DCMs, so chip power is divided by 8)
            //avddPower = ROUND_POWER(ADC_CONVERTED_VALUE(l_channel) * l_bulk_voltage) / 8;
            // TODO: Should it really be /8 or do we need to determine how many

            // TODO: Justin said to use Power Proxy for Fuji also, but left APSS reading to compare
            // TODO: do we want to weight the chips differently?  70:30, etc
        }
    }

    bool convertToOutput = false;
        // APSS readings will need to be converted from input to output power
        if (G_sysConfigData.apss_adc_map.sense_12v == 3)
        {
            // Fuji has 12V Sense on Channel 3
            convertToOutput = true;
        }
        else
        {
            // Blue Ridge/McKinley/Balcones: Calculate Vdn/Vio/Vpcie/AVdd powers

            uint16_t vioModulePower = 0;
            if (g_amec_sys.static_wof_data.xgpe_values_sram_addr)
            {
                // Read VIO Proxy Power (per chip) from XGPE/WOF data
                xgpe_wof_values_t l_XgpeWofValues;
                l_XgpeWofValues.value = in64(g_amec_sys.static_wof_data.xgpe_values_sram_addr);
                vioChipPower = (uint16_t)l_XgpeWofValues.fields.io_power_proxy_w;
                uint16_t vioModulePower = vioChipPower;
                if (!isSCM)
                {
                    // Save VIO power to send to master (to broadcast to partner chip)
                    G_dcom_slv_outbox_tx.avsVIOPower = vioChipPower;
                    // Read partners VIO power to calculate module VIO power
                    const uint16_t partnerVioPower = G_dcom_slv_inbox_rx.avs_vio_power[partnerChip];
                    vioModulePower += partnerVioPower;
                }
            }

            if (vcsiopciePower > 0)
            {
                // Blue Ridge / Balcones / McKinley - Calculate the PCIE power for this chip
                uint32_t avsVcsModulePower = avsVcsChipPower;
                if (!isSCM)
                {
                    // Get VCS power from DCM partner chip
                    const uint16_t partnerVcsPower = G_dcom_slv_outbox_rx[partnerChip].pwrvcs;
                    avsVcsModulePower += partnerVcsPower;
                }

                // VPCIE = (APSS VCS/VIO/VPCIE * RegEfficiency)  (need to convert to output power)
                //       - AVSBUS VCS (for module)
                //       - AVSBUS VDN (for module)
                //       - PROXY POWER (for module)
                int16_t vpcieModulePower = CONVERT_VRM_POWER_TO_OUTPUT(vcsiopciePower) - avsVcsModulePower - avsVdnModulePower - vioModulePower;
                if (vpcieModulePower < 0)
                {
                    vpcieChipPower = 0;
                }
                else
                {
                    vpcieChipPower = vpcieModulePower;
                    if (!isSCM)
                        vpcieChipPower /= 2;
                }
            }
        }

    // Determine chip VDN power (for DCMs)
    uint32_t avsVdnChipPower = avsVdnModulePower;
    if (!isSCM)
    {
        // VDN is per module so split to get the chip power
        avsVdnChipPower /= 2;
    }

    // Calculate processor power for this chip
    uint16_t procPower = avsVddChipPower + avsVcsChipPower + avsVdnChipPower + vioChipPower + vpcieChipPower;
    if (convertToOutput)
    {
        // Fuji uses all power readings from APSS (convert to output power)
        procPower = CONVERT_VRM_POWER_TO_OUTPUT(procPower);
    }
    sensor_update(AMECSENSOR_PTR(PWRPROC), procPower);
}

// Function Specification
//
// Name: amec_update_apss_sensors
//
// Description: Calculates sensor from raw ADC values obtained from APSS
//
// Thread: RealTime Loop
//
// Returns: FALSE if GPIO EPOW was asserted (sensors NOT updated)
//
// End Function Specification
bool amec_update_apss_sensors(void)
{
    uint16_t l_raw = 0;
    uint32_t l_trace32 = 0;
    static uint8_t L_trace_high_channel_reading[MAX_APSS_ADC_CHANNELS] = {0};  // used to threshold trace per channel
    static bool L_trace_time = TRUE;
    bool l_sensors_updated = TRUE;
    static bool L_trace_everest_workaround = TRUE;
    do
    {
        // Need to check to make sure APSS data has been received
        // via slave inbox first
        if (G_slv_inbox_received &&
           (G_pwr_reading_type == PWR_READING_TYPE_APSS) &&
           (0 == G_dcom_slv_inbox_doorbell_rx.apss_recovery_in_progress))
        {
            uint8_t l_proc   = G_pbax_id.chip_id;
            uint32_t temp32  = 0;
            uint64_t temp64  = 0;
            uint8_t  l_channel   = 0;

            // ----------------------------------------------------
            // Convert all ADC Channels immediately
            // ----------------------------------------------------
            for (l_channel = 0; l_channel < MAX_APSS_ADC_CHANNELS; l_channel++)
            {
                // These values returned are gain adjusted. The APSS readings for
                // the remote ground and voltage sense are returned in mVs, all other
                // readings are treated as mAs.
                G_lastValidAdcValue[l_channel] = amec_value_from_apss_adc(l_channel);
            }

            // --------------------------------------------------------------
            // Convert voltages into interim value - this has to happen first
            // --------------------------------------------------------------
            // Calculations involving bulk_voltage must be 64bit so final result
            // does not get truncated (before dividing by ADCMULT_TO_UNITS)
            uint64_t l_bulk_voltage = ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.sense_12v);

            // Everest workaround for not having 12v sense
            // Everest 12v sense is on channel 3 and no other P10 system is on channel 3
            // checking for channel 3 verifies this will only run on Everest
            if( (G_sysConfigData.apss_adc_map.sense_12v == 3) &&
                ((l_bulk_voltage < 9000) || (l_bulk_voltage > 15000)) )
            {
                if(L_trace_everest_workaround)
                {
                    TRAC_IMP("Everest workaround using 12000 for 12V sense not reading %d", (uint16_t)l_bulk_voltage);
                    L_trace_everest_workaround = FALSE;

                    // only need one error log, log on master only
                    if (OCC_MASTER == G_occ_role)
                    {
                        /*
                         * @errortype
                         * @moduleid    AMEC_UPDATE_APSS_SENSORS
                         * @reasoncode  APSS_12V_SENSE_HW_WORKAROUND
                         * @userdata1   0
                         * @userdata2   0
                         * @userdata4   OCC_NO_EXTENDED_RC
                         * @devdesc     Everest APSS 12V sense HW workaround detected
                         */
                        errlHndl_t l_err = createErrl(AMEC_UPDATE_APSS_SENSORS,
                                                      APSS_12V_SENSE_HW_WORKAROUND,
                                                      OCC_NO_EXTENDED_RC,
                                                      ERRL_SEV_INFORMATIONAL,
                                                      NULL,
                                                      DEFAULT_TRACE_SIZE,
                                                      0,
                                                      0);
                        commitErrl(&l_err);
                    }
                }
                l_bulk_voltage = 12000;
            }

            // Update channel sensors for all channels (except voltage sense and gnd)
            for (l_channel = 0; l_channel < MAX_APSS_ADC_CHANNELS; l_channel++)
            {
                if(l_channel == G_sysConfigData.apss_adc_map.current_12v_stby)
                {
                    // Save value of 12V Standby Current (.01A) in a sensor for lab use only
                    temp32 = ADC_CONVERTED_VALUE(l_channel)/100;  // convert mA to .01A
                    sensor_update(AMECSENSOR_PTR(CUR12VSTBY), (uint16_t) temp32);
                }
                else if((l_channel != G_sysConfigData.apss_adc_map.sense_12v) &&
                        (l_channel != G_sysConfigData.apss_adc_map.remote_gnd))
                {
                    temp32 = ROUND_POWER(ADC_CONVERTED_VALUE(l_channel) * l_bulk_voltage);
                    sensor_update(AMECSENSOR_PTR(PWRAPSSCH00 + l_channel), (uint16_t) temp32);
                    if (OCC_MASTER == G_occ_role)
                    {
                        // extra debug traces if the value seems too high, trace from master only
                        if( (l_channel != G_sysConfigData.apss_adc_map.total_current_12v) &&
                            (temp32 >= DEBUG_HIGH_READ) && (L_trace_high_channel_reading[l_channel] < NUM_DEBUG_TRACE) )
                        {
                            L_trace_high_channel_reading[l_channel]++;
                            // Read Raw Value in mA (divide masked channel data by 2)
                            l_trace32 = ((l_channel << 24) | (G_apss_ch_to_function[l_channel] << 16) | (temp32 & 0xFFFF));
                            l_raw = (G_dcom_slv_inbox_rx.adc[l_channel] & APSS_12BIT_ADC_MASK)/2;
                            TRAC_IMP("APSS channel/FuncID/Power[%08X] high??? raw/2=0x%04X, offset=0x%08X, gain=0x%08X",
                                     l_trace32, l_raw, G_sysConfigData.apss_cal[l_channel].offset,
                                     G_sysConfigData.apss_cal[l_channel].gain);
                            TRAC_IMP("ADC converted value[%d mA] 12V bulk voltage[%d mV]",
                                     (uint32_t)G_lastValidAdcValue[l_channel], (uint16_t)l_bulk_voltage);
                        }
                    }
                }
            }
            if (OCC_MASTER == G_occ_role)
            {
                amec_update_apss_gpio();
            }

            // ----------------------------------------------------------
            // Convert Raw Vdd/Vcs/Vio/Vpcie Power from APSS into sensors
            // ----------------------------------------------------------
            // Some sensor values are in Watts so after getting the mA readings we
            // multiply by the bulk voltage (mVs) which requires us to then divide
            // by 1000000 to get W (A.V), ie.
            //  divide by    1000 to get it back to milliUnits (0.001)
            //  divide by   10000 to get it to centiUnits      (0.01)
            //  divide by  100000 to get it to deciUnits       (0.1)
            //  divide by 1000000 to get it to Units           (1)

            //Update channel specific sensors based on saved pairing between function Ids and Channels.

            calculateProcPower(l_bulk_voltage);

            // All readings from APSS come back as milliUnits, so if we want
            // to convert one, we need to
            //  divide by    1 to get it back to milliUnits (0.001)
            //  divide by   10 to get it to centiUnits      (0.01)
            //  divide by  100 to get it to deciUnits       (0.1)
            //  divide by 1000 to get it to Units           (1)

            // ----------------------------------------------------
            // Convert Other Raw Misc Power from APSS into sensors
            // ----------------------------------------------------

            // Save total GPU adapter for this proc
            if (l_proc < MAX_GPU_DOMAINS)
            {
                // GPU0
                temp64 = l_bulk_voltage * ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.gpu[l_proc][0]);

                // GPU1
                temp64 += l_bulk_voltage * ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.gpu[l_proc][1]);

                // GPU2
                temp64 += l_bulk_voltage * ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.gpu[l_proc][2]);

                temp32 = ROUND_POWER(temp64);
                sensor_update( AMECSENSOR_PTR(PWRGPU), (uint16_t)temp32);
            }

            // ----------------------------------------------------
            // Convert Raw Bulk Power from APSS into sensors
            // ----------------------------------------------------
            // Not all systems provide a total system current/power, in which case it must be calculated.
            if(G_sysConfigData.apss_adc_map.total_current_12v != SYSCFG_INVALID_ADC_CHAN)
            {
                // Total System Power was provided, use directly
                temp32 = ROUND_POWER(ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.total_current_12v) * l_bulk_voltage);
                sensor_update(AMECSENSOR_PTR(PWRSYS), (uint16_t)temp32);
            }
            else
            {
                // No Total System Power - Add powers for channels
                temp64 = 0;
                for (l_channel = 0; l_channel < MAX_APSS_ADC_CHANNELS; l_channel++)
                {
                    if ((l_channel != G_sysConfigData.apss_adc_map.sense_12v) &&
                        (l_channel != G_sysConfigData.apss_adc_map.remote_gnd) &&
                        (l_channel != G_sysConfigData.apss_adc_map.current_12v_stby))
                    {
                        // Add power for this channel
                        temp64 += l_bulk_voltage * G_lastValidAdcValue[l_channel];
                    }
                }
                temp32 = ROUND_POWER(temp64);
                sensor_update(AMECSENSOR_PTR(PWRSYS), (uint16_t)temp32);
            }

            // Check if in oversubscription and trace time it took to get under cap
            if( L_trace_time && (AMEC_INTF_GET_OVERSUBSCRIPTION()) &&
               ((uint16_t)temp32 <= g_amec->pcap.ovs_node_pcap) )
            {
                L_trace_time = FALSE;
                TRAC_ERR("Pwr[%d] now under Oversubscription power limit[%d]  PPB[%d]", temp32,
                          g_amec->pcap.ovs_node_pcap, g_amec->proc[0].pwr_votes.ppb_fmax);
                TRAC_ERR("Time to get under oversub power limit[%d us]",
                          DURATION_IN_US_UNTIL_NOW_FROM(g_amec->oversub_status.oversubActiveTime));
            }
            else if(AMEC_INTF_GET_OVERSUBSCRIPTION() == FALSE)
                L_trace_time = TRUE;

            // Check the GPU presence signals
            amec_update_gpu_configuration();

            // ----------------------------------------------------
            // Clear Flag to indicate that AMEC has received the data.
            // ----------------------------------------------------
            G_slv_inbox_received = FALSE;
        }
        else
        {
            // Skip it...AMEC Health Monitor will figure out we didn't
            // update this sensor.
        }
    } while( 0 );

    return l_sensors_updated;

} // end amec_update_apss_sensors()

// Function Specification
//
// Name: calculate_AF
//
// Description: Calculate Acceleration Factor
// End Function Specification
void calculate_AF(uint16_t i_chip_voltage_100uv)
{
    sensor_t       *l_sensor_ptr = NULL;

    uint8_t  i = 0;
    uint8_t  j = 0;
    uint8_t  l_xv_index = 0;
    uint16_t l_proc_temp = 0;
    uint16_t l_Xt = 0;
    uint16_t l_Xt_square = 0;
    uint16_t l_Xt_cube = 0;
    uint16_t l_AFt = 0;
    uint16_t l_Xv = 0;
    uint16_t l_AFv = 0;
    uint16_t l_AF = 0;
    uint32_t l_vsense_sum = 0;
    static uint16_t L_ticks = 0;
    static bool     L_vsense_avg_avail = FALSE;
    static bool     L_trace_invalid_AFt = TRUE;
    static uint8_t  L_update_tag = 0;
    static uint16_t L_proc_temp_max = 0;
    static uint16_t L_vsense_samples = 0;
    static uint16_t L_vsense_avg = 0;
    static uint16_t L_vsense_readings[NUM_SAMPLES_VSENSE_AVG] = {0};

    // keep track of how often this is called
    L_ticks++;

    if(L_vsense_samples == NUM_SAMPLES_VSENSE_AVG)
    {
        L_vsense_samples = 0;
        L_vsense_avg_avail = TRUE;
    }
    else
    {
        L_vsense_samples++;
    }

    // save the reading to keep a running average
    L_vsense_readings[L_vsense_samples] = i_chip_voltage_100uv;

    // only update if we have been running long enough to calculate an average voltage
    if(L_vsense_avg_avail)
    {
        // Check if we have a new max temp temperature
        l_sensor_ptr = getSensorByGsid(TEMPPROCTHRM);
        l_proc_temp = l_sensor_ptr->sample;
        if(l_proc_temp > L_proc_temp_max)
        {
            L_proc_temp_max = l_proc_temp;

            // calculate the voltage chip average
            for(i=0; i<NUM_SAMPLES_VSENSE_AVG; i++)
            {
                l_vsense_sum += L_vsense_readings[i];
            }
            L_vsense_avg = (uint16_t)(l_vsense_sum / NUM_SAMPLES_VSENSE_AVG);
        }

        // check if it is time to calculate AF
        if(L_ticks == NUM_TICKS_AF_CALC)
        {
            // calculate AF for temperature (AFt)
            // first calculate l_Xt which is a scaled function of max proc temp
            if(L_proc_temp_max < 40)
                l_Xt = 0;
            else if(L_proc_temp_max > 112)
                l_Xt = 18;
            else
                l_Xt = (uint8_t)((L_proc_temp_max - 40) / 4);

            // Now calculate AFt
            // AFt = (Xt^^3 + 2Xt^^2 + 98Xt + 400) / 200
            l_Xt_cube = l_Xt * l_Xt * l_Xt;
            l_Xt_square = 2 * l_Xt * l_Xt;
            l_Xt *= 98;
            l_AFt = l_Xt_cube + l_Xt_square + l_Xt + 400;
            l_AFt /= 200;
            // sanity check l_AFt
            if((l_AFt < MIN_AFT) || (l_AFt > MAX_AFT))
            {
                if(L_trace_invalid_AFt)
                {
                    L_trace_invalid_AFt = FALSE;
                    TRAC_ERR("calculate_AF: AFt[%d] not within valid range %d to %d",
                              l_AFt, MIN_AFT, MAX_AFT);
                }
                if(l_AFt < MIN_AFT)
                    l_AFt = MIN_AFT;
                else
                    l_AFt = MAX_AFT;
            }

            // calculate AF for voltage (AFv)
            // first calculate a scaled Voltage l_Xv to get the scaled voltage as an integer from 93 to 110
            l_Xv = (uint16_t)(L_vsense_avg / 100);
            // round
            if((L_vsense_avg % 100) >= 50)
               l_Xv++;

            if(l_Xv < MIN_XV)
               l_Xv = MIN_XV;
            else if(l_Xv > MAX_XV)
               l_Xv = MAX_XV;

            // Use hard-code table to get from l_Xv to AFv
            l_xv_index = (uint8_t)(l_Xv - MIN_XV);
            l_AFv = G_AFv_lookup[l_xv_index];

            // Calculate AF
            l_AF = (l_AFt * l_AFv);

            // update the dcom values to be sent to master for the call home log
            L_update_tag++;
            if(L_update_tag == 0)
                 L_update_tag++;
            for( j = 0; j < DCOM_MAX_AF_ENTRIES-1; j++ )
            {
               // reading [0] is oldest, shift previous readings over
               G_dcom_slv_outbox_tx.af_calcs[j].update_tag = G_dcom_slv_outbox_tx.af_calcs[j+1].update_tag;
               G_dcom_slv_outbox_tx.af_calcs[j].AF = G_dcom_slv_outbox_tx.af_calcs[j+1].AF;
               G_dcom_slv_outbox_tx.af_calcs[j].AFv = G_dcom_slv_outbox_tx.af_calcs[j+1].AFv;
            }
            // place new AF at end
            G_dcom_slv_outbox_tx.af_calcs[DCOM_MAX_AF_ENTRIES-1].update_tag = L_update_tag;
            G_dcom_slv_outbox_tx.af_calcs[DCOM_MAX_AF_ENTRIES-1].AF = l_AF;
            G_dcom_slv_outbox_tx.af_calcs[DCOM_MAX_AF_ENTRIES-1].AFv = l_AFv;

            //reset for next AF calc
            L_ticks = 0;
            L_proc_temp_max = 0;
        }  // if time to calculate AF

    }  // if L_vsense_avg_avail

}  // calculate_AF()

// Calculate chip voltage and power and update sensors
void update_avsbus_power_sensors(const uint8_t i_types)
{
    static bool L_throttle_vdd = FALSE;
    static bool L_throttle_vcs = FALSE;
    static bool L_throttle_vdn = FALSE;
    static bool L_traced_invalid_type = FALSE;
    bool * L_throttle = &L_throttle_vdd;
    uint32_t l_loadline = G_oppb.vdd_sysparm.loadline_uohm;
    uint32_t l_distloss = G_oppb.vdd_sysparm.distloss_uohm;
    uint32_t l_currentSensor = CURVDD;
    uint32_t l_voltageSensor = VOLTVDD;
    uint32_t l_voltageChip = VOLTVDDSENSE;
    uint32_t l_powerSensor = PWRVDD;

    uint16_t l_proc_power = 0;
    uint8_t l_updated_types = 0;

    do
    {
        // find type that needs to be updated
        if( (i_types & AVSBUS_PGPE_VDD) &&
            !(l_updated_types & AVSBUS_PGPE_VDD) )
        {
           // defaults already initialized for Vdd
           l_updated_types |= AVSBUS_PGPE_VDD;
        }
        else if( (i_types & AVSBUS_PGPE_VCS) &&
            !(l_updated_types & AVSBUS_PGPE_VCS) )
        {
           // setup for Vcs
           L_throttle = &L_throttle_vcs;
           l_loadline = G_oppb.vcs_sysparm.loadline_uohm;
           l_distloss = G_oppb.vcs_sysparm.distloss_uohm;
           l_currentSensor = CURVCS;
           l_voltageSensor = VOLTVCS;
           l_voltageChip = VOLTVCSSENSE;
           l_powerSensor = PWRVCS;

           l_updated_types |= AVSBUS_PGPE_VCS;
        }
        else if( (i_types & AVSBUS_PGPE_VDN) &&
            !(l_updated_types & AVSBUS_PGPE_VDN) )
        {
           // setup for Vdn
           L_throttle = &L_throttle_vdn;
           l_loadline = G_oppb.vdn_sysparm.loadline_uohm;
           l_distloss = G_oppb.vdn_sysparm.distloss_uohm;
           l_currentSensor = CURVDN;
           l_voltageSensor = VOLTVDN;
           l_voltageChip = VOLTVDNSENSE;
           l_powerSensor = PWRVDN;

           l_updated_types |= AVSBUS_PGPE_VDN;
        }
        else
        {
           // invalid type
           if(!L_traced_invalid_type)
           {
               TRAC_ERR("update_avsbus_power_sensors: invalid type[0x%02X]  updated[0x%02X]",
                         i_types, l_updated_types);
               L_traced_invalid_type = TRUE;
           }
           break;
        }

        // Read latest voltage/current sensors
        uint32_t l_voltage_100uv = 0;
        uint32_t l_current_10ma = 0;
        sensor_t *l_sensor = getSensorByGsid(l_voltageSensor);
        if (l_sensor != NULL)
        {
            l_voltage_100uv = l_sensor->sample;
        }
        l_sensor = getSensorByGsid(l_currentSensor);
        if (l_sensor != NULL)
        {
            l_current_10ma = l_sensor->sample;
        }

        if ((l_voltage_100uv != 0) && (l_current_10ma != 0))
        {
            // Calculate voltage on just processor package (need to take load-line into account)
            // Voltage value stored in the sensor should be in 100uV (mV scale -1)
            // (current is in 10mA units, and load-line is in microOhms)
            // v(V)     = i(10mA)*(1 A/1000 mA) * r(1 uOhm)*(1 Ohm/1,000,000 uOhm)
            //          = i * (1 A/100) * r * (1 Ohm/1,000,000)
            //          = i * r / 100,000,000
            // v(uV)    = v(V) * 1,000,000
            // v(100uV) = v(uV) / 100
            //          = (v(V) * 1,000,000) / 100 = v(V) * 10,000
            //          = (i * r / 100,000,000) * 10,000 = i * r / 10,000
            // NOTE: distloss is the same as Rpath in the WOF algorithm
            const uint64_t l_volt_drop_100uv = (l_current_10ma * (l_loadline+l_distloss)) / 10000;
            // Calculate chip voltage
            int32_t l_chip_voltage_100uv = l_voltage_100uv - l_volt_drop_100uv;
            if ((l_chip_voltage_100uv <= 0) || (l_chip_voltage_100uv > 0xFFFF))
            {
                // Voltage out of range, do not write sensors
                if (!*L_throttle)
                {
                    TRAC_ERR("update_avsbus_power_sensors: chip voltage out of range! %d(100uV) - %d(100uV) = %d(100uV)",
                             l_voltage_100uv, WORD_LOW(l_volt_drop_100uv), l_chip_voltage_100uv);
                    *L_throttle = TRUE;
                }
            }
            else
            {
                *L_throttle = FALSE;

                // Update chip voltage (remote sense adjusted for loadline) (100uV units)
                sensor_update(AMECSENSOR_PTR(l_voltageChip), (uint16_t)l_chip_voltage_100uv);

                if(l_voltageChip == VOLTVDDSENSE)
                {
                    // Acceleration Factor Handling
                    calculate_AF((uint16_t)l_chip_voltage_100uv);
                }

                // Power value stored in the sensor should be in W (scale 0)
                // p(W) = v(V) * i(A) = v(100uV)*100/1,000,000 * i(10mA)*10/1000
                //                    = v(100uV)/10,000        * i(10mA)/100
                //                    = v(100uV) * i(10mA) / 1,000,000
                const uint32_t l_power = l_chip_voltage_100uv * l_current_10ma / 1000000;
                sensor_update(AMECSENSOR_PTR(l_powerSensor), (uint16_t)l_power);
                l_proc_power += l_power;
            }
        }
    }while(l_updated_types != i_types);

} // end update_avsbus_power_sensors()


// Function Specification
//
// Name: amec_update_avsbus_sensors
//
// Description: Read AVS Bus data and update sensors (called every tick)
//  reading V/I from OCC-PGPE Shared SRAM, only need to read Vdd temperature directly from AVSbus
//   Tick 0: idle
//   Tick 1: start Vdd temp read
//   Tick 2: process Vdd temp, start status read (Vdd)
//   Tick 3: process status
//   (back to tick 0)
//
//   Vdd temperature and status is read every 4 ticks (2ms)
//
// Thread: RealTime Loop
//
// End Function Specification
void amec_update_avsbus_sensors(void)
{
    static enum {
        AVSBUS_STATE_DISABLED            = 0,
        AVSBUS_STATE_INITIATE_READ       = 1,
        AVSBUS_STATE_PROCESS_TEMPERATURE = 2,
        AVSBUS_STATE_PROCESS_STATUS      = 3
    }
    L_avsbus_state = AVSBUS_STATE_INITIATE_READ;

    if (isSafeStateRequested())
    {
        L_avsbus_state = AVSBUS_STATE_DISABLED;
        G_avsbus_vdd_monitoring = FALSE;
    }

    switch (L_avsbus_state)
    {
        case AVSBUS_STATE_INITIATE_READ:
            // Start reading from AVS bus
            // Initiate AVS Bus read for Vdd temperature
            avsbus_read_start(AVSBUS_VDD, AVSBUS_TEMPERATURE);
            L_avsbus_state = AVSBUS_STATE_PROCESS_TEMPERATURE;

            break;  // case AVSBUS_STATE_INITIATE_READ

        case AVSBUS_STATE_PROCESS_TEMPERATURE:
            // Read and process Vdd temperature
            avsbus_read(AVSBUS_VDD, AVSBUS_TEMPERATURE);

            // Initiate read of status
            initiate_avsbus_read_status();
            L_avsbus_state = AVSBUS_STATE_PROCESS_STATUS;
            break;

        case AVSBUS_STATE_PROCESS_STATUS:
            {
                // Process the status
                process_avsbus_status();

                // Do nothing on next tick
                L_avsbus_state = AVSBUS_STATE_DISABLED;
            }
            break;

        case AVSBUS_STATE_DISABLED:
            // do nothing this tick, start temperature read on next tick
            L_avsbus_state = AVSBUS_STATE_INITIATE_READ;
            break;

        default:
            TRAC_ERR("amec_update_avsbus_sensors: INVALID AVSBUS STATE 0x%02X", L_avsbus_state);
            L_avsbus_state = AVSBUS_STATE_INITIATE_READ;
            break;
    }

    // read and update the V/I sensors from PGPE AVSbus readings
    // this will also save the data for WOF to use on this tick
    read_pgpe_produced_wof_values();

} // end amec_update_avsbus_sensors()

// Function Specification
//
// Name: amec_update_apss_gpio
//
// Description: Updates sensors based on the GPIO data from the APSS
//
// Thread: RealTime Loop
//
// End Function Specification
void amec_update_apss_gpio(void)
{
    // GPIO port numbers from system model
    uint8_t * l_vrhot_port_nums = G_sysConfigData.apss_gpio_map.vr_fan;

    // Actual values of the GPIO
    uint8_t   l_vrhot0 = 1, l_vrhot1 = 1;

    // Data is valid?
    uint8_t   l_valid0 = FALSE, l_valid1 = FALSE;

    // Get value from most recent APSS data
    l_valid0 = apss_gpio_get(l_vrhot_port_nums[0], &l_vrhot0); //GPIO_VR_HOT_MEM_PROC_0
    l_valid1 = apss_gpio_get(l_vrhot_port_nums[1], &l_vrhot1); //GPIO_VR_HOT_MEM_PROC_1

    // Only log once
    static uint8_t L_err_logged;

    // These signals are active low
    if( (l_valid0 && !l_vrhot0) || (l_valid1 && !l_vrhot1) )
    {
        // Update the sensor indicating that one of the vrhot signals was asserted
        sensor_update(AMECSENSOR_PTR(VRHOTMEMPRCCNT), 1);

        // Only log once
        if(!L_err_logged)
        {
            INTR_TRAC_ERR("GPIO_VR_HOT_MEM_PROC_0[%d, valid=%d] GPIO_VR_HOT_MEM_PROC_1[%d, valid=%d]",
                 l_vrhot0, l_valid0, l_vrhot1, l_valid1);
            /*
             * @errortype
             * @moduleid    AMEC_UPDATE_APSS_GPIO
             * @reasoncode  VR_HOT_MEM_PROC_ASSERTED
             * @userdata1   0
             * @userdata2   0
             * @userdata4   OCC_NO_EXTENDED_RC
             * @devdesc     GPIO_VR_HOT_MEM_PROC_0/1 was asserted
             */
            errlHndl_t l_err = createErrl(AMEC_UPDATE_APSS_GPIO,
                                          VR_HOT_MEM_PROC_ASSERTED,
                                          OCC_NO_EXTENDED_RC,
                                          ERRL_SEV_INFORMATIONAL,
                                          NULL,
                                          DEFAULT_TRACE_SIZE,
                                         0,
                                          0);

            // Manufacturing error only
            setErrlActions(l_err, ERRL_ACTIONS_MANUFACTURING_ERROR);

            // Processor callout
            addCalloutToErrl(l_err,
                             ERRL_CALLOUT_TYPE_HUID,
                             G_sysConfigData.proc_huid,
                             ERRL_CALLOUT_PRIORITY_HIGH);

            // APSS callout
            addCalloutToErrl(l_err,
                             ERRL_CALLOUT_TYPE_HUID,
                             G_sysConfigData.apss_huid,
                             ERRL_CALLOUT_PRIORITY_LOW);

            commitErrl(&l_err);

            L_err_logged = TRUE;
        }
    }
    else if ( (l_valid0 && l_vrhot0) && (l_valid1 && l_vrhot1) )
    {
        sensor_update(AMECSENSOR_PTR(VRHOTMEMPRCCNT), 0);
    }
}

// Function Specification
//
// Name: amec_update_gpu_configuration
//
// Description: Checks the APSS data to see which GPUs are present
//
// Thread: RealTime Loop
//
// End Function Specification
void amec_update_gpu_configuration(void)
{
    // GPIO port numbers from system model
    uint8_t * l_gpu_port_nums = G_sysConfigData.apss_gpio_map.gpu;

    // Actual values of the GPIO
    uint8_t   l_gpu_pres = 1;

    // Data is valid?
    bool   l_valid = FALSE;
    bool   l_all_valid = FALSE;

    uint8_t   i = 0;
    uint8_t   l_start_proc = (G_pbax_id.chip_id * GPU_PRES_SIGN_PER_OCC);

    uint8_t   l_valid_bitmask_proc = 0; // Bitmask for present GPUs behind just this proc
    uint8_t   l_valid_bitmask_sys = 0;  // Bitmask for present GPUs behind both procs
    uint8_t   l_num_gpus_sys = 0;       // Number of GPUs both procs

    // Check which GPUs are present
    for( i=0; i < MAX_GPU_PRES_SIGNALS; i++ )
    {
        l_valid = apss_gpio_get(l_gpu_port_nums[i], &l_gpu_pres);

        // Presence signal is active low
        l_gpu_pres = (l_gpu_pres ? 0 : 1);
        if(l_valid)
        {
            l_all_valid = TRUE;

            // Keep track of number and configuration of GPUs behind both procs
            l_num_gpus_sys += l_gpu_pres;
            l_valid_bitmask_sys |= (l_gpu_pres << i);

            // Also want to keep a separate tally of GPUs behind only this proc
            if( (i >= l_start_proc) && (i < (l_start_proc + GPU_PRES_SIGN_PER_OCC)) )
            {
                l_valid_bitmask_proc |= (l_gpu_pres << (i - l_start_proc));
            }
        }
        else
        {
            l_all_valid = FALSE;
            break;
        }
    }

    // If all GPU signals are valid, update the global if this is the first read.
    // If this is not the first read, make sure that the signals match the first.
    if(l_all_valid)
    {
        G_curr_proc_gpu_config = l_valid_bitmask_proc;
        G_curr_sys_gpu_config = l_valid_bitmask_sys;
        G_curr_num_gpus_sys = l_num_gpus_sys;
        if(!G_gpu_config_done)
        {
            G_gpu_config_done = TRUE;
            G_first_proc_gpu_config = l_valid_bitmask_proc;
            if(G_first_proc_gpu_config)
            {
               // GPUs are present enable monitoring
               G_gpu_monitoring_allowed = TRUE;
               G_task_table[TASK_ID_GPU_SM].flags = GPU_RTL_FLAGS;
            }
            G_first_sys_gpu_config = l_valid_bitmask_sys;
            G_first_num_gpus_sys = l_num_gpus_sys;
            TRAC_IMP("GPU presence detection completed. GPU configuration for this OCC: 0x%08X, total[%d]",
                     G_curr_proc_gpu_config, G_curr_num_gpus_sys);

            // Only if running OPAL need to notify dcom thread to update GPU presence in HOMER for OPAL
            if(G_sysConfigData.system_type.kvm)
            {
                ssx_semaphore_post(&G_dcomThreadWakeupSem);
            }
        }
        else if (G_curr_sys_gpu_config != G_first_sys_gpu_config)
        {
            TRAC_ERR("GPU presence has changed unexpectedly! Old:0x%02X, New:0x%02X",
                     G_first_sys_gpu_config, l_valid_bitmask_sys);
        }
    }
}
