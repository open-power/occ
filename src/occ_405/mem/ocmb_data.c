/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/mem/ocmb_data.c $                                 */
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
#include "ocmb_membuf.h"
#include "occ_service_codes.h"
#include "memory_service_codes.h"
#include "errl.h"
#include "trac.h"
#include "membuf_structs.h"
#include "memory.h"
#include "memory_data.h" 
#include "amec_sys.h"
#include "ocmb_register_addresses.h"

// Used for specifing buffer allocations
#define NUM_DATA_EMPTY_BUF  1

#define OCMB_DEADMAN_TIMER_FIELD    0xf
#define OCMB_DEADMAN_TIMER_SEL_MAX  0x8
#define OCMB_DEADMAN_TIMEBASE_SEL   0

//Notes MemData is defined @
// /afs/awd/proj/p9/eclipz/KnowledgeBase/eclipz/chips/p8/working/procedures/lib/gpe_data.h
// struct {
//   MemDataMcs  mcs;  // not used
//   MemDataSensorCache scache;
//   } MemData;
//Global array of membuf data buffers
GPE_BUFFER(OcmbMemData G_membuf_data[MAX_NUM_OCMBS + NUM_DATA_EMPTY_BUF]);

// parms for call to IPC_ST_MEMBUF_INIT_FUNCID
GPE_BUFFER(MemBufConfigParms_t G_gpe_membuf_config_args);


GPE_BUFFER(MemBufConfiguration_t G_membufConfiguration);
//Global array of membuf data pointers
OcmbMemData * G_membuf_data_ptrs[MAX_NUM_OCMBS] =
  { &G_membuf_data[0],  &G_membuf_data[1],  &G_membuf_data[2],  &G_membuf_data[3],
    &G_membuf_data[4],  &G_membuf_data[5],  &G_membuf_data[6],  &G_membuf_data[7],
    &G_membuf_data[8],  &G_membuf_data[9],  &G_membuf_data[10], &G_membuf_data[11],
    &G_membuf_data[12], &G_membuf_data[13], &G_membuf_data[14], &G_membuf_data[15]};

//Global structures for gpe get mem data parms
GPE_BUFFER(MemBufGetMemDataParms_t G_membuf_data_parms);

// GPE shared data area for gpe0 tracebuffer and size
extern gpe_shared_data_t G_shared_gpe_data;

//Global G_present_membufs is bitmask of all membufs
//(1 = present, 0 = not present. Membuf 0 has the most significant bit)
uint32_t G_present_membufs = 0;
uint32_t G_membuf_dts_enabled = 0;

//Memory data collect structures used for task data pointers
struct membuf_data_task {
        uint8_t start_membuf;
        uint8_t current_membuf;
        uint8_t end_membuf;
        uint8_t prev_membuf;
        OcmbMemData * membuf_data_ptr;
        GpeRequest gpe_req;
} __attribute__ ((__packed__));
typedef struct membuf_data_task membuf_data_task_t;

//Memory structures used for task data pointers.
membuf_data_task_t G_membuf_data_task = {
    .start_membuf = 0,
    .current_membuf = 0,
    .end_membuf = MAX_NUM_OCMBS - 1,
    .prev_membuf = MAX_NUM_OCMBS - 1,
    .membuf_data_ptr = &G_membuf_data[MAX_NUM_OCMBS]
};

dimm_sensor_flags_t G_dimm_enabled_sensors = {{0}};
dimm_sensor_flags_t G_dimm_configured_sensors = {{0}};

//AMEC needs to know when data for a membuf has been collected.
uint32_t G_updated_membuf_mask = 0;

// bitmap of membufs requiring i2c recovery
uint8_t  G_membuf_needs_recovery = 0;

// Latch for a Trace Entry
uint8_t G_membuf_queue_not_idle_traced = 0;

// Mask that is used by procedure to specify which membufs are present
#define ALL_MEMBUFS_MASK          0x0000ffff

//Returns the bitmask for the passed in membuf (uint32_t)
#define MEMBUF_BY_MASK(occ_membuf_id) \
         ((MEMBUF0_PRESENT_MASK >> occ_membuf_id) & ALL_MEMBUFS_MASK)


uint32_t membuf_configuration_create();


// Remove the membuf from the enabled bitmask.
void disable_membuf(uint32_t i_membuf)
{
    if(MEMBUF_PRESENT(i_membuf))
    {
        //remove checkstopped membuf from presence bitmap
        G_present_membufs &= ~(MEMBUF_BY_MASK(i_membuf));

        if(!IS_I2C_MEM_TYPE(G_sysConfigData.mem_type))
        {
            // remove the dimm temperature sensors behind this membuf
            G_dimm_enabled_sensors.bytes[i_membuf] = 0;
        }

        TRAC_IMP("Error detected on MemBuf[%d], G_present_membufs[0x%08X]",
                 i_membuf, G_present_membufs);

        TRAC_IMP("Updated bitmap of enabled dimm temperature sensors: 0x%08X %08X",
                 (uint32_t)(G_dimm_enabled_sensors.dw[0]>> 32),
                 (uint32_t)G_dimm_enabled_sensors.dw[0]);
    }
}

/**
 * ocmb_init
 * Initialize environment for collection of ocmb DTS and performance
 * data.
 * @post G_membufConfiguration populated
 * @post G_present_membufs populated
 * @post G_dimm_enabled_sensors populated
 * @post G_membuf_data_task populated
 * @post GPE request to call for recover created ?
 * @post GPE request to call for throttle conttrol created
 * @note HW Deadman timer enabled and set to max value
 * @note HW Any emergency throttle cleared
*/
void ocmb_init(void)
{
    //errlHndl_t err = NULL;
    int rc = 0;
    int gpe_request_rc = 0;
    int membuf_idx = 0;
    int dts_num = 0;
    int l_reset_on_error = 1;
    uint32_t missing_membuf_bitmap = 0;
    GpeRequest l_gpe_request;
    static scomList_t L_scomList[1] SECTION_ATTRIBUTE(".noncacheable");
    static MemBufScomParms_t L_ocmb_reg_parms SECTION_ATTRIBUTE(".noncacheable");

    do
    {
        TRAC_INFO("ocmb_init: Initializing Memory Data Controller");
        // Create configuration data use G_membufConfiguration
        G_membufConfiguration.config = 0;

        for(membuf_idx = 0; membuf_idx<MAX_NUM_OCMBS; ++membuf_idx)
        {
            if(MEMBUF_PRESENT(membuf_idx)) //based on HTMGT config cmd
            {
                G_membufConfiguration.config |= CHIP_CONFIG_MEMBUF(membuf_idx);
            }
        }

        rc = membuf_configuration_create(&G_membufConfiguration);
        if( rc )
        {
            l_reset_on_error = 1;
            break;
        }

        for(membuf_idx=0; membuf_idx<MAX_NUM_OCMBS; ++membuf_idx)
        {
            if( G_membufConfiguration.baseAddress[membuf_idx] )
            {
                // A valid inband Bar value was found.
                // Enable dts if enabled in HW and configured by TMGT.
                for(dts_num = 0; dts_num < G_gpe_membuf_config_args.max_dts; ++dts_num)
                {
                    if(G_membufConfiguration.dts_config & CONFIG_MEMDTSx(membuf_idx, dts_num))
                    {
                       if(G_dimm_configured_sensors.bytes[membuf_idx] & (DIMM_SENSOR0 >> dts_num))
                       {
                           G_dimm_enabled_sensors.bytes[membuf_idx] |= (DIMM_SENSOR0 >> dts_num);
                       }
                       else
                       {
                           TRAC_INFO("ocmb_init: Membuf[%d] Ignoring DIMM sensor%02x found "
                                     "enabled in hardware, but not configured by TMGT.",
                                     membuf_idx, dts_num);
                       }
                    }

                } // for each DTS

                if(G_membufConfiguration.ubdts_config & CHIP_CONFIG_MEMBUF(membuf_idx))
                {
                    if((MEMBUF_PRESENT(membuf_idx)) &&
                       (g_amec->proc[0].memctl[membuf_idx].membuf.membuf_hottest.temp_fru_type != DATA_FRU_NOT_USED))
                    {
                        G_membuf_dts_enabled |= MEMBUF_BY_MASK(membuf_idx);
                    }
                    else
                    {
                        TRAC_INFO("ocmb_init: Ignoring membuf[%d]. Sensor configured"
                                  " in hardware, but not configured by TMGT",
                                  membuf_idx);
                    }
                }
            }
            else if (MEMBUF_PRESENT(membuf_idx)) //Only true if 1+ related TMGT configured dts
            {
                TRAC_ERR("ocmb_init: OCMB[%d] There are dts configured by TMGT, but"
                         "not configured in hardware. Ignoring dts.",
                         membuf_idx);

                missing_membuf_bitmap |= MEMBUF_BY_MASK(membuf_idx);
                G_present_membufs &= ~(MEMBUF_BY_MASK(membuf_idx));
            }
        }

        TRAC_IMP("ocmb_init: G_present_membufs = 0x%08x", G_present_membufs);

        TRAC_IMP("bitmap of enabled dimm temperature sensors: 0x%08X%08X %08X%08X",
                 (uint32_t)(G_dimm_enabled_sensors.dw[0]>>32),
                 (uint32_t)G_dimm_enabled_sensors.dw[0],
                 (uint32_t)(G_dimm_enabled_sensors.dw[1]>>32),
                 (uint32_t)G_dimm_enabled_sensors.dw[1]);

        // Setup the OCMB deadman timer
        if(IS_OCM_DDR4_MEM_TYPE(G_sysConfigData.mem_type))
            L_scomList[0].scom = OCMB_MBASTR0Q;
        else
            L_scomList[0].scom = OCMB_MBASTR0Q_DDR5;

        L_scomList[0].commandType = MEMBUF_SCOM_RMW_ALL;

        ocmb_mbastr0q_t l_mbascfg;
        l_mbascfg.value = 0;

        // setup the mask bits
        l_mbascfg.fields.deadman_timer_sel = OCMB_DEADMAN_TIMER_FIELD;
        l_mbascfg.fields.deadman_tb_sel = 1;
        L_scomList[0].mask = l_mbascfg.value;

        // setup config data to enable the deadman timer
        l_mbascfg.fields.deadman_timer_sel = OCMB_DEADMAN_TIMER_SEL_MAX;
        l_mbascfg.fields.deadman_tb_sel    = OCMB_DEADMAN_TIMEBASE_SEL;
        L_scomList[0].data = l_mbascfg.value;


        L_ocmb_reg_parms.scomList = &L_scomList[0];
        L_ocmb_reg_parms.entries = 1;
        L_ocmb_reg_parms.error.ffdc = 0;


        rc = gpe_request_create(
                                &l_gpe_request,                 //gpe_reqest
                                &G_async_gpe_queue1,            //gpe1 queue
                                IPC_ST_MEMBUF_SCOM_FUNCID,      //function id
                                &L_ocmb_reg_parms,              //parms
                                SSX_SECONDS(5),                 //timeout
                                NULL,                           //callback
                                NULL,                           //callback args
                                ASYNC_REQUEST_BLOCKING );       // wait
        if(rc)
        {
            TRAC_ERR("ocmb_init: gpe_request_create failed. rc = 0x%08x", rc);
            break;
        }

        rc = gpe_request_schedule(&l_gpe_request);

        if(rc || L_ocmb_reg_parms.error.rc)
        {
            TRAC_ERR("ocmb_init: IPC_ST_MEMBUF_SCOM failure. rc = 0x%08x, "
                     "gpe_rc = 0x%08x, address = 0x%08x",
                     rc,
                     L_ocmb_reg_parms.error.rc,
                     L_ocmb_reg_parms.error.addr);
            if(!rc)
            {
                rc = L_ocmb_reg_parms.error.rc;
            }
            break;
        }

        // Setup the GPE request to do sensor data collection
        G_membuf_data_parms.error.ffdc = 0;
        G_membuf_data_parms.collect = -1;
        G_membuf_data_parms.touch = 1;  // OCC touch to reset ocmb deadman timer
        G_membuf_data_parms.data = 0;

        gpe_request_rc = 
            gpe_request_create(
                               &G_membuf_data_task.gpe_req,     //gpe_req for the task
                               &G_async_gpe_queue1,              //queue
                               IPC_ST_MEMBUF_DATA_FUNCID,        //Function ID
                               &G_membuf_data_parms,            //parm for the task
                               SSX_WAIT_FOREVER,                 //
                               NULL,                             //callback
                               NULL,                             //callback argument
                               0 );                              //options

        if( gpe_request_rc )
        {
            TRAC_ERR("ocmb_init: gpe_request_create failed for "
                     "G_membuf_data_task.gpe_req. rc = 0x%08x", gpe_request_rc);
            /* @
             * @errortype
             * @moduleid    MEM_MID_OCMB_INIT_MOD
             * @reasoncode  GPE_REQUEST_CREATE_FAILURE
             * @userdata1   rc - Return code of failing function
             * @userdata4   OCC_NO_EXTENDED_RC
             * @devdesc     Failed to create gpe request to read memory buffer sensors
             */
            errlHndl_t l_err =
                createErrl(
                           MEM_MID_OCMB_INIT_MOD,             //modId
                           GPE_REQUEST_CREATE_FAILURE,        //reasoncode
                           OCC_NO_EXTENDED_RC,                //Extended reasoncode
                           ERRL_SEV_PREDICTIVE,               //Severity
                           NULL,                              //Trace Buf
                           DEFAULT_TRACE_SIZE,                //Trace Size
                           gpe_request_rc,                    //userdata1
                           0                                  //userdata2
                          );

            // Capture the GPE1 trace buffer
            addUsrDtlsToErrl(l_err,
                             (uint8_t *) G_shared_gpe_data.gpe1_tb_ptr,
                             G_shared_gpe_data.gpe1_tb_sz,
                             ERRL_USR_DTL_STRUCT_VERSION_1,
                             ERRL_USR_DTL_TRACE_DATA);

            REQUEST_RESET(l_err);

            break;
        }

    } while(0);

    if(!rc && !gpe_request_rc)
    {
        if(missing_membuf_bitmap)
        {
            // already traced
            rc = RC_OCMB_DTS_NOT_CONFIGURED | missing_membuf_bitmap;
            l_reset_on_error = 0;
        }
        else if((G_dimm_enabled_sensors.dw[0] != G_dimm_configured_sensors.dw[0]) ||
                (G_dimm_enabled_sensors.dw[1] != G_dimm_configured_sensors.dw[1]))
        {
            // we use G_dimm_configured_sensors to know which I2C DIMMs are present which
            // won't show up enabled in hw so just ignore the mismatch for I2C type
            if(!IS_I2C_MEM_TYPE(G_sysConfigData.mem_type))
            {
                TRAC_INFO("ocmb_init: There are TMGT configured DIMM sensors that are not configured"
                         " in hardware. Bitmap of configured dimm dts: 0x%08X%08X %08X%08X",
                         (uint32_t)(G_dimm_configured_sensors.dw[0]>>32),
                         (uint32_t)G_dimm_configured_sensors.dw[0],
                         (uint32_t)(G_dimm_configured_sensors.dw[1]>>32),
                         (uint32_t)G_dimm_configured_sensors.dw[1]);

                rc = RC_DIMM_DTS_NOT_CONFIGURED;
                l_reset_on_error = 0;
            }
        }
    }

    if( rc )
    {
        ERRL_SEVERITY l_severity = ERRL_SEV_INFORMATIONAL;
        if(l_reset_on_error)
        {
            l_severity = ERRL_SEV_PREDICTIVE;
        }

        /* @
         * @errortype
         * @moduleid    MEM_MID_OCMB_INIT_MOD
         * @reasoncode  INVALID_CONFIG_DATA
         * @userdata1   rc - Return code of failing function
         * @userdata4   OCC_NO_EXTENDED_RC
         * @devdesc     Failed to initialize memory buffer sensors
         */
        errlHndl_t l_err =
            createErrl(
                       MEM_MID_OCMB_INIT_MOD,             //modId
                       INVALID_CONFIG_DATA,               //reasoncode
                       OCC_NO_EXTENDED_RC,                //Extended reasoncode
                       l_severity,                        //Severity
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


        if(l_reset_on_error)
        {
            REQUEST_RESET(l_err);
        }
        else
        {
            commitErrl(&l_err);
            ocmb_control_init();
        }
    }
    else if (!gpe_request_rc)
    {
        ocmb_control_init();
    }

    return;

} // end ocmb_init()


// Function Specification
//
// Name: get_membuf_data_ptr
//
// Description: Returns a pointer to the most up-to-date data for
//              the membuf associated with the specified id.
//              Returns NULL for membuf outside the range of 0 to 7.
//
// End Function Specification
OcmbMemData * get_membuf_data_ptr(const uint8_t i_occ_membuf_id)
{
    //The caller needs to send in a valid OCC membuf id. Since type is uchar
    //so there is no need to check for case less than 0.
    //If membuf id is invalid then returns NULL.
    if( i_occ_membuf_id < MAX_NUM_OCMBS )
    {
        //Returns a pointer to the most up-to-date membuf data.
        return G_membuf_data_ptrs[i_occ_membuf_id];
    }
    else
    {
        //Memory buffer id outside the range
        TRAC_ERR("get_membuf_data_ptr: Invalid OCC membuf id [0x%x]", i_occ_membuf_id);
        return( NULL );
    }
}


uint32_t membuf_configuration_create()
{
    bool rc = 0;
    GpeRequest l_request;

    do
    {
        G_gpe_membuf_config_args.membufConfiguration = &G_membufConfiguration;
        G_gpe_membuf_config_args.mem_type = MEMTYPE_OCMB_DDR4;

        // determine max number of dts there can be
        // this excludes the ubdts which all memory types may have 1 ubdts
        G_gpe_membuf_config_args.max_dts = 0; // default for OCC reading DIMMs via I2C
        if(!IS_I2C_MEM_TYPE(G_sysConfigData.mem_type))
        {
            if(IS_OCM_DDR4_MEM_TYPE(G_sysConfigData.mem_type))
            {
                G_gpe_membuf_config_args.max_dts = NUM_DTS_PER_OCMB_DDR4;
            }
            else if(IS_OCM_DDR5_MEM_TYPE(G_sysConfigData.mem_type))
            {
                G_gpe_membuf_config_args.max_dts = NUM_DTS_PER_OCMB_DDR5;
                G_gpe_membuf_config_args.mem_type = MEMTYPE_OCMB_DDR5;
            }
        }
        else if(IS_OCM_DDR5_MEM_TYPE(G_sysConfigData.mem_type))
            G_gpe_membuf_config_args.mem_type = MEMTYPE_OCMB_DDR5;

        rc = gpe_request_create(
                                &l_request,                 // request
                                &G_async_gpe_queue1,        // gpe queue
                                IPC_ST_MEMBUF_INIT_FUNCID, // Function Id
                                &G_gpe_membuf_config_args, // GPE arg_ptr
                                SSX_SECONDS(5),             // timeout
                                NULL,                       // callback
                                NULL,                       // callback arg
                                ASYNC_REQUEST_BLOCKING);
        if( rc )
        {
            TRAC_ERR("membuf_configuration_create: gpe_request_create failed for"
                     " IPC_ST_MEMBUF_INIT_FUNCID. rc = 0x%08x",rc);
            break;
        }

        TRAC_INFO("membuf_configuration_create: Scheduling request for IPC_ST_MEMBUF_INIT_FUNCID");
        gpe_request_schedule(&l_request);

        TRAC_INFO("membuf_configuration_create: GPE_membuf_configuration_create w/rc=0x%08x",
                  l_request.request.completion_state);

        if(ASYNC_REQUEST_STATE_COMPLETE != l_request.request.completion_state)
        {
            rc = l_request.request.completion_state;

            TRAC_ERR("membuf_configuration_create: IPC_ST_MEMBUF_INIT_FUNCID"
                     " request did not complete.");
            break;
        }

        if (G_gpe_membuf_config_args.error.rc != GPE_RC_SUCCESS)
        {
            rc = G_gpe_membuf_config_args.error.rc;
            TRAC_ERR("membuf_configuration_create: IPC_ST_MEMBUF_INIT_FUNCID"
                     " failed with rc=0x%08x.",
                     rc);
            break;
        }

    } while (0);

    return rc;

} // end membuf_configuration_create()



// Function Specification
//
// Name: ocmb_data
//
// Description: Collect membuf data. The task is used for membuf data collection
//
// End Function Specification
void ocmb_data( void )
{
    errlHndl_t    l_err   = NULL;    // Error handler
    int           rc      = 0;       // Return code
    OcmbMemData  *    l_temp  = NULL;
    membuf_data_task_t * l_membuf_data_ptr = &G_membuf_data_task;
    MemBufGetMemDataParms_t  * l_parms =
        (MemBufGetMemDataParms_t *)(l_membuf_data_ptr->gpe_req.cmd_data);
    static bool   L_gpe_scheduled = FALSE;
    static bool   L_gpe_error_logged = FALSE;
    static bool   L_gpe_had_1_tick = FALSE;

    do
    {
        // ------------------------------------------
        // Data Task Variable Initial State
        // ------------------------------------------
        // ->current_membuf:  The one 'read from' during this tick.
        //
        // ->prev_membuf:     the one that was 'read from' during the last tick
        //                     and will be used to update the
        //                     G_updated_membuf_mask during this tick.
        //
        // ->membuf_data_ptr: points to G_membuf_data_ptrs[] for
        //                     the membuf that is referenced by prev_membuf
        //                     (the one that was just 'read')

        //First, check to see if the previous GPE request still running
        //A request is considered idle if it is not attached to any of the
        //asynchronous request queues
        if( !(async_request_is_idle(&l_membuf_data_ptr->gpe_req.request)) )
        {
            //This may happen due to variability in the time that this
            //task runs.  Don't trace on the first occurrence.
            if( !G_membuf_queue_not_idle_traced && L_gpe_had_1_tick)
            {
                TRAC_INFO("ocmb_data: GPE is still running");
                G_membuf_queue_not_idle_traced = TRUE;
            }
            L_gpe_had_1_tick = TRUE;
            break;
        }
        else
        {
            //Request is idle
            L_gpe_had_1_tick = FALSE;
            if(G_membuf_queue_not_idle_traced)
            {
                TRAC_INFO("ocmb_data: GPE completed");
                G_membuf_queue_not_idle_traced = FALSE;
            }
        }

        //Need to complete collecting data for all assigned membufs from
        //previous interval and tick 0 is the current tick before collect data again.
        if( (l_membuf_data_ptr->current_membuf == l_membuf_data_ptr->end_membuf)
            && ((CURRENT_TICK & (MAX_NUM_TICKS - 1)) != 0) )
        {
            MEM_DBG("Did not collect membuf data. Need to wait for tick.");
            break;
        }

        //Check to see if the previous GPE request has succeeded.
        //A request is not considered complete until both the engine job
        //has finished without error and any callback has run to completion.
        if(L_gpe_scheduled)
        {
            //If the request is idle but not completed then there was an error
            //(as long as the request was scheduled).
            if(!async_request_completed(&l_membuf_data_ptr->gpe_req.request) || l_parms->error.rc )
            {
                // Check if the membuf has a channel checkstop. If it does then do not
                // log any errors, but remove it from the config
                if(l_parms->error.rc == MEMBUF_CHANNEL_CHECKSTOP)
                {
                    disable_membuf(l_membuf_data_ptr->prev_membuf);
                }
                else  // log the error if it was not a MEMBUF_CHANNEL_CHECKSTOP
                {
                    //log an error the first time this happens but keep on running.
                    //This should be informational (except mfg) since we are going to retry
                    //eventually, we will timeout on the dimm & membuf temps not being updated
                    //if this is a hard failure which will call out the membuf at that point.
                    if(!L_gpe_error_logged)
                    {
                        L_gpe_error_logged = TRUE;

                        // There was an error collecting the membuf sensor cache
                        TRAC_ERR("ocmb_data: gpe_get_mem_data failed. rc=0x%08x, cur=%d, prev=%d",
                                 l_parms->error.rc,
                                 l_membuf_data_ptr->current_membuf,
                                 l_membuf_data_ptr->prev_membuf);
                        /* @
                         * @errortype
                         * @moduleid    MEM_MID_TASK_DATA
                         * @reasoncode  MEMBUF_SCOM_ERROR
                         * @userdata1   l_parms->error.rc
                         * @userdata2   0
                         * @userdata4   OCC_NO_EXTENDED_RC
                         * @devdesc     Failed to get membuf data
                         */
                        l_err = createErrl(
                                MEM_MID_TASK_DATA,                      //modId
                                MEMBUF_SCOM_ERROR,                      //reasoncode
                                OCC_NO_EXTENDED_RC,                     //Extended reason code
                                ERRL_SEV_INFORMATIONAL,                 //Severity
                                NULL,                                   //Trace Buf
                                DEFAULT_TRACE_SIZE,                     //Trace Size
                                l_parms->error.rc,                            //userdata1
                                0                                       //userdata2
                                );

                        //force severity to predictive if mfg ipl (allows callout to be added to info error)
                        setErrlActions(l_err, ERRL_ACTIONS_MANUFACTURING_ERROR);

                        addUsrDtlsToErrl(l_err,                                   //io_err
                                (uint8_t *) &(l_membuf_data_ptr->gpe_req.ffdc),  //i_dataPtr,
                                sizeof(GpeFfdc),                                  //i_size
                                ERRL_USR_DTL_STRUCT_VERSION_1,                    //version
                                ERRL_USR_DTL_BINARY_DATA);                        //type


                        // Capture the GPE1 trace buffer
                        addUsrDtlsToErrl(l_err,
                                         (uint8_t *) G_shared_gpe_data.gpe1_tb_ptr,
                                         G_shared_gpe_data.gpe1_tb_sz,
                                         ERRL_USR_DTL_STRUCT_VERSION_1,
                                         ERRL_USR_DTL_TRACE_DATA);

                        //Callouts depend on the return code of the gpe_get_mem_data procedure
                        if(l_parms->error.rc == MEMBUF_GET_MEM_DATA_DIED)
                        {
                            //callout the processor
                            addCalloutToErrl(l_err,
                                             ERRL_CALLOUT_TYPE_HUID,
                                             G_sysConfigData.proc_huid,
                                             ERRL_CALLOUT_PRIORITY_LOW);
                        }
                        else if(l_parms->error.rc == MEMBUF_GET_MEM_DATA_SENSOR_CACHE_FAILED ||
                                l_parms->error.rc == MEMBUF_SCACHE_ERROR)
                        {
                            //callout the previous membuf if present
                            if(MEMBUF_PRESENT(l_membuf_data_ptr->prev_membuf))
                            {
                                addCalloutToErrl(l_err,
                                                 ERRL_CALLOUT_TYPE_HUID,
                                                 G_sysConfigData.membuf_huids[l_membuf_data_ptr->prev_membuf],
                                                 ERRL_CALLOUT_PRIORITY_HIGH);
                            }

                            //callout the processor
                            addCalloutToErrl(l_err,
                                             ERRL_CALLOUT_TYPE_HUID,
                                             G_sysConfigData.proc_huid,
                                             ERRL_CALLOUT_PRIORITY_LOW);
                        }
                        else if(l_parms->error.rc == MEMBUF_GET_MEM_DATA_UPDATE_FAILED)
                        {
                            //callout the current membuf if present
                            if(MEMBUF_PRESENT(l_membuf_data_ptr->current_membuf))
                            {
                                addCalloutToErrl(l_err,
                                                 ERRL_CALLOUT_TYPE_HUID,
                                                 G_sysConfigData.membuf_huids[l_membuf_data_ptr->current_membuf],
                                                 ERRL_CALLOUT_PRIORITY_HIGH);
                            }

                            //callout the processor
                            addCalloutToErrl(l_err,
                                             ERRL_CALLOUT_TYPE_HUID,
                                             G_sysConfigData.proc_huid,
                                             ERRL_CALLOUT_PRIORITY_LOW);
                        }
                        else
                        {
                            //callout the firmware
                            addCalloutToErrl(l_err,
                                             ERRL_CALLOUT_TYPE_COMPONENT_ID,
                                             ERRL_COMPONENT_ID_FIRMWARE,
                                             ERRL_CALLOUT_PRIORITY_MED);
                        }

                        commitErrl(&l_err);
                    }
                }
            }
            else
            {
                //If the previous GPE request succeeded then swap l_membuf_data_ptr
                //with the global one. The gpe routine will write new data into
                //a buffer that is not being accessed by the RTLoop code.
                l_temp = l_membuf_data_ptr->membuf_data_ptr;
                l_membuf_data_ptr->membuf_data_ptr =
                    G_membuf_data_ptrs[l_membuf_data_ptr->current_membuf];
                G_membuf_data_ptrs[l_membuf_data_ptr->prev_membuf] = l_temp;

                //Membuf data has been collected so set the bit in global mask.
                //AMEC code will know which membuf to update sensors for. AMEC is
                //responsible for clearing the bit later on.
                // prev membuf is the one that was just 'read from' in the last tick
                if( MEMBUF_PRESENT(l_membuf_data_ptr->prev_membuf) )
                {
                    G_updated_membuf_mask |= MEMBUF_BY_MASK(l_membuf_data_ptr->prev_membuf);
                }
            }
        }//if(L_gpe_scheduled)

        // If the membuf is not present, then we need to point to the empty G_membuf_data
        // so that we don't use old/stale data from a leftover G_membuf_data
        // (this is very handy for debug...)
        if( !MEMBUF_PRESENT(l_membuf_data_ptr->current_membuf))
        {
            G_membuf_data_ptrs[l_membuf_data_ptr->current_membuf] = &G_membuf_data[MAX_NUM_OCMBS];
        }

        //Update current membuf
        if ( l_membuf_data_ptr->current_membuf >= l_membuf_data_ptr->end_membuf )
        {
            l_membuf_data_ptr->prev_membuf = l_membuf_data_ptr->current_membuf;
            l_membuf_data_ptr->current_membuf = l_membuf_data_ptr->start_membuf;
            l_parms->touch = 1;
        }
        else
        {
            l_membuf_data_ptr->prev_membuf = l_membuf_data_ptr->current_membuf;
            l_membuf_data_ptr->current_membuf++;
            l_parms->touch = 0;
        }

        // ------------------------------------------
        // Membuf Data Task Variable State Changed
        // ------------------------------------------
        // ->current_membuf:  the one that will be 'written' to in order to
        //                     kick off the sensor cache population in the
        //                     membuf.
        //
        // ->prev_membuf:     the one that will be 'read from', meaning have
        //                     the sensor cache transferred from the membuf
        //                     to l_membuf_data_ptr->membuf_data_ptr
        //
        // ->membuf_data_ptr: points to G_membuf_data_ptrs[] for
        //                     the membuf that is referenced by prev_membuf
        //                     (the one that will be 'read')

        //If membuf is not present then skip it. This task assigned to this membuf will
        //be idle during this time it would have collected the data.
        if( MEMBUF_PRESENT(l_membuf_data_ptr->current_membuf)
            || MEMBUF_PRESENT(l_membuf_data_ptr->prev_membuf)
            || l_parms->touch == 1)
        {
            // Setup the 'get membuf data' parms
            // ->config controls which membuf we are reading from
            if( MEMBUF_PRESENT(l_membuf_data_ptr->prev_membuf) ){
              // If prev membuf is present, do the read of the sensor cache
              l_parms->collect = l_membuf_data_ptr->prev_membuf;
            }
            else{
              // If prev membuf is not present, don't do the read of the sensor cache.
              l_parms->collect = -1;
            }

            l_parms->data = (uint64_t *)(l_membuf_data_ptr->membuf_data_ptr);
            l_parms->error.ffdc = 0;

            // Pore flex schedule gpe_get_mem_data
            // Check gpe_request_schedule return code if error
            // then request OCC reset.
            rc = gpe_request_schedule( &(l_membuf_data_ptr->gpe_req) );
            if(rc)
            {
                TRAC_ERR("ocmb_data: gpe_request_schedule failed for membuf data collection. rc=%d", rc);
                /* @
                 * @errortype
                 * @moduleid    MEM_MID_TASK_DATA
                 * @reasoncode  SSX_GENERIC_FAILURE
                 * @userdata1   rc - Return code of failing function
                 * @userdata2   0
                 * @userdata4   ERC_MEMBUF_GPE_REQUEST_SCHEDULE_FAILURE
                 * @devdesc     Failed to get membuf data
                 */
                l_err = createErrl(
                        MEM_MID_TASK_DATA,                      //modId
                        SSX_GENERIC_FAILURE,                    //reasoncode
                        ERC_MEMBUF_GPE_REQUEST_SCHEDULE_FAILURE, //Extended reason code
                        ERRL_SEV_PREDICTIVE,                    //Severity
                        NULL,                                   //Trace Buf
                        DEFAULT_TRACE_SIZE,                     //Trace Size
                        rc,                                     //userdata1
                        l_parms->error.rc                       //userdata2
                        );

                addUsrDtlsToErrl(l_err,                                   //io_err
                        (uint8_t *) &(l_membuf_data_ptr->gpe_req.ffdc),  //i_dataPtr,
                        sizeof(GpeFfdc),                                  //i_size
                        ERRL_USR_DTL_STRUCT_VERSION_1,                    //version
                        ERRL_USR_DTL_BINARY_DATA);                        //type

                REQUEST_RESET(l_err);     //this will add firmware callout
                break;
            }

            L_gpe_scheduled = TRUE;
        }

    }
    while(0);

    return;
} // end ocmb_data()
