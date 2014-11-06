/******************************************************************************
// @file dcomSlaveTx.c
// @brief Slave OCC to Master OCC communication handler
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section dcomSlaveTx.c DCOMSLAVETX.C
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *   @00                abagepa   10/10/2011  Created
 *   @th002             thallet   11/01/2011  Misc Changes for Nov 1st Milestone
 *   @th005             thallet   11/23/2011  Added STATIC_ASSERT checks
 *   @01                tapiar    11/12/2011  Stage 3 updates
 *   @rc003             rickylie  02/03/2012  Verify & Clean Up OCC Headers & Comments
 *   @nh001             neilhsu   05/23/2012  Add missing error log tags
 *   @th010             thallet   07/11/2012  Pstate Enablement`
 *   @th022             thallet   10/08/2012  Changes for OCC Comm
 *   @rt001  897459     tapiar    08/19/2013  Upd: save active node pcap from doorbell
 *   @gm014  907707     milesg    12/05/2013  don't panic on pbax_send timeout failures
 *   @rt004  908817     tapiar    12/11/2013  Save of valid pcap field so master can use it
 *   @sb003  908290     sbroyles  12/18/2013  Test BCE request states
 *   @sb013  911625     sbroyles  01/15/2014  Fix to 908290 changes
 *   @wb001  919163     wilbryan  03/06/2014  Updating error call outs, descriptions, and severities
 *  @endverbatim
 *
 *///*************************************************************************/

#ifndef _DCOMSLAVETX_C
#define _DCOMSLAVETX_C

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
#include <amec_sys.h>   // @rt001a

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

// SSX Block Copy Request for the Slave Outbox Transmit Queue
BceRequest G_slv_outbox_tx_pba_request;

// SSX PBAX Request for Unicast PBAX Queue (Slave Doorbell to Master)
PbaxRequest G_pbax_unicast_request;

// Used by the slave to house the doorbell data that is sent in
// the slave unicast doorbell, stating that it put slave outbox in main memory.
// *
dcom_slv_outbox_doorbell_t G_dcom_slv_outbox_doorbell_tx;

// Make sure that the Slave Outbox TX Buffer is 1kB, otherwise cause
// error on the compile.
STATIC_ASSERT(  (NUM_BYTES_IN_SLAVE_OUTBOX != (sizeof(G_dcom_slv_outbox_tx)))  );

//*************************************************************************
// Function Prototypes
//*************************************************************************

//*************************************************************************
// Functions
//*************************************************************************

// Function Specification
//
// Name: dcom_build_slv_outbox
//
// Description: The purpose of this function is to fill out the Sensor Data into the
//              Slave Outbox Structures for transfer to the Master.
//              Build the slave outboxes so slave can send to master
//
// Flow:  09/21/11    FN=dcom_build_slv_outbox
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

    G_dcom_slv_outbox_tx.counter++;   // @th002

    // @01
    // call dcom_build_occfw_msg
    dcom_build_occfw_msg( SLAVE_OUTBOX );

    // Create message that will be sent to DCM peer
    dcom_build_dcm_sync_msg( SLAVE_OUTBOX );   // @th010

    l_addr_of_slv_outbox_in_main_mem = dcom_which_buffer_slv_outbox();

    l_addr_of_slv_outbox_in_main_mem += G_pob_id.chip_id*sizeof(dcom_slv_outbox_t);

    G_dcom_slv_outbox_doorbell_tx.pob_id = G_pob_id;
    G_dcom_slv_outbox_doorbell_tx.pcap_valid = g_amec->pcap_valid; //@rt004a
    G_dcom_slv_outbox_doorbell_tx.active_node_pcap = g_amec->pcap.active_node_pcap; //@rt001a
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
// Flow:  09/21/11    FN=dcom_which_buffer_slv_outbox
//
// End Function Specification

uint32_t dcom_which_buffer_slv_outbox(void)
{
    //Locals
    uint32_t l_mem_address = ADDR_SLAVE_OUTBOX_MAIN_MEM_PONG;

    // switch back and forth based on tick
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
// Flow:  09/20/11    FN=dcom_calc_slv_outbox_addr
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
// Flow:  09/21/11    FN=task_dcom_tx_slv_outbox
//
// Task Flags:  RTL_FLAG_NONMSTR, RTL_FLAG_MSTR, RTL_FLAG_OBS, RTL_FLAG_ACTIVE,
//              RTL_FLAG_NOAPSS, RTL_FLAG_RUN, RTL_FLAG_MSTR_READY
//
// End Function Specification
void task_dcom_tx_slv_outbox( task_t *i_self)
{
    static bool l_error = FALSE;
    uint32_t    l_orc = OCC_SUCCESS_REASON_CODE;
    uint32_t    l_orc_ext = OCC_NO_EXTENDED_RC;     // @nh001a
    // @sb003 Use a static local bool to track whether the BCE request used
    // here has ever been successfully created at least once
    static bool L_bce_slv_outbox_tx_request_created_once = FALSE;

    DCOM_DBG("3. TX Slave Outboxes\n");

    do
    {
        // build/setup outbox
        uint32_t l_addr_in_mem = dcom_build_slv_outbox();
        uint32_t l_ssxrc = 0;

        // @sb003
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
        // else {// This case can't happen, ignore it.}

        // @sb003 Only proceed if the BCE request state checked out
        if (l_proceed_with_request_and_schedule)
        {
            // set up outbox copy request
            l_ssxrc = bce_request_create(
                            &G_slv_outbox_tx_pba_request,       // block copy object
                            &G_pba_bcue_queue,                  // mainstore to sram copy engine
                            l_addr_in_mem,                      // mainstore address
                            (uint32_t) &G_dcom_slv_outbox_tx,   // sram starting address
                            sizeof(G_dcom_slv_outbox_tx),       // size of copy
                            SSX_WAIT_FOREVER,                   // no timeout
                            (AsyncRequestCallback)dcom_tx_slv_outbox_doorbell, // call back
                            NULL,                               // call back arguments
                            ASYNC_CALLBACK_IMMEDIATE            // callback mask
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
                l_orc = SSX_GENERIC_FAILURE;                    // @nh001c
                l_orc_ext = ERC_BCE_REQUEST_CREATE_FAILURE;     // @nh001a
                break;
            }

            // @sb003 Request created at least once
            L_bce_slv_outbox_tx_request_created_once = TRUE; // @sb013
            l_ssxrc = bce_request_schedule(&G_slv_outbox_tx_pba_request);   // actual copying

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
                l_orc = SSX_GENERIC_FAILURE;                    // @nh001c
                l_orc_ext = ERC_BCE_REQUEST_SCHEDULE_FAILURE;   // @nh001a
                break;
            }
        }

    } while (0);


    if ( l_orc != OCC_SUCCESS_REASON_CODE && l_error == FALSE)
    {
        // create and commit error
        // see return code doxygen tags for error description
        errlHndl_t  l_errl = createErrl(
                    DCOM_MID_TASK_TX_SLV_OUTBOX,    //modId
                    l_orc,                          //reasoncode    // @nh001c
                    l_orc_ext,                      //Extended reason code
                    ERRL_SEV_UNRECOVERABLE,         //Severity
                    NULL,                           //Trace Buf
                    DEFAULT_TRACE_SIZE,             //Trace Size
                    0,                              //userdata1
                    0                               //userdata2
                    );
                    
        // @wb001 -- Callout firmware
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
// Flow:  09/21/11    FN=dcom_tx_slv_outbox_doorbell
//
// End Function Specification
void dcom_tx_slv_outbox_doorbell( void )
{
    static bool l_error = FALSE;
    int         l_pbarc = 0;
    uint64_t    l_tmp =0;

    // save into temp
    memcpy( &l_tmp, &G_dcom_slv_outbox_doorbell_tx, sizeof(dcom_slv_outbox_doorbell_t));

    // send unicast doorbell
    l_pbarc = _pbax_send(    //gm014
                &G_pbax_unicast_target,
                l_tmp,
                SSX_MICROSECONDS(15));

    if ( l_pbarc != 0 && l_error == FALSE )
    {
        //failure occurred
        //This is running in a critical interrupt context.  Tracing not allowed!
        //TRAC_ERR("PBAX Send Failure in transimitting unicast doorbell - RC[%08X]", l_pbarc);

        l_error = TRUE;

        // create and commit error
#if 0    //try again on the next tick
        /* @
         * @errortype
         * @moduleid    DCOM_MID_SLV_OUTBOX_TX_DOORBELL
         * @reasoncode  SSX_GENERIC_FAILURE
         * @userdata1   N/A
         * @userdata4   OCC_NO_EXTENDED_RC
         * @devdesc     SSX PBAX related failure
         */
        errlHndl_t  l_errl = createErrl(
                    DCOM_MID_SLV_OUTBOX_TX_DOORBELL,    //modId
                    SSX_GENERIC_FAILURE,                //reasoncode    // @nh001c
                    OCC_NO_EXTENDED_RC,                 //Extended reason code
                    ERRL_SEV_UNRECOVERABLE,             //Severity
                    NULL,                               //Trace Buf
                    DEFAULT_TRACE_SIZE,                 //Trace Size
                    0,                                  //userdata1
                    0                                   //userdata2
                    );

        commitErrl( &l_errl );

        //TODO request a reset
#endif
    }
}


#endif //_DCOMSLAVETOMASTER_C

