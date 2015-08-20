/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe0/apss_read.c $                                    */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015                             */
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
#include <apss_structs.h>

extern int wait_spi_completion(int reg, int timeout); //wait_spi.c

void apss_start_pwr_meas_read(ipc_msg_t* cmd, void* arg)
{
    // Note: arg was set to 0 in ipc func table (ipc_func_tables.c), so don't use it.
    // the ipc arguments passed through the ipc_msg_t structure, has a pointer
    // to the G_gpe_start_pwr_meas_read_args struct.

    int      rc;
    uint64_t value; // a pointer to hold the putscom_abs register value
    ipc_async_cmd_t *async_cmd = (ipc_async_cmd_t*)cmd;
    apss_start_args_t *args = (apss_start_args_t*)async_cmd->cmd_data;
 

    // clear error, ffdc, and rc (feedback to 405)
    // These may be overwritten by error codes if errors occur
    // REVIEW: Since the OCC clears these fields, do we really have to repeat this here?
    args->error.error = 0;
    args->error.rc = 0;
    args->error.ffdc = 0;


    // wait for ADC completion, or timeout after 5 micro seconds. 
    // scom register SPIPSS_ADC_STATUS_REG's bit 0 (HWCTRL_ONGOING) 
    // indicates when completion occurs.
    rc = wait_spi_completion(SPIPSS_ADC_STATUS_REG, 5);
    if(rc) // Timeout Reached, and SPI transaction didn't complete, copy over rc
    {
        PK_TRACE("gpe0:apss_start_pwr_meas_read:wait_spi_completion failed with rc = 0x%08x", rc);
        args->error.rc = rc;
        pk_halt();
    }

    // Setup control regs: 

    // SPIPSS_ADC_CTRL_REG0:
    // frame_size=16, out_count=16, in_count=16
    //    rc = putscom(0, SPIPSS_ADC_CTRL_REG0, uint64_t 0x4000100000000000);
    value = 0x4000100000000000;
    rc = putscom_abs(SPIPSS_ADC_CTRL_REG0, &value);

    if(rc)
    {
        PK_TRACE("apss_start_pwr_meas_read on gpe0 failed with rc = 0x%08x", rc);
        args->error.rc = rc;
        pk_halt();
    }
    
    // SPIPSS_ADC_CTRL_REG1: ADC FSM
    // clock_divider=7, frames=16
    // rc = putscom_abs(SPIPSS_ADC_CTRL_REG1, 0x8093c00000000000);
    value = 0x8093c00000000000;
    rc = putscom_abs(SPIPSS_ADC_CTRL_REG1, &value);

    if(rc)
    {
        PK_TRACE("apss_start_pwr_meas_read on gpe0 failed with rc = 0x%08x", rc);
        args->error.rc = rc;
        pk_halt();
    }

    // SPIPSS_ADC_CTRL_REG2: ADC interframe delay
    // 5 usec
    // rc = putscom_abs(SPIPSS_ADC_CTRL_REG2, 0x0019000000000000);
    value = 0x0019000000000000;
    rc = putscom_abs(SPIPSS_ADC_CTRL_REG2, &value);

    if(rc)
    {
        PK_TRACE("apss_start_pwr_meas_read on gpe0 failed with rc = 0x%08x", rc);
        args->error.rc = rc;
        pk_halt();
    }

    // SPIPSS_ADC_WDATA_REG: 
    // APSS command to continue previous command 
    // rc = putscom_abs(SPIPSS_ADC_WDATA_REG, 0x0000000000000000);
    value = 0x0000000000000000;
    rc = putscom_abs(SPIPSS_ADC_WDATA_REG, &value);

    if(rc)
    {
        PK_TRACE("apss_start_pwr_meas_read on gpe0 failed with rc = 0x%08x", rc);
        args->error.rc = rc;
        pk_halt();
    }

    // SPIPSS_ADC_COMMAND_REG:
    // Start SPI Transaction
    // rc = putscom_abs(SPIPSS_ADC_COMMAND_REG, 0x8000000000000000);
    value = 0x8000000000000000;
    rc = putscom_abs(SPIPSS_ADC_COMMAND_REG, &value);

    if(rc)
    {
        PK_TRACE("apss_start_pwr_meas_read on gpe0 failed with rc = 0x%08x", rc);
        args->error.rc = rc;
        pk_halt();
    }


    // send back a response, IPC success even if ffdc/rc are non zeros
    rc = ipc_send_rsp(cmd, IPC_RC_SUCCESS);

    // return value is void
    return;
    //pk_halt;
}

void apss_continue_pwr_meas_read(ipc_msg_t* cmd, void* arg)
{
    // the ipc arguments are passed through the ipc_msg_t structure, has a pointer
    // to the G_gpe_continue_pwr_meas_read_args

    int          rc;
    ipc_async_cmd_t *async_cmd = (ipc_async_cmd_t*)cmd;
    apss_continue_args_t *args = (apss_continue_args_t*)async_cmd->cmd_data;
 

    // Clear error, ffdc, and rc (feedback to 405)
    // These may be overwritten by error codes if errors occur
    // REVIEW: Since the OCC clears these fields, do we really have to repeat this here?
    args->error.error = 0;
    args->error.rc = 0;
    args->error.ffdc = 0;


    // wait for ADC completion, or timeout after 100 micro seconds. 
    // scom register SPIPSS_ADC_STATUS_REG's bit 0 (HWCTRL_ONGOING) 
    // indicates when completion occurs.
    rc = wait_spi_completion(SPIPSS_ADC_STATUS_REG, 100);
    if(rc) // Timeout Reached, and SPI transaction didn't complete, copy returned status into rc
           // REVIEW: Should we also copy something into the ffdc as well?
    {
        PK_TRACE("gpe0:apss_continue_pwr_meas_read:wait_spi_completion failed with rc = 0x%08x", rc);
        args->error.rc = rc;
        pk_halt();
    }

    // REVIEW: ADC readings are done for 32 bytes = 4 * 64 bit reads 
    // they are saved in the common OCC-GPE0 area: verify using SIMICS
    // Check every scom read, store rc in the error.rc if it fails.

    rc = getscom_abs(SPIPSS_ADC_RDATA_REG0, (uint64_t*) args->meas_data);
    if(rc)
    {
        PK_TRACE("gpe0:apss_continue_pwr_meas_read:getscom_reg0 failed with rc = 0x%08x", rc);
        args->error.rc = rc;
        pk_halt();
    }
    
    rc = getscom_abs(SPIPSS_ADC_RDATA_REG1, (uint64_t*) &args->meas_data[1]);
    if(rc)
    {
        PK_TRACE("gpe0:apss_continue_pwr_meas_read:getscom_reg1 failed with rc = 0x%08x", rc);
        args->error.rc = rc;
        pk_halt();
    }
    
    rc = getscom_abs(SPIPSS_ADC_RDATA_REG2, (uint64_t*) &args->meas_data[2]);
    if(rc)
    {
        PK_TRACE("gpe0:apss_continue_pwr_meas_read:getscom_reg2 failed with rc = 0x%08x", rc);
        args->error.rc = rc;
        pk_halt();
    }
    
    rc = getscom_abs(SPIPSS_ADC_RDATA_REG3, (uint64_t*) &args->meas_data[3]);
    if(rc)
    {
        PK_TRACE("gpe0:apss_continue_pwr_meas_read:getscom_reg3 failed with rc = 0x%08x", rc);
        args->error.rc = rc;
        pk_halt();
    }
        
    // send back a response, IPC success even if ffdc/rc are non zeros
    rc = ipc_send_rsp(cmd, IPC_RC_SUCCESS);

    return;
    // halt;   // is it supposed to be treated as nop in the PPE?

}

void apss_complete_pwr_meas_read(ipc_msg_t* cmd, void* arg)
{
    // the ipc arguments are passed through the ipc_msg_t structure, has a pointer
    // to the G_gpe_complete_pwr_meas_read_args

    int          rc;
    ipc_async_cmd_t *async_cmd = (ipc_async_cmd_t*)cmd;
    apss_complete_args_t *args = (apss_complete_args_t*)async_cmd->cmd_data;
    

    // clear error, ffdc, and rc (feedback to 405)
    // These may be overwritten by error codes if errors occur
    // REVIEW: Since the OCC clears these fields, do we really have to repeat this here?
    args->error.error = 0;
    args->error.rc = 0;
    args->error.ffdc = 0;

    // wait for ADC completion, or timeout after 100 micro seconds. 
    // scom register SPIPSS_ADC_STATUS_REG's bit 0 (HWCTRL_ONGOING) 
    // indicates when completion occurs.
    rc = wait_spi_completion(SPIPSS_ADC_STATUS_REG, 100);
    if(rc) // Timeout Reached, and SPI transaction didn't complete, copy returned status into rc
           // REVIEW: Should we also copy something into the ffdc as well? whether in wait_spi_completion or here?
    {
        PK_TRACE("gpe0:apss_complete_pwr_meas_read:wait_spi_completion failed with rc = 0x%08x", rc);
        args->error.rc = rc;
        pk_halt();
    }
    

    rc = getscom_abs(TOD_VALUE_REG, &args->meas_data[3]);    // REVIEW: Check proper transfer to the OCC under simics
    if(rc)
    {
        PK_TRACE("apss_complete_pwr_meas_read on gpe0 failed with rc = 0x%08x", rc);
        args->error.rc = rc;
        pk_halt();
    }
    
    // send back a response, IPC success even if ffdc/rc are non zeros
    rc = ipc_send_rsp(cmd, IPC_RC_SUCCESS);

    return;
}
