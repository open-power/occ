/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/dcom/dcomSlaveTx.c $                                  */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2014                        */
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

#ifndef _DCOMSLAVETX_C
#define _DCOMSLAVETX_C

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

// SSX Block Copy Request for the Slave Outbox Transmit Queue
BceRequest G_slv_outbox_tx_pba_request;

// SSX PBAX Request for Unicast PBAX Queue (Slave Doorbell to Master)
PbaxRequest G_pbax_unicast_request;

// Used by the slave to house the doorbell data that is sent in
// the slave unicast doorbell, stating that it put slave outbox in main memory.
dcom_slv_outbox_doorbell_t G_dcom_slv_outbox_doorbell_tx;

// Make sure that the Slave Outbox TX Buffer is 1kB, otherwise cause
// error on the compile.
STATIC_ASSERT(  (NUM_BYTES_IN_SLAVE_OUTBOX != (sizeof(G_dcom_slv_outbox_tx)))  );
uint32_t G_slave_pbax_rc = 0;

// Function Specification
//
// Name: dcom_build_slv_outbox
//
// Description: The purpose of this function is to fill out the Sensor Data into the
//              Slave Outbox Structures for transfer to the Master.
//              Build the slave outboxes so slave can send to master
//
// End Function Specification

uint32_t dcom_build_slv_outbox(void)
{
    // Locals
    uint32_t l_addr_of_slv_outbox_in_main_mem = 0;

    static uint8_t      L_seq = 0xFF;

    L_seq++;

    G_dcom_slv_outbox_tx.seq = L_seq;
    G_dcom_slv_outbox_tx.version = 0;

    memset( G_dcom_slv_outbox_tx.occ_fw_mailbox, 0, sizeof( G_dcom_slv_outbox_tx.occ_fw_mailbox ));

    G_dcom_slv_outbox_tx.counter++;

    // Call dcom_build_occfw_msg
    dcom_build_occfw_msg( SLAVE_OUTBOX );

    // Create message that will be sent to DCM peer
    dcom_build_dcm_sync_msg( SLAVE_OUTBOX );

    l_addr_of_slv_outbox_in_main_mem = dcom_which_buffer_slv_outbox();

    l_addr_of_slv_outbox_in_main_mem += G_pob_id.chip_id*sizeof(dcom_slv_outbox_t);

    G_dcom_slv_outbox_doorbell_tx.pob_id = G_pob_id;
    G_dcom_slv_outbox_doorbell_tx.pcap_valid = g_amec->pcap_valid;
    G_dcom_slv_outbox_doorbell_tx.active_node_pcap = g_amec->pcap.active_node_pcap;
    G_dcom_slv_outbox_doorbell_tx.addr_slv_outbox_buffer = l_addr_of_slv_outbox_in_main_mem;


    return l_addr_of_slv_outbox_in_main_mem;
}


// Function Specification
//
// Name:  dcom_which_buffer_slv_outbox
//
// Description: Determines which buffer in the 'double buffer'
//              or ping/pong to use. Basically alternates between
//              returning the ping or the pong address
//
// End Function Specification

uint32_t dcom_which_buffer_slv_outbox(void)
{
    // Locals
    uint32_t l_mem_address = ADDR_SLAVE_OUTBOX_MAIN_MEM_PONG;

    // Switch back and forth based on tick
    if( CURRENT_TICK & 1 )
    {
        l_mem_address = ADDR_SLAVE_OUTBOX_MAIN_MEM_PING;
    }

    return l_mem_address;
}


// Function Specification
//
// Name: dcom_calc_slv_outbox_addr
//
// Description: get slave outbox main memory address
//
// End Function Specification
uint32_t dcom_calc_slv_outbox_addr( const dcom_slv_outbox_doorbell_t * i_doorbell, uint8_t * o_occ_id  )
{
    *o_occ_id = i_doorbell->pob_id.chip_id;
    return i_doorbell->addr_slv_outbox_buffer;
}


// Function Specification
//
// Name: task_dcom_tx_slv_outbox
//
// Description: Copy slave outboxes from SRAM to main memory
//              so slave can send data to master
//
// Task Flags:  RTL_FLAG_NONMSTR, RTL_FLAG_MSTR, RTL_FLAG_OBS, RTL_FLAG_ACTIVE,
//              RTL_FLAG_NOAPSS, RTL_FLAG_RUN, RTL_FLAG_MSTR_READY
//
// End Function Specification
void task_dcom_tx_slv_outbox( task_t *i_self)
{
    static bool l_error = FALSE;
    uint32_t    l_orc = OCC_SUCCESS_REASON_CODE;
    uint32_t    l_orc_ext = OCC_NO_EXTENDED_RC;
    // Use a static local bool to track whether the BCE request used
    // here has ever been successfully created at least once
    static bool L_bce_slv_outbox_tx_request_created_once = FALSE;

    DCOM_DBG("3. TX Slave Outboxes\n");

    do
    {
        // If there was a pbax_send failure, trace it here since we can't do it in the critical
        // interrupt context.
        if(G_slave_pbax_rc)
        {
           TRAC_ERR("task_dcom_tx_slv_outbox: PBAX Send Failure in transimitting doorbell - RC[%08X]", G_slave_pbax_rc);
        }

        // Build/setup outbox
        uint32_t l_addr_in_mem = dcom_build_slv_outbox();
        uint32_t l_ssxrc = 0;

        // See dcomMasterRx.c/task_dcom_rx_slv_outboxes for details on the
        // checking done here before creating and scheduling the request.
        bool l_proceed_with_request_and_schedule = FALSE;
        int l_req_idle = async_request_is_idle(&(G_slv_outbox_tx_pba_request.request));
        int l_req_complete = async_request_completed(&(G_slv_outbox_tx_pba_request.request));

        if (!L_bce_slv_outbox_tx_request_created_once)
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
            TRAC_INFO("BCE slv outbox tx request idle but not complete, \
                      callback_rc=%d options=0x%x state=0x%x abort_state=0x%x \
                      completion_state=0x%x",
                      G_slv_outbox_tx_pba_request.request.callback_rc,
                      G_slv_outbox_tx_pba_request.request.options,
                      G_slv_outbox_tx_pba_request.request.state,
                      G_slv_outbox_tx_pba_request.request.abort_state,
                      G_slv_outbox_tx_pba_request.request.completion_state);
            TRAC_INFO("Proceeding with BCE slv outbox tx request and schedule");
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
            TRAC_INFO("BCE slv outbox tx request not idle and not complete, \
                      callback_rc=%d options=0x%x state=0x%x abort_state=0x%x \
                      completion_state=0x%x",
                      G_slv_outbox_tx_pba_request.request.callback_rc,
                      G_slv_outbox_tx_pba_request.request.options,
                      G_slv_outbox_tx_pba_request.request.state,
                      G_slv_outbox_tx_pba_request.request.abort_state,
                      G_slv_outbox_tx_pba_request.request.completion_state);
            TRAC_INFO("NOT proceeding with BCE slv outbox tx request and schedule");
        }
        else
        {
            // This case can't happen, ignore it.
        }

        // Only proceed if the BCE request state checked out
        if (l_proceed_with_request_and_schedule)
        {
            // set up outbox copy request
            l_ssxrc = bce_request_create(
                            &G_slv_outbox_tx_pba_request,       // Block copy object
                            &G_pba_bcue_queue,                  // Mainstore to sram copy engine
                            l_addr_in_mem,                      // Mainstore address
                            (uint32_t) &G_dcom_slv_outbox_tx,   // SRAM starting address
                            sizeof(G_dcom_slv_outbox_tx),       // Size of copy
                            SSX_WAIT_FOREVER,                   // No timeout
                            (AsyncRequestCallback)dcom_tx_slv_outbox_doorbell, // Call back
                            NULL,                               // Call back arguments
                            ASYNC_CALLBACK_IMMEDIATE            // Callback mask
                            );

            if(l_ssxrc != SSX_OK)
            {
                /* @
                 * @errortype
                 * @moduleid    DCOM_MID_TASK_TX_SLV_OUTBOX
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
            L_bce_slv_outbox_tx_request_created_once = TRUE;
            l_ssxrc = bce_request_schedule(&G_slv_outbox_tx_pba_request);   // Actual copying

            if(l_ssxrc != SSX_OK)
            {
                /* @
                 * @errortype
                 * @moduleid    DCOM_MID_TASK_TX_SLV_OUTBOX
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
        }

    } while (0);

    if ( l_orc != OCC_SUCCESS_REASON_CODE && l_error == FALSE)
    {
        // Create and commit error
        // See return code doxygen tags for error description
        errlHndl_t  l_errl = createErrl(
                    DCOM_MID_TASK_TX_SLV_OUTBOX,    //ModId
                    l_orc,                          //Reasoncode
                    l_orc_ext,                      //Extended reasoncode
                    ERRL_SEV_UNRECOVERABLE,         //Severity
                    NULL,                           //Trace Buf
                    DEFAULT_TRACE_SIZE,             //Trace Size
                    0,                              //Userdata1
                    0                               //Userdata2
                    );

        // Callout firmware
        addCalloutToErrl(l_errl,
                         ERRL_CALLOUT_TYPE_COMPONENT_ID,
                         ERRL_COMPONENT_ID_FIRMWARE,
                         ERRL_CALLOUT_PRIORITY_HIGH);

        commitErrl( &l_errl );

        // TODO request a reset
        l_error = TRUE;
    }

}

// Function Specification
//
// Name: dcom_tx_slv_outbox_doorbell
//
// Description: transmit unicast doorbell to master
//              from slaves
//
// End Function Specification
void dcom_tx_slv_outbox_doorbell( void )
{
    int         l_pbarc = 0;
    uint64_t    l_tmp =0;

    // Save into temp
    memcpy( &l_tmp, &G_dcom_slv_outbox_doorbell_tx, sizeof(dcom_slv_outbox_doorbell_t));

    // Send unicast doorbell
    l_pbarc = _pbax_send(
                &G_pbax_unicast_target,
                l_tmp,
                SSX_MICROSECONDS(15));

    // Set this global so we know to trace this in the non-critical interrupt context
    G_slave_pbax_rc = l_pbarc;
}

#endif //_DCOMSLAVETOMASTER_C

