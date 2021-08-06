/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe1/gpe1_24x7.c $                                    */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2020                        */
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

#include "pk.h"
#include "pk_app_cfg.h"
#include "ppe42_scom.h"
#include "ipc_api.h"
#include "ipc_async_cmd.h"
#include "gpe_util.h"
#include "gpe_24x7_structs.h"
#include "gpe_pba_cntl.h"
#include "gpe1_24x7.h"
#include "string.h"

//#define DEBUG_24X7

/*
 * Function Specifications:
 *
 * Name: gpe_24x7
 * Description:  24x7 code on the GPE.  Owned by the performance team
 * Inputs:       cmd is a pointer to IPC msg's cmd and cmd_data struct
 * Outputs:      error: sets rc, address, and ffdc in the cmd_data's
 *                      GpeErrorStruct
 *
 * End Function Specification
 */
void gpe_24x7(ipc_msg_t* cmd, void* arg)
{
    // Note: arg was set to 0 in ipc func table (ipc_func_tables.c), so don't use it.
    // the ipc arguments passed through the ipc_msg_t structure, has a pointer
    // to the gpe_24x7_args_t struct.

    uint32_t rc                     = 0;
    ipc_async_cmd_t *async_cmd = (ipc_async_cmd_t*)cmd;
    gpe_24x7_args_t *args      = (gpe_24x7_args_t*)async_cmd->cmd_data;
    GpeErrorStruct *pError      = &(args->error);

    uint8_t ticks = args->numTicksPassed; // number of 500us ticks since last call
    static uint8_t  L_current_state = 1;  // 24x7 collection "state" to execute when called

#ifdef DEBUG_24X7
    PK_TRACE (">> gpe_24x7: Called in with ticks %d", ticks);
#endif

    static uint8_t   L_DELAY_1     = 0;
    static uint8_t   L_DELAY_2     = 0;
    static uint8_t   L_DELAY_3     = 0;
    static uint8_t   L_DELAY_4     = 0;
    static uint8_t   L_DELAY_5     = 0;
    static uint8_t   L_DELAY_6     = 0;
    static uint8_t   L_DELAY_7     = 0;
    static uint8_t   L_CUR_DELAY   = 0;
    static uint64_t  L_cur_speed   = 0;
    static uint64_t  L_prev_status = 0;

    static bool L_configure        = false;
    static bool L_DONT_RUN         = false;
    static bool L_INIT             = false;
    static bool L_PART_INIT        = false;

    //
    //control block memory area.
    //
    static volatile uint64_t* L_status     = (uint64_t*) (CNTL_STATUS_OFFSET | PBA_ENABLE);
    static volatile uint64_t* L_cmd        = (uint64_t*) (CNTL_CMD_OFFSET    | PBA_ENABLE);
    static volatile uint64_t* L_speed      = (uint64_t*) (CNTL_SPEED_OFFSET  | PBA_ENABLE);
    static volatile uint64_t* L_uav        = (uint64_t*) (CNTL_UAV_OFFSET    | PBA_ENABLE);
    static volatile uint64_t* L_mode       = (uint64_t*) (CNTL_MODE_OFFSET   | PBA_ENABLE);
    static volatile uint64_t* L_mode_state = (uint64_t*) (CNTL_MODE_STATE_OFFSET | PBA_ENABLE);

    static volatile uint64_t* L_version      = (uint64_t*) (DBG_VER_OFFSET  | PBA_ENABLE);
    static volatile uint64_t* L_tics_exceded = (uint64_t*) (DBG_TICS_OFFSET | PBA_ENABLE);
    static volatile uint64_t* L_marker       = (uint64_t*) (DBG_MARK        | PBA_ENABLE);
    static volatile uint64_t* L_DBG_ITER     = (uint64_t*) (DBG_ITER        | PBA_ENABLE);
    volatile uint8_t* L_DBG_STATE            = (uint8_t*)  (DBG_STATE       | PBA_ENABLE);
    static volatile uint64_t* L_commit       = (uint64_t*) (DBG_COMMIT      | PBA_ENABLE);

    pError->error = 0; // default success
    pError->ffdc = 0;

    //Populate version details
    //------------------------
    /**
     * 24x7 Version
     * [00:03] - Major revision
     * [04:11] - Minor revision
     * [12:15] - Bug fix release number
     * [16:23] - Day
     * [24:31] - Month
     * [32:47] - Year
     * [48:53] – Major spec version: [48:49] = <0-3> [50:53] = <0-15>
     * [54:55] – Minor spec version: [54:55] = <0-3>
     * [56:63] - Reserved
     **/
    static version_t ver;

    //Populate commit details
    //-----------------------
    /**
     * [00:23] - Change-Id
     * [24:63] - Reserved
     **/
    static commit_t com;

    do
    {
        //PBA Slave setup. Do this each time you enter this loop to be safe.
        gpe_pba_reset();
        if( ticks == 0 )  // First time 24x7 called since OCC started?
        {
            //1. read and update the control block
            //------------------------------------
            PK_TRACE("gpe_24x7: First call since OCC started. ticks = 0");

            //set configure to true
            L_configure = true;
            L_INIT = true;

            //initialize posting area
            rc = initialize_postings (pError);
            if ( rc )
            {
                break;
            }

            //set DMA
            rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_DMA, pError);
            if ( rc )
            {
                PK_TRACE("ERR>> gpe_24x7: PBASLVCTL1_C0040028 ticks=0");
                break;
            }

            //set code version
            ver.val.major      = 0x1;
            ver.val.minor      = 0x0;
            ver.val.bugfix     = 0x2;
            ver.val.day        = 0x06;
            ver.val.month      = 0x08;
            ver.val.year       = 0x2021;
            ver.val.spec_major = 0x1;
            ver.val.spec_minor = 0x1;
            ver.val.dd_level   = 0x2;
            *L_version         = ver.value;

            // set change-Id
            com.val.change_id = 0x0195809b;
            *L_commit         = com.value;

            //set status as initializing
            *L_status = CNTL_STATUS_INIT;

            //initialize cmd to NOP
            *L_cmd = CNTL_CMD_NOP;

            //get speed of collection and set delays accordingly.
            L_cur_speed = *L_speed;
            rc = set_speed (&L_cur_speed,&L_CUR_DELAY,L_status, pError);
            if ( rc )
            {
                PK_TRACE("ERR>> gpe_24x7: set_speed failed!");
                break;
            }

            // @TODO check if below is still needed
            //check if mode is set to monitor
            //support for debug modes (flexible IMA) not present currently.
            if(( *L_mode != CNTL_MODE_MONITOR ) && ( *L_mode != CNTL_MODE_DEBUG1 ))
                *L_status = CNTL_STATUS_ERR2;

            //set Dont run if the speed and mode info is not set to legal values.
            if( ( *L_status == CNTL_STATUS_ERR1 ) || ( *L_status == CNTL_STATUS_ERR2 ) )
                L_DONT_RUN = true;
        }
        else if(ticks > 1)  // longer than 500us since last call?
        {
            // It has been ticks*500us since last call
            PK_TRACE("gpe_24x7: It has been 0x%02X ticks since last call", ticks);
            rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_DMA, pError);
            if ( rc )
            {
                PK_TRACE("ERR>> gpe_24x7: PBASLVCTL1_C0040028, ticks > 1");
                break;
            }

            *L_tics_exceded ++;
        }

        //2. get any new command
        //----------------------
        if ( *L_cmd != CNTL_CMD_NOP )
        {
            rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_DMA, pError);
            if ( rc )
            {
                PK_TRACE("ERR>> gpe_24x7: PBASLVCTL1_C0040028 New Cmd");
                break;
            }

            switch(*L_cmd)
            {
                case CNTL_CMD_PAUSE:
                    L_DONT_RUN = true;
                    *L_status = CNTL_STATUS_PAUSE;
                    *L_cmd = CNTL_CMD_NOP;
                    PK_TRACE("gpe_24x7: in CNTL_CMD_PAUSE");
                    break;
                case CNTL_CMD_RESUME:
                    L_DONT_RUN = false;
                    *L_status = CNTL_STATUS_RUN;
                    *L_cmd = CNTL_CMD_NOP;
                    PK_TRACE("gpe_24x7: in CNTL_CMD_RESUME");
                    break;
                case CNTL_CMD_CLEAR:
                    L_DONT_RUN = false;
                    L_INIT = true;
                    PK_TRACE("gpe_24x7: in CNTL_CMD_CLEAR, L_INIT set to true");
                    *L_cmd = CNTL_CMD_NOP;
                    break;
                default:
                    PK_TRACE ("ERR>> gpe_24x7: Invalid New Command 0x%X",
                              (uint32_t)(*L_cmd&0xffffffff));
                    L_DONT_RUN = true;
                    *L_status = CNTL_STATUS_PAUSE;
                    rc = GPE_RC_24x7_INVALID_CMD;
                    gpe_set_ffdc(pError, 0, GPE_RC_24x7_INVALID_CMD, *L_cmd);
                    break;
            }

            if ( rc )
                break;
        }

        //3.get any new speed setting
        //---------------------------
        if ( *L_speed != L_cur_speed )
        {
            L_INIT = true;
            PK_TRACE("gpe_24x7: speed change, L_INIT set to true");
            rc = set_speed(&L_cur_speed,&L_CUR_DELAY,L_status,pError);
            if ( rc )
            {
                PK_TRACE("ERR>> gpe_24x7: set_speed failed!");
                break;
            }
        }

        //4.check for any system config changes via uav
        //---------------------------------------------
        if ( *L_uav != G_CUR_UAV )
        {
            L_INIT = true;
            L_PART_INIT = true;
            PK_TRACE("gpe_24x7: UAV[0:31] 0x%08X UAV[32:63] 0x%08X",  (G_CUR_UAV>>32),
                                                 (uint32_t)(G_CUR_UAV & 0xFFFFFFFF));
        }

        //5.check for mode change
        //-----------------------
        if ( *L_mode != G_CUR_MODE )
        {
            L_INIT = true;
            L_prev_status = *L_status;
            L_PART_INIT = true;
            L_DONT_RUN = false;
            PK_TRACE("gpe_24x7: mode change, L_INIT set to true");
        }

        if(( *L_mode != CNTL_MODE_MONITOR ) && ( *L_mode != CNTL_MODE_DEBUG1 ))
        {
            *L_status = CNTL_STATUS_ERR2;
        }

        //set Dont run if the speed and mode info is not set to legal values.
        if( ( *L_status == CNTL_STATUS_ERR1 ) || ( *L_status == CNTL_STATUS_ERR2 ) )
        {
            PK_TRACE ("ERR>> gpe_24x7: Speed & Mode not set to legal values! Don't Run");
            L_DONT_RUN = true;
        }

        //initialize postings if required from new cmd or change of speed or UAV change.
        if ( L_INIT )
        {
            rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_DMA, pError);
            if ( rc )
            {
                PK_TRACE("ERR>> gpe_24x7 PBASLVCTL1_C0040028 L_INIT");
                break;
            }

            *L_status = CNTL_STATUS_INIT;

            //need not initialize postings for UAV change.
            if ( L_PART_INIT )
                L_PART_INIT = false;
            else
            {
                rc = initialize_postings (pError);
                if ( rc )
                {
                    PK_TRACE ("ERR>> gpe_24x7: New Cmd/Speed/UAV initialize_posting failed!");
                    break;
                }
            }

            L_configure = true;
            L_cur_speed = *L_speed;

            // Disable PHB UAV for DD-1 as PHB scoms are broken
            // CQ: SW513154
            //G_CUR_UAV   = *L_uav & ~MASK_PHB;

            G_CUR_UAV   = *L_uav;

            G_CUR_MODE  = *L_mode;

            *L_marker = MARKER1;

            rc = set_speed(&L_cur_speed,&L_CUR_DELAY,L_status,pError);
            if ( rc )
            {
                PK_TRACE("ERR>> gpe_24x7: set_speed failed!");
                break;
            }

            //set the state to 1 if reconfig is required. config scoms are split across multiple states starting from 1.
            L_current_state = 1;

            PK_TRACE("gpe_24x7: in L_INIT L_current_state set to 1");
            L_INIT = false;
        }

        //5. Based on the current entry state number, appropriate group posting is done.
        //G1A,G2A (states 1,5)
        //G1B,G2B (states 3,7)
        //G3A     (states 2,6)
        //G3B     (states 4,8)
        //G4      (state  2)
        //G5      (state  4)
        //G6      (state  6)
        //G7      (state  8)
        //G8      (state  8)
        //
        //during first time entry or a re-init is triggered, the current run is used for pmu configuration.
        //configuration will continue across multiple re-entry slots till all configuration scoms are done.
        //scoms are generally kept at 16 per slot, to prevent from exceeding 50us runtime buget.
        if ( L_DONT_RUN == false )
        {
            rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_DMA, pError);
            if ( rc )
            {
                PK_TRACE("ERR>> gpe_24x7: PBASLVCTL1_C0040028 L_DONT_RUN==false");
                break;
            }


#ifdef DEBUG_24X7
            PK_TRACE("gpe_24x7: state = %d ",(int)L_current_state);
#endif
            *L_DBG_STATE = L_current_state;
            switch (L_current_state)
            {
                case 1:
                    if(L_configure)
                    {
                        rc = configure_pmu(L_current_state, L_cur_speed, pError);
                    }
                    else
                    {
                        if(L_DELAY_1 == 0)
                        {
                            rc = post_pmu_events(G1A, pError);
                            if ( rc )
                            {
                                break;
                            }
                            L_DELAY_1 = L_CUR_DELAY;
                        }
                        else
                            L_DELAY_1--;

                        if(L_DELAY_2 == 0)
                        {
                            rc = post_pmu_events(G2A, pError);
                            if ( rc )
                            {
                                break;
                            }
                            L_DELAY_2 = L_CUR_DELAY;
                        }
                        else
                            L_DELAY_2--;
                    }
                    break;

                case 2:
                    if(L_configure)
                    {
                        rc = configure_pmu(L_current_state, L_cur_speed, pError);
                    }
                    else
                    {
                        if(L_DELAY_3 == 0)
                        {
                            rc = post_pmu_events(G3A, pError);
                            if ( rc )
                            {
                                break;
                            }
                            L_DELAY_3 = L_CUR_DELAY;
                        }
                        else
                            L_DELAY_3--;

                        if(L_DELAY_4 == 0)
                        {
                            rc = post_pmu_events(G4, pError);
                            if ( rc )
                            {
                                break;
                            }
                            //for groups 4,5,6,7 8ms is the fastest possible collection speed.
                            L_DELAY_4 = L_CUR_DELAY/8;
                        }
                        else
                            L_DELAY_4--;
                    }
                    break;

                case 3:
                    if(L_configure)
                    {
                        rc = configure_pmu(L_current_state, L_cur_speed, pError);
                    }
                    else
                    {
                        if(L_DELAY_1 == 0)
                        {
                            rc = post_pmu_events(G1B, pError);
                            if ( rc )
                            {
                                break;
                            }
                            L_DELAY_1 = L_CUR_DELAY;
                        }
                        else
                            L_DELAY_1--;

                        if(L_DELAY_2 == 0)
                        {
                            rc = post_pmu_events(G2B, pError);
                            if ( rc )
                            {
                                break;
                            }
                            L_DELAY_2 = L_CUR_DELAY;
                        }
                        else
                            L_DELAY_2--;
                    }
                    break;

                case 4:
                    if(L_configure)
                    {
                        rc = configure_pmu(L_current_state, L_cur_speed, pError);
                    }
                    else
                    {
                        if(L_DELAY_3 == 0)
                        {
                            rc = post_pmu_events(G3B, pError);
                            if ( rc )
                            {
                                break;
                            }
                            L_DELAY_3 = L_CUR_DELAY;
                        }
                        else
                            L_DELAY_3--;

                        if(L_DELAY_5 == 0)
                        {
                            rc = post_pmu_events(G5, pError);
                            if ( rc )
                            {
                                break;
                            }

                            //for groups 4,5,6,7 8ms is the fastest possible collection speed.
                            L_DELAY_5 = L_CUR_DELAY/8;
                        }
                        else
                            L_DELAY_5--;
                    }
                    break;

                case 5:
                    if(L_configure)
                    {
                        rc = configure_pmu(L_current_state, L_cur_speed, pError);
                    }
                    else
                    {
                        if(L_DELAY_1 == 0)
                        {
                            rc = post_pmu_events(G1A, pError);
                            if ( rc )
                            {
                                break;
                            }
                            L_DELAY_1 = L_CUR_DELAY;
                        }
                        else
                            L_DELAY_1--;

                        if(L_DELAY_2 == 0)
                        {
                            rc = post_pmu_events(G2A, pError);
                            if ( rc )
                            {
                                break;
                            }
                            L_DELAY_2 = L_CUR_DELAY;
                        }
                        else
                            L_DELAY_2--;
                    }
                    break;

                case 6:
                    if(L_configure)
                    {
                        rc = configure_pmu(L_current_state, L_cur_speed, pError);
                    }
                    else
                    {
                        if(L_DELAY_3 == 0)
                        {
                            rc = post_pmu_events(G3A, pError);
                            if ( rc )
                            {
                                break;
                            }
                            L_DELAY_3 = L_CUR_DELAY;
                        }
                        else
                            L_DELAY_3--;

                        if(L_DELAY_6 == 0)
                        {
                            rc = post_pmu_events(G6, pError);
                            if ( rc )
                            {
                                break;
                            }
                            //for groups 4,5,6,7 8ms is the fastest possible collection speed.
                            L_DELAY_6 = L_CUR_DELAY/8;
                        }
                        else
                            L_DELAY_6--;
                    }
                    break;

                case 7:
                    if(L_configure)
                    {
                        rc = configure_pmu(L_current_state, L_cur_speed, pError);
                    }
                    else
                    {
                        if(L_DELAY_1 == 0)
                        {
                            rc = post_pmu_events(G1B, pError);
                            if ( rc )
                            {
                                break;
                            }
                            L_DELAY_1 = L_CUR_DELAY;
                        }
                        else
                            L_DELAY_1--;

                        if(L_DELAY_2 == 0)
                        {
                            rc = post_pmu_events(G2B, pError);
                            if ( rc )
                            {
                                break;
                            }
                            L_DELAY_2 = L_CUR_DELAY;
                        }
                        else
                            L_DELAY_2--;
                    }
                    break;

                case 8:
                    if(L_configure)
                    {
                        rc = configure_pmu(L_current_state, L_cur_speed, pError);
                        if ( rc )
                        {
                            break;
                        }

                        L_configure = false;
                        *L_status = CNTL_STATUS_RUN;
                        //*L_status = L_prev_status;
                        if(L_prev_status == CNTL_STATUS_PAUSE)
                        {
                            L_DONT_RUN = true;
                            *L_status = L_prev_status;
                        }
                        *L_mode_state = G_CUR_MODE;
                        //*L_mode = 0;
                        L_prev_status = 0;
                    }
                    else
                    {
                        if(L_DELAY_3 == 0)
                        {
                            rc = post_pmu_events(G3B, pError);
                            if ( rc )
                            {
                                break;
                            }

                            rc = post_pmu_events(G8, pError);
                            if ( rc )
                            {
                                break;
                            }
                            L_DELAY_3 = L_CUR_DELAY;
                        }
                        else
                            L_DELAY_3--;

                        if(L_DELAY_7 == 0)
                        {
                            rc = post_pmu_events(G7, pError);
                            if ( rc )
                            {
                                break;
                            }

                            //for groups 4,5,6,7 8ms is the fastest possible collection speed.
                            L_DELAY_7 = L_CUR_DELAY/8;
                        }
                        else
                            L_DELAY_7--;
                    }
                    break;

                default:
                    PK_TRACE("gpe_24x7: Invalid collection state: 0x%02X", L_current_state);
                    rc = GPE_RC_24x7_INVALID_STATE;
                    gpe_set_ffdc(pError, 0, GPE_RC_24x7_INVALID_STATE, L_current_state);
                    break;
            }

            if ( rc )
                break;
        }
        // Setup state to run on next call
        if(L_current_state == MAX_24x7_STATES)
            L_current_state = 1;
        else
            L_current_state++;
        //PK_TRACE("gpe_24x7: end state = %d",(int)L_current_state);

    } while(0);

    // send back a response, IPC success even if ffdc/rc are non zeros
    rc = ipc_send_rsp(cmd, IPC_RC_SUCCESS);
    //PK_TRACE("gpe_24x7: SRN exiting thread with rc =%d", rc);
    if(rc)
    {
        *L_DBG_ITER = MARKER2;
        PK_TRACE("gpe_24x7: Failed to send response back. Halting GPE1");
        gpe_set_ffdc(pError, 0x00, GPE_RC_IPC_SEND_FAILED, rc);
        pk_halt();
    }
}

/**
 * Function: configure_pmu
 **/
uint32_t configure_pmu(uint8_t state, uint64_t speed, GpeErrorStruct* o_err)
{
    uint32_t rc = 0;
    uint64_t ocmbInstChk, ocmbUAV, val;
    //write the configuration SCOMs for all pmus.
    int i,j,start = (state - 1) * 17,end = state * 17;
    static volatile uint64_t* L_conf_last = (uint64_t*) (DBG_CONF_OFFSET | PBA_ENABLE);
    static volatile uint64_t* L_DBG_UAV   = (uint64_t*) (DBG_UAV | PBA_ENABLE);

#ifdef DEBUG_24X7
    PK_TRACE (">> gpe_24x7: configure_pmu: state: 0x%02X", state);
#endif

    do
    {
        rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_DMA, o_err);
        if ( rc )
        {
            PK_TRACE("configure_pmu: 0x%08x putscom failed. rc = 0x%x", PBASLVCTL1_C0040028, rc);
            break;
        }

        *L_DBG_UAV = G_CUR_UAV;
        if(end > TOTAL_CONFIGS)
            end = TOTAL_CONFIGS;
        for(i = start; i < end; i++)
        {
//check the availability of unit before writing the configurations to the config scoms.
//use the unit wise masks to acertain availability of a unit.
            if( ((i>=0) && (i<40)) && (!(G_CUR_UAV & MASK_PB)) )
                continue;
            else if( ((i>=40) && (i<=43)) && (!(G_CUR_UAV & MASK_MC00) && (!(G_CUR_UAV & MASK_MC01)) ) )
                continue;
            else if( (i==44) && !(G_CUR_UAV & MASK_MC00) )
                continue;
            else if( (i==45) && !(G_CUR_UAV & MASK_MC01) )
                continue;
            else if( ((i>=46) && (i<=49)) && (!(G_CUR_UAV & MASK_MC10) && (!(G_CUR_UAV & MASK_MC11)) ) )
                continue;
            else if( (i==50) && !(G_CUR_UAV & MASK_MC10) )
                continue;
            else if( (i==51) && !(G_CUR_UAV & MASK_MC11) )
                continue;
            else if( ((i>=52) && (i<=55)) && (!(G_CUR_UAV & MASK_MC20) && (!(G_CUR_UAV & MASK_MC21)) ) )
                continue;
            else if( (i==56) && !(G_CUR_UAV & MASK_MC20) )
                continue;
            else if( (i==57) && !(G_CUR_UAV & MASK_MC21) )
                continue;
            else if( ((i>=58) && (i<=61)) && (!(G_CUR_UAV & MASK_MC30) && (!(G_CUR_UAV & MASK_MC31)) ) )
                continue;
            else if( (i==62) && !(G_CUR_UAV & MASK_MC30) )
                continue;
            else if( (i==63) && !(G_CUR_UAV & MASK_MC31) )
                continue;
            else if( ((i==64) || (i==65)) && (!(G_CUR_UAV & MASK_PEC0)) && (!(G_CUR_UAV & MASK_PEC1)) )
                continue;
            else if( ((i==66) || (i==67)) && (!((G_CUR_UAV & MASK_TLPM0) == MASK_XLINK0)) && (!((G_CUR_UAV & MASK_TLPM0) == MASK_ALINK0)) )
                continue;
            else if( ((i==68) || (i==69)) && (!((G_CUR_UAV & MASK_TLPM1) == MASK_XLINK1)) && (!((G_CUR_UAV & MASK_TLPM1) == MASK_ALINK1)) )
                continue;
            else if( ((i==70) || (i==71)) && (!((G_CUR_UAV & MASK_TLPM2) == MASK_XLINK2)) && (!((G_CUR_UAV & MASK_TLPM2) == MASK_ALINK2)) )
                continue;
            else if( ((i==72) || (i==73)) && (!((G_CUR_UAV & MASK_TLPM3) == MASK_XLINK3)) && (!((G_CUR_UAV & MASK_TLPM3) == MASK_ALINK3)) )
                continue;
            else if( ((i==74) || (i==75)) && (!((G_CUR_UAV & MASK_TLPM4) == MASK_XLINK4)) && (!((G_CUR_UAV & MASK_TLPM4) == MASK_ALINK4)) )
                continue;
            else if( ((i==76) || (i==77)) && (!((G_CUR_UAV & MASK_TLPM5) == MASK_XLINK5)) && (!((G_CUR_UAV & MASK_TLPM5) == MASK_ALINK5)) )
                continue;
            else if( ((i==78) || (i==79)) && (!((G_CUR_UAV & MASK_TLPM6) == MASK_XLINK6)) && (!((G_CUR_UAV & MASK_TLPM6) == MASK_ALINK6)) )
                continue;
            else if( ((i==80) || (i==81)) && (!((G_CUR_UAV & MASK_TLPM7) == MASK_XLINK7)) && (!((G_CUR_UAV & MASK_TLPM7) == MASK_ALINK7)) )
                continue;
            else if( ((i>=82) && (i<=87)) && (!((G_CUR_UAV & MASK_TLPM0) == MASK_OCAPI0)) )
                continue;
            else if( ((i>=88) && (i<=93)) && (!((G_CUR_UAV & MASK_TLPM3) == MASK_OCAPI3)) )
                continue;
            else if( ((i>=94) && (i<=99)) && (!((G_CUR_UAV & MASK_TLPM4) == MASK_OCAPI4)) )
                continue;
            else if( ((i>=100) && (i<=105)) && (!((G_CUR_UAV & MASK_TLPM5) == MASK_OCAPI5)) )
                continue;
            else if( ((i>=106) && (i<=111)) && (!((G_CUR_UAV & MASK_TLPM6) == MASK_OCAPI6)) )
                continue;
            else if( ((i>=112) && (i<=117)) && (!((G_CUR_UAV & MASK_TLPM7) == MASK_OCAPI7)) )
                continue;
            // PHYP would be doing the configuration for PHBs and 24x7 would 
            // be directly reading the counters
            else if( (i>=118) && (i<=129) )
                continue;
#if 0
            else if( ((i==118) || (i==119)) && !(G_CUR_UAV & MASK_PHB0) )
                continue;
            else if( ((i==120) || (i==121)) && !(G_CUR_UAV & MASK_PHB1) )
                continue;
            else if( ((i==122) || (i==123)) && !(G_CUR_UAV & MASK_PHB2) )
                continue;
            else if( ((i==124) || (i==125)) && !(G_CUR_UAV & MASK_PHB3) )
                continue;
            else if( ((i==126) || (i==127)) && !(G_CUR_UAV & MASK_PHB4) )
                continue;
            else if( ((i==128) || (i==129)) && !(G_CUR_UAV & MASK_PHB5) )
                continue;
#endif
            else if( (i==130) && !(G_CUR_UAV & MASK_OCMB) )
                continue;
            else if( ((i>=131) && (i<=134)) && !(G_CUR_UAV & MASK_NX) )
                continue;

            else if( i>=135 ) //Not supported
                continue;
            else
            {
                //Only single set of configuration(pre-scaler):CNPM-10 bit + MCS-12bit + All other units - 16 bit
                //for speeds >=1MS && <=2048MS
                if((speed >= CNTL_SPEED_1MS) && (speed <= CNTL_SPEED_2048MS))
                {
                    if ( i != 130 )
                    {
                        // For defect SW526292 - Write only bits [24:30] for 24x7 usage
                        if ( i >= 24 && i <= 39 )
                        {
                            rc = getScom (G_PMU_CONFIGS_8[i][0], &val, o_err);
                            if ( rc )
                            {
                                PK_TRACE("configure_pmu:: 0x%08x getscom failed. rc = 0x%x", G_PMU_CONFIGS_8[i][0], rc);
                                break;
                            }
                            val &= (~0x000000FE00000000);
                            val |= G_PMU_CONFIGS_8[i][1];
                            rc = putScom (G_PMU_CONFIGS_8[i][0], val, o_err);
                        }
                        else
                        {
                            rc = putScom (G_PMU_CONFIGS_8[i][0], G_PMU_CONFIGS_8[i][1], o_err);
                        }
                        if ( rc )
                        {
                            PK_TRACE("configure_pmu: 0x%08x putscom failed. rc = 0x%x", G_PMU_CONFIGS_8[i][0], rc);
                            break;
                        }
                    }
                    else
                    {
                        //Configure OCMB
                        ocmbInstChk = MASK_OCMB0;
                        ocmbUAV     = (G_CUR_UAV & MASK_OCMB);
                        if ( ocmbUAV && (G_membuf_config != NULL) )
                        {
                            for ( j=0; j<16; j++ )
                            {
                                if ( ocmbUAV & ocmbInstChk )
                                {
                                    // Write OCMB scom
                                    rc = membuf_put_scom(G_membuf_config,
                                                         j,
                                                         G_PMU_CONFIGS_8[i][0],
                                                         G_PMU_CONFIGS_8[i][1]);
                                    if ( rc )
                                    {
                                        PK_TRACE("configure_pmu:: 0x%08x 0x%08x membuf_put_scom failed. rc = 0x%x", G_membuf_config->baseAddress[j], G_PMU_CONFIGS_8[i][0], rc);
                                        break;
                                    }
                                }
                                ocmbInstChk >>= 1;
                            }
                            if ( rc )
                            {
                                break;
                            }
                        }
                        else
                        {
                            PK_TRACE("configure_pmu::Either OCMB is NOT enabled OR G_membuf_config is NULL");
                        }
                    }

                    *L_conf_last = (uint64_t)i;
                }
            }
        } //for(i = start; i < end; i++)
    }while(0);

#ifdef DEBUG_24X7
    PK_TRACE ("<< configure_pmu");
#endif
    return rc;
}

/**
 * function: post_pmu_events
 **/
uint32_t post_pmu_events (int grp, GpeErrorStruct* o_err)
{
#ifdef DEBUG_24X7
    PK_TRACE (">> post_pmu_events: Group %d", grp);
#endif

    uint32_t rc = 0;

    //read the scom pmulets. split/extract the counters.accumulate to main memory.
    volatile uint64_t* post_addr;
    static volatile uint64_t* L_DBG_GRP = (uint64_t*) (DBG_GRP_OFFSET | PBA_ENABLE);
    static volatile uint64_t* L_DBG_UNIT = (uint64_t*) (DBG_UNIT_OFFSET | PBA_ENABLE);

    //union to split a pmulet containg 4 counters into its constituents.
    union u1
    {
        struct event
        {
            uint16_t e[4];
        } ev;
        uint64_t pmulet;
    } u3;

    //initialize local vars
    int i=0,j=0;


    do
    {
        //set the PBA
        post_addr = (uint64_t*) (POSTING_START | PBA_ENABLE);
        rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_DMA, o_err);
        if ( rc )
        {
            PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
            break;
        }

        switch (grp)
        {
            case G1A://cnpm group - always written
                if(G_CUR_MODE == CNTL_MODE_MONITOR)
                    post_addr = (uint64_t*) (POST_OFFSET_G1AH | PBA_ENABLE);
                else if(G_CUR_MODE == CNTL_MODE_DEBUG1)
                    post_addr = (uint64_t*) (POST_OFFSET_DBG1AH | PBA_ENABLE);
                *L_DBG_GRP = G1A;
                *L_DBG_UNIT = 1;
                rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_ATOMIC, o_err);
                if ( rc )
                {
                    PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                    break;
                }

                *post_addr = INC_UPD_COUNT;
                post_addr++;
                for(i=0; i<8; i++)
                {
                    rc = getScom (G_PMULETS_1A[i], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMULETS_1A[i], rc);
                        break;
                    }

                    for(j=0; j<4; j++)
                    {
                        // PB unit is always avaiable and its events always populated
                        // so no need for PB unit check
                        if ( ( i == 0 && j == 1 && !( (G_CUR_UAV & MASK_MC00) || (G_CUR_UAV & MASK_MC01) ) ) ||
                             ( i == 0 && j == 2 && !( (G_CUR_UAV & MASK_MC00) || (G_CUR_UAV & MASK_MC01) ) ) ||
                             ( i == 1 && j == 1 && !( (G_CUR_UAV & MASK_MC10) || (G_CUR_UAV & MASK_MC11) ) ) ||
                             ( i == 1 && j == 2 && !( (G_CUR_UAV & MASK_MC10) || (G_CUR_UAV & MASK_MC11) ) ) ||
                             ( i == 4 && j == 1 && !( (G_CUR_UAV & MASK_MC20) || (G_CUR_UAV & MASK_MC21) ) ) ||
                             ( i == 4 && j == 2 && !( (G_CUR_UAV & MASK_MC20) || (G_CUR_UAV & MASK_MC21) ) ) ||
                             ( i == 5 && j == 1 && !( (G_CUR_UAV & MASK_MC30) || (G_CUR_UAV & MASK_MC31) ) ) ||
                             ( i == 5 && j == 2 && !( (G_CUR_UAV & MASK_MC30) || (G_CUR_UAV & MASK_MC31) ) ) ||
                             ( i == 0 && j == 3 && !(G_CUR_UAV & MASK_PEC0) ) ||
                             ( i == 4 && j == 3 && !(G_CUR_UAV & MASK_PEC1) ) )
                        {
                            post_addr++;
                            continue;
                        }
                        *post_addr = (uint64_t)u3.ev.e[j];
                        post_addr++;
                    }
                }
                if ( rc )
                {
                    break;
                }

                if(G_CUR_MODE == CNTL_MODE_MONITOR)
                    post_addr = (uint64_t*) (POST_OFFSET_G1AT | PBA_ENABLE);
                else if(G_CUR_MODE == CNTL_MODE_DEBUG1)
                    post_addr = (uint64_t*) (POST_OFFSET_DBG1AT | PBA_ENABLE);
                *post_addr = INC_UPD_COUNT;
                break;

            case G1B://cnpm group - always written
                if(G_CUR_MODE == CNTL_MODE_MONITOR)
                    post_addr = (uint64_t*) (POST_OFFSET_G1BH | PBA_ENABLE);
                else if(G_CUR_MODE == CNTL_MODE_DEBUG1)
                    post_addr = (uint64_t*) (POST_OFFSET_DBG1BH | PBA_ENABLE);
                *L_DBG_GRP = G1B;
                *L_DBG_UNIT = 1;
                rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_ATOMIC, o_err);
                if ( rc )
                {
                    PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                    break;
                }

                *post_addr = INC_UPD_COUNT;
                post_addr++;
                for(i=0; i<8; i++)
                {
                    rc = getScom (G_PMULETS_1B[i], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMULETS_1B[i], rc);
                        break;
                    }

                    for(j=0; j<4; j++)
                    {
                        *post_addr = (uint64_t)u3.ev.e[j];
                        post_addr++;
                    }
                }
                if ( rc )
                {
                    break;
                }

                if(G_CUR_MODE == CNTL_MODE_MONITOR)
                    post_addr = (uint64_t*) (POST_OFFSET_G1BT | PBA_ENABLE);
                else if(G_CUR_MODE == CNTL_MODE_DEBUG1)
                    post_addr = (uint64_t*) (POST_OFFSET_DBG1BT | PBA_ENABLE);
                *post_addr = INC_UPD_COUNT;
                break;

            case G2A://XLINKS and ALINKS - [0:3]. Read scoms based on availability.
                *L_DBG_GRP = G2A;
                if ( ((G_CUR_UAV & MASK_TLPM0) == MASK_XLINK0) ||
                     ((G_CUR_UAV & MASK_TLPM1) == MASK_XLINK1) ||
                     ((G_CUR_UAV & MASK_TLPM2) == MASK_XLINK2) ||
                     ((G_CUR_UAV & MASK_TLPM3) == MASK_XLINK3) )
                {
                    post_addr = (uint64_t*) (POST_OFFSET_G2A_X_H | PBA_ENABLE);
                }
                else
                {
                    post_addr = (uint64_t*) (POST_OFFSET_G2A_A_H | PBA_ENABLE);
                }
                rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_ATOMIC, o_err);
                if ( rc )
                {
                    PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                    break;
                }

                *post_addr = INC_UPD_COUNT;
                post_addr++;

                if ( ((G_CUR_UAV & MASK_TLPM0) == MASK_XLINK0) ||
                     ((G_CUR_UAV & MASK_TLPM0) == MASK_ALINK0) )
                {
                    rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_DMA, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                        break;
                    }

                    *L_DBG_UNIT = 1;
                    if ( (G_CUR_UAV & MASK_TLPM0) == MASK_XLINK0 )
                    {
                        post_addr = (uint64_t*) (POST_OFFSET_G2A_X_0 | PBA_ENABLE);
                    }
                    else
                    {
                        post_addr = (uint64_t*) (POST_OFFSET_G2A_A_0 | PBA_ENABLE);
                    }
                    rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_ATOMIC, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                        break;
                    }

                    rc = getScom (G_PMULETS_2A[0], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMULETS_2A[0], rc);
                        break;
                    }

                    // Read counter do not reset when read.
                    // Explicitly value 0 written to read counter after reading them.
                    rc = putScom (G_PMULETS_2A[0], 0, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x putscom failed. rc = 0x%x", G_PMULETS_2A[0], rc);
                        break;
                    }

                    for(j=0; j<4; j++)
                    {
                        *post_addr = (uint64_t)u3.ev.e[j];
                        post_addr++;
                    }

                    rc = getScom (G_PMULETS_2A[1], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMULETS_2A[1], rc);
                        break;
                    }

                    // Read counter do not reset when read.
                    // Explicitly value 0 written to read counter after reading them.
                    rc = putScom (G_PMULETS_2A[1], 0, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x putscom failed. rc = 0x%x", G_PMULETS_2A[1], rc);
                        break;
                    }

                    for(j=0; j<4; j++)
                    {
                        *post_addr = (uint64_t)u3.ev.e[j];
                        post_addr++;
                    }
                }

                if ( ((G_CUR_UAV & MASK_TLPM1) == MASK_XLINK1) ||
                     ((G_CUR_UAV & MASK_TLPM1) == MASK_ALINK1) )
                {
                    rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_DMA, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                        break;
                    }

                    *L_DBG_UNIT = 2;
                    if ( (G_CUR_UAV & MASK_TLPM1) == MASK_XLINK1 )
                    {
                        post_addr = (uint64_t*) (POST_OFFSET_G2A_X_1 | PBA_ENABLE);
                    }
                    else
                    {
                        post_addr = (uint64_t*) (POST_OFFSET_G2A_A_1 | PBA_ENABLE);
                    }
                    rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_ATOMIC, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                        break;
                    }

                    rc = getScom (G_PMULETS_2A[2], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMULETS_2A[2], rc);
                        break;
                    }

                    // Read counter do not reset when read.
                    // Explicitly value 0 written to read counter after reading them.
                    rc = putScom (G_PMULETS_2A[2], 0, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x putscom failed. rc = 0x%x", G_PMULETS_2A[2], rc);
                        break;
                    }

                    for(j=0; j<4; j++)
                    {
                        *post_addr = (uint64_t)u3.ev.e[j];
                        post_addr++;
                    }

                    rc = getScom (G_PMULETS_2A[3], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMULETS_2A[3], rc);
                        break;
                    }

                    // Read counter do not reset when read.
                    // Explicitly value 0 written to read counter after reading them.
                    rc = putScom (G_PMULETS_2A[3], 0, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x putscom failed. rc = 0x%x", G_PMULETS_2A[3], rc);
                        break;
                    }

                    for(j=0; j<4; j++)
                    {
                        *post_addr = (uint64_t)u3.ev.e[j];
                        post_addr++;
                    }
                }

                if ( ((G_CUR_UAV & MASK_TLPM2) == MASK_XLINK2) ||
                        ((G_CUR_UAV & MASK_TLPM2) == MASK_ALINK2) )
                {
                    rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_DMA, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                        break;
                    }

                    *L_DBG_UNIT = 3;
                    if ( (G_CUR_UAV & MASK_TLPM2) == MASK_XLINK2 )
                    {
                        post_addr = (uint64_t*) (POST_OFFSET_G2A_X_2 | PBA_ENABLE);
                    }
                    else
                    {
                        post_addr = (uint64_t*) (POST_OFFSET_G2A_A_2 | PBA_ENABLE);
                    }
                    rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_ATOMIC, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                        break;
                    }

                    rc = getScom (G_PMULETS_2A[4], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMULETS_2A[4], rc);
                        break;
                    }

                    // Read counter do not reset when read.
                    // Explicitly value 0 written to read counter after reading them.
                    rc = putScom (G_PMULETS_2A[4], 0, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x putscom failed. rc = 0x%x", G_PMULETS_2A[4], rc);
                        break;
                    }

                    for(j=0; j<4; j++)
                    {
                        *post_addr = (uint64_t)u3.ev.e[j];
                        post_addr++;
                    }

                    rc = getScom (G_PMULETS_2A[5], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMULETS_2A[5], rc);
                        break;
                    }

                    // Read counter do not reset when read.
                    // Explicitly value 0 written to read counter after reading them.
                    rc = putScom (G_PMULETS_2A[5], 0, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x putscom failed. rc = 0x%x", G_PMULETS_2A[5], rc);
                        break;
                    }

                    for(j=0; j<4; j++)
                    {
                        *post_addr = (uint64_t)u3.ev.e[j];
                        post_addr++;
                    }
                }

                if ( ((G_CUR_UAV & MASK_TLPM3) == MASK_XLINK3) ||
                        ((G_CUR_UAV & MASK_TLPM3) == MASK_ALINK3) )
                {
                    rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_DMA, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                        break;
                    }

                    *L_DBG_UNIT = 4;
                    if ( (G_CUR_UAV & MASK_TLPM3) == MASK_XLINK3 )
                    {
                        post_addr = (uint64_t*) (POST_OFFSET_G2A_X_3 | PBA_ENABLE);
                    }
                    else
                    {
                        post_addr = (uint64_t*) (POST_OFFSET_G2A_A_3 | PBA_ENABLE);
                    }
                    rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_ATOMIC, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                        break;
                    }

                    rc = getScom (G_PMULETS_2A[6], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMULETS_2A[6], rc);
                        break;
                    }

                    // Read counter do not reset when read.
                    // Explicitly value 0 written to read counter after reading them.
                    rc = putScom (G_PMULETS_2A[6], 0, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x putscom failed. rc = 0x%x", G_PMULETS_2A[6], rc);
                        break;
                    }

                    for(j=0; j<4; j++)
                    {
                        *post_addr = (uint64_t)u3.ev.e[j];
                        post_addr++;
                    }

                    rc = getScom (G_PMULETS_2A[7], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMULETS_2A[7], rc);
                        break;
                    }

                    // Read counter do not reset when read.
                    // Explicitly value 0 written to read counter after reading them.
                    rc = putScom (G_PMULETS_2A[7], 0, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x putscom failed. rc = 0x%x", G_PMULETS_2A[7], rc);
                        break;
                    }

                    for(j=0; j<4; j++)
                    {
                        *post_addr = (uint64_t)u3.ev.e[j];
                        post_addr++;
                    }
                }

                if ( ((G_CUR_UAV & MASK_TLPM0) == MASK_XLINK0) ||
                        ((G_CUR_UAV & MASK_TLPM1) == MASK_XLINK1) ||
                        ((G_CUR_UAV & MASK_TLPM2) == MASK_XLINK2) ||
                        ((G_CUR_UAV & MASK_TLPM3) == MASK_XLINK3) )
                {
                    post_addr = (uint64_t*) (POST_OFFSET_G2A_X_T | PBA_ENABLE);
                }
                else
                {
                    post_addr = (uint64_t*) (POST_OFFSET_G2A_A_T | PBA_ENABLE);
                }
                *post_addr = INC_UPD_COUNT;
                break;

            case G2B://XLINKS and ALINKS - [4:7]. Read scoms based on availability.
                *L_DBG_GRP = G2B;
                if ( ((G_CUR_UAV & MASK_TLPM4) == MASK_XLINK4) ||
                        ((G_CUR_UAV & MASK_TLPM5) == MASK_XLINK5) ||
                        ((G_CUR_UAV & MASK_TLPM6) == MASK_XLINK6) ||
                        ((G_CUR_UAV & MASK_TLPM7) == MASK_XLINK7) )
                {
                    post_addr = (uint64_t*) (POST_OFFSET_G2B_X_H | PBA_ENABLE);
                }
                else
                {
                    post_addr = (uint64_t*) (POST_OFFSET_G2B_A_H | PBA_ENABLE);
                }
                rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_ATOMIC, o_err);
                if ( rc )
                {
                    PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                    break;
                }

                *post_addr = INC_UPD_COUNT;
                post_addr++;

                if ( ((G_CUR_UAV & MASK_TLPM4) == MASK_XLINK4) ||
                        ((G_CUR_UAV & MASK_TLPM4) == MASK_ALINK4) )
                {
                    rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_DMA, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                        break;
                    }

                    *L_DBG_UNIT = 1;
                    if ( (G_CUR_UAV & MASK_TLPM4) == MASK_XLINK4 )
                    {
                        post_addr = (uint64_t*) (POST_OFFSET_G2B_X_4 | PBA_ENABLE);
                    }
                    else
                    {
                        post_addr = (uint64_t*) (POST_OFFSET_G2B_A_4 | PBA_ENABLE);
                    }
                    rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_ATOMIC, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                        break;
                    }

                    rc = getScom (G_PMULETS_2B[0], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMULETS_2B[0], rc);
                        break;
                    }

                    // Read counter do not reset when read.
                    // Explicitly value 0 written to read counter after reading them.
                    rc = putScom (G_PMULETS_2B[0], 0, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x putscom failed. rc = 0x%x", G_PMULETS_2B[0], rc);
                        break;
                    }

                    for(j=0; j<4; j++)
                    {
                        *post_addr = (uint64_t)u3.ev.e[j];
                        post_addr++;
                    }

                    rc = getScom (G_PMULETS_2B[1], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMULETS_2B[1], rc);
                        break;
                    }

                    // Read counter do not reset when read.
                    // Explicitly value 0 written to read counter after reading them.
                    rc = putScom (G_PMULETS_2B[1], 0, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x putscom failed. rc = 0x%x", G_PMULETS_2B[1], rc);
                        break;
                    }

                    for(j=0; j<4; j++)
                    {
                        *post_addr = (uint64_t)u3.ev.e[j];
                        post_addr++;
                    }
                }

                if ( ((G_CUR_UAV & MASK_TLPM5) == MASK_XLINK5) ||
                        ((G_CUR_UAV & MASK_TLPM5) == MASK_ALINK5) )
                {
                    rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_DMA, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                        break;
                    }

                    *L_DBG_UNIT = 2;
                    if ( (G_CUR_UAV & MASK_TLPM5) == MASK_XLINK5 )
                    {
                        post_addr = (uint64_t*) (POST_OFFSET_G2B_X_5 | PBA_ENABLE);
                    }
                    else
                    {
                        post_addr = (uint64_t*) (POST_OFFSET_G2B_A_5 | PBA_ENABLE);
                    }
                    rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_ATOMIC, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                        break;
                    }

                    rc = getScom (G_PMULETS_2B[2], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMULETS_2B[2], rc);
                        break;
                    }

                    // Read counter do not reset when read.
                    // Explicitly value 0 written to read counter after reading them.
                    rc = putScom (G_PMULETS_2B[2], 0, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x putscom failed. rc = 0x%x", G_PMULETS_2B[2], rc);
                        break;
                    }

                    for(j=0; j<4; j++)
                    {
                        *post_addr = (uint64_t)u3.ev.e[j];
                        post_addr++;
                    }

                    rc = getScom (G_PMULETS_2B[3], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMULETS_2B[3], rc);
                        break;
                    }

                    // Read counter do not reset when read.
                    // Explicitly value 0 written to read counter after reading them.
                    rc = putScom (G_PMULETS_2B[3], 0, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x putscom failed. rc = 0x%x", G_PMULETS_2B[3], rc);
                        break;
                    }

                    for(j=0; j<4; j++)
                    {
                        *post_addr = (uint64_t)u3.ev.e[j];
                        post_addr++;
                    }
                }

                if ( ((G_CUR_UAV & MASK_TLPM6) == MASK_XLINK6) ||
                        ((G_CUR_UAV & MASK_TLPM6) == MASK_ALINK6) )
                {
                    rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_DMA, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                        break;
                    }

                    *L_DBG_UNIT = 3;
                    if ( (G_CUR_UAV & MASK_TLPM6) == MASK_XLINK6 )
                    {
                        post_addr = (uint64_t*) (POST_OFFSET_G2B_X_6 | PBA_ENABLE);
                    }
                    else
                    {
                        post_addr = (uint64_t*) (POST_OFFSET_G2B_A_6 | PBA_ENABLE);
                    }
                    rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_ATOMIC, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                        break;
                    }

                    rc = getScom (G_PMULETS_2B[4], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMULETS_2B[4], rc);
                        break;
                    }

                    // Read counter do not reset when read.
                    // Explicitly value 0 written to read counter after reading them.
                    rc = putScom (G_PMULETS_2B[4], 0, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x putscom failed. rc = 0x%x", G_PMULETS_2B[4], rc);
                        break;
                    }

                    for(j=0; j<4; j++)
                    {
                        *post_addr = (uint64_t)u3.ev.e[j];
                        post_addr++;
                    }

                    rc = getScom (G_PMULETS_2B[5], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMULETS_2B[5], rc);
                        break;
                    }

                    // Read counter do not reset when read.
                    // Explicitly value 0 written to read counter after reading them.
                    rc = putScom (G_PMULETS_2B[5], 0, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x putscom failed. rc = 0x%x", G_PMULETS_2B[5], rc);
                        break;
                    }

                    for(j=0; j<4; j++)
                    {
                        *post_addr = (uint64_t)u3.ev.e[j];
                        post_addr++;
                    }
                }

                if ( ((G_CUR_UAV & MASK_TLPM7) == MASK_XLINK7) ||
                        ((G_CUR_UAV & MASK_TLPM7) == MASK_ALINK7) )
                {
                    rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_DMA, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                        break;
                    }

                    *L_DBG_UNIT = 4;
                    if ( (G_CUR_UAV & MASK_TLPM7) == MASK_XLINK7 )
                    {
                        post_addr = (uint64_t*) (POST_OFFSET_G2B_X_7 | PBA_ENABLE);
                    }
                    else
                    {
                        post_addr = (uint64_t*) (POST_OFFSET_G2B_A_7 | PBA_ENABLE);
                    }
                    rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_ATOMIC, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                        break;
                    }

                    rc = getScom (G_PMULETS_2B[6], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMULETS_2B[6], rc);
                        break;
                    }

                    // Read counter do not reset when read.
                    // Explicitly value 0 written to read counter after reading them.
                    rc = putScom (G_PMULETS_2B[6], 0, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x putscom failed. rc = 0x%x", G_PMULETS_2B[6], rc);
                        break;
                    }

                    for(j=0; j<4; j++)
                    {
                        *post_addr = (uint64_t)u3.ev.e[j];
                        post_addr++;
                    }

                    rc = getScom (G_PMULETS_2B[7], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMULETS_2B[7], rc);
                        break;
                    }

                    // Read counter do not reset when read.
                    // Explicitly value 0 written to read counter after reading them.
                    rc = putScom (G_PMULETS_2B[7], 0, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x putscom failed. rc = 0x%x", G_PMULETS_2B[7], rc);
                        break;
                    }

                    for(j=0; j<4; j++)
                    {
                        *post_addr = (uint64_t)u3.ev.e[j];
                        post_addr++;
                    }
                }

                if ( ((G_CUR_UAV & MASK_TLPM4) == MASK_XLINK4) ||
                        ((G_CUR_UAV & MASK_TLPM5) == MASK_XLINK5) ||
                        ((G_CUR_UAV & MASK_TLPM6) == MASK_XLINK6) ||
                        ((G_CUR_UAV & MASK_TLPM7) == MASK_XLINK7) )
                {
                    post_addr = (uint64_t*) (POST_OFFSET_G2B_X_T | PBA_ENABLE);
                }
                else
                {
                    post_addr = (uint64_t*) (POST_OFFSET_G2B_A_T | PBA_ENABLE);
                }
                *post_addr = INC_UPD_COUNT;
                break;

            case G3A://OCAPI - [0,3,4,5]
                *L_DBG_GRP = G3A;
                post_addr = (uint64_t*) (POST_OFFSET_G3A_O_H | PBA_ENABLE);
                rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_ATOMIC, o_err);
                if ( rc )
                {
                    PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                    break;
                }

                *post_addr = INC_UPD_COUNT;
                post_addr++;
                if ( (G_CUR_UAV & MASK_TLPM0) == MASK_OCAPI0 )
                {
                    rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_DMA, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                        break;
                    }

                    *L_DBG_UNIT = 1;
                    post_addr = (uint64_t*) (POST_OFFSET_G3A_O_0 | PBA_ENABLE);
                    rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_ATOMIC, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                        break;
                    }

                    rc = getScom (G_PMULETS_3A[0], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMULETS_3A[0], rc);
                        break;
                    }

                    // Read counter do not reset when read.
                    // Perform read to corresponding config register to reset the read counter.
                    rc = getScom (G_PMU_CONFIGS_8[82][0], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMU_CONFIGS_8[82][0], rc);
                        break;
                    }

                    for(j=0; j<4; j++)
                    {
                        *post_addr = (uint64_t)u3.ev.e[j];
                        post_addr++;
                    }

                    rc = getScom (G_PMULETS_3A[1], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMULETS_3A[1], rc);
                        break;
                    }

                    // Read counter do not reset when read.
                    // Perform read to corresponding config register to reset the read counter.
                    rc = getScom (G_PMU_CONFIGS_8[85][0], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMU_CONFIGS_8[85][0], rc);
                        break;
                    }

                    for(j=0; j<4; j++)
                    {
                        *post_addr = (uint64_t)u3.ev.e[j];
                        post_addr++;
                    }
                }

                if ( (G_CUR_UAV & MASK_TLPM3) == MASK_OCAPI3 )
                {
                    rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_DMA, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                        break;
                    }

                    *L_DBG_UNIT = 2;
                    post_addr = (uint64_t*) (POST_OFFSET_G3A_O_3 | PBA_ENABLE);
                    rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_ATOMIC, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                        break;
                    }

                    rc = getScom (G_PMULETS_3A[2], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMULETS_3A[2], rc);
                        break;
                    }

                    // Read counter do not reset when read.
                    // Perform read to corresponding config register to reset the read counter.
                    rc = getScom (G_PMU_CONFIGS_8[88][0], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMU_CONFIGS_8[88][0], rc);
                        break;
                    }

                    for(j=0; j<4; j++)
                    {
                        *post_addr = (uint64_t)u3.ev.e[j];
                        post_addr++;
                    }

                    rc = getScom (G_PMULETS_3A[3], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMULETS_3A[3], rc);
                        break;
                    }

                    // Read counter do not reset when read.
                    // Perform read to corresponding config register to reset the read counter.
                    rc = getScom (G_PMU_CONFIGS_8[91][0], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMU_CONFIGS_8[91][0], rc);
                        break;
                    }

                    for(j=0; j<4; j++)
                    {
                        *post_addr = (uint64_t)u3.ev.e[j];
                        post_addr++;
                    }
                }

                if ( (G_CUR_UAV & MASK_TLPM4) == MASK_OCAPI4 )
                {
                    rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_DMA, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                        break;
                    }

                    *L_DBG_UNIT = 3;
                    post_addr = (uint64_t*) (POST_OFFSET_G3A_O_4 | PBA_ENABLE);
                    rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_ATOMIC, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                        break;
                    }

                    rc = getScom (G_PMULETS_3A[4], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMULETS_3A[4], rc);
                        break;
                    }

                    // Read counter do not reset when read.
                    // Perform read to corresponding config register to reset the read counter.
                    rc = getScom (G_PMU_CONFIGS_8[94][0], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMU_CONFIGS_8[94][0], rc);
                        break;
                    }

                    for(j=0; j<4; j++)
                    {
                        *post_addr = (uint64_t)u3.ev.e[j];
                        post_addr++;
                    }

                    rc = getScom (G_PMULETS_3A[5], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMULETS_3A[5], rc);
                        break;
                    }

                    // Read counter do not reset when read.
                    // Perform read to corresponding config register to reset the read counter.
                    rc = getScom (G_PMU_CONFIGS_8[97][0], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMU_CONFIGS_8[97][0], rc);
                        break;
                    }

                    for(j=0; j<4; j++)
                    {
                        *post_addr = (uint64_t)u3.ev.e[j];
                        post_addr++;
                    }
                }

                if ( (G_CUR_UAV & MASK_TLPM5) == MASK_OCAPI5 )
                {
                    rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_DMA, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                        break;
                    }

                    *L_DBG_UNIT = 4;
                    post_addr = (uint64_t*) (POST_OFFSET_G3A_O_5 | PBA_ENABLE);
                    rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_ATOMIC, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                        break;
                    }

                    rc = getScom (G_PMULETS_3A[6], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMULETS_3A[6], rc);
                        break;
                    }

                    // Read counter do not reset when read.
                    // Perform read to corresponding config register to reset the read counter.
                    rc = getScom (G_PMU_CONFIGS_8[100][0], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMU_CONFIGS_8[100][0], rc);
                        break;
                    }

                    for(j=0; j<4; j++)
                    {
                        *post_addr = (uint64_t)u3.ev.e[j];
                        post_addr++;
                    }

                    rc = getScom (G_PMULETS_3A[7], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMULETS_3A[7], rc);
                        break;
                    }

                    // Read counter do not reset when read.
                    // Perform read to corresponding config register to reset the read counter.
                    rc = getScom (G_PMU_CONFIGS_8[103][0], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMU_CONFIGS_8[103][0], rc);
                        break;
                    }

                    for(j=0; j<4; j++)
                    {
                        *post_addr = (uint64_t)u3.ev.e[j];
                        post_addr++;
                    }
                }

                post_addr = (uint64_t*) (POST_OFFSET_G3A_O_T | PBA_ENABLE);
                *post_addr = INC_UPD_COUNT;
                break;

            case G3B://OCAPI - [6,7]
                *L_DBG_GRP = G3B;
                post_addr = (uint64_t*) (POST_OFFSET_G3B_O_H | PBA_ENABLE);
                rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_ATOMIC, o_err);
                if ( rc )
                {
                    PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                    break;
                }

                *post_addr = INC_UPD_COUNT;
                post_addr++;
                if ( (G_CUR_UAV & MASK_TLPM6) == MASK_OCAPI6 )
                {
                    rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_DMA, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                        break;
                    }

                    *L_DBG_UNIT = 1;
                    post_addr = (uint64_t*) (POST_OFFSET_G3B_O_6 | PBA_ENABLE);
                    rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_ATOMIC, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                        break;
                    }

                    rc = getScom (G_PMULETS_3B[0], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMULETS_3B[0], rc);
                        break;
                    }

                    // Read counter do not reset when read.
                    // Perform read to corresponding config register to reset the read counter.
                    rc = getScom (G_PMU_CONFIGS_8[106][0], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMU_CONFIGS_8[106][0], rc);
                        break;
                    }

                    for(j=0; j<4; j++)
                    {
                        *post_addr = (uint64_t)u3.ev.e[j];
                        post_addr++;
                    }

                    rc = getScom (G_PMULETS_3B[1], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMULETS_3B[1], rc);
                        break;
                    }

                    // Read counter do not reset when read.
                    // Perform read to corresponding config register to reset the read counter.
                    rc = getScom (G_PMU_CONFIGS_8[109][0], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMU_CONFIGS_8[109][0], rc);
                        break;
                    }

                    for(j=0; j<4; j++)
                    {
                        *post_addr = (uint64_t)u3.ev.e[j];
                        post_addr++;
                    }
                }

                if ( (G_CUR_UAV & MASK_TLPM7) == MASK_OCAPI7 )
                {
                    rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_DMA, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                        break;
                    }

                    *L_DBG_UNIT = 2;
                    post_addr = (uint64_t*) (POST_OFFSET_G3B_O_7 | PBA_ENABLE);
                    rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_ATOMIC, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                        break;
                    }

                    rc = getScom (G_PMULETS_3B[2], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMULETS_3B[2], rc);
                        break;
                    }

                    // Read counter do not reset when read.
                    // Perform read to corresponding config register to reset the read counter.
                    rc = getScom (G_PMU_CONFIGS_8[112][0], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMU_CONFIGS_8[112][0], rc);
                        break;
                    }

                    for(j=0; j<4; j++)
                    {
                        *post_addr = (uint64_t)u3.ev.e[j];
                        post_addr++;
                    }

                    rc = getScom (G_PMULETS_3B[3], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMULETS_3B[3], rc);
                        break;
                    }

                    // Read counter do not reset when read.
                    // Perform read to corresponding config register to reset the read counter.
                    rc = getScom (G_PMU_CONFIGS_8[115][0], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMU_CONFIGS_8[115][0], rc);
                        break;
                    }

                    for(j=0; j<4; j++)
                    {
                        *post_addr = (uint64_t)u3.ev.e[j];
                        post_addr++;
                    }
                }

                post_addr = (uint64_t*) (POST_OFFSET_G3B_O_T | PBA_ENABLE);
                *post_addr = INC_UPD_COUNT;
                break;

            case G4:
                *L_DBG_GRP = 4;
                post_addr = (uint64_t*) (POST_OFFSET_G4H | PBA_ENABLE);
                rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_ATOMIC, o_err);
                if ( rc )
                {
                    PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                    break;
                }

                *post_addr = INC_UPD_COUNT;
                post_addr++;
                if (G_CUR_UAV & MASK_PHB0)
                {
                    rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_DMA, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                        break;
                    }

                    *L_DBG_UNIT = 1;
                    rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_ATOMIC, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                        break;
                    }

                    post_addr = (uint64_t*) (POST_OFFSET_G4_0 | PBA_ENABLE);
                    rc = getScom (G_PMULETS_4[0], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMULETS_4[0], rc);
                        break;
                    }

                    for(i=0; i<4; i++)
                    {
                        *post_addr = (uint64_t)u3.ev.e[i];
                        post_addr++;
                    }
                }

                if (G_CUR_UAV & MASK_PHB1)
                {
                    rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_DMA, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                        break;
                    }

                    *L_DBG_UNIT = 2;
                    rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_ATOMIC, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                        break;
                    }

                    post_addr = (uint64_t*) (POST_OFFSET_G4_1 | PBA_ENABLE);
                    rc = getScom (G_PMULETS_4[1], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMULETS_4[1], rc);
                        break;
                    }

                    for(i=0; i<4; i++)
                    {
                        *post_addr = (uint64_t)u3.ev.e[i];
                        post_addr++;
                    }
                }

                if (G_CUR_UAV & MASK_PHB2)
                {
                    rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_DMA, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                        break;
                    }

                    *L_DBG_UNIT = 3;
                    rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_ATOMIC, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                        break;
                    }

                    post_addr = (uint64_t*) (POST_OFFSET_G4_2 | PBA_ENABLE);
                    rc = getScom (G_PMULETS_4[2], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMULETS_4[2], rc);
                        break;
                    }

                    for(i=0; i<4; i++)
                    {
                        *post_addr = (uint64_t)u3.ev.e[i];
                        post_addr++;
                    }
                }

                if (G_CUR_UAV & MASK_PHB3)
                {
                    rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_DMA, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                        break;
                    }

                    *L_DBG_UNIT = 4;
                    rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_ATOMIC, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                        break;
                    }

                    post_addr = (uint64_t*) (POST_OFFSET_G4_3 | PBA_ENABLE);
                    rc = getScom (G_PMULETS_4[3], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMULETS_4[3], rc);
                        break;
                    }

                    for(i=0; i<4; i++)
                    {
                        *post_addr = (uint64_t)u3.ev.e[i];
                        post_addr++;
                    }
                }

                if (G_CUR_UAV & MASK_PHB4)
                {
                    rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_DMA, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                        break;
                    }

                    *L_DBG_UNIT = 5;
                    rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_ATOMIC, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                        break;
                    }

                    post_addr = (uint64_t*) (POST_OFFSET_G4_4 | PBA_ENABLE);
                    rc = getScom (G_PMULETS_4[4], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMULETS_4[4], rc);
                        break;
                    }

                    for(i=0; i<4; i++)
                    {
                        *post_addr = (uint64_t)u3.ev.e[i];
                        post_addr++;
                    }
                }

                if (G_CUR_UAV & MASK_PHB5)
                {
                    rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_DMA, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                        break;
                    }

                    *L_DBG_UNIT = 6;
                    rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_ATOMIC, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                        break;
                    }

                    post_addr = (uint64_t*) (POST_OFFSET_G4_5 | PBA_ENABLE);
                    rc = getScom (G_PMULETS_4[5], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMULETS_4[5], rc);
                        break;
                    }

                    for(i=0; i<4; i++)
                    {
                        *post_addr = (uint64_t)u3.ev.e[i];
                        post_addr++;
                    }
                }

                if (G_CUR_UAV & MASK_NX)
                {
                    rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_DMA, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                        break;
                    }

                    *L_DBG_UNIT = 6;
                    rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_ATOMIC, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                        break;
                    }

                    // Read NX PMON-0
                    post_addr = (uint64_t*) (POST_OFFSET_G4_6 | PBA_ENABLE);
                    rc = getScom (G_PMULETS_4[6], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMULETS_4[6], rc);
                        break;
                    }

                    for(i=0; i<4; i++)
                    {
                        *post_addr = (uint64_t)u3.ev.e[i];
                        post_addr++;
                    }

                    // Read counter do not reset when read.
                    // A reset pulse has to be generated by writing 1 (followed by 0)
                    // to Bit position 2 for registers:
                    // NX.PBI.CQ_WRAP.NXCQ_SCOM.NX_PMU0_CONTROL_REG and
                    // NX.PBI.CQ_WRAP.NXCQ_SCOM.NX_PMU1_CONTROL_REG
                    // Explicitly value 0 written to read counter after reading them.
                    rc = getScom (G_PMULETS_4[8], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMULETS_4[8], rc);
                        break;
                    }
                    u3.pmulet |= 0x2000000000000000;
                    rc = putScom (G_PMULETS_4[8], u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x putscom failed. rc = 0x%x", G_PMULETS_4[8], rc);
                        break;
                    }

                    rc = getScom (G_PMULETS_4[8], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMULETS_4[8], rc);
                        break;
                    }
                    u3.pmulet &= ~0x2000000000000000;
                    rc = putScom (G_PMULETS_4[8], u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x putscom failed. rc = 0x%x", G_PMULETS_4[8], rc);
                        break;
                    }

                    // Read PMON-1
                    post_addr = (uint64_t*) (POST_OFFSET_G4_7 | PBA_ENABLE);
                    rc = getScom (G_PMULETS_4[7], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMULETS_4[7], rc);
                        break;
                    }

                    for(i=0; i<4; i++)
                    {
                        *post_addr = (uint64_t)u3.ev.e[i];
                        post_addr++;
                    }

                    // Read counter do not reset when read.
                    // A reset pulse has to be generated by writing 1 (followed by 0)
                    // to Bit position 2 for registers:
                    // NX.PBI.CQ_WRAP.NXCQ_SCOM.NX_PMU0_CONTROL_REG and
                    // NX.PBI.CQ_WRAP.NXCQ_SCOM.NX_PMU1_CONTROL_REG
                    // Explicitly value 0 written to read counter after reading them.
                    rc = getScom (G_PMULETS_4[9], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMULETS_4[9], rc);
                        break;
                    }
                    u3.pmulet |= 0x2000000000000000;
                    rc = putScom (G_PMULETS_4[9], u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x putscom failed. rc = 0x%x", G_PMULETS_4[9], rc);
                        break;
                    }

                    rc = getScom (G_PMULETS_4[9], &u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x getscom failed. rc = 0x%x", G_PMULETS_4[9], rc);
                        break;
                    }
                    u3.pmulet &= ~0x2000000000000000;
                    rc = putScom (G_PMULETS_4[9], u3.pmulet, o_err);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x putscom failed. rc = 0x%x", G_PMULETS_4[9], rc);
                        break;
                    }
                }

                post_addr = (uint64_t*) (POST_OFFSET_G4T | PBA_ENABLE);
                *post_addr = INC_UPD_COUNT;
                break;

            case G5://OCMB - [0:3]
                *L_DBG_GRP = 5;
                post_addr = (uint64_t*) (POST_OFFSET_G5H | PBA_ENABLE);
                rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_ATOMIC, o_err);
                if ( rc )
                {
                    PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                    break;
                }

                *post_addr = INC_UPD_COUNT;
                post_addr++;

                rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_DMA, o_err);
                if ( rc )
                {
                    PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                    break;
                }

                rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_ATOMIC, o_err);
                if ( rc )
                {
                    PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                    break;
                }

                if ( G_membuf_config == NULL )
                {
                    PK_TRACE("post_pmu_events: G_membuf_config is NULL. No OCMB (G5) reading.");
                    break;
                }

                // For OCMB:[0:3]
                for(j=0; j<=3; j++)
                {
                    if ( (j == 0) && !(G_CUR_UAV & MASK_OCMB0) )
                    {
                        continue;
                    }
                    else if ( (j == 1) && !(G_CUR_UAV & MASK_OCMB1) )
                    {
                        continue;
                    }
                    else if ( (j == 2) && !(G_CUR_UAV & MASK_OCMB2) )
                    {
                        continue;
                    }
                    else if ( (j == 3) && !(G_CUR_UAV & MASK_OCMB3) )
                    {
                        continue;
                    }

                    *L_DBG_UNIT = (j+1);

                    // Read OCMB scom
                    rc = membuf_get_scom(G_membuf_config,
                                         j,
                                         G_PMULETS_5678[0],
                                         &u3.pmulet);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x 0x%08x membuf_get_scom failed. rc = 0x%x", G_membuf_config->baseAddress[j], G_PMULETS_5678[0], rc);
                        break;
                    }

                    post_addr = (uint64_t*) ( (uint32_t)(POST_OFFSET_G5_0 | PBA_ENABLE) + (uint32_t)(0x20 * j) );
                    for(i=0; i<4; i++)
                    {
                        *post_addr = (uint64_t)u3.ev.e[i];
                        post_addr++;
                    }
                }

                post_addr = (uint64_t*) (POST_OFFSET_G5T | PBA_ENABLE);
                *post_addr = INC_UPD_COUNT;
                break;

            case G6://OCMB - [4:7]
                *L_DBG_GRP = 6;
                post_addr = (uint64_t*) (POST_OFFSET_G6H | PBA_ENABLE);
                rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_ATOMIC, o_err);
                if ( rc )
                {
                    PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                    break;
                }

                *post_addr = INC_UPD_COUNT;
                post_addr++;

                rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_DMA, o_err);
                if ( rc )
                {
                    PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                    break;
                }

                rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_ATOMIC, o_err);
                if ( rc )
                {
                    PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                    break;
                }

                if ( G_membuf_config == NULL )
                {
                    PK_TRACE("post_pmu_events: G_membuf_config is NULL. No OCMB (G6) reading.");
                    break;
                }

                // For OCMB:[4:7]
                for(j=4; j<=7; j++)
                {
                    if ( (j == 4) && !(G_CUR_UAV & MASK_OCMB4) )
                    {
                        continue;
                    }
                    else if ( (j == 5) && !(G_CUR_UAV & MASK_OCMB5) )
                    {
                        continue;
                    }
                    else if ( (j == 6) && !(G_CUR_UAV & MASK_OCMB6) )
                    {
                        continue;
                    }
                    else if ( (j == 7) && !(G_CUR_UAV & MASK_OCMB7) )
                    {
                        continue;
                    }

                    *L_DBG_UNIT = (j+1);

                    // Read OCMB scom
                    rc = membuf_get_scom(G_membuf_config,
                                         j,
                                         G_PMULETS_5678[0],
                                         &u3.pmulet);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x 0x%08x membuf_get_scom failed. rc = 0x%x", G_membuf_config->baseAddress[j], G_PMULETS_5678[0], rc);
                        break;
                    }

                    post_addr = (uint64_t*) ( (uint32_t)(POST_OFFSET_G6_4 | PBA_ENABLE) + (uint32_t)(0x20 * (j-4)) );
                    for(i=0; i<4; i++)
                    {
                        *post_addr = (uint64_t)u3.ev.e[i];
                        post_addr++;
                    }
                }

                post_addr = (uint64_t*) (POST_OFFSET_G6T | PBA_ENABLE);
                *post_addr = INC_UPD_COUNT;
                break;

            case G7://OCMB - [8:11]
                *L_DBG_GRP = 7;
                post_addr = (uint64_t*) (POST_OFFSET_G7H | PBA_ENABLE);
                rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_ATOMIC, o_err);
                if ( rc )
                {
                    PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                    break;
                }

                *post_addr = INC_UPD_COUNT;
                post_addr++;

                rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_DMA, o_err);
                if ( rc )
                {
                    PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                    break;
                }

                rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_ATOMIC, o_err);
                if ( rc )
                {
                    PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                    break;
                }

                if ( G_membuf_config == NULL )
                {
                    PK_TRACE("post_pmu_events: G_membuf_config is NULL. No OCMB (G7) reading.");
                    break;
                }

                // For OCMB:[8:11]
                for(j=8; j<=11; j++)
                {
                    if ( (j == 8) && !(G_CUR_UAV & MASK_OCMB8) )
                    {
                        continue;
                    }
                    else if ( (j == 9) && !(G_CUR_UAV & MASK_OCMB9) )
                    {
                        continue;
                    }
                    else if ( (j == 10) && !(G_CUR_UAV & MASK_OCMB10) )
                    {
                        continue;
                    }
                    else if ( (j == 11) && !(G_CUR_UAV & MASK_OCMB11) )
                    {
                        continue;
                    }

                    *L_DBG_UNIT = (j+1);

                    // Read OCMB scom
                    rc = membuf_get_scom(G_membuf_config,
                                         j,
                                         G_PMULETS_5678[0],
                                         &u3.pmulet);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x 0x%08x membuf_get_scom failed. rc = 0x%x", G_membuf_config->baseAddress[j], G_PMULETS_5678[0], rc);
                        break;
                    }

                    post_addr = (uint64_t*) ( (uint32_t)(POST_OFFSET_G7_8 | PBA_ENABLE) + (uint32_t)(0x20 * (j-8)) );
                    for(i=0; i<4; i++)
                    {
                        *post_addr = (uint64_t)u3.ev.e[i];
                        post_addr++;
                    }
                }

                post_addr = (uint64_t*) (POST_OFFSET_G7T | PBA_ENABLE);
                *post_addr = INC_UPD_COUNT;
                break;

            case G8://OCMB - [12:15]
                *L_DBG_GRP = 8;
                post_addr = (uint64_t*) (POST_OFFSET_G8H | PBA_ENABLE);
                rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_ATOMIC, o_err);
                if ( rc )
                {
                    PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                    break;
                }

                *post_addr = INC_UPD_COUNT;
                post_addr++;

                rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_DMA, o_err);
                if ( rc )
                {
                    PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                    break;
                }

                rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_ATOMIC, o_err);
                if ( rc )
                {
                    PK_TRACE("post_pmu_events: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
                    break;
                }

                if ( G_membuf_config == NULL )
                {
                    PK_TRACE("post_pmu_events: G_membuf_config is NULL. No OCMB (G8) reading.");
                    break;
                }

                // For OCMB:[12:15]
                for(j=12; j<=15; j++)
                {
                    if ( (j == 12) && !(G_CUR_UAV & MASK_OCMB12) )
                    {
                        continue;
                    }
                    else if ( (j == 13) && !(G_CUR_UAV & MASK_OCMB13) )
                    {
                        continue;
                    }
                    else if ( (j == 14) && !(G_CUR_UAV & MASK_OCMB14) )
                    {
                        continue;
                    }
                    else if ( (j == 15) && !(G_CUR_UAV & MASK_OCMB15) )
                    {
                        continue;
                    }

                    *L_DBG_UNIT = (j+1);

                    // Read OCMB scom
                    rc = membuf_get_scom(G_membuf_config,
                                         j,
                                         G_PMULETS_5678[0],
                                         &u3.pmulet);
                    if ( rc )
                    {
                        PK_TRACE("post_pmu_events: 0x%08x 0x%08x membuf_get_scom failed. rc = 0x%x", G_membuf_config->baseAddress[j], G_PMULETS_5678[0], rc);
                        break;
                    }

                    post_addr = (uint64_t*) ( (uint32_t)(POST_OFFSET_G8_12 | PBA_ENABLE) + (uint32_t)(0x20 * (j-12)) );
                    for(i=0; i<4; i++)
                    {
                        *post_addr = (uint64_t)u3.ev.e[i];
                        post_addr++;
                    }
                }

                post_addr = (uint64_t*) (POST_OFFSET_G8T | PBA_ENABLE);
                *post_addr = INC_UPD_COUNT;
                break;

            default:
                PK_TRACE("gpe_24x7: Invalid Group: %d", grp);
                rc = GPE_RC_24x7_INVALID_GROUP;
                gpe_set_ffdc(o_err, 0, GPE_RC_24x7_INVALID_GROUP, grp);
                break;
        }

        if ( rc )
            break;
    } while (0);

#ifdef DEBUG_24X7
    PK_TRACE ("<< post_pmu_events %d rc: 0x%08X", grp, rc);
#endif

    return rc;
}

/**
 * function: initialize_postings
 **/
uint32_t initialize_postings (GpeErrorStruct* o_err)
{//initialize posting area.
    volatile uint64_t* post_addr = NULL;
    int i=0;
    uint32_t rc = 0;

    PK_TRACE (">> initialize_postings");

    rc = putScom (PBASLVCTL1_C0040028, PBASLV_SET_DMA, o_err);
    if ( rc )
    {
        PK_TRACE("initialize_postings: PBASLVCTL1_C0040028 putscom failed. rc = 0x%08x", rc);
        return rc;
    }

    // @TODO check if this be improvised
    post_addr = (uint64_t*) (POSTING_START | PBA_ENABLE);
    for(i=0; i<TOTAL_POSTINGS; i++)
    {
        *post_addr = (uint64_t)0x0;
        post_addr++;
    }
    PK_TRACE ("<< initialize_postings");

    return rc;
}

/**
 * function: set_speed
 **/
uint32_t set_speed (uint64_t* speed, uint8_t* delay, volatile uint64_t* status, GpeErrorStruct* o_err)
{
    uint32_t rc = GPE_RC_SUCCESS;

    //set counter-scom read delay according to speed setting.
    PK_TRACE (">> set_speed 0x%08X", (uint32_t)(*speed & 0xffffffff));

    switch(*speed)
    {
        case CNTL_SPEED_1MS:
            *delay = 0;
            break;
        case CNTL_SPEED_2MS:
            *delay = 1;
            break;
        case CNTL_SPEED_4MS:
            *delay = 2;
            break;
        case CNTL_SPEED_8MS:
            *delay = 3;
            break;
        case CNTL_SPEED_16MS:
            *delay = 4;
            break;
        case CNTL_SPEED_32MS:
            *delay = 5;
            break;
        case CNTL_SPEED_64MS:
            *delay = 6;
            break;
        case CNTL_SPEED_128MS:
            *delay = 7;
            break;
        case CNTL_SPEED_256MS:
            *delay = 8;
            break;
        case CNTL_SPEED_512MS:
            *delay = 9;
            break;
        case CNTL_SPEED_1024MS:
            *delay = 10;
            break;
        case CNTL_SPEED_2048MS:
            *delay = 11;
            break;
        default:
            *status = CNTL_STATUS_ERR1;
            rc = GPE_RC_24x7_INVALID_SPEED;
            gpe_set_ffdc(o_err, 0, GPE_RC_24x7_INVALID_SPEED, *speed);
            break;
    }

    PK_TRACE ("<< set_speed: delay %d rc %d", *delay, rc);

    return rc;
}

/**
 * function: getScom wrapper
 **/
uint32_t getScom (const uint32_t i_addr, uint64_t* o_data, GpeErrorStruct* o_err)
{
    uint32_t rc = GPE_RC_SUCCESS;

    rc = getscom_abs (i_addr, o_data);
    if (rc != 0)
    {
        PK_TRACE ("ERR>> getScom: i_addr: 0x%08X rc: 0x%08X", i_addr, rc);
        gpe_set_ffdc(o_err, i_addr, rc, GPE_RC_SCOM_GET_FAILED);
    }

    return rc;
}

/**
 * function: putScom wrapper
 **/
uint32_t putScom (const uint32_t i_addr, uint64_t i_data, GpeErrorStruct* o_err)
{
    uint32_t rc = GPE_RC_SUCCESS;

    rc = putscom_abs (i_addr, i_data);
    if (rc != 0)
    {
        PK_TRACE ("ERR>> putScom: i_addr: 0x%08X rc: 0x%08X", i_addr, rc);
        gpe_set_ffdc(o_err, i_addr, rc, GPE_RC_SCOM_PUT_FAILED);
    }

    return rc;
}

