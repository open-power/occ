/******************************************************************************
 * @file amec_dps.h
 * @brief AMEC Dynamic Power Saving (DPS) Algorithms Header File
 ******************************************************************************/
/******************************************************************************
 *
 *      @page ChangeLogs Change Logs
 *      @section _amec_dps_h amec_dps.h
 *      @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *   @ly001  853751     lychen    09/17/2012  Initial Revision
 *   @ry002  862116     ronda     11/26/2012  Moved speed to freq conversion function
 *
 *  @endverbatim
 *
 ******************************************************************************/

#ifndef _AMEC_DPS_H
#define _AMEC_DPS_H

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
#define TWO_TO_THE_POWER_OF_FIFTEEN      32768

/*----------------------------------------------------------------------------*/
/* Typedef / Enum                                                             */
/*----------------------------------------------------------------------------*/
///DPS Algorithm Model
typedef struct amec_dps
{
    ///Frequency request for core voting box
    uint16_t                    freq_request;
    ///Utilization speed request
    uint16_t                    util_speed_request;
    ///Utilization threshold for moving down in frequency (low side)
    uint16_t                    tlutil;
    ///Step size for going up in speed
    uint16_t                    step_up;
    ///Step size for going down in speed
    uint16_t                    step_down;
    ///Number of utilization samples in sliding window
    uint16_t                    sample_count_util;
    ///Epsilon used for determining if a core is active (units of 0.01%)
    uint16_t                    epsilon_perc;
    ///Threshold for going up in frequency
    uint16_t                    alpha_up;
    ///Threshold for going down in frequency
    uint16_t                    alpha_down;
    ///8-bit mask for dynamic power save type (=0:none active)
    uint8_t                     type;
}amec_dps_t;

/*----------------------------------------------------------------------------*/
/* Function Prototypes                                                        */
/*----------------------------------------------------------------------------*/

                         
/**
 *  @brief  Update per-core utilization variables.
 *
 *  This function updates all the per-core utilization variables. These
 *  variables are used to populate the slack sensors.
 *
 *  @param void
 *  @return void
 */
void amec_dps_update_core_util(void);

/**
 *  @brief  Update utilization sensors for a core group.
 *
 *  This function updates the utilization (slack) sensors for a
 *  given core group.
 *
 *  @param i_part_id ID of the core group
 *  @return void
 */
void amec_dps_partition_update_sensors(const uint16_t i_part_id);

/**
 *  @brief  DPS algorithm function.
 *
 *  This function implements the different DPS algorithms for a
 *  given core group.
 *
 *  @param i_part_id ID of the core group
 *  @return void
 */
void amec_dps_partition_alg(const uint16_t i_part_id);

/**
 *  @brief  Main DPS function.
 *
 *  This function is the entry-point for running DPS algorithms
 *  and is aware of the different core groups defined.
 *
 *  @param void
 *  @return void
 */
void amec_dps_main(void);

#endif  /* #ifndef _AMEC_DPS_H */
