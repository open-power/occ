/******************************************************************************
// @file dcomMasterToSlave.c
// @brief Master OCC to Slave OCC communication handler
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section dcomMasterToSlave.c DCOMMASTERTOSLAVE.C
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *   @00                abagepa   10/10/2011  Created
 *   @th002             thallet   11/01/2011  Misc Changes for Nov 1st Milestone
 *   @pb00A             pbavari   11/15/2011  Set G_ApssPwrMeasCompleted to TRUE
 *                                            for SVN SIMICS
 *   @th005             thallet   11/01/2011  Added STATIC_ASSERT checks
 *   @01                tapiar    11/12/2011  Stage 3 updates
 *   @rc003             rickylie  02/03/2012  Verify & Clean Up OCC Headers & Comments
 *   @nh001             neilhsu   05/23/2012  Add missing error log tags
 *   @th010             thallet   07/11/2012  Pstate Enablement
 *   @th022             thallet   10/08/2012  Moved CNFG Data commands to diff file
 *   @ly003  861535     lychen    11/19/2012  Remove APSS configuration/gathering of Altitude & Temperature
 *   @th034  879027     thallet   04/18/2013  Broadcast Power Data over PBAX
 *   @at013  878755     alvinwan  04/17/2013  OCC power capping implementation
 *   @th032             thallet   04/16/2013  Tuleta HW Bringup
 *   @th040  887069     thallet   06/11/2013  Support Nom & FFO Freq Setting for Mnfg
 *   @fk001  879727     fmkassem  04/16/2013  OCC powercap support.
 *   @gs007  888247     gjsilva   06/19/2013  OCC mnfg support for frequency distribution
 *   @gs015  905166     gjsilva   11/04/2013  Full support for IPS function
 *   @gs016  905781     gjsilva   11/12/2013  Fix for Master->Slave doorbell loss of synchronization
 *   @gs017  905990     gjsilva   11/13/2013  Full support for tunable parameters
 *   @rt004   905638    tapiar    11/13/2013  Tunable parameters
 *   @gm014  907707     milesg    12/05/2013  don't panic on pbax_send timeout failures
 *   @jh00a  909791     joshych   12/16/2013  Enhance APSS pwr meas trace
 *   @sb003  908290     sbroyles  12/18/2013  Test BCE request states
 *   @jh00b  910184     joshych   01/10/2014  Add check for checkstop
 *   @sb013  911625     sbroyles  01/15/2014  Fix to 908290 changes
 *   @fk005  911760     fmkassem  01/14/2014  APSS data collection retry support.
 *   @gs025  913663     gjsilva   01/30/2014  Full fupport for soft frequency boundaries
 *   @sb023  913769     sbroyles  02/08/2014  Fix task_dcom_tx_slv_inbox !idle &&
 *                                            !complete hang.
 *   @gm028  911670     milesg    02/27/2014  Fixed compiler fails from stradale
 *   @gs027  918066     gjsilva   03/12/2014  Misc functions from ARL
 *   @wb001  919163     wilbryan  03/06/2014  Updating error call outs, descriptions, and severities
 *
 *  @endverbatim
 *
 *///*************************************************************************/

#ifndef _DCOMMASTERTX_C
#define _DCOMMASTERTX_C

/** \defgroup Master-Slave Communication
 *
 */
//*************************************************************************
// Includes
//*************************************************************************
#include <pgp_pmc.h>
#include "pgp_pba.h"
#include <rtls.h>
#include <apss.h>
#include <dcom.h>
#include <dcom_service_codes.h>
#include <occ_service_codes.h>
#include <trac.h>
#include <proc_pstate.h>
#include <amec_sys.h>
#include <amec_master_smh.h>

//*************************************************************************
// Externs
//*************************************************************************

extern UINT8 g_amec_tb_record; // from amec_amester.c for syncronized traces

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************

//*************************************************************************
// Structures
//*************************************************************************

//*************************************************************************
// Globals
//*************************************************************************


// SSX Block Copy Request for the Slave Inbox Transmit Queue
BceRequest G_slv_inbox_tx_pba_request;

// Used by the master to house the doorbell data that is sent in
// the master multicast doorbell, stating that it put slave inbox in main memory.
dcom_slv_inbox_doorbell_t G_dcom_slv_inbox_doorbell_tx;

// Make sure that the Slave Inbox TX Buffer is 256B, otherwise cause
// error on the compile.
STATIC_ASSERT(  (NUM_BYTES_IN_SLAVE_INBOX != (sizeof(G_dcom_slv_inbox_tx)/MAX_OCCS))  );

// Store return code and failed packet # from pbax_send so we can trace it
uint32_t G_pbax_rc = 0;
uint32_t G_pbax_packet = 0xffffffff;

//Used to keep count of number of APSS data collection fails.
uint8_t G_apss_fail_updown_count = 0x00;

//*************************************************************************
// Function Prototypes
//*************************************************************************

//*************************************************************************
// Functions
//*************************************************************************

// Function Specification
//
// Name: dcom_build_slv_inbox
//
// Description: The purpose of this function is to copy the Control Data into
//              the Slave inbox structures so that the Master can send it out.
//              Build the slave inboxes so master can send them to slaves
//
// Flow:  06/19/13    FN=dcom_build_slv_inbox
//
// changedby: @at013c, @fk001c
//
// End Function Specification

uint32_t dcom_build_slv_inbox(void)
{
    // Locals
    uint32_t l_addr_of_slv_inbox_in_main_mem = 0;
    uint32_t l_slv_idx = 0;
    uint32_t l_core_idx = 0;
    uint32_t l_cntr_idx = 0;
    uint32_t l_mem_intr_idx = 0;

    static uint8_t      L_seq = 0xFF;

    L_seq++;

    //If there was a pbax_send failure, trace it here since we can't do it in the critical
    //interrupt context.
    if(G_pbax_rc)
    {
        TRAC_INFO("PBAX Send Failure in transimitting multicast doorbell - RC[%08X], packet[%d]", G_pbax_rc, G_pbax_packet);
    }


    //INBOX...............
    //For each occ slave collect its occ data.
    for(; l_slv_idx < MAX_OCCS; l_slv_idx++)
    {
        G_dcom_slv_inbox_tx[l_slv_idx].seq = L_seq;
        G_dcom_slv_inbox_tx[l_slv_idx].version = 0;
        //G_dcom_slv_inbox_tx[l_slv_idx].ambient_temp =   //Activate when ambient temperature is available.
        //G_dcom_slv_inbox_tx[l_slv_idx].altitude =  //Acitivate when altitude is available.

        //TODO: adc,gpio,and tod are only sent here for sanity check and for bringup only.
        //If the values are needed by the slaves, they should use the values sent in the doorbell.
        //Probably remove them after bringup is complete.
        memcpy( G_dcom_slv_inbox_tx[l_slv_idx].adc, G_apss_pwr_meas.adc, sizeof(G_dcom_slv_inbox_tx[l_slv_idx].adc));
        memcpy( G_dcom_slv_inbox_tx[l_slv_idx].gpio, G_apss_pwr_meas.gpio, sizeof(G_dcom_slv_inbox_tx[l_slv_idx].gpio));
        memcpy( G_dcom_slv_inbox_tx[l_slv_idx].tod, &G_apss_pwr_meas.tod, sizeof(  G_dcom_slv_inbox_tx[l_slv_idx].tod ));  //TODO - this doesn't work

        memset( G_dcom_slv_inbox_tx[l_slv_idx].occ_fw_mailbox, 0, sizeof( G_dcom_slv_inbox_tx[l_slv_idx].occ_fw_mailbox ));

        //Collect frequency data for each core
        for( l_core_idx = 0; l_core_idx < MAX_CORES; l_core_idx++)
        {
            // TODO - uncomment once macro exists
            //G_dcom_slv_inbox_tx[l_slv_idx].freq250usp0cy = FREQ250USPC[l_core_idx];
            G_dcom_slv_inbox_tx[l_slv_idx].freq250usp0cy[l_core_idx] = l_core_idx;
        }

        //collect data for each centaur throttle
        for( l_cntr_idx = 0; l_cntr_idx < MAX_CENTAUR_THROTTLES; l_cntr_idx++)
        {
            // TODO - uncomment once macro exists
            //G_dcom_slv_inbox_tx[l_slv_idx].memsp2msP0MxCyPz = MEMSP2MSP0MxCyPz[l_cntr_idx];
            G_dcom_slv_inbox_tx[l_slv_idx].memsp2msP0MxCyPz[l_cntr_idx] = l_cntr_idx;
        }

        //collect data for each mem interleave group throttle
        for( l_mem_intr_idx = 0; l_mem_intr_idx < MAX_MEM_INTERLEAVE_GROUP_THROTTLES; l_mem_intr_idx++)
        {
            // TODO - uncomment once macro exists
            //G_dcom_slv_inbox_tx[l_slv_idx].memsp2msP0IGx = MEMSP2MSP0IG[l_mem_intr_idx];
            G_dcom_slv_inbox_tx[l_slv_idx].memsp2msP0IGx[l_mem_intr_idx] = l_mem_intr_idx;
        }

        //collect mnfg parameters that need to be sent to slaves
        G_dcom_slv_inbox_tx[l_slv_idx].foverride_enable = g_amec->mnfg_parms.auto_slew;
        G_dcom_slv_inbox_tx[l_slv_idx].foverride = g_amec->mnfg_parms.foverride;
        G_dcom_slv_inbox_tx[l_slv_idx].emulate_oversub = AMEC_INTF_GET_OVERSUBSCRIPTION_EMULATION();

        //collect Idle Power Saver parameters to be sent to slaves
        G_dcom_slv_inbox_tx[l_slv_idx].ips_freq_request = g_amec->mst_ips_parms.freq_request;

        //collect Tunable Paramaters to be sent to slaves
        G_dcom_slv_inbox_tx[l_slv_idx].alpha_up = G_mst_tunable_parameter_table_ext[0].adj_value;
        G_dcom_slv_inbox_tx[l_slv_idx].alpha_down = G_mst_tunable_parameter_table_ext[1].adj_value;
        G_dcom_slv_inbox_tx[l_slv_idx].sample_count_util = G_mst_tunable_parameter_table_ext[2].adj_value;
        G_dcom_slv_inbox_tx[l_slv_idx].step_up = G_mst_tunable_parameter_table_ext[3].adj_value;
        G_dcom_slv_inbox_tx[l_slv_idx].step_down = G_mst_tunable_parameter_table_ext[4].adj_value;
        G_dcom_slv_inbox_tx[l_slv_idx].epsilon_perc = G_mst_tunable_parameter_table_ext[5].adj_value;
        G_dcom_slv_inbox_tx[l_slv_idx].tlutil = G_mst_tunable_parameter_table_ext[6].adj_value;
        G_dcom_slv_inbox_tx[l_slv_idx].tunable_param_overwrite = G_mst_tunable_parameter_overwrite;

        //collect soft frequency bondaries to be sent to slaves
        G_dcom_slv_inbox_tx[l_slv_idx].soft_fmin = G_mst_soft_fmin;
        G_dcom_slv_inbox_tx[l_slv_idx].soft_fmax = G_mst_soft_fmax;

        //send trace recording bit to slaves for synchronized tracing. //Lefurgy
        G_dcom_slv_inbox_tx[l_slv_idx].tb_record = g_amec_tb_record;

        G_dcom_slv_inbox_tx[l_slv_idx].counter++;  // @th002

        memcpy( &G_dcom_slv_inbox_tx[l_slv_idx].sys_mode_freq,
                &G_sysConfigData.sys_mode_freq,
                sizeof( freqConfig_t ));   // @th040
    }

    //Clear the tunable parameter overwrite once we collect the new values
    G_mst_tunable_parameter_overwrite = 0;

    //@01a
    dcom_build_occfw_msg( SLAVE_INBOX );

    // Copy Data from one DCM pair's Outbox to other DCM pair's inbox
    dcom_build_dcm_sync_msg( SLAVE_INBOX );   // @th010

    l_addr_of_slv_inbox_in_main_mem = dcom_which_buffer();

    //DOORBELL.................
    //Prepare data for doorbell.  This is sent to all OCCs

    G_dcom_slv_inbox_doorbell_tx.pob_id = G_pob_id;
    G_dcom_slv_inbox_doorbell_tx.magic1 = PBAX_MAGIC_NUMBER2_32B;
    G_dcom_slv_inbox_doorbell_tx.addr_slv_inbox_buffer0 = l_addr_of_slv_inbox_in_main_mem;

    memcpy( (void *) &G_dcom_slv_inbox_doorbell_tx.pcap,
            (void *) &G_master_pcap_data,               // @at013c @fk001c
            sizeof(pcap_config_data_t));

    G_dcom_slv_inbox_doorbell_tx.ppb_fmax = G_sysConfigData.master_ppb_fmax; //master ppb fmax is calculated in amec_ppb_fmax_calc

    memcpy( (void *) &G_dcom_slv_inbox_doorbell_tx.adc[0],
            (void *) &G_apss_pwr_meas.adc[0],
            sizeof( G_dcom_slv_inbox_doorbell_tx.adc ));

    G_dcom_slv_inbox_doorbell_tx.gpio[0] = G_apss_pwr_meas.gpio[0];
    G_dcom_slv_inbox_doorbell_tx.gpio[1] = G_apss_pwr_meas.gpio[1];
    G_dcom_slv_inbox_doorbell_tx.tod = G_apss_pwr_meas.tod;

    G_dcom_slv_inbox_doorbell_tx.magic_counter++;
    G_dcom_slv_inbox_doorbell_tx.magic2 = PBAX_MAGIC_NUMBER_32B;

    return l_addr_of_slv_inbox_in_main_mem;
}


// Function Specification
//
// Name:  dcom_which_buffer
//
// Description: Determines which buffer in the 'double buffer'
//              or ping/pong to use. Basically alternates between
//              returning the ping or the pong address
//
// Flow:  08/23/11    FN=dcom_which_buffer
//
// End Function Specification

uint32_t dcom_which_buffer(void)
{
    //Locals
    uint32_t l_mem_address = ADDR_SLAVE_INBOX_MAIN_MEM_PONG;

    // switch back and forth based on tick
    if( CURRENT_TICK & 1 )
    {
        l_mem_address = ADDR_SLAVE_INBOX_MAIN_MEM_PING;
    }

    return l_mem_address;
}


// Function Specification
//
// Name: task_dcom_tx_slv_inbox
//
// Description: Copy slave inboxes from SRAM to main memory
//              so master can send data to slave
//
// Flow:  08/23/11    FN=task_dcom_tx_slv_inbox
//
// Task Flags: RTL_FLAG_MSTR, RTL_FLAG_OBS, RTL_FLAG_ACTIVE
//
// Changes: @fk005c
//
// End Function Specification
void task_dcom_tx_slv_inbox( task_t *i_self)
{
    static bool l_error = FALSE;
    uint32_t    l_orc = OCC_SUCCESS_REASON_CODE;
    uint32_t    l_orc_ext = OCC_NO_EXTENDED_RC;     // @nh001a
    uint64_t    l_start = ssx_timebase_get();
    bool        l_pwr_meas = FALSE;                 // @th002
    bool        l_request_reset = FALSE;            // @fk005a
    bool        l_ssx_failure = FALSE;
    // @sb003 Use a static local bool to track whether the BCE request used
    // here has ever been successfully created at least once
    static bool L_bce_slv_inbox_tx_request_created_once = FALSE;

    DCOM_DBG("4. TX Slave Inbox\n");

    do
    {
        // If we are in standby, we need to fake out
        // the APSS data since we aren't talking to APSS.
        if( OCC_STATE_STANDBY == CURRENT_STATE() )   // @th022
        {
           G_ApssPwrMeasCompleted = TRUE;            // @th022
        }

        l_pwr_meas = G_ApssPwrMeasCompleted;   // @th002

        //did apss pwr complete?
        if( l_pwr_meas == TRUE )   // @th002
        {
#ifdef DCOM_DEBUG
            uint64_t l_end = ssx_timebase_get();
            DCOM_DBG("Got APSS after waiting %d us\n",(int)( (l_end-l_start) / ( SSX_TIMEBASE_FREQUENCY_HZ / 1000000 ) ));
#endif

            //@fk005a
            APSS_SUCCESS();

            // build/setup inboxes
            uint32_t l_addr_in_mem = dcom_build_slv_inbox();
            uint32_t l_ssxrc = 0;

            // @sb003
            // See dcomMasterRx.c/task_dcom_rx_slv_outboxes for details on the
            // checking done here before creating and scheduling the request.
            bool l_proceed_with_request_and_schedule = FALSE;
            int l_req_idle = async_request_is_idle(&(G_slv_inbox_tx_pba_request.request));
            int l_req_complete = async_request_completed(&(G_slv_inbox_tx_pba_request.request));

            if (!L_bce_slv_inbox_tx_request_created_once)
            {
                // Do this case first, all other cases assume that this is
                // true!
                // This is the first time we have created a request so
                // always proceed with request create and schedule
                l_proceed_with_request_and_schedule = TRUE;
            }
            else if (l_req_idle && l_req_complete)
            {
                // Most likely case first.  The request was created
                // and scheduled and has completed without error.  Proceed.
                // Proceed with request create and schedule.
                l_proceed_with_request_and_schedule = TRUE;
            }
            else if (l_req_idle && !l_req_complete)
            {
                // There was an error on the schedule request or the request
                // was scheduled but was canceled, killed or errored out.
                // Proceed with request create and schedule.
                l_proceed_with_request_and_schedule = TRUE;
                // Trace important information from the request
                TRAC_INFO("BCE slv inbox tx request idle but not complete, \
                          callback_rc=%d options=0x%x state=0x%x abort_state=0x%x \
                          completion_state=0x%x",
                          G_slv_inbox_tx_pba_request.request.callback_rc,
                          G_slv_inbox_tx_pba_request.request.options,
                          G_slv_inbox_tx_pba_request.request.state,
                          G_slv_inbox_tx_pba_request.request.abort_state,
                          G_slv_inbox_tx_pba_request.request.completion_state);
                TRAC_INFO("Proceeding with BCE slv inbox tx request and schedule");
            }
            else if (!l_req_idle && !l_req_complete)
            {
                // The request was created and scheduled but is still in
                // progress or still enqueued OR there was some error
                // creating the request so it was never scheduled.  The latter
                // case is unlikely and will generate an error message when
                // it occurs.  It will also have to happen after the request
                // was created at least once or we'll never get here.  If the
                // request does fail though before the state parms in the
                // request are reset (like a bad parameter error), then this
                // represents a hang condition that we can't recover from.
                // DO NOT proceed with request create and schedule.
                l_proceed_with_request_and_schedule = FALSE;
                // Trace important information from the request
                TRAC_INFO("BCE slv inbox tx request not idle and not complete, \
                          callback_rc=%d options=0x%x state=0x%x abort_state=0x%x \
                          completion_state=0x%x",
                          G_slv_inbox_tx_pba_request.request.callback_rc,
                          G_slv_inbox_tx_pba_request.request.options,
                          G_slv_inbox_tx_pba_request.request.state,
                          G_slv_inbox_tx_pba_request.request.abort_state,
                          G_slv_inbox_tx_pba_request.request.completion_state);
                TRAC_INFO("NOT proceeding with BCE slv inbox tx request and schedule");
            }
            // else {// This case can't happen, ignore it.}

            // @sb003 Only proceed if the BCE request state checked out
            if (l_proceed_with_request_and_schedule)
            {
                // set up inboxes copy request
                l_ssxrc = bce_request_create(
                                &G_slv_inbox_tx_pba_request,        // block copy object
                                &G_pba_bcue_queue,                  // mainstore to sram copy engine
                                l_addr_in_mem,                      // mainstore address
                                (uint32_t) &G_dcom_slv_inbox_tx[0], // sram starting address
                                sizeof(G_dcom_slv_inbox_tx),        // size of copy            // @th002
                                SSX_WAIT_FOREVER,                   // no timeout
                                (AsyncRequestCallback)dcom_tx_slv_inbox_doorbell, // call back
                                NULL,                               // call back arguments
                                ASYNC_CALLBACK_IMMEDIATE            // callback mask
                                );

                if(l_ssxrc != SSX_OK)
                {
                    /* @
                     * @errortype
                     * @moduleid    DCOM_MID_TASK_TX_SLV_INBOX
                     * @reasoncode  SSX_GENERIC_FAILURE
                     * @userdata1   N/A
                     * @userdata4   ERC_BCE_REQUEST_CREATE_FAILURE
                     * @devdesc     SSX BCE related failure
                     */
                    TRAC_ERR("PBA request create failure rc=[%08X]",l_ssxrc);
                    l_orc = SSX_GENERIC_FAILURE;                    // @nh001c
                    l_orc_ext = ERC_BCE_REQUEST_CREATE_FAILURE;     // @nh001a
                    l_ssx_failure = TRUE;                           // @wb001
                    break;
                }

                // @sb003 Request created at least once
                L_bce_slv_inbox_tx_request_created_once = TRUE; // @sb013
                l_ssxrc = bce_request_schedule(&G_slv_inbox_tx_pba_request);          // actual copying

                if(l_ssxrc != SSX_OK)
                {
                    /* @
                     * @errortype
                     * @moduleid    DCOM_MID_TASK_TX_SLV_INBOX
                     * @reasoncode  SSX_GENERIC_FAILURE
                     * @userdata1   N/A
                     * @userdata4   ERC_BCE_REQUEST_SCHEDULE_FAILURE
                     * @devdesc     SSX BCE related failure
                     */
                    TRAC_ERR("PBA request schedule failure rc=[%08X]",l_ssxrc);
                    l_orc = SSX_GENERIC_FAILURE;                    // @nh001c
                    l_orc_ext = ERC_BCE_REQUEST_SCHEDULE_FAILURE;   // @nh001a
                    l_ssx_failure = TRUE;                           // @wb001
                    break;
                }
            }
            // @sb023 Moved the break statement here in case we decide not to
            // schedule the BCE request.
            break;
        }
        else
        {
            // check time and break out if we reached limit
            // @th032 -- TODO: shrink this later depending on how much
            // work we are doing in RTL
            if ((ssx_timebase_get() - l_start) < SSX_MICROSECONDS(150))
            {
                continue;
            }
            else
            {
                //Failure occurred, step up the FAIL_COUNT
                APSS_FAIL();


                if (G_apss_fail_updown_count >= APSS_DATA_FAIL_MAX)
                {
                    TRAC_ERR("task_dcom_tx_slv_inbox: APSS data collection failure exceeded threshold. fail_count=%i, threshold:%i",
                             G_apss_fail_updown_count, APSS_DATA_FAIL_MAX);

                    /* @
                     * @errortype
                     * @moduleid    DCOM_MID_TASK_TX_SLV_INBOX
                     * @reasoncode  INTERNAL_FAILURE
                     * @userdata1   N/A
                     * @userdata4   OCC_NO_EXTENDED_RC
                     * @devdesc     Time out waiting on power measurement completion
                     */
                    TRAC_ERR("Timed out waiting apss meas completion (dcom_start:%d us, apss_start:%d us, apss_end:%d us)",
                             (int) ((l_start)/(SSX_TIMEBASE_FREQUENCY_HZ/1000000)),
                             (int) ((G_gpe_apss_time_start)/(SSX_TIMEBASE_FREQUENCY_HZ/1000000)),
                             (int) ((G_gpe_apss_time_end)/(SSX_TIMEBASE_FREQUENCY_HZ/1000000))); // @jh00ac
                    l_orc = INTERNAL_FAILURE;           // @nh001c
                    l_orc_ext = OCC_NO_EXTENDED_RC;     // @nh001a
                    l_request_reset = TRUE;             // @fk005a

                }
                break;
            }
        }

    } while (1);  // @th002

    //If an error exists and we have not logged one before or there's a new request to reset, then log error.
    if ( (l_orc != OCC_SUCCESS_REASON_CODE) && ((l_error == FALSE) || (l_request_reset == TRUE)))
    {
        // create and commit error only once.
        errlHndl_t  l_errl = createErrl(
                    DCOM_MID_TASK_TX_SLV_INBOX,     //modId
                    l_orc,                          //reasoncode    // @nh001c
                    l_orc_ext,                      //Extended reason code
                    ERRL_SEV_UNRECOVERABLE,         //Severity
                    NULL,                           //Trace Buf
                    DEFAULT_TRACE_SIZE,             //Trace Size
                    0,                              //userdata1
                    0                               //userdata2
                    );

        // @wb001 -- Callout to firmware
        addCalloutToErrl(l_errl,
                         ERRL_CALLOUT_TYPE_COMPONENT_ID,
                         ERRL_COMPONENT_ID_FIRMWARE,
                         ERRL_CALLOUT_PRIORITY_HIGH);

        if ( FALSE == l_ssx_failure ) // @wb001
        {
            // Callout to processor
            addCalloutToErrl(l_errl,
                             ERRL_CALLOUT_TYPE_HUID,
                             G_sysConfigData.proc_huid,
                             ERRL_CALLOUT_PRIORITY_LOW);

            // Callout to APSS
            addCalloutToErrl(l_errl,
                             ERRL_CALLOUT_TYPE_HUID,
                             G_sysConfigData.apss_huid,
                             ERRL_CALLOUT_PRIORITY_LOW);
        }

        if (l_request_reset)
        {
            REQUEST_RESET(l_errl);
        }
        else
        {
            commitErrl(&l_errl);
        }

        l_error = TRUE;
        l_request_reset = FALSE;
    }

}


// Function Specification
//
// Name: dcom_tx_slv_inbox_doorbell
//
// Description: transmit doorbells to slaves
//              from master
//
// Flow:  08/23/11    FN=dcom_tx_slv_inbox_doorbell
//
// End Function Specification
void dcom_tx_slv_inbox_doorbell( void )
{
    int         l_pbarc      = 0;
    int         l_tmp        = 0;
    int         l_jj         = 0;
    uint64_t    l_start      = ssx_timebase_get();

    /// Caclulate how many 8 byte packets are in the doorbell
    l_tmp = sizeof( G_dcom_slv_inbox_doorbell_tx ) / sizeof(uint64_t);

    /// Loop through all packets, sending one at a time.  It should send
    /// the previous packet almost immediately, but it is worth noting that
    /// it is *possible* that the PowerBus is backed up, in which case it may
    /// take a short amount of time (~1us <TBD>) to send each packet.
    /// Estimated transfer time, under normal circumstances is 1kB/1us.
    for(l_jj=0; l_jj<l_tmp; l_jj++)
    {
        // Send 8 bytes of multicast doorbell
        l_pbarc = _pbax_send( &G_pbax_multicast_target, //gm014
                              G_dcom_slv_inbox_doorbell_tx.words[l_jj],
                              SSX_MICROSECONDS(15));

        //Set this global so we know to trace this in the non-critical interrupt context
        G_pbax_rc = l_pbarc;
        if ( (l_pbarc != 0 ) )
        {
            G_pbax_packet = l_jj;
            //Trace causes a panic in a critical interrupt! Don't trace here!(tries to pend a semaphore)

            /// Break out of for loop and stop sending the rest of the doorbell
            /// packets, since this likely occured b/c of a timeout.
            break;
        }
    }

    // <TULETA HW BRINGUP TIMING> @th032
    uint64_t l_delta = (ssx_timebase_get() - l_start);
    G_dcomTime.master.doorbellStartTx = l_start;
    G_dcomTime.master.doorbellStopTx = ssx_timebase_get();
    G_dcomTime.master.doorbellMaxDeltaTx = (l_delta > G_dcomTime.master.doorbellMaxDeltaTx) ?
        l_delta : G_dcomTime.master.doorbellMaxDeltaTx;
    G_dcomTime.master.doorbellSeq = G_dcom_slv_inbox_doorbell_tx.magic_counter;
    G_dcomTime.master.doorbellNumSent++;
    // </TULETA HW BRINGUP TIMING> @th032

    DCOM_DBG("Sent multicast doorbell\n");
}  // @th034 - rewrote most of functino

#endif //_DCOMMASTERTOSLAVE_C

