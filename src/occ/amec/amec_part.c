/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/amec/amec_part.c $                                    */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2014                        */
/* [+] Google Inc.                                                        */
/* [+] International Business Machines Corp.                              */
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
#include <mode.h>
#include <amec_sys.h>
#include <amec_part.h>
#include <trac.h>
#include <dcom.h>

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
/* Code                                                                       */
/*----------------------------------------------------------------------------*/

// Given partition id, return pointer to existing partition or NULL
static amec_part_t* amec_part_find_by_id(amec_part_config_t* i_config,
                                         const uint8_t i_id)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    amec_part_t *l_part = NULL;
    uint16_t l_idx = 0;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    for (l_idx=0; l_idx<AMEC_PART_MAX_PART; l_idx++)
    {
        if (i_config->part_list[l_idx].id == i_id && i_config->part_list[l_idx].valid)
        {
            l_part = &(i_config->part_list[l_idx]);
            break;
        }
    }

    return l_part;
}

// Function Specification
//
// Name: amec_isr_speed_to_freq
//
// Description: Given a core, return a valid partition that owns it, or NULL.
//
// End Function Specification
amec_part_t* amec_part_find_by_core(amec_part_config_t* i_config,
                                    const uint16_t i_core_index)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    amec_part_t                 *l_part = NULL;
    uint16_t                    l_part_index = 0;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    do
    {
        if (i_core_index >= MAX_NUM_CORES)
        {
            break; // illegal core number
        }
        l_part_index = i_config->core2part[i_core_index];
        if (l_part_index >= AMEC_PART_MAX_PART)
        {
            break; // illegal partition number
        }
        if (i_config->part_list[l_part_index].valid)
        {
            l_part = (amec_part_t*) &(i_config->part_list[l_part_index]);
        }
    } while(0);

    return l_part;
}

// Function Specification
//
// Name: amec_part_add
//
// Description: Add a core group.
//
// End Function Specification
void amec_part_add(uint8_t i_id)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    amec_part_t                 *l_part;
    uint16_t                    l_idx = 0;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    do
    {
        l_part = (amec_part_t*) amec_part_find_by_id(&g_amec->part_config,
                                                     i_id);
        if (l_part != NULL)
        {
            break; // Partition already exists
        }

        // Find new slot
        for (l_idx=0; l_idx<AMEC_PART_MAX_PART; l_idx++)
        {
            if (!g_amec->part_config.part_list[l_idx].valid)
            {
                l_part = &g_amec->part_config.part_list[l_idx];
                break;
            }
        }

        if (l_part == NULL)
        {
            // This should never happen, since table should be large enough
            // to hold as many partitions as can be created on system.
            break;
        }

        // Enter new partition into table with default values
        l_part->id = i_id;
        l_part->valid = 1;  //Mark them as valid
        l_part->ncores = 0; //No cores
        for (l_idx=0; l_idx<AMEC_PART_NUM_CORES; l_idx++)
        {
            l_part->core_list[l_idx] = AMEC_PART_NUM_CORES;
        }

        // For now, create a single partition with all cores assigned to it
        // until we write the interface to talk to PHYP
        if (i_id == 0)
        {
            l_part->ncores = MAX_NUM_CORES;
            for (l_idx=0; l_idx<MAX_NUM_CORES; l_idx++)
            {
                l_part->core_list[l_idx] = l_idx;
            }
        }

        // Set default soft frequency boundaries to use full frequency range
        l_part->soft_fmin = 0x0000;
        l_part->soft_fmax = 0xFFFF;

        // Set default values for DPS parameters (Favor Energy)
        l_part->dpsalg.step_up = 8;
        l_part->dpsalg.step_down = 8;
        l_part->dpsalg.tlutil = 9800;
        l_part->dpsalg.sample_count_util = 16;
        l_part->dpsalg.epsilon_perc = 1800;
        l_part->dpsalg.alpha_up = 9800;
        l_part->dpsalg.alpha_down = 9800;
        l_part->dpsalg.type = 0; //No algorithm is selected yet
        l_part->dpsalg.freq_request = UINT16_MAX;

        l_part->es_policy = OCC_INTERNAL_MODE_MAX_NUM;
        l_part->follow_sysmode = TRUE;
    }
    while (0);

    return;
}

void amec_part_init()
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    uint16_t                    l_idx = 0;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    // Initial core to partition mapping. Cores point to an invalid core group
    // id.
    // For now, we assume a single partition (id 0) with all cores assigned to
    // it. Once the PHYP interface has been written, we can remove this
    // assumption.
    for (l_idx=0; l_idx<MAX_NUM_CORES; l_idx++)
    {
        g_amec->part_config.core2part[l_idx] = 0; // AMEC_PART_INVALID_ID;
    }

    for (l_idx=0; l_idx<AMEC_PART_MAX_PART; l_idx++)
    {
        // Creating all possible partitions
        amec_part_add(l_idx);

        g_amec->part_config.part_list[l_idx].dpsalg.util_speed_request = g_amec->sys.fmax;
    }
}

void amec_part_update_dps_parameters(amec_part_t* io_part)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    // If partition doesn't exist, just exit
    if (io_part == NULL)
    {
        return;
    }

    // Check to see if parameter values have been overwritten by user
    if (g_amec->slv_dps_param_overwrite == TRUE)
    {
        // Then, exit right away
        return;
    }

    // By default, let's use the DPS-Favor Energy settings
    io_part->dpsalg.step_up = 8;
    io_part->dpsalg.step_down = 8;
    io_part->dpsalg.tlutil = 9800;
    io_part->dpsalg.sample_count_util = 16;
    io_part->dpsalg.epsilon_perc = 1800;
    io_part->dpsalg.alpha_up = 9800;
    io_part->dpsalg.alpha_down = 9800;
    io_part->dpsalg.type = 41;

    // If core group policy is DPS-Favor Performance, then write those settings
    if (io_part->es_policy == OCC_INTERNAL_MODE_DPS_MP)
    {
        io_part->dpsalg.step_up = 1000;
        io_part->dpsalg.step_down = 8;
        io_part->dpsalg.tlutil = 1000;
        io_part->dpsalg.sample_count_util = 1;
        io_part->dpsalg.epsilon_perc = 0;
        io_part->dpsalg.alpha_up = 9990;
        io_part->dpsalg.alpha_down = 9990;
        io_part->dpsalg.type = 41;

        // Check if this is a multi-node system
        if (G_sysConfigData.system_type.single == 0)
        {
            // These parameter values will result in static turbo frequency
            io_part->dpsalg.alpha_down = 0;
            io_part->dpsalg.tlutil = 0;

            TRAC_IMP("Multi-node system found! Updating DPS-FP parameters: single_node_bit[%u] alpha_down[%u] tlutil[%u]",
                     G_sysConfigData.system_type.single,
                     io_part->dpsalg.alpha_down,
                     io_part->dpsalg.tlutil);
        }
    }
}

void amec_part_update_perf_settings(amec_part_t* io_part)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    uint16_t                    j = 0;
    uint16_t                    l_core_index = 0;
    amec_core_perf_counter_t*   l_perf;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    // If partition doesn't exist, just exit
    if (io_part == NULL)
    {
        return;
    }

    switch (io_part->es_policy)
    {
        case OCC_INTERNAL_MODE_DPS:
        case OCC_INTERNAL_MODE_DPS_MP:
            // These policies require that we reset the internal performance
            // settings of the cores in the partition
            for (j=0; j<io_part->ncores; j++)
            {
                l_core_index = io_part->core_list[j];
                l_perf = &g_amec->proc[0].core[l_core_index % MAX_NUM_CORES].core_perf;

                memset(l_perf->ptr_util_slack_avg_buffer, 0, 2*MAX_UTIL_SLACK_AVG_LEN);
                memset(l_perf->ptr_util_active_avg_buffer, 0, 2*MAX_UTIL_SLACK_AVG_LEN);
                l_perf->util_active_core_counter = 0;
                l_perf->util_slack_core_counter = 0;
                l_perf->util_slack_accumulator = 0;
                l_perf->util_active_accumulator = 0;
                l_perf->ptr_putUtilslack = 0;
            }
            break;

        default:
            // For all other policies, reset the DPS frequency request
            for (j=0; j<io_part->ncores; j++)
            {
                l_core_index = io_part->core_list[j];
                g_amec->proc[0].core[l_core_index % MAX_NUM_CORES]
                    .core_perf.dps_freq_request = UINT16_MAX;
            }
            break;
    }
}

void AMEC_part_update_sysmode_policy(OCC_MODE i_occ_internal_mode)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    uint16_t                    i = 0;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    for (i=0; i<AMEC_PART_MAX_PART; i++)
    {
        // If a core group is not valid, skip it
        if (!g_amec->part_config.part_list[i].valid)
        {
            continue;
        }
        // If a core group has no cores assigned, skip it
        if (g_amec->part_config.part_list[i].ncores == 0)
        {
            continue;
        }

        // If a core group is not following the system power mode, skip it
        if (!g_amec->part_config.part_list[i].follow_sysmode)
        {
            continue;
        }

        // Set power mode for this core group to the system power mode
        switch (i_occ_internal_mode)
        {
            case OCC_MODE_NOMINAL:
                g_amec->part_config.part_list[i].es_policy = OCC_INTERNAL_MODE_NOM;
                break;

            case OCC_MODE_DYN_POWER_SAVE:
                g_amec->part_config.part_list[i].es_policy = OCC_INTERNAL_MODE_DPS;
                break;

            case OCC_MODE_DYN_POWER_SAVE_FP:
                g_amec->part_config.part_list[i].es_policy = OCC_INTERNAL_MODE_DPS_MP;
                break;

            default:
                g_amec->part_config.part_list[i].es_policy = OCC_INTERNAL_MODE_UNDEFINED;
                break;
        }

        // Update the DPS parameters based on the power policy
        if ((g_amec->part_config.part_list[i].es_policy == OCC_INTERNAL_MODE_DPS) ||
            (g_amec->part_config.part_list[i].es_policy == OCC_INTERNAL_MODE_DPS_MP))
        {
            amec_part_update_dps_parameters(&(g_amec->part_config.part_list[i]));
        }

        // Update internal performance settings for this partition
        amec_part_update_perf_settings(&(g_amec->part_config.part_list[i]));

        // Useful trace for debug
        //TRAC_INFO("AMEC_part_update_sysmode_policy: Core_group[%u] Num_cores[%u] ES_policy[0x%02x]",
        //          i, g_amec->part_config.part_list[i].ncores,
        //          g_amec->part_config.part_list[i].es_policy);
    }
}

void AMEC_part_overwrite_dps_parameters(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    uint16_t                    i = 0;
    uint16_t                    j = 0;
    uint16_t                    l_core_index = 0;
    amec_part_t*                l_part = NULL;
    amec_core_perf_counter_t*   l_perf = NULL;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    for (i=0; i<AMEC_PART_MAX_PART; i++)
    {
        // If a core group is not valid, skip it
        if (!g_amec->part_config.part_list[i].valid)
        {
            continue;
        }
        // If a core group has no cores assigned, skip it
        if (g_amec->part_config.part_list[i].ncores == 0)
        {
            continue;
        }

        // Overwrite the DPS parameters based on values sent by Master OCC
        l_part = &(g_amec->part_config.part_list[i]);
        l_part->dpsalg.alpha_up = G_dcom_slv_inbox_rx.alpha_up;
        l_part->dpsalg.alpha_down = G_dcom_slv_inbox_rx.alpha_down;
        l_part->dpsalg.sample_count_util = G_dcom_slv_inbox_rx.sample_count_util;
        l_part->dpsalg.step_up = G_dcom_slv_inbox_rx.step_up;
        l_part->dpsalg.step_down = G_dcom_slv_inbox_rx.step_down;
        l_part->dpsalg.epsilon_perc = G_dcom_slv_inbox_rx.epsilon_perc;
        l_part->dpsalg.tlutil = G_dcom_slv_inbox_rx.tlutil;
        // The algorithm type cannot be selected by customer, hardcoded here.
        l_part->dpsalg.type = 41;

        // Update internal performance settings for each core on this partition
        for (j=0; j<l_part->ncores; j++)
        {
            l_core_index = l_part->core_list[j];
            l_perf = &g_amec->proc[0].core[l_core_index % MAX_NUM_CORES].core_perf;

            memset(l_perf->ptr_util_slack_avg_buffer, 0, 2*MAX_UTIL_SLACK_AVG_LEN);
            memset(l_perf->ptr_util_active_avg_buffer, 0, 2*MAX_UTIL_SLACK_AVG_LEN);
            l_perf->util_active_core_counter = 0;
            l_perf->util_slack_core_counter = 0;
            l_perf->util_slack_accumulator = 0;
            l_perf->util_active_accumulator = 0;
            l_perf->ptr_putUtilslack = 0;
        }
    }
}

/*----------------------------------------------------------------------------*/
/* End                                                                        */
/*----------------------------------------------------------------------------*/
