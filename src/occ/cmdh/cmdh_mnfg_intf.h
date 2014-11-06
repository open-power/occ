/**
 * @file cmdh_mnfg_intf.h
 * @brief Header file for Manufacturing interface.
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
 *  @gs004              gjsilva   05/15/2013  Created
 *  @gm002   885429     milesg    05/30/2013  support for list/get sensors
 *  @gs006   884384     gjsilva   05/30/2013  Support for mnfg auto-slewing function
 *  @gm004   892961     milesg    07/25/2013  Support memory auto slewing
 *
 *  @endverbatim
 */
#ifndef CMDH_MNFG_INTF_H
#define CMDH_MNFG_INTF_H

//*************************************************************************
// Includes
//*************************************************************************
#include "cmdh_fsp.h"
#include "sensor.h"

//*************************************************************************
// Defines/Enums
//*************************************************************************

typedef enum {
    MNFG_RUN_STOP_SLEW      = 0x02,
    MNFG_LIST_SENSORS       = 0x05,
    MNFG_GET_SENSOR         = 0x06,
    MNFG_OVERSUB_EMULATION  = 0x07,
    MNFG_RETRIEVE_EAR       = 0x08,
    MNFG_MEMORY_SLEW        = 0x09,
    MNFG_SET_FMINMAX        = 0x0A,
    MNFG_FAILSAFE_EMULATION = 0x0C,
    MNFG_CPM_STRESS_CALI    = 0x0D,
    MNFG_UV_CONTROL         = 0x0E,
    MNFG_FCHECK_CONTROL     = 0x0F,
} MNFG_CMD;

#define MNFG_INTF_SLEW_START    0x00
#define MNFG_INTF_SLEW_STOP     0x01
#define MNFG_INTF_SINGLE_STEP   0x00
#define MNFG_INTF_FULL_SLEW     0x01

#define MNFG_INTF_RUN_STOP_SLEW_RSP_SIZE 6

/**
 * @struct mnfg_run_stop_slew_cmd_t 
 * @brief Used by OCC to get mnfg run/stop slew
 *        command
 */
typedef struct __attribute__ ((packed))
{
    struct    cmdh_fsp_cmd_header;
    uint8_t   sub_cmd;
    uint8_t   version;
    uint8_t   action;
    uint8_t   bottom_mode;
    int8_t    bottom_percent;
    uint8_t   high_mode;
    int8_t    high_percent;
    uint8_t   step_mode;
    uint8_t   step_delay;
}mnfg_run_stop_slew_cmd_t;

/**
 *  @struct mnfg_run_stop_slew_rsp_t
 *  @brief Used by OCC firmware to respond to mnfg run/stop
 *         slew command
 */
typedef struct __attribute__ ((packed))
{
    struct    cmdh_fsp_rsp_header;
    uint16_t  slew_count;
    uint16_t  fstart;
    uint16_t  fstop;
    uint16_t  checksum;
}mnfg_run_stop_slew_rsp_t;

/**
 * @struct mnfg_mem_slew_cmd_t 
 * @brief Used by OCC to get mnfg memory slew
 *        command
 */
typedef struct __attribute__ ((packed))
{
    struct    cmdh_fsp_cmd_header;
    uint8_t   sub_cmd;
    uint8_t   version;
    uint8_t   action;
}mnfg_mem_slew_cmd_t;

#define MNFG_INTF_MEM_SLEW_RSP_SIZE 2

/**
 *  @struct mnfg_mem_slew_rsp_t
 *  @brief Used by OCC firmware to respond to mnfg memory 
 *         slew command
 */
typedef struct __attribute__ ((packed))
{
    struct    cmdh_fsp_rsp_header;
    uint16_t  slew_count;
    uint16_t  checksum;
}mnfg_mem_slew_rsp_t;

/**
 * @struct mnfg_emul_oversub_cmd_t 
 * @brief Used by OCC to get mnfg emulate oversubscription 
 *        command
 */
typedef struct __attribute__ ((packed))
{
    struct    cmdh_fsp_cmd_header;
    uint8_t   sub_cmd;
    uint8_t   version;
    uint8_t   action;
    uint8_t   reserved;
}mnfg_emul_oversub_cmd_t;

/**
 *  @struct mnfg_emul_oversub_rsp_t
 *  @brief Used by OCC firmware to respond to mnfg emulate
 *         oversubscription command
 */
typedef struct __attribute__ ((packed))
{
    struct    cmdh_fsp_rsp_header;
    uint8_t   state;
    uint16_t  checksum;
}mnfg_emul_oversub_rsp_t;

#define MFG_LIST_SENSOR_VERSION 0
#define MFG_MAX_NUM_SENSORS 50 //20 bytes per sensor, 4k response packet (4k stack is limiting factor here).
/**
 * @struct cmdh_mfg_list_sensor_query_t 
 * @brief Used by mfg to get sensor data 
 */
typedef struct __attribute__ ((packed))
{
    struct      cmdh_fsp_cmd_header;
    uint8_t     sub_cmd;
    uint8_t     version;
    uint16_t    start_gsid;
    uint8_t     present;
    uint16_t    location;
    uint16_t    type;
}cmdh_mfg_list_sensors_query_t;

typedef struct __attribute__ ((packed))
{
    uint16_t    gsid;
    char        name[MAX_SENSOR_NAME_SZ]; //16 bytes
    uint16_t    sample;
}cmdh_mfg_sensor_rec_t;

/**
 *  @struct cmdh_mfg_list_sensors_resp_t
 *  @brief Used by OCC firmware to respond to the
 *         "MFG_LIST_SENSORS" mfg command.  Follows
 *         the TMGT/OCC specification.
 */
typedef struct __attribute__ ((packed))
{
    struct                  cmdh_fsp_rsp_header;
    uint8_t                 truncated;
    uint8_t                 num_sensors;
    cmdh_mfg_sensor_rec_t   sensor[MFG_MAX_NUM_SENSORS];
    uint16_t                checksum;
}cmdh_mfg_list_sensors_resp_t;

#define MFG_GET_SENSOR_VERSION 0
/**
 * @struct cmdh_mfg_get_sensor_query_t 
 * @brief Used by mfg to get sensor data 
 */
typedef struct __attribute__ ((packed))
{
    struct      cmdh_fsp_cmd_header;
    uint8_t     sub_cmd;
    uint8_t     version;
    uint16_t    gsid;
}cmdh_mfg_get_sensor_query_t;

/**
 *  @struct cmdh_mfg_get_sensor_resp_t
 *  @brief Used by OCC firmware to respond to the
 *         "MFG_GET_SENSOR" mfg command.  Follows
 *         the TMGT/OCC specification.
 */
typedef struct __attribute__ ((packed))
{
    struct                  cmdh_fsp_rsp_header;
    uint16_t                gsid;
    uint16_t                sample;
    uint8_t                 status;
    uint32_t                accumulator;
    uint16_t                min;
    uint16_t                max;
    char                    name[MAX_SENSOR_NAME_SZ];
    char                    units[MAX_SENSOR_UNIT_SZ];
    uint32_t                freq;
    uint32_t                scalefactor;
    uint16_t                location;
    uint16_t                type;
    uint16_t                checksum;
}cmdh_mfg_get_sensor_resp_t;

/*******************************************************************/
/* Function Definitions                                            */
/*******************************************************************/

void cmdh_mnfg_test_parse (const cmdh_fsp_cmd_t * i_cmd_ptr,
                                 cmdh_fsp_rsp_t * o_rsp_ptr);

uint8_t cmdh_mnfg_emulate_oversub(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                        cmdh_fsp_rsp_t * o_rsp_ptr);

uint8_t cmdh_mnfg_list_sensors(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                     cmdh_fsp_rsp_t * o_rsp_ptr);

uint8_t cmdh_mnfg_get_sensor(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                     cmdh_fsp_rsp_t * o_rsp_ptr);

uint8_t cmdh_mnfg_run_stop_slew(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                      cmdh_fsp_rsp_t * o_rsp_ptr);

#endif
