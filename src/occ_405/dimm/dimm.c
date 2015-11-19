/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/dimm/dimm.c $                                     */
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

//#define DIMM_DEBUG

#include <ssx.h>
#include <occhw_async.h>
#include <gpe_export.h>

#include <trac_interface.h>
#include <trac.h>
#include <occ_common.h>
#include <comp_ids.h>
#include <occ_service_codes.h>
#include <dimm.h>
#include <dimm_service_codes.h>
#include <state.h>
#include <occ_sys_config.h>
#include "sensor.h"
#include "amec_sys.h"


extern bool G_mem_monitoring_allowed;
extern task_t G_task_table[TASK_END];

uint8_t G_dimm_state = DIMM_STATE_INIT;     // Curret state of DIMM state machine
uint8_t G_maxDimmPorts = NUM_DIMM_PORTS;

bool     G_dimm_i2c_reset_required = false;
uint32_t G_dimm_i2c_reset_cause = 0;

#define MAX_CONSECUTIVE_DIMM_RESETS 1
typedef struct {
    bool     disabled;
    uint8_t  errorCount;
    uint64_t lastReading;
} dimmData_t;
dimmData_t G_dimm[NUM_DIMM_PORTS][NUM_DIMMS_PER_CENTAUR] = {{{false,0}}};

#define DIMM_TICK (CURRENT_TICK % MAX_NUM_TICKS)

// If still no i2c interrupt after MAX_TICK_COUNT_WAIT, then try next operation anyway
#define MAX_TICK_COUNT_WAIT 2

#define DIMM_AND_PORT ((G_dimm_sm_args.i2cPort<<8) | G_dimm_sm_args.dimm)
// GPE Requests
GpeRequest G_dimm_sm_request;
// GPE arguments
GPE_BUFFER(dimm_sm_args_t  G_dimm_sm_args);


// Read OCC_MISC register to see if an I2C interrupt was generated for
// the specified engine.
bool check_for_i2c_interrupt(const uint8_t i_engine)
{
    bool l_interruptTriggered = false;
    ocb_occmisc_t occmisc;
    occmisc.value = in32(OCB_OCCMISC);

    // I2CM_INTR_STATUS has a one bit status for each engine: C, D, and E
    if (PIB_I2C_ENGINE_E == i_engine)
    {
        // Engine E
        l_interruptTriggered = occmisc.fields.i2cm_intr_status & 0x01;
    }
    else if (PIB_I2C_ENGINE_D == i_engine)
    {
        // Engine D
        l_interruptTriggered = occmisc.fields.i2cm_intr_status & 0x02;
    }
    else if (PIB_I2C_ENGINE_C == i_engine)
    {
        // Engine C
        l_interruptTriggered = occmisc.fields.i2cm_intr_status & 0x04;
    }
    else
    {
        // Invalid engine
        DIMM_DBG("check_for_i2c_interrupt: invalid engine 0x%02X", i_engine);
    }

    if (!l_interruptTriggered)
    {
        DIMM_DBG("check_for_i2c_interrupt: no interrupt for engine 0x%02X", i_engine);
    }

    return l_interruptTriggered;

} // end check_for_i2c_interrupt()


// Determine the I2C address for specified DIMM
uint8_t get_dimm_addr(uint8_t i_dimm)
{
    //if (MEMORY_TYPE_NIMBUS == G_sysConfigData.mem_type)
    return 0x30 | (i_dimm<<1);
}


// Initialize the memory task data
void memory_init()
{
    if(G_mem_monitoring_allowed)
    {
        // Check if memory task is running (default task is for NIMBUS)
        const task_id_t mem_task = TASK_ID_DIMM_SM;
        if(!rtl_task_is_runnable(mem_task))
        {
            if (MEM_TYPE_NIMBUS ==  G_sysConfigData.mem_type)
            {
                // Init DIMM state manager IPC request
                memory_nimbus_init();
            }
            else
            {
                // TODO CUMULUS NOT SUPPORTED YET IN PHASE1
#if 0
                TRAC_INFO("memory_init: calling centaur_init()");
                centaur_init(); //no rc, handles errors internally
#endif
                TRAC_ERR("memory_init: invalid memory type 0x%02X", G_sysConfigData.mem_type);
                /*
                 * @errortype
                 * @moduleid    DIMM_MID_MEMORY_INIT
                 * @reasoncode  MEMORY_INIT_FAILED
                 * @userdata1   memory type
                 * @userdata2   0
                 * @devdesc     Invalid memory type detected
                 */
                errlHndl_t err = createErrl(DIMM_MID_MEMORY_INIT,
                                            MEMORY_INIT_FAILED,
                                            OCC_NO_EXTENDED_RC,
                                            ERRL_SEV_PREDICTIVE,
                                            NULL,
                                            DEFAULT_TRACE_SIZE,
                                            G_sysConfigData.mem_type,
                                            0);
                REQUEST_RESET(err);
            }

            // check if the init resulted in a reset
            if(isSafeStateRequested())
            {
                TRAC_ERR("memory_init: OCC is being reset, memory init failed (type=0x%02X)",
                         G_sysConfigData.mem_type);
            }
            else
            {
                // Initialization was successful.  Set task flags to allow memory
                // tasks to run and also prevent from doing initialization again.
                G_task_table[mem_task].flags = MEMORY_DATA_RTL_FLAGS;
                //G_task_table[TASK_ID_CENTAUR_CONTROL].flags = MEMORY_CONTROL_RTL_FLAGS;
            }
        }
    }

} // end memory_init()


// Create DIMM state machine IPC request
void memory_nimbus_init()
{
    DIMM_DBG("memory_nimbus_init: Creating request GPE1 DIMM request objects");
    gpe_request_create(&G_dimm_sm_request,
                       &G_async_gpe_queue1,             // queue
                       IPC_ST_DIMM_SM_FUNCID,           // entry_point
                       &G_dimm_sm_args,                 // entry_point arg
                       SSX_WAIT_FOREVER,                // no timeout
                       NULL,                            // callback
                       NULL,                            // callback arg
                       ASYNC_CALLBACK_IMMEDIATE);       // options
}


// Scan all of the DIMM temps and keep track of the hottest
void update_hottest_dimm()
{
    // Find/save the hottest DIMM temperature for the last set of readings
    uint8_t hottest = 0, hottest_loc = 0;
    int pIndex, dIndex;
    for (pIndex = 0; pIndex < G_maxDimmPorts; ++pIndex)
    {
        for (dIndex = 0; dIndex < NUM_DIMMS_PER_CENTAUR; ++dIndex)
        {
            if (g_amec->proc[0].memctl[pIndex].centaur.dimm_temps[dIndex].cur_temp > hottest)
            {
                hottest = g_amec->proc[0].memctl[pIndex].centaur.dimm_temps[dIndex].cur_temp;
                hottest_loc = (pIndex*8) + dIndex;
            }
        }
    }

    DIMM_DBG("update_hottest_dimm: hottest DIMM temp for this sample: %dC (loc=%d)", hottest, hottest_loc);
    if(hottest > g_amec->proc[0].memctl[0].centaur.tempdimmax.sample_max)
    {
        // Save hottest DIMM location ever sampled
        DIMM_DBG("update_hottest_dimm: Hottest DIMM ever sampled was DIMM%d %dC (prior %dC)",
                 hottest_loc, hottest,  g_amec->proc[0].memctl[0].centaur.tempdimmax.sample_max);
        sensor_update(&g_amec->proc[0].memctl[0].centaur.locdimmax, hottest_loc);
    }
    // Nimbus has no Centaurs, but store hottest temp in memctl[0]
    sensor_update(&g_amec->proc[0].memctl[0].centaur.tempdimmax, hottest);
}


// Update current I2C port/DIMM index to next potential DIMM
void use_next_dimm(uint8_t * i_port, uint8_t * i_dimm)
{
    if (++*i_dimm == NUM_DIMMS_PER_CENTAUR)
    {
        // Finished all DIMMs for current port, switch to new port
        *i_port = 1 - *i_port;
        *i_dimm = 0;
    }

    // Check if we are starting a new set of readings and if so, update hottest DIMM
    if ((*i_port == 0) && (*i_dimm == 0))
    {
        update_hottest_dimm();
    }
}


// Called after a failure to read a DIMM temperature.  The error will
// be counted and if threshold is reached, and error will be created with
// the DIMM as a callout and then set flag to trigger I2C reset
void mark_dimm_failed()
{
    const uint8_t port = G_dimm_sm_args.i2cPort;
    const uint8_t dimm = G_dimm_sm_args.dimm;
    INTR_TRAC_ERR("mark_dimm_failed: DIMM%04X failed in state %d with rc=0x%02X "
                  "(ffdc 0x%08X%08X, err_count=%d, completion_state 0x%02X)",
                  DIMM_AND_PORT, G_dimm_sm_args.state,
                  G_dimm_sm_args.error.rc,
                  WORD_HIGH(G_dimm_sm_args.error.ffdc),
                  WORD_LOW(G_dimm_sm_args.error.ffdc),
                  G_dimm[port][dimm].errorCount,
                  G_dimm_sm_request.request.completion_state);

    if (++G_dimm[port][dimm].errorCount > MAX_CONSECUTIVE_DIMM_RESETS)
    {
        // Disable collection on this DIMM, collect FFDC and log error
        G_dimm[port][dimm].disabled = true;
        INTR_TRAC_ERR("mark_dimm_failed: disabling DIMM%04X due to %d consecutive errors (state=%d)",
                      DIMM_AND_PORT, G_dimm[port][dimm].errorCount, G_dimm_sm_args.state);
        errlHndl_t l_err = NULL;
        /*
         * @errortype
         * @moduleid    DIMM_MID_MARK_DIMM_FAILED
         * @reasoncode  DIMM_GPE_FAILURE
         * @userdata1   GPE returned rc code
         * @userdata4   ERC_DIMM_COMPLETE_FAILURE
         * @devdesc     Failure writing dimm i2c mode register
         */
        l_err = createErrl(DIMM_MID_MARK_DIMM_FAILED,
                           DIMM_GPE_FAILURE,
                           ERC_DIMM_COMPLETE_FAILURE,
                           ERRL_SEV_INFORMATIONAL,
                           NULL,
                           DEFAULT_TRACE_SIZE,
                           G_dimm_sm_args.error.rc,
                           0);
        addUsrDtlsToErrl(l_err,
                         (uint8_t*)&G_dimm_sm_request.ffdc,
                         sizeof(G_dimm_sm_request.ffdc),
                         ERRL_STRUCT_VERSION_1,
                         ERRL_USR_DTL_BINARY_DATA);
        addCalloutToErrl(l_err,
                         ERRL_CALLOUT_TYPE_HUID,
                         G_sysConfigData.dimm_huids[port][dimm],
                         ERRL_CALLOUT_PRIORITY_HIGH);
        commitErrl(&l_err);
    }

    // Reset DIMM I2C engine
    G_dimm_i2c_reset_required = true;
    G_dimm_i2c_reset_cause = port<<24 | dimm<<16 | (G_dimm_sm_args.error.rc & 0xFFFF);
    G_dimm_state = DIMM_STATE_RESET_MASTER;

} // end mark_dimm_failed()


// Schedule a GPE request for the specified DIMM state
bool schedule_dimm_req(uint8_t i_state)
{
    bool l_scheduled = false;
    bool scheduleRequest = true;

    DIMM_DBG("dimm_sm called with state 0x%02X (tick=%d)", i_state, DIMM_TICK);

    if (!async_request_is_idle(&G_dimm_sm_request.request))
    {
        INTR_TRAC_ERR("dimm_sm: request is not idle.");
    }
    else
    {
        switch(i_state)
        {
            // Init
            case DIMM_STATE_INIT:
                break;

                // Read DIMM temp
            case DIMM_STATE_WRITE_MODE:
            case DIMM_STATE_WRITE_ADDR:
            case DIMM_STATE_INITIATE_READ:
            case DIMM_STATE_READ_TEMP:
                break;

                // I2C reset
            case DIMM_STATE_RESET_MASTER:
            case DIMM_STATE_RESET_SLAVE_P0:
            case DIMM_STATE_RESET_SLAVE_P0_COMPLETE:
            case DIMM_STATE_RESET_SLAVE_P1:
            case DIMM_STATE_RESET_SLAVE_P1_COMPLETE:
                break;

            default:
                INTR_TRAC_ERR("dimm_sm: Invalid state (0x%02X)", i_state);
                errlHndl_t err = NULL;
                /*
                 * @errortype
                 * @moduleid    DIMM_MID_DIMM_SM
                 * @reasoncode  DIMM_INVALID_STATE
                 * @userdata1   DIMM state
                 * @userdata2   0
                 * @devdesc     Invalid DIMM I2C state requested
                 */
                err = createErrl(DIMM_MID_DIMM_SM,
                                 DIMM_INVALID_STATE,
                                 OCC_NO_EXTENDED_RC,
                                 ERRL_SEV_PREDICTIVE,
                                 NULL,
                                 DEFAULT_TRACE_SIZE,
                                 i_state,
                                 0);
                // Request reset since this should never happen.
                REQUEST_RESET(err);
                scheduleRequest = false;
                break;
        }

        if (scheduleRequest)
        {
            // Clear errors and init common arguments for GPE
            G_dimm_sm_args.error.error = 0;
            G_dimm_sm_args.state = i_state;

            DIMM_DBG("dimm_sm: Scheduling GPE1 DIMM I2C state 0x%02X (tick %d)", i_state, DIMM_TICK);
            int l_rc = gpe_request_schedule(&G_dimm_sm_request);
            if (0 == l_rc)
            {
                l_scheduled = true;
            }
            else
            {
                errlHndl_t l_err = NULL;
                INTR_TRAC_ERR("dimm_sm: schedule failed w/rc=0x%08X (%d us)",
                              l_rc, (int) ((ssx_timebase_get())/(SSX_TIMEBASE_FREQUENCY_HZ/1000000)));
                /*
                 * @errortype
                 * @moduleid    DIMM_MID_DIMM_SM
                 * @reasoncode  SSX_GENERIC_FAILURE
                 * @userdata1   GPE shedule returned rc code
                 * @userdata2   state
                 * @devdesc     dimm_sm schedule failed
                 */
                l_err = createErrl(DIMM_MID_DIMM_SM,
                                   SSX_GENERIC_FAILURE,
                                   ERC_DIMM_SCHEDULE_FAILURE,
                                   ERRL_SEV_PREDICTIVE,
                                   NULL,
                                   DEFAULT_TRACE_SIZE,
                                   l_rc,
                                   i_state);
                // Request reset since this should never happen.
                REQUEST_RESET(l_err);
            }
        }
    }

    return l_scheduled;

} // end schedule_dimm_req()


// Check if the last I2C operation completed, and force failure if not
bool check_for_i2c_failure()
{
    bool failed = false;

    if (false == G_dimm_i2c_reset_required)
    {
        // Check if I2C operation is complete
        if (ASYNC_REQUEST_STATE_COMPLETE == G_dimm_sm_request.request.completion_state)
        {
            // Check if I2C operation failed
            if ((GPE_RC_SUCCESS != G_dimm_sm_args.error.rc) &&
                (GPE_RC_NOT_COMPLETE != G_dimm_sm_args.error.rc))
            {
                mark_dimm_failed();
                failed = true;
            }
        }
    }
    return failed;
}


// Handle the DIMM reset states
uint8_t dimm_reset_sm()
{
    uint8_t nextState = G_dimm_state;

    switch (G_dimm_state)
    {
        case DIMM_STATE_RESET_MASTER:
            if (DIMM_TICK == 0)
            {
                G_dimm_sm_args.i2cEngine = G_sysConfigData.dimm_i2c_engine;
                if (schedule_dimm_req(DIMM_STATE_RESET_MASTER))
                {
                    nextState = DIMM_STATE_RESET_SLAVE_P0;
                }
            }
            // else wait for tick 0
            break;

        case DIMM_STATE_RESET_SLAVE_P0:
            G_dimm_sm_args.i2cPort = 0;
            if (schedule_dimm_req(DIMM_STATE_RESET_SLAVE_P0))
            {
                nextState = DIMM_STATE_RESET_SLAVE_P0_WAIT;
            }
            break;

        case DIMM_STATE_RESET_SLAVE_P0_WAIT:
            // Delay to allow reset to complete
            DIMM_DBG("dimm_reset_sm: waiting during slave port 0 reset");
            nextState = DIMM_STATE_RESET_SLAVE_P0_COMPLETE;
            break;

        case DIMM_STATE_RESET_SLAVE_P0_COMPLETE:
            if (schedule_dimm_req(DIMM_STATE_RESET_SLAVE_P0_COMPLETE))
            {
                if (G_maxDimmPorts > 1)
                {
                    nextState = DIMM_STATE_RESET_SLAVE_P1;
                }
                else
                {
                    // If there is only one port, skip slave port 1
                    nextState = DIMM_STATE_INIT;
                    G_dimm_i2c_reset_required = false;
                    TRAC_INFO("dimm_reset_sm: I2C reset completed (1 port)");
                }
            }
            break;

        case DIMM_STATE_RESET_SLAVE_P1:
            G_dimm_sm_args.i2cPort = 1;
            if (schedule_dimm_req(DIMM_STATE_RESET_SLAVE_P1))
            {
                nextState = DIMM_STATE_RESET_SLAVE_P1_WAIT;
            }
            break;

        case DIMM_STATE_RESET_SLAVE_P1_WAIT:
            // Delay to allow reset to complete
            nextState = DIMM_STATE_RESET_SLAVE_P1_COMPLETE;
            break;

        case DIMM_STATE_RESET_SLAVE_P1_COMPLETE:
            if (schedule_dimm_req(DIMM_STATE_RESET_SLAVE_P1_COMPLETE))
            {
                nextState = DIMM_STATE_INIT;
                G_dimm_i2c_reset_required = false;
                TRAC_INFO("dimm_reset_sm: I2C reset completed");
            }
            break;

        default:
            INTR_TRAC_ERR("dimm_reset_sm: INVALID STATE: 0x%02X when reset is required", G_dimm_state);
            nextState = DIMM_STATE_RESET_MASTER;
            break;
    }

    return nextState;

} // end dimm_reset_sm()


// Function Specification
//
// Name:  task_dimm_sm
//
// Description: DIMM State Machine - Called every other tick to collect all of
//              the DIMM temperatures.
//
// Task Flags: RTL_FLAG_ACTIVE
//
// End Function Specification
void task_dimm_sm(struct task *i_self)
{
    static uint8_t L_dimmIndex = 0x00;
    static uint8_t L_dimmPort  = 0x00;
    static uint8_t L_notReadyCount = 0;
#define MAX_READ_ATTEMPT 3
    static uint8_t L_readAttempt = 0;
    static bool L_readIssued = false;

    if (G_mem_monitoring_allowed)
    {
        //DIMM_DBG("task_dimm_sm: request state=0x%02X, completion_state=0x%02X, abort_state=0x%02X",
        //         G_dimm_sm_request.request.state, G_dimm_sm_request.request.completion_state, G_dimm_sm_request.request.abort_state);

        if (check_for_i2c_failure())
        {
            // After reset, move on to next DIMM
            use_next_dimm(&L_dimmPort, &L_dimmIndex);
        }

        uint8_t nextState = G_dimm_state;

        if (G_dimm_i2c_reset_required)
        {
            nextState = dimm_reset_sm();
        }
        else if (G_dimm_state == DIMM_STATE_INIT)
        {
            // Setup I2C Interrupt Mask Register
            DIMM_DBG("DIMM_STATE_INIT: (I2C Engine 0x%02X, Memory Type 0x%02X)",
                     G_sysConfigData.dimm_i2c_engine, G_sysConfigData.mem_type);
            G_dimm_sm_args.i2cEngine = G_sysConfigData.dimm_i2c_engine;
            if (schedule_dimm_req(DIMM_STATE_INIT))
            {
                nextState = DIMM_STATE_WRITE_MODE;
            }
        }
        else
        {
            bool intTriggered = check_for_i2c_interrupt(G_sysConfigData.dimm_i2c_engine);
            if (intTriggered == false)
            {
                ++L_notReadyCount;
            }

            // Check if prior command completed (or timed out waiting for it)
            if (intTriggered || (L_notReadyCount > MAX_TICK_COUNT_WAIT))
            {
                if (ASYNC_REQUEST_STATE_COMPLETE == G_dimm_sm_request.request.completion_state)
                {
                    // IPC request completed, now check return code
                    if (GPE_RC_SUCCESS == G_dimm_sm_args.error.rc)
                    {
                        // last request completed without error
                        switch (G_dimm_sm_args.state)
                        {
                            case DIMM_STATE_INIT:
                                // Save max I2C ports
                                if (G_maxDimmPorts != G_dimm_sm_args.maxPorts)
                                {
                                    G_maxDimmPorts = G_dimm_sm_args.maxPorts;
                                    DIMM_DBG("task_dimm_sm: updating DIMM Max I2C Ports to %d", G_maxDimmPorts);
                                }
                                break;

                            case DIMM_STATE_READ_TEMP:
                                if (L_readIssued)
                                {
                                    const uint8_t port = G_dimm_sm_args.i2cPort;
                                    const uint8_t dimm = G_dimm_sm_args.dimm;

                                    // Last DIMM read completed, update sensor and clear error count
                                    DIMM_DBG("task_dimm_sm: Successfully read DIMM%04X temperature: %dC, tick %d",
                                             DIMM_AND_PORT, G_dimm_sm_args.temp, DIMM_TICK);
                                    g_amec->proc[0].memctl[port].centaur.dimm_temps[dimm].cur_temp = G_dimm_sm_args.temp;
                                    G_dimm[port][dimm].lastReading = ((ssx_timebase_get())/(SSX_TIMEBASE_FREQUENCY_HZ/1000000));
                                    G_dimm[port][dimm].errorCount = 0;

                                    // Move on to next DIMM
                                    use_next_dimm(&L_dimmPort, &L_dimmIndex);
                                    L_readIssued = false;
                                }
                                break;

                            default:
                                // Nothing to do
                                break;
                        }
                    }
                    else
                    {
                        // last request did not return success
                        switch (G_dimm_sm_args.state)
                        {
                            case DIMM_STATE_INITIATE_READ:
                                if (++L_readAttempt < MAX_READ_ATTEMPT)
                                {
                                    // The initiate_read didnt complete, retry
                                    DIMM_DBG("task_dimm_sm: initiate read didn't start (%d attempts)", L_readAttempt);
                                    // Force the read again
                                    G_dimm_state = DIMM_STATE_INITIATE_READ;
                                    nextState = G_dimm_state;
                                }
                                else
                                {
                                    INTR_TRAC_ERR("task_dimm_sm: initiate read didn't start after %d attempts... forcing reset", L_readAttempt);
                                    mark_dimm_failed();
                                }
                                break;

                            case DIMM_STATE_READ_TEMP:
                                if (L_readIssued)
                                {
                                    if (++L_readAttempt < MAX_READ_ATTEMPT)
                                    {
                                        DIMM_DBG("task_dimm_sm: read didn't complete (%d attempts)", L_readAttempt);
                                        // Force the read again
                                        G_dimm_state = DIMM_STATE_READ_TEMP;
                                        nextState = G_dimm_state;
                                    }
                                    else
                                    {
                                        INTR_TRAC_ERR("task_dimm_sm: read did not complete after %d attempts... forcing reset", L_readAttempt);
                                        mark_dimm_failed();
                                    }
                                }
                                break;

                            default:
                                // Nothing to do
                                break;
                        }
                    }
                }
            }

            if (false == G_dimm_i2c_reset_required)
            {
                // Handle new DIMM state
                switch (G_dimm_state)
                {
                    case DIMM_STATE_WRITE_MODE:
                        // Only start a DIMM read on tick 0 or 8
                        if ((DIMM_TICK == 0) || (DIMM_TICK == 8))
                        {
                            // If DIMM has huid/sensor then it should be present
                            if ((0 != G_sysConfigData.dimm_huids[L_dimmPort][L_dimmIndex]) &&
                                (G_dimm[L_dimmPort][L_dimmIndex].disabled == false))
                            {
                                G_dimm_sm_args.i2cPort = L_dimmPort;
                                G_dimm_sm_args.dimm = L_dimmIndex;
                                DIMM_DBG("task_dimm_sm: Starting collection for DIMM%04X at tick %d",
                                         DIMM_AND_PORT, DIMM_TICK);
                                if (schedule_dimm_req(DIMM_STATE_WRITE_MODE))
                                {
                                    nextState = DIMM_STATE_WRITE_ADDR;
                                }
                            }
                            else
                            {
                                // Skip current DIMM and move on to next one
                                use_next_dimm(&L_dimmPort, &L_dimmIndex);
                            }
                        }
                        break;

                    case DIMM_STATE_WRITE_ADDR:
                        if (intTriggered || (L_notReadyCount > MAX_TICK_COUNT_WAIT))
                        {
                            G_dimm_sm_args.dimm = L_dimmIndex;
                            G_dimm_sm_args.i2cAddr = get_dimm_addr(L_dimmIndex);
                            if (schedule_dimm_req(DIMM_STATE_WRITE_ADDR))
                            {
                                nextState = DIMM_STATE_INITIATE_READ;
                                L_readAttempt = 0;
                                L_readIssued = false;
                            }
                        }
                        break;

                    case DIMM_STATE_INITIATE_READ:
                        if (intTriggered || (L_notReadyCount > MAX_TICK_COUNT_WAIT))
                        {
                            G_dimm_sm_args.dimm = L_dimmIndex;
                            if (schedule_dimm_req(DIMM_STATE_INITIATE_READ))
                            {
                                nextState = DIMM_STATE_READ_TEMP;
                            }
                        }
                        break;

                    case DIMM_STATE_READ_TEMP:
                        if (intTriggered || (L_notReadyCount > MAX_TICK_COUNT_WAIT))
                        {
                            if (schedule_dimm_req(DIMM_STATE_READ_TEMP))
                            {
                                L_readIssued = true;
                                nextState = DIMM_STATE_WRITE_MODE;
                            }
                        }
                        break;

                    default:
                        INTR_TRAC_ERR("task_dimm_sm: INVALID STATE: 0x%02X", G_dimm_state);
                        break;
                }
            }
            else
            {
                // Previous op triggered reset
                nextState = dimm_reset_sm();
            }
        }

        if (nextState != G_dimm_state)
        {
            DIMM_DBG("task_dimm_sm: Updating state to 0x%02X (DIMM%04X) end of tick %d", nextState, (L_dimmPort<<8)|L_dimmIndex, DIMM_TICK);
            G_dimm_state = nextState;
            L_notReadyCount = 0;
        }
    }

} // end task_dimm_sm()


