/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/amec/amec_part.c $                                */
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
// Name: amec_part_find_by_core
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

        // Set default values for Dynamic Performance
        l_part->dpsalg.step_up = 8;
        l_part->dpsalg.step_down = 8;
        l_part->dpsalg.tlutil = 9800;
        l_part->dpsalg.sample_count_util = 16;
        l_part->dpsalg.epsilon_perc = 1800;
        l_part->dpsalg.alpha_up = 9800;
        l_part->dpsalg.alpha_down = 9800;
        l_part->dpsalg.type = 0; //No algorithm is selected yet
        l_part->dpsalg.freq_request = UINT16_MAX;
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


/*----------------------------------------------------------------------------*/
/* End                                                                        */
/*----------------------------------------------------------------------------*/
