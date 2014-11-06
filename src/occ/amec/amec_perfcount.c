/******************************************************************************
 * @file amec_perfcount.c
 * @brief AMEC Performance Counter Code
 *
 * Structure Definition / Helper Functions for Performance Counter Calculations
 *
 ******************************************************************************/
/******************************************************************************
 *
 *      @page ChangeLogs Change Logs
 *      @section _amec_perfcount_c amec_perfcount.c
 *      @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *   @ly001  853751     lychen    09/17/2012  Initial Revision
 *
 *  @endverbatim
 *
 ******************************************************************************/

/*----------------------------------------------------------------------------*/
/* Includes                                                                   */
/*----------------------------------------------------------------------------*/
#include <string.h>
#include <common_types.h>
#include <sensor.h>
#include <amec_sys.h>
#include <amec_part.h>
#include <amec_perfcount.h>

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

// Function Specification
//
// Name: amec_calc_dps_util_counters
//
// Description: Calculate the performance counter for a core.
//
// Flow:              FN= None
//
// End Function Specification
void amec_calc_dps_util_counters(const uint8_t i_core_id)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    amec_part_t                 *l_part = NULL;
    amec_core_perf_counter_t    *l_perf = NULL;
    sensor_ptr_t                l_sensor = NULL;
    uint16_t                    l_utilization = 0;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    l_perf = &g_amec->proc[0].core[i_core_id].core_perf;
    // Read sensor for this core
    l_sensor = AMECSENSOR_ARRAY_PTR(UTIL2MSP0C0, i_core_id);
    l_utilization = l_sensor->sample;
    l_part = amec_part_find_by_core(&g_amec->part_config, i_core_id);

    // Type 41 input: Check if core's utilization is within
    // epsilon of the slack threshold: if yes declare the core
    // active
    if (l_part != NULL)
    {
        if (l_utilization > (l_part->dpsalg.tlutil - l_part->dpsalg.epsilon_perc))
        {
            // indicate core is active
            l_perf->util_active_core_counter++;

            if (l_utilization < l_part->dpsalg.tlutil)
            {
                // indicate core has some slack
                l_perf->util_slack_core_counter++;
            }
        }
    }
}

// Function Specification
//
// Name: amec_core_perf_counter_ctor
//
// Description: Build the performance counter for a core.
//
// Flow:              FN= None
//
// End Function Specification
amec_core_perf_counter_t* amec_core_perf_counter_ctor(amec_core_perf_counter_t* i_this_ptr,
                                                      const uint8_t i_proc_id,
                                                      const uint8_t i_core_id)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    
    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    //zero everything out, regardless of when we were called
    memset(i_this_ptr, 0x00, sizeof(amec_core_perf_counter_t));

    // Create space for per-core active and slack counts (for DPS #41 algorithm)
    memset(i_this_ptr->ptr_util_slack_avg_buffer, 0,
           (uint32_t)(2*MAX_UTIL_SLACK_AVG_LEN));
    memset(i_this_ptr->ptr_util_active_avg_buffer, 0,
           (uint32_t)(2*MAX_UTIL_SLACK_AVG_LEN));

    // DPS frequency request. Choose highest performance by default.
    i_this_ptr->dps_freq_request = UINT16_MAX; // input to core freq voting box

    //pass out i_this_ptr in case we're allocating this dynamically
    return i_this_ptr;
}

/*----------------------------------------------------------------------------*/
/* End                                                                        */
/*----------------------------------------------------------------------------*/
