/******************************************************************************
// @file amec_data.h
// @brief AMEC Data header file
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section _amec_data_h amec_data.h
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *   @th015             thallet   08/03/2012  New file
 *   @gs001             gsilva    08/03/2012  New file
 *   @fk001  879727     fmkassem  04/16/2013  OCC powercap support
 *   @gs010  899888     gjsilva   09/24/2013  Process data format 0x13 from TMGT
 *   @gs015  905166     gjsilva   11/04/2013  Full support for IPS function
 *
 *  @endverbatim
 *
 *///*************************************************************************/
#ifndef _AMEC_DATA_H
#define _AMEC_DATA_H

//*************************************************************************
// Includes
//*************************************************************************
#include <occ_common.h>
#include <ssx.h>
#include <ssx_app_cfg.h>
#include <amec_smh.h>
#include <errl.h>
#include <mode.h>

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************

//*************************************************************************
// Structures
//*************************************************************************

//*************************************************************************
// Globals
//*************************************************************************

//*************************************************************************
// Function Prototypes
//*************************************************************************

//*************************************************************************
// Functions
//*************************************************************************

// This is used to change the current Freq data AMEC
// is using
errlHndl_t AMEC_data_write_fcurr(const OCC_MODE i_mode);

// This is used to store the thermal thresholds AMEC is using
errlHndl_t AMEC_data_write_thrm_thresholds(const OCC_MODE i_mode);

// This is used to store the IPS config data AMEC is using
errlHndl_t AMEC_data_write_ips_cnfg(void);

// This is used to notify AMEC that there is a change
// to the configuration data
errlHndl_t AMEC_data_change(const uint32_t i_data_mask);

// Writes pcap data sent by master to slave accessable structure.
void amec_data_write_pcap();

#endif

