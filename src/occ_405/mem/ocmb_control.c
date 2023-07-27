/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/mem/ocmb_control.c $                              */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2016,2023                        */
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
#include <occ_common.h>
#include "gpe_export.h"
#include "memory_service_codes.h"
#include "errl.h"
#include "trac.h"
#include "memory.h"
#include "memory_data.h"
#include "membuf_structs.h"
#include "ocmb_firmware_registers.h"
#include "ocmb_register_addresses.h"
#include "amec_sys.h"

typedef enum
{
    NM_THROTTLE_MBA = 0,
    NM_THROTTLE_SYNC = 1,
    NUM_THROTTLE_SCOMS = 2,
} eOcmbThrottleInfo;

extern memory_control_task_t G_memory_control_task;

GPE_BUFFER(MemBufScomParms_t G_membuf_control_reg_parms);
GPE_BUFFER(scomList_t G_memThrottle[NUM_THROTTLE_SCOMS]);


uint16_t throttle_convert_2_numerator(uint16_t i_throttle, uint8_t i_membuf);
void update_nlimits(uint32_t i_membuf);


void ocmb_control_init( void )
{
    errlHndl_t err = NULL;
    int        rc_gpe = 0;
    ocmb_mba_farb3q_t l_mbafarbq;

    //initialize the active throttle limits
    memset(G_memoryThrottleLimits, 0, sizeof(G_memoryThrottleLimits));

    // set upt the throttle scom regs. //slot 0, slot1 or per-port
    if(IS_OCM_DDR4_MEM_TYPE(G_sysConfigData.mem_type))
        G_memThrottle[NM_THROTTLE_MBA].scom = OCMB_MBA_FARB3Q;
    else
        G_memThrottle[NM_THROTTLE_MBA].scom = OCMB_MBA_FARB3Q_DDR5;

    l_mbafarbq.value = 0;
    l_mbafarbq.fields.cfg_nm_n_per_slot = -1;
    l_mbafarbq.fields.cfg_nm_n_per_port = -1;
    G_memThrottle[NM_THROTTLE_MBA].mask = l_mbafarbq.value;

    // Set up GPE parameters
    G_membuf_control_reg_parms.error.ffdc = 0;
    G_membuf_control_reg_parms.entries    = 0;
    G_membuf_control_reg_parms.scomList   = &G_memThrottle[0];

    //--------------------------------------------------
    // Initializes GPE memory Control Task, but
    // doesn't actually run anything until RTL
    //--------------------------------------------------
    rc_gpe = gpe_request_create(
                                &G_memory_control_task.gpe_req,            // gpe_req for the task
                                &G_async_gpe_queue1,                       // queue
                                IPC_ST_MEMBUF_SCOM_FUNCID,                 // Function ID
                                &G_membuf_control_reg_parms,               // parm for the task
                                SSX_WAIT_FOREVER,                          //
                                NULL,                                      // callback
                                NULL,                                      // callback argument
                                ASYNC_CALLBACK_IMMEDIATE );                // options

    if (rc_gpe)
    {
        //If fail to create gpe request  then there is a problem.
        TRAC_ERR("ocmb_control_init: Failed to initialize membuf control task [rc_gpe=0x%x]", rc_gpe);

        /* @
         * @errortype
         * @moduleid    MEM_MID_OCMB_INIT_MOD
         * @reasoncode  SSX_GENERIC_FAILURE
         * @userdata1   l_rc_gpe  - Return code of failing function
         * @userdata2   0
         * @userdata4   ERC_MEMBUF_GPE_REQUEST_CREATE_FAILURE
         * @devdesc     Failed to initialize GPE routine
         */
        err = createErrl(
                         MEM_MID_OCMB_INIT_MOD,                      //modId
                         SSX_GENERIC_FAILURE,                        //reasoncode
                         ERC_MEMBUF_GPE_REQUEST_CREATE_FAILURE,      //Extended reason code
                         ERRL_SEV_PREDICTIVE,                        //Severity
                         NULL,                                       //Trace Buf
                         DEFAULT_TRACE_SIZE,                         //Trace Size
                         rc_gpe,                                     //userdata1
                         0                                           //userdata2
                        );

        addUsrDtlsToErrl(err,                                             //io_err
                         (uint8_t *) &G_memory_control_task.gpe_req.ffdc, //i_dataPtr,
                         sizeof(GpeFfdc),                                 //i_size
                         ERRL_USR_DTL_STRUCT_VERSION_1,                   //version
                         ERRL_USR_DTL_BINARY_DATA);                       //type

        REQUEST_RESET(err);
    }
}


// Function Specification
//
// Name: ocmb_control
//
// Description: Performs membuf control.
// return TRUE  settings changed HW needs to be updated.
// return FALSE settings did  not change
//
// End Function Specification
bool ocmb_control( memory_control_task_t * i_memControlTask )
{
    static bool G_throttle_update_since_last_sync = FALSE;
    bool    throttle_updated = FALSE;
    int     l_membuf = i_memControlTask->curMemIndex;
    amec_membuf_t * l_membuf_ptr = NULL;

    MemBufScomParms_t * l_parms =
        (MemBufScomParms_t *)(i_memControlTask->gpe_req.cmd_data);

    if(MEMBUF_PRESENT(l_membuf))
    {
        l_membuf_ptr = &g_amec->proc[0].memctl[l_membuf].membuf;

        // update min/max settings
        update_nlimits(l_membuf);

        // calculate new N values
        ocmb_mba_farb3q_t l_mbafarbq;
        uint16_t l_nm_n_per_slot =
            throttle_convert_2_numerator(g_amec->mem_speed_request, l_membuf);

        uint16_t l_nm_n_per_chip = G_memoryThrottleLimits[l_membuf].max_n_per_chip;
        amec_mem_speed_t l_nm_speed;

        //combine port and slot(mba) settings (16 bit) in to a single 32bit value
        l_nm_speed.mba_n = l_nm_n_per_slot;
        l_nm_speed.chip_n = l_nm_n_per_chip;


        // Check if the throttle value has been updated since the last
        // time we sent it.  If it has, then send a new value, otherwise
        // do nothing.
        if ( l_nm_speed.word32 != l_membuf_ptr->portpair[0].last_mem_speed_sent.word32 )
        {
            G_throttle_update_since_last_sync = TRUE;
            throttle_updated = TRUE;
            /// Set up Scom Registers - array of Scoms

            /// [0]: Set up N/M throttle MBA
            G_memThrottle[NM_THROTTLE_MBA].commandType = MEMBUF_SCOM_RMW;
            G_memThrottle[NM_THROTTLE_MBA].instanceNumber = l_membuf;
            // Set up value to be written
            l_mbafarbq.fields.cfg_nm_n_per_slot = l_nm_n_per_slot;
            l_mbafarbq.fields.cfg_nm_n_per_port = l_nm_n_per_chip;
            G_memThrottle[NM_THROTTLE_MBA].data = l_mbafarbq.value;

            /// [1]: throttle sync
            G_memThrottle[NM_THROTTLE_SYNC].commandType = MEMBUF_SCOM_MEMBUF_SYNC;


            // Update the last sent throttle value, this will get
            // cleared if the GPE does not complete successfully.
            l_membuf_ptr->portpair[0].last_mem_speed_sent.word32 = l_nm_speed.word32;
        }
        else // no throttle change
        {
            G_memThrottle[NM_THROTTLE_MBA].commandType = MEMBUF_SCOM_NOP;
        }
    }
    else // membuf is not present
    {
        G_memThrottle[NM_THROTTLE_MBA].commandType = MEMBUF_SCOM_NOP;
    }

    // Only send sync if last membuf possition and any throttles changed since
    // last sync sent, even if this membuf does not exist
    if(l_membuf == i_memControlTask->endMemIndex &&
       G_throttle_update_since_last_sync)
    {
        /// [1]: throttle sync
        G_memThrottle[NM_THROTTLE_SYNC].commandType = MEMBUF_SCOM_MEMBUF_SYNC;
        G_throttle_update_since_last_sync = FALSE;
        throttle_updated = TRUE;
    }
    else // No sync needed.
    {
        G_memThrottle[NM_THROTTLE_SYNC].commandType = MEMBUF_SCOM_NOP;
    }

    if(throttle_updated)
    {
        /// Set up GPE parameters
        l_parms->scomList     = G_memThrottle;
        l_parms->entries      = NUM_THROTTLE_SCOMS;
        l_parms->error.ffdc   = 0;
    }

    return throttle_updated;
}


//////////////////////////
// Function Specification
//
// Name: throttle_convert_2_numerator
//
// Description: Converts a throttle percentage into an 'N' value that can
//              be written to the hardware.
//
//
// Thread: RTL
//
// End Function Specification
uint16_t throttle_convert_2_numerator(uint16_t i_throttle, uint8_t i_membuf)
{
#define THROTTLE_100_PERCENT_VALUE 1000

    uint32_t l_nvalue = 0;
    memory_throttle_t* l_mba = &G_memoryThrottleLimits[i_membuf];

    if(MBA_CONFIGURED(i_membuf))
    {
        // Convert the throttle ( actually in units of 0.1 %) to a "N" value
        l_nvalue = (l_mba->max_n_per_mba * i_throttle) /
              THROTTLE_100_PERCENT_VALUE;

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
// Name: update_nlimits
//
// Description: Updates the memory throttle settings for;
//              1) new settings from FSP
//              2) change to/from TURBO or DPS mode
//              3) enter/exit oversubscription
//
// Thread: RTL
//
// End Function Specification
#define TRACE_THROTTLE_DELAY 8
void update_nlimits(uint32_t i_membuf)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    static uint32_t L_trace_throttle_count = 0;
    uint16_t l_mba_maxn, l_chip_maxn;
    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    do
    {
        memory_throttle_t* l_active_limits =
            &G_memoryThrottleLimits[i_membuf];

        mem_throt_config_data_v40_t* l_state_limits =
            &G_sysConfigData.mem_throt_limits[i_membuf];

        //Minimum N value is not state dependent
        l_active_limits->min_n_per_mba = l_state_limits->min_n_per_mba;

        //oversubscription?
        if(AMEC_INTF_GET_OVERSUBSCRIPTION())
        {
            l_mba_maxn = l_state_limits->oversub_n_per_mba;
            l_chip_maxn = l_state_limits->oversub_n_per_chip;
        }
        else if(CURRENT_MODE() == OCC_MODE_DISABLED)
        {
            l_mba_maxn = l_state_limits->mode_disabled_n_per_mba;
            l_chip_maxn = l_state_limits->mode_disabled_n_per_chip;
        }
        else if(CURRENT_MODE() == OCC_MODE_FMAX)
        {
            l_mba_maxn = l_state_limits->fmax_n_per_mba;
            l_chip_maxn = l_state_limits->fmax_n_per_chip;
        }
        else //all other modes will use ultra turbo settings
        {
            l_mba_maxn = l_state_limits->ut_n_per_mba;
            l_chip_maxn = l_state_limits->ut_n_per_chip;
        }

        l_active_limits->max_n_per_chip = l_chip_maxn;

        //Trace when the MBA max N value changes
        if(l_mba_maxn != l_active_limits->max_n_per_mba)
        {
            l_active_limits->max_n_per_mba = l_mba_maxn;

            //Don't trace every MBA changing, just one
            if(!L_trace_throttle_count)
            {
                L_trace_throttle_count = TRACE_THROTTLE_DELAY;
                TRAC_IMP("New MBA throttle max|min N values: mba[0x%08x]",
                        (uint32_t)((l_mba_maxn << 16) | l_active_limits->min_n_per_mba));
                break;
            }
        }

        if(L_trace_throttle_count)
        {
            L_trace_throttle_count--;
        }

    }while(0);
}
