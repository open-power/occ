/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe0/apss_init.c $                                    */
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
#include "pss_constants.h"
#include <apss_structs.h>       //H file common with occ_405
#include "gpe_util.h"

// Default to Auto-2 for now, should get set when the mode
// is initialized, and before any APSS data is gathered.
uint8_t G_apss_mode = APSS_MODE_AUTO2;

/*
 * Function Specification
 *
 * Name: apss_start_spi_command
 *
 * Description: Writes the P2S_COMMAND register to trigger the execution
 *              of a command loaded into the P2S_WDATA_REG
 *
 * End Function Specification
 */
uint32_t apss_start_spi_command(initGpioArgs_t * args, uint8_t i_noWait)
{
    uint32_t rc = 0;
    uint64_t regValue = 0x8000000000000000;

    // Start SPI transaction
    rc = putscom_abs(SPIPSS_P2S_COMMAND_REG, regValue);
    if (rc)
    {
        PK_TRACE("apss_start_spi_command: SPIPSS_P2S_COMMAND_REG putscom failed. rc = 0x%08x",
                 rc);
        gpe_set_ffdc(&(args->error), SPIPSS_P2S_COMMAND_REG, rc, 0x8000000000000000);
    }
    else
    {

        busy_wait(5);

        if (!i_noWait)
        {
            rc = wait_spi_completion(&(args->error), SPIPSS_P2S_STATUS_REG, 10);
            if (rc)
            {
                PK_TRACE("apss_start_spi_command: Timed out waiting for ops to complete. rc = 0x%08x",
                         rc);
                //FFDC set in wait_spi_completion
            }
        }
    }

    return rc;
}

/*
 * Function Specification
 *
 * Name: apss_init_gpio
 *
 * Description: Initialize the APSS GPIO ports
 *
 * End Function Specification
 */
void apss_init_gpio(ipc_msg_t* cmd, void* arg)
{
    //Note: arg was set to 0 in ipc func table (ipc_func_tables.c), so don't use it

    uint32_t            rc;
    uint32_t            ipc_send_rc;
    ipc_async_cmd_t     *async_cmd = (ipc_async_cmd_t*)cmd;
    initGpioArgs_t      *args = (initGpioArgs_t*)async_cmd->cmd_data;
    uint64_t            regValue = 0;

    PK_TRACE("apss_init_gpio: started.");

    do
    {
        // Wait for SPI operations to be complete (up to 10usec timeout)
        rc = wait_spi_completion(&(args->error), SPIPSS_P2S_STATUS_REG, 10);
        if (rc)
        {
            PK_TRACE("apss_init_gpio: Timed out waiting for ops to complete. rc = 0x%08x", rc);
            //FFDC set in wait_spi_completion
            break;
        }

        ////////////////////////////
        // Setup the control regs
        // frame_size=16, out_count=16, in_delay1=never, in_count2=16
        regValue = 0x410FC00004000000;
        rc = putscom_abs(SPIPSS_P2S_CTRL_REG0, regValue);
        if (rc)
        {
            PK_TRACE("apss_init_gpio: SPIPSS_P2S_CTRL_REG0 putscom failed. rc = 0x%08x", rc);
            gpe_set_ffdc(&(args->error), SPIPSS_P2S_CTRL_REG0, rc, regValue);
            break;
        }
        // bridge_enable, clock_divider=36, 2 frames
        regValue = 0x8090400000000000;
        rc = putscom_abs(SPIPSS_P2S_CTRL_REG1, regValue);
        if (rc)
        {
            PK_TRACE("apss_init_gpio: SPIPSS_P2S_CTRL_REG1 putscom failed. rc = 0x%08x", rc);
            gpe_set_ffdc(&(args->error), SPIPSS_P2S_CTRL_REG1, rc, regValue);
            break;
        }
        // inter_frame_delay=50 (5usec)
        regValue = 0x0019000000000000;
        rc = putscom_abs(SPIPSS_P2S_CTRL_REG2, regValue);
        if (rc)
        {
            PK_TRACE("apss_init_gpio: SPIPSS_P2S_CTRL_REG2 putscom failed. rc = 0x%08x", rc);
            gpe_set_ffdc(&(args->error), SPIPSS_P2S_CTRL_REG2, rc, regValue);
            break;
        }

        uint64_t port = 0;
        //Loop through the 2 ports setup
        for (port=0; port <= 1; port++)
        {
            //////////////////////
            // Direction (APSS cmd 0x4xxx); Configure GPIO mode (input or output)
            regValue = args->config0.direction;
            regValue = regValue << 48;
            regValue |= 0x4000000000000000;
            regValue |= (port << 56);

            rc = putscom_abs(SPIPSS_P2S_WDATA_REG, regValue);
            if (rc)
            {
                PK_TRACE("apss_init_gpio: SPIPSS_P2S_WDATA_REG putscom failed. value:0x%X. rc = 0x%08x",
                         regValue, rc);
                gpe_set_ffdc(&(args->error), SPIPSS_P2S_WDATA_REG, rc, regValue);
                break;
            }

            // Start SPI transaction
            rc = apss_start_spi_command(args,0);
            if (rc)
            {
                PK_TRACE("apss_init_gpio: SPI command start failed. rc = 0x%08x", rc);
                //FFDC already added
                break;
            }

            //---------------
            // Drive (APSS cmd 0x5xxx)
            regValue = args->config0.drive;
            regValue = regValue << 48;
            regValue |= 0x5000000000000000;
            regValue |= (port << 56);

            rc = putscom_abs(SPIPSS_P2S_WDATA_REG, regValue);
            if (rc)
            {
                PK_TRACE("apss_init_gpio: SPIPSS_P2S_WDATA_REG putscom failed. value:0x%X. rc = 0x%08x",
                         regValue, rc);
                gpe_set_ffdc(&(args->error), SPIPSS_P2S_WDATA_REG, rc, regValue);
                break;
            }

            // Start SPI transaction
            if (port == 0)
            {
                rc = apss_start_spi_command(args, 0);
            }else
            {
                //No need to wait since it's the last command.
                rc = apss_start_spi_command(args, 1);
            }

            if (rc)
            {
                PK_TRACE("apss_init_gpio: SPI command start failed. rc = 0x%08x", rc);
                //FFDC already set
                break;
            }
        }//End of port while loop.
    }while(0);

    // send back a successful response.  OCC will check rc and ffdc
    ipc_send_rc = ipc_send_rsp(cmd, IPC_RC_SUCCESS);

    if(ipc_send_rc)
    {
        PK_TRACE("apss_init_gpio: Failed to send response back. rc = 0x%08x. Halting GPE0",
                 ipc_send_rc);
        gpe_set_ffdc(&(args->error), 0x00, ipc_send_rc, regValue);
        pk_halt();
    }

    if(rc == 0) // if ipc_send_rc is 0, wont reach this instruction (pk_halt)
    {
        PK_TRACE("apss_init_gpio: completed successfully.");
    }

}

/*
 * Function Specification
 *
 * Name: apss_init_mode
 *
 * Description: Initialize the APSS mode
 *
 * End Function Specification
 */
void apss_init_mode(ipc_msg_t* cmd, void* arg)
{
    //Note: arg was set to 0 in ipc func table (ipc_func_tables.c), so don't use it

    uint32_t                rc = GPE_RC_SUCCESS;
    uint32_t                ipc_rc = IPC_RC_SUCCESS;
    uint32_t                ipc_send_rc;
    ipc_async_cmd_t         *async_cmd = (ipc_async_cmd_t*)cmd;
    setApssModeArgs_t       *args = (setApssModeArgs_t*)async_cmd->cmd_data;
    uint64_t                regValue = 0;

    PK_TRACE("apss_init_mode: started.");

    do
    {
        // Wait for SPI operations to be complete (up to 10usec timeout)
        rc = wait_spi_completion(&(args->error), SPIPSS_P2S_STATUS_REG, 10);
        if (rc)
        {
            PK_TRACE("apss_init_mode: Timed out waiting for ops to complete. rc = 0x%08x", rc);
            //FFDC set in wait_spi_completion
            break;
        }

        ////////////////////////////
        // Setup the control regs
        // frame_size=16, out_count1=16
        regValue = 0x4100000000000000;
        rc = putscom_abs(SPIPSS_P2S_CTRL_REG0, regValue);
        if (rc)
        {
            PK_TRACE("apss_init_mode: SPIPSS_P2S_CTRL_REG0 putscom failed. rc = 0x%08x", rc);
            gpe_set_ffdc(&(args->error), SPIPSS_P2S_CTRL_REG0, rc, regValue);
            break;
        }

        // bridge_enable, clock_divider=36, 1 frames
        regValue = 0x8090000000000000;
        rc = putscom_abs(SPIPSS_P2S_CTRL_REG1, regValue);
        if (rc)
        {
            PK_TRACE("apss_init_mode: SPIPSS_P2S_CTRL_REG1 putscom failed. rc = 0x%08x", rc);
            gpe_set_ffdc(&(args->error), SPIPSS_P2S_CTRL_REG1, rc, regValue);
            break;
        }
        // inter_frame_delay=50 (5usec)
        regValue = 0x0019000000000000;
        rc = putscom_abs(SPIPSS_P2S_CTRL_REG2, regValue);
        if (rc)
        {
            PK_TRACE("apss_init_mode: SPIPSS_P2S_CTRL_REG2 putscom failed. rc = 0x%08x", rc);
            gpe_set_ffdc(&(args->error), SPIPSS_P2S_CTRL_REG2, rc, regValue);
            break;
        }

        //Check for requested APSS mode.
        if (args->config.mode == APSS_MODE_COMPOSITE)
        {
            // APSS command to set composite data streaming mode (APSS cmd 0x8xxx, reserved bits are 1)
            // binary: 100011aaaa0000gg000000000000000000000000000000000000000000000000
            regValue = args->config.numAdcChannelsToRead - 1;  //aaaa => Address of last ADC channel (countOfADCChannels - 1)
            regValue = regValue << 6;                   //Make space for GPIO port count
            regValue |= (args->config.numGpioPortsToRead) & 0x03;   //gg => Num of GPIO ports
            regValue = (regValue << 48) | 0x8C00000000000000; //Add Command at D15-D12
            G_apss_mode = APSS_MODE_COMPOSITE;
        }
        else if (args->config.mode == APSS_MODE_AUTO2)
        {
            // Set Auto2 mode to scan all 16 ADC channels
            regValue = 0x3FC0000000000000;
            G_apss_mode = APSS_MODE_AUTO2;
        }
        else
        {
            //Invalid mode.
            PK_TRACE("apss_init_mode: Given invalid APSS Mode. Mode:0x%X", args->config.mode);
            rc = GPE_RC_INVALID_APSS_MODE;
            gpe_set_ffdc(&(args->error), 0x00, rc, args->config.mode);
            ipc_rc = IPC_RC_CMD_FAILED;
            break;
        }

        rc = putscom_abs(SPIPSS_P2S_WDATA_REG, regValue);
        if (rc)
        {
            PK_TRACE("apss_init_mode: SPIPSS_P2S_WDATA_REG putscom to set MODE failed. value:0x%X. rc = 0x%08x",
                     regValue, rc);
            gpe_set_ffdc(&(args->error), SPIPSS_P2S_WDATA_REG, rc, regValue);
            break;
        }

        regValue = 0x8000000000000000;
        // Start SPI transaction
        rc = putscom_abs(SPIPSS_P2S_COMMAND_REG, regValue);
        if (rc)
        {
            PK_TRACE("apss_init_mode: SPIPSS_P2S_COMMAND_REG putscom failed. rc = 0x%08x",
                     rc);
            gpe_set_ffdc(&(args->error), SPIPSS_P2S_COMMAND_REG, rc, 0x8000000000000000);
        }

    }while(0);

    // send back a response
    PK_TRACE("apss_init_mode: Sending APSS response ReturnCode:0x%X. APSSrc:0x%X (0 = Success)",
             ipc_rc, rc);
    ipc_send_rc = ipc_send_rsp(cmd, ipc_rc);

    //If we fail to send ipc response, then this error takes prescedence over any other error.
    //TODO: See if there's another space to write the error out to.
    if(ipc_send_rc)
    {
        PK_TRACE("apss_init_mode: Failed to send response back to mode initialization. Halting GPE0",
                 ipc_send_rc);
        gpe_set_ffdc(&(args->error), 0x00, ipc_send_rc, regValue);
        pk_halt();
    }

    if(rc == 0 && ipc_rc == IPC_RC_SUCCESS) // if ipc_send_rc, wont reach this instruction (pk_halt)
    {
        PK_TRACE("apss_init_mode: completed successfully.");
    }
}
