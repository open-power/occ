/******************************************************************************
 * @file amec_parm.h
 * @brief AMEC Parameter Header File
 ******************************************************************************/
/******************************************************************************
 *
 *      @page ChangeLogs Change Logs
 *      @section _amec_parm_h amec_parm.h
 *      @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *   @ly001  853751     lychen    09/17/2012  Initial Revision
 *   @ly002  860316     lychen    11/07/2012  Rename adding a core group function
 *   @gs008  894661     gjsilva   08/08/2013  Initial support for DPS-FP mode
 *   @gs009  897228     gjsilva   08/28/2013  Enablement of DPS-FP Mode
 *   @gs017  905990     gjsilva   11/13/2013  Full support for tunable parameters
 *   @gs018  907196     gjsilva   11/20/2013  Base support for soft frequency boundaries
 *   @gs025  913663     gjsilva   01/30/2014  Full fupport for soft frequency boundaries
 *
 *  @endverbatim
 *
 ******************************************************************************/

#ifndef _AMEC_PART_H
#define _AMEC_PART_H

/*----------------------------------------------------------------------------*/
/* Includes                                                                   */
/*----------------------------------------------------------------------------*/
#include <mode.h>
#include <sensor.h>
#include <occ_sys_config.h>
#include <amec_dps.h>

/*----------------------------------------------------------------------------*/
/* Constants                                                                  */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Globals                                                                    */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Defines                                                                    */
/*----------------------------------------------------------------------------*/

// Number of cores in system
#define AMEC_PART_NUM_CORES     (MAX_NUM_OCC * MAX_NUM_CORES)

// On a given OCC, only 12 partitions can be defined
#define AMEC_PART_MAX_PART      MAX_NUM_CORES

// Invalid core group ID
#define AMEC_PART_INVALID_ID    0xFF

/*----------------------------------------------------------------------------*/
/* Typedef / Enum                                                             */
/*----------------------------------------------------------------------------*/

typedef struct amec_part
{
    ///Flag to indicate if the core group should follow the system power policy
    BOOLEAN                     follow_sysmode;
    ///EnergyScale power savings policy
    OCC_INTERNAL_MODE           es_policy;
    ///Total number of cores in this core group.
    uint8_t                     ncores;
    ///List of cores group. Indices 0 to ncores-1 are valid. Valid values: 0 to
    ///AMEC_PART_NUM_CORES-1.
    uint8_t                     core_list[AMEC_PART_NUM_CORES];
    ///Partition ID
    uint8_t                     id;
    ///Valid bit (=1 in use, =0 not in use)
    uint8_t                     valid;
    ///Soft min frequency boundary sent by PHYP
    uint16_t                    soft_fmin;
    ///Soft max frequency boundary sent by PHYP
    uint16_t                    soft_fmax;
    ///Power saving state
    amec_dps_t                  dpsalg;
    ///slack utilization sensor
    sensor_t                    util2msslack;
} amec_part_t;

/**
 * @struct amec_part_config_t
 * @brief Partition Configuration Structure
 *
 * Main structure that contains the partition configuration for PLPM work.
 */
typedef struct amec_part_config
{
    ///Data structure holding core to partition mapping. The value 
    ///AMEC_PART_INVALID_ID means that a core is not mapped to a partition. A
    ///value less than that means core is mapped to that partition index.
    uint16_t                    core2part[MAX_NUM_CORES];

    ///Data structure holding all active partitions (core groups)
    amec_part_t                 part_list[AMEC_PART_MAX_PART];
} amec_part_config_t;

/*----------------------------------------------------------------------------*/
/* Function Prototypes                                                        */
/*----------------------------------------------------------------------------*/

/**
 * @brief Given a core, return a valid partition that owns it, or NULL.
 * @param i_config Pointer to partition configuration
 * @param i_core_index The core number 0 to AMEC_PART_NUM_CORES - 1
 *
 * @return Valid pointer to partition that owns core, else NULL.
 */
amec_part_t* amec_part_find_by_core(amec_part_config_t* i_config,
                                    const uint16_t i_core_index);

/**
 * @brief Add a core group.
 *
 * This function adds a new core group and should only be called within the
 * AMEC ISR.
 *  
 * @param i_id Core group ID that needs to be added
 * 
 * @return void
 */
void amec_part_add(uint8_t i_id); // @ly002c

/**
 * @brief Add a core group.
 * 
 * @return void
 */
void amec_part_init(void);

/**
 * @brief Update the parameter values depending on the DPS mode 
 *        that has been selected (Favor Energy or Favor
 *        Performance).
 * 
 * @param io_part A valid partition descriptor
 * 
 * @return void
 */
void amec_part_update_dps_parameter(amec_part_t* io_part);

/**
 * @brief Update the internal performance settings for those 
 *        cores that belong to the input partition.
 * 
 * @param io_part A valid partition descriptor 
 *  
 * @return void 
 */
void amec_part_update_perf_settings(amec_part_t* io_part);

/**
 * @brief Update the power mode on all core groups that are following the 
 * system mode. 
 * 
 * @param i_occ_internal_mode OCC internal system mode 
 *  
 * @return void 
 */
void AMEC_part_update_sysmode_policy(OCC_MODE i_occ_internal_mode);

/**
 * @brief Overwrite the tunable parameters used by the DPS algorithms 
 * whenever the Master OCC sends them. 
 * 
 * @param void
 *  
 * @return void 
 */
void AMEC_part_overwrite_dps_parameters(void);

#endif //_AMEC_PART_H
