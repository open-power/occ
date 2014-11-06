/**
 *      @page ChangeLogs Change Logs
 *      @section _amec_sensors_power_h amec_sensors_power.h
 *      @verbatim
 *      
 *  Flag     Def/Fea    Userid    Date      Description
 *  -------- ---------- --------  --------  --------------------------------------
 *  @th00b              thallet   02/24/2012  New file
 *  @pb00E              pbavari   03/11/2012  Added correct include file
 *  @gs027   918066     gjsilva   03/12/2014  Misc functions from ARL
 *
 *  @endverbatim
 */

#ifndef _AMEC_SENSORS_POWER_H
#define _AMEC_SENSORS_POWER_H

/*----------------------------------------------------------------------------*/
/* Includes                                                                   */
/*----------------------------------------------------------------------------*/
//@pb00Ec - changed from common.h to occ_common.h for ODE support
#include <occ_common.h>
#include <ssx.h>
#include <ssx_app_cfg.h>
#include "amec_external.h"

/*----------------------------------------------------------------------------*/
/* Defines/Constants                                                          */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* Function Declarations                                                      */
/*----------------------------------------------------------------------------*/

// Function that is called by AMEC State Machine that will update the AMEC
// sensors for data that comes from the APSS (Power Data from APSS ADCs)
void amec_update_apss_sensors(void);

// Function that is called by AMEC State Machine that will update the AMEC
// sensors for data that comes from the SPIVID chip (VR_FAN, SoftOC)
void amec_update_vrm_sensors(void);

// Function that is called by AMEC State Machine that will update the AMEC
// sensors for external voltage measurement
void amec_update_external_voltage(void);

#endif // _AMEC_SENSORS_POWER_H
