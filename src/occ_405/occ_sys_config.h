/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/occ_sys_config.h $                                */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2016                        */
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

#ifndef _occ_sys_config_h
#define _occ_sys_config_h

// SSX has Pstate defined in pgp_common.h file. Without below ifdef
// compiler complains about redefinition for OCC.
#ifdef OCC_FIRMWARE
#include "occhw_common.h"
#else
#endif
#include <state.h>
#include <apss.h>
#include <dimm.h>

#define MAX_NUM_OCC              4
#define MAX_NUM_NODES            4
#define MAX_NUM_CORES           24
#define MAX_THREADS_PER_CORE     4
#define MAX_NUM_CHIP_MODULES     4
#define MAX_NUM_POWER_SUPPLIES   4
#define MAX_NUM_MEM_CONTROLLERS  8
#define MAX_NUM_CENTAURS         8
#define NUM_PROC_VRMS            2
#define MAX_NUM_MCU_PORTS        4

#define NUM_PROC_CHIPS_PER_OCC          1
#define NUM_CENTAURS_PER_MEM_CONTROLLER 1
#define NUM_PORT_PAIRS_PER_CENTAUR      2
#define NUM_DIMMS_PER_CENTAUR           8
#define NUM_MBAS_PER_CENTAUR            2

#define UPPER_LIMIT_PROC_FREQ_MHZ     6000

//Number of samples per second for performance-related algorithms (e.g. UTIL4MSP0Cy)
#define AMEC_DPS_SAMPLING_RATE         250
//Time interval for averaging utilization and frequency (IPS algorithm)
#define AMEC_IPS_AVRG_INTERVAL           3

// System Structures
typedef union
{
    struct
    {
        uint8_t     kvm:        1;
        uint8_t     reserved:   5;
        uint8_t     ite:        1;
        uint8_t     single:     1;
    };
    uint8_t byte;
} eSystemType;

// APSS Structures & Defines
#define SYSCFG_INVALID_ADC_CHAN      0xff
#define SYSCFG_INVALID_PIN           0xff

#define MAX_VRFAN_SIGNALS       4
#define MAX_APSS_MEM_TEMPS      8
#define MAX_ADC_IO_DOMAINS      3
#define MAX_ADC_FAN_DOMAINS     2
#define MAX_ADC_STORAGE_DOMAINS 2
#define MAX_CENT_EN_VCACHE      4
#define MAX_DOM_OC_LATCH        4
#define MAX_CONN_OC_SIGNALS     5
#define MAX_PROC_CENT_CH        4

// List of all possible APSS Channel assignments (Function IDs)
// Each channel in the APSS will be associated with only one of these
// function ids for each system type as defined in the mrw.
// If a channel is assigned a 0x00, that means it's not assigned.
typedef enum
{
    ADC_RESERVED                = 0x00,
    ADC_MEMORY_PROC_0           = 0x01,
    ADC_MEMORY_PROC_1           = 0x02,
    ADC_MEMORY_PROC_2           = 0x03,
    ADC_MEMORY_PROC_3           = 0x04,
    ADC_VDD_PROC_0              = 0x05,
    ADC_VDD_PROC_1              = 0x06,
    ADC_VDD_PROC_2              = 0x07,
    ADC_VDD_PROC_3              = 0x08,
    ADC_VCS_VIO_VPCIE_PROC_0    = 0x09,
    ADC_VCS_VIO_VPCIE_PROC_1    = 0x0A,
    ADC_VCS_VIO_VPCIE_PROC_2    = 0x0B,
    ADC_VCS_VIO_VPCIE_PROC_3    = 0x0C,
    ADC_IO_A                    = 0x0D,
    ADC_IO_B                    = 0x0E,
    ADC_IO_C                    = 0x0F,
    ADC_FANS_A                  = 0x10,
    ADC_FANS_B                  = 0x11,
    ADC_STORAGE_A               = 0x12,
    ADC_STORAGE_B               = 0x13,
    ADC_12V_SENSE               = 0x14,
    ADC_GND_REMOTE_SENSE        = 0x15,
    ADC_TOTAL_SYS_CURRENT       = 0x16,
    ADC_MEM_CACHE               = 0x17,
    ADC_GPU_SENSE               = 0x18,
    ADC_MEMORY_PROC_0_0         = 0x19, //NOTE: On Habanero, the processor has 4 centaurs with 1 APSS channel
    ADC_MEMORY_PROC_0_1         = 0x1A, //      assigned to each one of them. ADC_MEMORY_PROC_0 will be used
    ADC_MEMORY_PROC_0_2         = 0x1B, //      for the first one and these are for the other 3.
    NUM_ADC_ASSIGNMENT_TYPES    // This should always be the last member
} eApssAdcChannelAssignments;

typedef enum
{
    GPIO_RESERVED               = 0x0000,
    GPIO_FAN_WATCHDOG_ERROR     = 0x0001,
    GPIO_FAN_FULL_SPEED         = 0x0002,
    GPIO_FAN_ERROR              = 0x0003,
    GPIO_FAN_RESERVED           = 0x0004,
    GPIO_VR_HOT_MEM_PROC_0      = 0x0005,
    GPIO_VR_HOT_MEM_PROC_1      = 0x0006,
    GPIO_VR_HOT_MEM_PROC_2      = 0x0007,
    GPIO_VR_HOT_MEM_PROC_3      = 0x0008,
    GPIO_CENT_EN_VCACHE0        = 0x0009,
    GPIO_CENT_EN_VCACHE1        = 0x000A,
    GPIO_CENT_EN_VCACHE2        = 0x000B,
    GPIO_CENT_EN_VCACHE3        = 0x000C,
    CME_THROTTLE_N              = 0x000D,
    GND_OC_N                    = 0x000E,
    DOM_A_OC_LATCH              = 0x000F,
    DOM_B_OC_LATCH              = 0x0010,
    DOM_C_OC_LATCH              = 0x0011,
    DOM_D_OC_LATCH              = 0x0012,
    NUM_GPIO_ASSIGNMENT_TYPES    // This should always be the last member
} eApssGpioAssignments;

typedef struct
{
  // Gain and Offset, Stored in a exponent/mantissa format (like P7 PS Gain/Offset)
  uint32_t gain;        // Think "multiplier"
  uint32_t offset;      // Will probably always be 0, since APSS is so accurate
  uint8_t  gnd_select;
} apssCalibrationData_t;

typedef struct
{
  // Value stored will be APSS GPIO Number, if not present, set to INVALID = xFF.
  // Example:  vr_fan[0] = 4; (VR_FAN is APSS GPIO #4)
  uint8_t dom_oc_latch[MAX_DOM_OC_LATCH];
  uint8_t gnd_oc_n;
  uint8_t cme_throttle_n;
  uint8_t cent_en_vcache[MAX_CENT_EN_VCACHE];
  uint8_t vr_fan[MAX_VRFAN_SIGNALS];
  uint8_t fans_watchdog_error;
  uint8_t fans_full_speed;
  uint8_t fans_error;
  uint8_t fans_reserved;
} apssGpioPinData_t;

typedef struct
{
  // Value stored will be APSS ADC Channel Number, if rail is not present, set
  // to INVALID = xFF
  uint8_t memory[MAX_NUM_CHIP_MODULES][MAX_PROC_CENT_CH];
  uint8_t vdd[MAX_NUM_CHIP_MODULES];
  uint8_t io[MAX_ADC_IO_DOMAINS];
  uint8_t fans[MAX_ADC_FAN_DOMAINS];
  uint8_t storage_media[MAX_ADC_STORAGE_DOMAINS];
  uint8_t total_current_12v;
  uint8_t vcs_vio_vpcie[MAX_NUM_CHIP_MODULES];
  uint8_t sense_12v;
  uint8_t remote_gnd;
  uint8_t mem_cache;
  uint8_t gpu;
} apssAdcChannelData_t;

// Master/Slave Configuration
typedef struct
{
  // Bitmask that states if OCC is master capable
  // (i.e. 0x11 = Chip_Id 000 & Chip_Id 100 can both be masters)
  uint8_t is_master_capable;

  // Which Chip_Id should be initial master
  // (i.e. default_master = 0x00 for almost all cases)
  uint8_t default_master;

} masterCapability_t;


// System Frequency
typedef struct
{
    uint16_t table[OCC_MODE_COUNT];  // Table w/ freq for each mode
    uint8_t  update_count;           //
    uint8_t  _reserved;              // Align to 2 b/c we may use it in PBAX broadcast
} freqConfig_t;   // @th040

// Power Cap Structures
typedef struct
{
    uint16_t current_pcap;     // Node power cap requested by customer (AEM) in 1W units
    uint16_t soft_min_pcap;    // Minimum node power cap allowed in 1W units
    uint16_t hard_min_pcap;    // Minimum guaranteed node power cap in 1W units
    uint16_t max_pcap;         // Maximum customer settable node power cap in 1W units
    uint16_t oversub_pcap;     // Node power cap to be used for oversubscripion in 1W units
    uint16_t system_pcap;      // Fixed node power cap required by the system in 1W units
    uint8_t  unthrottle;       // Only used on ITEs -- is indicated from CMM
    uint8_t  pcap_data_count;  // Used by OCC only.  Initialized to 0 and incremented by 1 with every new packet.
} pcap_config_data_t;

// Memory Throttle settings
typedef struct
{
    uint16_t    min_n_per_mba;      //minimum value
    uint16_t    min_mem_power;      // Max mem Power @min (x0.1W)

    uint16_t    pcap1_n_per_mba;    //max mba value for Power Cap Level 1
    uint16_t    pcap1_mem_power;    //max  memory power @PCAP L1

    uint16_t    pcap2_n_per_mba;    //max mba value for Power Cap Level 2
    uint16_t    pcap2_mem_power;    //max  memory power @PCAP L2

    uint16_t    nom_n_per_mba;      //max mba value for nominal mode
    uint16_t    nom_n_per_chip;     //chip setting for nominal mode
    uint16_t    nom_mem_power;      //max memory power @Redundant

    uint16_t    ovs_n_per_mba;      //max mba value for oversubscription
    uint16_t    ovs_n_per_chip;     //chip setting for oversubscription
    uint16_t    ovs_mem_power;      //max memory power @oversubscription
} mem_throt_config_data_t;

// Sys Config Structure

// Needs to be 128 bytes aligned so we can do a DMA to transfer this to SRAM from
// Main Memory.

// One global structure which is the same in all OCCs

// TODO: If we cannot dynamically change this structure and have it
// picked up by host boot & occ, then we will want to add reserved
// fields for each 'section' of the structure for future-proofing

typedef struct
{
  // ------------------------------------
  // Identification & Debug Flags
  // ------------------------------------

  // Gives us a version number of this structure for ease of debug
  uint8_t version;

  // Gives us a lot of debug flags that we can use to change OCC behavior
  // at startup.
  uint8_t debug_reserved[7];

  // ------------------------------------
  // System Configuration
  // ------------------------------------

  // Instead of system-type, lets try to send all system attributes
  // that matter instead of having tables in OCC code.
  eSystemType system_type;  // OCC usage of this byte is TBD

  // Processor HUID - HUID for this OCC processor, used by OCC for processor error call out
  uint32_t proc_huid;

  // Backplane HUID - Used by OCC for system backplane error call out (i.e. VRM errors will call out backplane)
  uint32_t backplane_huid;

  // APSS HUID - Used by OCC for APSS error call out
  uint32_t apss_huid;

  // DPSS HUID - Used by OCC for DPSS error call out
  uint32_t dpss_huid;

  // Contains how many OCCs & how many proc modules are present.
  uint8_t sys_num_proc_present;

  // ------------------------------------
  // Max Frequency for each mode
  // ------------------------------------
  freqConfig_t sys_mode_freq;

  // ------------------------------------
  // APSS Configuration Data
  // ------------------------------------

  // Holds the gain/offset needed to translate from raw ADC to actual power
  // measurements
  apssCalibrationData_t apss_cal[MAX_APSS_ADC_CHANNELS];

  // Holds the mapping of GPIO signals to APSS pins
  apssGpioPinData_t apss_gpio_map;

  // Holds the mapping of ADC signals to APSS adc channels
  apssAdcChannelData_t apss_adc_map;

  // GPIO Port Mode
  uint8_t apssGpioPortsMode[MAX_APSS_GPIO_PORTS];

  // ------------------------------------
  // Power Cap Configuration Data updated by Slaves
  // ------------------------------------
  pcap_config_data_t pcap;

  // ------------------------------------
  // Master/Slave OCC Configuration Data
  // ------------------------------------

  // Information Needed to translate from PowerBus ID to PBAX Id (OCC Id)
  //   - Index = Powerbus Id will get you PBAX Id as value at that index
  //   - Example pob2pbax_chip[1] = 4; (PowerBus ID 001 = PBAX ID 100
  uint8_t pob2pbax_chip[MAX_NUM_OCC];
  uint8_t pob2pbax_node[MAX_NUM_NODES];

  // Bitmask that states if OCC is present
  // (i.e. 0x11 = Chip_Id 000 & Chip_Id 100 are only OCCs present)
  uint8_t is_occ_present;

  // Tells us which OCCs can be master & which OCC should start out as master
  masterCapability_t master_config;

  // ------------------------------------
  // Oversubscription Configuration Data
  // ------------------------------------

  // ppb_fmax as set by Master OCC.  Initialized to 0xFFFF
  uint32_t master_ppb_fmax;

  // --------------------------------------
  // Memory Configuration Data
  // --------------------------------------
  uint32_t centaur_huids[MAX_NUM_CENTAURS];
  uint32_t dimm_huids[MAX_NUM_CENTAURS][NUM_DIMMS_PER_CENTAUR];
  uint8_t mem_type;
  uint8_t dimm_i2c_engine;

  // --------------------------------------
  // Memory Throttle limits
  // --------------------------------------
  mem_throt_config_data_t mem_throt_limits[MAX_NUM_CENTAURS][NUM_MBAS_PER_CENTAUR];

  // --------------------------------------
  // Vdd/Vcs Uplift vid codes
  // --------------------------------------
  // Current Vdd VID Uplift
  int8_t vdd_vid_uplift_cur;
  // Vdd VID delta to be applied to Pstate table
  int8_t vdd_vid_delta;
  // Current VCS VID Uplift request
  int8_t vcs_vid_uplift_cur;
  // Vcs VID delta to be applied to Pstate table
  int8_t vcs_vid_delta;

} occSysConfigData_t;  __attribute__ ((__aligned__ (128)))

extern occSysConfigData_t G_sysConfigData;


// Individual OCC Config Structure
// Needs to be 128 bytes aligned so we can do a DMA to transfer this to SRAM from
// Main Memory.

// One global structure per OCC (so there will be 8 up in main memory)

// TODO: If we cannot dynamically change this structure and have it
// picked up by host boot & occ, then we will want to add reserved
// fields for each 'section' of the structure for future-proofing

typedef struct
{
  int dummy;

} occModuleConfigData_t;  __attribute__ ((__aligned__ (128)))


// Default PstateSuperStructure that can be used instead of TMGT provided one.
extern const unsigned char G_defaultOccPstateSuperStructure[];

// MASTER PCAP values.
extern pcap_config_data_t G_master_pcap_data;


extern uint16_t    G_conn_oc_pins_bitmap;

#ifdef FSPLESS_SIMICS
void sysConfigFspLess(void);
#endif

#endif // _occ_sys_config_h
