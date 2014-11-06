/******************************************************************************
// @file amec_sensors_centaur.h
// @brief OCC AMEC Sensors Derived from Centaur
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section amec_sensors_centaur.h AMEC_SENSORS_CENTAUR.H
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *   @th018  852950     thallet   09/12/2012  Added Centaur thermal readings       
 *
 *  @endverbatim
 *
 *///*************************************************************************/

#ifndef _AMEC_SENSORS_CENTAUR_H
#define _AMEC_SENSORS_CENTAUR_H


//*************************************************************************
// Function Prototypes
//*************************************************************************
void amec_update_centaur_sensors(uint8_t i_centaur);
void amec_update_centaur_temp_sensors(void);


#endif

