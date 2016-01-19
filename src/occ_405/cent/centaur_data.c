/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/cent/centaur_data.c $                             */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2014,2016                        */
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
#include "centaur_data.h"
#include "centaur_control.h"
#include "occhw_async.h"
#include "threadSch.h"
#include "pmc_register_addresses.h"
#include "centaur_data_service_codes.h"
#include "occ_service_codes.h"
#include "errl.h"
#include "trac.h"
#include "rtls.h"
#include "apss.h"
#include "state.h"
#include "occhw_scom.h"
//#include "centaur_firmware_registers.h"
//#include "centaur_register_addresses.h"

//*************************************************************************/
// Externs
//*************************************************************************/

//*************************************************************************/
// Macros
//*************************************************************************/

//*************************************************************************/
// Defines/Enums
//*************************************************************************/

// Enumerated list of possible centaur operations
typedef enum
{
    L4_LINE_DELETE,
    READ_NEST_LFIR6,
    READ_THERM_STATUS,
    RESET_DTS_FSM,
    DISABLE_SC,
    CLEAR_NEST_LFIR6,
    ENABLE_SC,
    READ_SCAC_LFIR,
    NUM_CENT_OPS
}cent_ops_enum;


#define MBCCFGQ_REG                 ((uint32_t)0x0201140ful)
#define LINE_DELETE_ON_NEXT_CE      ((uint64_t)0x0080000000000000ull)
#define SCAC_LFIR_REG               ((uint32_t)0x020115c0ul)
#define SCAC_CONFIG_REG             ((uint32_t)0x020115ceul)
#define SCAC_MASTER_ENABLE          ((uint64_t)0x8000000000000000ull)
#define CENT_NEST_LFIR_REG          ((uint32_t)0x0204000aul)
#define CENT_NEST_LFIR_AND_REG      ((uint32_t)0x0204000bul)
#define CENT_NEST_LFIR6             ((uint64_t)0x0200000000000000ull)
#define CENT_THRM_CTRL_REG          ((uint32_t)0x02050012ul)
#define CENT_THRM_CTRL4             ((uint64_t)0x0800000000000000ull)
#define CENT_THRM_STATUS_REG        ((uint32_t)0x02050013ul)
#define CENT_THRM_PARITY_ERROR26    ((uint64_t)0x0000002000000000ull)
#define CENT_MAX_DEADMAN_TIMER      0xf
#define CENT_DEADMAN_TIMER_2SEC     0x8
//*************************************************************************/
// Structures
//*************************************************************************/

//*************************************************************************/
// Globals
//*************************************************************************/

/* TEMP/TODO: PORE/MemData issues */
#if 0
//Global array of centaur data buffers
GPE_BUFFER(MemData G_centaur_data[NUM_CENTAUR_DATA_BUFF +
                                  NUM_CENTAUR_DOUBLE_BUF +
                                  NUM_CENTAUR_DATA_EMPTY_BUF]);

//pore request for scoming centaur registers
PoreFlex G_cent_scom_req;

//input/output parameters for gpe_scom_centaur()
GPE_BUFFER(GpeScomParms G_cent_scom_gpe_parms);

//scom command list entry
GPE_BUFFER(scomList_t G_cent_scom_list_entry[NUM_CENT_OPS]);

//buffer for storing output from running gpe_scom_centaur()
GPE_BUFFER(uint64_t G_cent_scom_data[MAX_NUM_CENTAURS]) = {0};

//Global array of centaur data pointers
MemData * G_centaur_data_ptrs[MAX_NUM_CENTAURS] = { &G_centaur_data[0],
   &G_centaur_data[1], &G_centaur_data[2], &G_centaur_data[3],
   &G_centaur_data[4], &G_centaur_data[5], &G_centaur_data[6],
   &G_centaur_data[7]};

//Global structures for gpe get mem data parms
GPE_BUFFER(GpeGetMemDataParms G_centaur_data_parms);

//Pore flex request for the GPE job that is used for centaur init.
PoreFlex G_centaur_reg_pore_req;

//Centaur structures used for task data pointers.
centaur_data_task_t G_centaur_data_task = {
    .start_centaur = 0,
    .current_centaur = 0,
    .end_centaur = 7,
    .prev_centaur = 7,
    .centaur_data_ptr = &G_centaur_data[8]
};
#endif

cent_sensor_flags_t G_cent_enabled_sensors = {0};

//AMEC needs to know when data for a centaur has been collected.
uint32_t G_updated_centaur_mask = 0;

//Global G_present_centaurs is bitmask of all centaurs
//(1 = present, 0 = not present. Core 0 has the most significant bit)
uint32_t G_present_centaurs = 0;

// Latch for a Trace Entry
uint8_t G_centaur_queue_not_idle_traced = 0;

// bitmap of centaurs requiring i2c recovery
uint8_t      G_centaur_needs_recovery = 0;

// bitmap of centaurs that have NEST LFIR6 asserted...
// This tells amec code to treat the centaur temperature as invalid
uint8_t      G_centaur_nest_lfir6 = 0;

//*************************************************************************/
// Function Prototypes
//*************************************************************************/

//*************************************************************************/
// Functions
//*************************************************************************/

// Function Specification
//
// Name: cent_recovery
//
// Description: i2c recovery procedure and other hw workarounds
//
// End Function Specification

//number of times in a row we must go without needing i2c recovery
//before we declare success and allow tracing again
#define I2C_REC_TRC_THROT_COUNT 8

//threshold of times LFIR6 is asserted (up/down counter) before tracing
#define NEST_LFIR6_MAX_COUNT 4

//number of SC polls to wait between i2c recovery attempts
#define CENT_SC_MAX_INTERVAL 256

/* TEMP/TODO: Reenable when needed */
#if 0
//determine scom address of MCIFIR register for given Centaur n
#define MCS0_MCIFIR_N(n) \
        ( (n<4)? (MCS0_MCIFIR + ((MCS1_MCIFIR - MCS0_MCIFIR) * (n))) : (MCS4_MCIFIR + ((MCS5_MCIFIR - MCS4_MCIFIR) * (n-4))) )
#endif

//mask for channel checkstop
#define MCIFIR_CHAN_CKSTOP_MASK 0x0000000100000000

/* TEMP/TODO: Reenable when needed */
#if 0
bool cent_chan_checkstop(const uint8_t i_cent)
{
    uint32_t l_scom_addr = 0;
    bool     l_rc = FALSE;
    uint64_t l_data;
    int      l_scom_rc = 0;

    // Determine scom address of MCIFIR register
    l_scom_addr = MCS0_MCIFIR_N(i_cent);

    // Do a getscom on MCIFIR register for i_cent
    l_scom_rc = getscom_ffdc(l_scom_addr, &l_data, 0);
    if(!l_scom_rc)
    {
        //check for channel checkstop (bit 31)
        if(l_data & MCIFIR_CHAN_CKSTOP_MASK)
        {
            l_rc = TRUE;

            if(CENTAUR_PRESENT(i_cent))
            {
                //remove checkstopped centaur from presence bitmap
                G_present_centaurs &= ~(CENTAUR_BY_MASK(i_cent));

                //remove the dimm temperature sensors behind this centaur from presence bitmap
                G_cent_enabled_sensors.bytes[i_cent] = 0x00;

                TRAC_IMP("Channel checkstop detected on Centaur[%d] scom_addr[0x%08X] G_present_centaurs[0x%08X]",
                         i_cent,
                         l_scom_addr,
                         G_present_centaurs);

                TRAC_IMP("Updated bitmap of enabled dimm temperature sensors: 0x%08X %08X",
                         G_cent_enabled_sensors.words[0],
                         G_cent_enabled_sensors.words[1]);
            }
        }
    }
    else
    {
        TRAC_ERR("cent_chan_checkstop: Error accessing MCIFIR register for Centaur[%d] scom_addr[0x%08X]",
                 i_cent,
                 l_scom_addr);
    }
    return l_rc;
}
#endif // #if 0

/* TEMP/TODO: Reenable when needed */
#if 0
void cent_recovery(uint32_t i_cent)
{
    int l_rc = 0;
    errlHndl_t    l_err   = NULL;
    uint32_t l_prev_cent = G_cent_scom_list_entry[L4_LINE_DELETE].instanceNumber;
    uint8_t l_cent_mask = CENTAUR0_PRESENT_MASK >> l_prev_cent;
    static bool L_not_idle_traced = FALSE;
    static uint8_t L_cent_callouts = 0;
    static bool L_gpe_scheduled = FALSE;
    static uint8_t L_i2c_rec_trc_throt = 0;
    static bool L_gpe_had_1_tick = FALSE;
    static uint8_t L_nest_lfir6_count[MAX_NUM_CENTAURS] = {0};
    static uint8_t L_nest_lfir6_traced = 0;
    static uint8_t L_nest_lfir6_logged = 0;
    static uint16_t L_i2c_recovery_delay[MAX_NUM_CENTAURS] = {0};
    static bool L_i2c_finish_recovery[MAX_NUM_CENTAURS] = {FALSE};

    do
    {
        //First, check to see if the previous GPE request is still running.
        //A request is considered idle if it is not attached to any of the
        //asynchronous request queues.
        if( !(async_request_is_idle(&G_cent_scom_req.request)) )
        {
            //This can happen due to variability in when the task runs from
            //one tick to another.  Only trace if it has had a full tick.
            if(!L_not_idle_traced && L_gpe_had_1_tick)
            {
                TRAC_INFO("cent_recovery: Centaur recovery GPE is still running. cent[%d], entries[%d], state[0x%08x]",
                          l_prev_cent,
                          G_cent_scom_gpe_parms.entries,
                          G_cent_scom_req.request.state);
                L_not_idle_traced = TRUE;
            }
            L_gpe_had_1_tick = TRUE;
            break;
        }
        else
        {
            //Request is idle
            L_gpe_had_1_tick = FALSE;
            //allow the trace again if it's idle
            if(L_not_idle_traced)
            {
                TRAC_INFO("cent_recovery: GPE completed. cent[%d],",
                          l_prev_cent);
                L_not_idle_traced = FALSE;
            }
        }

        //Request completed

        //Check for failure and log an error if we haven't already logged one for this centaur
        //but keep retrying.
        if(L_gpe_scheduled &&
           (!async_request_completed(&G_cent_scom_req.request) || G_cent_scom_gpe_parms.rc) &&
           (!(L_cent_callouts & l_cent_mask)))
        {
            // Check if the centaur has a channel checkstop. If it does, then do not
            // log any errors
            if(!(cent_chan_checkstop(l_prev_cent)))
            {
                //Mark the centaur as being called out
                L_cent_callouts |= l_cent_mask;

                // There was an error doing the recovery scoms
                TRAC_ERR("cent_recovery: gpe_scom_centaur failed. rc[0x%08x] cent[%d] entries[%d] errorIndex[0x%08X]",
                         G_cent_scom_gpe_parms.rc,
                         l_prev_cent,
                         G_cent_scom_gpe_parms.entries,
                         G_cent_scom_gpe_parms.errorIndex);

                /* @
                 * @errortype
                 * @moduleid    CENT_RECOVERY_MOD
                 * @reasoncode  CENT_SCOM_ERROR
                 * @userdata1   rc - Return code of failing scom
                 * @userdata2   index of failing scom
                 * @userdata4   0
                 * @devdesc     OCC to Centaur communication failure
                 */
                l_err = createErrl(
                        CENT_RECOVERY_MOD,                      //modId
                        CENT_SCOM_ERROR,                        //reasoncode
                        OCC_NO_EXTENDED_RC,                     //Extended reason code
                        ERRL_SEV_PREDICTIVE,                    //Severity
                        NULL,                                   //Trace Buf
                        DEFAULT_TRACE_SIZE,                     //Trace Size
                        G_cent_scom_gpe_parms.rc,               //userdata1
                        G_cent_scom_gpe_parms.errorIndex        //userdata2
                        );

                //dump ffdc contents collected by ssx
                addUsrDtlsToErrl(l_err,                                    //io_err
                         (uint8_t *) &(G_cent_scom_req.ffdc),              //i_dataPtr,
                         sizeof(PoreFfdc),                                 //i_size
                         ERRL_USR_DTL_STRUCT_VERSION_1,                    //version
                         ERRL_USR_DTL_BINARY_DATA);                        //type

                //callout the centaur
                addCalloutToErrl(l_err,
                                 ERRL_CALLOUT_TYPE_HUID,
                                 G_sysConfigData.centaur_huids[l_prev_cent],
                                 ERRL_CALLOUT_PRIORITY_MED);

                //callout the processor
                addCalloutToErrl(l_err,
                                 ERRL_CALLOUT_TYPE_HUID,
                                 G_sysConfigData.proc_huid,
                                 ERRL_CALLOUT_PRIORITY_MED);
                commitErrl(&l_err);
            }
        }

#if 0   //set this to 1 for testing hard failures
        if(CURRENT_TICK > 120000)
        {
            G_cent_scom_list_entry[READ_NEST_LFIR6].data = CENT_NEST_LFIR6;
        }
#endif

        // check the centaur nest lfir register for parity errors from thermal (bit 6)
        // NOTE: recovery will occur 8 ticks from now so that all entries target the
        // same centaur in a given tick (simplifies callouts)
        if(G_cent_scom_list_entry[READ_NEST_LFIR6].data & CENT_NEST_LFIR6)
        {
            //Increment the per-centaur LFIR[6] threshold counter
            if(L_nest_lfir6_count[l_prev_cent] < NEST_LFIR6_MAX_COUNT)
            {
                L_nest_lfir6_count[l_prev_cent]++;

                //log an error the first time we see this.  Error will be predictive
                //if mfg mode ipl and informational otherwise
                if(!(L_nest_lfir6_logged & l_cent_mask))
                {
                    //only log error once
                    L_nest_lfir6_logged |= l_cent_mask;

                    TRAC_ERR("cent_recovery: NEST LFIR[6] was asserted on cent[%d] lfir[%08x%08x], thrm_stat[%08x%08x]",
                             l_prev_cent,
                             (uint32_t)(G_cent_scom_list_entry[READ_NEST_LFIR6].data >> 32),
                             (uint32_t)(G_cent_scom_list_entry[READ_NEST_LFIR6].data),
                             (uint32_t)(G_cent_scom_list_entry[READ_THERM_STATUS].data >> 32),
                             (uint32_t)(G_cent_scom_list_entry[READ_THERM_STATUS].data));

                    //only log the error if the thermal parity error is being masked (per Mike Pardeik)
                    if(G_cent_scom_list_entry[READ_THERM_STATUS].data & CENT_THRM_PARITY_ERROR26)
                    {
                        /* @
                         * @errortype
                         * @moduleid    CENT_RECOVERY_MOD
                         * @reasoncode  CENT_LFIR_ERROR
                         * @userdata1   0
                         * @userdata2   0
                         * @userdata4   OCC_NO_EXTENDED_RC
                         * @devdesc     Centaur has an unexpected FIR bit set
                         */
                        l_err = createErrl(
                                CENT_RECOVERY_MOD,                      //modId
                                CENT_LFIR_ERROR,                        //reasoncode
                                OCC_NO_EXTENDED_RC,                     //Extended reason code
                                ERRL_SEV_INFORMATIONAL,                 //Severity
                                NULL,                                   //Trace Buf
                                DEFAULT_TRACE_SIZE,                     //Trace Size
                                0,                                      //userdata1
                                0);                                     //userdata2

                        //force severity to predictive if mfg ipl (allows callout to be added to info error)
                        setErrlActions(l_err, ERRL_ACTIONS_MANUFACTURING_ERROR);

                        //add centaur callout
                        addCalloutToErrl(l_err,
                                         ERRL_CALLOUT_TYPE_HUID,
                                         G_sysConfigData.centaur_huids[l_prev_cent],
                                         ERRL_CALLOUT_PRIORITY_HIGH);

                        commitErrl(&l_err);
                    }
                }

                //Trace if it looks like a hard failure (error will be logged if we time out later)
                if((L_nest_lfir6_count[l_prev_cent] == NEST_LFIR6_MAX_COUNT) &&
                   !(L_nest_lfir6_traced & l_cent_mask))
                {
                    //one-time trace of hitting the threshold
                    TRAC_IMP("cent_recovery: NEST LFIR[6] count reached thresh[%d]. cent[%d] scom[%08x%08x]",
                             NEST_LFIR6_MAX_COUNT,
                             l_prev_cent,
                             (uint32_t)(G_cent_scom_list_entry[READ_NEST_LFIR6].data >> 32),
                             (uint32_t)(G_cent_scom_list_entry[READ_NEST_LFIR6].data));
                    L_nest_lfir6_traced |= l_cent_mask;
                }
            }

            G_centaur_nest_lfir6 |= l_cent_mask;
        }
        else
        {
            //decrement the per-centaur LFIR[6] threshold counter
            if(L_nest_lfir6_count[l_prev_cent] > 0)
            {
                L_nest_lfir6_count[l_prev_cent]--;
            }

            G_centaur_nest_lfir6 &= ~l_cent_mask;
        }

        // Check if the previous centaur is in the process of recovery
        if(L_i2c_finish_recovery[l_prev_cent] == TRUE)
        {
            // Capture the scac_lfir as ffdc but only once
            if(L_i2c_recovery_delay[l_prev_cent] == CENT_SC_MAX_INTERVAL)
            {
                TRAC_ERR("cent_recovery: centaur[%d] scac_lfir[0x%08x%08x]",
                         l_prev_cent,
                         (uint32_t)(G_cent_scom_list_entry[READ_SCAC_LFIR].data >> 32),
                         (uint32_t)(G_cent_scom_list_entry[READ_SCAC_LFIR].data));
            }
        }

        //Now we can start working on the next centaur (i_cent)
        l_cent_mask = CENTAUR0_PRESENT_MASK >> i_cent;

        //reset for next pass
        L_gpe_scheduled = FALSE;

        //check if this centaur requires lfir6 recovery
        if(G_centaur_nest_lfir6 & l_cent_mask)
        {
            //Set the command type from GPE_SCOM_NOP to GPE_SCOM_RMW
            //these entries will reset the centaur DTS FSM and clear LFIR 6
            //if recovery worked, LFIR 6 should remain cleared.
            G_cent_scom_list_entry[RESET_DTS_FSM].commandType = GPE_SCOM_WRITE;
            G_cent_scom_list_entry[CLEAR_NEST_LFIR6].commandType = GPE_SCOM_WRITE;
        }
        else
        {
            //these ops aren't needed so disable them
            G_cent_scom_list_entry[RESET_DTS_FSM].commandType = GPE_SCOM_NOP;
            G_cent_scom_list_entry[CLEAR_NEST_LFIR6].commandType = GPE_SCOM_NOP;
        }

        //Decrement the delay counter for centaur i2c recovery
        if(L_i2c_recovery_delay[i_cent])
        {
           L_i2c_recovery_delay[i_cent]--;
        }
        //check if this centaur requires i2c recovery (dimm sensor has error status bit set)
        if(G_centaur_needs_recovery & l_cent_mask)
        {
            //If the delay time is up, do the i2c recovery
            if(!L_i2c_recovery_delay[i_cent])
            {
                if(!L_i2c_rec_trc_throt)
                {
                    TRAC_INFO("cent_recovery: Performing centaur[%d] i2c recovery procedure. required bitmap = 0x%02X",
                              i_cent, G_centaur_needs_recovery);
                }

                //restart the recovery delay
                L_i2c_recovery_delay[i_cent] = CENT_SC_MAX_INTERVAL;

                //don't allow tracing for at least I2C_REC_TRC_THROT_COUNT calls to this function
                //where we didn't require i2c recovery
                L_i2c_rec_trc_throt = I2C_REC_TRC_THROT_COUNT;

                //clear the request for i2c recovery here
                G_centaur_needs_recovery &= ~l_cent_mask;

                //Set the command type from GPE_SCOM_NOP to GPE_SCOM_RMW
                //this entry will disable the centaur sensor cache and
                //set a flag to finish the recovery in a later call of this
                //function (cent_recovery for a given centaur is called every
                //2 msec)
                G_cent_scom_list_entry[DISABLE_SC].commandType = GPE_SCOM_RMW;
                G_cent_scom_list_entry[ENABLE_SC].commandType = GPE_SCOM_NOP;
                L_i2c_finish_recovery[i_cent] = TRUE;
            }
        }
        else
        {
            //Centaur didn't require i2c recovery so decrement the throttle count if
            //not already 0.
            if(L_i2c_rec_trc_throt)
            {
                L_i2c_rec_trc_throt--;

                //Trace on transition from 1 to 0 only
                if(!L_i2c_rec_trc_throt)
                {
                    TRAC_INFO("cent_recovery: Centaur i2c recovered on all present centaurs");
                }
            }

            //these ops aren't needed so disable them
            G_cent_scom_list_entry[DISABLE_SC].commandType = GPE_SCOM_NOP;
            G_cent_scom_list_entry[ENABLE_SC].commandType = GPE_SCOM_NOP;

            // Finish the i2c recovery if it was started for this centaur
            if(L_i2c_finish_recovery[i_cent] == TRUE)
            {
                TRAC_INFO("cent_recovery: Finishing centaur[%d] i2c centaur recovery procedure",
                          i_cent);

                //clear the finish_recovery flag for this centaur
                L_i2c_finish_recovery[i_cent] = FALSE;

                //Set the command type from GPE_SCOM_NOP to GPE_SCOM_RMW
                //this entry will re-enable the centaur sensor cache
                //which will also cause the i2c master to be reset
                G_cent_scom_list_entry[DISABLE_SC].commandType = GPE_SCOM_NOP;
                G_cent_scom_list_entry[ENABLE_SC].commandType = GPE_SCOM_RMW;
            }
        }

        //Set the target centaur for all ops
        G_cent_scom_list_entry[L4_LINE_DELETE].instanceNumber = i_cent;
        G_cent_scom_list_entry[READ_NEST_LFIR6].instanceNumber = i_cent;
        G_cent_scom_list_entry[READ_THERM_STATUS].instanceNumber = i_cent;
        G_cent_scom_list_entry[RESET_DTS_FSM].instanceNumber = i_cent;
        G_cent_scom_list_entry[CLEAR_NEST_LFIR6].instanceNumber = i_cent;
        G_cent_scom_list_entry[DISABLE_SC].instanceNumber = i_cent;
        G_cent_scom_list_entry[ENABLE_SC].instanceNumber = i_cent;
        G_cent_scom_list_entry[READ_SCAC_LFIR].instanceNumber = i_cent;

        // Set up GPE parameters
        G_cent_scom_gpe_parms.rc         = 0;
        G_cent_scom_gpe_parms.entries    = NUM_CENT_OPS;
        G_cent_scom_gpe_parms.scomList   = (uint32_t) (&G_cent_scom_list_entry[0]);
        G_cent_scom_gpe_parms.options    = 0;
        G_cent_scom_gpe_parms.errorIndex = 0;

        // Submit Pore GPE without blocking
        l_rc = pore_flex_schedule(&G_cent_scom_req);
        if(l_rc)
        {
            TRAC_ERR("cent_recovery: pore_flex_schedule failed. rc = 0x%08x", l_rc);
            /* @
             * @errortype
             * @moduleid    CENT_RECOVERY_MOD
             * @reasoncode  SSX_GENERIC_FAILURE
             * @userdata1   rc - Return code of failing function
             * @userdata2   0
             * @userdata4   0
             * @devdesc     Internal failure (code bug)
             */
            l_err = createErrl(
                    CENT_RECOVERY_MOD,                      //modId
                    SSX_GENERIC_FAILURE,                    //reasoncode
                    OCC_NO_EXTENDED_RC,                     //Extended reason code
                    ERRL_SEV_PREDICTIVE,                    //Severity
                    NULL,                                   //Trace Buf
                    DEFAULT_TRACE_SIZE,                     //Trace Size
                    l_rc,                                   //userdata1
                    0                                       //userdata2
                );

            REQUEST_RESET(l_err);
            break;
        }
        L_gpe_scheduled = TRUE;

    }while(0);
}
#endif // #if 0

// Function Specification
//
// Name: task_centaur_data
//
// Description: Collect centaur data. The task is used for centaur data
//              collection
//
// End Function Specification
/* TEMP/TODO: Reenable when needed */
#if 0
void task_centaur_data( task_t * i_task )
{
    errlHndl_t    l_err   = NULL;    // Error handler
    int           rc      = 0;       // Return code
    MemData  *    l_temp  = NULL;
    centaur_data_task_t * l_centaur_data_ptr = (centaur_data_task_t *)i_task->data_ptr;
    GpeGetMemDataParms  * l_parms = (GpeGetMemDataParms *)(l_centaur_data_ptr->gpe_req.parameter);
    static bool   L_gpe_scheduled = FALSE;
    static bool   L_gpe_error_logged = FALSE;
    static bool   L_gpe_had_1_tick = FALSE;

    do
    {
        // ------------------------------------------
        // Centaur Data Task Variable Initial State
        // ------------------------------------------
        // ->current_centaur:  the one that was just 'written' to last tick to
        //                     kick off the sensor cache population in the
        //                     centaur.  It will be 'read from' during this tick.
        //
        // ->prev_centaur:     the one that was 'read from' during the last tick
        //                     and will be used to update the
        //                     G_updated_centaur_mask during this tick.
        //
        // ->centaur_data_ptr: points to G_centaur_data_ptrs[] for
        //                     the centaur that is referenced by prev_centaur
        //                     (the one that was just 'read')

        //First, check to see if the previous GPE request still running
        //A request is considered idle if it is not attached to any of the
        //asynchronous request queues
        if( !(async_request_is_idle(&l_centaur_data_ptr->gpe_req.request)) )
        {
            //This may happen due to variability in the time that this
            //task runs.  Don't trace on the first occurrence.
            if( !G_centaur_queue_not_idle_traced && L_gpe_had_1_tick)
            {
                TRAC_INFO("task_centaur_data: GPE is still running");
                G_centaur_queue_not_idle_traced = TRUE;
            }
            L_gpe_had_1_tick = TRUE;
            break;
        }
        else
        {
            //Request is idle
            L_gpe_had_1_tick = FALSE;
            if(G_centaur_queue_not_idle_traced)
            {
                TRAC_INFO("task_centaur_data: GPE completed");
                G_centaur_queue_not_idle_traced = FALSE;
            }
        }

        //Need to complete collecting data for all assigned centaurs from
        //previous interval and tick 0 is the current tick before collect data again.
        if( (l_centaur_data_ptr->current_centaur == l_centaur_data_ptr->end_centaur)
            && ((CURRENT_TICK & (MAX_NUM_TICKS - 1)) != 0) )
        {
            CENT_DBG("Did not collect centaur data. Need to wait for tick.");
            break;
        }

        //Check to see if the previous GPE request has succeeded.
        //A request is not considered complete until both the engine job
        //has finished without error and any callback has run to completion.
        if(L_gpe_scheduled)
        {
            //If the request is idle but not completed then there was an error
            //(as long as the request was scheduled).
            if(!async_request_completed(&l_centaur_data_ptr->gpe_req.request) || l_parms->rc )
            {
                // Check if the centaur has a channel checkstop. If it does, then do not
                // log any errors
                if(!(cent_chan_checkstop(l_centaur_data_ptr->prev_centaur)))
                {
                    //log an error the first time this happens but keep on running.
                    //eventually, we will timeout on the dimm & centaur temps not being updated
                    //and fans will go to max speed (probably won't be able to throttle for
                    //same reason we can't access the centaur here).
                    if(!L_gpe_error_logged)
                    {
                        L_gpe_error_logged = TRUE;

                        // There was an error collecting the centaur sensor cache
                        TRAC_ERR("task_centaur_data: gpe_get_mem_data failed. rc=0x%08x%08x, cur=%d, prev=%d",
                                 (uint32_t)(l_parms->rc >> 32),
                                 (uint32_t)(l_parms->rc),
                                 l_centaur_data_ptr->current_centaur,
                                 l_centaur_data_ptr->prev_centaur);
                        /* @
                         * @errortype
                         * @moduleid    CENT_TASK_DATA_MOD
                         * @reasoncode  CENT_SCOM_ERROR
                         * @userdata1   l_parms->rc
                         * @userdata2   0
                         * @userdata4   OCC_NO_EXTENDED_RC
                         * @devdesc     Failed to get centaur data
                         */
                        l_err = createErrl(
                                CENT_TASK_DATA_MOD,                     //modId
                                CENT_SCOM_ERROR,                        //reasoncode
                                OCC_NO_EXTENDED_RC,                     //Extended reason code
                                ERRL_SEV_PREDICTIVE,                    //Severity
                                NULL,                                   //Trace Buf
                                DEFAULT_TRACE_SIZE,                     //Trace Size
                                l_parms->rc,                            //userdata1
                                0                                       //userdata2
                                );

                        addUsrDtlsToErrl(l_err,                                   //io_err
                                (uint8_t *) &(l_centaur_data_ptr->gpe_req.ffdc),  //i_dataPtr,
                                sizeof(PoreFfdc),                                 //i_size
                                ERRL_USR_DTL_STRUCT_VERSION_1,                    //version
                                ERRL_USR_DTL_BINARY_DATA);                        //type

                        //Callouts depend on the return code of the gpe_get_mem_data procedure
                        if(l_parms->rc == GPE_GET_MEM_DATA_DIED)
                        {
                            //callout the processor
                            addCalloutToErrl(l_err,
                                             ERRL_CALLOUT_TYPE_HUID,
                                             G_sysConfigData.proc_huid,
                                             ERRL_CALLOUT_PRIORITY_LOW);
                        }
                        else if(l_parms->rc == GPE_GET_MEM_DATA_SENSOR_CACHE_FAILED)
                        {
                            //callout the previous centaur if present
                            if(CENTAUR_PRESENT(l_centaur_data_ptr->prev_centaur))
                            {
                                addCalloutToErrl(l_err,
                                                 ERRL_CALLOUT_TYPE_HUID,
                                                 G_sysConfigData.centaur_huids[l_centaur_data_ptr->prev_centaur],
                                                 ERRL_CALLOUT_PRIORITY_HIGH);
                            }

                            //callout the processor
                            addCalloutToErrl(l_err,
                                             ERRL_CALLOUT_TYPE_HUID,
                                             G_sysConfigData.proc_huid,
                                             ERRL_CALLOUT_PRIORITY_LOW);
                        }
                        else if(l_parms->rc == GPE_GET_MEM_DATA_UPDATE_FAILED)
                        {
                            //callout the current centaur if present
                            if(CENTAUR_PRESENT(l_centaur_data_ptr->current_centaur))
                            {
                                addCalloutToErrl(l_err,
                                                 ERRL_CALLOUT_TYPE_HUID,
                                                 G_sysConfigData.centaur_huids[l_centaur_data_ptr->current_centaur],
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
                //If the previous GPE request succeeded then swap l_centaur_data_ptr
                //with the global one. The gpe routine will write new data into
                //a buffer that is not being accessed by the RTLoop code.
                l_temp = l_centaur_data_ptr->centaur_data_ptr;
                l_centaur_data_ptr->centaur_data_ptr =
                    G_centaur_data_ptrs[l_centaur_data_ptr->current_centaur];
                G_centaur_data_ptrs[l_centaur_data_ptr->prev_centaur] = l_temp;

                //Centaur data has been collected so set the bit in global mask.
                //AMEC code will know which centaur to update sensors for. AMEC is
                //responsible for clearing the bit later on.
                // prev centaur is the one that was just 'read from' in the last tick
                if( CENTAUR_PRESENT(l_centaur_data_ptr->prev_centaur) )
                {
                    G_updated_centaur_mask |= CENTAUR_BY_MASK(l_centaur_data_ptr->prev_centaur);
                }
            }
        }//if(L_gpe_scheduled)

        // If the centaur is not present, then we need to point to the empty G_centaur_data
        // so that we don't use old/stale data from a leftover G_centaur_data
        // (this is very handy for debug...)
        if( !CENTAUR_PRESENT(l_centaur_data_ptr->current_centaur))
        {
            G_centaur_data_ptrs[l_centaur_data_ptr->current_centaur] = &G_centaur_data[9];
        }

        //Update current centaur
        if ( l_centaur_data_ptr->current_centaur >= l_centaur_data_ptr->end_centaur )
        {
            l_centaur_data_ptr->prev_centaur = l_centaur_data_ptr->current_centaur;
            l_centaur_data_ptr->current_centaur = l_centaur_data_ptr->start_centaur;
        }
        else
        {
            l_centaur_data_ptr->prev_centaur = l_centaur_data_ptr->current_centaur;
            l_centaur_data_ptr->current_centaur++;
        }

        // ------------------------------------------
        // Centaur Data Task Variable State Changed
        // ------------------------------------------
        // ->current_centaur:  the one that will be 'written' to in order to
        //                     kick off the sensor cache population in the
        //                     centaur.
        //
        // ->prev_centaur:     the one that will be 'read from', meaning have
        //                     the sensor cache transferred from the Centaur
        //                     to l_centaur_data_ptr->centaur_data_ptr
        //
        // ->centaur_data_ptr: points to G_centaur_data_ptrs[] for
        //                     the centaur that is referenced by prev_centaur
        //                     (the one that will be 'read')

        //If centaur is not present then skip it. This task assigned to this centaur will
        //be idle during this time it would have collected the data.
        if( CENTAUR_PRESENT(l_centaur_data_ptr->current_centaur)
            || CENTAUR_PRESENT(l_centaur_data_ptr->prev_centaur) )
        {
            // Setup the 'get centaur data' parms
            // ->config controls which Centaur we are reading from
            if( CENTAUR_PRESENT(l_centaur_data_ptr->prev_centaur) ){
              // If prev centaur is present, do the read of the sensor cache
              l_parms->collect = l_centaur_data_ptr->prev_centaur;
            }
            else{
              // If prev centaur is not present, don't do the read of the sensor cache.
              l_parms->collect = -1;
            }

            // ->config_update controls which Centaur we are writing to
            if( CENTAUR_PRESENT(l_centaur_data_ptr->current_centaur) ){
              // If cur centaur is present, do the write to kick off the sensor cache collect
              l_parms->update = l_centaur_data_ptr->current_centaur;
            }
            else{
              // If cur centaur is not present, don't do the write to kick off the sensor cache collect
              l_parms->update = -1;
            }

            l_parms->data = (uint32_t) l_centaur_data_ptr->centaur_data_ptr;
            l_parms->rc = 0;

            // Pore flex schedule gpe_get_mem_data
            // Check pore_flex_schedule return code if error
            // then request OCC reset.
            rc = pore_flex_schedule( &(l_centaur_data_ptr->gpe_req) );
            if(rc)
            {
                TRAC_ERR("task_centaur_data: pore_flex_schedule failed for centaur data collection. rc=%d", rc);
                /* @
                 * @errortype
                 * @moduleid    CENT_TASK_DATA_MOD
                 * @reasoncode  SSX_GENERIC_FAILURE
                 * @userdata1   rc - Return code of failing function
                 * @userdata2   0
                 * @userdata4   ERC_CENTAUR_PORE_FLEX_SCHEDULE_FAILURE
                 * @devdesc     Failed to get centaur data
                 */
                l_err = createErrl(
                        CENT_TASK_DATA_MOD,                     //modId
                        SSX_GENERIC_FAILURE,                    //reasoncode
                        ERC_CENTAUR_PORE_FLEX_SCHEDULE_FAILURE, //Extended reason code
                        ERRL_SEV_PREDICTIVE,                    //Severity
                        NULL,                                   //Trace Buf
                        DEFAULT_TRACE_SIZE,                     //Trace Size
                        rc,                                     //userdata1
                        l_parms->rc                             //userdata2
                        );

                addUsrDtlsToErrl(l_err,                                   //io_err
                        (uint8_t *) &(l_centaur_data_ptr->gpe_req.ffdc),  //i_dataPtr,
                        sizeof(PoreFfdc),                                 //i_size
                        ERRL_USR_DTL_STRUCT_VERSION_1,                    //version
                        ERRL_USR_DTL_BINARY_DATA);                        //type

                REQUEST_RESET(l_err);     //this will add firmware callout
                break;
            }

            L_gpe_scheduled = TRUE;
        }

    } while(0);

    //handle centaur i2c recovery requests and centaur workaround
    if(CENTAUR_PRESENT(l_centaur_data_ptr->current_centaur))
    {
        cent_recovery(l_centaur_data_ptr->current_centaur);
    }
    return;
}
#endif // #if 0

#define CENTAUR_SENSCACHE_ENABLE 0x020115CC
// Function Specification
//
// Name: cent_get_enabled_sensors
//
// Description: Reads
//
// End Function Specification
/* TEMP/TODO: Reenable when needed */
#if 0
int cent_get_enabled_sensors()
{
    int l_rc = 0;
    unsigned int l_cent;

    do
    {
        // Set up scom list entry (there's only 1)
        G_cent_scom_list_entry[0].scom  = CENTAUR_SENSCACHE_ENABLE;       //scom address
        G_cent_scom_list_entry[0].commandType = GPE_SCOM_READ_VECTOR;     //scom operation to perform
        G_cent_scom_list_entry[0].instanceNumber = 0;                     //Ignored for READ_VECTOR operation
        G_cent_scom_list_entry[0].pData = (uint64_t *) G_cent_scom_data;  //scom data will be stored here

        // Set up GPE parameters
        G_cent_scom_gpe_parms.rc         = 0;
        G_cent_scom_gpe_parms.entries    = 1;
        G_cent_scom_gpe_parms.scomList   = (uint32_t) (&G_cent_scom_list_entry[0]);
        G_cent_scom_gpe_parms.options    = 0;
        G_cent_scom_gpe_parms.errorIndex = 0;

        //Initializes PoreFlex
        l_rc = pore_flex_create( &G_cent_scom_req,     // gpe_req for the task
               &G_pore_gpe1_queue,                     // queue
               gpe_scom_centaur,                       // entry point
               (uint32_t) &G_cent_scom_gpe_parms,      // parm for the task
               SSX_SECONDS(2),                         // timeout
               NULL,                                   // callback
               NULL,                                   // callback argument
               ASYNC_REQUEST_BLOCKING );               // options
        if(l_rc)
        {
            TRAC_ERR("cent_get_enabled_sensors: pore_flex_create failed. rc = 0x%08x", l_rc);
            break;
        }

        // Submit Pore GPE and wait for completion
        l_rc = pore_flex_schedule(&G_cent_scom_req);
        if(l_rc)
        {
            TRAC_ERR("cent_get_enabled_sensors: pore_flex_schedule failed. rc = 0x%08x", l_rc);
            break;
        }

        //consolidate scom data into a smaller, cacheable 8 byte buffer
        for(l_cent = 0; l_cent < MAX_NUM_CENTAURS; l_cent++)
        {
            G_cent_enabled_sensors.bytes[l_cent] = ((uint8_t*)(&G_cent_scom_data[l_cent]))[0];
        }

        TRAC_IMP("bitmap of enabled dimm temperature sensors: 0x%08X %08X",
                 G_cent_enabled_sensors.words[0],
                 G_cent_enabled_sensors.words[1]);
    }while(0);
    return l_rc;
}
#endif // #if 0

// Function Specification
//
// Name: centaur_init
//
// Description: Initialize procedures for collecting centaur data. It
//        needs to be run in occ main and before RTLoop started.
//        This will also initialize the centaur watchdog.
//
// End Function Specification
/* TEMP/TODO: Reenable when needed */
#if 0
void centaur_init( void )
{
    errlHndl_t l_err   = NULL;  // Error handler
    int        rc      = 0;    // Return code
    int        l_jj    = 0;     // Indexer
    static scomList_t   L_scomList[2]       SECTION_ATTRIBUTE(".noncacheable");
    static GpeScomParms L_centaur_reg_parms SECTION_ATTRIBUTE(".noncacheable");

    do
    {
        /// Initialize Centaur & Centaur Data Structures
        /// This needs to run before RTLoop starts as init needs to be
        /// done before task to collect centaur data starts.

        TRAC_INFO("centaur_init: Initializing Centaur ... " );

        /// Before anything else, we need to call this procedure to
        /// determine which Centaurs are out there, their config info.
        /// and Type/EC Level

        rc = centaur_configuration_create();
        if( rc )
        {
            TRAC_ERR("centaur_init: Centaur Config Create failed with rc=0x%08x ", rc );
            break;
        }

        /// Set up Centaurs present global variable for use by OCC
        /// looping though the bitmask.

        G_present_centaurs = 0;
        for(l_jj=0; l_jj<MAX_NUM_CENTAURS; l_jj++)
        {
            // Check if this centaur is even possible to be present
            // by ANDing it against ALL_CENTAURS_MASK in this macro

            if( CENTAUR_BY_MASK(l_jj) )
            {
                if( G_centaurConfiguration.baseAddress[l_jj] )
                {
                    // G_cent_ba is != 0, so a valid Bar Address was found
                    // This means there is a VALID centaur there.
                    G_present_centaurs |= (CENTAUR0_PRESENT_MASK >> l_jj);

                    // Trace out the CFAM Chip ID, which includes Type & EC
                    TRAC_INFO("centaur_init: Centaur[%d] Found, Chip Id=0x%08x",l_jj, mb_id(l_jj));
                }
            }
        }

        TRAC_IMP("centaur_init: G_present_centaurs = 0x%08x", G_present_centaurs);

        //initialize global bitmap of enabled centaur temperature sensors (for dimms)
        rc = cent_get_enabled_sensors();

        // Set up recovery scom list entries
        G_cent_scom_list_entry[L4_LINE_DELETE].scom  = MBCCFGQ_REG;                //scom address
        G_cent_scom_list_entry[L4_LINE_DELETE].commandType = GPE_SCOM_RMW;         //scom operation to perform
        G_cent_scom_list_entry[L4_LINE_DELETE].mask = LINE_DELETE_ON_NEXT_CE;      //mask of bits to change
        G_cent_scom_list_entry[L4_LINE_DELETE].data = LINE_DELETE_ON_NEXT_CE;      //scom data (always set the bit)

        //one time init for reading LFIR6
        G_cent_scom_list_entry[READ_NEST_LFIR6].scom  = CENT_NEST_LFIR_REG;         //scom address
        G_cent_scom_list_entry[READ_NEST_LFIR6].commandType = GPE_SCOM_READ;        //scom operation to perform
        G_cent_scom_list_entry[READ_NEST_LFIR6].mask = 0;                           //mask (not used for reads)
        G_cent_scom_list_entry[READ_NEST_LFIR6].data = 0;                           //scom data (initialize to 0)

        //one time init for reading centaur thermal status register
        G_cent_scom_list_entry[READ_THERM_STATUS].scom  = CENT_THRM_STATUS_REG;     //scom address
        G_cent_scom_list_entry[READ_THERM_STATUS].commandType = GPE_SCOM_READ;      //scom operation to perform
        G_cent_scom_list_entry[READ_THERM_STATUS].mask = 0;                         //mask (not used for reads)
        G_cent_scom_list_entry[READ_THERM_STATUS].data = 0;                         //scom data (initialize to 0)

        //one time init to reset the centaur dts FSM
        G_cent_scom_list_entry[RESET_DTS_FSM].scom  = CENT_THRM_CTRL_REG;           //scom address
        G_cent_scom_list_entry[RESET_DTS_FSM].commandType = GPE_SCOM_NOP;           //init to no-op (only runs if needed)
        G_cent_scom_list_entry[RESET_DTS_FSM].mask = 0;                             //mask (not used for writes)
        G_cent_scom_list_entry[RESET_DTS_FSM].data = CENT_THRM_CTRL4;               //scom data (sets bit4)

        //one time init to clear centaur NEST LFIR 6
        G_cent_scom_list_entry[CLEAR_NEST_LFIR6].scom  = CENT_NEST_LFIR_AND_REG;    //scom address
        G_cent_scom_list_entry[CLEAR_NEST_LFIR6].commandType = GPE_SCOM_NOP;        //init to no-op (only runs if needed)
        G_cent_scom_list_entry[CLEAR_NEST_LFIR6].mask = 0;                          //mask (not used for writes)
        G_cent_scom_list_entry[CLEAR_NEST_LFIR6].data = ~CENT_NEST_LFIR6;           //scom data

        //one time init to disable centaur sensor cache
        G_cent_scom_list_entry[DISABLE_SC].scom  = SCAC_CONFIG_REG;                 //scom address
        G_cent_scom_list_entry[DISABLE_SC].commandType = GPE_SCOM_NOP;              //init to no-op (only runs if needed)
        G_cent_scom_list_entry[DISABLE_SC].mask = SCAC_MASTER_ENABLE;               //mask of bits to change
        G_cent_scom_list_entry[DISABLE_SC].data = 0;                                //scom data (disable sensor cache)

        //one time init to enable centaur sensor cache
        G_cent_scom_list_entry[ENABLE_SC].scom  = SCAC_CONFIG_REG;                  //scom address
        G_cent_scom_list_entry[ENABLE_SC].commandType = GPE_SCOM_NOP;               //init to no-op (only runs if needed)
        G_cent_scom_list_entry[ENABLE_SC].mask = SCAC_MASTER_ENABLE;                //mask of bits to change
        G_cent_scom_list_entry[ENABLE_SC].data = SCAC_MASTER_ENABLE;                //scom data (enable sensor cache)

        //one time init for reading centaur sensor cache lfir
        G_cent_scom_list_entry[READ_SCAC_LFIR].scom  = SCAC_LFIR_REG;               //scom address
        G_cent_scom_list_entry[READ_SCAC_LFIR].commandType = GPE_SCOM_READ;         //scom operation to perform
        G_cent_scom_list_entry[READ_SCAC_LFIR].mask = 0;                            //mask (not used for reads)
        G_cent_scom_list_entry[READ_SCAC_LFIR].data = 0;                            //scom data (initialize to 0)

        /// Set up Centuar Scom Registers - array of Scoms
        ///   [0]:  Setup deadman timer
        /// NOTE: max timeout is about 2 seconds.

        L_scomList[0].scom        = CENTAUR_MBSCFGQ;
        L_scomList[0].commandType = GPE_SCOM_RMW_ALL;

        centaur_mbscfgq_t l_mbscfg;
        l_mbscfg.value = 0;

        //set up the mask bits
        l_mbscfg.fields.occ_deadman_timer_sel = CENT_MAX_DEADMAN_TIMER;
        L_scomList[0].mask = l_mbscfg.value;

        //set up the data bits
        l_mbscfg.fields.occ_deadman_timer_sel = CENT_DEADMAN_TIMER_2SEC;
        L_scomList[0].data = l_mbscfg.value;

        /// Set up Centaur Scom Registers - array of Scoms
        ///   [1]: clear the emergency throttle bit

        L_scomList[1].scom        = CENTAUR_MBSEMERTHROQ;
        L_scomList[1].commandType = GPE_SCOM_RMW_ALL;

        centaur_mbsemerthroq_t l_mbs_et;
        l_mbs_et.value = 0;

        //set up the data
        L_scomList[1].data = l_mbs_et.value;

        //set up the mask
        l_mbs_et.fields.emergency_throttle_ip = 1;
        L_scomList[1].mask = l_mbs_et.value;

        L_centaur_reg_parms.scomList   = (uint32_t) (&L_scomList[0]);
        L_centaur_reg_parms.entries    = 2;
        L_centaur_reg_parms.options    = 0;
        L_centaur_reg_parms.rc         = 0;
        L_centaur_reg_parms.errorIndex = 0;

        //Initialize PoreFlex
        rc = pore_flex_create( &G_centaur_reg_pore_req,  //gpe_req for the task
                &G_pore_gpe1_queue,                 //queue
                gpe_scom_centaur,                   //entry point
                (uint32_t) &L_centaur_reg_parms,    //parm for the task
                SSX_SECONDS(5),                     //no timeout
                NULL,                               //callback
                NULL,                               //callback argument
                ASYNC_REQUEST_BLOCKING );           //options
        if(rc)
        {
            TRAC_ERR("centaur_init: pore_flex_create failed for G_centaur_reg_pore_req. rc = 0x%08x", rc);
            break;
        }

        // Submit Pore GPE and wait for completion
        rc = pore_flex_schedule(&G_centaur_reg_pore_req);

        // Check for errors on Scom
        if(rc || L_centaur_reg_parms.rc)
        {
           TRAC_ERR("centaur_init: gpe_scom_centaur failure. rc = 0x%08x, gpe_rc = 0x%08x, index = 0x%08x",
                   rc,
                   L_centaur_reg_parms.rc,
                   L_centaur_reg_parms.errorIndex);
           if(!rc)
           {
               rc = L_centaur_reg_parms.rc;
           }
           break;
        }

        /// Set up the OCC Centaur Data Collection Procedure
        /// Includes initializing the centaur procedure parameters
        /// to gather the 'centaur' data, but we will set them to
        /// invalid (-1) until the task sets them up

        G_centaur_data_parms.rc      =  0;
        G_centaur_data_parms.collect = -1;
        G_centaur_data_parms.update  = -1;
        G_centaur_data_parms.data    =  0;

        //Initializes existing PoreFlex object for centaur data
        rc = pore_flex_create( &G_centaur_data_task.gpe_req,     //gpe_req for the task
                &G_pore_gpe1_queue,               //queue
                gpe_get_mem_data,                 //entry point
                (uint32_t) &G_centaur_data_parms, //parm for the task
                SSX_WAIT_FOREVER,                 //
                NULL,                             //callback
                NULL,                             //callback argument
                0 );                              //options

        if(rc)
        {
            TRAC_ERR("centaur_init: pore_flex_create failed for G_centaur_data_task.gpe_req. rc = 0x%08x", rc);
            break;
        }

        //Initialize existing PoreFlex object for centaur recovery
        rc = pore_flex_create( &G_cent_scom_req,     // gpe_req for the task
             &G_pore_gpe1_queue,                     // queue
             gpe_scom_centaur,                       // entry point
             (uint32_t) &G_cent_scom_gpe_parms,      // parm for the task
             SSX_WAIT_FOREVER,                       //
             NULL,                                   // callback
             NULL,                                   // callback argument
             0);                                     // options
        if(rc)
        {
            TRAC_ERR("centaur_init: pore_flex_create failed for G_cent_scom_req. rc = 0x%08x", rc);
            break;
        }

        /// Initialization complete, Centaur Control & Data Collection
        /// Tasks can now run

    } while(0);

    if( rc )
    {

        /* @
         * @errortype
         * @moduleid    CENTAUR_INIT_MOD
         * @reasoncode  SSX_GENERIC_FAILURE
         * @userdata1   rc - Return code of failing function
         * @userdata2   Return code of failing GPE
         * @userdata4   OCC_NO_EXTENDED_RC
         * @devdesc     Failed to initialize Centaurs
         */
        l_err = createErrl(
                CENTAUR_INIT_MOD,                           //modId
                SSX_GENERIC_FAILURE,                        //reasoncode
                OCC_NO_EXTENDED_RC,                         //Extended reason code
                ERRL_SEV_PREDICTIVE,                        //Severity
                NULL,                                       //Trace Buf
                DEFAULT_TRACE_SIZE,                         //Trace Size
                rc,                                         //userdata1
                L_centaur_reg_parms.rc                      //userdata2
                );

        addUsrDtlsToErrl(l_err,                                          //io_err
                         (uint8_t *) &G_centaur_reg_pore_req.ffdc,       //i_dataPtr,
                         sizeof(PoreFfdc),                               //i_size
                         ERRL_USR_DTL_STRUCT_VERSION_1,                  //version
                         ERRL_USR_DTL_BINARY_DATA);                      //type

        REQUEST_RESET(l_err);
    }
    else
    {
        // Only initialize the control structures if we haven't had
        // any errors yet
        centaur_control_init();
    }

    return;
}
#endif // #if 0

// Function Specification
//
// Name: cent_get_centaur_data_ptr
//
// Description: Returns a pointer to the most up-to-date centaur data for
//              the centaur associated with the specified OCC centaur id.
//              Returns NULL for centaur ID outside the range of 0 to 7.
//
// End Function Specification
/* TEMP/TODO: Reenable when needed */
#if 0
MemData * cent_get_centaur_data_ptr( const uint8_t i_occ_centaur_id )
{
    //The caller needs to send in a valid OCC centaur id. Since type is uchar
    //so there is no need to check for case less than 0.
    //If centaur id is invalid then returns NULL.
    if( i_occ_centaur_id < MAX_NUM_CENTAURS )
    {
        //Returns a pointer to the most up-to-date centaur data.
        return G_centaur_data_ptrs[i_occ_centaur_id];
    }
    else
    {
        //Core id outside the range
        TRAC_ERR("cent_get_centaur_data_ptr: Invalid OCC centaur id [0x%x]", i_occ_centaur_id);
        return( NULL );
    }
}
#endif
