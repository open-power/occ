/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/firdata/ast_mboxdd.c $                            */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2017                             */
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
/**
 *  @file ast_mboxdd.C
 *
 *  @brief Implementation of the PNOR Accesser using the AST MBOX protocol
 */

/*****************************************************************************/
// I n c l u d e s
/*****************************************************************************/
#include <native.h>
#include <norflash.h>
#include <ast_mboxdd.h>
#include <lpc.h>

errorHndl_t writeRegSIO(uint8_t i_regAddr, uint8_t i_data)
{
    errorHndl_t l_err = NO_ERROR;

    do {
        size_t reg_size = sizeof(uint8_t);

        /* Write out the register address */
        l_err = lpc_write( LPC_TRANS_IO, SIO_ADDR_REG_2E,
                           &i_regAddr,
                           reg_size );
        if( l_err ) { break; }

        /* Write out the register data */
        l_err = lpc_write( LPC_TRANS_IO, SIO_DATA_REG_2F,
                           &i_data,
                           reg_size );
        if( l_err ) { break; }

    } while(0);

    return l_err;
}

errorHndl_t readRegSIO(uint8_t i_regAddr, uint8_t* o_data)
{
    errorHndl_t l_err = NO_ERROR;

    do {
        size_t reg_size = sizeof(uint8_t);

        /* Write out the register address */
         l_err = lpc_write( LPC_TRANS_IO, SIO_ADDR_REG_2E,
                           &i_regAddr,
                           reg_size );
        if( l_err ) { break; }

        /* Read in the register data */
        l_err = lpc_read( LPC_TRANS_IO, SIO_DATA_REG_2F,
                          o_data,
                          reg_size );
        if( l_err ) { break; }

    } while(0);

    return l_err;
}

errorHndl_t mboxOut(uint64_t i_addr, uint8_t i_byte)
{
    size_t len = sizeof(i_byte);

    return lpc_write( LPC_TRANS_IO,
                      i_addr + MBOX_IO_BASE,
                      &i_byte,
                      len );
}

errorHndl_t mboxIn(uint64_t i_addr, uint8_t *o_byte)
{
    size_t len = sizeof(uint8_t);

    return lpc_read( LPC_TRANS_IO,
                     i_addr + MBOX_IO_BASE,
                     o_byte,
                     len );
}

errorHndl_t doMessage( astMbox_t *io_mbox, mboxMessage_t *io_msg )
{
    uint8_t* l_data = (uint8_t*)io_msg;
    errorHndl_t l_err = NO_ERROR;
    uint8_t l_stat1;
    uint32_t l_loops = 0;
    bool l_prot_error = false;
    int i;

    io_msg->iv_seq = io_mbox->iv_mboxMsgSeq++;

    do
    {
        /* Write message out */
        for (i = 0; i < BMC_MBOX_DATA_REGS && !l_err; i++)
        {
            l_err = mboxOut(i, l_data[i]);
        }

        if ( l_err )
        {
            break;
        }

        /* Clear status1 response bit as it was just set via reg write*/
        l_err = mboxOut(MBOX_STATUS_1, MBOX_STATUS1_RESP);

        if ( l_err )
        {
            break;
        }

        /* Ping BMC */
        l_err = mboxOut(MBOX_HOST_CTRL, MBOX_CTRL_INT_SEND);

        if ( l_err )
        {
            break;
        }

        /* Wait for response */
        while ( l_loops++ < MBOX_MAX_RESP_WAIT_US && !l_err )
        {
            l_err = mboxIn(MBOX_STATUS_1, &l_stat1);

            if ( l_err )
            {
                TRAC_ERR("doMessage error from MBOX_STATUS_1");
                break;
            }

            if ( l_stat1 & MBOX_STATUS1_RESP )
            {
                break;
            }
            sleep(1000);
        }

        if ( l_err )
        {
            TRAC_ERR( "Got error waiting for response !");
            break;
        }

        if ( !(l_stat1 & MBOX_STATUS1_RESP) )
        {
            TRAC_ERR( "Timeout waiting for response !");

            // Don't try to interrupt the BMC anymore
            l_err = mboxOut(MBOX_HOST_CTRL, 0);
            if ( l_err)
            {
                //Note the command failed
                TRAC_ERR( "Error communicating with MBOX daemon");
                TRAC_ERR( "Mbox status 1 reg: %x", l_stat1);
            }

            // Tell the code below that we generated the error
            //  (not an LPC error)
            l_prot_error = true;
            break;
        }

        /* Clear status */
        l_err = mboxOut(MBOX_STATUS_1, MBOX_STATUS1_RESP);
        if (l_err)
        {
            TRAC_ERR( "Got error clearing status");
            break;
        }

        // Remember some message fields before they get overwritten
        // by the response
        uint8_t old_seq = io_msg->iv_seq;

        // Read response
        for (i = 0; i < BMC_MBOX_DATA_REGS && !l_err; i++)
        {
            l_err = mboxIn(i, &l_data[i]);
        }

        if ( l_err )
        {
            TRAC_ERR( "Got error reading response !");
            break;
        }

        if (old_seq != io_msg->iv_seq)
        {
            TRAC_ERR( "bad sequence number in mbox message, got %d want %d",
                       io_msg->iv_seq, old_seq);
            l_err = -1;
            break;
        }

        if (io_msg->iv_resp != MBOX_R_SUCCESS)
        {
            TRAC_ERR( "BMC mbox command failed with err %d",
                       io_msg->iv_resp);
            l_err = -1;
            // Tell code below that we generated the error (not an LPC error)
            l_prot_error = true;
            break;
        }

    }
    while(0);

    // If we got an LPC error, commit it and generate our own
    if ( l_err && !l_prot_error )
    {
        l_err = -1;
    }

    return l_err;
}

errorHndl_t initializeMbox(void)
{
    errorHndl_t l_errl = NO_ERROR;

    do
    {
        size_t reg_size = sizeof(uint8_t);

        //First need to unlock SIO registers
        /* Send SuperIO password - send A5 twice to offset 0x2E */
        uint8_t data = SIO_PASSWORD_REG;
        l_errl = lpc_write( LPC_TRANS_IO, SIO_ADDR_REG_2E,
                           &data, reg_size );
        if( l_errl ) { break; }

        l_errl = lpc_write( LPC_TRANS_IO, SIO_ADDR_REG_2E,
                           &data, reg_size );
        if( l_errl ) { break; }

        //Second need to select Mailbox SIO Device
        //  Register 0x07 is device select reg
        //  Device 0x0E is the sio mailbox device
        l_errl = writeRegSIO( SIO_DEVICE_SELECT_REG, SIO_MB );
        if( l_errl ) { break; }

        //First disable SIO Mailbox engine to configure it
        // 0x30 - Enable/Disable Reg
        // 0x00 - Disable Device (previously selected mailbox device)
        l_errl = writeRegSIO( 0x30, 0x00 );

        if (l_errl)
        {
            break;
        }

        // Set MBOX Base Address
        //Regs 0x60/0x61 are a BAR-like reg to configure the MBOX base address
        l_errl = writeRegSIO( 0x60, (MBOX_IO_BASE >> 8) & 0xFF );

        if (l_errl)
        {
            break;
        }

        // Set other half of MBOX Base Address
        l_errl = writeRegSIO( 0x61, MBOX_IO_BASE & 0xFF );

        if (l_errl)
        {
            break;
        }

        //Configure MBOX IRQs
        //Regs 0x70 / 0x71 control that
        l_errl = writeRegSIO( 0x70, MBOX_LPC_IRQ );

        if (l_errl)
        {
            break;
        }

        //Other half of MBOX IRQ Configuration
        // 1 == Low level trigger
        l_errl = writeRegSIO( 0x71, 1 );

        if (l_errl)
        {
            break;
        }

        //Re-enable Device now that base addr and IRQs are configured
        // 1 == Enable Device
        l_errl = writeRegSIO( 0x30, 0x01 );

        if (l_errl)
        {
            break;
        }
    }
    while(0);

    return l_errl;
}
