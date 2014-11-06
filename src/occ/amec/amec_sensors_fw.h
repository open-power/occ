/**
 *      @page ChangeLogs Change Logs
 *      @section _amec_sensors_fw_h amec_sensors_fw.h
 *      @verbatim
 *      
 *  Flag     Def/Fea    Userid    Date      Description
 *  -------- ---------- --------  --------  --------------------------------------
 *  @th00b              thallet   02/24/2012  New file
 *  @pb00E              pbavari   03/11/2012  Added correct include file
 *
 *  @endverbatim
 */

#ifndef _AMEC_SENSORS_FW_H
#define _AMEC_SENSORS_FW_H

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
// Function that updates the AMEC interim structures with AMEC State Durations
void amec_slv_update_smh_sensors(int i_smh_state, uint32_t i_duration);

// Function that updates the AMEC interim structures with GPE Engine Durations
void amec_slv_update_gpe_sensors(uint8_t i_gpe_engine);

// Function that updates the AMEC FW sensors
void amec_update_fw_sensors(void);


#endif // _AMEC_SENSORS_FW_H
