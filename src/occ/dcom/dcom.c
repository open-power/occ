/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/dcom/dcom.c $                                         */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2015                        */
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

#include <pgp_pmc.h>
#include "pgp_pba.h"
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
#include "scom.h"

#define PBAX_CONFIGURE_RCV_GROUP_MASK 0xff

#define PBAX_BROADCAST_GROUP 0xFF

extern uint8_t G_occ_interrupt_type;

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

uint8_t     G_occ_role                = OCC_SLAVE;

uint8_t     G_dcm_occ_role            = OCC_DCM_SLAVE;

// PowerBus ID of this OCC.  Contains ChipId & NodeId.
pob_id_t    G_pob_id                  = {0};

// PBAX 'Target' Structure (Register Abstraction) that has the data needed for
// a multicast operation.
PbaxTarget  G_pbax_multicast_target;

// PBAX 'Target' Structure (Register Abstraction) that has the data needed for
// a unicast operation from the OCC Slave to the OCC Master.
PbaxTarget  G_pbax_unicast_target;

// Number of occ's that *should* be present
uint8_t     G_occ_num_present;

// DCM Status from all Slaves
proc_gpsm_dcm_sync_occfw_t G_dcm_sync_occfw_table[MAX_OCCS];

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
    int         l_rc = 0;
    tpc_gp0_t   l_tp_gp0_read;

    // Used as a debug tool to correlate time between OCCs & System Time
    getscom_ffdc( TOD_VALUE_REG, &G_dcomTime.tod, NULL); // Commits errors internally
    G_dcomTime.base = ssx_timebase_get();

    // Scom will timeout if it can't be read
    l_rc = getscom_ffdc( TPC_GP0, (uint64_t *) &l_tp_gp0_read, NULL);  // Commits errors internally

    if( l_rc == 0 )
    {
        // Added check for Murano ChipId swizzle
        if(CFAM_CHIP_TYPE_MURANO == cfam_chip_type())
        {
            // Murano has a different numbering scheme than you would
            // expect.  It uses NodeId to denote DCM Id, and ChipId to
            // denote chip within the DCM.  This is due to they way the
            // PowerBus works for routing.
            //
            // To fix this, we need to manipulate our internal copy of
            // ChipId/NodeId to match the way OCC FW uses them. We do this by
            // multiplying the NodeId by 2 then adding chip Id to get a unique
            // new ChipId (Max Node = 3, Max Chip = 1 by design
            //
            // Note that Murano is not multi-drawer capable, so we can
            // fix our node id at 0

#define MAX_MURANO_CHIP_IDS 2
#define MAX_MURANO_NODE_IDS 4
            if( (l_tp_gp0_read.fields.tc_chip_id_dc < MAX_MURANO_CHIP_IDS)
                    && (l_tp_gp0_read.fields.tc_node_id_dc < MAX_MURANO_NODE_IDS))
            {
                // TODO: Check if possible to use node_id read from the chip GPIOs

                // Translate between chip ID & Module Id for Tuleta
                uint8_t tuleta_chip2module[] = {0,0,2,2,1,1,3,3};

                G_pob_id.chip_id = (l_tp_gp0_read.fields.tc_chip_id_dc
                        + ( MAX_MURANO_CHIP_IDS * l_tp_gp0_read.fields.tc_node_id_dc));
                G_pob_id.node_id = 0;

                // The module id is only used by Power Measurements
                G_pob_id.module_id = tuleta_chip2module[G_pob_id.chip_id];
            }
            else
            {
                // Chip Ids don't make any sense
                TRAC_ERR("Proc ChipId (%d) and/or NodeId (%d) don't make sense for Murano",
                        l_tp_gp0_read.fields.tc_chip_id_dc,
                        l_tp_gp0_read.fields.tc_node_id_dc);
                /* @
                 * @errortype
                 * @moduleid    DCOM_MID_INIT_ROLES
                 * @reasoncode  INTERNAL_HW_FAILURE
                 * @userdata1   TP.GP0 SCOM (upper)
                 * @userdata2   TP.GP0 SCOM (lower)
                 * @userdata4   ERC_CHIP_IDS_INVALID
                 * @devdesc     Failure determining OCC role
                 */
                errlHndl_t  l_errl = createErrl(
                        DCOM_MID_INIT_ROLES,            //ModId
                        INTERNAL_HW_FAILURE,            //Reasoncode
                        ERC_CHIP_IDS_INVALID,           //Extended reasoncode
                        ERRL_SEV_UNRECOVERABLE,         //Severity
                        NULL,                           //Trace Buf
                        DEFAULT_TRACE_SIZE,             //Trace Size
                        l_tp_gp0_read.words.high_order, //Userdata1
                        l_tp_gp0_read.words.low_order   //Userdata2
                        );

                // Callout firmware
                addCalloutToErrl(l_errl,
                                 ERRL_CALLOUT_TYPE_COMPONENT_ID,
                                 ERRL_COMPONENT_ID_FIRMWARE,
                                 ERRL_CALLOUT_PRIORITY_HIGH);

                // Commit log
                commitErrl( &l_errl );
            }
        }
        else
        {
            // Save off chip and node ids directly as read
            G_pob_id.chip_id = l_tp_gp0_read.fields.tc_chip_id_dc;
            G_pob_id.node_id = l_tp_gp0_read.fields.tc_node_id_dc;

            // Check if special SMP wrap mode is turned on. In this mode, a
            // single drawer is configured as two virtual nodes. However, OCC
            // still needs to treat it as a single node.
            // As a temporary solution, HWSV is going to set bit 17 of the GP0
            // register to inform OCC that SMP wrap is on.

#define SMP_WRAP_MASK 0x00004000
            if(l_tp_gp0_read.words.high_order & SMP_WRAP_MASK)
            {
                TRAC_INFO("dcom_initialize_roles: Temporary fix - SMP wrap mode has been detected");

                // This is a single drawer
                G_pob_id.node_id = 0;

                // Translate the NodeId and ChipId into the correct internal
                // representation for OCC to work.
                if(l_tp_gp0_read.fields.tc_node_id_dc == 0)
                {
                    G_pob_id.chip_id = 2 * l_tp_gp0_read.fields.tc_chip_id_dc;
                }
                else if(l_tp_gp0_read.fields.tc_node_id_dc == 1)
                {
                    G_pob_id.chip_id = (l_tp_gp0_read.fields.tc_chip_id_dc) ? 1 : 3;
                }
            }

            // If this is a FSP-less system, then use the node ID as the
            // chip ID. This is because the HW assigns the OCCs as being in
            // different nodes with the same chip IDs.  This is known to be
            // true for the Firestone chipset.
            // Naples chip (Garrison) use chipID.
            if ((G_occ_interrupt_type != FSP_SUPPORTED_OCC) &&
                (CFAM_CHIP_TYPE_NAPLES != cfam_chip_type()) )

            {
                G_pob_id.node_id = 0;
                G_pob_id.chip_id = l_tp_gp0_read.fields.tc_node_id_dc;

                TRAC_IMP("dcom_initialize_roles: Overriding chip_id[%d] with node_id[%d]",
                         l_tp_gp0_read.fields.tc_chip_id_dc,
                         l_tp_gp0_read.fields.tc_node_id_dc);
            }

            // Save off low 2 bits of chip ID as module ID.  Won't be
            // more than 4 on venice since it is SCMs.
            G_pob_id.module_id = (G_pob_id.chip_id & 0x03);
        }

        // Always start as OCC Slave
        G_occ_role = OCC_SLAVE;
        rtl_set_run_mask(RTL_FLAG_NOTMSTR);

        // Save off OCC role inside DCM chip
        if(gpsm_dcm_slave_p())
        {
            G_dcm_occ_role = OCC_DCM_SLAVE;
        }
        else
        {
            G_dcm_occ_role = OCC_DCM_MASTER;
        }

        TRAC_IMP("Proc ChipId=%d, NodeId=%d, isDcm=%d, isDcmMaster=%d, ChipEC=0x%08x",
                G_pob_id.chip_id,
                G_pob_id.node_id,
                gpsm_dcm_mode_p(),
                !gpsm_dcm_slave_p(),
                cfam_id() );
    }
    else
    {
        //get scom failure
        TRAC_ERR("getscom failure rc[0x%08X]", -l_rc );

        /* @
         * @errortype
         * @moduleid    DCOM_MID_INIT_ROLES
         * @reasoncode  INTERNAL_HW_FAILURE
         * @userdata1   getscom failure rc
         * @userdata4   ERC_GETSCOM_FAILURE
         * @devdesc     Failure determining OCC role
         */
        errlHndl_t  l_errl = createErrl(
                    DCOM_MID_INIT_ROLES,            //ModId
                    INTERNAL_HW_FAILURE,            //Reasoncode
                    ERC_GETSCOM_FAILURE,            //Extended reasoncode
                    ERRL_SEV_UNRECOVERABLE,         //Severity
                    NULL,                           //Trace Buf
                    DEFAULT_TRACE_SIZE,             //Trace Size
                    l_rc,                           //Userdata1
                    0                               //Userdata2
                    );

        // Callout firmware
        addCalloutToErrl(l_errl,
                         ERRL_CALLOUT_TYPE_COMPONENT_ID,
                         ERRL_COMPONENT_ID_FIRMWARE,
                         ERRL_CALLOUT_PRIORITY_HIGH);

        // Commit log
        commitErrl( &l_errl );

        // TODO request a reset of OCC
        // we are toast without this working correctly
    }

    // Set the initial presence mask, and count the number of occ's present
    G_sysConfigData.is_occ_present |= (0x01 << G_pob_id.chip_id);
    G_occ_num_present = __builtin_popcount(G_sysConfigData.is_occ_present);

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
    pbax_id_t  l_pbaxid = dcom_pbusid2pbaxid(G_pob_id);

    //SSX return codes
    int l_rc = 0;

    do
    {
        pbax_send_disable();

        // Check if conversion has valid information
        if (( l_pbaxid.chip_id > MAX_PBAX_CHIP_ID ) ||
                ( l_pbaxid.node_id == INVALID_NODE_ID ))
        {
            TRAC_ERR("Error converting pbusids to pbaxids. chip_id[0x%08x], node_id[0x%08x]",
                     l_pbaxid.chip_id, l_pbaxid.node_id);
            l_rc = -1; // Force error to be logged below.
            break;
        }

        l_rc = pbax_configure(G_occ_role,        // master
                              l_pbaxid.node_id,  // node id
                              l_pbaxid.chip_id,  // chipd id
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
            l_rc = pbax_queue_create( &G_pbax_read_queue[1],//queue
                    ASYNC_ENGINE_PBAX_PUSH1,                //engine
                    G_pbax_queue_rx1_buffer,                //cq base
                    NUM_ENTRIES_PBAX_QUEUE1,                //cq entries
                    PBAX_INTERRUPT_PROTOCOL_AGGRESSIVE      //protocol
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
            // TODO: Change this to PBAX_GROUP for Venice
            l_rc = pbax_target_create( &G_pbax_multicast_target,    // target,
                    PBAX_BROADCAST,             // type
                    PBAX_SYSTEM,                // scope TODO
                    0,                          // queue
                    l_pbaxid.node_id,           // node
                    PBAX_BROADCAST_GROUP);      // chip_or_group

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

        // Commit log and request reset
        REQUEST_RESET(l_errl);
    }
}

// Function Specification
//
// Name: dcom_pbusid2pbaxid
//
// Description: Translate between PowerBus ID and pbax ID
//
// End Function Specification
pbax_id_t dcom_pbusid2pbaxid(pob_id_t i_pobid)
{
    pbax_id_t l_pbax_id_t = {0};

    // Check if chip id and nod id are valid
    if((i_pobid.chip_id < MAX_NUM_OCC)
        && (i_pobid.node_id < MAX_NUM_NODES))
    {
        l_pbax_id_t.chip_id = G_sysConfigData.pob2pbax_chip[i_pobid.chip_id];
        l_pbax_id_t.node_id = G_sysConfigData.pob2pbax_node[i_pobid.node_id];
    }
    else
    {
        // Invalid data found

        l_pbax_id_t.chip_id = MAX_PBAX_CHIP_ID;
        l_pbax_id_t.node_id = INVALID_NODE_ID;

        TRAC_ERR("Invalid Powerbus ID, could NOT convert chip id[%x] and node id[%x] to PBAX id",
                   i_pobid.chip_id,i_pobid.node_id);
    }

    return l_pbax_id_t;
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

            // Call request nominal macro to change state
            REQUEST_NOMINAL();
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
// Name: dcom_build_dcm_sync_msg
//
// Description: Copy messages from DCM Master (OCC Slave) to
//              DCM Slave (also OCC Slave) and vice versa
//
// End Function Specification
void dcom_build_dcm_sync_msg(const dcom_error_type_t i_which_msg)
{
    // If the OCC Master isn't a DCM, no one else is a DCM either, so
    // no need to bother sending these messages back & forth.
    if(proc_is_dcm())
    {
        if ( i_which_msg == SLAVE_INBOX )
        {
            uint32_t l_slv_idx = 0;
            for(l_slv_idx = 0; l_slv_idx < MAX_OCCS; l_slv_idx++)
            {
                // Populate G_dcm_sync_occfw_table with the data from all OCC Slaves
                G_dcm_sync_occfw_table[l_slv_idx] = G_dcom_slv_outbox_rx[l_slv_idx].dcm_sync;
            }

            // DCM are always in even/odd numbered pairs sequentially as
            // DCM master = even number[0,2,4,6] DCM Slave = odd number [1,3,5,7]
            // with DCM pairs being [0,1], [2,3], [4,5], [6,7]
            // so we can do this simple swizzle here and not need a table to
            // do the conversion.
            for(l_slv_idx = 0; l_slv_idx < MAX_OCCS; l_slv_idx+=2)
            {
                G_dcom_slv_inbox_tx[l_slv_idx].dcm_sync = G_dcm_sync_occfw_table[l_slv_idx+1];
                G_dcom_slv_inbox_tx[l_slv_idx+1].dcm_sync = G_dcm_sync_occfw_table[l_slv_idx];
            }
        }
        else if ( i_which_msg == SLAVE_OUTBOX )
        {
            G_dcom_slv_outbox_tx.dcm_sync = proc_gpsm_dcm_sync_get_state();
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

    // If we are master, we don't want to update based on
    // the data sent to us, because it corrupts the 'golden' data
    // If we are in standby, we don't want to update because
    // the data may not have been set up yet, and would be set to zero.
    if(OCC_MASTER != G_occ_role )
    {
        // Update the system mode frequencies if they have changed
        int l_mode    = 0;
        bool l_change = FALSE;
        bool l_all_zero = TRUE;

        // Check if all values are zero
        for(l_mode = 0; l_mode<OCC_MODE_COUNT; l_mode++)
        {
            if( (0 != G_dcom_slv_inbox_rx.sys_mode_freq.table[l_mode]) )
            {
                l_all_zero = FALSE;
                break;
            }
        }

        extern data_cnfg_t * G_data_cnfg;
        if( l_all_zero == FALSE)
        {
            for(l_mode =0; l_mode<OCC_MODE_COUNT; l_mode++)
            {
                // Don't trust a frequency of 0x0000
                if( (0 != G_dcom_slv_inbox_rx.sys_mode_freq.table[l_mode]) )
                {
                    if(G_sysConfigData.sys_mode_freq.table[l_mode]
                            != G_dcom_slv_inbox_rx.sys_mode_freq.table[l_mode])
                    {
                        TRAC_INFO("New Frequency for Mode %d: Old: %d MHz -> New: %d MHz",l_mode,
                                G_sysConfigData.sys_mode_freq.table[l_mode],
                                G_dcom_slv_inbox_rx.sys_mode_freq.table[l_mode]);

                        // Update mode frequency
                        G_sysConfigData.sys_mode_freq.table[l_mode] =
                            G_dcom_slv_inbox_rx.sys_mode_freq.table[l_mode];

                        l_change = TRUE;
                    }
                }
            }

            if(l_change)
            {
                // Update "update count" for debug purposes
                G_sysConfigData.sys_mode_freq.update_count =
                    G_dcom_slv_inbox_rx.sys_mode_freq.update_count;

                // Change Data Request Mask to indicate we got this data
                extern data_cnfg_t * G_data_cnfg;
                G_data_cnfg->data_mask |= DATA_MASK_FREQ_PRESENT;

                // Notify AMEC that the frequencies have changed
                AMEC_data_change(DATA_MASK_FREQ_PRESENT);
            }
        }
        else
        {
            // Clear Data Request Mask and data
            G_data_cnfg->data_mask &= (~DATA_MASK_FREQ_PRESENT);
            memset(&G_sysConfigData.sys_mode_freq.table[0], 0, sizeof(G_sysConfigData.sys_mode_freq.table));
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
            g_amec->slv_dps_param_overwrite = TRUE;
        }
        else
        {
            g_amec->slv_dps_param_overwrite = FALSE;
        }
    }

    // Copy soft frequency boundaries sent by Master OCC
    g_amec->part_config.part_list[0].soft_fmin = G_dcom_slv_inbox_rx.soft_fmin;
    g_amec->part_config.part_list[0].soft_fmax = G_dcom_slv_inbox_rx.soft_fmax;

    // Update DCM Sync var that will be used in thread
    proc_gpsm_dcm_sync_update_from_mbox(&G_dcom_slv_inbox_rx.dcm_sync);

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
    uint64_t     l_pba_errpt2 = 0;
    uint64_t     l_pba_fir = 0;
    pba_xshcsn_t l_pba_shcs;
    pba_xcfg_t   l_pbax_cfg;

    SsxAddress   l_pba_shcs_addr = PBA_XSHCS0;
    int          l_pba_errpt_scom_rc = 0;
    int          l_pba_fir_scom_rc = 0;
    errlHndl_t   l_err = NULL;
    static bool  L_pba_reset_logged[2] = {FALSE};

    // Skip if waiting for a reset, no sense in trying to recover when going to be reset anyway
    if((TRUE == isSafeStateRequested()) || (CURRENT_STATE() == OCC_STATE_SAFE))
       return;

    if(i_queue == 1)
       l_pba_shcs_addr = PBA_XSHCS1;

    l_pba_shcs.words.high_order = in32(l_pba_shcs_addr);

    TRAC_ERR("dcom_pbax_error_handler: Start error handler for queue %d PBA_XSHCS[0x%08x]",
              i_queue,
              l_pba_shcs.words.high_order);

    do
    {
       l_pba_fir_scom_rc = _getscom(PBA_FIR, &l_pba_fir, SCOM_TIMEOUT);
       if(l_pba_fir_scom_rc)
       {
          // trace scom failure and skip clearing pba fir
          TRAC_ERR("dcom_pbax_error_handler: Failure reading PBA_FIR rc[%08x]",
                    l_pba_fir_scom_rc);
       }

       l_pba_errpt_scom_rc = _getscom(PBA_ERRPT2, &l_pba_errpt2, SCOM_TIMEOUT);
       if(l_pba_errpt_scom_rc)
       {
          // trace scom failure
          TRAC_ERR("dcom_pbax_error_handler: Failure reading PBA_ERRPT2 rc[%08x]",
                    l_pba_errpt_scom_rc);
       }

       TRAC_ERR("dcom_pbax_error_handler: PBA_FIR[0x%08x%08x] PBA_ERRPT2[0x%08x%08x]",
                 (uint32_t)(l_pba_fir >> 32),
                 (uint32_t)(l_pba_fir & 0x00000000ffffffffull),
                 (uint32_t)(l_pba_errpt2 >> 32),
                 (uint32_t)(l_pba_errpt2 & 0x00000000ffffffffull));

       // reset queue and clear the error condition to allow future pbax reads

       // 1. Disable the pushQ and reset the read & write pointer by writing 0 to push_enable (bit 31)
       l_pba_shcs.fields.push_enable = 0;
       out32(l_pba_shcs_addr, l_pba_shcs.words.high_order);

       // 2. Clear the error status by setting rcv_reset (bit 3) in the PBAX CFG register
       l_pbax_cfg.value = in64(PBA_XCFG);
       l_pbax_cfg.fields.rcv_reset = 1;
       out64(PBA_XCFG, l_pbax_cfg.value);


       // 3.  Clear bits 32 and 35 in PBAFIR and zero out PBAERRPT2
       // only clear PBAFIR if it was successfully read
       if(!l_pba_fir_scom_rc)
       {
          l_pba_fir &= (~0x0000000090000000);
          l_pba_fir_scom_rc = _putscom(PBA_FIR, l_pba_fir, SCOM_TIMEOUT);
          if(l_pba_fir_scom_rc)
          {
             TRAC_ERR("dcom_pbax_error_handler: Failure writing 0x%08x%08x to PBA_FIR rc[%08x]",
                       (uint32_t)(l_pba_fir >> 32),
                       (uint32_t)(l_pba_fir & 0x00000000ffffffffull),
                       l_pba_fir_scom_rc);
          }

       }

       // to clear error report 2 write to error report 0
       l_pba_errpt_scom_rc = _putscom(PBA_ERRPT0, 0, SCOM_TIMEOUT);
       if(l_pba_errpt_scom_rc)
       {
          TRAC_ERR("dcom_pbax_error_handler: Failure writing 0 to PBA_ERRPT0 rc[%08x]",
                    l_pba_errpt_scom_rc);
       }

       // 4.  Reenable the pushQ (set push_enable bit 31)
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

