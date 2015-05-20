/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/cmdh/cmdh_fsp_cmds_datacnfg.h $                       */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2015                        */
/* [+] International Business Machines Corp.                              */
/*                                                                        */
/*                                                                        */
/* Licensed under the Apache License, Version 2.0 (the "License");        */
/* you may not use this file except in compliance with the License.       */
/* You may obtain a copy of the License at                                */
/*                                                                        */
/*     http://www.apache.org/licenses/LICENSE-2.0                         */
/*                                                                        */
/* Unless required by applicable law or agreed to in writing, software    */
/* distributed under the License is distributed on an "AS IS" BASIS,      */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or        */
/* implied. See the License for the specific language governing           */
/* permissions and limitations under the License.                         */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */

#ifndef _CMDH_FSP_CMDS_DATACNFG_H
#define _CMDH_FSP_CMDS_DATACNFG_H

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
#include "apss.h"

// Enum of the various CnfgData command formats that
// are sent to OCC over the TMGT<->OCC interface.
typedef enum
{
   DATA_FORMAT_PSTATE_SUPERSTRUCTURE = 0x01,
   DATA_FORMAT_FREQ                  = 0x02,
   DATA_FORMAT_SET_ROLE              = 0x03,
   DATA_FORMAT_APSS_CONFIG           = 0x04,
   DATA_FORMAT_MEM_CFG               = 0x05,
   DATA_FORMAT_POWER_CAP             = 0x07,
   DATA_FORMAT_SYS_CNFG              = 0x0f,
   DATA_FORMAT_IPS_CNFG              = 0x11,
   DATA_FORMAT_MEM_THROT             = 0x12,
   DATA_FORMAT_THRM_THRESHOLDS       = 0x13,
   DATA_FORMAT_VOLT_UPLIFT           = 0x20,
   DATA_FORMAT_WOF_CORE_FREQ         = 0x30,
   DATA_FORMAT_WOF_VRM_EFF           = 0x31,
   DATA_FORMAT_CLEAR_ALL             = 0xff,
} eConfigDataFormatVersion;

// Enum of the various Cnfg Data Masks that are used
// to signal that OCC has received cnfg data
typedef enum
{
   DATA_MASK_PSTATE_SUPERSTRUCTURE = 0x00000001,
   DATA_MASK_FREQ_PRESENT          = 0x00000002,
   DATA_MASK_SET_ROLE              = 0x00000004,
   DATA_MASK_APSS_CONFIG           = 0x00000008,
   DATA_MASK_PCAP_PRESENT          = 0x00000010,
   DATA_MASK_SYS_CNFG              = 0x00000020,
   //0x00000040 not assigned.
   DATA_MASK_THRM_THRESHOLDS       = 0x00000080,
   DATA_MASK_IPS_CNFG              = 0x00000100,
   DATA_MASK_MEM_CFG               = 0x00000200,
   DATA_MASK_MEM_THROT             = 0x00000400,
   DATA_MASK_VOLT_UPLIFT           = 0x00000800,
} eConfigDataPriorityMask;

typedef enum
{
    DATA_FRU_PROC               = 0x00,
    DATA_FRU_CENTAUR            = 0x01,
    DATA_FRU_DIMM               = 0x02,
    DATA_FRU_VRM                = 0x03,
    DATA_FRU_MAX,
} eConfigDataFruType;

// Set OCC Role Masks
#define OCC_ROLE_MASTER_MASK        0x01
#define OCC_ROLE_FIR_MASTER_MASK    0x40

// Used by TMGT to send OCC the PstateSuperStruct
typedef struct __attribute__ ((packed))
{
    struct    cmdh_fsp_cmd_header;
    uint8_t   format;
    uint8_t   reserved[3];
    PstateSuperStructure pstatess;
}cmdh_store_cnfgdata_pstatess_t;
#define CMDH_CNFGDATA_PSTATESS_DATALEN (sizeof(PstateSuperStructure) + 4)

// Used by TMGT to send OCC the frequencies for each mode.
typedef struct __attribute__ ((packed))
{
    struct cmdh_fsp_cmd_header;
    uint8_t format;
    uint8_t version;
}cmdh_store_mode_freqs_t;

// Old way used by TMGT to send OCC the frequencies for each mode.
typedef struct __attribute__ ((packed))
{
    struct cmdh_fsp_cmd_header;
    uint8_t format;
    uint8_t version;
    uint8_t mode_count;
}cmdh_store_mode_freqs_old_t;

// Used by TMGT to tell OCC if it is a master or not
typedef struct __attribute__ ((packed))
{
    struct cmdh_fsp_cmd_header;
    uint8_t format;
    uint8_t role;
    uint8_t reserved[2];
}cmdh_set_role_t;

// Used by TMGT to send OCC the APSS ADC Config Data
typedef struct  __attribute__ ((packed))
{
  uint8_t  assignment;
  uint8_t  gnd_select;
  uint32_t gain;
  uint32_t offset;
} apss_cfg_adc_v00_t; //Used by FSP

typedef struct  __attribute__ ((packed))
{
  uint8_t  assignment;
  uint16_t ipmisensorId;
  uint8_t  gnd_select;
  uint32_t gain;
  uint32_t offset;
} apss_cfg_adc_v10_t; //Used by Habanero

// Used by TMGT to send OCC the GPIO Config Data
typedef struct  __attribute__ ((packed))
{
  uint8_t  mode;
  uint8_t  __reserved;
  uint8_t  assignment[8];
} apss_cfg_gpio_t;

// Used by TMGT to send OCC the full APSS config data.
typedef struct __attribute__ ((packed))
{
    struct cmdh_fsp_cmd_header;
    uint8_t              format;
    uint8_t              version;
    uint8_t              reserved[2];
    apss_cfg_adc_v00_t   adc[MAX_APSS_ADC_CHANNELS];
    apss_cfg_gpio_t      gpio[MAX_APSS_GPIO_PORTS];
}cmdh_apss_config_v00_t; // Used by FSP

typedef struct __attribute__ ((packed))
{
    struct cmdh_fsp_cmd_header;
    uint8_t              format;
    uint8_t              version;
    uint8_t              reserved[2];
    apss_cfg_adc_v10_t   adc[MAX_APSS_ADC_CHANNELS];
    apss_cfg_gpio_t      gpio[MAX_APSS_GPIO_PORTS];
}cmdh_apss_config_v10_t; //Used by Habanero

// Used by TMGT to send OCC the PCAP config data.
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

// Used by TMGT to send OCC the PCAP config data.
typedef struct __attribute__ ((packed))
{
    struct cmdh_fsp_cmd_header;
    uint8_t              format;
    uint8_t              version;
    cmdh_pcap_config_data_t   pcap_config;
}cmdh_pcap_config_t;

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

// Used by TMGT to send OCC the System config data.
typedef struct __attribute__ ((packed))
{
    uint8_t  system_type;     // OCC usage of this byte is TBD
    uint64_t proc_huid;       // Processor HUID
    uint64_t backplane_huid;  // Backplane HUID
    uint64_t apss_huid;       // APSS HUID
    uint64_t dpss_huid;       // DPSS HUID
} cmdh_sys_config_data_t;

// Used by TMGT to send OCC the system config data.
typedef struct __attribute__ ((packed))
{
    struct cmdh_fsp_cmd_header;
    uint8_t              format;
    uint8_t              version;
    cmdh_sys_config_data_t   sys_config;
}cmdh_sys_config_t;

typedef struct __attribute__ ((packed))
{
    uint8_t  system_type;     // General system type
    uint16_t proc_sid;        // Processor Sensor ID
    uint16_t core_sid[24];    // 12 cores. 2 bytes for Temp, followed by 2 bytes for Frequency.
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

// Used by TMGT to send OCC the IPS config data.
typedef struct __attribute__ ((packed))
{
    uint8_t     iv_ipsEnabled;          // Idle Power Save Enabled (0 or 1)
    uint16_t    iv_delayTimeforEntry;   // Delay Time in seconds to enter Idle Power
    uint8_t     iv_utilizationForEntry; // Utilization threshold percentage to enter Idle Power
    uint16_t    iv_delayTimeforExit;    // Delay Time in seconds to exit Idle Power
    uint8_t     iv_utilizationForExit;  // Utilization threshold percentage to exit Idle Power

} cmdh_ips_config_data_t;

// Used by TMGT to send OCC the IPS config data.
typedef struct __attribute__ ((packed))
{
    struct                  cmdh_fsp_cmd_header;
    uint8_t                 iv_format;
    uint8_t                 iv_version;
    cmdh_ips_config_data_t  iv_ips_config;
}cmdh_ips_config_t;

extern cmdh_ips_config_data_t   G_ips_config_data;

// Used by TMGT to send OCC thermal control thresholds
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

// Used by TMGT to send OCC thermal control thresholds
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


// Header data for mem cfg packet
typedef struct __attribute__ ((packed))
{
    struct cmdh_fsp_cmd_header;
    uint8_t                    format;
    uint8_t                    version;
    uint8_t                    num_data_sets;
}cmdh_mem_cfg_header_t;

// Maps an HUID to a centaur or dimm
// NOTE: 0xFF for dimm means this is for a centaur
typedef struct __attribute__ ((packed))
{
    uint64_t                   huid;
    uint8_t                    centaur_num;
    uint8_t                    dimm_num;
    uint16_t                   reserved;
}cmdh_mem_cfg_data_set_t;

// Config packet definition used by TMGT to
// send HUID mappings for centaurs and dimms
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


// Header data for mem throttle packet
typedef struct __attribute__ ((packed))
{
    struct cmdh_fsp_cmd_header;
    uint8_t                    format;
    uint8_t                    version;
    uint8_t                    num_data_sets;
}cmdh_mem_throt_header_t;

// Provides memory throttle min and max values
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

// Config packet definition used by TMGT to
// send mem throttle min/max settings.
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


// Used to mark present the config data TMGT has sent us.
typedef struct data_cnfg
{
  uint32_t                      data_mask;
  cmdh_thrm_thresholds_t        thrm_thresh;
} data_cnfg_t;

// Used by TMGT to send OCC the Vdd and Vcs uplift values
typedef struct __attribute__ ((packed))
{
    struct               cmdh_fsp_cmd_header;
    uint8_t              format;
    uint8_t              version;
    uint8_t              vdd_vid_uplift; //Only positive uplift values are supported
    uint8_t              vcs_vid_uplift; //Only positive uplift values are supported
}cmdh_uplift_config_t;

errlHndl_t DATA_store_cnfgdata (const cmdh_fsp_cmd_t * i_cmd_ptr,
                                    cmdh_fsp_rsp_t * i_rsp_ptr);

uint8_t DATA_request_cnfgdata ();

uint32_t DATA_get_present_cnfgdata ();

errlHndl_t DATA_get_thrm_thresholds(cmdh_thrm_thresholds_t **o_thrm_thresh);

errlHndl_t DATA_get_ips_cnfg(cmdh_ips_config_data_t **o_ips_cnfg);

#endif

