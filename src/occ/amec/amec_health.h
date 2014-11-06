/**
 * @file amec_health.h
 * @brief Header file for OCC Health Monitor.
 *
 *
*/
/**
 *      @page ChangeLogs Change Logs
 *      @section _cmdh_mnfg_intf_h cmdh_mnfg_intf.h
 *      @verbatim
 *
 *  Flag     Def/Fea    Userid    Date        Description
 *  -------- ---------- --------  ---------   ----------------------------------
 *  @gs012   903325     gjsilva   09/26/2013  Created
 *  @gm013   907548     milesg    11/22/2013  Memory therm monitoring support
 *  @gs021   909855     gjsilva   12/18/2013  Support for processor OT condition
 *  @gs023   912003     gjsilva   01/16/2014  Generate VRHOT signal and control loop
 *
 *  @endverbatim
 */
#ifndef AMEC_HEALTH_H
#define AMEC_HEALTH_H

//*************************************************************************
// Includes
//*************************************************************************
#include "cmdh_fsp_cmds_datacnfg.h"

//*************************************************************************
// Defines/Enums
//*************************************************************************
// Error threshold check is done every 16msec. Error timer is 5x16 = 80msec
#define AMEC_HEALTH_ERROR_TIMER 5

/*******************************************************************/
/* Function Definitions                                            */
/*******************************************************************/

void amec_health_check_proc_temp(void);
void amec_health_check_proc_timeout(void);
void amec_health_check_proc_vrhot();
void amec_health_check_cent_temp(void);
void amec_health_check_cent_timeout(void);
void amec_health_check_dimm_temp(void);
void amec_health_check_dimm_timeout(void);

#endif
