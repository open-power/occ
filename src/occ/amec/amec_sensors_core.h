/**
 *      @page ChangeLogs Change Logs
 *      @section _amec_sensors_core_h amec_sensors_core.h
 *      @verbatim
 *      
 *  Flag     Def/Fea    Userid    Date      Description
 *  -------- ---------- --------  --------  --------------------------------------
 *  @th00b              thallet   02/24/2012  New file
 *  @pb00E              pbavari   03/11/2012  Added correct include file
 *  @gs024   912700     gjsilva   01/22/2014  Identify fast/deep winkle states
 *  @gs028   917695     gjsilva   03/04/2014  Check if cores are in a sleep state
 *           917695     gjsilva   03/05/2014  Changes from code review
 *
 *  @endverbatim
 */

#ifndef _AMEC_SENSORS_CORE_H
#define _AMEC_SENSORS_CORE_H

/*----------------------------------------------------------------------------*/
/* Includes                                                                   */
/*----------------------------------------------------------------------------*/
//@pb00Ec - changed from common.h to occ_common.h for ODE support
#include <occ_common.h>
#include <ssx.h>
#include <ssx_app_cfg.h>
#include "amec_external.h"
#include <occ_sys_config.h>


/*----------------------------------------------------------------------------*/
/* Defines/Constants                                                          */
/*----------------------------------------------------------------------------*/
// See bit definition of PM State History Register for OCC
#define OCC_PM_STATE_MASK      0x1F000000  //Mask bits 0:2 of the register
#define OCC_PAST_FAST_SLEEP    0x18000000  //Core has been in fast sleep
#define OCC_PAST_DEEP_SLEEP    0x1C000000  //Core has been in deep sleep
#define OCC_PAST_FAST_WINKLE   0x1E000000  //Core has been in fast winkle
#define OCC_PAST_DEEP_WINKLE   0x1F000000  //Core has been in deep winkle
#define OCC_PAST_CORE_CLK_STOP 0x08000000  //Core has been in an idle state with core clocks stopped

/*----------------------------------------------------------------------------*/
/* Structures                                                                 */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Function Declarations                                                      */
/*----------------------------------------------------------------------------*/
void amec_update_fast_core_data_sensors(void);

void amec_update_proc_core_sensors(uint8_t i_core);



#endif // _AMEC_SENSORS_CORE_H
