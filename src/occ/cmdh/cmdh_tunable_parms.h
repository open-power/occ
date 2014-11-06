/**
 * @file cmdh_tunable_parms.h
 * @brief Header file for Tunable Parms interface.
 *
 *
*/
/**
 *      @page ChangeLogs Change Logs
 *      @section _cmdh_tunable_parms_h cmdh_tunable_parms.h
 *      @verbatim
 *
 *  Flag     Def/Fea    Userid    Date        Description
 *  -------- ---------- --------  ---------   ----------------------------------
 *  @rt004   905638     tapiar    11/13/2013  Created
 *
 *  @endverbatim
 */
#ifndef CMDH_TUNABLE_PARMS_H
#define CMDH_TUNABLE_PARMS_H

//*************************************************************************
// Includes
//*************************************************************************
#include "cmdh_fsp_cmds.h"

//*************************************************************************
// Defines/Enums
//*************************************************************************

typedef enum {
    TUNABLE_PARMS_QUERY     = 0x00,
    TUNABLE_PARMS_WRITE     = 0x01,
    TUNABLE_PARMS_RESTORE   = 0x02,
} TUNABLE_PARMS_CMD;


/**
 * @struct tunable_parms_query_cmd_t 
 * @brief Used by OCC to get tunable parms query 
 *        command
 */
typedef struct __attribute__ ((packed))
{
    struct    cmdh_fsp_cmd_header;
    uint8_t   sub_cmd;
    uint8_t   version;
}tunable_parms_query_cmd_t;

#define TUNABLE_PARMS_MAX_PARMS 29

#define TUNABLE_PARMS_QUERY_VERSION 0
#define TUNABLE_PARMS_WRITE_VERSION 0

typedef struct __attribute__ ((packed))
{
    uint8_t id;
    uint8_t value[2];
}tunable_parm_write_entry_t;

/**
 * @struct tunable_parms_write_cmd_t 
 * @brief Used by OCC to get tunable parms write 
 *        command
 */
typedef struct __attribute__ ((packed))
{
    struct                      cmdh_fsp_cmd_header;
    uint8_t                     sub_cmd;
    uint8_t                     version;
    uint8_t                     numParms;
    tunable_parm_write_entry_t  data[TUNABLE_PARMS_MAX_PARMS];
}tunable_parms_write_cmd_t;


/**
 * @struct tunable_parms_query_rsp_t 
 * @brief Used by OCC to get tunable parms query 
 *        response
 */
typedef struct __attribute__ ((packed))
{
    struct                      cmdh_fsp_rsp_header;
    uint8_t                     version;
    uint8_t                     numParms;
    cmdh_tunable_param_table_t  data[TUNABLE_PARMS_MAX_PARMS];
}tunable_parms_query_rsp_t;



/*******************************************************************/
/* Function Definitions                                            */
/*******************************************************************/

errlHndl_t cmdh_tunable_parms(  const cmdh_fsp_cmd_t * i_cmd_ptr,
                                cmdh_fsp_rsp_t * o_rsp_ptr);

uint8_t cmdh_tunable_parms_query(   const cmdh_fsp_cmd_t * i_cmd_ptr,
                                    cmdh_fsp_rsp_t * o_rsp_ptr);

uint8_t cmdh_tunable_parms_write(   const cmdh_fsp_cmd_t * i_cmd_ptr,
                                    cmdh_fsp_rsp_t * o_rsp_ptr);

uint8_t cmdh_tunable_parms_restore( const cmdh_fsp_cmd_t * i_cmd_ptr,
                                    cmdh_fsp_rsp_t * o_rsp_ptr);


#endif
