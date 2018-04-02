/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/cent/centaur_control.c $                          */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2014,2017                        */
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

//*************************************************************************/
// Includes
//*************************************************************************/
#include <occ_common.h>
#include "gpe_export.h"
#include "centaur_control.h"
#include "centaur_data.h"
#include "occhw_async.h"
#include "threadSch.h"
#include "centaur_data_service_codes.h"
#include "occ_service_codes.h"
#include "errl.h"
#include "trac.h"
#include "rtls.h"
#include "apss.h"
#include "state.h"
#include "centaur_structs.h"
#include "centaur_firmware_registers.h"
#include "centaur_register_addresses.h"
#include "amec_sys.h"
#include "memory.h"

//*************************************************************************/
// Externs
//*************************************************************************/

//*************************************************************************/
// Macros
//*************************************************************************/

//*************************************************************************/
// Defines/Enums
//*************************************************************************/

// Used for Centaur Initialization of Registers
typedef enum
{
  NM_THROTTLE_MBA01       = 0,
  NM_THROTTLE_MBA23       = 1,
  NUM_CENT_THROTTLE_SCOMS = 2,
} eCentaurThrottleRegs;


//*************************************************************************/
// Structures
//*************************************************************************/

//*************************************************************************/
// Globals
//*************************************************************************/

//Pore flex request for the GPE job that is used for centaur init.
GpeRequest G_centaur_control_request;

// @see CentaurScomParms in  centaur_structs.h
GPE_BUFFER(CentaurScomParms_t G_centaur_control_reg_parms);
// scomList_t  @see centaur_configuration.h
GPE_BUFFER(scomList_t G_centaurThrottle[NUM_CENT_THROTTLE_SCOMS]);

//bitmap of configured MBA's (2 per centaur, lsb is centaur 0/mba 0)
//same variable used for tracking bit maps of rdimms (4 per mc, 2 mc pairs)
uint16_t G_configured_mbas = 0;

//*************************************************************************/
// Function Prototypes
//*************************************************************************/

//*************************************************************************/
// Functions
//*************************************************************************/

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
    memory_throttle_t* l_mba = &G_memoryThrottleLimits[i_cent][i_mba];

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
        memory_throttle_t* l_active_limits01 =
            &G_memoryThrottleLimits[i_cent][0];
        memory_throttle_t* l_active_limits23 =
            &G_memoryThrottleLimits[i_cent][1];

        mem_throt_config_data_t* l_state_limits01 =
            &G_sysConfigData.mem_throt_limits[i_cent][0];
        mem_throt_config_data_t* l_state_limits23 =
            &G_sysConfigData.mem_throt_limits[i_cent][1];

        //Minimum N value is not state dependent
        l_active_limits01->min_n_per_mba = l_state_limits01->min_n_per_mba;
        l_active_limits23->min_n_per_mba = l_state_limits23->min_n_per_mba;

        //Power Capping memory?
        if(g_amec->pcap.active_mem_level == 1)
        {
            l_mba01_mba_maxn = l_state_limits01->pcap_n_per_mba;
            l_mba01_chip_maxn = l_state_limits01->pcap_n_per_chip;
            l_mba23_mba_maxn = l_state_limits23->pcap_n_per_mba;
            l_mba23_chip_maxn = l_state_limits23->pcap_n_per_chip;
        }
        else if(CURRENT_MODE() == OCC_MODE_NOMINAL)
        {
            l_mba01_mba_maxn = l_state_limits01->nom_n_per_mba;
            l_mba01_chip_maxn = l_state_limits01->nom_n_per_chip;
            l_mba23_mba_maxn = l_state_limits23->nom_n_per_mba;
            l_mba23_chip_maxn = l_state_limits23->nom_n_per_chip;
        }
        else //all other modes will use turbo settings
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
// Name: centaur_control
//
// Description: Performs centaur control.
// return TRUE  settings changed HW needs to be updated.
// return FALSE settings did  not change
//
// End Function Specification
bool centaur_control( memory_control_task_t * i_memControlTask )
{
    bool        throttle_updated = TRUE;
    amec_centaur_t      *l_cent_ptr = NULL;
    int                 l_cent = i_memControlTask->curMemIndex;

    CentaurScomParms_t * l_parms =
          (CentaurScomParms_t *)(i_memControlTask->gpe_req.cmd_data);

    do
    {
        l_cent_ptr = &g_amec->proc[0].memctl[l_cent].centaur;


        //update min/max settings for both MBA's according to ovs and mode
        cent_update_nlimits(l_cent);

        //calculate new N values
        centaur_mba_farb3qn_t l_mbafarbq;
        uint16_t l_mba01_n_per_mba =
            centaurThrottle_convert2Numerator(g_amec->mem_speed_request, l_cent, 0);
        uint16_t l_mba23_n_per_mba =
            centaurThrottle_convert2Numerator(g_amec->mem_speed_request, l_cent, 1);
        uint16_t l_mba01_n_per_chip = G_memoryThrottleLimits[l_cent][0].max_n_per_chip;
        uint16_t l_mba23_n_per_chip = G_memoryThrottleLimits[l_cent][1].max_n_per_chip;
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
            throttle_updated = FALSE;
            break;
        }

        /// Set up Centuar Scom Registers - array of Scoms
        ///   [0]:  N/M Throttle MBA01
        ///   [1]:  N/M Throttle MBA23

        //only write to MBA01 if configured
        if(MBA_CONFIGURED(l_cent, 0))
        {
            /// [0]: Set up N/M throttle MBA01
            G_centaurThrottle[NM_THROTTLE_MBA01].commandType = CENTAUR_SCOM_RMW;
            G_centaurThrottle[NM_THROTTLE_MBA01].instanceNumber = l_cent;
            // Set up value to be written
            l_mbafarbq.fields.cfg_nm_n_per_mba = l_mba01_n_per_mba;
            l_mbafarbq.fields.cfg_nm_n_per_chip = l_mba01_n_per_chip;
            G_centaurThrottle[NM_THROTTLE_MBA01].data = l_mbafarbq.value;
        }
        else
        {
            G_centaurThrottle[NM_THROTTLE_MBA01].commandType = CENTAUR_SCOM_NOP;
        }

        //only write to MBA23 if configured
        if(MBA_CONFIGURED(l_cent, 1))
        {
            /// [1]: Set up N/M throttle MBA23
            G_centaurThrottle[NM_THROTTLE_MBA23].commandType = CENTAUR_SCOM_RMW;
            G_centaurThrottle[NM_THROTTLE_MBA23].instanceNumber = l_cent;
            // Set up value to be written
            l_mbafarbq.fields.cfg_nm_n_per_mba = l_mba23_n_per_mba;
            l_mbafarbq.fields.cfg_nm_n_per_chip = l_mba23_n_per_chip;
            G_centaurThrottle[NM_THROTTLE_MBA23].data = l_mbafarbq.value;
        }
        else
        {
            G_centaurThrottle[NM_THROTTLE_MBA23].commandType = CENTAUR_SCOM_NOP;
        }

        /// Set up GPE parameters
        l_parms->scomList     = G_centaurThrottle;
        l_parms->entries      = NUM_CENT_THROTTLE_SCOMS;
        l_parms->error.ffdc   = 0;

        // Update the last sent throttle value, this will get
        // cleared if the GPE does not complete successfully.
        l_cent_ptr->portpair[0].last_mem_speed_sent.word32 = l_mba01_speed.word32;
        l_cent_ptr->portpair[1].last_mem_speed_sent.word32 = l_mba23_speed.word32;

    } while(0);

    return throttle_updated;
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
        memset(G_memoryThrottleLimits, 0, sizeof(G_memoryThrottleLimits));

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
        G_centaur_control_reg_parms.error.ffdc = 0;
        G_centaur_control_reg_parms.entries    = 0;
        G_centaur_control_reg_parms.scomList   = &G_centaurThrottle[0];

        //--------------------------------------------------
        // Initializes GPE Centaur Control Task, but
        // doesn't actually run anything until RTL
        //--------------------------------------------------
        l_rc_gpe = gpe_request_create(
                &G_memory_control_task.gpe_req,            // gpe_req for the task
                &G_async_gpe_queue1,                       // queue
                IPC_ST_CENTAUR_SCOM_FUNCID,                // Function ID
                &G_centaur_control_reg_parms,              // parm for the task
                SSX_WAIT_FOREVER,                          //
                NULL,                                      // callback
                NULL,                                      // callback argument
                ASYNC_CALLBACK_IMMEDIATE );                // options
        if(l_rc_gpe)
        {
            break;
        }

    }
    while(0);


    if( l_rc_gpe )
    {
        //If fail to create gpe request  then there is a problem.
        TRAC_ERR("centaur_control_init: Failed to initialize centaur control task [l_rc_gpe=0x%x]", l_rc_gpe);

        /* @
         * @errortype
         * @moduleid    CENTAUR_INIT_MOD
         * @reasoncode  SSX_GENERIC_FAILURE
         * @userdata1   l_rc_gpe  - Return code of failing function
         * @userdata2   0
         * @userdata4   ERC_CENTAUR_GPE_REQUEST_CREATE_FAILURE
         * @devdesc     Failed to initialize GPE routine
         */
        l_err = createErrl(
                CENTAUR_INIT_MOD,                           //modId
                SSX_GENERIC_FAILURE,                        //reasoncode
                ERC_CENTAUR_GPE_REQUEST_CREATE_FAILURE,     //Extended reason code
                ERRL_SEV_PREDICTIVE,                        //Severity
                NULL,                                       //Trace Buf
                DEFAULT_TRACE_SIZE,                         //Trace Size
                l_rc_gpe,                                   //userdata1
                0                                           //userdata2
                );

        addUsrDtlsToErrl(l_err,                                           //io_err
                         (uint8_t *) &G_centaur_control_request.ffdc,     //i_dataPtr,
                         sizeof(GpeFfdc),                                 //i_size
                         ERRL_USR_DTL_STRUCT_VERSION_1,                   //version
                         ERRL_USR_DTL_BINARY_DATA);                       //type

        REQUEST_RESET(l_err);
    }

    return;
}

bool check_centaur_checkstop(memory_control_task_t * i_memControlTask )
{
    errlHndl_t l_err       = NULL;
    int cent = i_memControlTask->curMemIndex;
    // Check if the centaur has a channel checkstop. If it does,
    // then do not log any errors. We also don't want to throttle
    // a centaur that is in this condition.
    if(G_centaur_control_reg_parms.error.rc != CENTAUR_CHANNEL_CHECKSTOP)
    {
        TRAC_ERR("task_memory_control: IPC_ST_CENTAUR_SCOM failed. "
                 "cent=%d rc=%x, index=0x%08x",
                 cent, G_centaur_control_reg_parms.error.rc,
                 G_centaur_control_reg_parms.error.addr);

        /* @
         * @errortype
         * @moduleid    CENT_CONTROL_MOD
         * @reasoncode  CENT_SCOM_ERROR
         * @userdata1   rc - Return code of scom operation
         * @userdata2   index of scom operation that failed
         * @userdata4   OCC_NO_EXTENDED_RC
         * @devdesc     OCC access to centaur failed
         */
        l_err = createErrl(
            CENT_CONTROL_MOD,                       // modId
            CENT_SCOM_ERROR,                        // reasoncode
            OCC_NO_EXTENDED_RC,                     // Extended reason code
            ERRL_SEV_PREDICTIVE,                    // Severity
            NULL,                                   // Trace Buf
            DEFAULT_TRACE_SIZE,                     // Trace Size
            G_centaur_control_reg_parms.error.rc,   // userdata1
            G_centaur_control_reg_parms.error.addr  // userdata2
            );

        addUsrDtlsToErrl(l_err,                                  //io_err
                         (uint8_t *) &(i_memControlTask->gpe_req.ffdc),    //i_dataPtr,
                         sizeof(GpeFfdc),                                  //i_size
                         ERRL_USR_DTL_STRUCT_VERSION_1,                    //version
                         ERRL_USR_DTL_BINARY_DATA);                        //type

        //callout the centaur
        addCalloutToErrl(l_err,
                         ERRL_CALLOUT_TYPE_HUID,
                         G_sysConfigData.centaur_huids[cent],
                         ERRL_CALLOUT_PRIORITY_MED);

        //callout the processor
        addCalloutToErrl(l_err,
                         ERRL_CALLOUT_TYPE_HUID,
                         G_sysConfigData.proc_huid,
                         ERRL_CALLOUT_PRIORITY_MED);

        commitErrl(&l_err);

        return TRUE; // a centaur channel checkstop error occured
    }
    return FALSE;    // No centaur channel checkstop errors

}


