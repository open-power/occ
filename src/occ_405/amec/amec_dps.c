/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/amec/amec_dps.c $                                 */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2019                        */
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

/*----------------------------------------------------------------------------*/
/* Includes                                                                   */
/*----------------------------------------------------------------------------*/
#include <common_types.h>
#include <occ_sys_config.h>
#include <proc_data.h>
#include <amec_sys.h>
#include <amec_perfcount.h>
#include <amec_part.h>
#include <amec_dps.h>
#include <sensor.h>
#include <amec_controller.h>

/*----------------------------------------------------------------------------*/
/* Constants                                                                  */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Globals                                                                    */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Typedef / Enum                                                             */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Internal Function Prototypes                                               */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Code                                                                       */
/*----------------------------------------------------------------------------*/

// Function Specification
//
// Name: amec_dps_update_core_util
//
// Description: Update per-core utilization variables.
//
// End Function Specification
void amec_dps_update_core_util(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    uint16_t                    l_temp16 = 0;
    uint8_t                     l_tempreg = 0;
    uint8_t                     l_idx = 0;
    amec_core_perf_counter_t*   l_perf = NULL;
    amec_part_t                 *l_part = NULL;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    // If g_amec->fw.dps_no_update_flag=1, no updating is allowed
    if (!g_amec->fw.dps_no_update_flag)
    {
        // Update moving average of util_slack and util_active for all cores
        for(l_idx=0; l_idx<MAX_NUM_CORES; l_idx++)
        {
            if (!CORE_PRESENT(l_idx) || CORE_OFFLINE(l_idx))
            {
                continue; //nothing to do if the core's disabled
            }

            l_part = amec_part_find_by_core(&g_amec->part_config, l_idx);

            // If this core doesn't belong to a core group, then do nothing
            if (l_part == NULL)
            {
                continue;
            }

            // Get pointer to the perf struc of this core
            l_perf = &g_amec->proc[0].core[l_idx].core_perf;

            l_perf->ptr_putUtilslack--; // Decrement put pointer
            if (l_perf->ptr_putUtilslack > TWO_TO_THE_POWER_OF_FIFTEEN)
            {
                // Wrap circular pointer. WARNING -> buffer size must be a
                // power of 2
                l_perf->ptr_putUtilslack &= (MAX_UTIL_SLACK_AVG_LEN-1);
            }

            // Locate oldest sample associated with 32-bit moving average
            // WARNING -> we need to read this sample first in case entire
            // buffer is used which would result in the write overwriting the
            // oldest sample we need to read.
            l_temp16=(uint16_t)(l_perf->ptr_putUtilslack+l_part->dpsalg.sample_count_util) & (MAX_UTIL_SLACK_AVG_LEN-1);
            l_tempreg=l_perf->ptr_util_slack_avg_buffer[l_temp16];
            // Bleed off oldest sample from moving average
            l_perf->util_slack_accumulator = l_perf->util_slack_accumulator-(uint32_t)l_tempreg;
            l_tempreg=l_perf->ptr_util_active_avg_buffer[l_temp16];
            // Bleed off oldest sample from moving average
            l_perf->util_active_accumulator = l_perf->util_active_accumulator-(uint32_t)l_tempreg;
            // Add in newest sample into moving average
            l_tempreg = l_perf->util_slack_core_counter;
            l_perf->util_slack_accumulator = l_perf->util_slack_accumulator+(uint32_t)l_tempreg;
            // Write new sample into buffer.
            l_perf->ptr_util_slack_avg_buffer[l_perf->ptr_putUtilslack]=l_tempreg;
            // Add in newest sample into moving average
            l_tempreg = l_perf->util_active_core_counter;
            l_perf->util_active_accumulator = l_perf->util_active_accumulator+(uint32_t)l_tempreg;
            // Write new sample into buffer.
            l_perf->ptr_util_active_avg_buffer[l_perf->ptr_putUtilslack]=l_tempreg;

            // Reset counters every 2msec
            l_perf->util_active_core_counter=0;
            l_perf->util_slack_core_counter=0;
        }
    }
}

// Function Specification
//
// Name: amec_dps_partition_update_sensors
//
// Description: Update utilization sensors for a core group.
//
// End Function Specification
void amec_dps_partition_update_sensors(const uint16_t i_part_id)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    amec_core_perf_counter_t*   l_perf = NULL;
    uint16_t                    l_core_index = 0;
    uint8_t                     l_idx = 0;
    uint32_t                    l_cg_active_accumulator = 0;
    uint32_t                    l_cg_slack_accumulator = 0;
    uint16_t                    l_cg_util_slack_perc = 0;
    uint32_t                    l_divide32[2] = {0, 0};
    static bool                 L_trace = FALSE;
    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    for (l_idx=0; l_idx<g_amec->part_config.part_list[i_part_id].ncores; l_idx++)
    {
        l_core_index = g_amec->part_config.part_list[i_part_id].core_list[l_idx];
        l_perf = &g_amec->proc[0].core[l_core_index % MAX_NUM_CORES].core_perf;

        // Sum accumulators for core group
        l_cg_active_accumulator += l_perf->util_active_accumulator;
        l_cg_slack_accumulator += l_perf->util_slack_accumulator;
    }

    if (l_cg_active_accumulator == 0)
    {
        // Check for divide by zero
        l_cg_util_slack_perc=16384;  // if no active cores, set slack=100%
    }
    else
    {
        l_divide32[1]=(uint32_t)l_cg_active_accumulator;
        l_divide32[0]=(uint32_t)l_cg_slack_accumulator<<14;  // *16384 because 16384=1.0
        l_divide32[0] /= l_divide32[1];
        l_cg_util_slack_perc=(uint16_t)(l_divide32[0]);
    }

    // Update the sensor of the utilization slack
    // verify the sensor exists for the core group
    if(i_part_id < AMEC_PART_MAX_PART)
    {
        sensor_update(&g_amec->part_config.part_list[i_part_id].util2msslack, l_cg_util_slack_perc);
    }
    else if(!L_trace)
    {
        // indicates missing support for sensors based on AMEC_PART_MAX_PART
        TRAC_ERR("amec_dps_partition_update_sensors: Invalid part id[0x%04x]", i_part_id);
        L_trace = TRUE;
    }
}

// Function Specification
//
// Name: amec_dps_partition_alg
//
// Description: DPS algorithm function.
//
// End Function Specification
void amec_dps_partition_alg(const uint16_t i_part_id)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    amec_core_perf_counter_t*   l_perf = NULL;
    uint16_t                    l_freq = 0;
    uint16_t                    l_core_index = 0;
    uint16_t                    l_idx = 0;
    uint16_t                    l_tempreg = 0;
    uint16_t                    l_temp16 = 0;
    uint32_t                    l_divide32[2] = {0, 0};
    static bool                 L_trace = FALSE;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    // Switch on dps_type of the core group
    switch (g_amec->part_config.part_list[i_part_id].dpsalg.type)
    {
    case 0x00:
        // Type 0 algorithm: no dynamic power savings enabled
        for (l_idx=0; l_idx<g_amec->part_config.part_list[i_part_id].ncores; l_idx++)
        {
            l_core_index = g_amec->part_config.part_list[i_part_id].core_list[l_idx];
            l_perf = &g_amec->proc[0].core[l_core_index % MAX_NUM_CORES].core_perf;
            l_perf->dps_freq_request = UINT16_MAX;
        }
        break;

    case 41:
        // Type 41 algorithm
        // verify the part id is valid
        if(i_part_id >= AMEC_PART_MAX_PART)
        {
            if(!L_trace)
            {
                // indicates missing support for sensors based on AMEC_PART_MAX_PART
                TRAC_ERR("amec_dps_partition_alg: Invalid part id[0x%04x]", i_part_id);
                L_trace = TRUE;
            }
            break;
        }

        // l_tempreg=measure of slack over gamma time interval on a per core basis
        l_tempreg = (uint16_t)g_amec->part_config.part_list[i_part_id].util2msslack.sample;
        // Convert to 10000 is equivalent to 100.00% utilization.
        l_divide32[1]=(uint32_t)53687;
        l_divide32[0]=(uint32_t)l_tempreg<<15;
        l_divide32[0] /= l_divide32[1];
        // l_tempreg is level of utilization using per core info
        l_tempreg=10000-(uint16_t)(l_divide32[0]);

        l_temp16 = g_amec->part_config.part_list[i_part_id].dpsalg.util_speed_request;
        if (l_tempreg > g_amec->part_config.part_list[i_part_id].dpsalg.alpha_up)
        {
            l_temp16 = g_amec->part_config.part_list[i_part_id].dpsalg.util_speed_request +
                g_amec->part_config.part_list[i_part_id].dpsalg.step_up;
        }
        else
        {
            if (l_tempreg < g_amec->part_config.part_list[i_part_id].dpsalg.alpha_down)
            {
                l_temp16 = g_amec->part_config.part_list[i_part_id].dpsalg.util_speed_request -
                    g_amec->part_config.part_list[i_part_id].dpsalg.step_down;
            }
        }

        // Now limit final speed in l_temp16 to be between allowed speeds.
        if (l_temp16 > g_amec->sys.max_speed)
        {
            l_temp16 = g_amec->sys.max_speed;
        }
        if (l_temp16 < g_amec->sys.min_speed)
        {
            l_temp16 = g_amec->sys.min_speed;
        }

        // Generate vote for utilization
        g_amec->part_config.part_list[i_part_id].dpsalg.util_speed_request = l_temp16;
        l_freq = amec_controller_speed2freq(l_temp16, G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MODE_DYN_PERF]);
        g_amec->part_config.part_list[i_part_id].dpsalg.freq_request = l_freq;

        for (l_idx=0; l_idx<g_amec->part_config.part_list[i_part_id].ncores; l_idx++)
        {
            l_core_index = g_amec->part_config.part_list[i_part_id].core_list[l_idx];
            l_perf = &g_amec->proc[0].core[l_core_index % MAX_NUM_CORES].core_perf;
            l_perf->dps_freq_request = l_freq;
        }
        // End algorithm type 41
        break;

    default:
        break;
    }
}

// Function Specification
//
// Name: amec_dps_main
//
// Description: Main DPS function.
//
// End Function Specification
void amec_dps_main(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    uint16_t                    l_idx = 0;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    // First, update the utilization variables for all cores
    amec_dps_update_core_util();

    if(CURRENT_MODE() == OCC_MODE_DYN_PERF)
    {
        amec_dps_partition_update_sensors(0);
        amec_dps_partition_alg(0);
        // send the frequency request from the DPS algorithm
        G_dcom_slv_outbox_tx.fwish = g_amec->part_config.part_list[0].dpsalg.freq_request;
    }
    else
    {
        // Else, send wof base frequency of the system
        G_dcom_slv_outbox_tx.fwish = G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_WOF_BASE];
    }

    // We also need to send the Factual to the Master OCC
    for (l_idx=0; l_idx<MAX_NUM_CORES; l_idx++)
    {
        // Find the first valid core and send its frequency
        if (CORE_PRESENT(l_idx) && !CORE_OFFLINE(l_idx))
        {
            G_dcom_slv_outbox_tx.factual = g_amec->proc[0].core[l_idx].f_request;
            break;
        }
    }
}

/*----------------------------------------------------------------------------*/
/* End                                                                        */
/*----------------------------------------------------------------------------*/
