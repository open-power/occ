/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/amec/amec_sensors_power.c $                       */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2020                        */
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

extern uint8_t G_gpu_volt_type[MAX_GPU_DOMAINS][MAX_NUM_GPU_PER_DOMAIN];

#define GPU0_USING_VOLT2(proc, channel) ((channel == G_sysConfigData.apss_adc_map.gpu[proc][0]) && (G_gpu_volt_type[proc][0] == 2))
#define GPU1_USING_VOLT2(proc, channel) ((channel == G_sysConfigData.apss_adc_map.gpu[proc][1]) && (G_gpu_volt_type[proc][1] == 2))

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
    bool l_sensors_updated = TRUE;
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
            uint8_t  l_idx   = 0;

            // ----------------------------------------------------
            // Convert all ADC Channels immediately
            // ----------------------------------------------------
            for (l_idx = 0; l_idx < MAX_APSS_ADC_CHANNELS; l_idx++)
            {
                // These values returned are gain adjusted. The APSS readings for
                // the remote ground and voltage sense are returned in mVs, all other
                // readings are treated as mAs.
                G_lastValidAdcValue[l_idx] = amec_value_from_apss_adc(l_idx);
            }

            // --------------------------------------------------------------
            // Convert voltages into interim value - this has to happen first
            // --------------------------------------------------------------
            // Calculations involving bulk_voltage must be 64bit so final result
            // does not get truncated (before dividing by ADCMULT_TO_UNITS)
            uint64_t l_bulk_voltage = ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.sense_12v);
            uint64_t l_bulk_voltage_2 = ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.sense_volt2);

            if (OCC_MASTER == G_occ_role)
            {
                // Update channel sensors for all channels (except voltage sense and gnd)
                for (l_idx = 0; l_idx < MAX_APSS_ADC_CHANNELS; l_idx++)
                {
                    if(l_idx == G_sysConfigData.apss_adc_map.current_12v_stby)
                    {
                        // Save value of 12V Standby Current (.01A) in a sensor for lab use only
                        temp32 = ADC_CONVERTED_VALUE(l_idx)/100;  // convert mA to .01A
                        sensor_update(AMECSENSOR_PTR(CUR12VSTBY), (uint16_t) temp32);
                    }
                    else if((l_idx != G_sysConfigData.apss_adc_map.sense_12v) &&
                            (l_idx != G_sysConfigData.apss_adc_map.sense_volt2) &&
                            (l_idx != G_sysConfigData.apss_adc_map.remote_gnd))
                    {
                        uint64_t l_bulk_v = l_bulk_voltage;
                        if ( GPU0_USING_VOLT2(0, l_idx) || GPU1_USING_VOLT2(0, l_idx) ||
                             GPU0_USING_VOLT2(1, l_idx) || GPU1_USING_VOLT2(1, l_idx) ||
                             (l_idx == G_sysConfigData.apss_adc_map.total_current_volt2) )
                        {
                            // Channel is using Voltge 2
                            l_bulk_v = l_bulk_voltage_2;
                        }
                        temp32 = ROUND_POWER(ADC_CONVERTED_VALUE(l_idx) * l_bulk_v);
                        sensor_update(AMECSENSOR_PTR(PWRAPSSCH0 + l_idx), (uint16_t) temp32);
                    }
                }

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

            // Make sure there is a channel for processor power else proc power sensor is using AVS bus and will
            // be updated in update_avsbus_power_sensors() instead
            if( (G_sysConfigData.apss_adc_map.vdd[l_proc] != SYSCFG_INVALID_ADC_CHAN) ||
                (G_sysConfigData.apss_adc_map.vcs_vio_vpcie[l_proc] != SYSCFG_INVALID_ADC_CHAN) )
            {
                uint32_t l_vdd = ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.vdd[l_proc]);
                uint32_t l_vcs_vio_vpcie = ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.vcs_vio_vpcie[l_proc]);
                temp32 = ROUND_POWER((l_vcs_vio_vpcie + l_vdd) * l_bulk_voltage);
                sensor_update(AMECSENSOR_PTR(PWRPROC), (uint16_t) temp32);
            }

            // Save off the combined power from all modules
            for (l_idx=0; l_idx < MAX_NUM_CHIP_MODULES; l_idx++)
            {
                uint32_t l_vd = ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.vdd[l_idx]);
                uint32_t l_vpcie = ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.vcs_vio_vpcie[l_idx]);
                g_amec->proc_snr_pwr[l_idx] = ROUND_POWER((l_vpcie + l_vd) * l_bulk_voltage);
            }

            // All readings from APSS come back as milliUnits, so if we want
            // to convert one, we need to
            //  divide by    1 to get it back to milliUnits (0.001)
            //  divide by   10 to get it to centiUnits      (0.01)
            //  divide by  100 to get it to deciUnits       (0.1)
            //  divide by 1000 to get it to Units           (1)

            // ----------------------------------------------------
            // Convert Other Raw Misc Power from APSS into sensors
            // ----------------------------------------------------

            // Memory: Add up all channels for the same processor.
            temp32 = ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.memory[l_proc][0]);
            temp32 += ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.memory[l_proc][1]);
            temp32 += ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.memory[l_proc][2]);
            temp32 += ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.memory[l_proc][3]);
            //Only for FSP-LESS systems do we add in membuf power because it is measured on its own A/D channel, but is part of memory power
            if (FSP_SUPPORTED_OCC != G_occ_interrupt_type)
            {
                temp32 += ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.mem_cache);
            }
            temp32 = ROUND_POWER(temp32  * l_bulk_voltage);
            sensor_update( AMECSENSOR_PTR(PWRMEM), (uint16_t)temp32);

            // Save off the combined power from all memory
            for (l_idx=0; l_idx < MAX_NUM_CHIP_MODULES; l_idx++)
            {
                uint32_t l_temp = ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.memory[l_idx][0]);
                l_temp += ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.memory[l_idx][1]);
                l_temp += ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.memory[l_idx][2]);
                l_temp += ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.memory[l_idx][3]);
                g_amec->mem_snr_pwr[l_idx] = ROUND_POWER(l_temp  * l_bulk_voltage);
            }

            // Save total GPU adapter for this proc
            if (l_proc < MAX_GPU_DOMAINS)
            {
                // GPU0
                uint64_t l_bulk_v = l_bulk_voltage;
                if (G_gpu_volt_type[l_proc][0] == 2)
                {
                    l_bulk_v = l_bulk_voltage_2;
                }
                temp32 = l_bulk_v * ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.gpu[l_proc][0]);
                // GPU1
                if (G_gpu_volt_type[l_proc][1] == 2)
                {
                    l_bulk_v = l_bulk_voltage_2;
                }
                else
                {
                    l_bulk_v = l_bulk_voltage;
                }
                temp32 += l_bulk_v * ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.gpu[l_proc][1]);
                // GPU2
                temp32 += l_bulk_voltage * ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.gpu[l_proc][2]);
                temp32 = ROUND_POWER(temp32);
                sensor_update( AMECSENSOR_PTR(PWRGPU), (uint16_t)temp32);
            }

            // ----------------------------------------------------
            // Convert Raw Bulk Power from APSS into sensors
            // ----------------------------------------------------
            // Not all systems provide a total system current/power, in which case it must be calculated.
            temp32 = 0;
            const uint32_t syspwr_volt1 = l_bulk_voltage   * ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.total_current_12v);
            const uint32_t syspwr_volt2 = l_bulk_voltage_2 * ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.total_current_volt2);
            if(0 != syspwr_volt1)
            {
                // Total System Power (voltage 1 (12V)) was provided, use directly
                temp32 = syspwr_volt1;
                if (0 != syspwr_volt2)
                {
                    // Total System Power (voltage 2) was provided, use directly
                    temp32 += syspwr_volt2;
                }
                else
                {
                    // No Total System Power (voltage 2) - Add powers for GPUs using voltage 2
                    for (l_idx = 0; l_idx < MAX_APSS_ADC_CHANNELS; l_idx++)
                    {
                        if ( GPU0_USING_VOLT2(0, l_idx) || GPU1_USING_VOLT2(0, l_idx) ||
                             GPU0_USING_VOLT2(1, l_idx) || GPU1_USING_VOLT2(1, l_idx) )
                        {
                            temp32 += l_bulk_voltage_2 * G_lastValidAdcValue[l_idx];
                        }
                    }
                }
            }
            else
            {
                // No Total System Power (voltage 1 (12V)) - Add powers for channels using voltage 1
                for (l_idx = 0; l_idx < MAX_APSS_ADC_CHANNELS; l_idx++)
                {
                    if ( (0 != syspwr_volt2) &&
                         ( GPU0_USING_VOLT2(0, l_idx) || GPU1_USING_VOLT2(0, l_idx) ||
                           GPU0_USING_VOLT2(1, l_idx) || GPU1_USING_VOLT2(1, l_idx) ) )
                    {
                        // Total System Power (voltage 2) was provided, so skip GPU channels using voltage 2
                        continue;
                    }

                    uint64_t l_bulk_v = l_bulk_voltage;
                    if ( GPU0_USING_VOLT2(0, l_idx) || GPU1_USING_VOLT2(0, l_idx) ||
                         GPU0_USING_VOLT2(1, l_idx) || GPU1_USING_VOLT2(1, l_idx) ||
                         (l_idx == G_sysConfigData.apss_adc_map.total_current_volt2) )
                    {
                        // GPU (or total current) is using voltage 2
                        l_bulk_v = l_bulk_voltage_2;
                    }

                    if ((l_idx != G_sysConfigData.apss_adc_map.sense_12v) &&
                        (l_idx != G_sysConfigData.apss_adc_map.remote_gnd) &&
                        (l_idx != G_sysConfigData.apss_adc_map.current_12v_stby) &&
                        (l_idx != G_sysConfigData.apss_adc_map.sense_volt2))
                    {
                        // Add power for this channel
                        temp32 += l_bulk_v * G_lastValidAdcValue[l_idx];
                    }
                }
            }
            temp32 = ROUND_POWER(temp32);
            sensor_update(AMECSENSOR_PTR(PWRSYS), (uint16_t)temp32);

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


// Calculate chip voltage and power and update sensors
void update_avsbus_power_sensors(const avsbus_type_e i_type)
{
    static bool L_throttle_vdd = FALSE;
    bool * L_throttle = &L_throttle_vdd;
    uint32_t l_loadline = G_oppb.vdd_sysparm.loadline_uohm;
    uint32_t l_distloss = G_oppb.vdd_sysparm.distloss_uohm;
    uint32_t l_currentSensor = CURVDD;
    uint32_t l_voltageSensor = VOLTVDD;
    uint32_t l_voltageChip = VOLTVDDSENSE;
    uint32_t l_powerSensor = PWRVDD;
    uint32_t l_powerSensor2 = PWRVDN;

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

            // Power value stored in the sensor should be in W (scale 0)
            // p(W) = v(V) * i(A) = v(100uV)*100/1,000,000 * i(10mA)*10/1000
            //                    = v(100uV)/10,000        * i(10mA)/100
            //                    = v(100uV) * i(10mA) / 1,000,000
            const uint32_t l_power = l_chip_voltage_100uv * l_current_10ma / 1000000;
            sensor_update(AMECSENSOR_PTR(l_powerSensor), (uint16_t)l_power);

            // check if there is an APSS with processor channel that would be providing the processor power sensor
            if( ( G_pwr_reading_type != PWR_READING_TYPE_APSS ) ||
                ( (G_sysConfigData.apss_adc_map.vdd[G_pbax_id.chip_id] == SYSCFG_INVALID_ADC_CHAN) &&
                  (G_sysConfigData.apss_adc_map.vcs_vio_vpcie[G_pbax_id.chip_id] == SYSCFG_INVALID_ADC_CHAN) ) )
            {
               // no proc pwr from APSS, update the processor power sensor with AVS bus total processor power
               // Vdd + Vdn
               sensor_t *l_sensor2 = getSensorByGsid(l_powerSensor2);
               const uint16_t l_proc_power = (uint16_t)l_power + l_sensor2->sample;
               sensor_update(AMECSENSOR_PTR(PWRPROC), l_proc_power);
            }
        }

    }

} // end update_avsbus_power_sensors()


// Function Specification
//
// Name: amec_update_avsbus_sensors
//
// Description: Read AVS Bus data and update sensors (called every tick)
//  reading V/I from OCC-PGPE Shared SRAM, only need to read Vdd temperature directly from AVSbus
//   Tick 0: start Vdd temp read
//   Tick 1: process Vdd temp, start status read (Vdd)
//   Tick 2: process status, start Vdd temp read
//   (back to tick 1)
//
//   Vdd temperature and status is read every 2 ticks
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

                // Initiate read of Vdd temperature
                avsbus_read_start(AVSBUS_VDD, AVSBUS_TEMPERATURE);
                L_avsbus_state = AVSBUS_STATE_PROCESS_TEMPERATURE;
            }
            break;

        case AVSBUS_STATE_DISABLED:
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
