/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/cmdh/cmdh_dbug_cmd.c $                            */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015                             */
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

//*************************************************************************/
// Includes
//*************************************************************************/
#include <common_types.h>       // imageHdr_t declaration and image header macro
#include <occ_service_codes.h>  // For reason code
#include <errl.h>               // For error handle
#include <trac.h>               // For traces
#include <state.h>
#include <cmdh_dbug_cmd.h>
#include <cmdh_fsp.h>
#include <cmdh_fsp_cmds.h>
#include <centaur_data.h>
//#include <gpe_data.h>
#include <proc_data.h>
#include <apss.h>

//*************************************************************************/
// Externs
//*************************************************************************/

//*************************************************************************/
// Macros
//*************************************************************************/

//*************************************************************************/
// Defines/Enums
//*************************************************************************/

//*************************************************************************/
// Structures
//*************************************************************************/

//*************************************************************************/
// Globals
//*************************************************************************/

//*************************************************************************/
// Function Prototypes
//*************************************************************************/

//*************************************************************************/
// Functions
//*************************************************************************/

// Function Specification
//
// Name:  dbug_err_inject
//
// Description: Injects an error
//
// End Function Specification
void dbug_err_inject(const cmdh_fsp_cmd_t * i_cmd_ptr,
                           cmdh_fsp_rsp_t * i_rsp_ptr)
{
    errlHndl_t l_err;
    cmdh_dbug_inject_errl_query_t *l_cmd_ptr = (cmdh_dbug_inject_errl_query_t*) i_cmd_ptr;

    i_rsp_ptr->data_length[0] = 0;
    i_rsp_ptr->data_length[1] = 0;
    G_rsp_status = ERRL_RC_SUCCESS;

    if(!strncmp(l_cmd_ptr->comp, "RST", OCC_TRACE_NAME_SIZE))
    {
        l_err = createErrl(CMDH_DBUG_MID,     //modId
                           INTERNAL_FAILURE,             //reasoncode
                           OCC_NO_EXTENDED_RC,           //Extended reason code
                           ERRL_SEV_PREDICTIVE,          //Severity
                           NULL,                         //Trace Buf
                           DEFAULT_TRACE_SIZE,           //Trace Size
                           0xff,                         //userdata1
                           0);                           //userdata2

        if (INVALID_ERR_HNDL == l_err)
        {
            G_rsp_status = ERRL_RC_INTERNAL_FAIL;
        }

        addCalloutToErrl(l_err,
                         ERRL_CALLOUT_TYPE_HUID,         //callout type (HUID/CompID)
                         G_sysConfigData.proc_huid,      //callout data
                         ERRL_CALLOUT_PRIORITY_HIGH);    //priority

        REQUEST_RESET(l_err);
    }
    else
    {
        l_err = createErrl(CMDH_DBUG_MID,     //modId
                           INTERNAL_FAILURE,             //reasoncode
                           OCC_NO_EXTENDED_RC,           //Extended reason code
                           ERRL_SEV_UNRECOVERABLE,       //Severity
                           TRAC_get_td(l_cmd_ptr->comp), //Trace Buf
                           DEFAULT_TRACE_SIZE,           //Trace Size
                           0xff,                         //userdata1
                           0);                           //userdata2

        if (INVALID_ERR_HNDL == l_err)
        {
            G_rsp_status = ERRL_RC_INTERNAL_FAIL;
        }

        // Commit Error log
        commitErrl(&l_err);
    }

    if (G_rsp_status == ERRL_RC_INTERNAL_FAIL)
    {
        TRAC_ERR("cmdh_dbug_inject_errl: Fail creating ERR Log\n");
    }
    else
    {
        TRAC_INFO("cmdh_dbug_inject_errl: inject errl for COMP : %s\n", l_cmd_ptr->comp);
    }

    return;
}

// Function Specification
//
// Name:  dbug_centaur_dump
//
// Description: Injects an error
//
// End Function Specification
void dbug_centaur_dump(const cmdh_fsp_cmd_t * i_cmd_ptr,
                             cmdh_fsp_rsp_t * i_rsp_ptr)
{
/* TEMP -- NOT SUPPORTED (Don't have MemData structure anymore */
#if 0
    uint16_t l_datalen = 0;
    uint8_t l_jj=0;

    // Determine the size of the data we are returning
    l_datalen = (sizeof(MemData) * MAX_NUM_CENTAURS);

    // Fill out the response with the data we are returning
    for(l_jj=0; l_jj < MAX_NUM_CENTAURS; l_jj++)
    {
        MemData * l_sensor_cache_ptr =
            cent_get_centaur_data_ptr(l_jj);

        memcpy((void *)&(i_rsp_ptr->data[l_jj*sizeof(MemData)]),
               (void *)l_sensor_cache_ptr,
               sizeof(MemData));
    }

    // Fill out the rest of the response data
    i_rsp_ptr->data_length[0] = CONVERT_UINT16_UINT8_HIGH(l_datalen);
    i_rsp_ptr->data_length[1] = CONVERT_UINT16_UINT8_LOW(l_datalen);
    G_rsp_status              = ERRL_RC_SUCCESS;
#endif
    return;
}

// Function Specification
//
// Name:  dbug_apss_dump
//
// Description: Dumps the APSS power measurement raw ADC / GPIO data
//
// End Function Specification
void dbug_apss_dump(const cmdh_fsp_cmd_t * i_cmd_ptr,
                          cmdh_fsp_rsp_t * i_rsp_ptr)
{
    uint16_t l_datalen = 0;

    // Determine the size of the data we are returning
    l_datalen = (sizeof(apssPwrMeasStruct_t));

    memcpy((void *) &(i_rsp_ptr->data[0]),
           (void *) &G_apss_pwr_meas,
           sizeof(apssPwrMeasStruct_t));

    // Fill out the rest of the response data
    i_rsp_ptr->data_length[0] = CONVERT_UINT16_UINT8_HIGH(l_datalen);
    i_rsp_ptr->data_length[1] = CONVERT_UINT16_UINT8_LOW(l_datalen);
    G_rsp_status              = ERRL_RC_SUCCESS;

    return;
}

// Function Specification
//
// Name:  dbug_proc_data_dump
//
// Description: Dumps the processor core data
//
// End Function Specification
void dbug_proc_data_dump(const cmdh_fsp_cmd_t * i_cmd_ptr,
                               cmdh_fsp_rsp_t * i_rsp_ptr)
{
/* TEMP -- NOT SUPPORTED (Don't have CoreData structure anymore) */
#if 0
    uint16_t l_datalen = 0;
    uint8_t l_jj=0;

    // Determine the size of the data we are returning
    l_datalen = (sizeof(CoreData) * MAX_NUM_CORES);

    // Fill out the response with the data we are returning
    for(l_jj=0; l_jj < MAX_NUM_CORES; l_jj++)
    {
        CoreData * l_core_data_ptr =
            proc_get_bulk_core_data_ptr(l_jj);

        memcpy((void *) &(i_rsp_ptr->data[l_jj*sizeof(CoreData)]),
               (void *) l_core_data_ptr,
               sizeof(CoreData));
    }

    // Fill out the rest of the response data
    i_rsp_ptr->data_length[0] = CONVERT_UINT16_UINT8_HIGH(l_datalen);
    i_rsp_ptr->data_length[1] = CONVERT_UINT16_UINT8_LOW(l_datalen);
    G_rsp_status              = ERRL_RC_SUCCESS;
#endif
    return;
}

// Function Specification
//
// Name:  cmdhDbugCmd
//
// Description:   Entry-point for CMDH Debug Commands
//
// End Function Specification
errlHndl_t cmdhDbugCmd(void * i_arg)
{
    errlHndl_t               l_errl    = NULL;
    cmdhDbugCmdArg_t * l_arg     = (cmdhDbugCmdArg_t *) i_arg;
    cmdh_fsp_cmd_t *         l_cmd_ptr = l_arg->i_cmd_ptr;
    cmdh_fsp_rsp_t *         l_rsp_ptr = l_arg->io_rsp_ptr;
    uint8_t                  l_sub_cmd = 0;

    // Sub Command for debug is always first byte of data
    l_sub_cmd = l_cmd_ptr->data[0];

    // Trace that a debug command was run
    TRAC_INFO("Debug Command: Sub:0x%02x\n", l_sub_cmd);

    // Build up a successful default response
    G_rsp_status  = ERRL_RC_SUCCESS;
    l_rsp_ptr->data_length[0] = 0;
    l_rsp_ptr->data_length[1] = 0;

    switch (l_sub_cmd)
    {
        case DBUG_INJECT_ERRL:
            dbug_err_inject(l_cmd_ptr, l_rsp_ptr);
            break;

        case DBUG_CENTAUR_SENSOR_CACHE:
             dbug_centaur_dump(l_cmd_ptr, l_rsp_ptr);
             break;

        case DBUG_DUMP_RAW_AD:
             dbug_apss_dump(l_cmd_ptr, l_rsp_ptr);
             break;

        case DBUG_DUMP_PROC_DATA:
             dbug_proc_data_dump(l_cmd_ptr, l_rsp_ptr);
             break;

        case DBUG_READ_SCOM:            // Obsolete
        case DBUG_PUT_SCOM:             // Obsolete
        case DBUG_POKE:                 // Can't allow in trusted
        case DBUG_GET_TRACE:
        case DBUG_CLEAR_TRACE:
        case DBUG_SET_PEXE_EVENT:
        case DBUG_DUMP_THEMAL:
        case DBUG_DUMP_POWER:
        case DBUG_MEM_PWR_CTL:
        case DBUG_PERFCOUNT:
        case DBUG_TEST_INTF:
        case DBUG_SET_BUS_SPEED:        // Obsolete
        case DBUG_FAN_CONTROL:          // Obsolete
        case DBUG_IIC_READ:             // Obsolete
        case DBUG_IIC_WRITE:            // Obsolete
        case DBUG_GPIO_READ:
        case DBUG_CALCULATE_MAX_DIFF:
        case DBUG_FORCE_ELOG:
        case DBUG_SWITCH_PHASE:
        case DBUG_INJECT_ERR:
        case DBUG_VERIFY_V_F:
        case DBUG_DUMP_PPM_DATA:
        default:
            G_rsp_status = ERRL_RC_INVALID_DATA;
            break;
    }

    return l_errl;
}
