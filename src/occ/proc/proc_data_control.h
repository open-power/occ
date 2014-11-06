/******************************************************************************
// @file proc_data_control.h
// @brief Data codes for proc component.
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section  _proc_data_h proc_data.h
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *   @th010             thallet   07/11/2012  Created
 *   @th015             thallet   08/03/2012  Function to set core Pstates
 *   @gm025  915973     milesg    02/14/2014  Full support for sapphire (KVM) mode
 *
 *  @endverbatim
 *
 *///*************************************************************************/
 
#ifndef _PROC_DATA_CONTROL_H
#define _PROC_DATA_CONTROL_H

//*************************************************************************
// Includes
//*************************************************************************
#include <occ_common.h>
#include <ssx.h>
#include "rtls.h"	
#include "gpe_control.h"

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Macros
//*************************************************************************

// Initialze the structures used by the GPE
void proc_core_data_control_init( void );

// Task that sets the PMCR, PMBR, PMICR
void task_core_data_control( task_t * i_task );

// Function to demonstrate setting Pstates to all cores
void proc_set_pstate_all(Pstate i_pstate);

// Function to demonstrate setting Pstates one core
void proc_set_core_pstate(Pstate i_pstate, uint8_t i_core);

// Sets the Pmin/Pmax clip values for one core
void proc_set_core_bounds(Pstate i_pmin, Pstate i_pmax, uint8_t i_core);
#endif
