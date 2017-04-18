/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/dcom/dcomSlaveRx.c $                                  */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2015                        */
/* [+] Google Inc.                                                        */
/* [+] International Business Machines Corp.                              */
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

#ifndef _DCOMSLAVERX_C
#define _DCOMSLAVERX_C

#include <pgp_pmc.h>
#include "pgp_pba.h"
#include <rtls.h>
#include <apss.h>
#include <dcom.h>
#include <dcom_service_codes.h>
#include <occ_service_codes.h>
#include <trac.h>
#include <proc_pstate.h>
#include <amec_data.h>

// SSX Block Copy Request for the Slave Inbox Receive Queue
BceRequest G_slv_inbox_rx_pba_request;

// SSX PBAX Request for Multicast PBAX Queue (Master Doorbell to Slaves)
PbaxRequest G_pbax_multicast_request;

// SSX PBAX Request for Receiving PBAX Messages
PbaxRequest G_pbax_rx_request;

// Used by the slave to house the doorbell data that is received from
// the master multicast doorbell, stating that it put slave inbox in main memory.
dcom_slv_inbox_doorbell_t G_dcom_slv_inbox_doorbell_rx;

// Make sure that the Slave Inbox RX Buffer is 256B, otherwise cause
// error on the compile.
STATIC_ASSERT(  (NUM_BYTES_IN_SLAVE_INBOX != (sizeof(G_dcom_slv_inbox_rx)))  );

// Indicate if we need to lower the Pmax_rail
bool     G_apss_lower_pmax_rail = FALSE;

// Function Specification
//
// Name: dcom_calc_slv_inbox_addr
//
// Description: get slave inbox main memory address
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
// Task Flags: RTL_FLAG_OBS, RTL_FLAG_ACTIVE
//
// End Function Specification
void task_dcom_rx_slv_inbox( task_t *i_self)
{
    uint32_t    l_orc = OCC_SUCCESS_REASON_CODE;
    uint32_t    l_orc_ext = OCC_NO_EXTENDED_RC;
    uint64_t    l_start = ssx_timebase_get();
    uint32_t    l_bytes = 0;
    // Use a static local bool to track whether the BCE request used
    // here has ever been successfully created at least once
    static bool L_bce_slv_inbox_rx_request_created_once = FALSE;

    DCOM_DBG("1. RX Slave Inbox\n");

    // Increment debug counter
    G_dcomSlvInboxCounter.totalTicks++;
    G_dcomTime.slave.doorbellStartWaitRx = l_start;

    do
    {
        // Doorbell from the master OCC
        l_bytes =  dcom_rx_slv_inbox_doorbell();

        if(l_bytes >= sizeof(G_dcom_slv_inbox_doorbell_rx))
        {
            // Looks like we got a valid doorbell so notify slave
            // code of pcap info
            amec_data_write_pcap();

#ifdef DCOM_DEBUG
            uint64_t l_end = ssx_timebase_get();
            DCOM_DBG("Got Doorbell from Master after waiting %d us\n",(int)( (l_end-l_start) / ( SSX_TIMEBASE_FREQUENCY_HZ / 1000000 ) ));
#endif
            G_dcomTime.slave.doorbellStopWaitRx = ssx_timebase_get();
            uint64_t l_delta = G_dcomTime.slave.doorbellStopWaitRx - G_dcomTime.slave.doorbellStartWaitRx;
            G_dcomTime.slave.doorbellMaxDeltaWaitRx = (l_delta > G_dcomTime.slave.doorbellMaxDeltaWaitRx) ?
                l_delta : G_dcomTime.slave.doorbellMaxDeltaWaitRx;
            G_dcomTime.slave.doorbellNumRx++;

            // Increment debug counter
            G_dcomSlvInboxCounter.totalSuccessful++;

            // Main memory address
            uint32_t l_addr_in_mem = dcom_calc_slv_inbox_addr();

            // Check valid address (should be inside inbox addresses range)
            if ( (ADDR_SLAVE_INBOX_MAIN_MEM_PING <= l_addr_in_mem) &&
                 ((ADDR_SLAVE_INBOX_MAIN_MEM_PONG+(sizeof(dcom_slv_inbox_t)*MAX_OCCS)) > l_addr_in_mem) )
            {
                uint32_t l_ssxrc = 0;

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
                else
                {
                    // This case can't happen, ignore it.
                }

                // Only proceed if the BCE request state checked out
                if (l_proceed_with_request_and_schedule)
                {
                    // Copy request from main memory to SRAM
                    l_ssxrc = bce_request_create(
                                    &G_slv_inbox_rx_pba_request,        // Block copy object
                                    &G_pba_bcde_queue,                  // Mainstore to sram copy engine
                                    l_addr_in_mem,                      // Mainstore address
                                    (uint32_t)&G_dcom_slv_inbox_rx,     // SRAM starting address
                                    sizeof(G_dcom_slv_inbox_rx),        // Size of copy
                                    SSX_WAIT_FOREVER,                   // No timeout
                                    (AsyncRequestCallback)dcom_rx_slv_inbox_callback, // Call back
                                    NULL,                               // Call back arguments
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
                        l_orc = SSX_GENERIC_FAILURE;
                        l_orc_ext = ERC_BCE_REQUEST_CREATE_FAILURE;
                        break;
                    }

                    // Request created at least once
                    L_bce_slv_inbox_rx_request_created_once = TRUE;
                    l_ssxrc = bce_request_schedule(&G_slv_inbox_rx_pba_request); // Actual copying

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
                        l_orc = SSX_GENERIC_FAILURE;
                        l_orc_ext = ERC_BCE_REQUEST_SCHEDULE_FAILURE;
                        break;
                    }
                    break;
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
                l_orc = INTERNAL_INVALID_INPUT_DATA;
                l_orc_ext = OCC_NO_EXTENDED_RC;
                break;
            }
            break;
        }
        else
        {
            if(l_bytes)
            {
               TRAC_INFO("Only got %d bytes from master",l_bytes);
            }
            // Check time and break out if we reached limit
            if ((ssx_timebase_get() - l_start) < SSX_MICROSECONDS(100))    // TODO: Shrink this down
            {
                continue;
            }
            else
            {
                // TODO:  We need to signal this

                G_dcomTime.slave.doorbellErrorFlags.timeoutRx = 1;
                G_dcomTime.slave.doorbellTimeoutWaitRx = ssx_timebase_get();

                // Let's signal that master is not ready, and then
                // start task waiting for master to talk again.
                rtl_start_task(TASK_ID_DCOM_WAIT_4_MSTR);
                rtl_set_run_mask_deferred(RTL_FLAG_MSTR_READY);
                TRAC_INFO("[%d]: Lost connection to master",(int) G_pob_id.chip_id);
                break;
            }
        }
    }
    while( 1 );

    if ( l_orc != OCC_SUCCESS_REASON_CODE )
    {
        // Create and commit error
        dcom_error_check( SLAVE_INBOX, FALSE, l_orc, l_orc_ext);
    }
    else
    {
        // Done, lets clear our counter
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
// End Function Specification
uint32_t dcom_rx_slv_inbox_doorbell( void )
{
    int           l_pbarc            = 0;
    uint32_t      l_read             = 0;
    uint32_t      l_bytes_so_far     = 0;
    uint64_t      l_start            = ssx_timebase_get();
    pba_xshcsn_t  l_pba_shcs0;
    uint8_t       l_num_queue_entires = 0;
    uint8_t       l_write_ptr = 0;
    uint8_t       l_read_ptr = 0;
    uint8_t       l_queue_full = 0;
    bool          l_read_2_doorbells = FALSE;


    G_dcomTime.slave.doorbellStartRx = l_start;

    while(l_bytes_so_far < sizeof(G_dcom_slv_inbox_doorbell_rx))
    {
        // Read 8 bytes of the doorbell from master
        l_pbarc = pbax_read(
                &G_pbax_read_queue[0],
                &G_dcom_slv_inbox_doorbell_rx.words[(l_bytes_so_far / sizeof(uint64_t))],
                sizeof(uint64_t), // Must read 8 bytes at a time
                &l_read
                );

        DCOM_DBG("Doorbell (Multicast) Read: %d bytes\n",l_read);

        // We got an error reading from the PBAX, return to caller
        if ( l_pbarc != 0 )
        {
            G_dcomTime.slave.doorbellErrorFlags.hwError = 1;
            // Failure occurred
            TRAC_ERR("Slave PBAX Read Failure in receiving doorbell from master - RC[%08X]", l_pbarc);

            // Handle pbax read failure on queue 0
            dcom_pbax_error_handler(0);
            break;
        }

        // Didn't read any bytes from pbax.  We are either done, or we
        // simply don't have any data to read
        if(0 == l_read)
        {
            if ((ssx_timebase_get() - l_start) > SSX_MICROSECONDS(3))
            {
                if(l_bytes_so_far){
                    G_dcomTime.slave.doorbellErrorFlags.incomplete = 1;
                }
                else{
                    G_dcomTime.slave.doorbellErrorFlags.timeout = 1;
                }
                break;
            }
        }
        else{
            // If we just read some data, reset this EndOfMessage counter
            l_start = ssx_timebase_get();
        }

        // Increment the number of bytes we have in the buffer
        l_bytes_so_far += l_read;

        // If it doesnt match the magic number, and it's the first
        // packet we read, just drop it on the floor and start over
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
        // If this is the last packet, make sure the magic number matches
        else if (sizeof(G_dcom_slv_inbox_doorbell_rx) == l_bytes_so_far)
        {
           if(PBAX_MAGIC_NUMBER_32B != G_dcom_slv_inbox_doorbell_rx.magic2)
           {
               TRAC_INFO("Slave Inbox - End Magic Number Mismatch [0x%08X]",
                         G_dcom_slv_inbox_doorbell_rx.magic2);
               G_dcomTime.slave.doorbellErrorFlags.badMagicNumEnd = 1;

               // Decrement the number of bytes we return so it fails
               // any valid length checks, but still indicates to us that
               // we 'got' data, just not good data if it is included in
               // an error log.
               l_bytes_so_far = (sizeof(G_dcom_slv_inbox_doorbell_rx) - 1);
           }
           else
           {
               if(G_dcom_slv_inbox_doorbell_rx.magic_counter != (G_dcomTime.slave.doorbellSeq + 1))
               {
                   G_dcomTime.slave.doorbellErrorFlags.badSequence = 1;
               }
               G_dcomTime.slave.doorbellSeq = G_dcom_slv_inbox_doorbell_rx.magic_counter;
           }

           // if there is another full doorbell on the queue read that out to empty the queue
           // and use that data as it is newer
           l_pba_shcs0.value = in64(PBA_XSHCS0);
           l_queue_full = l_pba_shcs0.fields.push_full;
           l_write_ptr = (uint8_t)(l_pba_shcs0.fields.push_write_ptr);
           l_read_ptr = (uint8_t)(l_pba_shcs0.fields.push_read_ptr);

           // pointer is for an 8 byte queue entry, doorbell size is 128B == 16 queue entries
           // when pointers equal each other that can be either empty or full so we need to
           // check the full bit to handle that case
           if(l_read_ptr > l_write_ptr)
           {
              l_num_queue_entires = NUM_ENTRIES_PBAX_QUEUE0 - (l_read_ptr - l_write_ptr);
           }
           else
           {
              l_num_queue_entires = l_write_ptr - l_read_ptr;
           } 
           if(l_queue_full || (l_num_queue_entires >= 16))
           {
              TRAC_IMP("dcom_rx_slv_inbox_doorbell: 2 doorbells PBA_XSHCS0[0x%08x]: full = %d 8B entries = %d read = %d write = %d",
                        l_pba_shcs0.words.high_order,
                        l_queue_full,
                        l_num_queue_entires,
                        l_read_ptr,
                        l_write_ptr);
              if(l_read_2_doorbells)
              {
                 // this shoud never happen but not going to read a 3rd to prevent infinite loop
                 // we will either drain on next tick or overflow and recover from overflow
                 TRAC_ERR("dcom_rx_slv_inbox_doorbell:  3rd full doorbell present!!! PBA_XSHCS0[0x%08x]",
                          l_pba_shcs0.words.high_order);
                 break;
              }
              // set up to start reading the 2nd doorbell
              l_read_2_doorbells = TRUE;
              l_read = 0;
              l_bytes_so_far = 0;
              l_start = ssx_timebase_get();
           }
           else
           {
             break;
           }
        }
    }

    G_dcomTime.slave.doorbellStopRx = ssx_timebase_get();
    uint64_t l_delta = G_dcomTime.slave.doorbellStopRx - G_dcomTime.slave.doorbellStartRx;
    G_dcomTime.slave.doorbellMaxDeltaRx = (l_delta > G_dcomTime.slave.doorbellMaxDeltaRx) ?
        l_delta : G_dcomTime.slave.doorbellMaxDeltaRx;

    return l_bytes_so_far;
}

// Function Specification
//
// Name: task_dcom_wait_for_master
//
// Description: The purpose of this task is to wait for the reception of the
//              first master multicast doorbell. After it is received, it sets
//              a RTL flag indicating as such.
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
    uint32_t            l_num_read              = 0;
    static bool         L_first_doorbell_rcvd   = FALSE;
    static bool         L_queue_enabled         = FALSE;
    static uint32_t     L_pobid_retries_left    = POBID_RETRIES;
    static uint16_t     L_no_master_doorbell_cnt = 0;
    static uint16_t     L_trace_every_count = 1;

    DCOM_DBG("0. Wait for Master\n");

    do
    {
        // If this is the first time we are in this task, enable
        // the queue, since now until forever we are able to read from it.
        if(!L_queue_enabled)
        {
            pbax_queue_enable(&G_pbax_read_queue[0]);
            L_queue_enabled = TRUE;
        }

        l_num_read = (CURRENT_TICK) ? dcom_rx_slv_inbox_doorbell() : 0;

        G_dcomSlvInboxCounter.totalTicks++;

        if(l_num_read < sizeof(G_dcom_slv_inbox_doorbell_rx))
        {
            if (L_first_doorbell_rcvd)
            {
                // We didn't get a doorbell from the Master, increment our
                // counter
                L_no_master_doorbell_cnt++;

                if (L_no_master_doorbell_cnt % L_trace_every_count == 0)
                {
                    TRAC_INFO("task_dcom_wait_for_master: experiencing data collection problems! fail_count=%i",
                              L_no_master_doorbell_cnt);
                }

                if (L_no_master_doorbell_cnt == APSS_DATA_FAIL_PMAX_RAIL)
                {
                    L_trace_every_count = 50;
                }

                if (L_no_master_doorbell_cnt == APSS_DATA_FAIL_MAX)
                {
                    // If we still don't get a doorbell from the Master for this
                    // long, we will request a reset
                    TRAC_ERR("Detected a problem with slave data collection: hard time-out[%d]. Requesting a reset!",
                             APSS_DATA_FAIL_MAX);

                    /* @
                     * @errortype
                     * @moduleid    DCOM_MID_TASK_WAIT_FOR_MASTER
                     * @reasoncode  APSS_SLV_LONG_TIMEOUT
                     * @userdata1   Time-out value
                     * @userdata4   OCC_NO_EXTENDED_RC
                     * @devdesc     Detected a problem with APSS data collection (long time-out)
                     */
                    errlHndl_t  l_errl = createErrl(
                                    DCOM_MID_TASK_WAIT_FOR_MASTER,  //modId
                                    APSS_SLV_LONG_TIMEOUT,          //reasoncode
                                    OCC_NO_EXTENDED_RC,             //Extended reason code
                                    ERRL_SEV_PREDICTIVE,            //Severity
                                    NULL,                           //Trace Buf
                                    DEFAULT_TRACE_SIZE,             //Trace Size
                                    APSS_DATA_FAIL_MAX,             //userdata1
                                    0                               //userdata2
                                    );

                    // Callout to firmware
                    addCalloutToErrl(l_errl,
                                     ERRL_CALLOUT_TYPE_COMPONENT_ID,
                                     ERRL_COMPONENT_ID_FIRMWARE,
                                     ERRL_CALLOUT_PRIORITY_MED);

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

                    REQUEST_RESET(l_errl);
                }
            }

            break;
        }

        // Special handling for recieving the 1st master doorbell since we need to also
        // set up a unicast PBAX target to the master.
        if(!L_first_doorbell_rcvd)
        {
            // Convert powerbus id to pbax id (sets node_id to INVALID_NODE_ID on failure)
            l_pbaxid = dcom_pbusid2pbaxid(G_dcom_slv_inbox_doorbell_rx.pob_id); // Traces failure internally
            if(l_pbaxid.node_id == INVALID_NODE_ID)
            {
                // We received an invalid power bus id from the master.
                // This may be a communication failure, so allow some retries.
                if(L_pobid_retries_left)
                {
                    L_pobid_retries_left--;
                    break;
                }

                // Retries exceeded.  Log error and request reset.

                /* @
                 * @errortype
                 * @moduleid    DCOM_MID_WAIT_FOR_MASTER
                 * @reasoncode  INTERNAL_FAILURE
                 * @userdata1   0
                 * @userdata4   OCC_NO_EXTENDED_RC
                 * @devdesc     An invalid power bus ID was sent from the master OCC
                 */
                errlHndl_t  l_errl = createErrl(
                    DCOM_MID_WAIT_FOR_MASTER,           //ModId
                    INTERNAL_FAILURE,                   //Reasoncode
                    OCC_NO_EXTENDED_RC,                 //Extended reasoncode
                    ERRL_SEV_UNRECOVERABLE,             //Severity
                    NULL,                               //Trace Buf
                    DEFAULT_TRACE_SIZE,                 //Trace Size
                    0,                                  //Userdata1
                    0                                   //Userdata2
                    );

                // Commit log and request reset
                REQUEST_RESET(l_errl);

                break;
            }

            // Set up the master pbax unicast target for sending doorbells to the master
            l_rc = pbax_target_create(&G_pbax_unicast_target,   //target
                                      PBAX_UNICAST,             //type
                                      PBAX_GROUP,               //scope
                                      1,                        //queue
                                      l_pbaxid.node_id,         //node
                                      l_pbaxid.chip_id);        //chip (or group) id
            if(l_rc)
            {
                // Log an error and request reset when pbax_target_create fails

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
                    DCOM_MID_WAIT_FOR_MASTER,           //ModId
                    SSX_GENERIC_FAILURE,                //Reasoncode
                    OCC_NO_EXTENDED_RC,                 //Extended reasoncode
                    ERRL_SEV_UNRECOVERABLE,             //Severity
                    NULL,                               //Trace Buf
                    DEFAULT_TRACE_SIZE,                 //Trace Size
                    l_rc,                               //Userdata1
                    0                                   //Userdata2
                    );

                // Commit log and request reset
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

            // Initialization was successful
            L_first_doorbell_rcvd = TRUE;

        }
        else
        {
             TRAC_INFO("[%d] Restablished contact via doorbell from Master (after %d ticks)",(int) G_pob_id.chip_id, L_no_master_doorbell_cnt);

             // reset the no_master_doorbell counter
             L_no_master_doorbell_cnt = 0;
             L_trace_every_count = 1;
        }

        // Got a multicast doorbell
        G_dcomTime.slave.doorbellNumWaitRx++;

        // Set MASTER READY on global RTL FLAG
        rtl_clr_run_mask_deferred(RTL_FLAG_MSTR_READY);

        // CLEAR the RTL run FLAG for current task
        rtl_stop_task(TASK_ID_DCOM_WAIT_4_MSTR);

    }while(0);

}

#endif //_DCOMSLAVERX_C

