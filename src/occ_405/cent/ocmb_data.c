/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/cent/ocmb_data.c $                                */
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
#include "centaur_data.h"
#include "ocmb_membuf.h"
#include "occ_service_codes.h"
#include "centaur_data_service_codes.h"
#include "errl.h"
#include "trac.h"
#include "membuf_structs.h"

/**
 * GPE shared data area for gpe0 tracebuffer and size
 */
extern gpe_shared_data_t G_shared_gpe_data;

/**
 * ocmb_init
 * Initialize environment for collection of ocmb DTS and performance
 * data.
 * @post G_membufConfiguration populated
 * @post G_present_centaurs populated
 * @post G_dimm_present_sensors
 * @post G_ocmb_data_task populated
 * @post GPE request to call for recover created ?
 * @post GPE request to call for throttle conttrol created
 * @note HW Deadman timer enabled and set to max value
 * @note HW Any emergency throttle cleared
*/
void ocmb_init(void)
{
    //errlHndl_t err = NULL;
    int rc = 0;
    int membuf_idx = 0;
    do
    {
        TRAC_INFO("ocmb_init: Initializing Memory Data Controller");
        // Create configuration data use G_membufConfiguration
        G_membufConfiguration.config = 0;

        for(membuf_idx = 0; membuf_idx<MAX_NUM_OCMBS; ++membuf_idx)
        {
            if(CENTAUR_PRESENT(membuf_idx)) //based on HTMGT config cmd
            {
                G_membufConfiguration.config |= CHIP_CONFIG_MEMBUF(membuf_idx);
            }
        }

        rc = membuf_configuration_create(&G_membufConfiguration);
        if( rc )
        {
            break;
        }

        for(membuf_idx=0; membuf_idx<MAX_NUM_OCMBS; ++membuf_idx)
        {
            if( G_membufConfiguration.baseAddress[membuf_idx] )
            {
                // A valid inband Bar Address was found, check which DTS are
                // enabled.
                if(G_membufConfiguration.dts_config & CONFIG_MEMDTS0(membuf_idx))
                {
                    G_dimm_enabled_sensors.bytes[membuf_idx] = DIMM_SENSOR0;
                }
                if(G_membufConfiguration.dts_config & CONFIG_MEMDTS1(membuf_idx))
                {
                    G_dimm_enabled_sensors.bytes[membuf_idx] = (DIMM_SENSOR0 >> 1);
                }
                TRAC_INFO("ocmb_init: Membuf[%d] Found.",
                          membuf_idx);
            }
        }

        TRAC_IMP("ocmb_init: G_present_OCMBs = 0x%08x", G_present_centaurs);

        G_dimm_present_sensors = G_dimm_enabled_sensors;

        TRAC_IMP("bitmap of present dimm temperature sensors: 0x%08X%08X%08X%08X",
                 (uint32_t)(G_dimm_enabled_sensors.dw[0]>>32),
                 (uint32_t)G_dimm_enabled_sensors.dw[0],
                 (uint32_t)(G_dimm_enabled_sensors.dw[1]>>32),
                 (uint32_t)G_dimm_enabled_sensors.dw[1]);

        // Setup the GPE request to do sensor data collection
        G_membuf_data_parms.error.ffdc = 0;
        G_membuf_data_parms.collect = -1;
        G_membuf_data_parms.update = -1;
        G_membuf_data_parms.data = 0;

        rc = gpe_request_create(
                &G_ocmb_data_task.gpe_req,       //gpe_req for the task
                &G_async_gpe_queue1,              //queue
                IPC_ST_MEMBUF_DATA_FUNCID,        //Function ID
                &G_membuf_data_parms,            //parm for the task
                SSX_WAIT_FOREVER,                 //
                NULL,                             //callback
                NULL,                             //callback argument
                0 );                              //options
        if( rc )
        {
            TRAC_ERR("ocmb_init: gpe_request_create failed for "
                     "G_ocmb_data_task.gpe_req. rc = 0x%08x", rc);
            break;
        }

    } while(0);

    if( rc )
    {
        /* @
         * @errortype
         * @moduleid    OCMB_INIT_MOD
         * @reasoncode  SSX_GENERIC_FAILURE
         * @userdata1   rc - Return code of failing function
         * @userdata4   OCC_NO_EXTENDED_RC
         * @devdesc     Failed to initialize memory buffer sensors
         */
        errlHndl_t l_err =
            createErrl(
                       OCMB_INIT_MOD,                     //modId
                       SSX_GENERIC_FAILURE,               //reasoncode
                       OCC_NO_EXTENDED_RC,                //Extended reasoncode
                       ERRL_SEV_PREDICTIVE,               //Severity
                       NULL,                              //Trace Buf
                       DEFAULT_TRACE_SIZE,                //Trace Size
                       rc,                                //userdata1
                       0                                  //userdata2
                      );

        // Capture the GPE1 trace buffer
        addUsrDtlsToErrl(l_err,
                         (uint8_t *) G_shared_gpe_data.gpe1_tb_ptr,
                         G_shared_gpe_data.gpe1_tb_sz,
                         ERRL_USR_DTL_STRUCT_VERSION_1,
                         ERRL_USR_DTL_TRACE_DATA);


        REQUEST_RESET(l_err);
    }
    else
    {
        ocmb_control_init();
    }

    return;
}
