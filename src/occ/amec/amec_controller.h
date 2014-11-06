/******************************************************************************
// @file amec_controller.h
// @brief OCC AMEC Controller Structure
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section amec_controller.h AMEC_CONTROLLER.H
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *   @ry002             ronda     11/20/2012  New File 
 *   @ry003  870734     ronda     02/20/2013  Thermal controller for memory
 *   @gs012  903325     gjsilva   10/18/2013  Log Processor OT errors
 *   @gm013  907548     milesg    11/22/2013  Memory therm monitoring support
 *   @gs023  912003     gjsilva   01/16/2014  Generate VRHOT signal and control loop
 *
 *  @endverbatim
 *
 *///*************************************************************************/

#ifndef _AMEC_CONTROLLER_H
#define _AMEC_CONTROLLER_H
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
// Maximum speed supported for memory controllers (100.0%)
#define AMEC_MEMORY_MAX_STEP                 1000

// Minimum speed supported for memory controllers (1.0%)
#define AMEC_MEMORY_MIN_STEP                 10

// Step size for increasing or decreasing memory speed (1.0%)
#define AMEC_MEMORY_STEP_SIZE                10

// Limit value on speed change to avoid overflow of the controller
#define AMEC_MEMORY_SPEED_CHANGE_LIMIT       (UINT16_MAX/4)/AMEC_MEMORY_STEP_SIZE


/*----------------------------------------------------------------------------*/
/* Typedef / Enum                                                             */
/*----------------------------------------------------------------------------*/
///Controller Model
typedef struct amec_controller
{
    ///Over-temperature error for FRU callout (in degrees C)
    uint8_t                       ot_error;
    ///Setpoint or reference for the controller
    uint16_t                      setpoint;
    ///Proportional gain of the controller
    uint16_t                      Pgain;
    ///Theoretical controller residue summation
    uint16_t                      total_res;
    ///Final speed request of the controller
    uint16_t                      speed_request;
    ///Final frequency request of the controller
    uint16_t                      freq_request;
    ///Expiration time for a temperature reading (in seconds)
    uint8_t                       temp_timeout;
}amec_controller_t;


/*----------------------------------------------------------------------------*/
/* Function Prototypes                                                        */
/*----------------------------------------------------------------------------*/

/**
 *  @brief  Thermal Control Loop.
 *
 *  This function implements the Proportional Controller for the thermal
 *  control loop. Although it doesn't return any results, it populates the
 *  thermal vote in the field g_amec->thermalproc.speed_request.
 *
 *  @return No return from this function.
 */
void amec_controller_proc_thermal();

/**
 *  @brief  Thermal Control Loop based on Centaur
 *          temperatures.
 *
 *  This function implements a Proportional Controller for the
 *  thermal control loop based on Centaur temperatures.
 *  Although it doesn't return any results, it populates the
 *  thermal vote in the field g_amec->thermalcent.speed_request.
 *
 *  @return No return from this function.
 */
void amec_controller_centaur_thermal();


/**
 *  @brief  Thermal Control Loop based on DIMM temperatures.
 *
 *  This function implements a Proportional Controller for the
 *  thermal control loop based on DIMM temperatures. Although it
 *  doesn't return any results, it populates the thermal vote in
 *  the field g_amec->thermaldimm.speed_request.
 *
 *  @return No return from this function.
 */
void amec_controller_dimm_thermal();


/**
 *  @brief  Helper function to convert speed to MHz
 *
 *  This function converts a given speed in percentage (%) to a
 *  frequency in MHz.
 *
 *  @param  i_speed Input speed in percentage (0% - 100%)
 *  @param  i_fmax Maximum frequency of the system
 *
 *  @return  Equivalent frequency in MHz
 */
uint16_t amec_controller_speed2freq (const uint16_t i_speed, const uint16_t i_fmax);


void amec_controller_vrhotproc();


#endif //_AMEC_CONTROLLER_H
