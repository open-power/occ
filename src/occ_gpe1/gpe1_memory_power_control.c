/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe1/gpe1_memory_power_control.c $                    */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2017                        */
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
#include "mem_structs.h"
#include "gpe1.h"
#include "mca_addresses.h"
#include "gpe1_memory_power_control.h"

/*
 * Function Specifications:
 *
 * Name: gpe_mem_power_control
 *
 * Description:  memory power control code on the GPE
 *
 * Inputs:       cmd is a pointer to IPC msg's cmd and cmd_data struct
 *
 * Outputs:      error: sets rc, address, and ffdc in the cmd_data's
 *                      GpeErrorStruct
 *
 * End Function Specification
 */

void gpe_mem_power_control(ipc_msg_t* cmd, void* arg)
{
    // Note: arg was set to 0 in ipc func table (ipc_func_tables.c), so don't use it.
    // the ipc arguments passed through the ipc_msg_t structure, has a pointer
    // to the mem_power_control_args_t struct.

    int      rc;
    uint64_t pcrRegValue;      // holds Power Control Register0
    uint64_t strRegValue;      // holds STR0 SCOM register value
    ipc_async_cmd_t *async_cmd = (ipc_async_cmd_t*)cmd;
    mem_power_control_args_t *args = (mem_power_control_args_t*)async_cmd->cmd_data;

    uint8_t mc = args->mc, port = args->port;  // memory controller pair and port

    args->error.error = 0;
    args->error.ffdc = 0;

    do
    {
        // Read Power Control Register 0 SCOM
        rc = getscom_abs(POWER_CTRL_REG0(mc,port), &pcrRegValue);

        if(rc)
        {
            PK_TRACE("gpe_mem_power_control: Power Control Register 0 read fails"
                     "MC#|Port:0x%08x, Address:0x%08x, rc:0x%08x",
                     (uint32_t)((mc << 16) | port),
                     POWER_CTRL_REG0(mc,port), rc);

            gpe_set_ffdc(&(args->error), POWER_CTRL_REG0(mc,port),
                         GPE_RC_SCOM_GET_FAILED, rc);
            break;
        }

        // Read STR0 SCOM Register
        rc = getscom_abs(STR_REG0(mc,port), &strRegValue);

        if(rc)
        {
            PK_TRACE("gpe_mem_power_control: STR Register 0 read fails"
                     "MC#|Port:0x%08x, Address:0x%08x, rc:0x%08x",
                     (uint32_t)((mc << 16) | port),
                     STR_REG0(mc,port), rc);

            gpe_set_ffdc(&(args->error), STR_REG0(mc,port),
                         GPE_RC_SCOM_GET_FAILED, rc);
            break;
        }

        switch(args->mem_pwr_ctl)
        {
            // MEM_PWR_CTL_OFF: clears the following memory control bits:
            //           -  master_enable (PCR0(2)),
            //           -  power_down_enable (PCR0(22)),
            //           -  STR_enable (STR(0)),
            //           -  disable_memory_clocks (STR(1)).
        case MEM_PWR_CTL_OFF:
            pcrRegValue = CLR_2BITS(pcrRegValue, PCR0_MASTER_ENABLE_BIT, PCR0_POWERDOWN_ENABLE_BIT);
            strRegValue = CLR_2BITS(strRegValue, STR0_STR_ENABLE_BIT, STR0_DISABLE_MEMORY_CLOCKS_BIT);
            break;

            // MEM_PWR_CTL_POWER_DOWN:
            //         sets:
            //           -  master_enable (PCR0(2)),
            //           -  power_down_enable (PCR0(22)),
            //         and clears:
            //           -  STR_enable (STR(0)),
            //           -  disable_memory_clocks (STR(1)).
        case MEM_PWR_CTL_POWER_DOWN:
            pcrRegValue = SET_2BITS(pcrRegValue, PCR0_MASTER_ENABLE_BIT, PCR0_POWERDOWN_ENABLE_BIT);

            strRegValue = CLR_2BITS(strRegValue, STR0_STR_ENABLE_BIT, STR0_DISABLE_MEMORY_CLOCKS_BIT);
            break;

            // MEM_PWR_CTL_PD_AND_STR:
            //         sets:
            //           -  master_enable (PCR0(2)),
            //           -  power_down_enable (PCR0(22)),
            //           -  STR_enable (STR(0)),
            //         and clears:
            //           -  disable_memory_clocks (STR(1)).
        case MEM_PWR_CTL_PD_AND_STR:
        // HW bug with disabling memory clock, treat PD_AND_STR_CLK_STOP same as PD_AND_STR
        // It is unknown if this will ever be fixed in a future RIT
        case MEM_PWR_CTL_PD_AND_STR_CLK_STOP:
            pcrRegValue = SET_2BITS(pcrRegValue, PCR0_MASTER_ENABLE_BIT, PCR0_POWERDOWN_ENABLE_BIT);
            strRegValue = SET_BIT(strRegValue, STR0_STR_ENABLE_BIT);

            strRegValue = CLR_BIT(strRegValue, STR0_DISABLE_MEMORY_CLOCKS_BIT);
            break;

            // MEM_PWR_CTL_PD_AND_STR_CLK_STOP: set the following memory power control bits
            //           -  master_enable (PCR0(2)),
            //           -  power_down_enable (PCR0(22)),
            //           -  STR_enable (STR(0)),
            //           -  disable_memory_clocks (STR(1)).
//        case MEM_PWR_CTL_PD_AND_STR_CLK_STOP:
//            pcrRegValue = SET_2BITS(pcrRegValue, PCR0_MASTER_ENABLE_BIT, PCR0_POWERDOWN_ENABLE_BIT);
//            strRegValue = SET_2BITS(strRegValue, STR0_STR_ENABLE_BIT, STR0_DISABLE_MEMORY_CLOCKS_BIT);
//            break;

        default:
            PK_TRACE("gpe_mem_power_control: Invalid memory power control command:0x%d",
                     args->mem_pwr_ctl);
            rc = GPE_RC_INVALID_MEM_PWR_CTL;

            gpe_set_ffdc(&(args->error), 0, rc, args->mem_pwr_ctl);
            break;
        }
        if(rc)
        {
            break;
        }

        // Write Modified Power Control Register 0 SCOM
        rc = putscom_abs(POWER_CTRL_REG0(mc,port), pcrRegValue);

        if(rc)
        {
            PK_TRACE("gpe_mem_power_control: Power Control Register 0 write fails"
                     "MC#|Port:0x%08x, Address:0x%08x, rc:0x%08x",
                     (uint32_t)((mc << 16) | port),
                     POWER_CTRL_REG0(mc,port), rc);

            gpe_set_ffdc(&(args->error), POWER_CTRL_REG0(mc,port),
                         GPE_RC_SCOM_PUT_FAILED, rc);
            break;
        }

        // Write Modified STR0 SCOM Register
        rc = putscom_abs(STR_REG0(mc,port), strRegValue);

        if(rc)
        {
            PK_TRACE("gpe_mem_power_control: STR Register 0 write fails"
                     "MC#|Port:0x%08x, Address:0x%08x, rc:0x%08x",
                     (uint32_t)((mc << 16) | port),
                     STR_REG0(mc,port), rc);

            gpe_set_ffdc(&(args->error), STR_REG0(mc,port),
                         GPE_RC_SCOM_PUT_FAILED, rc);
        }
    } while(0);


    if(rc)
    {
        GPE1_DIMM_DBG("gpe_mem_power_control: Failed to apply memory power control");
    }
    else
    {
        GPE1_DIMM_DBG("gpe_mem_power_control: Memory Power Control Register successfully written"
                      "mc|port#:0x%04x, PCR0:0x%08x, STR0:0x%08x",
                      (uint16_t)((mc << 8) | port),
                      pcrRegValue, strRegValue);
    }


    // send back a response, IPC success even if ffdc/rc are non zeros
    rc = ipc_send_rsp(cmd, IPC_RC_SUCCESS);
    if(rc)
    {
        PK_TRACE("gpe_mem_power_control: Failed to send response back. Halting GPE1", rc);
        gpe_set_ffdc(&(args->error), 0x00, GPE_RC_IPC_SEND_FAILED, rc);
        pk_halt();
    }

}
