/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/amec/amec_sensors_power.c $                       */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2016                        */
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
#include <cmdh_snapshot.h>
// @TODO - TEMP - uncomment when vrm file is updated
//#include <vrm.h>
#include "amec_oversub.h"

/******************************************************************************/
/* Globals                                                                    */
/******************************************************************************/
// This holds the converted ADC Reads
uint32_t G_lastValidAdcValue[MAX_APSS_ADC_CHANNELS] = {0};

// There are only MAX_APSS_ADC_CHANNELS channels.  Therefore if the channel value
// is greater then the MAX, then there was no channel associated with the function id.
#define ADC_CONVERTED_VALUE(i_chan) \
    ((i_chan < MAX_APSS_ADC_CHANNELS) ? G_lastValidAdcValue[i_chan] : 0)

extern uint8_t G_occ_interrupt_type;

//*************************************************************************
// Code
//*************************************************************************

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
         * Note that in the case of the remote ground and 12V sense the gain
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
        if (l_offset & 0x80000000)
        {
            // Negative offset
            l_raw -= (~l_offset + 1);
        }
        else
        {
            l_raw += l_offset;
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
// Function Specification
//
// Name: amec_update_apss_sensors
//
// Description: Calculates sensor from raw ADC values obtained from APSS
//
// Thread: RealTime Loop
//
// End Function Specification
void amec_update_apss_sensors(void)
{
    /*
     * Removed fake apss config data, do not reuse, the old hardcoded
     * values will not work with the new code used in processing APSS channel
     * data.
     * Code is in place to receive command code 0x21 SET CONFIG DATA
     * which should popluate the ADC and GPIO maps as well as the APSS
     * calibration data for all 16 ADC channels.
     */
    // Need to check to make sure APSS data has been received
    // via slave inbox first
    if (G_slv_inbox_received)
    {
        uint8_t l_proc   = G_pbax_id.module_id;
        uint32_t temp32  = 0;
        uint8_t  l_idx   = 0;
        uint32_t l_bulk_current_sum = 0;

        // ----------------------------------------------------
        // Convert all ADC Channels immediately
        // ----------------------------------------------------
        for (l_idx = 0; l_idx < MAX_APSS_ADC_CHANNELS; l_idx++)
        {
            // These values returned are gain adjusted. The APSS readings for
            // the remote ground and 12V sense are returned in mVs, all other
            // readings are treated as mAs.
            G_lastValidAdcValue[l_idx] = amec_value_from_apss_adc(l_idx);

            // Add up all channels now, we will subtract ones later that don't
            // count towards the system power
            l_bulk_current_sum += G_lastValidAdcValue[l_idx];
        }

        // --------------------------------------------------------------
        // Convert 12Vsense into interim value - this has to happen first
        // --------------------------------------------------------------
        // Calculations involving bulk_voltage must be 64bit so final result
        // does not get truncated (before dividing by ADCMULT_TO_UNITS)
        uint64_t l_bulk_voltage = ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.sense_12v);

        if (OCC_MASTER == G_occ_role)
        {
            // Update channel sensors for all channels (except 12v and gnd)
            for (l_idx = 0; l_idx < MAX_APSS_ADC_CHANNELS; l_idx++)
            {
                if ((l_idx != G_sysConfigData.apss_adc_map.sense_12v) &&
                    (l_idx != G_sysConfigData.apss_adc_map.remote_gnd))
                {
                    temp32 = ((ADC_CONVERTED_VALUE(l_idx) * l_bulk_voltage)+ADCMULT_ROUND)/ADCMULT_TO_UNITS;
                    sensor_update(AMECSENSOR_PTR(PWRAPSSCH0 + l_idx), (uint16_t) temp32);
                }
            }
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

        uint32_t l_vdd = ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.vdd[l_proc]);
        uint32_t l_vcs_vio_vpcie = ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.vcs_vio_vpcie[l_proc]);
        temp32 = ((l_vcs_vio_vpcie + l_vdd) * l_bulk_voltage)/ADCMULT_TO_UNITS;
        sensor_update(AMECSENSOR_PTR(PWR250USP0), (uint16_t) temp32);

        // Save off the combined power from all modules
        for (l_idx=0; l_idx < MAX_NUM_CHIP_MODULES; l_idx++)
        {
            uint32_t l_vd = ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.vdd[l_idx]);
            uint32_t l_vpcie = ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.vcs_vio_vpcie[l_idx]);
            g_amec->proc_snr_pwr[l_idx] =  ((l_vpcie + l_vd) * l_bulk_voltage)/ADCMULT_TO_UNITS;
        }

        // All readings from APSS come back as milliUnits, so if we want
        // to convert one, we need to
        //  divide by    1 to get it back to milliUnits (0.001)
        //  divide by   10 to get it to centiUnits      (0.01)
        //  divide by  100 to get it to deciUnits       (0.1)
        //  divide by 1000 to get it to Units           (1)
        // Vdd has both a power and a current sensor, we convert the Vdd power
        // to Watts and the current to centiAmps
        temp32 = ((l_vdd * l_bulk_voltage)+ADCMULT_ROUND)/ADCMULT_TO_UNITS;
        sensor_update( AMECSENSOR_PTR(PWR250USVDD0), (uint16_t)temp32);
        temp32 = ((l_vcs_vio_vpcie * l_bulk_voltage)+ADCMULT_ROUND)/ADCMULT_TO_UNITS;
        sensor_update( AMECSENSOR_PTR(PWR250USVCS0), (uint16_t)temp32);

        // ----------------------------------------------------
        // Convert Other Raw Misc Power from APSS into sensors
        // ----------------------------------------------------

        // Fans: Add up all Fan channels
        temp32  = ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.fans[0]);
        temp32 += ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.fans[1]);
        temp32  = ((temp32  * l_bulk_voltage)+ADCMULT_ROUND)/ADCMULT_TO_UNITS;
        sensor_update( AMECSENSOR_PTR(PWR250USFAN), (uint16_t)temp32);

        // I/O: Add up all I/O channels
        temp32  = ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.io[0]);
        temp32 += ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.io[1]);
        temp32 += ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.io[2]);
        temp32 = ((temp32  * l_bulk_voltage)+ADCMULT_ROUND)/ADCMULT_TO_UNITS;
        sensor_update( AMECSENSOR_PTR(PWR250USIO), (uint16_t)temp32);

        // Memory: Add up all channels for the same processor.
        temp32 = ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.memory[l_proc][0]);
        temp32 += ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.memory[l_proc][1]);
        temp32 += ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.memory[l_proc][2]);
        temp32 += ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.memory[l_proc][3]);
        //Only for FSP-LESS systems do we add in Centaur power because it is measured on its own A/D channel, but is part of memory power
        if (FSP_SUPPORTED_OCC != G_occ_interrupt_type)
        {
            temp32 += ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.mem_cache);
        }
        temp32 = ((temp32  * l_bulk_voltage)+ADCMULT_ROUND)/ADCMULT_TO_UNITS;
        sensor_update( AMECSENSOR_PTR(PWR250USMEM0), (uint16_t)temp32);

        // Save off the combined power from all memory
        for (l_idx=0; l_idx < MAX_NUM_CHIP_MODULES; l_idx++)
        {
            uint32_t l_temp = ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.memory[l_idx][0]);
            l_temp += ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.memory[l_idx][1]);
            l_temp += ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.memory[l_idx][2]);
            l_temp += ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.memory[l_idx][3]);
            g_amec->mem_snr_pwr[l_idx] = ((l_temp  * l_bulk_voltage)+ADCMULT_ROUND)/ADCMULT_TO_UNITS;
        }

        // Storage/Media
        temp32  = ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.storage_media[0]);
        temp32 += ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.storage_media[1]);
        temp32  = ((temp32  * l_bulk_voltage)+ADCMULT_ROUND)/ADCMULT_TO_UNITS;
        sensor_update( AMECSENSOR_PTR(PWR250USSTORE), (uint16_t)temp32);

        // GPU adapter
        temp32 = ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.gpu);
        temp32 = ((temp32 * l_bulk_voltage)+ADCMULT_ROUND)/ADCMULT_TO_UNITS;
        sensor_update( AMECSENSOR_PTR(PWR250USGPU), (uint16_t)temp32);

        // ----------------------------------------------------
        // Convert Raw Bulk Power from APSS into sensors
        // ----------------------------------------------------
        // We don't get this adc channel in some systems, we have to add it manually.
        // With valid sysconfig data the code here should automatically use what
        // is provided by the APSS if it is available, or manually sum it up if not.
        temp32 = ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.total_current_12v);
        temp32 = ((temp32 * l_bulk_voltage)+ADCMULT_ROUND)/ADCMULT_TO_UNITS;

        // To calculated the total 12V current based on a sum of all ADC channels,
        // Subract adc channels that don't measure power
        l_bulk_current_sum -= ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.sense_12v);
        l_bulk_current_sum -= ADC_CONVERTED_VALUE(G_sysConfigData.apss_adc_map.remote_gnd);

        // If we don't have a ADC channel that measures the bulk 12v power, use
        // the ADC sum instead
        if(0 == temp32)
        {
            temp32 = ((l_bulk_current_sum * l_bulk_voltage)+ADCMULT_ROUND)/ADCMULT_TO_UNITS;
        }
        sensor_update(AMECSENSOR_PTR(PWR250US), (uint16_t)temp32);

        // Calculate average frequency of all OCCs.
        uint32_t    l_allOccAvgFreqOver250us = 0;
        uint8_t     l_presentOCCs = 0;
        uint8_t     l_occCount = 0;

        // Add up the average freq from all OCCs.
        for (l_occCount = 0; l_occCount < MAX_OCCS; l_occCount++)
        {
            if (G_sysConfigData.is_occ_present & (1<< l_occCount))
            {
                l_allOccAvgFreqOver250us += G_dcom_slv_outbox_rx[l_occCount].freqa4msp0;
                l_presentOCCs++;
            }
        }
        //Calculate average of all the OCCs.
        l_allOccAvgFreqOver250us /= l_presentOCCs;

        // Save the max and min pwr250us sensors and keep an accumulator of the
        // average frequency over 30 seconds.
        if (g_pwr250us_over30sec.count == 0)
        {
            //The counter has been reset, therefore initialize the stored values.
            g_pwr250us_over30sec.max = (uint16_t) temp32;
            g_pwr250us_over30sec.min = (uint16_t) temp32;
            g_pwr250us_over30sec.freqaAccum = l_allOccAvgFreqOver250us;
        }
        else
        {
            //Check for max.
            if (temp32 > g_pwr250us_over30sec.max)
            {
                g_pwr250us_over30sec.max = (uint16_t) temp32;
            }
            //Check for min.
            if (temp32 < g_pwr250us_over30sec.min)
            {
                g_pwr250us_over30sec.min = (uint16_t) temp32;
            }
            //Average frequency accumulator.
            g_pwr250us_over30sec.freqaAccum += l_allOccAvgFreqOver250us;
        }

        //Count of number of updates.
        g_pwr250us_over30sec.count++;

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
}

// @TODO - TEMP - SPIVRMs are no longer defined, pgp_vrm.h and vrm.c are not present
#if 0

// Function Specification
//
// Name: amec_update_vrm_sensors
//
// Description: Updates sensors that use data from the VRMs
// (e.g., VR_FAN, FANS_FULL_SPEED, VR_HOT).
//
// Thread: RealTime Loop
//
// End Function Specification
void amec_update_vrm_sensors(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    int                         l_rc = 0;
    int                         l_vrfan = 0;
    int                         l_softoc = 0;
    int                         l_minus_np1_regmode = 0;
    int                         l_minus_n_regmode = 0;
    static uint8_t              L_error_count = 0;
    uint8_t                     l_pin_value = 1; // active low, so set default to high
    uint8_t                     l_vrhot_count = 0;
    errlHndl_t                  l_err = NULL;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    // VR_FAN and SOFT_OC come from SPIVID
    l_rc = vrm_read_state(SPIVRM_PORT(0),
                          &l_minus_np1_regmode,
                          &l_minus_n_regmode,
                          &l_vrfan,
                          &l_softoc);
    if (l_rc == 0)
    {
        // Update the VR_FAN sensor
        sensor_update( AMECSENSOR_PTR(VRFAN250USPROC), (uint16_t)l_vrfan );

        // Clear our error count
        L_error_count = 0;

        // No longer reading gpio from APSS in GA1 due to instability in
        // APSS composite mode
        //apss_gpio_get(l_pin, &l_pin_value);

        // VR_HOT sensor is a counter of number of times the VRHOT signal
        // has been asserted
        l_vrhot_count = AMECSENSOR_PTR(VRHOT250USPROC)->sample;

        // Check if VR_FAN is asserted AND if 'fans_full_speed' GPIO is ON.
        // Note that this GPIO is active low.
        if (AMECSENSOR_PTR(VRFAN250USPROC)->sample && !(l_pin_value))
        {
            // VR_FAN is asserted and 'fans_full_speed' GPIO is ON,
            // then increment our VR_HOT counter
            if (l_vrhot_count < g_amec->vrhotproc.setpoint)
            {
                l_vrhot_count++;
            }
        }
        else
        {
            // Reset our VR_HOT counter
            l_vrhot_count = 0;
        }
        sensor_update(AMECSENSOR_PTR(VRHOT250USPROC), l_vrhot_count);
    }
    else
    {
        // Increment our error count
        L_error_count++;

        // Don't allow the error count to wrap
        if (L_error_count == 0)
        {
            L_error_count = 0xFF;
        }
    }

    sensor_update( AMECSENSOR_PTR(VRFAN250USMEM), 0 );
    sensor_update( AMECSENSOR_PTR(VRHOT250USMEM), 0 );
}

#endif // #if 0 @TODO - TEMP - SPIVRMs are no longer defined, pgp_vrm.h and vrm.c are not present

// Function Specification
//
// Name: amec_update_external_voltage
//
// Description: Measure actual external voltage
//
// Thread: RealTime Loop
//
// End Function Specification
void amec_update_external_voltage()
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    uint32_t                    l_data = 0;
    uint16_t                    l_temp = 0;
    uint16_t                    l_vdd = 0;
    uint16_t                    l_vcs = 0;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    // Collect the external voltage data
// @TODO - TEMP - External Votage regulator is not defined in simics yet
//    l_data = in32(PMC_GLOBAL_ACTUAL_VOLTAGE_REG);

    // Extract the Vdd vid code and convert to voltage
    l_temp = (l_data & 0xFF000000) >>24;
    l_vdd = 16125 - ((uint32_t)l_temp * 625)/10;

    // Extract the Vcs vid code and convert to voltage
    l_temp = (l_data & 0x00FF0000) >>16;
    l_vcs = 16125 - ((uint32_t)l_temp * 625)/10;

    sensor_update( AMECSENSOR_PTR(VOLT250USP0V0), (uint16_t) l_vdd);
    sensor_update( AMECSENSOR_PTR(VOLT250USP0V1), (uint16_t) l_vcs);
}

// Function Specification
//
// Name: amec_update_current_sensor
//
// Description: Estimates Vdd output current based on input power and Vdd voltage setting.
//   Compute CUR250USVDD0 (current out of Vdd regulator)
//
// Flow:
//
// Thread: RealTime Loop
//
// Changedby:
//
// Task Flags:
//
// End Function Specification
void amec_update_current_sensor(void)
{
    uint32_t result32; //temporary result
    uint16_t l_pow_reg_input_dW = AMECSENSOR_PTR(PWR250USVDD0)->sample * 10; // convert to dW by *10.
    uint16_t l_vdd_reg = AMECSENSOR_PTR(VOLT250USP0V0)->sample;
    uint32_t l_pow_reg_output_mW;
    uint32_t l_curr_output;


    /* Step 1 */

    // 1. Get PWR250USVDD0  (the input power to regulator)
    // 2. Look up efficiency using PWR250USVDD0 as index (and interpolate)
    // 3. Calculate output power = PWR250USVDD0 * efficiency
    // 4. Calculate output current = output power / Vdd set point

    /* Determine regulator efficiency */
    // use 85% efficiency all the time
    result32 = 8500;

    // Compute regulator output power.  out = in * efficiency
    //    in: min=0W max=300W = 3000dW
    //    eff: min=0 max=10000=100% (.01% units)
    //    p_out: max=3000dW * 10000 = 30,000,000 (dW*0.0001) < 2^25, fits in 25 bits
    l_pow_reg_output_mW = (uint32_t)l_pow_reg_input_dW * (uint32_t)result32;
    // Scale up p_out by 10x to give better resolution for the following division step
    //    p_out: max=30M (dW*0.0001) in 25 bits
    //    * 10    = 300M (dW*0.00001) in 29 bits
    l_pow_reg_output_mW *= 10;
    // Compute current out of regulator.  curr_out = power_out (*10 scaling factor) / voltage_out
    //    p_out: max=300M (dW*0.00001) in 29 bits
    //    v_out: min=5000 (0.0001 V)  max=16000(0.0001 V) in 14 bits
    //    i_out: max = 300M/5000 = 60000 (dW*0.00001/(0.0001V)= 0.01A), in 16 bits.
    // VOLT250USP0V0 in units of 0.0001 V = 0.1 mV. (multiply by 0.1 to get mV)
    l_curr_output = l_pow_reg_output_mW / l_vdd_reg;
    sensor_update(AMECSENSOR_PTR(CUR250USVDD0), l_curr_output);

}

/*----------------------------------------------------------------------------*/
/* End                                                                        */
/*----------------------------------------------------------------------------*/
