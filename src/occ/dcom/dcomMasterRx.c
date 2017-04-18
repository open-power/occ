/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/dcom/dcomMasterRx.c $                                 */
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

#ifndef _DCOMMASTERRX_C
#define _DCOMMASTERRX_C

#include <pgp_pmc.h>
#include "pgp_pba.h"
#include <rtls.h>
#include <apss.h>
#include <dcom.h>
#include <dcom_service_codes.h>
#include <occ_service_codes.h>
#include <trac.h>
#include <proc_pstate.h>
#include <amec_master_smh.h>

uint8_t G_slv_outbox_complete = 0;

// SSX Block Copy Request for the Slave Outbox Receive Queue
BceRequest G_slv_outbox_rx_pba_request[MAX_OCCS];

// Used by the master to house the doorbell data that is received from
// the slave unicast doorbells from each slave, stating that it put its slave
// outbox in main memory.
dcom_slv_outbox_doorbell_t G_dcom_slv_outbox_doorbell_rx[NUM_ENTRIES_PBAX_QUEUE1];

// Make sure that the Slave Outbox RX Buffer is 1kB / OCC, otherwise cause
// error on the compile.
STATIC_ASSERT(  (NUM_BYTES_IN_SLAVE_OUTBOX != (sizeof(G_dcom_slv_outbox_rx)/MAX_OCCS))  );

// Function Specification
//
// Name: setbit_slvoutbox_complete
//
// Description: Helper function to set bit when outbox is complete
//
// End Function Specification
void setbit_slvoutbox_complete(uint8_t i_bit)
{
    if(i_bit < MAX_NUM_OCC)
    {
        G_slv_outbox_complete |= (1 << i_bit);
    }
}


// Function Specification
//
// Name: task_dcom_rx_slv_outboxes
//
// Description: Copy Slave outboxes from Main Memory to SRAM
//              on master
//
// Task Flags:  RTL_FLAG_OBS, RTL_FLAG_ACTIVE, RTL_FLAG_MSTR,
//              RTL_FLAG_NOAPSS, RTL_FLAG_RUN
//
// End Function Specification
void task_dcom_rx_slv_outboxes( task_t *i_self)
{
    static uint32_t  L_wait4slaves = 0;
    uint32_t        l_orc = OCC_SUCCESS_REASON_CODE;
    uint32_t        l_orc_ext = OCC_NO_EXTENDED_RC;
    uint8_t         l_slv_response_mask = 0;
    uint8_t         l_slv = 0;
    uint32_t        l_num_doorbells_rxd = 0;

    // Use a static local bool to track whether the BCE request used
    // here has ever been successfully created at least once
    static bool     L_bce_slv_outbox_rx_request_created_once[MAX_OCCS] = {FALSE,};

    DCOM_DBG("2. RX Slave Outboxes\n");

    do
    {
        // Doorbell from the slave
        l_num_doorbells_rxd = dcom_rx_slv_outbox_doorbell();

        // How many doorbells were received?
        if( l_num_doorbells_rxd < G_occ_num_present )
        {
            if ( L_wait4slaves > MAX_WAIT_FOR_SLAVES )
            {
                /* @
                 * @errortype
                 * @moduleid    DCOM_MID_TASK_RX_SLV_OUTBOX
                 * @reasoncode  INTERNAL_FAILURE
                 * @userdata1   N/A
                 * @userdata4   ERC_GENERIC_TIMEOUT
                 * @devdesc     Generic timeout failure
                 */
                TRAC_ERR("Time out waiting for slaves" );
                l_orc = INTERNAL_FAILURE;
                l_orc_ext = ERC_GENERIC_TIMEOUT;
                break;
            }

            L_wait4slaves++;
        }

        // if we got more than 1 doorbell from a slave only use the newest for that slave
        // Do this first and then make BCE schedule requests

        uint32_t l_slv_idx = 0;
        uint32_t l_slv_main_mem_addr[MAX_OCCS] = {0};

        // Loop through all doorbells received
        for(; l_slv_idx < l_num_doorbells_rxd; l_slv_idx++)
        {
            // Index/occ id
            l_slv = 0;

            // Main memory address
            uint32_t l_addr = dcom_calc_slv_outbox_addr( &G_dcom_slv_outbox_doorbell_rx[l_slv_idx], &l_slv);
            l_slv_main_mem_addr[l_slv] = l_addr;
            G_slave_active_pcaps[l_slv].active_pcap = G_dcom_slv_outbox_doorbell_rx[l_slv_idx].active_node_pcap;
            G_slave_active_pcaps[l_slv].pcap_valid = G_dcom_slv_outbox_doorbell_rx[l_slv_idx].pcap_valid;

            // Add slave to mask of responding slaves
            l_slv_response_mask |= (0x01 << l_slv);
        }

        // Loop thru all slaves to copy down outbox from main memory
        for(l_slv = 0; l_slv < MAX_OCCS; l_slv++)
        {
           if(l_slv_main_mem_addr[l_slv])
           {
              // Check valid address (should be inside inbox addresses range)
              if ( (ADDR_SLAVE_OUTBOX_MAIN_MEM_PING <= l_slv_main_mem_addr[l_slv]) &&
                 ((ADDR_SLAVE_OUTBOX_MAIN_MEM_PONG+(sizeof(dcom_slv_outbox_t)*MAX_OCCS)) >
                      l_slv_main_mem_addr[l_slv]) )
              {
                  DCOM_DBG("2.X. Copy down Slave Outboxes from %x\n",l_slv_main_mem_addr[l_slv]);
                  uint32_t l_ssxrc = 0;

                  // Using a global bce request requires some special consideration
                  // of the possible request states.  Note that since this task
                  // runs in the critical section of the RTL tick that external
                  // non-critical interrupts are disabled.  This includes the PIT
                  // interrupt from the OCB timer used to generate the interrupt
                  // that runs the RTL tick code which led us here.  The point is
                  // that this code cannot be re-entrant which implies that if a
                  // request is created without error then it will also be
                  // scheduled before this task runs again.  In the good path we
                  // can never get here and have a BCE request that was not yet
                  // scheduled.
                  // There are four possible request states:
                  // 1. request is idle and complete: The request was created
                  // and scheduled and has completed without error.
                  // 2. request is idle and not complete: The request was created
                  // and scheduled but was either canceled, killed or has errored
                  // out, or there was an error scheduling the request.
                  // 3. request is not idle and not complete: The request was
                  // created and scheduled but is still in progress or still
                  // enqueued.  Note that there is a special case here where this
                  // could also mean that this is the first time we are running
                  // this task so the global request is uninitialized.  It could
                  // also mean there was an error creating the request (unlikely)
                  // so it was never scheduled.
                  // 4. request is not idle and complete:  This can't happen.

                  bool l_proceed_with_request_and_schedule = FALSE;
                  int l_req_idle = async_request_is_idle(&(G_slv_outbox_rx_pba_request[l_slv].request));
                  int l_req_complete = async_request_completed(&(G_slv_outbox_rx_pba_request[l_slv].request));

                  if (!L_bce_slv_outbox_rx_request_created_once[l_slv])
                  {
                      // Do this case first, all other cases assume that this is true!
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
                      TRAC_INFO("BCE slv outbox rx request idle but not complete, \
                              callback_rc=%d options=0x%x state=0x%x abort_state=0x%x \
                              completion_state=0x%x",
                              G_slv_outbox_rx_pba_request[l_slv].request.callback_rc,
                              G_slv_outbox_rx_pba_request[l_slv].request.options,
                              G_slv_outbox_rx_pba_request[l_slv].request.state,
                              G_slv_outbox_rx_pba_request[l_slv].request.abort_state,
                              G_slv_outbox_rx_pba_request[l_slv].request.completion_state);
                      TRAC_INFO("Proceeding with BCE slv outbox rx request and schedule");
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
                      TRAC_INFO("BCE slv outbox rx request not idle and not complete, callback_rc[%d] options[0x%x] state[0x%x] abort_state[0x%x] completion_state[0x%x]",
                              G_slv_outbox_rx_pba_request[l_slv].request.callback_rc,
                              G_slv_outbox_rx_pba_request[l_slv].request.options,
                              G_slv_outbox_rx_pba_request[l_slv].request.state,
                              G_slv_outbox_rx_pba_request[l_slv].request.abort_state,
                              G_slv_outbox_rx_pba_request[l_slv].request.completion_state);

                      TRAC_INFO("NOT proceeding with BCE slv outbox rx request and schedule for slave[0x%02X]",
                              l_slv);
                  }
                  else
                  {
                      // This is not a possible state.
                  }

                  // Only proceed if the BCE request state checked out
                  if (l_proceed_with_request_and_schedule)
                  {
                      // Copy request from main memory to SRAM
                      l_ssxrc = bce_request_create(
                                    &G_slv_outbox_rx_pba_request[l_slv],             // block copy object
                                    &G_pba_bcde_queue,                               // mainstore to sram copy engine
                                    l_slv_main_mem_addr[l_slv],                      // mainstore address
                                    (uint32_t)&G_dcom_slv_outbox_rx[l_slv],          // sram starting address
                                    sizeof(dcom_slv_outbox_t),                       // size of copy
                                    SSX_WAIT_FOREVER,                                // no timeout
                                    (AsyncRequestCallback)setbit_slvoutbox_complete, // call back
                                    (void *)&l_slv,                                  // call back arguments
                                    ASYNC_CALLBACK_IMMEDIATE                         // blocking request
                                    );
                      if(l_ssxrc != SSX_OK)
                      {
                          /* @
                           * @errortype
                           * @moduleid    DCOM_MID_TASK_RX_SLV_OUTBOX
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
                     L_bce_slv_outbox_rx_request_created_once[l_slv] = TRUE;
                     l_ssxrc = bce_request_schedule(&G_slv_outbox_rx_pba_request[l_slv]); // Actual copying

                     if(l_ssxrc != SSX_OK)
                     {
                         /* @
                          * @errortype
                          * @moduleid    DCOM_MID_TASK_RX_SLV_OUTBOX
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
              }
              else
              {
                  /* @
                   * @errortype
                   * @moduleid    DCOM_MID_TASK_RX_SLV_OUTBOX
                   * @reasoncode  INTERNAL_INVALID_INPUT_DATA
                   * @userdata1   N/A
                   * @userdata4   OCC_NO_EXTENDED_RC
                   * @devdesc     Memory related failure
                   */
                  TRAC_ERR("Invalid address from calculate slave inbox address function [%08X]", l_slv_main_mem_addr[l_slv]);
                  l_orc = INTERNAL_INVALID_INPUT_DATA;
                  l_orc_ext = OCC_NO_EXTENDED_RC;
                  break;
              }
           }  //if l_slv_main_mem_addr (slave is present)
        }  // for l_slv copy outbox from main memory
        L_wait4slaves = 0;
    }
    while( 0 );

    // Update the number of OCCs only if there is a new one that showed up
    if((G_sysConfigData.is_occ_present | l_slv_response_mask) != G_sysConfigData.is_occ_present)
    {
        uint8_t l_temp = G_sysConfigData.is_occ_present;

        // Update the mask that stores which OCCs we know are present because they
        // are responding to master OCC (via doorbell).  Only set, never clear.
        // i.e. Don't remove the old ones.  This is what is reported to TMGT
        G_sysConfigData.is_occ_present |= l_slv_response_mask;

        // Since we changed the mask, also update which ones are present.
        G_occ_num_present = __builtin_popcount(G_sysConfigData.is_occ_present);
        TRAC_IMP("Updated OCCs Present -- OldMask: 0x%02x, NewMask: 0x%02x",
                l_temp,
                G_sysConfigData.is_occ_present);
    }

    // Activly responding to master
    G_dcomTime.master.allOccStatusMask.alive  = l_slv_response_mask;
    // Was here, but stopped responding to master
    G_dcomTime.master.allOccStatusMask.zombie =
        ( G_sysConfigData.is_occ_present & ~l_slv_response_mask );
    // Has never been here or has never responded
    G_dcomTime.master.allOccStatusMask.dead   = ~G_sysConfigData.is_occ_present;

    if ( l_orc != OCC_SUCCESS_REASON_CODE )
    {
        // create and commit error
        dcom_error_check( SLAVE_OUTBOX, FALSE, l_orc, l_orc_ext);
    }
    else
    {
        // done, lets clear our counter
        dcom_error_check_reset( SLAVE_OUTBOX );
    }

}

// Function Specification
//
// Name: dcom_rx_slv_outbox_doorbell
//
// Description: Receive unicast doorbell and save data
//              from slave (slave to master)
//
// End Function Specification
uint32_t dcom_rx_slv_outbox_doorbell( void )
{
    int         l_pbarc = 0;
    uint32_t    l_read = 0;

    // Grab doorbells from slave, read out the whole queue to prevent overflow
    l_pbarc = pbax_read(
            &G_pbax_read_queue[1],
            &G_dcom_slv_outbox_doorbell_rx[0],
            sizeof(dcom_slv_outbox_doorbell_t)*NUM_ENTRIES_PBAX_QUEUE1,
            &l_read
            );

    if (l_pbarc != 0)
    {
        // Failure occurred
        TRAC_ERR("Master PBAX Read Failure in receiving slave doorbells - RC[%08X]", l_pbarc);

        // Handle pbax read failure on queue 1
        dcom_pbax_error_handler(1);
    }

    // Return the number of doorbells read by dividing the bytes read by the doorbell size
    return (l_read/sizeof(dcom_slv_outbox_doorbell_t));
}

#endif // DCOMMASTERRX_C

