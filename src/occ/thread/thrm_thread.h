/**
 * @file thrm_thread.h
 * @brief Header file for thermal thread.
 *
 *
*/
/**
 *      @page ChangeLogs Change Logs
 *      @section _thrm_thread_h thrm_thread.h
 *      @verbatim
 *
 *  Flag     Def/Fea    Userid    Date        Description
 *  ------- ---------- --------  ---------   ----------------------------------
 *  @gs019             gjsilva   11/21/2013  Created
 *  @gs020  909320     gjsilva   12/12/2013  Support for VR_FAN thermal control
 *
 *  @endverbatim
 */
#ifndef THRM_THREAD_H
#define THRM_THREAD_H

//*************************************************************************
// Includes
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************
#define THRM_MAX_NUM_ZONES  8

//*************************************************************************
// Structures
//*************************************************************************

typedef struct thrm_fru_control
{
    uint8_t                     t_control;
    uint8_t                     error;
    uint8_t                     warning;
    uint8_t                     warning_reset;
    uint16_t                    t_inc_zone[THRM_MAX_NUM_ZONES];
}thrm_fru_control_t;

typedef struct thrm_fru_data
{
    // Flag to indicate that we have timed out trying to read this thermal sensor
    uint8_t                     read_failure;
    // Current temperature of this FRU
    uint16_t                    Tcurrent;
    // Previous temperature of this FRU
    uint16_t                    Tprevious;
    // Coooling requests to be sent back to TMGT
    uint16_t                    FanIncZone[THRM_MAX_NUM_ZONES];
    // Flag to indicate if the temperature for this FRU is increasing or not
    BOOLEAN                     temp_increasing;
}thrm_fru_data_t;

/*******************************************************************/
/* Function Definitions                                            */
/*******************************************************************/

errlHndl_t thrm_thread_load_thresholds();
void THRM_thread_update_thresholds();
uint8_t THRM_thread_get_cooling_request();
BOOLEAN thrm_thread_fan_control(const uint8_t i_frutype,
                                const uint16_t i_fru_temperature);
BOOLEAN thrm_thread_vrm_fan_control(const uint16_t i_vrfan);
void thrm_thread_main();

#endif
