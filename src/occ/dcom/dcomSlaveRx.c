/******************************************************************************
// @file dcomSlaveRx.c
// @brief Slave OCC to Master OCC communication handler
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section dcomSlaveRx.c DCOMSLAVERX.C
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *   @th022             thallet   10/08/2012  Changes for OCC Comm
 *   @th024             thallet   10/16/2012  Cleanup to use modifier function
 *   @th034  879027     thallet   04/18/2013  Broadcast Critical Power over PBAX
 *   @th032             thallet   04/26/2013  Added a bunch of timings for PBAX characterization
 *   @gs016  905781     gjsilva   11/12/2013  Fix for Master->Slave doorbell loss of synchronization
 *   @rt004  908817     tapiar    12/11/2013  Update pcap infromation via write_data_pcap
 *                                            once doorbell data is verfied
 *   @sb003  908290     sbroyles  12/18/2013  Test BCE request states
 *   @sb013  911625     sbroyles  01/15/2014  Fix to 908290 changes
 *   @wb003  920760     wilbryan  03/25/2014  Update SRCs to match TPMD SRCs
 *   @gm037  925908     milesg    05/07/2014  Redundant OCC/APSS support
 *  @endverbatim
 *
 *///*************************************************************************/

#ifndef _DCOMSLAVERX_C
#define _DCOMSLAVERX_C

/** \defgroup Slave to Master Communication
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
#include <amec_data.h> // @rt004a

//*************************************************************************
// Externs
//*************************************************************************

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

// SSX Block Copy Request for the Slave Inbox Receive Queue
BceRequest G_slv_inbox_rx_pba_request;

// SSX PBAX Request for Multicast PBAX Queue (Master Doorbell to Slaves)
PbaxRequest G_pbax_multicast_request;  //does not get used should it??

// SSX PBAX Request for Receiving PBAX Messages
PbaxRequest G_pbax_rx_request;

// Used by the slave to house the doorbell data that is received from
// the master multicast doorbell, stating that it put slave inbox in main memory.
dcom_slv_inbox_doorbell_t G_dcom_slv_inbox_doorbell_rx;

// Make sure that the Slave Inbox RX Buffer is 256B, otherwise cause
// error on the compile.
STATIC_ASSERT(  (NUM_BYTES_IN_SLAVE_INBOX != (sizeof(G_dcom_slv_inbox_rx)))  );

//*************************************************************************
// Function Prototypes
//*************************************************************************

//*************************************************************************
// Functions
//*************************************************************************

// Function Specification
//
// Name: dcom_calc_slv_inbox_addr
//
// Description: get slave inbox main memory address
//
// Flow:  08/23/11    FN=dcom_calc_slv_inbox_addr
//
// End Function Specification
uint32_t dcom_calc_slv_inbox_addr(void)
{
    return (G_dcom_slv_inbox_doorbell_rx.addr_slv_inbox_buffer0 + ( G_pob_id.chip_id * sizeof(dcom_slv_inbox_t) ));
}


// Function Specification
//
// Name: dcom_rx_slv_inbox_callback
//
// Description: Callback to set inbox received to true
//
// Flow:            FN=None
//
// End Function Specification
void dcom_rx_slv_inbox_callback( void )
{
    // AMEC will use this, and reset it when done
    G_slv_inbox_received = TRUE;
}


// Function Specification
//
// Name: task_dcom_rx_slv_inbox
//
// Description: Copy Slave inbox from Main Memory to SRAM
//              on slave
//
// Flow:  10/24/11    FN=task_dcom_rx_slv_inbox
//
// Task Flags: RTL_FLAG_OBS, RTL_FLAG_ACTIVE
//
// End Function Specification
void task_dcom_rx_slv_inbox( task_t *i_self)
{
    uint32_t    l_orc = OCC_SUCCESS_REASON_CODE;
    uint32_t    l_orc_ext = OCC_NO_EXTENDED_RC;     // @nh001a
    uint64_t    l_start = ssx_timebase_get();
    uint32_t    l_bytes = 0;
    // @sb003 Use a static local bool to track whether the BCE request used
    // here has ever been successfully created at least once
    static bool L_bce_slv_inbox_rx_request_created_once = FALSE;

    DCOM_DBG("1. RX Slave Inbox\n");

    // Increment debug counter
    G_dcomSlvInboxCounter.totalTicks++;
    G_dcomTime.slave.doorbellStartWaitRx = l_start;

    do
    {
        l_bytes =  dcom_rx_slv_inbox_doorbell();
        // doorbell from the master
        if(l_bytes >= sizeof(G_dcom_slv_inbox_doorbell_rx))
        {

            // looks like we got a valid doorbell so notify slave
            // code of pcap info @rt004a
            amec_data_write_pcap();

#ifdef DCOM_DEBUG
            uint64_t l_end = ssx_timebase_get();
            DCOM_DBG("Got Doorbell from Master after waiting %d us\n",(int)( (l_end-l_start) / ( SSX_TIMEBASE_FREQUENCY_HZ / 1000000 ) ));
#endif
            // <TULETA HW BRINGUP TIMING>
            G_dcomTime.slave.doorbellStopWaitRx = ssx_timebase_get();
            uint64_t l_delta = G_dcomTime.slave.doorbellStopWaitRx - G_dcomTime.slave.doorbellStartWaitRx;
            G_dcomTime.slave.doorbellMaxDeltaWaitRx = (l_delta > G_dcomTime.slave.doorbellMaxDeltaWaitRx) ?
                l_delta : G_dcomTime.slave.doorbellMaxDeltaWaitRx;
            G_dcomTime.slave.doorbellNumRx++;
            // </TULETA HW BRINGUP TIMING>

            // Increment debug counter
            G_dcomSlvInboxCounter.totalSuccessful++;

            // main memory address
            uint32_t l_addr_in_mem = dcom_calc_slv_inbox_addr();

            //check valid address (should be inside inbox addresses range)
            if ( (ADDR_SLAVE_INBOX_MAIN_MEM_PING <= l_addr_in_mem) &&
                 ((ADDR_SLAVE_INBOX_MAIN_MEM_PONG+(sizeof(dcom_slv_inbox_t)*MAX_OCCS)) > l_addr_in_mem) )
            {
                uint32_t l_ssxrc = 0;
                // @sb003
                // See dcomMasterRx.c/task_dcom_rx_slv_outboxes for details on the
                // checking done here before creating and scheduling the request.
                bool l_proceed_with_request_and_schedule = FALSE;
                int l_req_idle = async_request_is_idle(&(G_slv_inbox_rx_pba_request.request));
                int l_req_complete = async_request_completed(&(G_slv_inbox_rx_pba_request.request));

                if (!L_bce_slv_inbox_rx_request_created_once)
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
                    TRAC_INFO("BCE slv inbox rx request idle but not complete, \
                              callback_rc=%d options=0x%x state=0x%x abort_state=0x%x \
                              completion_state=0x%x",
                              G_slv_inbox_rx_pba_request.request.callback_rc,
                              G_slv_inbox_rx_pba_request.request.options,
                              G_slv_inbox_rx_pba_request.request.state,
                              G_slv_inbox_rx_pba_request.request.abort_state,
                              G_slv_inbox_rx_pba_request.request.completion_state);
                    TRAC_INFO("Proceeding with BCE slv inbox rx request and schedule");
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
                    TRAC_INFO("BCE slv inbox rx request not idle and not complete, \
                              callback_rc=%d options=0x%x state=0x%x abort_state=0x%x \
                              completion_state=0x%x",
                              G_slv_inbox_rx_pba_request.request.callback_rc,
                              G_slv_inbox_rx_pba_request.request.options,
                              G_slv_inbox_rx_pba_request.request.state,
                              G_slv_inbox_rx_pba_request.request.abort_state,
                              G_slv_inbox_rx_pba_request.request.completion_state);
                    TRAC_INFO("NOT proceeding with BCE slv inbox rx request and schedule");
                }
                // else {// This case can't happen, ignore it.}

                // @sb003 Only proceed if the BCE request state checked out
                if (l_proceed_with_request_and_schedule)
                {
                    // copy request from main memory to SRAM
                    l_ssxrc = bce_request_create(
                                    &G_slv_inbox_rx_pba_request,        // block copy object
                                    &G_pba_bcde_queue,                  // mainstore to sram copy engine
                                    l_addr_in_mem,                      // mainstore address
                                    (uint32_t)&G_dcom_slv_inbox_rx,     // sram starting address
                                    sizeof(G_dcom_slv_inbox_rx),        // size of copy
                                    SSX_WAIT_FOREVER,                   // no timeout
                                    (AsyncRequestCallback)dcom_rx_slv_inbox_callback, // call back
                                    NULL,                               // call back arguments
                                    ASYNC_CALLBACK_IMMEDIATE            // blocking request
                                    );

                    if(l_ssxrc != SSX_OK)
                    {
                        /* @
                         * @errortype
                         * @moduleid    DCOM_MID_TASK_RX_SLV_INBOX
                         * @reasoncode  SSX_GENERIC_FAILURE
                         * @userdata1   N/A
                         * @userdata4   ERC_BCE_REQUEST_CREATE_FAILURE
                         * @devdesc     SSX BCE related failure
                         */
                        TRAC_ERR("PBA request create failure rc=[%08X]",l_ssxrc);
                        l_orc = SSX_GENERIC_FAILURE;                    // @nh001c
                        l_orc_ext = ERC_BCE_REQUEST_CREATE_FAILURE;     // @nh001a
                        break;
                    }

                    // @sb003 Request created at least once
                    L_bce_slv_inbox_rx_request_created_once = TRUE; // @sb013
                    l_ssxrc = bce_request_schedule(&G_slv_inbox_rx_pba_request);    // actual copying

                    if(l_ssxrc != SSX_OK)
                    {
                        /* @
                         * @errortype
                         * @moduleid    DCOM_MID_TASK_RX_SLV_INBOX
                         * @reasoncode  SSX_GENERIC_FAILURE
                         * @userdata1   N/A
                         * @userdata4   ERC_BCE_REQUEST_SCHEDULE_FAILURE
                         * @devdesc     SSX BCE related failure
                         */
                        TRAC_ERR("PBA request schedule failure rc=[%08X]",l_ssxrc);
                        l_orc = SSX_GENERIC_FAILURE;                    // @nh001c
                        l_orc_ext = ERC_BCE_REQUEST_SCHEDULE_FAILURE;   // @nh001a
                        break;
                    }
                    break;   // @th002
                }
            }
            else
            {
                /* @
                 * @errortype
                 * @moduleid    DCOM_MID_TASK_RX_SLV_INBOX
                 * @reasoncode  INTERNAL_INVALID_INPUT_DATA
                 * @userdata1   N/A
                 * @userdata4   OCC_NO_EXTENDED_RC
                 * @devdesc     Firmware related internal error
                 */
                TRAC_ERR("Invalid address from calculate slave inbox address function [%08X]", l_addr_in_mem );
                l_orc = INTERNAL_INVALID_INPUT_DATA; // @wb003
                l_orc_ext = OCC_NO_EXTENDED_RC;     // @nh001a
                break;
            }
            break;   // @th002
        }
        else
        {
            if(l_bytes)
            {
               TRAC_INFO("Only got %d bytes from master",l_bytes);
            }
            // check time and break out if we reached limit
            if ((ssx_timebase_get() - l_start) < SSX_MICROSECONDS(100))    // @th002 -- TODO: shrink this down later
            {
                continue;
            }
            else
            {
#if 0
                /* @
                 * @errortype
                 * @moduleid    DCOM_MID_TASK_RX_SLV_INBOX
                 * @reasoncode  INTERNAL_FAILURE
                 * @userdata1   N/A
                 * @userdata4   ERC_GENERIC_TIMEOUT
                 * @devdesc     Generic timeout failure
                 */
                TRAC_ERR("Time out waiting for receive doorbell" );
                l_orc = INTERNAL_FAILURE;               // @nh001c
                l_orc_ext = ERC_GENERIC_TIMEOUT;        // @nh001a
#endif
                // TODO:  We need to signal this

                // <TULETA HW BRINGUP TIMING>
                G_dcomTime.slave.doorbellErrorFlags.timeoutRx = 1;
                G_dcomTime.slave.doorbellTimeoutWaitRx = ssx_timebase_get();
                // <TULETA HW BRINGUP TIMING>

                // Let's signal that master is not ready, and then
                // start task waiting for master to talk again.
                rtl_start_task(TASK_ID_DCOM_WAIT_4_MSTR);      // @th024
                rtl_set_run_mask_deferred(RTL_FLAG_MSTR_READY);
                TRAC_INFO("[%d]: Lost connection to master",(int) G_pob_id.chip_id);
                break;
            }
        }
    }
    while( 1 );    // @th002


    if ( l_orc != OCC_SUCCESS_REASON_CODE )
    {
        // create and commit error
        dcom_error_check( SLAVE_INBOX, FALSE, l_orc, l_orc_ext);    // @nh001c
    }
    else
    {
        // done, lets clear our counter
        dcom_error_check_reset( SLAVE_INBOX );
    }

}


// Function Specification
//
// Name: dcom_rx_slv_inbox_doorbell
//
// Description: receive multicast doorbell and save data
//              from master (master to slave)
//
// Flow:  09/02/11    FN=dcom_rx_slv_inbox_doorbell
//
// End Function Specification
uint32_t dcom_rx_slv_inbox_doorbell( void )
{
    static bool l_trace_once       = FALSE;
    int         l_pbarc            = 0;
    uint32_t    l_read             = 0;
    uint32_t    l_bytes_so_far     = 0;
    uint64_t    l_start            = ssx_timebase_get();

    G_dcomTime.slave.doorbellStartRx = l_start;

    while(l_bytes_so_far < sizeof(G_dcom_slv_inbox_doorbell_rx))
    {
        /// Read 8 bytes of the doorbell from master
        l_pbarc = pbax_read(
                &G_pbax_read_queue[0],
                &G_dcom_slv_inbox_doorbell_rx.words[(l_bytes_so_far / sizeof(uint64_t))],
                sizeof(uint64_t),   // Must read 8 bytes at a time
                &l_read
                );

        DCOM_DBG("Doorbell (Multicast) Read: %d bytes\n",l_read);

        /// We got an error reading from the PBAX, return to caller
        if ( l_pbarc != 0 )
        {
            G_dcomTime.slave.doorbellErrorFlags.hwError = 1;
            if ( FALSE == l_trace_once )
            {
                // failure occurred but only trace it once
                TRAC_ERR("PBAX Read Failure in receiving multicast doorbell - RC[%08X]", l_pbarc);
                l_trace_once  = TRUE;
            }
            break;
        }

        /// Didn't read any bytes from pbax.  We are either done, or we
        /// simply don't have any data to read
        if(0 == l_read){
            if ((ssx_timebase_get() - l_start) > SSX_MICROSECONDS(3))
            {
                // <TULETA HW BRINGUP TIMING>
                if(l_bytes_so_far){
                    G_dcomTime.slave.doorbellErrorFlags.incomplete = 1;
                }
                else{
                    G_dcomTime.slave.doorbellErrorFlags.timeout = 1;
                }
                // </TULETA HW BRINGUP TIMING>
                break;
            }
        }
        else{
            /// If we just read some data, reset this EndOfMessage counter
            l_start = ssx_timebase_get();
        }

        /// Increment the number of bytes we have in the buffer
        l_bytes_so_far += l_read;

        /// If it doesnt match the magic number, and it's the first
        /// packet we read, just drop it on the floor and start over
        if( 8 == l_bytes_so_far)
        {
            if(PBAX_MAGIC_NUMBER2_32B != G_dcom_slv_inbox_doorbell_rx.magic1)
            {
                l_read         = 0;
                l_bytes_so_far = 0;
                G_dcomTime.slave.doorbellErrorFlags.dropPacket = 1;

                TRAC_INFO("Slave Inbox - Start Magic Number Mismatch [0x%08X]",
                          G_dcom_slv_inbox_doorbell_rx.magic1);
            }
        }
        /// If this is the last packet, make sure the magic number matches
        else if (sizeof(G_dcom_slv_inbox_doorbell_rx) == l_bytes_so_far)
        {
           if(PBAX_MAGIC_NUMBER_32B != G_dcom_slv_inbox_doorbell_rx.magic2)
           {
               TRAC_INFO("Slave Inbox - End Magic Number Mismatch [0x%08X]",
                         G_dcom_slv_inbox_doorbell_rx.magic2);
               G_dcomTime.slave.doorbellErrorFlags.badMagicNumEnd = 1;

               /// Decrement the number of bytes we return so it fails
               /// any valid length checks, but still indicates to us that
               /// we 'got' data, just not good data if it is included in
               /// an error log.
               l_bytes_so_far = (sizeof(G_dcom_slv_inbox_doorbell_rx) - 1);
           }
           else
           {
               // <TULETA HW BRINGUP TIMING>
               if(G_dcom_slv_inbox_doorbell_rx.magic_counter != (G_dcomTime.slave.doorbellSeq + 1))
               {
                   G_dcomTime.slave.doorbellErrorFlags.badSequence = 1;
               }
               G_dcomTime.slave.doorbellSeq = G_dcom_slv_inbox_doorbell_rx.magic_counter;
               // </TULETA HW BRINGUP TIMING>
           }
           break;
        }
    }

    // <TULETA HW BRINGUP TIMING>
    G_dcomTime.slave.doorbellStopRx = ssx_timebase_get();
    uint64_t l_delta = G_dcomTime.slave.doorbellStopRx - G_dcomTime.slave.doorbellStartRx;
    G_dcomTime.slave.doorbellMaxDeltaRx = (l_delta > G_dcomTime.slave.doorbellMaxDeltaRx) ?
        l_delta : G_dcomTime.slave.doorbellMaxDeltaRx;
    // </TULETA HW BRINGUP TIMING>

    return l_bytes_so_far;
}  // @th034 -- rewrote most of function


// Function Specification
//
// Name: task_dcom_wait_for_master
//
// Description: The purpose of this task is to wait for the reception of the
//              first master multicast doorbell. After it is received, it sets
//              a RTL flag indicating as such.
//
// Flow:  09/27/11    FN=task_dcom_wait_for_master
//
// End Function Specification
#define POBID_RETRIES   2
void task_dcom_wait_for_master( task_t *i_self)
{
    // Don't try and read the doorbell on the first tick of the RTL.
    // If we do, then we will clear the RTL flags too early, and the actual
    // task will miss the doorbell.
    // This task will skip executing once every 12.41 days b/c of a rollover of
    // CURRENT_TICK.  This is not a concern, so we won't bother to check that case.
    int                 l_rc                    = 0;
    pbax_id_t           l_pbaxid;
    uint32_t            l_num_read              = 0;   // @th034
    static bool         L_first_doorbell_rcvd   = FALSE;
    static bool         L_queue_enabled         = FALSE;
    static uint32_t     L_pobid_retries_left    = POBID_RETRIES;

    DCOM_DBG("0. Wait for Master\n");


    do
    {
        /// If this is the first time we are in this task, enable
        /// the queue, since now until forever we are able to read from it.
        if(!L_queue_enabled)
        {
            pbax_queue_enable(&G_pbax_read_queue[0]);
            L_queue_enabled = TRUE;
        }

        l_num_read = (CURRENT_TICK) ? dcom_rx_slv_inbox_doorbell() : 0;   // @th034

        G_dcomSlvInboxCounter.totalTicks++;

        if(l_num_read < sizeof(G_dcom_slv_inbox_doorbell_rx))
        {
            // Don't log an error if we don't get doorbell within any certain timeout
            // period.  This will be taken care of by another function
            break;
        }

        // Special handling for recieving the 1st master doorbell since we need to also
        // set up a unicast PBAX target to the master -- gm037
        if(!L_first_doorbell_rcvd)
        {
            //convert powerbus id to pbax id (sets node_id to INVALID_NODE_ID on failure)
            l_pbaxid = dcom_pbusid2pbaxid(G_dcom_slv_inbox_doorbell_rx.pob_id); //traces failure internally
            if(l_pbaxid.node_id == INVALID_NODE_ID) 
            {
                //we received an invalid power bus id from the master.
                //This may be a communication failure, so allow some retries
                if(L_pobid_retries_left)
                {
                    L_pobid_retries_left--;
                    break;
                }

                //retries exceeded.  Log error and request reset.

                /* @
                 * @errortype
                 * @moduleid    DCOM_MID_WAIT_FOR_MASTER
                 * @reasoncode  INTERNAL_FAILURE
                 * @userdata1   0
                 * @userdata4   OCC_NO_EXTENDED_RC
                 * @devdesc     An invalid power bus ID was sent from the master OCC
                 */    
                errlHndl_t  l_errl = createErrl(
                    DCOM_MID_WAIT_FOR_MASTER,           //modId
                    INTERNAL_FAILURE,                   //reasoncode
                    OCC_NO_EXTENDED_RC,                 //Extended reason code 
                    ERRL_SEV_UNRECOVERABLE,             //Severity
                    NULL,                               //Trace Buf
                    DEFAULT_TRACE_SIZE,                 //Trace Size
                    0,                                  //userdata1
                    0                                   //userdata2
                    );

                // commit log and request reset
                REQUEST_RESET(l_errl);

                break;
            }

            //Set up the master pbax unicast target for sending doorbells to the master
            l_rc = pbax_target_create(&G_pbax_unicast_target,   //target
                                      PBAX_UNICAST,             //type
                                      PBAX_GROUP,               //scope
                                      1,                        //queue
                                      l_pbaxid.node_id,         //node
                                      l_pbaxid.chip_id);        //chip (or group) id
            if(l_rc)
            {
                //log an error and request reset when pbax_target_create fails 

                TRAC_ERR("pbax_target_create failed creating pbax target to master OCC. rc=%d, node=%d, chip=%d",
                         l_rc,
                         l_pbaxid.node_id,
                         l_pbaxid.chip_id);
                /* @
                 * @errortype
                 * @moduleid    DCOM_MID_WAIT_FOR_MASTER
                 * @reasoncode  SSX_GENERIC_FAILURE
                 * @userdata1   SSX RC 
                 * @userdata4   OCC_NO_EXTENDED_RC
                 * @devdesc     Failure initializing a PBAX queue 
                 */    
                errlHndl_t  l_errl = createErrl(
                    DCOM_MID_WAIT_FOR_MASTER,           //modId
                    SSX_GENERIC_FAILURE,                //reasoncode    // @nh001c
                    OCC_NO_EXTENDED_RC,                 //Extended reason code 
                    ERRL_SEV_UNRECOVERABLE,             //Severity
                    NULL,                               //Trace Buf
                    DEFAULT_TRACE_SIZE,                 //Trace Size
                    l_rc,                               //userdata1
                    0                                   //userdata2
                    );

                // commit log and request reset -- @gm007
                REQUEST_RESET(l_errl);

                break;
            }

            TRAC_IMP("Slave OCC[%d] Received first doorbell from Master OCC[%d]",
                     (int) G_pob_id.chip_id,
                     G_dcom_slv_inbox_doorbell_rx.pob_id.chip_id);
    
            // First message is dropped, so mark it as counted.
            G_dcomSlvInboxCounter.totalTicks       = 0;
            G_dcomSlvInboxCounter.totalSuccessful  = 0;
            G_dcomSlvInboxCounter.currentFailCount = 0;

            // initialization was successful
            L_first_doorbell_rcvd = TRUE;
    
        }
        else
        {
             TRAC_INFO("[%d] Restablished contact via doorbell from Master",(int) G_pob_id.chip_id);   // @th002
        }

        //got a multicast doorbell
        G_dcomTime.slave.doorbellNumWaitRx++;              // @th032
    
        //set MASTER READY on global RTL FLAG
        rtl_clr_run_mask_deferred(RTL_FLAG_MSTR_READY);    // @th032
    
        //CLEAR the RTL run FLAG for current task
        rtl_stop_task(TASK_ID_DCOM_WAIT_4_MSTR);   // @th024

    }while(0);

}


#endif //_DCOMSLAVERX_C

