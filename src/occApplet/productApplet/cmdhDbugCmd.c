/******************************************************************************
// @file cmdhDebugCmd.c
// @brief CMDH Debug Command
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section cmdhDbugCmd.c CMDHDEBUGCMD.c
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *   @th00d             thallet   04/25/2012  created
 *   @nh004   864941    neilhsu   12/20/2012  Support get/delete errl & added trace info
 *   @th031   878471    thallet   04/15/2013  Centaur Throttles
 *   @th032             thallet   04/26/2013  Tuleta HW Bringup
 *   @th036   881677    thallet   05/07/2013  Cleanup
 *   @gm006  SW224414   milesg    09/16/2013  Reset and FFDC improvements
 *   @rt001  897459     tapiar    10/02/2013  Update module ids with unique ids
 *   @fk002  905632     fmkassem  11/05/2013  Remove CriticalPathMonitor code
 *
 *  @endverbatim
 *
 *///*************************************************************************/


//*************************************************************************
// Includes
//*************************************************************************
#include <common_types.h>       // imageHdr_t declaration and image header macro
#include <occ_service_codes.h>  // For reason code
#include <aplt_service_codes.h> // For test applet module ID
#include <errl.h>               // For error handle
#include <trac.h>               // For traces
#include <state.h>
#include <appletId.h>
#include <cmdhDbugCmd.h>
#include <cmdh_fsp.h>
#include <cmdh_fsp_cmds.h>
#include <centaur_data.h>
#include <gpe_data.h>
#include <proc_data.h>
#include <apss.h>

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************
#define CMDH_DBUG_APPLET_ID  "Cmdh_Dbug_Aplt\0"

//*************************************************************************
// Structures
//*************************************************************************

//*************************************************************************
// Globals
//*************************************************************************

//*************************************************************************
// Function Prototypes
//*************************************************************************

//*************************************************************************
// Functions
//*************************************************************************

// Function Specification
//
// Name:  dbug_err_inject
//
// Description: Injects an error
//
// Flow:  --/--/--    FN=
//
// End Function Specification
void dbug_err_inject(const cmdh_fsp_cmd_t * i_cmd_ptr,
                           cmdh_fsp_rsp_t * i_rsp_ptr)  // @nh004c
{
    errlHndl_t l_err;
    cmdh_dbug_inject_errl_query_t *l_cmd_ptr = (cmdh_dbug_inject_errl_query_t*) i_cmd_ptr;

    i_rsp_ptr->data_length[0] = 0;
    i_rsp_ptr->data_length[1] = 0;
    i_rsp_ptr->rc = ERRL_RC_SUCCESS;

    if(!strncmp(l_cmd_ptr->comp, "RST", OCC_TRACE_NAME_SIZE)) //@gm006
    {
        l_err = createErrl(TEST_APLT_MODID_ERRLTEST,     //modId
                           INTERNAL_FAILURE,             //reasoncode
                           OCC_NO_EXTENDED_RC,           //Extended reason code
                           ERRL_SEV_PREDICTIVE,          //Severity
                           NULL,                         //Trace Buf
                           DEFAULT_TRACE_SIZE,           //Trace Size
                           0xff,                         //userdata1
                           0);                           //userdata2

        if (INVALID_ERR_HNDL == l_err)
        {
            i_rsp_ptr->rc = ERRL_RC_INTERNAL_FAIL;
        }

        addCalloutToErrl(l_err,
                         ERRL_CALLOUT_TYPE_HUID,         //callout type (HUID/CompID)
                         G_sysConfigData.proc_huid,      //callout data
                         ERRL_CALLOUT_PRIORITY_HIGH);    //priority

        REQUEST_RESET(l_err);
    }
    else
    {
        l_err = createErrl(TEST_APLT_MODID_ERRLTEST,     //modId
                           INTERNAL_FAILURE,             //reasoncode
                           OCC_NO_EXTENDED_RC,           //Extended reason code
                           ERRL_SEV_UNRECOVERABLE,       //Severity
                           TRAC_get_td(l_cmd_ptr->comp), //Trace Buf
                           DEFAULT_TRACE_SIZE,           //Trace Size
                           0xff,                         //userdata1
                           0);                           //userdata2

        if (INVALID_ERR_HNDL == l_err)
        {
            i_rsp_ptr->rc = ERRL_RC_INTERNAL_FAIL;
        }

        // Commit Error log
        commitErrl(&l_err);
    }

    if (i_rsp_ptr->rc == ERRL_RC_INTERNAL_FAIL)
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
// Flow:  --/--/--    FN=
//
// End Function Specification
void dbug_centaur_dump(const cmdh_fsp_cmd_t * i_cmd_ptr,
                             cmdh_fsp_rsp_t * i_rsp_ptr)  // @nh004c
{
    uint16_t l_datalen = 0;
    uint8_t l_jj=0;

    // Determine the size of the data we are returning
    l_datalen = (sizeof(MemData) * MAX_NUM_CENTAURS);

    // Fill out the response with the data we are returning
    for(l_jj=0; l_jj < MAX_NUM_CENTAURS; l_jj++)
    {
        MemData * l_sensor_cache_ptr =
            cent_get_centaur_data_ptr(l_jj);

        memcpy((void *) &(i_rsp_ptr->data[l_jj*sizeof(MemData)]),
                (void *) l_sensor_cache_ptr,
                sizeof(MemData));
    }

    // Fill out the rest of the response data
    i_rsp_ptr->data_length[0] = CONVERT_UINT16_UINT8_HIGH(l_datalen);
    i_rsp_ptr->data_length[1] = CONVERT_UINT16_UINT8_LOW(l_datalen);
    i_rsp_ptr->rc             = ERRL_RC_SUCCESS;


    return;
}


// Function Specification
//
// Name:  dbug_apss_dump
//
// Description: Dumps the APSS power measurement raw ADC / GPIO data
//
// Flow:  --/--/--    FN=
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
    i_rsp_ptr->rc             = ERRL_RC_SUCCESS;


    return;
}


// Function Specification
//
// Name:  dbug_proc_data_dump
//
// Description: Dumps the processor core data
//
// Flow:  --/--/--    FN=
//
// End Function Specification
void dbug_proc_data_dump(const cmdh_fsp_cmd_t * i_cmd_ptr,
                               cmdh_fsp_rsp_t * i_rsp_ptr)
{
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
    i_rsp_ptr->rc             = ERRL_RC_SUCCESS;


    return;
}

// Function Specification
//
// Name:  cmdhDbugCmd
//
// Description:   Entry-point for CMDH Debug Commands
//
// Flow: --/--/--    FN=
//
// End Function Specification
errlHndl_t cmdhDbugCmd(void * i_arg)
{
    errlHndl_t               l_errl    = NULL;
    cmdhDbugCmdAppletArg_t * l_arg     = (cmdhDbugCmdAppletArg_t *) i_arg;
    cmdh_fsp_cmd_t *         l_cmd_ptr = l_arg->i_cmd_ptr;
    cmdh_fsp_rsp_t *         l_rsp_ptr = l_arg->io_rsp_ptr;
    uint8_t                  l_sub_cmd = 0;

    // Sub Command for debug is always first byte of data
    l_sub_cmd = l_cmd_ptr->data[0];

    // Trace that a debug command was run
    TRAC_INFO("Debug Command via Applet: Sub:0x%02x\n", l_sub_cmd);

    // Build up a successful default response
    l_rsp_ptr->rc = ERRL_RC_SUCCESS;
    l_rsp_ptr->data_length[0] = 0;
    l_rsp_ptr->data_length[1] = 0;

    switch ( l_sub_cmd )
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
            l_rsp_ptr->rc = ERRL_RC_INVALID_DATA;
            break;
    } //end switch


    return l_errl;
}





/*****************************************************************************/
// Image Header
/*****************************************************************************/
IMAGE_HEADER (G_cmdhDbugCmd,cmdhDbugCmd,CMDH_DBUG_APPLET_ID,OCC_APLT_CMDH_DBUG);

