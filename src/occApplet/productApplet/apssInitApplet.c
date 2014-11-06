/******************************************************************************
// @file apssInitApplet.c
// @brief APSS initialization product applet
*/

/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section apssInitApplet.c APSSINITAPPLET.c
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *   @pb004             pbavari   09/13/2011  created
 *   @pb009             pbavari   10/31/2011  Moved apss_initialize retry
 *                                            inside applet
 *   @dw000             dwoodham  12/12/2011  Update call to IMAGE_HEADER
 *   @rc003             rickylie  02/03/2012  Verify & Clean Up OCC Headers & Comments
 *   @nh001             neilhsu   05/23/2012  Add missing error log tags
 *   @at009  859308     alvinwan  10/15/2012  Added tracepp support
 *   @ly003  861535     lychen    11/19/2012  Remove APSS configuration/gathering of Altitude & Temperature
 *   @th042   892056    thallet   07/19/2013  Send OCC to safe mode if first APSS GPE fails
 *   @at023  910877     alvinwan  01/09/2014  Excessive fan increase requests error for mfg 
 *   @fk005  911760     fmkassem  01/21/2014  APSS retry support.
 *
 *  @endverbatim
 *
 *///*************************************************************************/

//*************************************************************************
// Includes
//*************************************************************************
#include <common_types.h>       // imageHdr_t declaration and image header macro
#include <errl.h>               // For error handle
#include <trac.h>               // For traces
#include <occ_service_codes.h>  // for SSX_GENERIC_FAILURE  // @nh001a
#include <pss_service_codes.h>  // APSS module ids
#include <apss.h>               // APSS defines
#include <appletId.h>           // For applet ID num dw000a
#include "ssx_io.h"             // For printf
#include <state.h>

//*************************************************************************
// Externs
//*************************************************************************
extern PoreEntryPoint GPE_apss_initialize_gpio;
extern PoreEntryPoint GPE_apss_set_composite_mode;
extern PoreFlex G_meas_start_request;
extern PoreFlex G_meas_cont_request;
extern PoreFlex G_meas_complete_request;
extern apss_start_args_t    G_gpe_start_pwr_meas_read_args;
extern apss_continue_args_t G_gpe_continue_pwr_meas_read_args;
extern apss_complete_args_t G_gpe_complete_pwr_meas_read_args;
extern PoreEntryPoint GPE_apss_start_pwr_meas_read;
extern PoreEntryPoint GPE_apss_continue_pwr_meas_read;
extern PoreEntryPoint GPE_apss_complete_pwr_meas_read;
//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************
#define APSS_INITIALIZE_ID  "APSS Init Aplt\0"

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
// Name:  apss_initialize
//
// Description: Completes all APSS initialization including GPIOs, altitude and
//              mode
//
// Flow:  07/20/11    FN=apss_initialize
//
// changeTags: @fk005c, 
// 
// End Function Specification
errlHndl_t apss_initialize()
{
    errlHndl_t l_err = NULL;
    PoreFlex request;

    // Setup the GPIO init structure to pass to the GPE program
    G_gpe_apss_initialize_gpio_args.error.error = 0;
    G_gpe_apss_initialize_gpio_args.error.ffdc = 0;
    G_gpe_apss_initialize_gpio_args.config0.direction
                                = G_gpio_config[0].direction;
    G_gpe_apss_initialize_gpio_args.config0.drive
                                = G_gpio_config[0].drive;
    G_gpe_apss_initialize_gpio_args.config0.interrupt
                                = G_gpio_config[0].interrupt;
    G_gpe_apss_initialize_gpio_args.config1.direction
                                = G_gpio_config[1].direction;
    G_gpe_apss_initialize_gpio_args.config1.drive = G_gpio_config[1].drive;
    G_gpe_apss_initialize_gpio_args.config1.interrupt
                                = G_gpio_config[1].interrupt;

    // Create/schedule GPE_apss_initialize_gpio and wait for it to complete (BLOCKING)
    TRAC_INFO("Creating request for GPE_apss_initialize_gpio");
    pore_flex_create(&request,                                  // request
                     &G_pore_gpe0_queue,                        // queue
                     (void*)GPE_apss_initialize_gpio,           // GPE entry_point
                     (uint32_t)&G_gpe_apss_initialize_gpio_args,// GPE argument_ptr
                     SSX_SECONDS(5),                            // timeout
                     NULL,                                      // callback
                     NULL,                                      // callback arg
                     ASYNC_REQUEST_BLOCKING);                   // options
    // Schedule the request to be executed
    pore_flex_schedule(&request);

    // Check for a timeout, will create the error log later
    // NOTE: As of 2013/07/16, simics will still fail here on a OCC reset

    if(ASYNC_REQUEST_STATE_TIMED_OUT == request.request.completion_state)
    {
        // For whatever reason, we hit a timeout.  It could be either
        // that the HW did not work, or the request didn't ever make
        // it to the front of the queue.
        // Let's log an error, and include the FFDC data if it was
        // generated.
        TRAC_ERR("Timeout communicating with PORE-GPE for APSS Init");
    }

    TRAC_INFO("GPE_apss_initialize_gpio completed w/rc=0x%08x\n",
              request.request.completion_state);

    // Only continue if completed without errors...
    if (ASYNC_REQUEST_STATE_COMPLETE == request.request.completion_state)
    {
        // @ly003c - start
        // Setup the composite mode structure to pass to the GPE program
        G_gpe_apss_set_composite_mode_args.error.error = 0;
        G_gpe_apss_set_composite_mode_args.error.ffdc = 0;
        G_gpe_apss_set_composite_mode_args.config.numAdcChannelsToRead =
                        G_apss_composite_config.numAdcChannelsToRead;
        G_gpe_apss_set_composite_mode_args.config.numGpioPortsToRead =
                        G_apss_composite_config.numGpioPortsToRead;

        // Create/schedule GPE_apss_set_composite_mode and wait for it to complete (BLOCKING)
        TRAC_INFO("Creating request for GPE_apss_set_composite_mode");
        pore_flex_create(&request,                                    // request
                         &G_pore_gpe0_queue,                          // queue
                         (void*)GPE_apss_set_composite_mode,          // GPE entry_point
                         (uint32_t)&G_gpe_apss_set_composite_mode_args,// GPE argument_ptr
                         SSX_SECONDS(5),                              // timeout
                         NULL,                                        // callback
                         NULL,                                        // callback arg
                         ASYNC_REQUEST_BLOCKING);                     // options
        pore_flex_schedule(&request);

        // Check for a timeout, will create the error log later
        // NOTE: As of 2013/07/16, simics will still fail here on a OCC reset

        if(ASYNC_REQUEST_STATE_TIMED_OUT == request.request.completion_state)
        {
            // For whatever reason, we hit a timeout.  It could be either
            // that the HW did not work, or the request didn't ever make
            // it to the front of the queue.
            // Let's log an error, and include the FFDC data if it was
            // generated.
            TRAC_ERR("Timeout communicating with PORE-GPE for APSS Init");
        }

        TRAC_INFO("GPE_apss_set_composite_mode completed w/rc=0x%08x",
                  request.request.completion_state);

        if (ASYNC_REQUEST_STATE_COMPLETE != request.request.completion_state)
        {
            /*
             * @errortype
             * @moduleid    PSS_MID_APSS_INIT
             * @reasoncode  INTERNAL_FAILURE
             * @userdata1   GPE returned rc code
             * @userdata2   GPE returned abort code
             * @userdata4   ERC_PSS_COMPOSITE_MODE_FAIL
             * @devdesc     Failure from GPE for setting composite mode on
             *              APSS
             */
            l_err = createErrl(PSS_MID_APSS_INIT,               // i_modId,
                               INTERNAL_FAILURE,                // i_reasonCode,
                               ERC_PSS_COMPOSITE_MODE_FAIL,     // extended reason code
                               ERRL_SEV_UNRECOVERABLE,          // i_severity
                               NULL,                            // TODO - tracDesc_t i_trace,
                               0x0000,                          // i_traceSz,
                               request.request.completion_state,       // i_userData1,
                               request.request.abort_state);           // i_userData2
            addUsrDtlsToErrl(l_err,
                             (uint8_t*)&G_gpe_apss_set_composite_mode_args,
                             sizeof(G_gpe_apss_set_composite_mode_args),
                             ERRL_STRUCT_VERSION_1,             // TODO
                             ERRL_USR_DTL_TRACE_DATA);

            // Returning an error log will cause us to go to safe
            // state so we can report error to FSP
        }
        // @ly003c - end


        TRAC_INFO("apss_initialize: Creating request G_meas_start_request.");
        //Create the request for measure start. Scheduling will happen in apss.c
        pore_flex_create(&G_meas_start_request,
                         &G_pore_gpe0_queue,                          // queue
                         (void*)GPE_apss_start_pwr_meas_read,         // entry_point
                         (uint32_t)&G_gpe_start_pwr_meas_read_args,   // entry_point arg
                         SSX_WAIT_FOREVER,                            // no timeout
                         NULL,                                        // callback
                         NULL,                                        // callback arg
                         ASYNC_CALLBACK_IMMEDIATE);                   // options
						 
        TRAC_INFO("apss_initialize: Creating request G_meas_cont_request.");						 
        //Create the request for measure continue. Scheduling will happen in apss.c
        pore_flex_create(&G_meas_cont_request,
                         &G_pore_gpe0_queue,                          // request
                         (void*)GPE_apss_continue_pwr_meas_read,      // entry_point
                         (uint32_t)&G_gpe_continue_pwr_meas_read_args, // entry_point arg
                         SSX_WAIT_FOREVER,                            // no timeout
                         NULL,                                        // callback
                         NULL,                                        // callback arg
                         ASYNC_CALLBACK_IMMEDIATE);                   // options

        TRAC_INFO("apss_initialize: Creating request G_meas_complete_request.");						 
        //Create the request for measure complete. Scheduling will happen in apss.c
        pore_flex_create(&G_meas_complete_request,
                         &G_pore_gpe0_queue,                          // queue
                         (void*)GPE_apss_complete_pwr_meas_read,      // entry_point
                         (uint32_t)&G_gpe_complete_pwr_meas_read_args,// entry_point arg
                         SSX_WAIT_FOREVER,                            // no timeout
                         (AsyncRequestCallback)reformat_meas_data,    // callback,
                         (void*)NULL,                                 // callback arg
                         ASYNC_CALLBACK_IMMEDIATE);                   // options
						 
    }
    else
    {
        /*
         * @errortype
         * @moduleid    PSS_MID_APSS_INIT
         * @reasoncode  INTERNAL_FAILURE
         * @userdata1   GPE returned rc code
         * @userdata2   GPE returned abort code
         * @userdata4   ERC_PSS_GPIO_INIT_FAIL
         * @devdesc     Failure from GPE for gpio initialization on APSS
         */
        l_err = createErrl(PSS_MID_APSS_INIT,           // i_modId,
                           INTERNAL_FAILURE,            // i_reasonCode,
                           ERC_PSS_GPIO_INIT_FAIL,      // extended reason code
                           ERRL_SEV_UNRECOVERABLE,      // i_severity
                           NULL,                        // tracDesc_t i_trace,
                           0x0000,                      // i_traceSz,
                           request.request.completion_state,   // i_userData1,
                           request.request.abort_state);       // i_userData2
        addUsrDtlsToErrl(l_err,
                         (uint8_t*)&G_gpe_apss_initialize_gpio_args,
                         sizeof(G_gpe_apss_initialize_gpio_args),
                         ERRL_STRUCT_VERSION_1,        // TODO
                         ERRL_USR_DTL_TRACE_DATA);

        // Returning an error log will cause us to go to safe
        // state so we can report error to FSP
    }

    return l_err;
}


// Function Specification
//
//  Name: apssInitApplet
//
//  Description: Entry point function
//
//  Flow: --/--/----     FN= None
//
// End Function Specification
errlHndl_t apssInitApplet(void * i_arg)
{
    errlHndl_t l_err = NULL;

    // Initialize APSS
    l_err = apss_initialize();

    if( NULL != l_err)
    {
        TRAC_ERR("APSS Init failed! (retrying) ErrLog[%p]", l_err );
        setErrlSevToInfo(l_err);
        // commit & delete
        commitErrl( &l_err );

        // Retry one more time
        l_err = apss_initialize();

        if( NULL != l_err)
        {
            TRAC_ERR("APSS Init failed again! ErrLog[%p]",l_err);
        }
    }

    return l_err;
}
/*****************************************************************************/
// Image Header
/*****************************************************************************/
// @dw000 - Add applet ID arg to IMAGE_HEADER macro call
IMAGE_HEADER (G_apss_initialize,apssInitApplet,APSS_INITIALIZE_ID,OCC_APLT_APSS_INIT);

