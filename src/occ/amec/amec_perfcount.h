/******************************************************************************
 * @file amec_perfcount.h
 * @brief AMEC Internal headers / structures
 ******************************************************************************/
/******************************************************************************
 *
 *      @page ChangeLogs Change Logs
 *      @section _amec_perfcount_h amec_perfcount.h
 *      @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *   @ly001  853751     lychen    09/17/2012  Initial Revision
 *
 *  @endverbatim
 *
 ******************************************************************************/
 
#ifndef _AMEC_PERFCOUNT_H
#define _AMEC_PERFCOUNT_H
/*----------------------------------------------------------------------------*/
/* Includes                                                                   */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Constants                                                                  */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Globals                                                                    */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Defines                                                                    */
/*----------------------------------------------------------------------------*/
#define MAX_UTIL_SLACK_AVG_LEN    64  // Max # of samples in utilization slack averaging buffer

/*----------------------------------------------------------------------------*/
/* Typedef / Enum                                                             */
/*----------------------------------------------------------------------------*/

/// Core Performance Counter Model
typedef struct amec_core_perf_counter
{
    ///32-bit accumulator of util_slack counter
    uint32_t                    util_slack_accumulator;
    ///32-bit accumulator of util_active counter
    uint32_t                    util_active_accumulator;
    ///Circular buffer pointer to put Utilslack signal
    uint16_t                    ptr_putUtilslack;
    ///Frequency request
    uint16_t                    dps_freq_request;
    ///32-bit pointer to utilization slack averaging buffer
    uint8_t                     ptr_util_slack_avg_buffer[2*MAX_UTIL_SLACK_AVG_LEN];
    ///32-bit pointer to utilization active averaging buffer
    uint8_t                     ptr_util_active_avg_buffer[2*MAX_UTIL_SLACK_AVG_LEN];
    ///8-bit counter of cores that are active (utilization>CPU_utilization_threshold)
    uint8_t                     util_active_core_counter;
    ///8-bit counter of cores with slack (utilization<type 1 alg UTIL tlutil)
    uint8_t                     util_slack_core_counter;
}amec_core_perf_counter_t;

/*----------------------------------------------------------------------------*/
/* Function Prototypes                                                        */
/*----------------------------------------------------------------------------*/

/**
 * @brief Calculate the performance counter for a core
 * @param i_core_id Core ID that this struct belongs on
 *
 * @return void
 */
void amec_calc_dps_util_counters(const uint8_t i_core_id);

/**
 * @brief Build the performance counter for a core
 * @param this_ptr  perf counter we wish to set up. NULL if you
 *                  wish to allocate one dynamically. A pointer
 *                  will be returned.
 * @param i_proc_id Processor ID that this struct belongs on
 * @param i_core_id Core ID that this struct belongs on
 *
 * @return amec_core_perf_counter_t* on success, NULL if an
 *         error
 */
amec_core_perf_counter_t* amec_core_perf_counter_ctor(amec_core_perf_counter_t* i_this_ptr,
                                                      const uint8_t i_proc_id,
                                                      const uint8_t i_core_id);

#endif
