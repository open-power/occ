/******************************************************************************
// @file centaur_data.h
// @brief Centaur external functions & data.
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section  _centaur_data_h centaur_data.h
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *   @th00c             thallet   03/14/2012  Created
 *   @th013             thallet   07/17/2012  SW150201: OCC VPO: inbound scom
 *   @th016             thallet   08/13/2012  Only 4 Centaurs possible on Murano
 *   @th018  852950     thallet   09/12/2012  Added Centaur thermal readings       
 *   @th031  878471     thallet   04/15/2013  Centaur Throttles
 *   @th032             thallet   04/26/2013  Centaur Bringup Changes
 *   @th045  893135     thallet   07/26/2013  Updated for new Centaur Procedures
 *   @gm004  892961     milesg    07/25/2013  Removed centaur_init from init section
 *   @gm012  905097     milesg    10/31/2013  Fix Centaur enablement
 *   @gm013  907548     milesg    11/22/2013  Memory therm monitoring support
 *   @gm015  907601     milesg    12/06/2013  L4 Bank Delete circumvention and centaur i2c recovery
 *   @gm016  909061     milesg    12/10/2013  Support memory throttling due to temperature
 *
 *  @endverbatim
 *
 *///*************************************************************************/
 
#ifndef _CENTAUR_DATA_H
#define _CENTAUR_DATA_H

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

// Mask that is used by procedure to specify which centaurs are present
#define ALL_CENTAURS_MASK          0x000000ff              // @th016
#define ALL_CENTAURS_MASK_GPE      0x000000ff00000000ull   // @th016
// Centaur0, used by OCC
#define CENTAUR0_PRESENT_MASK      0x00000080ul
#define CENTAUR0_PRESENT_MASK_GPE  0x0000008000000000ull

// Used for specifing buffer allocations
#define NUM_CENTAUR_DATA_BUFF       MAX_NUM_CENTAURS
#define NUM_CENTAUR_DOUBLE_BUF      1     
#define NUM_CENTAUR_DATA_EMPTY_BUF  1    

// Specify a command BAR addresses for all centaur operations
#define CENTAUR_MEM_DATA_BAR        MEM_DATA_BAR_SELECT_1   // @th032

// Enum for specifying each Centaur
enum eOccCentaurs
{
  CENTAUR_0  = 0,
  CENTAUR_1  = 1,
  CENTAUR_2  = 2,
  CENTAUR_3  = 3,
  CENTAUR_4  = 4,
  CENTAUR_5  = 5,
  CENTAUR_6  = 6,
  CENTAUR_7  = 7,
};  

//*************************************************************************
// Macros
//*************************************************************************
//Returns 0 if the specified centaur is not present. Otherwise, returns none-zero.
#define CENTAUR_PRESENT(occ_cent_id) \
         ((CENTAUR0_PRESENT_MASK >> occ_cent_id) & G_present_centaurs)

//Returns 0 if the specified centaur is not updated. Otherwise, returns none-zero.
#define CENTAUR_UPDATED(occ_cent_id) \
         ((CENTAUR0_PRESENT_MASK >> occ_cent_id) & G_updated_centaur_mask)

//Returns 0 if the specified centaur is not updated. Otherwise, returns none-zero.
#define CLEAR_CENTAUR_UPDATED(occ_cent_id) \
         G_updated_centaur_mask &= ~(CENTAUR0_PRESENT_MASK >> occ_cent_id)

//Returns the bitmask for the passed in Centaur (uint32_t)
#define CENTAUR_BY_MASK(occ_cent_id) \
         ((CENTAUR0_PRESENT_MASK >> occ_cent_id) & ALL_CENTAURS_MASK)

//Returns the bitmask for the passed in Centaur (uint64_t)
#define CENTAUR_BY_MASK_GPE(occ_cent_id) \
         (((uint64_t) CENTAUR0_PRESENT_MASK_GPE >> occ_cent_id) & ALL_CENTAURS_MASK_GPE)

#define DIMM_SENSOR0 0x80

#define CENTAUR_SENSOR_ENABLED(occ_cent_id, sensor_num) \
        (G_cent_enabled_sensors.bytes[occ_cent_id] & (DIMM_SENSOR0 >> (sensor_num)))

#define MBA_CONFIGURED(occ_cent_id, mba_num) \
        (G_configured_mbas & (1 << ((occ_cent_id * 2) + mba_num)))
//*************************************************************************
// Structures
//*************************************************************************

//Centaur data collect structures used for task data pointers
struct centaur_data_task {
        uint8_t start_centaur;
        uint8_t current_centaur;     
        uint8_t end_centaur;
        uint8_t prev_centaur;       // @th013   
        MemData * centaur_data_ptr;
        PoreFlex gpe_req;
} __attribute__ ((__packed__));
typedef struct centaur_data_task centaur_data_task_t;

typedef union
{
    uint64_t bigword;
    uint32_t words[2];
    uint8_t  bytes[8];
}cent_sensor_flags_t;


//*************************************************************************
// Globals
//*************************************************************************

//Global centaur structures used for task data pointers
extern centaur_data_task_t G_centaur_data_task;

//Global is bitmask of centaurs
extern uint32_t G_present_centaurs;

//AMEC needs to know when data for a centaur has been collected.
extern uint32_t G_updated_centaur_mask;

//global bitmap of enabled dimm sensors
extern cent_sensor_flags_t G_cent_enabled_sensors;

//global bitmap of dimms that have ever gone over the error temperature
extern cent_sensor_flags_t G_dimm_overtemp_bitmap;

//global bitmap of dimms temps that have been updated
extern cent_sensor_flags_t G_dimm_temp_updated_bitmap;

//global bitmap of centaurs that have ever gone over the error temperature 
extern uint8_t G_cent_overtemp_bitmap;

//global bitmap of centaur temperatures that have been updated
extern uint8_t G_cent_temp_updated_bitmap;

//bitmap of configured MBA's (2 per centaur, lsb is centaur0/mba0)
extern uint16_t G_configured_mbas;
//*************************************************************************
// Function Prototypes
//*************************************************************************

//Collect centaur data for all centaur in specified range 
void task_centaur_data( task_t * i_task );

//Initialize structures for collecting centaur data.
//void centaur_init( void ) INIT_SECTION;
void centaur_init( void ); //gm004

//handles centaur i2c recovery and other workarounds
void cent_recovery(uint32_t i_cent);

//Returns a pointer to the most up-to-date centaur data for the centaur
//associated with the specified OCC centaur id.
MemData * cent_get_centaur_data_ptr( const uint8_t i_centaur_id );

#endif //_CENTAUR_DATA_H

