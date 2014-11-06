/******************************************************************************
// @file cmdh_fsp_cmds_datacnfg.h
// @brief Command Handling for FSP Communication.
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section _cmdh_fsp_cmds_datacnfg_h cmdh_fsp_cmds_datacnfg_h
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *   @th022             thallet   10/05/2012  Created
 *   @at013  878755     alvinwan  04/17/2013  OCC power capping implementation
 *   @at014  882077     alvinwan  05/09/2013  Support APSS and System Config data from TMGT
 *   @at015  885884     alvinwan  06/10/2013  Support Observation/Active state change
 *   @jh004  889884     joshych   07/24/2013  Support CPM param and updated frequency packet
 *   @th046  894648     thallet   08/08/2013  Fix Coreq problem with TMGT
 *   @gs010  899888     gjsilva   09/24/2013  Process data format 0x13 from TMGT
 *   @rt001  902613     tapiar    10/14/2013  Process data format 0x11 from TMGT
 *   @gm012  905097     milesg    10/31/2013  support mem throttle & mem config packets
 *   @gs015  905166     gjsilva   11/04/2013  Full support for IPS function
 *   @fk002  905632     fmkassem  11/05/2013  Remove CriticalPathMonitor code
 *   @fk008  942864     fmkassem  09/15/2014  BMC - APSS config data
 *   @gs042  942940     gjsilva   10/24/2014  Support for data packets in BMC-based systems
 *   @gs043  943177     gjsilva   10/30/2014  Support for mem data packets in BMC-based systems
 *
 *  @endverbatim
 *
 *///*************************************************************************/

#ifndef _CMDH_FSP_CMDS_DATACNFG_H
#define _CMDH_FSP_CMDS_DATACNFG_H

//*************************************************************************
// Includes
//*************************************************************************
#include "ssx.h"		
#include "cmdh_service_codes.h" 
#include "errl.h"             
#include "trac.h"
#include "rtls.h"
#include "occ_common.h"
#include "state.h"
#include "cmdh_fsp.h"
#include "gpsm.h"
#include "pstates.h"
#include "cmdh_fsp_cmds.h"
#include "apss.h"  // @at014a

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************

//---------------------------------------------------------
// ConfigData Store Command
//---------------------------------------------------------

/**
 * @enum eConfigDataFormatVersion
 * @brief Enum of the various CnfgData command formats that 
 *        are sent to OC over the TMGT<->OCC interface.  
 */
typedef enum
{
   DATA_FORMAT_PSTATE_SUPERSTRUCTURE = 0x01,
   DATA_FORMAT_FREQ                  = 0x02,
   DATA_FORMAT_SET_ROLE              = 0x03,
   DATA_FORMAT_APSS_CONFIG           = 0x04,
   DATA_FORMAT_MEM_CFG               = 0x05,
   DATA_FORMAT_POWER_CAP             = 0x07,  // @at013a
   DATA_FORMAT_SYS_CNFG              = 0x0f,  // @at014a
   DATA_FORMAT_IPS_CNFG              = 0x11,  // @rt001a
   DATA_FORMAT_MEM_THROT             = 0x12,
   DATA_FORMAT_THRM_THRESHOLDS       = 0x13,  // @gs010a
   DATA_FORMAT_CLEAR_ALL             = 0xff,  // @at015a
} eConfigDataFormatVersion;

/**
 * @enum eConfigDataPriorityMask
 * @brief Enum of the various Cnfg Data Masks that are used
 *        to signal that OCC has received cnfg data
 */
typedef enum
{
   DATA_MASK_PSTATE_SUPERSTRUCTURE = 0x00000001,
   DATA_MASK_FREQ_PRESENT          = 0x00000002,
   DATA_MASK_SET_ROLE              = 0x00000004,
   DATA_MASK_APSS_CONFIG           = 0x00000008,
   DATA_MASK_PCAP_PRESENT          = 0x00000010,  // @at013a
   DATA_MASK_SYS_CNFG              = 0x00000020,  // @at014a
   //0x00000040 not assigned.
   DATA_MASK_THRM_THRESHOLDS       = 0x00000080,  // @gs010a
   DATA_MASK_IPS_CNFG              = 0x00000100,  // @rt001a
   DATA_MASK_MEM_CFG               = 0x00000200,
   DATA_MASK_MEM_THROT             = 0x00000400,
} eConfigDataPriorityMask;

typedef enum
{
    DATA_FRU_PROC               = 0x00,
    DATA_FRU_CENTAUR            = 0x01,
    DATA_FRU_DIMM               = 0x02,
    DATA_FRU_VRM                = 0x03,
    DATA_FRU_MAX,
} eConfigDataFruType;

/**
 * @struct cmdh_store_cnfgdata_pstatess_t
 * @brief Used by TMGT to send OCC the PstateSuperStruct
 */
typedef struct __attribute__ ((packed))
{
    struct    cmdh_fsp_cmd_header;
    uint8_t   format;
    uint8_t   reserved[3];
    PstateSuperStructure pstatess;
}cmdh_store_cnfgdata_pstatess_t;  // @th010
#define CMDH_CNFGDATA_PSTATESS_DATALEN (sizeof(PstateSuperStructure) + 4)

/**
 * @struct cmdh_store_mode_freqs_t
 * @brief Used by TMGT to send OCC the frequencies for each mode.
 */
typedef struct __attribute__ ((packed)) // @jh004c
{
    struct cmdh_fsp_cmd_header;
    uint8_t format;
    uint8_t version;
}cmdh_store_mode_freqs_t;


/// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
/// TODO: START of HACK for 889884
/// 

/**
 * @struct cmdh_store_mode_freqs_old_t
 * @brief Old way used by TMGT to send OCC the frequencies for each mode.
 */
typedef struct __attribute__ ((packed))
{
    struct cmdh_fsp_cmd_header;
    uint8_t format;
    uint8_t version;
    uint8_t mode_count;
}cmdh_store_mode_freqs_old_t;

///
/// TODO: END of HACK
/// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

/**
 * @struct cmdh_set_role_t
 * @brief Used by TMGT to tell OCC if it is a master or not
 */
typedef struct __attribute__ ((packed))
{
    struct cmdh_fsp_cmd_header;
    uint8_t format;
    uint8_t role;
    uint8_t reserved[2];
}cmdh_set_role_t;

/**
 * @struct apss_cfg_adc_t
 * @brief Used by TMGT to send OCC the APSS ADC Config Data
 */
//@fk008c
typedef struct  __attribute__ ((packed))
{
  uint8_t  assignment;
  uint8_t  gnd_select;
  uint32_t gain;
  uint32_t offset;
} apss_cfg_adc_v00_t; //Used by FSP

//@fk008a
typedef struct  __attribute__ ((packed))
{
  uint8_t  assignment;
  uint16_t ipmisensorId;
  uint8_t  gnd_select;
  uint32_t gain;
  uint32_t offset;
} apss_cfg_adc_v10_t; //Used by Habanero

/**
 * @struct apss_cfg_gpio_t
 * @brief Used by TMGT to send OCC the GPIO Config Data
 */
typedef struct  __attribute__ ((packed))
{
  uint8_t  mode;
  uint8_t  __reserved;
  uint8_t  assignment[8];
} apss_cfg_gpio_t; 

/**
 * @struct cmdh_apss_config_t
 * @brief Used by TMGT to send OCC the full APSS config data.
 */
//@fk008c
typedef struct __attribute__ ((packed))
{
    struct cmdh_fsp_cmd_header;
    uint8_t              format;
    uint8_t              version;     // @at014c
    uint8_t              reserved[2]; // @at014c
    apss_cfg_adc_v00_t   adc[MAX_APSS_ADC_CHANNELS]; // @at014c
    apss_cfg_gpio_t      gpio[MAX_APSS_GPIO_PORTS];  // @at014c
}cmdh_apss_config_v00_t; //Used by FSP

//@fk008a
typedef struct __attribute__ ((packed))
{
    struct cmdh_fsp_cmd_header;
    uint8_t              format;
    uint8_t              version;
    uint8_t              reserved[2];
    apss_cfg_adc_v10_t   adc[MAX_APSS_ADC_CHANNELS];
    apss_cfg_gpio_t      gpio[MAX_APSS_GPIO_PORTS];
}cmdh_apss_config_v10_t; //Used by Habanero

// @at013a - start
/**
 * @struct cmdh_pcap_config_data_t
 * @brief Used by TMGT to send OCC the PCAP config data.
 */
typedef struct __attribute__ ((packed))
{
    uint16_t current_pcap;     // Node power cap requested by customer (AEM) in 1W units
    uint16_t soft_min_pcap;    // Minimum node power cap allowed in 1W units
    uint16_t hard_min_pcap;    // Minimum guaranteed node power cap in 1W units
    uint16_t max_pcap;         // Maximum customer settable node power cap in 1W units
    uint16_t oversub_pcap;     // Node power cap to be used for oversubscripion in 1W units
    uint16_t system_pcap;      // Fixed node power cap required by the system in 1W units
    uint8_t  unthrottle;       // Only used on ITEs -- is indicated from CMM
} cmdh_pcap_config_data_t;

/**
 * @struct cmdh_pcap_config_t
 * @brief Used by TMGT to send OCC the PCAP config data.
 */
typedef struct __attribute__ ((packed))
{
    struct cmdh_fsp_cmd_header;
    uint8_t              format;
    uint8_t              version;
    cmdh_pcap_config_data_t   pcap_config;
}cmdh_pcap_config_t;
// @at013a - end

typedef struct __attribute__ ((packed))
{
    uint16_t min_pcap;        // Minimum guaranteed node power cap in 1W units
    uint16_t sys_max_pcap;    // Fixed node power cap required by the system in 1W units
    uint16_t oversub_pcap;    // Node power cap to be used for oversubscripion in 1W units
} cmdh_pcap_config_data_v10_t;

typedef struct __attribute__ ((packed))
{
    struct cmdh_fsp_cmd_header;
    uint8_t              format;
    uint8_t              version;
    cmdh_pcap_config_data_v10_t   pcap_config;
}cmdh_pcap_config_v10_t;

// @at014a - start
/**
 * @struct cmdh_sys_config_data_t
 * @brief Used by TMGT to send OCC the System config data.
 */
typedef struct __attribute__ ((packed))
{
    uint8_t  system_type;     // OCC usage of this byte is TBD
    uint64_t proc_huid;       // Processor HUID
    uint64_t backplane_huid;  // Backplane HUID
    uint64_t apss_huid;       // APSS HUID
    uint64_t dpss_huid;       // DPSS HUID
} cmdh_sys_config_data_t;

/**
 * @struct cmdh_sys_config_t
 * @brief Used by TMGT to send OCC the system config data.
 */
typedef struct __attribute__ ((packed))
{
    struct cmdh_fsp_cmd_header;
    uint8_t              format;
    uint8_t              version;
    cmdh_sys_config_data_t   sys_config;
}cmdh_sys_config_t;
// @at014a - end

typedef struct __attribute__ ((packed))
{
    uint8_t  system_type;     // General system type
    uint16_t sensor_id[25];   // FIXME: Probably we need to break down into individual fields
    uint16_t backplane_sid;   // Backplane Sensor ID
    uint16_t apss_sid;        // APSS Sensor ID
} cmdh_sys_config_data_v10_t;

typedef struct __attribute__ ((packed))
{
    struct cmdh_fsp_cmd_header;
    uint8_t              format;
    uint8_t              version;
    cmdh_sys_config_data_v10_t   sys_config;
}cmdh_sys_config_v10_t;

// @rt001a - start
/**
 * @struct cmdh_ips_config_data_t
 * @brief Used by TMGT to send OCC the IPS config data.
 */
typedef struct __attribute__ ((packed))
{
    uint8_t     iv_ipsEnabled;          // Idle Power Save Enabled (0 or 1)
    uint16_t    iv_delayTimeforEntry;   // Delay Time in seconds to enter Idle Power
    uint8_t     iv_utilizationForEntry; // Utilization threshold percentage to enter Idle Power
    uint16_t    iv_delayTimeforExit;    // Delay Time in seconds to exit Idle Power
    uint8_t     iv_utilizationForExit;  // Utilization threshold percentage to exit Idle Power

} cmdh_ips_config_data_t;

/**
 * @struct cmdh_ips_config_t
 * @brief Used by TMGT to send OCC the IPS config data.
 */
typedef struct __attribute__ ((packed))
{
    struct                  cmdh_fsp_cmd_header;
    uint8_t                 iv_format;
    uint8_t                 iv_version;
    cmdh_ips_config_data_t  iv_ips_config;
}cmdh_ips_config_t;

extern cmdh_ips_config_data_t   G_ips_config_data;

// @rt001a - end


/**
 * @struct cmdh_thrm_thresholds_set_t
 * @brief Used by TMGT to send OCC thermal control thresholds
 */
typedef struct __attribute__ ((packed))
{
    uint8_t              fru_type;
    uint8_t              t_control;
    uint8_t              dvfs;
    uint8_t              error;
    uint8_t              pm_t_control;
    uint8_t              pm_dvfs;
    uint8_t              pm_error;
    uint8_t              acoustic_t_control;
    uint8_t              acoustic_dvfs;
    uint8_t              acoustic_error;
    uint8_t              warning;
    uint8_t              warning_reset;
    uint8_t              error_count;
    uint8_t              sample_time;
    uint8_t              sample_error_count;
    uint8_t              max_read_timeout;
    uint16_t             t_inc_zone1;
    uint16_t             t_inc_zone2;
    uint16_t             t_inc_zone3;
    uint16_t             t_inc_zone4;
    uint16_t             t_inc_zone5;
    uint16_t             t_inc_zone6;
    uint16_t             t_inc_zone7;
    uint16_t             t_inc_zone8;
}cmdh_thrm_thresholds_set_t;

/**
 * @struct cmdh_thrm_thresholds_t
 * @brief Used by TMGT to send OCC thermal control thresholds
 */
typedef struct __attribute__ ((packed))
{
    struct cmdh_fsp_cmd_header;
    uint8_t                    format;
    uint8_t                    version;
    uint8_t                    fan_control_loop_time;
    uint8_t                    num_data_sets;
    cmdh_thrm_thresholds_set_t data[DATA_FRU_MAX];
}cmdh_thrm_thresholds_t;

typedef struct __attribute__ ((packed))
{
    uint8_t              fru_type;
    uint8_t              dvfs;
    uint8_t              error;
    uint8_t              max_read_timeout;
}cmdh_thrm_thresholds_set_v10_t;

typedef struct __attribute__ ((packed))
{
    struct cmdh_fsp_cmd_header;
    uint8_t              format;
    uint8_t              version;
    uint8_t              num_data_sets;
    cmdh_thrm_thresholds_set_v10_t data[DATA_FRU_MAX];
}cmdh_thrm_thresholds_v10_t;


/**
 * @struct cmdh_mem_cfg_header_t
 * @brief header data for mem cfg packet
 */
typedef struct __attribute__ ((packed))
{
    struct cmdh_fsp_cmd_header;
    uint8_t                    format;
    uint8_t                    version;
    uint8_t                    num_data_sets;
}cmdh_mem_cfg_header_t;

/**
 * @struct cmdh_mem_cfg_data_set_t
 * @brief Maps an HUID to a centaur or dimm
 * 0xFF for dimm means this is for a centaur
 */
typedef struct __attribute__ ((packed))
{
    uint64_t                   huid;
    uint8_t                    centaur_num;
    uint8_t                    dimm_num;
    uint16_t                   reserved;
}cmdh_mem_cfg_data_set_t;

/**
 * @struct cmdh_mem_cfg_t
 * @brief config packet definition used by TMGT to
 * send HUID mappings for centaurs and dimms
 */
typedef struct __attribute__ ((packed))
{
    cmdh_mem_cfg_header_t      header;
    cmdh_mem_cfg_data_set_t    data_set[1];
}cmdh_mem_cfg_t;

typedef struct __attribute__ ((packed))
{
    uint32_t                   reserved;
    uint16_t                   hw_sensor_id;
    uint16_t                   temp_sensor_id;
    uint8_t                    centaur_num;
    uint8_t                    dimm_num;
    uint16_t                   reserved2;
}cmdh_mem_cfg_data_set_v10_t;

typedef struct __attribute__ ((packed))
{
    cmdh_mem_cfg_header_t       header;
    cmdh_mem_cfg_data_set_v10_t data_set[1];
}cmdh_mem_cfg_v10_t;


/**
 * @struct cmdh_mem_throt_header_t
 * @brief header data for mem throttle packet
 */
typedef struct __attribute__ ((packed))
{
    struct cmdh_fsp_cmd_header;
    uint8_t                    format;
    uint8_t                    version;
    uint8_t                    num_data_sets;
}cmdh_mem_throt_header_t;

/**
 * @struct cmdh_mem_throt_data_set_t
 * @brief provides memory throttle min and max values
 */
typedef struct __attribute__ ((packed))
{
    uint8_t                    centaur_num;
    uint8_t                    mba_num;
    uint16_t                   min_ot_n_per_mba;
    uint16_t                   nom_n_per_mba;
    uint16_t                   nom_n_per_chip;
    uint16_t                   turbo_n_per_mba;
    uint16_t                   turbo_n_per_chip;
    uint16_t                   ovs_n_per_mba;
    uint16_t                   ovs_n_per_chip;
}cmdh_mem_throt_data_set_t;

/**
 * @struct cmdh_mem_throt_t
 * @brief config packet definition used by TMGT to
 * send mem throttle min/max settings. 
 */
typedef struct __attribute__ ((packed))
{
    cmdh_mem_throt_header_t      header;
    cmdh_mem_throt_data_set_t    data_set[1];
}cmdh_mem_throt_t;

typedef struct __attribute__ ((packed))
{
    uint8_t              centaur_num;
    uint8_t              mba_num;
    uint16_t             min_ot_n_per_mba;
    uint16_t             redupwr_n_per_mba;
    uint16_t             redupwr_n_per_chip;
    uint16_t             ovs_n_per_mba;
    uint16_t             ovs_n_per_chip;
}cmdh_mem_throt_data_set_v10_t;

typedef struct __attribute__ ((packed))
{
    cmdh_mem_throt_header_t       header;
    cmdh_mem_throt_data_set_v10_t data_set[1];
}cmdh_mem_throt_v10_t;


/**
 * @struct data_cnfg_t
 * @brief Used to mark present the config data TMGT has sent us.
 */
typedef struct data_cnfg
{
  uint32_t                      data_mask;
  cmdh_thrm_thresholds_t        thrm_thresh;
} data_cnfg_t;


/*******************************************************************/
/* Function Definitions                                            */
/*******************************************************************/

errlHndl_t DATA_store_cnfgdata (const cmdh_fsp_cmd_t * i_cmd_ptr,
                                    cmdh_fsp_rsp_t * i_rsp_ptr);

uint8_t DATA_request_cnfgdata ();

uint32_t DATA_get_present_cnfgdata ();

errlHndl_t DATA_get_thrm_thresholds(cmdh_thrm_thresholds_t **o_thrm_thresh);

errlHndl_t DATA_get_ips_cnfg(cmdh_ips_config_data_t **o_ips_cnfg);

#endif

