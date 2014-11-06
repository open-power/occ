/******************************************************************************
// @file centaur_control.h
// @brief Centaur external control functions & data.
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section  _centaur_control_h centaur_control.h
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *   @th031  878471     thallet   04/15/2013  Centaur Throttles
 *   @gm004  892961     milesg    07/25/2013  Removed centaur_control_init from init section
 *   @gm016  909061     milesg    12/10/2013  Support memory throttling due to temperature
 *
 *  @endverbatim
 *
 *///*************************************************************************/
 
#ifndef _CENTAUR_CONTROL_H
#define _CENTAUR_CONTROL_H

//*************************************************************************
// Includes
//*************************************************************************
#include <occ_common.h>
#include <ssx.h>
#include "rtls.h"	
#include "gpe_data.h"
#include "occ_sys_config.h"

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Structures
//*************************************************************************

typedef enum
{
    CENTAUR_CONTROL_GPE_STILL_RUNNING = 0x01,
    CENTAUR_CONTROL_RESERVED_1        = 0x02,
    CENTAUR_CONTROL_RESERVED_2        = 0x04,
    CENTAUR_CONTROL_RESERVED_3        = 0x08,
    CENTAUR_CONTROL_RESERVED_4        = 0x10,
    CENTAUR_CONTROL_RESERVED_5        = 0x20,
    CENTAUR_CONTROL_RESERVED_6        = 0x40,
    CENTAUR_CONTROL_RESERVED_7        = 0x80,
} eCentaurControlTraceFlags;

//Centaur data collect structures used for task data pointers
struct centaur_control_task {
        uint8_t  startCentaur;
        uint8_t  prevCentaur;
        uint8_t  curCentaur;     
        uint8_t  endCentaur;
        uint8_t  traceThresholdFlags;
        PoreFlex gpe_req;
} __attribute__ ((__packed__));
typedef struct centaur_control_task centaur_control_task_t;

//per mba throttle values -- gm016
typedef struct
{
   uint16_t max_n_per_mba;      //mode and OVS dependent, from config data
   uint16_t max_n_per_chip;     //mode and OVS dependent, from config data
   uint16_t min_n_per_mba;      //from config data
   uint16_t m;
} centaur_throttle_t;

//*************************************************************************
// Globals
//*************************************************************************

//Global centaur structures used for task data pointers
extern centaur_control_task_t G_centaur_control_task;

//*************************************************************************
// Function Prototypes
//*************************************************************************

//Collect centaur data for all centaur in specified range 
void task_centaur_control( task_t * i_task );

//Initialize structures for collecting centaur data.
//void centaur_control_init( void ) INIT_SECTION;
void centaur_control_init( void ); //gm004

#endif //_CENTAUR_CONTROL_H

