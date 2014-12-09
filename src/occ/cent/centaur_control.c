/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/cent/centaur_control.c $                              */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* COPYRIGHT International Business Machines Corp. 2011,2014              */
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

//*************************************************************************
// Includes
//*************************************************************************
#include "centaur_control.h"
#include "centaur_data.h"
#include "pgp_async.h"
#include "threadSch.h"
#include "pmc_register_addresses.h"
#include "centaur_data_service_codes.h"
#include "occ_service_codes.h"
#include "errl.h"
#include "trac.h"
#include "rtls.h"
#include "apss.h"
#include "state.h"
#include "gpe_scom.h"
#include "centaur_firmware_registers.h"
#include "centaur_register_addresses.h"
#include "amec_sys.h"

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************

// Used for Centaur Initialization of Registers
typedef enum
{
  NM_THROTTLE_MBA01       = 0,
  NM_THROTTLE_MBA23       = 1,
  MBS_THROTTLE_SYNC       = 2,
  NUM_CENT_THROTTLE_SCOMS = 3,
} eCentaurThrottleRegs;


//*************************************************************************
// Structures
//*************************************************************************

//*************************************************************************
// Globals
//*************************************************************************

//Pore flex request for the GPE job that is used for centaur init.
PoreFlex G_centaur_control_pore_req;
GPE_BUFFER(GpeScomParms G_centaur_control_reg_parms);
GPE_BUFFER(scomList_t G_centaurThrottle[NUM_CENT_THROTTLE_SCOMS]);

//Centaur structures used for task data pointers.
centaur_control_task_t G_centaur_control_task =
{
    .startCentaur           = 0,     // First Centaur
    .prevCentaur            = 7,     // Previous Centaur written to
    .curCentaur             = 0,     // Current Centaur
    .endCentaur             = 7,     // Last Centaur
    .traceThresholdFlags    = 0,     // Trace Throttle Flags
};

// Per-MBA N/M Throttle Register "M & N" Values
centaur_throttle_t G_centaurThrottleLimits[MAX_NUM_CENTAURS][NUM_MBAS_PER_CENTAUR];

//bitmap of configured MBA's (2 per centaur, lsb is centaur 0/mba 0)
uint16_t G_configured_mbas = 0;

//*************************************************************************
// Function Prototypes
//*************************************************************************

//*************************************************************************
// Functions
//*************************************************************************

//////////////////////////
// Function Specification
//
// Name: centaurThrottle_convert2Numerator
//
// Description: Converts a throttle percentage into an 'N' value that can
//              be written to the hardware.
//
//
// Thread: RTL
//
// End Function Specification
uint16_t centaurThrottle_convert2Numerator(uint16_t i_throttle, uint8_t i_cent, uint8_t i_mba)
{
#define CENTAUR_THROTTLE_100_PERCENT_VALUE 1000

    uint32_t l_nvalue = 0;
    centaur_throttle_t* l_mba = &G_centaurThrottleLimits[i_cent][i_mba];

    if(MBA_CONFIGURED(i_cent, i_mba))
    {
        // Convert the throttle ( actually in units of 0.1 %) to a "N" value
        l_nvalue = (l_mba->max_n_per_mba * i_throttle) /
              CENTAUR_THROTTLE_100_PERCENT_VALUE;

        //Clip to per-mba min and max values
        if(l_nvalue < l_mba->min_n_per_mba)
        {
            l_nvalue = l_mba->min_n_per_mba;
        }
        if(l_nvalue > l_mba->max_n_per_mba)
        {
            l_nvalue = l_mba->max_n_per_mba;
        }
    }

    return (uint16_t)l_nvalue;
}

//////////////////////////
// Function Specification
//
// Name: cent_update_nlimits
//
// Description: Updates the memory throttle settings for;
//              1) new settings from FSP
//              2) change to/from TURBO or DPS mode
//              3) enter/exit oversubscription
//
//
// Thread: RTL
//
// End Function Specification
#define CENT_TRACE_THROTTLE_DELAY 8
void cent_update_nlimits(uint32_t i_cent)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    static uint32_t L_trace_throttle_count = 0;
    uint16_t l_mba01_mba_maxn, l_mba01_chip_maxn, l_mba23_mba_maxn, l_mba23_chip_maxn;
    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    do
    {
        centaur_throttle_t* l_active_limits01 =
            &G_centaurThrottleLimits[i_cent][0];
        centaur_throttle_t* l_active_limits23 =
            &G_centaurThrottleLimits[i_cent][1];
        mem_throt_config_data_t* l_state_limits01 =
            &G_sysConfigData.mem_throt_limits[i_cent][0];
        mem_throt_config_data_t* l_state_limits23 =
            &G_sysConfigData.mem_throt_limits[i_cent][1];

        //Minimum N value is not state dependent
        l_active_limits01->min_n_per_mba = l_state_limits01->min_ot_n_per_mba;
        l_active_limits23->min_n_per_mba = l_state_limits23->min_ot_n_per_mba;

        //oversubscription?
        if(AMEC_INTF_GET_OVERSUBSCRIPTION())
        {
            l_mba01_mba_maxn = l_state_limits01->ovs_n_per_mba;
            l_mba01_chip_maxn = l_state_limits01->ovs_n_per_chip;
            l_mba23_mba_maxn = l_state_limits23->ovs_n_per_mba;
            l_mba23_chip_maxn = l_state_limits23->ovs_n_per_chip;
        }
        else if(CURRENT_MODE() == OCC_MODE_NOMINAL)
        {
            l_mba01_mba_maxn = l_state_limits01->nom_n_per_mba;
            l_mba01_chip_maxn = l_state_limits01->nom_n_per_chip;
            l_mba23_mba_maxn = l_state_limits23->nom_n_per_mba;
            l_mba23_chip_maxn = l_state_limits23->nom_n_per_chip;
        }
        else //DPS, TURBO, FFO, and SPS modes will use these settings
        {
            l_mba01_mba_maxn = l_state_limits01->turbo_n_per_mba;
            l_mba01_chip_maxn = l_state_limits01->turbo_n_per_chip;
            l_mba23_mba_maxn = l_state_limits23->turbo_n_per_mba;
            l_mba23_chip_maxn = l_state_limits23->turbo_n_per_chip;
        }

        l_active_limits01->max_n_per_chip = l_mba01_chip_maxn;
        l_active_limits23->max_n_per_chip = l_mba23_chip_maxn;

        //Trace when the MBA max N value changes
        if((l_mba01_mba_maxn != l_active_limits01->max_n_per_mba) ||
           (l_mba23_mba_maxn != l_active_limits23->max_n_per_mba))
        {
            l_active_limits01->max_n_per_mba = l_mba01_mba_maxn;
            l_active_limits23->max_n_per_mba = l_mba23_mba_maxn;

            //Don't trace every MBA changing, just one
            if(!L_trace_throttle_count)
            {
                L_trace_throttle_count = CENT_TRACE_THROTTLE_DELAY;
                TRAC_IMP("New MBA throttle max|min N values: mba01[0x%08x], mba23[0x%08x]",
                        (uint32_t)((l_mba01_mba_maxn << 16) | l_active_limits01->min_n_per_mba),
                        (uint32_t)((l_mba23_mba_maxn << 16) | l_active_limits23->min_n_per_mba));
                break;
            }
        }

        if(L_trace_throttle_count)
        {
            L_trace_throttle_count--;
        }

    }while(0);
}


// Function Specification
//
// Name: task_centaur_control
//
// Description: Collect centaur data. The task is used for centaur data
//              collection
//
// End Function Specification
#define CENTAUR_CONTROL_SCOM_TIMEOUT 16  //wait up to 16 ticks before logging timeout failure
void task_centaur_control( task_t * i_task )
{
    errlHndl_t            l_err     = NULL;    // Error handler
    int                   rc        = 0;       // Return code
    uint32_t              l_cent;
    amec_centaur_t        *l_cent_ptr = NULL;
    static uint8_t        L_scom_timeout[MAX_NUM_CENTAURS] = {0}; //track # of consecutive failures
    static bool           L_gpe_scheduled = FALSE;
    static uint8_t        L_gpe_fail_logged = 0;
    static bool           L_gpe_idle_traced = FALSE;
    static bool           L_gpe_had_1_tick = FALSE;

    // Pointer to the task data structure
    centaur_control_task_t * l_centControlTask =
            (centaur_control_task_t *) i_task->data_ptr;


    // Pointer to parameter field for GPE request
    GpeScomParms * l_parms =
          (GpeScomParms *)(l_centControlTask->gpe_req.parameter);

    do
    {
        l_cent = l_centControlTask->curCentaur;
        l_cent_ptr = &g_amec->proc[0].memctl[l_cent].centaur;

        //First, check to see if the previous GPE request still running
        //A request is considered idle if it is not attached to any of the
        //asynchronous request queues
        if( !(async_request_is_idle(&l_centControlTask->gpe_req.request)) )
        {
            L_scom_timeout[l_cent]++;
            //This can happen due to variability in when the task runs
            if(!L_gpe_idle_traced && L_gpe_had_1_tick)
            {
                TRAC_INFO("task_centaur_control: GPE is still running. cent[%d]", l_cent);
                l_centControlTask->traceThresholdFlags |= CENTAUR_CONTROL_GPE_STILL_RUNNING;
                L_gpe_idle_traced = TRUE;
            }
            L_gpe_had_1_tick = TRUE;
            break;
        }
        else
        {
            //Request is idle
            L_gpe_had_1_tick = FALSE;
            if(L_gpe_idle_traced)
            {
                TRAC_INFO("task_centaur_control: GPE completed. cent[%d]", l_cent);
                L_gpe_idle_traced = FALSE;
            }
        }

        //check scom status
        if(L_gpe_scheduled)
        {
            if(!async_request_completed(&l_centControlTask->gpe_req.request) || l_parms->rc)
            {

                if(!(L_gpe_fail_logged & (CENTAUR0_PRESENT_MASK >> l_cent)))
                {
                    L_gpe_fail_logged |= CENTAUR0_PRESENT_MASK >> l_cent;
                    TRAC_ERR("task_centaur_control: gpe_scom_centaur failed. l_cent=%d rc=%x, index=0x%08x", l_cent, l_parms->rc, l_parms->errorIndex);

                    /* @
                     * @errortype
                     * @moduleid    CENT_TASK_CONTROL_MOD
                     * @reasoncode  CENT_SCOM_ERROR
                     * @userdata1   rc - Return code of scom operation
                     * @userdata2   index of scom operation that failed
                     * @userdata4   OCC_NO_EXTENDED_RC
                     * @devdesc     OCC access to centaur failed
                     */
                    l_err = createErrl(
                            CENT_TASK_CONTROL_MOD,                  // modId
                            CENT_SCOM_ERROR,                        // reasoncode
                            OCC_NO_EXTENDED_RC,                     // Extended reason code
                            ERRL_SEV_PREDICTIVE,                    // Severity
                            NULL,                                   // Trace Buf
                            DEFAULT_TRACE_SIZE,                     // Trace Size
                            l_parms->rc,                            // userdata1
                            l_parms->errorIndex                     // userdata2
                            );

                    addUsrDtlsToErrl(l_err,                                  //io_err
                            (uint8_t *) &(l_centControlTask->gpe_req.ffdc),  //i_dataPtr,
                            sizeof(PoreFfdc),                                //i_size
                            ERRL_USR_DTL_STRUCT_VERSION_1,                   //version
                            ERRL_USR_DTL_BINARY_DATA);                       //type

                    //callout the centaur
                    addCalloutToErrl(l_err,
                                     ERRL_CALLOUT_TYPE_HUID,
                                     G_sysConfigData.centaur_huids[l_cent],
                                     ERRL_CALLOUT_PRIORITY_MED);

                    //callout the processor
                    addCalloutToErrl(l_err,
                                     ERRL_CALLOUT_TYPE_HUID,
                                     G_sysConfigData.proc_huid,
                                     ERRL_CALLOUT_PRIORITY_MED);

                    commitErrl(&l_err);
                }//if(l_gpe_fail_logged & (CENTAUR0_PRESENT_MASK >> l_cent))

                //Request failed. Keep count of failures and request a reset if we reach a
                //max retry count
                L_scom_timeout[l_cent]++;
                if(L_scom_timeout[l_cent] == CENTAUR_CONTROL_SCOM_TIMEOUT)
                {
                    break;
                }

            }//if(!async_request_completed(&l_centControlTask->gpe_req.request) || l_parms->rc)
            else
            {
                //request completed successfully.  reset the timeout.
                L_scom_timeout[l_cent] = 0;
            }
        }//if(L_gpe_scheduled)

        //The previous GPE job completed. Now get ready for the next job.
        L_gpe_scheduled = FALSE;

        //Update current centaur if we didn't fail
        if ( l_cent >= l_centControlTask->endCentaur )
        {
            l_cent  = l_centControlTask->startCentaur;
        }
        else
        {
            l_cent++;
        }
        l_centControlTask->curCentaur = l_cent;
        l_cent_ptr = &g_amec->proc[0].memctl[l_cent].centaur;

        //If centaur is not present or neither MBA is configured then skip it.
        if(!CENTAUR_PRESENT(l_cent) ||
            (!MBA_CONFIGURED(l_cent, 0) && !MBA_CONFIGURED(l_cent, 1)))
        {
            break;
        }

        //update min/max settings for both MBA's according to ovs and mode
        cent_update_nlimits(l_cent);

        //calculate new N values
        centaur_mba_farb3qn_t l_mbafarbq;
        uint16_t l_mba01_n_per_mba =
            centaurThrottle_convert2Numerator(g_amec->mem_speed_request, l_cent, 0);
        uint16_t l_mba23_n_per_mba =
            centaurThrottle_convert2Numerator(g_amec->mem_speed_request, l_cent, 1);
        uint16_t l_mba01_n_per_chip = G_centaurThrottleLimits[l_cent][0].max_n_per_chip;
        uint16_t l_mba23_n_per_chip = G_centaurThrottleLimits[l_cent][1].max_n_per_chip;
        amec_cent_mem_speed_t l_mba01_speed;
        amec_cent_mem_speed_t l_mba23_speed;

        //combine chip and mba settings (16 bit) in to a single 32bit value
        l_mba01_speed.mba_n = l_mba01_n_per_mba;
        l_mba01_speed.chip_n = l_mba01_n_per_chip;
        l_mba23_speed.mba_n = l_mba23_n_per_mba;
        l_mba23_speed.chip_n = l_mba23_n_per_chip;


        // Check if the throttle value has been updated since the last
        // time we sent it.  If it has, then send a new value, otherwise
        // do nothing.
        if ( ( l_mba01_speed.word32 == l_cent_ptr->portpair[0].last_mem_speed_sent.word32 ) &&
             ( l_mba23_speed.word32 == l_cent_ptr->portpair[1].last_mem_speed_sent.word32 )
           )
        {
            break;
        }

        //TRAC_INFO("task_centaur_control: New centaur[%d] throttle values mba01[0x%08x], mba23[0x%08x], throt[%d] ",
        //        l_cent,
        //        l_mba01_speed.word32,
        //        l_mba23_speed.word32,
        //        g_amec->mem_speed_request);

        /// Set up Centuar Scom Registers - array of Scoms
        ///   [0]:  N/M Throttle MBA01
        ///   [1]:  N/M Throttle MBA23
        ///   [2]:  MB SYNC

        //only write to MBA01 if configured
        if(MBA_CONFIGURED(l_cent, 0))
        {
            /// [0]: Set up N/M throttle MBA01
            G_centaurThrottle[NM_THROTTLE_MBA01].commandType = GPE_SCOM_RMW;
            G_centaurThrottle[NM_THROTTLE_MBA01].instanceNumber = l_cent;
            // Set up value to be written
            l_mbafarbq.fields.cfg_nm_n_per_mba = l_mba01_n_per_mba;
            l_mbafarbq.fields.cfg_nm_n_per_chip = l_mba01_n_per_chip;
            G_centaurThrottle[NM_THROTTLE_MBA01].data = l_mbafarbq.value;
        }
        else
        {
            G_centaurThrottle[NM_THROTTLE_MBA01].commandType = GPE_SCOM_NOP;
        }

        //only write to MBA23 if configured
        if(MBA_CONFIGURED(l_cent, 1))
        {
            /// [1]: Set up N/M throttle MBA23
            G_centaurThrottle[NM_THROTTLE_MBA23].commandType = GPE_SCOM_RMW;
            G_centaurThrottle[NM_THROTTLE_MBA23].instanceNumber = l_cent;
            // Set up value to be written
            l_mbafarbq.fields.cfg_nm_n_per_mba = l_mba23_n_per_mba;
            l_mbafarbq.fields.cfg_nm_n_per_chip = l_mba23_n_per_chip;
            G_centaurThrottle[NM_THROTTLE_MBA23].data = l_mbafarbq.value;
        }
        else
        {
            G_centaurThrottle[NM_THROTTLE_MBA23].commandType = GPE_SCOM_NOP;
        }


        /// [2]: Set up the SYNC
        ///
        ///    0:7  select mask of MCS units
        ///    8:15 select the sync type (12 = N/M throttle)
        ///   57:63 must be zeros to address DW0 in cacheline
        //G_centaurThrottle[MBS_THROTTLE_SYNC].commandType = GPE_SCOM_NOP;
        G_centaurThrottle[MBS_THROTTLE_SYNC].commandType = GPE_SCOM_CENTAUR_SYNC_ALL;
        G_centaurThrottle[MBS_THROTTLE_SYNC].data = CENTAUR_RESET_N_M_THROTTLE_COUNTER_SYNC |
                                                    CENTAUR_MYSTERY_SYNC; //This is the "PC" sync bit

        /// Set up GPE parameters
        l_parms->scomList     = (uint32_t) (&G_centaurThrottle);
        l_parms->entries      = 3;
        l_parms->options      = 0;
        l_parms->rc           = 0;
        l_parms->errorIndex   = 0;

        // Update the last sent throttle value, this will get
        // cleared if the GPE does not complete successfully.
        l_cent_ptr->portpair[0].last_mem_speed_sent.word32 = l_mba01_speed.word32;
        l_cent_ptr->portpair[1].last_mem_speed_sent.word32 = l_mba23_speed.word32;


        // Pore flex schedule gpe_scom_centaur
        // Check pore_flex_schedule return code if error
        // then request OCC reset.
        rc = pore_flex_schedule( &(l_centControlTask->gpe_req) );
        if( rc )
        {
            //Error in schedule gpe get centaur data
            TRAC_ERR("task_centaur_control: Failed to schedule gpe rc=%x", rc);

            /* @
             * @errortype
             * @moduleid    CENT_TASK_CONTROL_MOD
             * @reasoncode  SSX_GENERIC_FAILURE
             * @userdata1   rc - Return code of failing function
             * @userdata2   0
             * @userdata4   ERC_CENTAUR_PORE_FLEX_SCHEDULE_FAILURE
             * @devdesc     OCC Failed to schedule a GPE job for centaur
             */
            l_err = createErrl(
                    CENT_TASK_CONTROL_MOD,                  // modId
                    SSX_GENERIC_FAILURE,                    // reasoncode
                    ERC_CENTAUR_PORE_FLEX_SCHEDULE_FAILURE, // Extended reason code
                    ERRL_SEV_UNRECOVERABLE,                 // Severity
                    NULL,                                   // Trace Buf
                    DEFAULT_TRACE_SIZE,                     // Trace Size
                    rc,                                     // userdata1
                    l_parms->rc                             // userdata2
                    );

            addUsrDtlsToErrl(l_err,                               //io_err
                    (uint8_t *) &(l_centControlTask->gpe_req.ffdc),  //i_dataPtr,
                    sizeof(PoreFfdc),                                //i_size
                    ERRL_USR_DTL_STRUCT_VERSION_1,                   //version
                    ERRL_USR_DTL_BINARY_DATA);                       //type

            REQUEST_RESET(l_err);   //This will add a firmware callout for us
            break;
        }

        L_gpe_scheduled = TRUE;
    } while(0);

    if(L_scom_timeout[l_cent] == CENTAUR_CONTROL_SCOM_TIMEOUT)
    {
        TRAC_ERR("task_centaur_control: Timeout scomming centaur[%d]", l_cent);
        /* @
         * @errortype
         * @moduleid    CENT_TASK_CONTROL_MOD
         * @reasoncode  INTERNAL_FAILURE
         * @userdata1   centaur number
         * @userdata2   0
         * @userdata4   OCC_NO_EXTENDED_RC
         * @devdesc     Timed out trying to set the memory throttle settings
         *              throttle settings.
         */
        l_err = createErrl(
                CENT_TASK_CONTROL_MOD,                  // modId
                INTERNAL_FAILURE,                       // reasoncode
                OCC_NO_EXTENDED_RC,                     // Extended reason code
                ERRL_SEV_PREDICTIVE,                    // Severity
                NULL,                                   // Trace Buf
                DEFAULT_TRACE_SIZE,                     // Trace Size
                l_cent,                                 // userdata1
                0                                       // userdata2
                );

        addUsrDtlsToErrl(l_err,                               //io_err
                (uint8_t *) &(l_centControlTask->gpe_req.ffdc),  //i_dataPtr,
                sizeof(PoreFfdc),                                //i_size
                ERRL_USR_DTL_STRUCT_VERSION_1,                   //version
                ERRL_USR_DTL_BINARY_DATA);                       //type

        //callout the centaur
        addCalloutToErrl(l_err,
                         ERRL_CALLOUT_TYPE_HUID,
                         G_sysConfigData.centaur_huids[l_cent],
                         ERRL_CALLOUT_PRIORITY_MED);

        //callout the processor
        addCalloutToErrl(l_err,
                         ERRL_CALLOUT_TYPE_HUID,
                         G_sysConfigData.proc_huid,
                         ERRL_CALLOUT_PRIORITY_MED);

        REQUEST_RESET(l_err);
    }

    return;
}


// Function Specification
//
// Name: centaur_control_init
//
// Description:  Do one-time setup for centaur control task
//
//
//
// Precondition:  We must have determined the present centaurs already
//
// End Function Specification
void centaur_control_init( void )
{
    errlHndl_t l_err       = NULL;
    int        l_rc_gpe    = 0;
    centaur_mba_farb3qn_t l_mbafarbq;

    do
    {
        //initialize the active throttle limits
        memset(G_centaurThrottleLimits, 0, sizeof(G_centaurThrottleLimits));

        //Do one-time setup items for the task here.

        //--------------------------------------------------
        // Set up Centuar Regs
        //   [0]:  for MBAFARBQ0
        //   [1]:  for MBAFARBQ1
        //--------------------------------------------------
        //
        G_centaurThrottle[NM_THROTTLE_MBA01].scom = CENTAUR_MBA_FARB3Q0;
        G_centaurThrottle[NM_THROTTLE_MBA23].scom = CENTAUR_MBA_FARB3Q1;
        l_mbafarbq.value = 0;
        l_mbafarbq.fields.cfg_nm_n_per_mba = -1; //all bits set
        l_mbafarbq.fields.cfg_nm_n_per_chip = -1; //all bits set
        G_centaurThrottle[NM_THROTTLE_MBA01].mask = l_mbafarbq.value;
        G_centaurThrottle[NM_THROTTLE_MBA23].mask = l_mbafarbq.value;

        // Set up GPE parameters
        G_centaur_control_reg_parms.rc         = 0;
        G_centaur_control_reg_parms.entries    = 0;
        G_centaur_control_reg_parms.scomList   = (uint32_t) (&G_centaurThrottle[0]);
        G_centaur_control_reg_parms.options    = 0;
        G_centaur_control_reg_parms.errorIndex = 0;

        //--------------------------------------------------
        // Initializes PoreFlex for Centaur Control Task, but
        // doesn't actually run anything until RTL
        //--------------------------------------------------
        l_rc_gpe = pore_flex_create(
                &G_centaur_control_task.gpe_req,           // gpe_req for the task
                &G_pore_gpe1_queue,                        // queue
                gpe_scom_centaur,                          // entry point
                (uint32_t) &G_centaur_control_reg_parms,   // parm for the task
                SSX_WAIT_FOREVER,                          //
                NULL,                                      // callback
                NULL,                                      // callback argument
                0 );                                       // options
        if(l_rc_gpe)
        {
            break;
        }

    }
    while(0);


    if( l_rc_gpe )
    {
        //If fail to create pore flex object then there is a problem.
        TRAC_ERR("centaur_control_init: Failed to initialize centaur control task [l_rc_gpe=0x%x]", l_rc_gpe);

        /* @
         * @errortype
         * @moduleid    CENTAUR_INIT_MOD
         * @reasoncode  SSX_GENERIC_FAILURE
         * @userdata1   l_rc_gpe  - Return code of failing function
         * @userdata2   0
         * @userdata4   ERC_CENTAUR_PORE_FLEX_CREATE_FAILURE
         * @devdesc     Failed to initialize GPE routine
         */
        l_err = createErrl(
                CENTAUR_INIT_MOD,                           //modId
                SSX_GENERIC_FAILURE,                        //reasoncode
                ERC_CENTAUR_PORE_FLEX_CREATE_FAILURE,       //Extended reason code
                ERRL_SEV_PREDICTIVE,                        //Severity
                NULL,                                       //Trace Buf
                DEFAULT_TRACE_SIZE,                         //Trace Size
                l_rc_gpe,                                   //userdata1
                0                                           //userdata2
                );

        addUsrDtlsToErrl(l_err,                                           //io_err
                         (uint8_t *) &G_centaur_control_pore_req.ffdc,    //i_dataPtr,
                         sizeof(PoreFfdc),                                //i_size
                         ERRL_USR_DTL_STRUCT_VERSION_1,                   //version
                         ERRL_USR_DTL_BINARY_DATA);                       //type

        REQUEST_RESET(l_err);
    }

    return;
}




