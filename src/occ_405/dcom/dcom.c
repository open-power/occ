/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/dcom/dcom.c $                                     */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2020                        */
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

#ifndef _DCOM_C
#define _DCOM_C

//#include <pgp_pmc.h>
#include "ssx.h"
#include "occhw_pba.h"
#include <rtls.h>
#include <apss.h>
#include <dcom.h>
#include <dcom_service_codes.h>
#include <occ_service_codes.h>
#include <trac.h>
#include <state.h>
#include <proc_pstate.h>
#include <amec_data.h>
#include <amec_sys.h>
#include "pss_constants.h"

extern uint8_t G_occ_interrupt_type;
extern uint16_t G_proc_fmax_mhz;   // max(turbo,uturbo) frequencies
extern data_cnfg_t * G_data_cnfg;

dcom_timing_t G_dcomTime;

DMA_BUFFER( dcom_slv_inbox_t G_dcom_slv_inbox_tx[MAX_OCCS]) = {{0}};
DMA_BUFFER( dcom_slv_outbox_t G_dcom_slv_outbox_tx) = {0};

DMA_BUFFER( dcom_slv_outbox_t G_dcom_slv_outbox_rx[MAX_OCCS]) = {{0}};
DMA_BUFFER( dcom_slv_inbox_t G_dcom_slv_inbox_rx) = {0};

// =========================================================
// Master & Slave
// =========================================================

// PBAX Circ Queue buffers (where PBAX will put the data in OCC SRAM, so that OCC
// can grab it.

PBAX_CQ_READ_BUFFER(G_pbax_queue_rx1_buffer,NUM_ENTRIES_PBAX_QUEUE1);
PBAX_CQ_READ_BUFFER(G_pbax_queue_rx0_buffer,NUM_ENTRIES_PBAX_QUEUE0);

// Initialize Globals

// Indicate that Slave OCC got an inbox from master
bool        G_slv_inbox_received      = FALSE;

// Counters to debug Master/Slave communication errors
dcom_fail_count_t G_dcomSlvInboxCounter = {0};

uint8_t     G_occ_role = OCC_SLAVE;

// PBAX ID of this OCC is also its PowerBus ID.  Contains ChipId & NodeId.
pob_id_t    G_pbax_id                  = {0};

// PBAX 'Target' Structure (Register Abstraction) that has the data needed for
// a multicast operation.
PbaxTarget  G_pbax_multicast_target;

// PBAX 'Target' Structure (Register Abstraction) that has the data needed for
// a unicast operation from the OCC Slave to the OCC Master.
PbaxTarget  G_pbax_unicast_target;

// Number of occ's that *should* be present
uint8_t     G_occ_num_present;

// Master/slave event flags
uint32_t    G_master_event_flags              = 0;
uint32_t    G_slave_event_flags               = 0;
uint32_t    G_master_event_flags_ack          = 0;
uint32_t    G_slave_event_flags_ack[MAX_OCCS] = {0};

// Helper function to determine if slave inboxes are valid
bool isDcomSlvInboxValid(void)
{
    return (G_dcomSlvInboxCounter.currentFailCount ? FALSE : TRUE);
}

// Function Specification
//
// Name: dcom_initialize_roles
//
// Description: Initialize roles so we know if we are master or slave
//
// End Function Specification
void dcom_initialize_roles(void)
{
    G_occ_role = OCC_SLAVE;

    // Locals
    pba_xcfg_t pbax_cfg_reg;

    G_dcomTime.tod = in64(OCB_OTBR) >> 4;
    G_dcomTime.base = ssx_timebase_get();
    pbax_cfg_reg.value = in64(PBA_XCFG);

    if(pbax_cfg_reg.fields.rcv_groupid < MAX_NUM_NODES &&
       pbax_cfg_reg.fields.rcv_chipid < MAX_NUM_OCC)
    {

        TRAC_IMP("Proc ChipId (%d)  NodeId (%d)",
                 pbax_cfg_reg.fields.rcv_chipid,
                 pbax_cfg_reg.fields.rcv_groupid);

        G_pbax_id.node_id   = pbax_cfg_reg.fields.rcv_groupid;
        G_pbax_id.chip_id   = pbax_cfg_reg.fields.rcv_chipid;
        // Always start as OCC Slave
        G_occ_role = OCC_SLAVE;
        rtl_set_run_mask(RTL_FLAG_NOTMSTR);


        // Set the initial presence mask, and count the number of occ's present
        G_sysConfigData.is_occ_present |= (0x01 << G_pbax_id.chip_id);
        G_occ_num_present = __builtin_popcount(G_sysConfigData.is_occ_present);

    }
    else // Invalid chip/node ID(s)
    {
        TRAC_ERR("Proc ChipId (%d) and/or NodeId (%d) too high: request reset",
                 pbax_cfg_reg.fields.rcv_chipid,
                 pbax_cfg_reg.fields.rcv_groupid);
        /* @
         * @errortype
         * @moduleid    DCOM_MID_INIT_ROLES
         * @reasoncode  INVALID_CONFIG_DATA
         * @userdata1   PBAXCFG (upper)
         * @userdata2   PBAXCFG (lower)
         * @userdata4   ERC_CHIP_IDS_INVALID
         * @devdesc     Failure to determine OCC chip ID
         */
        errlHndl_t  l_errl = createErrl(
            DCOM_MID_INIT_ROLES,            //ModId
            INVALID_CONFIG_DATA,            //Reasoncode
            ERC_CHIP_IDS_INVALID,           //Extended reasoncode
            ERRL_SEV_UNRECOVERABLE,         //Severity
            NULL,                           //Trace Buf
            DEFAULT_TRACE_SIZE,             //Trace Size
            pbax_cfg_reg.words.high_order,  //Userdata1
            pbax_cfg_reg.words.low_order    //Userdata2
            );

        // Callout firmware
        addCalloutToErrl(l_errl,
                         ERRL_CALLOUT_TYPE_COMPONENT_ID,
                         ERRL_COMPONENT_ID_FIRMWARE,
                         ERRL_CALLOUT_PRIORITY_HIGH);

        //Add processor callout
        addCalloutToErrl(l_errl,
                         ERRL_CALLOUT_TYPE_HUID,
                         G_sysConfigData.proc_huid,
                         ERRL_CALLOUT_PRIORITY_LOW);

        CHECKPOINT_FAIL_AND_HALT(l_errl);
    }

// Initialize DCOM Thread Sem
    ssx_semaphore_create( &G_dcomThreadWakeupSem, // Semaphore
                          1,                      // Initial Count
                          0);                     // No Max Count

}

// Function Specification
//
// Name: dcom_initialize_pbax_queues
//
// Description: Initialize the PBAX Queues for sending doorbells
//
// End Function Specification
void dcom_initialize_pbax_queues(void)
{
    // SSX return codes
    int l_rc = 0;

    do
    {
        //disabled pbax send before configuring PBAX
        pbax_send_disable();

        // Node and Chip IDs are set by Hostboot and are not needed
        // for pbax_configure
        l_rc = pbax_configure(G_occ_role,                     // master
                              G_pbax_id.node_id,              // node id
                              G_pbax_id.chip_id,              // chip id
                              PBAX_CONFIGURE_RCV_GROUP_MASK); // group_mask

        if(l_rc != 0)
        {
            TRAC_ERR("Error configuring the pbax rc[%x]",l_rc);
            break;
        }

        //enabled pbax send does not return errors
        pbax_send_enable();

        if(G_occ_role == OCC_SLAVE)
        {
            // create pbax rx queue 1
            l_rc = pbax_queue_create( &G_pbax_read_queue[1], //queue
                    ASYNC_ENGINE_PBAX_PUSH1,                 //engine
                    G_pbax_queue_rx1_buffer,                 //cq base
                    NUM_ENTRIES_PBAX_QUEUE1,                 //cq entries
                    PBAX_INTERRUPT_PROTOCOL_AGGRESSIVE       //protocol
                    );

            if(l_rc != 0)
            {
                TRAC_ERR("Error creating pbax queue 1 rc[%x]",l_rc);
                break;
            }

            // create pbax rx queue 0
            l_rc = pbax_queue_create( &G_pbax_read_queue[0],//queue
                    ASYNC_ENGINE_PBAX_PUSH0,                //engine
                    G_pbax_queue_rx0_buffer,                //cq base
                    NUM_ENTRIES_PBAX_QUEUE0,                //cq entries
                    PBAX_INTERRUPT_PROTOCOL_AGGRESSIVE      //protocol
                    );

            if(l_rc != 0)
            {
                TRAC_ERR("Error creating pbax queue 0 rc[%x]",l_rc);
                break;
            }

            // enable the read 1 queue
            l_rc = pbax_queue_enable(&G_pbax_read_queue[1]);

            if(l_rc != 0)
            {
                TRAC_ERR("Error enabling queue 1 rc[%x]",l_rc);
                break;
            }
        }

        if(G_occ_role == OCC_MASTER)
        {
            l_rc = pbax_target_create( &G_pbax_multicast_target,    // target,
                    PBAX_BROADCAST,                                 // type
                    PBAX_SYSTEM,                                    // scope
                    0,                                              // queue
                    G_pbax_id.node_id,                              // node
                    PBAX_BROADCAST_GROUP,                           // chip_or_group
                    0);                                             // cnt (Sends [cnt+1]*8 bytes)

            if(l_rc != 0)
            {
                TRAC_ERR("Error creating pbax target for master TX operations SSXrc[%x]",l_rc);
                break;
            }

        }

    }while(0);

    if(l_rc)
    {
        /* @
         * @errortype
         * @moduleid    DCOM_MID_INIT_PBAX_QUEUES
         * @reasoncode  SSX_GENERIC_FAILURE
         * @userdata1   SSX RC
         * @userdata4   OCC_NO_EXTENDED_RC
         * @devdesc     Failure initializing the PBAX queues
         */
        errlHndl_t  l_errl = createErrl(
            DCOM_MID_INIT_PBAX_QUEUES,          //ModId
            SSX_GENERIC_FAILURE,                //Reasoncode
            OCC_NO_EXTENDED_RC,                 //Extended reasoncode
            ERRL_SEV_UNRECOVERABLE,             //Severity
            NULL,                               //Trace Buf
            DEFAULT_TRACE_SIZE,                 //Trace Size
            l_rc,                               //Userdata1
            0                                   //Userdata2
            );

        CHECKPOINT_FAIL_AND_HALT(l_errl);
    }
}

// Function Specification
//
// Name: dcom_error_check
//
// Description: keep track of failure counts
//
// End Function Specification
void dcom_error_check( const dcom_error_type_t i_error_type, const bool i_clear_error, const uint32_t i_orc, const uint32_t i_orc_ext)
{
    static uint16_t L_rx_slv_outbox_fail_count = 0;
    uint16_t        l_modId = 0;
    uint16_t        *l_count_ptr = NULL;

    if ( i_error_type == SLAVE_INBOX )
    {
        l_count_ptr = &G_dcomSlvInboxCounter.currentFailCount;
        l_modId = DCOM_MID_TASK_RX_SLV_INBOX;
    }
    // if the i_error_type == SLAVE_OUTBOX then set the outbox count
    else
    {
        l_count_ptr = &L_rx_slv_outbox_fail_count;
        l_modId = DCOM_MID_TASK_RX_SLV_OUTBOX;
    }

    if ( i_clear_error )
    {
        *l_count_ptr = 0;
    }
    else
    {
        (*l_count_ptr)++;

        if ( *l_count_ptr == DCOM_250us_GAP )
        {
            // Trace an imp trace log
            TRAC_IMP("l_count_ptr[%d], L_outbox[%d], L_inbox[%d]",
                *l_count_ptr,
                L_rx_slv_outbox_fail_count,
                G_dcomSlvInboxCounter.currentFailCount );
        }
        else if ( *l_count_ptr == DCOM_4MS_GAP )
        {
            // Create and commit error log
            // NOTE: SRC tags are NOT needed here, they are
            //       taken care of by the caller
            errlHndl_t  l_errl = createErrl(
                l_modId,                        //ModId
                i_orc,                          //Reasoncode
                i_orc_ext,                      //Extended reasoncode
                ERRL_SEV_UNRECOVERABLE,         //Severity
                NULL,                           //Trace Buf
                DEFAULT_TRACE_SIZE,             //Trace Size
                *l_count_ptr,                   //Userdata1
                0                               //Userdata2
                );

            // Commit log
            commitErrl( &l_errl );

        }
        else if ( *l_count_ptr == DCOM_1S_GAP )
        {
            // Create and commit error log
            // NOTE: SRC tags are NOT needed here, they are
            //       taken care of by the caller
            errlHndl_t  l_errl = createErrl(
                l_modId,                        //ModId
                i_orc,                          //Reasoncode
                i_orc_ext,                      //Extended reasoncode
                ERRL_SEV_UNRECOVERABLE,         //Severity
                NULL,                           //Trace Buf
                DEFAULT_TRACE_SIZE,             //Trace Size
                *l_count_ptr,                   //Userdata1
                0                               //Userdata2
                );

            // Commit log
            // Call request reset macro
            REQUEST_RESET(l_errl);
        }
    }
}

// Function Specification
//
// Name: dcom_build_occfw_msg
//
// Description: Copy data into occ fw msg portion
//
// End Function Specification
void dcom_build_occfw_msg( const dcom_error_type_t i_which_msg )
{
    if ( i_which_msg == SLAVE_INBOX )
    {
        uint32_t l_slv_idx = 0;

        // For each occ slave
        for(; l_slv_idx < MAX_OCCS; l_slv_idx++)
        {
            G_dcom_slv_inbox_tx[l_slv_idx].occ_fw_mailbox[0] = G_occ_external_req_state;
            G_dcom_slv_inbox_tx[l_slv_idx].occ_fw_mailbox[1] = G_occ_external_req_mode;

            G_dcom_slv_inbox_tx[l_slv_idx].occ_fw_mailbox[2] = G_master_event_flags;
            G_dcom_slv_inbox_tx[l_slv_idx].occ_fw_mailbox[3] = G_slave_event_flags_ack[l_slv_idx];

            G_dcom_slv_inbox_tx[l_slv_idx].occ_fw_mailbox[4] = 0;
        }
    }
    else if ( i_which_msg == SLAVE_OUTBOX )
    {
        G_dcom_slv_outbox_tx.occ_fw_mailbox[0] = CURRENT_STATE();

        if(G_sysConfigData.system_type.kvm )
        {
            G_dcom_slv_outbox_tx.occ_fw_mailbox[1] = G_occ_external_req_mode_kvm;
        }
        else
        {
            G_dcom_slv_outbox_tx.occ_fw_mailbox[1] = CURRENT_MODE();
        }

        G_dcom_slv_outbox_tx.occ_fw_mailbox[2] = G_master_event_flags_ack;
        G_dcom_slv_outbox_tx.occ_fw_mailbox[3] = G_slave_event_flags;

        G_dcom_slv_outbox_tx.occ_fw_mailbox[4] = SMGR_validate_get_valid_states();
    }

}


// Function Specification
//
// Name: task_dcom_parse_occfwmsg
//
// Description: Purpose of this task is to handle and acknowledge
//              fw messages passed from Master to Slave and vice versa.
//
// End Function Specification
void task_dcom_parse_occfwmsg(task_t *i_self)
{
    if(G_occ_role == OCC_MASTER)
    {
        // Local slave index counter
        uint32_t l_slv_idx      = 0;

        // Loop and collect occ data for each slave occ
        for(; l_slv_idx < MAX_OCCS; l_slv_idx++)
        {
            // Verify all slave are in correct state and mode
            G_dcom_slv_outbox_rx[l_slv_idx].occ_fw_mailbox[0] = CURRENT_STATE();

            if(G_sysConfigData.system_type.kvm )
            {
                G_dcom_slv_outbox_rx[l_slv_idx].occ_fw_mailbox[1] = G_occ_external_req_mode_kvm;
            }
            else
            {
                G_dcom_slv_outbox_rx[l_slv_idx].occ_fw_mailbox[1] = CURRENT_MODE();
            }

            // Acknowledge all slave event flags
            G_slave_event_flags_ack[l_slv_idx] = G_dcom_slv_outbox_rx[l_slv_idx].occ_fw_mailbox[3];

            // Clear master event flags if slave has acknowledged them and the event has cleared
            G_master_event_flags &= ~G_dcom_slv_outbox_rx[l_slv_idx].occ_fw_mailbox[2];

        }

    }//End master role check

    // Check if master has changed state and mode and update if changed
    // so that we can handle it in a thread.
    if( (G_occ_master_state != G_dcom_slv_inbox_rx.occ_fw_mailbox[0])
        || (G_occ_master_mode != G_dcom_slv_inbox_rx.occ_fw_mailbox[1]) )
    {
        if( ! isSafeStateRequested() )
        {
            G_occ_master_state = G_dcom_slv_inbox_rx.occ_fw_mailbox[0];
            G_occ_master_mode  = G_dcom_slv_inbox_rx.occ_fw_mailbox[1];
            ssx_semaphore_post(&G_dcomThreadWakeupSem);
        }
    }

    // Copy mnfg parameters into g_amec structure
    g_amec->foverride_enable = G_dcom_slv_inbox_rx.foverride_enable;
    g_amec->foverride = G_dcom_slv_inbox_rx.foverride;

    // Copy IPS parameters sent by Master OCC
    g_amec->slv_ips_freq_request = G_dcom_slv_inbox_rx.ips_freq_request;

    // Copy DPS tunable parameters sent by Master OCC if required
    if(G_dcom_slv_inbox_rx.tunable_param_overwrite)
    {
        AMEC_part_overwrite_dps_parameters();

        if(G_dcom_slv_inbox_rx.tunable_param_overwrite == 1)
        {
            // parameter(s) overwritten by user
            g_amec->slv_dps_param_overwrite = TRUE;
        }
        else // ==2 use defaults for all parameters
        {
            g_amec->slv_dps_param_overwrite = FALSE;
        }

        // check if user has WOF enabled
        if(G_dcom_slv_inbox_rx.wof_enable)
        {
            if(g_amec->wof.wof_disabled & WOF_RC_USER_DISABLED_WOF)
            {
               set_clear_wof_disabled( CLEAR,
                                       WOF_RC_USER_DISABLED_WOF,
                                       ERC_WOF_USER_DISABLED_WOF );
               TRAC_INFO("User enabled WOF! wof_disabled = 0x%08X", g_amec->wof.wof_disabled);
            }
        }
        else // user has WOF disabled
        {
            if(!(g_amec->wof.wof_disabled & WOF_RC_USER_DISABLED_WOF))
            {
               set_clear_wof_disabled( SET,
                                       WOF_RC_USER_DISABLED_WOF,
                                       ERC_WOF_USER_DISABLED_WOF );
               TRAC_INFO("User disabled WOF! wof_disabled = 0x%08X", g_amec->wof.wof_disabled);
            }
        }
    }

    // Copy soft frequency boundaries sent by Master OCC
    g_amec->part_config.part_list[0].soft_fmin = G_dcom_slv_inbox_rx.soft_fmin;
    g_amec->part_config.part_list[0].soft_fmax = G_dcom_slv_inbox_rx.soft_fmax;

    // acknowledge all masters event flags
    G_master_event_flags_ack = G_dcom_slv_inbox_rx.occ_fw_mailbox[2];

    // clear slave event flags if master has acknowledged them and the event has cleared
    G_slave_event_flags = (G_slave_event_flags & (~(G_dcom_slv_inbox_rx.occ_fw_mailbox[3])));
}


// Function Specification
//
// Name: dcom_pbax_error_handler
//
// Description: Handle an error from a pbax_read call
//
// End Function Specification
void dcom_pbax_error_handler(const uint8_t i_queue)
{
    pba_xshcsn_t l_pba_shcs;
    pba_xcfg_t   l_pbax_cfg;

    SsxAddress   l_pba_shcs_addr = PBA_XSHCS0;
    errlHndl_t   l_err = NULL;
    static bool  L_pba_reset_logged[2] = {FALSE};

    // Skip if waiting for a reset, no sense in trying to recover when going to be reset anyway
    if((TRUE == isSafeStateRequested()) || (CURRENT_STATE() == OCC_STATE_SAFE))
        return;

    if(i_queue == 1)
        l_pba_shcs_addr = PBA_XSHCS1;

    l_pba_shcs.words.high_order = in32(l_pba_shcs_addr);
    uint32_t xsndstat = 0;
    uint32_t xrcvstat = 0;
    xsndstat = in32(PBA_XSNDSTAT);
    xrcvstat = in32(PBA_XRCVSTAT);

    TRAC_ERR("dcom_pbax_error_handler: Start error handler for queue %d PBA_XSHCS[0x%08x] PBA_XSNDSTAT[0x%08X] PBA_XRCVSTAT[0x%08X]",
             i_queue, l_pba_shcs.words.high_order, xsndstat, xrcvstat);

    do
    {
        // reset queue and clear the error condition to allow future pbax reads

        // 1. Disable the pushQ and reset the read & write pointer by writing 0 to push_enable (bit 31)
        l_pba_shcs.fields.push_enable = 0;
        out32(l_pba_shcs_addr, l_pba_shcs.words.high_order);

        // 2. Clear the error status by setting rcv_reset (bit 3) in the PBAX CFG register
        l_pbax_cfg.value = in64(PBA_XCFG);
        l_pbax_cfg.fields.rcv_reset = 1;
        out64(PBA_XCFG, l_pbax_cfg.value);

        // 3.  Reenable the pushQ (set push_enable bit 31)
        l_pba_shcs.fields.push_enable = 1;
        out32(l_pba_shcs_addr, l_pba_shcs.words.high_order);

        TRAC_INFO("dcom_pbax_error_handler: Success resetting queue %d PBA_XSHCS[0x%08x] PBA_XCFG[0x%08x]",
                  i_queue, in32(l_pba_shcs_addr), in32(PBA_XCFG));

        if(L_pba_reset_logged[i_queue] == FALSE)
        {
            // log error to indicate queue was reset
            /* @
             * @errortype
             * @moduleid    DCOM_MID_PBAX_ERROR_HANDLER
             * @reasoncode  PBAX_QUEUE_RESET
             * @userdata1   PBA queue
             * @userdata4   OCC_NO_EXTENDED_RC
             * @devdesc     PBAX queue reset
             */
            l_err = createErrl( DCOM_MID_PBAX_ERROR_HANDLER,    //modId
                                PBAX_QUEUE_RESET,               //reasoncode
                                OCC_NO_EXTENDED_RC,             //Extended reason code
                                ERRL_SEV_INFORMATIONAL,         //Severity
                                NULL,                           //Trace Buf
                                DEFAULT_TRACE_SIZE,             //Trace Size
                                i_queue,                        //userdata1
                                0);                             //userdata2
            commitErrl(&l_err);
            L_pba_reset_logged[i_queue] = TRUE;
        }
    }while(0);
}

#endif //_DCOM_C

