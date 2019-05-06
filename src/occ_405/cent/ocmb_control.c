/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/cent/ocmb_control.c $                             */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2016,2019                        */
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
#include "centaur_data_service_codes.h"
#include "errl.h"
#include "trac.h"
#include "memory.h"
#include "membuf_structs.h"
#include "ocmb_firmware_registers.h"
#include "ocmb_register_addresses.h"
#include "amec_sys.h"

// These are found in centaur_control.c.  They are valid for Ocmb
extern MemBufScomParms_t G_membuf_control_reg_parms;
GPE_BUFFER(extern scomList_t *G_centaurThrottle);
extern memory_control_task_t G_memory_control_task;
void cent_update_nlimits(uint32_t i_cent);
uint16_t centaurThrottle_convert2Numerator(uint16_t i_throttle, uint8_t i_cent, uint8_t i_mba);

typedef enum
{
    NM_THROTTLE_MBA = 0,
    NM_THROTTLE_SYNC = 1,
    NUM_THROTTLE_SCOMS = 2,
} eOcmbThrottleInfo;


void ocmb_control_init( void )
{
    errlHndl_t err = NULL;
    int        rc_gpe = 0;
    ocmb_mba_farb3q_t l_mbafarbq;

    //initialize the active throttle limits
    memset(G_memoryThrottleLimits, 0, sizeof(G_memoryThrottleLimits));

    // set upt the throttle scom regs. //slot 0, slot1 or per-port
    G_centaurThrottle[NM_THROTTLE_MBA].scom = OCMB_MBA_FARB3Q;
    l_mbafarbq.value = 0;
    l_mbafarbq.fields.cfg_nm_n_per_slot = -1;
    l_mbafarbq.fields.cfg_nm_n_per_port = -1;
    G_centaurThrottle[NM_THROTTLE_MBA].mask = l_mbafarbq.value;

    // Set up GPE parameters
    G_membuf_control_reg_parms.error.ffdc = 0;
    G_membuf_control_reg_parms.entries    = 0;
    G_membuf_control_reg_parms.scomList   = &G_centaurThrottle[0];

    //--------------------------------------------------
    // Initializes GPE Centaur Control Task, but
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
         * @moduleid    OCMB_INIT_MOD
         * @reasoncode  SSX_GENERIC_FAILURE
         * @userdata1   l_rc_gpe  - Return code of failing function
         * @userdata2   0
         * @userdata4   ERC_CENTAUR_GPE_REQUEST_CREATE_FAILURE
         * @devdesc     Failed to initialize GPE routine
         */
        err = createErrl(
                         OCMB_INIT_MOD,                          //modId
                         SSX_GENERIC_FAILURE,                        //reasoncode
                         ERC_CENTAUR_GPE_REQUEST_CREATE_FAILURE,     //Extended reason code
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
// Description: Performs centaur control.
// return TRUE  settings changed HW needs to be updated.
// return FALSE settings did  not change
//
// End Function Specification
bool ocmb_control( memory_control_task_t * i_memControlTask )
{
    bool    throttle_updated = TRUE;
    int     l_membuf = i_memControlTask->curMemIndex;
    amec_centaur_t * l_cent_ptr = NULL;

    MemBufScomParms_t * l_parms =
        (MemBufScomParms_t *)(i_memControlTask->gpe_req.cmd_data);

    l_cent_ptr = &g_amec->proc[0].memctl[l_membuf].centaur;

    // update min/max settings
    cent_update_nlimits(l_membuf);

    // calculate new N values
    ocmb_mba_farb3q_t l_mbafarbq;
    uint16_t l_mba01_n_per_mba =
        centaurThrottle_convert2Numerator(g_amec->mem_speed_request, l_membuf, 0);

    uint16_t l_mba01_n_per_chip = G_memoryThrottleLimits[l_membuf][0].max_n_per_chip;
    amec_cent_mem_speed_t l_mba01_speed;

    //combine port and slot(mba) settings (16 bit) in to a single 32bit value
    l_mba01_speed.mba_n = l_mba01_n_per_mba;
    l_mba01_speed.chip_n = l_mba01_n_per_chip;


    // Check if the throttle value has been updated since the last
    // time we sent it.  If it has, then send a new value, otherwise
    // do nothing.
    if ( l_mba01_speed.word32 != l_cent_ptr->portpair[0].last_mem_speed_sent.word32 )
    {

        /// Set up Centuar Scom Registers - array of Scoms
        ///   [0]:  N/M Throttle MBA

        /// [0]: Set up N/M throttle MBA
        G_centaurThrottle[NM_THROTTLE_MBA].commandType = MEMBUF_SCOM_RMW;
        G_centaurThrottle[NM_THROTTLE_MBA].instanceNumber = l_membuf;
        // Set up value to be written
        l_mbafarbq.fields.cfg_nm_n_per_slot = l_mba01_n_per_mba;
        l_mbafarbq.fields.cfg_nm_n_per_port = l_mba01_n_per_chip;
        G_centaurThrottle[NM_THROTTLE_MBA].data = l_mbafarbq.value;

        G_centaurThrottle[NM_THROTTLE_SYNC].commandType = MEMBUF_SCOM_MEMBUF_SYNC;

        /// Set up GPE parameters
        l_parms->scomList     = G_centaurThrottle;
        l_parms->entries      = NUM_THROTTLE_SCOMS;
        l_parms->error.ffdc   = 0;

        // Update the last sent throttle value, this will get
        // cleared if the GPE does not complete successfully.
        l_cent_ptr->portpair[0].last_mem_speed_sent.word32 = l_mba01_speed.word32;
    }
    else
    {
        throttle_updated = FALSE;
    }

    return throttle_updated;
}
