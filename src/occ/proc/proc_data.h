/******************************************************************************
// @file proc_data.h
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
 *                      nguyenp   09/27/2011  Created
 *                      nguyenp   09/27/2011  Added bulk core data and proc core
 *                                            initilization code.
 *   @01                nguyenp   10/19/2011  Added fast core data code.
 *                                            Added new interfaces to gain access to
 *                                            bulk core and fast core data.
 *                                            Added macro to translate occ core id
 *                                            to hardware core id and vice versa.
 *   @rc001             rickylie  12/30/2011  Moved debug trace defines to trac.h
 *                                            remove print_core_data_sensors
 *                                            remove print_core_status
 *                                            remove print_fast_core_data
 *   @rc003             rickylie  02/03/2012  Verify & Clean Up OCC Headers & Comments
 *   @th00b             thallet   02/28/12    Added a couple enums,defines,macros
 *   @pb00E             pbavari   03/11/2012  Added correct include file
 *   @th00f             thallet   06/27/2012  Change to use Core Data struct as provided by HW team
 *   @sbpde  922027     sbroyles  04/04/2014  Add empath error core mask
 *
 *  @endverbatim
 *
 *///*************************************************************************/

#ifndef _PROC_DATA_H
#define _PROC_DATA_H

//*************************************************************************
// Includes
//*************************************************************************
//@pb00Ec - changed from common.h to occ_common.h for ODE support
#include <occ_common.h>
#include <ssx.h>
#include "rtls.h"
#include "gpe_data.h"

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Macros
//*************************************************************************
//>@01a
//Returns 0 if the specified core is not present. Otherwise, returns none-zero.
#define CORE_PRESENT(occ_core_id) \
         ((CORE0_PRESENT_MASK >> occ_core_id) & G_present_cores)

//Takes an OCC core id and converts it to a core id that
//can be used by the hardware. The caller needs to send in
//a valid core id. Since type is uchar so there is no need to check for
//case less than 0. If core id is invalid then returns unconfigured core 16.
#define CORE_OCC2HW(occ_core_id) \
        ((occ_core_id <= 15) ? G_occ2hw_core_id[occ_core_id] : 16)

//Takes a hardware core id and returns a OCC core id.
//The caller needs to send in a valid core id. Since type is uchar so
//there is no need to check for case less than 0. If core id
//is invalid then returns unconfigured core 16.
#define CORE_HW2OCC(hw_core_id) \
    ((hw_core_id <= 15) ? G_hw2occ_core_id[hw_core_id] : 16)
//<@01a

//*************************************************************************
// Defines/Enums
//*************************************************************************
#define ALL_CORES_MASK          0xffff0000
#define CORE0_PRESENT_MASK      0x80000000ul
#define CORE0_PRESENT_MASK_GPE  0x8000000000000000ull  // @th008a

#define    MAX_NUM_HW_CORES    16
#define MAX_NUM_FW_CORES    12

#define    THREADS_PER_CORE    8
#define    MAX_MEM_PARTS        4

//>@01a
#define NUM_FAST_CORE_DATA_BUFF     2
#define NUM_CORE_DATA_BUFF       7     // @th00b
#define NUM_CORE_DATA_DOUBLE_BUF 2     // @th00b
#define NUM_CORE_DATA_EMPTY_BUF  1     // @th00b

#define LO_CORES_MASK       0x7e000000
#define HI_CORES_MASK       0x007e0000
#define HW_CORES_MASK       0xffff0000
//<@01a

enum eOccProcCores
{
  CORE_0  = 0,
  CORE_1  = 1,
  CORE_2  = 2,
  CORE_3  = 3,
  CORE_4  = 4,
  CORE_5  = 5,
  CORE_6  = 6,
  CORE_7  = 7,
  CORE_8  = 8,
  CORE_9  = 9,
  CORE_10 = 10,
  CORE_11 = 11,
};  // @th00b

//*************************************************************************
// Structures
//*************************************************************************

typedef CoreData gpe_bulk_core_data_t;    // @th00f

//Processor data collect structures used for task data pointers
//gpe_req.request.parameter points to GpeGetCoreDataParms
struct bulk_core_data_task {
        uint8_t start_core;
        uint8_t current_core;
        uint8_t end_core;
        gpe_bulk_core_data_t * core_data_ptr;
        PoreFlex gpe_req;
} __attribute__ ((__packed__));
typedef struct bulk_core_data_task bulk_core_data_task_t;

//>@01a
//Only PCBS_LOCAL_PSTATE_FREQ_TARGET_STATUS_REG register is being
//collected at this time. Other register will be added when needed.
struct fast_core_data {
    uint64_t    pcbs_lpstate_freq_target_sr;
} __attribute__ ((__packed__));
typedef struct fast_core_data fast_core_data_t;

//gpe fast core data structure
struct gpe_fast_core_data {
    uint32_t    tod;
    uint32_t    reserved;
    fast_core_data_t core_data[MAX_NUM_HW_CORES];
} __attribute__ ((__packed__));
typedef struct gpe_fast_core_data gpe_fast_core_data_t;
//<@01a

//*************************************************************************
// Globals
//*************************************************************************

//Global low and high cores structures used for task data pointers
extern bulk_core_data_task_t G_low_cores;
extern bulk_core_data_task_t G_high_cores;

//Global G_present_cores is bitmask of all OCC core numbering
extern uint32_t G_present_cores;

//>@01a
//Global G_present_hw_cores is bitmask of all hardware cores
extern uint32_t G_present_hw_cores;
extern uint8_t G_occ2hw_core_id[MAX_NUM_HW_CORES];
extern uint8_t G_hw2occ_core_id[MAX_NUM_HW_CORES];
//<@01a

//AMEC needs to know when data for a core has been collected.
extern uint32_t G_updated_core_mask;

// >> @sbpde
// External reference to empath error mask
extern uint32_t G_empath_error_core_mask;
// << @sbpde

//*************************************************************************
// Function Prototypes
//*************************************************************************
/*----------------------------------------------------------------------------*/
/* Macros                                                                     */
/*----------------------------------------------------------------------------*/
//Returns 0 if the specified core is not present. Otherwise, returns none-zero.
#define CORE_PRESENT(occ_core_id) \
         ((CORE0_PRESENT_MASK >> occ_core_id) & G_present_cores)

//Returns 0 if the specified core is not updated. Otherwise, returns none-zero.
#define CORE_UPDATED(occ_core_id) \
         ((CORE0_PRESENT_MASK >> occ_core_id) & G_updated_core_mask)

//Returns 0 if the specified core is not updated. Otherwise, returns none-zero.
#define CLEAR_CORE_UPDATED(occ_core_id) \
         G_updated_core_mask &= ~(CORE0_PRESENT_MASK >> occ_core_id)

// >> @sbpde
// Evaluates to true if an empath collection error has occurred on a core
#define CORE_EMPATH_ERROR(occ_core_id) \
        ((CORE0_PRESENT_MASK >> occ_core_id) & G_empath_error_core_mask)
// << @sbpde

//Takes an OCC core id and converts it to a core id that
//can be used by the hardware. The caller needs to send in
//a valid core id. Since type is uchar so there is no need to check for
//case less than 0. If core id is invalid then returns unconfigured core 16.
#define CORE_OCC2HW(occ_core_id) \
        ((occ_core_id <= 15) ? G_occ2hw_core_id[occ_core_id] : 16)

//Takes a hardware core id and returns a OCC core id.
//The caller needs to send in a valid core id. Since type is uchar so
//there is no need to check for case less than 0. If core id
//is invalid then returns unconfigured core 16.
#define CORE_HW2OCC(hw_core_id) \
    ((hw_core_id <= 15) ? G_hw2occ_core_id[hw_core_id] : 16)
//<@01a

/*----------------------------------------------------------------------------*/
/* Collect bulk core data function                                            */
/*----------------------------------------------------------------------------*/
//Collect bulk core data for all cores in specified range
void task_core_data( task_t * i_task );

/*----------------------------------------------------------------------------*/
/* proc core initialization function                                          */
/*----------------------------------------------------------------------------*/
//Initialize structures for collecting core data.
void proc_core_init( void ) INIT_SECTION;

//>@01a
/*----------------------------------------------------------------------------*/
/* Collect fast core data function                                            */
/*----------------------------------------------------------------------------*/
//Collect fast core data for all configured cores on every tick
void task_fast_core_data( task_t * i_task );

/*----------------------------------------------------------------------------*/
/* Get bulk core data pointer                                                 */
/*----------------------------------------------------------------------------*/
//Returns a pointer to the most up-to-date bulk core data for the core
//associated with the specified OCC core id.
gpe_bulk_core_data_t * proc_get_bulk_core_data_ptr( const uint8_t i_occ_core_id );

/*----------------------------------------------------------------------------*/
/* Get fast core data pointer                                                 */
/*----------------------------------------------------------------------------*/
//Returns a pointer to the most up-to-date fast core data
gpe_fast_core_data_t * proc_get_fast_core_data_ptr( void );
//<@01a

/*----------------------------------------------------------------------------*/
/* Debug                                                                      */
/*----------------------------------------------------------------------------*/
//@rc001

//*************************************************************************
// Functions
//*************************************************************************

#endif //_PROC_DATA_H
