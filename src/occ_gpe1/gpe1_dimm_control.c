/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe1/gpe1_dimm_control.c $                            */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2019                        */
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
#include "ppe42_scom.h"
#include "ipc_api.h"
#include "ipc_async_cmd.h"
#include "gpe_err.h"
#include "gpe_util.h"
#include "dimm_structs.h"
#include "mca_addresses.h"
#include "gpe1.h"
#include "gpe1_dimm.h"


/*
 * Function Specification:
 *
 * Name: gpe1_sleep
 *
 * Description:  Delay for a specified period of time.
 *
 * Inputs:       i_microseconds: time to sleep in microseconds
 *
 * Note:         i_microseconds must be < (2^16)/nest_freq microseconds.
 *               (about 114 seconds for the fastest nest_freq supported)
 *
 * return:       none
 *
 * End Function Specification
 */
void gpe1_sleep(uint32_t i_microseconds)
{
    uint32_t current_count = pk_timebase32_get();
    uint32_t prev_count = current_count;
    uint32_t timebase_zero_adjust = -current_count;
    uint32_t change_timeout = 0;
    uint32_t end_count = PK_INTERVAL_SCALE((uint32_t)PK_MICROSECONDS(i_microseconds));

    while((current_count + timebase_zero_adjust) < end_count)
    {
        prev_count = current_count;

        current_count = pk_timebase32_get();

        if(prev_count == current_count)
        {
            ++change_timeout;
            if(change_timeout > 32)
            {
                PK_TRACE("TIMEBASE is not moving!");
                // timebase is not moving
                break;
            }
        }
        else
        {
            change_timeout = 0;
        }
    }
}


/*
 * Function Specifications:
 *
 * Name: gpe_scom_nvdimms_nimbus
 *
 * Description: Sends SCOMs to NVDIMMs to tell them to back up their data.
 *              Occurs when GPIO_EPOW is asserted.
 *
 * Inputs:      cmd is a pointer to IPC msg's cmd and cmd_data struct
 *
 * End Function Specification
 */
void gpe_scom_nvdimms_nimbus(ipc_msg_t* cmd, void* arg)
{
    int rc = 0;
    int mc = 0;
    int port = 0;
    uint64_t regValue = 0;
    uint64_t mbarpc_regValue = 0;
    uint64_t mbastr_regValue = 0;
    ipc_async_cmd_t *async_cmd = (ipc_async_cmd_t*)cmd;

    epow_gpio_args_t *args = (epow_gpio_args_t*)async_cmd->cmd_data;

    PK_TRACE("gpe_scom_nvdimms_nimbus: configured_mbas: 0x%04x",
                args->configured_mbas );

    // If mc is configured, send scoms

    // OCC does not know which slots the NVDIMMs are installed in.
    // NVDIMMs will only be installed in ports 6 & 7
    // To expedite the save of data, the scoms will only be done for those 2 ports
    // mc=0 => MCS0 --> MCA port 0 --> DIMM0/1
    //                  MCA port 1 --> DIMM0/1
    //         MCS1 --> MCA port 2 --> DIMM0/1
    //                  MCA port 3 --> DIMM0/1
    // mc=1 => MCS2 --> MCA port 4 --> DIMM0/1
    //                  MCA port 5 --> DIMM0/1
    //         MCS3 --> MCA port 6 --> DIMM0/1 (potential NVDIMMs)
    //                  MCA port 7 --> DIMM0/1 (potential NVDIMMs)

    mc = 1;

    // Step 0 - In MCFGP, set MCFGP_VALID
    //
    //          bit  0    - MCFGP_VALID
    const uint32_t MCFGP_ADDRESS_MCS3 = 0x0301088A;
    rc = getscom_abs(MCFGP_ADDRESS_MCS3, &regValue);
    if (rc)
    {
        PK_TRACE("E>gpe_scom_nvdimms_nimbus: Failed to read (MCFGP) Reg:0x%08X, rc:0x%08x",
                 MCFGP_ADDRESS_MCS3, rc);
    }
    else
    {
        regValue |= 0x8000000000000000; // set MCFGP_VALID (bit 0)
        PK_TRACE("gpe_scom_nvdimms_nimbus: Writing MCFGP (0x%08X) to 0x%08X%08X",
                 MCFGP_ADDRESS_MCS3, WORD_HIGH(regValue), WORD_LOW(regValue));
        rc = putscom_abs(MCFGP_ADDRESS_MCS3, regValue);
        if (rc)
        {
            PK_TRACE("E>gpe_scom_nvdimms_nimbus: Failed to set MCFGP_VALID (MCFGP)"
                     " Reg:0x%08X, Data:0x%08X %08X, rc:0x%08x",
                     MCFGP_ADDRESS_MCS3, WORD_HIGH(regValue), WORD_LOW(regValue), rc);
            gpe_set_ffdc(&(args->error), MCFGP_ADDRESS_MCS3,
                         GPE_RC_SCOM_PUT_FAILED, rc);
        }
    }

    const uint32_t reg_MCBIST = MCBIST_CTRL_REG(mc);
    rc = getscom_abs(reg_MCBIST, &regValue);
    if (rc)
    {
        PK_TRACE("E>gpe_scom_nvdimms_nimbus: Failed to read (MCBIST) Reg:0x%08X, rc:0x%08x",
                 reg_MCBIST, rc);
    }
    else
    {
        // Step 1 - In MCBIST_CTRL_REG, stop mcbist
        //          (must be done first)
        //
        //          bit  1   - MCB_STOP
        regValue |= 0x4000000000000000; // stop mcbist (set bit 1)
        rc = putscom_abs(reg_MCBIST, regValue);
        if (rc)
        {
            PK_TRACE("E>gpe_scom_nvdimms_nimbus: Failed to stop mcbist (MCBIST)"
                     " Reg:0x%08X, Data:0x%08X %08X, rc:0x%08x",
                     reg_MCBIST, WORD_HIGH(regValue), WORD_LOW(regValue), rc);
            gpe_set_ffdc(&(args->error), reg_MCBIST, GPE_RC_SCOM_PUT_FAILED, rc);
        }
    }

    // Iterate over each bit in configured_mbas for MCS3 (MCA port 6 and 7)
    uint16_t mask = 0x8000 >> 6; // Starting at MCS3 / MCA port 6
    for (port = 2; port < NUM_PORTS_PER_MBA; ++port)
    {
        if (args->configured_mbas & mask)
        {
            PK_TRACE("gpe_scom_nvdimms_nimbus: scoms for mc: %d, port: %d", mc, port);

            const uint32_t reg_MBARPC0Q = POWER_CTRL_REG0(mc,port);
            rc = getscom_abs(reg_MBARPC0Q, &mbarpc_regValue);
            if (rc)
            {
                PK_TRACE("E>gpe_scom_nvdimms_nimbus: Failed to read (MBARPC0Q) Reg:0x%08X, rc:0x%08x",
                         reg_MBARPC0Q, rc);
            }
            else
            {
                // Step 2 - In MBARPC0Q, disable power domain control
                //          (must be done before domains are set)
                //
                //          bit  2   - power domain control
                mbarpc_regValue &= 0xDFFFFFFFFFFFFFFF; // disable power control (clear bit 2)
                rc = putscom_abs(reg_MBARPC0Q, mbarpc_regValue);
                if (rc)
                {
                    PK_TRACE("E>gpe_scom_nvdimms_nimbus: Failed to disable power domain control (MBARPC0Q)"
                             " Reg:0x%08X, Data:0x%08X %08X, rc:0x%08x",
                             reg_MBARPC0Q, WORD_HIGH(mbarpc_regValue), WORD_LOW(mbarpc_regValue), rc);
                    gpe_set_ffdc(&(args->error), reg_MBARPC0Q,
                                 GPE_RC_SCOM_PUT_FAILED, rc);
                }

                // Step 3 - In MBARPC0Q, set domain to MAXALL_MIN0(0b010),
                //          disable minimum domain reduction and set power down delay to 0
                //
                //          bits 3:5   - min/max domains
                //          bit  22    - min domain reduction enable
                //          bit  23:32 - min domain reduction time
                mbarpc_regValue &= 0xEBFFFC007FFFFFFF; // MAXALL_MIN0 & delay (clear bits 3,5,22-32)
                mbarpc_regValue |= 0x0800000000000000; // MAXALL_MIN0 (set bit 4)
                rc = putscom_abs(reg_MBARPC0Q, mbarpc_regValue);
                if (rc)
                {
                    PK_TRACE("E>gpe_scom_nvdimms_nimbus: Failed to set domains (MBARPC0Q)"
                             " Reg:0x%08X, Data:0x%08X %08X, rc:0x%08x",
                             reg_MBARPC0Q, WORD_HIGH(mbarpc_regValue), WORD_LOW(mbarpc_regValue), rc);
                    gpe_set_ffdc(&(args->error), reg_MBARPC0Q,
                                 GPE_RC_SCOM_PUT_FAILED, rc);
                }

                // Step 4 - In MBASTR0Q, enable STR entry and set entry delay to 0
                //
                //          bit  0    - STR enable
                //          bit  2:11 - STR entry time
                const uint32_t reg_MBASTR0Q = STR_REG0(mc,port);
                rc = getscom_abs(reg_MBASTR0Q, &mbastr_regValue);
                if (rc)
                {
                    PK_TRACE("E>gpe_scom_nvdimms_nimbus: Failed to read (MBASTR0Q) Reg:0x%08X, rc:0x%08x",
                             reg_MBASTR0Q, rc);
                }
                else
                {
                    mbastr_regValue &= 0xC00FFFFFFFFFFFFF; // set entry time to 0 (clear bits 2-11)
                    mbastr_regValue |= 0x8000000000000000; // enable STR (set bit 0)
                    rc = putscom_abs(reg_MBASTR0Q, mbastr_regValue);
                    if (rc)
                    {
                        PK_TRACE("E>gpe_scom_nvdimms_nimbus: Failed to enable STR entry (MBASTR0Q)"
                                 " Reg:0x%08X, Data:0x%08X %08X, rc:0x%08x",
                                 reg_MBASTR0Q, WORD_HIGH(mbastr_regValue), WORD_LOW(mbastr_regValue), rc);
                        gpe_set_ffdc(&(args->error), reg_MBASTR0Q,
                                     GPE_RC_SCOM_PUT_FAILED, rc);
                    }
                }

                // Step 5 - In MBARPC0Q, re-enable power domain control
                //
                //          bit  2   - power domain control
                mbarpc_regValue |= 0x2000000000000000; // enable power control (set bit 2)
                rc = putscom_abs(reg_MBARPC0Q, mbarpc_regValue);
                if (rc)
                {
                    PK_TRACE("E>gpe_scom_nvdimms_nimbus: Failed to re-enable power domain control (MBARPC0Q)"
                             " Reg:0x%08X, Data:0x%08X %08X, rc:0x%08x",
                             reg_MBARPC0Q, WORD_HIGH(mbarpc_regValue), WORD_LOW(mbarpc_regValue), rc);
                    gpe_set_ffdc(&(args->error), reg_MBARPC0Q,
                                 GPE_RC_SCOM_PUT_FAILED, rc);
                }

                // Step 6 (new) - In FARB0Q (Final Arb parameters), disable rcd recovery
                //
                //          bit  54   - Disable the rcd recovery procedure
                const uint32_t reg_FARB0Q = FINAL_ARB_PARMS(mc,port);
                rc = getscom_abs(reg_FARB0Q, &regValue);
                if (rc)
                {
                    PK_TRACE("E>gpe_scom_nvdimms_nimbus: Failed to read (FARB0Q) Reg:0x%08X, rc:0x%08x",
                             reg_FARB0Q, rc);
                }
                else
                {
                    regValue |= 0x0000000000000200; // disable rcd recovery procedure (set bit 54)
                    rc = putscom_abs(reg_FARB0Q, regValue);
                    if (rc)
                    {
                        PK_TRACE("E>gpe_scom_nvdimms_nimbus: Failed to disable rcd procedure (FARB0Q)"
                                 " Reg:0x%08X, Data:0x%08X %08X, rc:0x%08x",
                                 reg_FARB0Q, WORD_HIGH(regValue), WORD_LOW(regValue), rc);
                        gpe_set_ffdc(&(args->error), reg_FARB0Q,
                                     GPE_RC_SCOM_PUT_FAILED, rc);
                    }
                }

                // Step 7 (new) - In FARB6Q (DDR Port Status Register), check if DRAMS are in STR
                //
                //          bit  15  - Indicates if DRAMS on this port are in STR
                const uint32_t reg_FARB6Q = DDR_PORT_STATUS_REG(mc,port);
                PK_TRACE("gpe_scom_nvdimms_nimbus: Waiting for DRAM to reach STR (0x%08X)", reg_FARB6Q);
                uint64_t lastReg = 0;
                do
                {
                    rc = getscom_abs(reg_FARB6Q, &regValue);
                    if (rc)
                    {
                        PK_TRACE("E>gpe_scom_nvdimms_nimbus: Failed to read (FARB6Q) Reg:0x%08X, rc:0x%08x",
                                 reg_FARB6Q, rc);
                        break;
                    }
                    else
                    {
                        if (lastReg != regValue)
                        {
                            PK_TRACE("gpe_scom_nvdimms_nimbus: FARB6Q = 0x%08X %08X", WORD_HIGH(regValue), WORD_LOW(regValue));
                            lastReg = regValue;
                        }
                    }
                }
                while ((regValue & 0x0001000000000000) == 0); // wait for bit 15 to get set
                if (rc == 0)
                {
                    PK_TRACE("gpe_scom_nvdimms_nimbus: DRAM is in STR");
                }

                // Step 8 (was 6) - In FARB5Q (DDR Interface SCOM Control), assert ddr_resetn
                //
                //          bit  4   - assert ddr_resetn
                const uint32_t reg_FARB5Q = DDR_IF_SCOM_CTRL(mc,port);
                rc = getscom_abs(reg_FARB5Q, &regValue);
                if (rc)
                {
                    PK_TRACE("E>gpe_scom_nvdimms_nimbus: Failed to read (FARB5Q) Reg:0x%08X, rc:0x%08x",
                             reg_FARB5Q, rc);
                }
                else
                {
                    regValue &= 0xF7FFFFFFFFFFFFFF; // assert ddr_resetn (clear bit 4)
                    rc = putscom_abs(reg_FARB5Q, regValue);
                    if (rc)
                    {
                        PK_TRACE("E>gpe_scom_nvdimms_nimbus: Failed to assert ddr_resetn (FARB5Q)"
                                 " Reg:0x%08X, Data:0x%08X %08X, rc:0x%08x",
                                 reg_FARB5Q, WORD_HIGH(regValue), WORD_LOW(regValue), rc);
                        gpe_set_ffdc(&(args->error), reg_FARB5Q,
                                     GPE_RC_SCOM_PUT_FAILED, rc);
                    }
                }
            }
        }
        mask = mask >> 1;
    } // for each MBA port

    PK_TRACE("gpe_scom_nvdimms_nimbus: completed (rc=%d)", rc);

    // Always send back success
    rc = ipc_send_rsp(cmd, IPC_RC_SUCCESS);
    if(rc)
    {
        PK_TRACE("E>gpe_scom_nvdimms_nimbus: Failed to send response back (rc=%d)", rc);
        gpe_set_ffdc(&(args->error), 0x00, GPE_RC_IPC_SEND_FAILED, rc);
        pk_halt();
    }

    PK_TRACE("gpe_scom_nvdimms_nimbus: exiting");

} // end gpe_scom_nvdimms_nimbus()

/*
 * Function Specifications:
 *
 * Name: gpe_dimm_control
 *
 * Description:  DIMM control code on the GPE
 *
 * Inputs:       cmd is a pointer to IPC msg's cmd and cmd_data struct
 *
 * Outputs:      error: sets rc, address, and ffdc in the cmd_data's
 *                      GpeErrorStruct
 *
 * End Function Specification
 */
void gpe_dimm_control(ipc_msg_t* cmd, void* arg)
{
    // Note: arg was set to 0 in ipc func table (ipc_func_tables.c), so don't use it.
    // the ipc arguments passed through the ipc_msg_t structure, has a pointer
    // to the dimm_control_args_t struct.

    int      rc;
    uint64_t regValue; // a pointer to hold the putscom_abs register value
    ipc_async_cmd_t *async_cmd = (ipc_async_cmd_t*)cmd;
    dimm_control_args_t *args = (dimm_control_args_t*)async_cmd->cmd_data;

    int mc = args->mc, port = args->port; // memory controller pair and port

    args->error.error = 0;
    args->error.ffdc = 0;

    do
    {   // read N/M DIMM Throttling Control SCOM Register for specified MC pair and port numbers
        rc = getscom_abs(N_M_DIMM_TCR(mc,port), &regValue);
        if(rc)
        {
            PK_TRACE("gpe_dimm_control: N/M DIMM Throttling Control Register read fails"
                     "MC#|Port:0x%08x, Address:0x%08x, rc:0x%08x",
                     (uint32_t)((mc << 16) | port),
                     N_M_DIMM_TCR(mc,port), rc);

            gpe_set_ffdc(&(args->error), N_M_DIMM_TCR(mc,port),
                         GPE_RC_SCOM_GET_FAILED, rc);
            break;
        }

        // Store the M values if needed
        if( args->dimmDenominatorValues.need_m )
        {
            args->dimmDenominatorValues.m_value = ((regValue & 0x1FFF80000) >> 19);
        }

        // If this isn't set, we didn't need to set the N value, just needed M
        if(!args->dimmNumeratorValues.new_n)
        {
            break;
        }

        // Clear old N values for slot and port
        regValue &= 0x1FFFFFFFF;
        // copy slot and port N values (31 bits) from passed dimmNumeratorValues
        regValue |=  ((uint64_t) (args->dimmNumeratorValues.word32 & 0xFFFFFFFE)) << 32 ;

        // Write new slot and port N Values
        rc = putscom_abs(N_M_DIMM_TCR(mc,port), regValue);
        if(rc)
        {
            PK_TRACE("gpe_dimm_control: N/M DIMM Throttling Control Register write fails"
                     "dimm#:%d, Address:0x%04x, Nvalues:0x%08x, rc:0x%08x",
                     (uint16_t)((mc << 8) | port),
                     N_M_DIMM_TCR(mc,port), regValue, rc);

            gpe_set_ffdc(&(args->error), N_M_DIMM_TCR(mc,port),
                         GPE_RC_SCOM_PUT_FAILED, rc);
            break;
        }
        else
        {
            GPE1_DIMM_DBG("gpe_dimm_control: N/M DIMM Throttling Control Register write"
                          "mc|port#:0x%04x, Address:0x%08x, Nvalues:0x%08x",
                          (uint16_t)((mc << 8) | port),
                          N_M_DIMM_TCR(mc,port), regValue);
        }
    } while(0);

    // send back a response, IPC success even if ffdc/rc are non zeros
    rc = ipc_send_rsp(cmd, IPC_RC_SUCCESS);
    if(rc)
    {
        PK_TRACE("E>gpe_dimm_control: Failed to send response back. Halting GPE1 (rc=%d)", rc);
        gpe_set_ffdc(&(args->error), 0x00, GPE_RC_IPC_SEND_FAILED, rc);
        pk_halt();
    }
}

/*
 * Function Specifications:
 *
 * Name: gpe_reset_mem_deadman
 *
 * Description:  Read memory performance counter for one MCA.
 *               This effectively resets the memory deadman timer
 *
 * Inputs:       cmd is a pointer to IPC msg's cmd and cmd_data struct
 *
 * Outputs:      error: sets rc, address, and ffdc in the cmd_data's
 *                      GpeErrorStruct
 *
 * End Function Specification
 */

void gpe_reset_mem_deadman(ipc_msg_t* cmd, void* arg)
{
    // Note: arg was set to 0 in ipc func table (ipc_func_tables.c), so don't use it.
    // the ipc arguments passed through the ipc_msg_t structure, has a pointer
    // to the reset_mem_deadman_args_t struct.

    static bool L_init_complete[NUM_NIMBUS_MCAS] = {FALSE};
    int      rc = 0;
    uint64_t regValue; // a pointer to hold get/put SCOM register value
    ipc_async_cmd_t *async_cmd = (ipc_async_cmd_t*)cmd;
    reset_mem_deadman_args_t *args = (reset_mem_deadman_args_t*)async_cmd->cmd_data;

    args->error.error = 0;
    args->error.ffdc = 0;

    do
    {
        int mca = args->mca; // Nimbus MCA; mc_pair = mca >>2 and port = mca & 3

        // Part of init is to enable the deadman timer for this MCA
        if(!L_init_complete[mca])
        {
            // Read STR Register 0
            rc = getscom_abs(STR_REG0_MCA(mca), &regValue);

            if(rc)
            {
                PK_TRACE("gpe_reset_mem_deadman: Failed to read STR0 to program deadman timer"
                         " MCA:0x%08x, Address:0x%08x, rc:0x%08x", mca, STR_REG0_MCA(mca), rc);

                gpe_set_ffdc(&(args->error), STR_REG0_MCA(mca),
                             GPE_RC_SCOM_GET_FAILED, rc);
                break;
            }

            // set the deadman timer to the max value: bits 57:60 = 0b1000
            regValue |= 0x0000000000000040;
            regValue &= ~(0x0000000000000038);

            // Write Modified STR Register 0
            rc = putscom_abs(STR_REG0_MCA(mca), regValue);

            if(rc)
            {
                PK_TRACE("gpe_reset_mem_deadman: Failed to program deadman timer (STR REG0)"
                         " MCA:0x%08x, Data:0x%08x, rc:0x%08x", mca, (uint32_t)regValue, rc);

                gpe_set_ffdc(&(args->error), STR_REG0_MCA(mca),
                             GPE_RC_SCOM_PUT_FAILED, rc);
                break;
            }
        }

        // The "Deadman" timer is reset by reading from this performance monitor counts register
        rc = getscom_abs(PERF_MON_COUNTS_IDLE_MCA(mca), &regValue);
        if(rc)
        {
            PK_TRACE("gpe_reset_mem_deadman: Performance Monitor Counts read failed"
                     " MCA:0x%08x, Address:0x%08x, rc:0x%08x",
                     mca, PERF_MON_COUNTS_IDLE_MCA(mca), rc);

            gpe_set_ffdc(&(args->error), PERF_MON_COUNTS_IDLE_MCA(mca),
                         GPE_RC_SCOM_GET_FAILED, rc);
            break;
        }
        else
        {
            args->idle_counts.med_idle_cnt = ((regValue & 0xFFFFFFFF00000000) >> 32);
            args->idle_counts.high_idle_cnt = (regValue & 0xFFFFFFFF);
        }

        // Now that we are poking the deadman timer as second part of init check for and clear
        // any previous emergency throttle that may have happened from the last time OCC was running
        if(!L_init_complete[mca])
        {
            L_init_complete[mca] = TRUE; // Done handling initialization

            // Read Emergency Throttle Register
            rc = getscom_abs(ER_THROTTLE_MCA(mca), &regValue);

            if(rc)
            {
                PK_TRACE("gpe_reset_mem_deadman: Failed to read emergency throttle register"
                         " MCA:0x%08x, Address:0x%08x, rc:0x%08x", mca, ER_THROTTLE_MCA(mca), rc);

                gpe_set_ffdc(&(args->error), ER_THROTTLE_MCA(mca), GPE_RC_SCOM_GET_FAILED, rc);
                break;
            }

            // clear Emergency Throttle In-Progress bit if set, this is indication that OCC has been
            // re-started from permanent safe mode without an IPL
            if(regValue & ER_THROTTLE_IN_PROGRESS_MASK)
            {
                PK_TRACE("gpe_reset_mem_deadman: Enabled timer and clearing throttle for MCA:0x%08x", mca);
                regValue &= ~(ER_THROTTLE_IN_PROGRESS_MASK);

                // Write Modified Emergency Throttle Register
                rc = putscom_abs(ER_THROTTLE_MCA(mca), regValue);

                if(rc)
                {
                    PK_TRACE("gpe_reset_mem_deadman: Failed to clear emergency throttle"
                             " MCA:0x%08x, rc:0x%08x", mca, rc);

                    gpe_set_ffdc(&(args->error), ER_THROTTLE_MCA(mca), GPE_RC_SCOM_PUT_FAILED, rc);
                    break;
                }

                // Clear the emergency throttle engaged FIR bit
                rc = getscom_abs(MCA_CAL_FIR_REG_MCA(mca), &regValue);

                if(rc)
                {
                    PK_TRACE("gpe_reset_mem_deadman: Failed to read MCA FIR register"
                             " MCA:0x%08x, Address:0x%08x, rc:0x%08x",
                             mca, MCA_CAL_FIR_REG_MCA(mca), rc);

                    gpe_set_ffdc(&(args->error), MCA_CAL_FIR_REG_MCA(mca), GPE_RC_SCOM_GET_FAILED, rc);
                    break;
                }
                regValue &= ~(MCA_FIR_THROTTLE_ENGAGED_MASK);

                // Write Modified MCA FIR Register
                rc = putscom_abs(MCA_CAL_FIR_REG_MCA(mca), regValue);

                if(rc)
                {
                    PK_TRACE("gpe_reset_mem_deadman: Failed to clear emergency throttle FIR bit"
                             " MCA:0x%08x, rc:0x%08x", mca, rc);

                    gpe_set_ffdc(&(args->error), MCA_CAL_FIR_REG_MCA(mca), GPE_RC_SCOM_PUT_FAILED, rc);
                    break;
                }
            }
            else
            {
                PK_TRACE("gpe_reset_mem_deadman: Enabled timer for MCA:0x%08x", mca);
            }
        }  // if !L_init_complete

        // In addition to resetting the "deadman" counter, get some performance information
        rc = getscom_abs(PERF_MON_COUNTS0_MCA(mca), &regValue);
        if(rc)
        {
            PK_TRACE("gpe_reset_mem_deadman: Performance Monitor Counts0 read failed"
                     " MCA:0x%08x, Address:0x%08x, rc:0x%08x",
                     mca, PERF_MON_COUNTS0_MCA(mca), rc);

            gpe_set_ffdc(&(args->error), PERF_MON_COUNTS0_MCA(mca),
                         GPE_RC_SCOM_GET_FAILED, rc);
            break;
        }
        else
        {
            args->rd_wr_counts.mba_read_cnt = ((regValue & 0xFFFFFFFF00000000) >> 32);
            args->rd_wr_counts.mba_write_cnt = (regValue & 0xFFFFFFFF);
        }

    } while(0);

    // send back a response, IPC success even if ffdc/rc are non zeros
    rc = ipc_send_rsp(cmd, IPC_RC_SUCCESS);
    if(rc)
    {
        PK_TRACE("E>gpe_reset_mem_deadman: Failed to send response back. Halting GPE1 (rc=%d)", rc);
        gpe_set_ffdc(&(args->error), 0x00, GPE_RC_IPC_SEND_FAILED, rc);
        pk_halt();
    }
}
