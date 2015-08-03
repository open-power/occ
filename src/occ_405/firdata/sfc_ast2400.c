/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/firdata/sfc_ast2400.C $                               */
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

/*****************************************************************************/
/* I n c l u d e s */
/*****************************************************************************/
#include <native.h>
#include <norflash.h>
/*#include <string.h> */
#include <sfc_ast2400.h>
#include <lpc.h>
extern int TRACE_LPC;
int TRACE_SFC = 0;

/*****************************************************************************/
/* C o n s t a n t s */
/*****************************************************************************/




/*****************************************************************************/
/* G l o b a l s */
/*****************************************************************************/


/*****************************************************************************/
/* M e t h o d s */
/*****************************************************************************/

/**
 * @brief Read data from the flash
 */
errorHndl_t readFlash( Sfc_t* i_sfc,
                      uint32_t i_addr,
                                  size_t i_size,
                                  void* o_data )
{
    /*TRACDCOMP( ENTER_MRK"readFlash> i_addr=0x%.8x, i_size=0x%.8x",  i_addr, i_size ); */
    errorHndl_t l_err = 0;

    do{
        uint32_t* word_ptr = (uint32_t*)(o_data);
        uint32_t word_size = (ALIGN_4(i_size))/4;
        uint32_t words_read = 0;
        for( words_read = 0;
             words_read < word_size;
             words_read ++ )
        {
            /*Read directly from MMIO space */
            uint32_t lpc_addr = i_sfc->iv_mmioOffset | (i_addr + words_read*4);
            size_t reg_size = sizeof(uint32_t);
            l_err = lpc_read( LPC_TRANS_FW,
                              lpc_addr,
                              (uint8_t*)&(word_ptr[words_read]),
                              reg_size );
            if( l_err ) {  break; }
        }
        if( l_err ) {  break; }
    }while(0);

    /*TRACDCOMP( EXIT_MRK"readFlash> err=%.8X", ERRL_GETEID_SAFE(l_err) ); */
    return l_err;
}


/**
 * @brief Write data into flash
 */
errorHndl_t writeFlash( Sfc_t* i_sfc,
                        uint32_t i_addr,
                        size_t i_size,
                        void* i_data )
{
    TRACFCOMP( "writeFlash> i_addr=0x%.8x, i_size=0x%.8x", i_addr, i_size );
    errorHndl_t l_err = 0;
    size_t l_bytes_left = i_size;
    size_t l_bytes_to_write = 0;
    uint32_t l_addr_to_write = i_addr;
    uint8_t* l_dataptr = (uint8_t*)(i_data);

    do {
        /* Enable write mode */
        l_err = enableWriteMode(i_sfc);
        if( l_err ) { break; }

        /* Page Program (PP) command only supports 256 bytes at a time */
        if( l_bytes_left <= PAGE_PROGRAM_BYTES )
        {
            l_bytes_to_write = l_bytes_left;
            l_bytes_left = 0;
        }
        else
        {
            l_bytes_to_write = PAGE_PROGRAM_BYTES;
            l_bytes_left -= PAGE_PROGRAM_BYTES;
        }

        /* Send in the Page Program command with the data to write */
        uint8_t opcode = SPI_JEDEC_PAGE_PROGRAM;
        l_err = sendSpiCmd( i_sfc, opcode, l_addr_to_write,
                            l_bytes_to_write,
                            l_dataptr,
                            0, NULL );
        if( l_err ) { break; }

        /* Move to the next chunk */
        l_addr_to_write += l_bytes_to_write;
        l_dataptr += l_bytes_to_write;

        /* Wait for idle */
        l_err = pollOpComplete(i_sfc);
        if( l_err ) { break; }

        /*check for special Micron Flag Status reg */
        if(i_sfc->iv_flashWorkarounds & HWWK_MICRON_WRT_ERASE)
        {
            l_err = micronFlagStatus(i_sfc);
            if(l_err) { break; }
        }

    } while(l_bytes_left);

    /*TRACDCOMP( EXIT_MRK"writeFlash> err=%.8X", ERRL_GETEID_SAFE(l_err) ); */
    return l_err;
}

/**
 * @brief Erase a block of flash
 */
errorHndl_t eraseFlash( Sfc_t* i_sfc,
                        uint32_t i_addr )
{
    TRACFCOMP(">>eraseFlash> Block 0x%.8X", i_addr );
    errorHndl_t l_err = 0;

    do {
        // Enable write mode
        l_err = enableWriteMode(i_sfc);
        if( l_err ) { break; }

        // Send erase command
        uint8_t opcode = SPI_JEDEC_SECTOR_ERASE;
        l_err = sendSpiCmd( i_sfc, opcode, i_addr, 0, 0, 0, NULL );
        if( l_err ) { break; }

        // Wait for idle
        l_err = pollOpComplete(i_sfc);
        if( l_err ) { break; }

        //check for special Micron Flag Status reg
        if(i_sfc->iv_flashWorkarounds & HWWK_MICRON_WRT_ERASE)
        {
            l_err = micronFlagStatus(i_sfc);
            if(l_err) { break; }
        }
    } while(0);

    return l_err;
}


/**
 * @brief Initialize and configure the SFC hardware
 */
errorHndl_t hwInit( Sfc_t* i_sfc )
{
    TRACFCOMP( ENTER_MRK"hwInit>" );
    errorHndl_t l_err = NO_ERROR;

    do {
        size_t reg_size = sizeof(uint8_t);

        /*** Initialize the LPC2AHB logic */

        /* Send SuperIO password - send A5 twice */
        uint8_t data = 0xA5;
        l_err = lpc_write( LPC_TRANS_IO, SIO_ADDR_2E,
                           &data, reg_size );
        if( l_err ) { break; }

        l_err = lpc_write( LPC_TRANS_IO, SIO_ADDR_2E,
                           &data, reg_size );
        if( l_err ) { break; }


        /* Select logical device D (iLPC2AHB) */
        l_err = writeRegSIO( i_sfc, 0x07, 0x0D );
        if( l_err ) { break; }


        /* Enable iLPC->AHB */
        l_err = writeRegSIO( i_sfc, 0x30, 0x01 );
        if( l_err ) { break; }


        /*** Setup the SPI Controller */

        /* Enable writing to the controller */
        SpiControlReg04_t ctlreg;
        l_err = readRegSPIC( i_sfc, CTLREG_04, &ctlreg.data32 );
        if( l_err ) { break; }
        TRACFCOMP("dc99> First read of CTLREG_04=%.8X", ctlreg.data32);
        ctlreg.cmdMode = 2;/*0b10; //10:Normal Write (CMD + Address + Write data) */
        l_err = writeRegSPIC( i_sfc, CTLREG_04, ctlreg.data32 );
        if( l_err ) { break; }

        SpiConfigReg00_t confreg;
        l_err = readRegSPIC( i_sfc, CONFREG_00, &confreg.data32 );
        if( l_err ) { break; }
        confreg.inactiveX2mode = 1; /*Enable CE# Inactive pulse width X2 mode */
        confreg.enableWrite = 1; /*Enable flash memory write */
        l_err = writeRegSPIC( i_sfc, CONFREG_00, confreg.data32 );
        if( l_err ) { break; }


        /*
         * Setup control reg and for our use, switching
         * to 1-bit mode, clearing user mode if set, etc...
         *
         * Also configure SPI clock to something safe
         * like HCLK/8 (24Mhz)
         */
        ctlreg.fourByteMode = 1;
        ctlreg.ioMode = 0;/*0b00; //single bit or controlled by bit[3] */
        ctlreg.pulseWidth = 0x0; /*0000: 16T (1T = 1 HCLK clock) */
        ctlreg.cmdData = 0x00;
        ctlreg.spiClkFreq = 0x4; /*HCLK/8 */
        ctlreg.dummyCycleRead1 = 0; /*no dummy cycles */
        ctlreg.dummyCycleRead2 = 0;/*0b00; //no dummy cycles */
        ctlreg.cmdMode = 0;/*0b00; //00:Normal Read (03h + Address + Read data) */
        i_sfc->iv_ctlRegDefault = ctlreg;  /* Default setup is regular read mode */

        /* Configure for read */
        l_err = writeRegSPIC( i_sfc, CTLREG_04, ctlreg.data32 );
        if( l_err ) { break; }

        /* Setup flash-specific settings here, if there are any */

    } while(0);

    TRACFCOMP( "hwInit> err=%.8X", l_err );
    return l_err;
}

/**
 * @brief Send a SPI command
 */
errorHndl_t sendSpiCmd( Sfc_t* i_sfc,
                        uint8_t i_opCode,
                        uint32_t i_address,
                        size_t i_writeCnt,
                        const uint8_t* i_writeData,
                        size_t i_readCnt,
                        uint8_t* o_readData )
{
    errorHndl_t l_err = NO_ERROR;
    size_t opsize = 0;
    if(TRACE_SFC){
    TRACFCOMP( ENTER_MRK"sendSpiCmd> i_opCode=%.2X, i_address=%.8X, i_writeCnt=0x%X, i_writeData=%p", i_opCode, i_address, i_writeCnt, i_writeData );
    TRACFCOMP( "  , i_readCnt=0x%X, o_readData=%p", i_readCnt, o_readData );
    }

    do {
        /*Do a read of flash address zero to workaround */
        /* a micron bug with extended reads */
        if( (HWWK_MICRON_EXT_READ & i_sfc->iv_flashWorkarounds)
            && (i_readCnt > 4) )
        {
            uint32_t ignored = 0;
            l_err = readFlash( i_sfc, 0, 1, &ignored );
            if(l_err) { break; }
        }

        /* Put controller into command mode (instead of read mode) */
        l_err = commandMode( i_sfc, true );
        if( l_err ) { break; }

        /* Write command to the beginning of the flash space */
        opsize = sizeof(i_opCode);
        l_err = lpc_write( LPC_TRANS_FW, i_sfc->iv_mmioOffset,
                           &i_opCode,
                           opsize ); /*just send opcode */
        if( l_err ) { break; }

        /* Send address if there is one */
        if( i_address != NO_ADDRESS )
        {
            /* Write address to the beginning of the flash space */
            opsize = sizeof(i_address);
            l_err = lpc_write( LPC_TRANS_FW, i_sfc->iv_mmioOffset,
                               (uint8_t*)&i_address,
                               opsize ); /*only supporting 4-byte addresses */
            if( l_err ) { break; }
        }

        /* Send in the rest of the write data */
        if( i_writeCnt && i_writeData )
        {
            size_t bytes_left = i_writeCnt;
            uint8_t* curptr = (uint8_t*)(i_writeData);
            while( bytes_left )
            {
                /* Write the last partial word if there is one */
                if( bytes_left < 4 )
                {
                    opsize = bytes_left;
                    l_err = lpc_write( LPC_TRANS_FW, i_sfc->iv_mmioOffset,
                                       curptr,
                                       opsize );
                    break;
                }

                /* Write data into the beginning of the flash space, */
                /*  in command mode this doesn't write the flash */
                /*  but instead is a pass-through to the area we */
                /*  really want to write */
                opsize = sizeof(uint32_t);
                l_err = lpc_write( LPC_TRANS_FW, i_sfc->iv_mmioOffset,
                                   curptr,
                                   opsize );
                if( l_err ) { break; }

                curptr += 4;
                bytes_left -= 4;
            }
            if( l_err ) { break; }
        }

        /* Read back the results */
        if( i_readCnt && o_readData )
        {
            size_t bytes_left = i_readCnt;
            uint8_t* curptr = o_readData;
            while( bytes_left )
            {
                /* Grab the last partial word if there is one */
                if( bytes_left < 4 )
                {
                    opsize = bytes_left;
                    l_err = lpc_read( LPC_TRANS_FW, i_sfc->iv_mmioOffset,
                                      curptr,
                                      opsize );
                    break;
                }

                /* Read data from the beginning of the flash space, */
                /*  in command mode this doesn't read the flash */
                /*  but instead is a pass-through to the data we */
                /*  really want */
                opsize = sizeof(uint32_t);
                l_err = lpc_read( LPC_TRANS_FW, i_sfc->iv_mmioOffset,
                                  curptr,
                                  opsize );
                if( l_err ) { break; }

                curptr += 4;
                bytes_left -= 4;
            }
            if( l_err ) { break; }
        }
    } while(0);


    /* No matter what, put the logic back into read mode */
    int tmp_err = commandMode( i_sfc, false );
    if( tmp_err )
    {
        if( !l_err )
        {
            l_err = tmp_err;
        }
    }

    /*TRACDCOMP( EXIT_MRK"sendSpiCmd> o_readData=%.2X, err=%.8X", o_readData == NULL ? 0 : o_readData[0], ERRL_GETEID_SAFE(l_err) ); */
    return l_err;
}

/**
 * @brief Enter/exit command mode
 */
errorHndl_t commandMode( Sfc_t* i_sfc,
                        bool i_enter )
{
    errorHndl_t l_err = NO_ERROR;
    /*TRACDCOMP( ENTER_MRK"commandMode(%d)", i_enter ); */

    /*
     * There is only a limited addressable window within LPC space.  The AST
     * has its control register space at too far of a distance from the read
     * space for them both to fit in a single window.  Rather than moving the
     * window around we will use the iLPC2AHB backdoor inside the SuperIO
     * controller to do both register accesses and to write into the flash.
     *
     * High level flow to write into control space:
     *    Stop active control (SPI04 Control Reg)
     *    Enable command mode (SPI04 Control Reg)
     *    Write actual command into flash base addr (0x0E000000)
     */

    do {
        SpiControlReg04_t ctlreg = i_sfc->iv_ctlRegDefault;

        /* Switch to user mode, CE# dropped */
        ctlreg.stopActiveCtl = 1;
        ctlreg.cmdMode = 3;/*0b11; //User Mode (Read/Write Data) */
        l_err = writeRegSPIC( i_sfc, CTLREG_04, ctlreg.data32 );
        if( l_err ) { break; }

        if( i_enter ) /*ast_sf_start_cmd */
        {
            /* user mode, CE# active */
            ctlreg.stopActiveCtl = 0;
            l_err = writeRegSPIC( i_sfc, CTLREG_04, ctlreg.data32 );
            if( l_err ) { break; }
        }
        else /*ast_sf_end_cmd */
        {
            /* Switch back to read mode */
            l_err = writeRegSPIC( i_sfc, CTLREG_04, i_sfc->iv_ctlRegDefault.data32 );
            if( l_err ) { break; }
        }
    } while(0);

    /*TRACDCOMP( EXIT_MRK"commandMode> err=%.8X", ERRL_GETEID_SAFE(l_err) ); */
    return l_err;
}

/**
 * @brief Enable write mode
 */
errorHndl_t enableWriteMode( Sfc_t* i_sfc )
{
    errorHndl_t l_err = NO_ERROR;
    /*TRACDCOMP( ENTER_MRK"enableWriteMode>" ); */

    /* Some flashes need it to be hammered */
    NorStatusReg_t status;
//    status.data8 = 0x55;
    size_t i = 0;
    for( i = 0; i < 10; i++ )
    {
        /* Send the command to enable writes */
        uint8_t opcode = SPI_JEDEC_WRITE_ENABLE;
        l_err = sendSpiCmd( i_sfc, opcode, NO_ADDRESS, 0, NULL, 0, NULL );
        if( l_err ) { break; }

        /* Check to see if it worked */
        opcode = SPI_JEDEC_READ_STATUS;
        l_err = sendSpiCmd( i_sfc, opcode, NO_ADDRESS, 0, NULL, 1, &(status.data8) );
        if( l_err ) { break; }
        TRACDCOMP( "SPI_JEDEC_READ_STATUS=%.2X", status.data8 );

        if( status.writeEnable )
        {
            break;
        }
    }

    if( !l_err && !status.writeEnable )
    {
        TRACFCOMP( "Could not enable writes" );
        l_err = -1;
    }

    /*TRACDCOMP( EXIT_MRK"enableWriteMode> err=%.8X", ERRL_GETEID_SAFE(l_err) ); */
    return l_err;
}

/**
 * @brief Write a single byte into the SIO
 */
errorHndl_t writeRegSIO( Sfc_t* i_sfc,
                        uint8_t i_regAddr,
                                    uint8_t i_data )
{ /*lpc_sio_outb */
    errorHndl_t l_err = NO_ERROR;
    /*TRACFCOMP( "writeRegSIO> i_regAddr=0x%.2X, i_data=0x%.2X", i_regAddr, i_data );*/

    do {
        size_t reg_size = sizeof(uint8_t);

        /* AST2400 integrates a Super I/O module with */
        /*  LPC protocol (I/O cycle 0x2E/0x2F) */

        /* Write out the register address */
        l_err = lpc_write( LPC_TRANS_IO, SIO_ADDR_2E,
                           &i_regAddr,
                           reg_size );
        if( l_err ) { break; }

        /* Write out the register data */
        l_err = lpc_write( LPC_TRANS_IO, SIO_DATA_2F,
                           &i_data,
                           reg_size );
        if( l_err ) { break; }

    } while(0);

    /*TRACDCOMP( EXIT_MRK"writeRegSIO> err=%.8X", ERRL_GETEID_SAFE(l_err) ); */
    return l_err;
}

/**
 * @brief Read a single byte from the SIO
 */
errorHndl_t readRegSIO( Sfc_t* i_sfc,
                       uint8_t i_regAddr,
                                   uint8_t* o_data )
{ 
    errorHndl_t l_err = NO_ERROR;
    /*TRACDCOMP( ENTER_MRK"readRegSIO> i_regAddr=0x%.2X", i_regAddr ); */

    do {
        size_t reg_size = sizeof(uint8_t);

        /* AST2400 integrates a Super I/O module with */
        /*  LPC protocol (I/O cycle 0x2E/0x2F) */

        /* Write out the register address */
        l_err = lpc_write( LPC_TRANS_IO, SIO_ADDR_2E,
                           &i_regAddr,
                           reg_size );
        if( l_err ) { break; }

        /* Read in the register data */
        l_err = lpc_read( LPC_TRANS_IO, SIO_DATA_2F,
                          o_data,
                          reg_size );
        if( l_err ) { break; }

    } while(0);

    /*TRACDCOMP( EXIT_MRK"readRegSIO> o_data=0x%.2X, err-%.8X", *o_data, ERRL_GETEID_SAFE(l_err) ); */
    return l_err;
}

/**
 * @brief Prepare the iLPC2AHB address regs
 */
errorHndl_t setupAddrLPC2AHB( Sfc_t* i_sfc,
                             uint32_t i_addr )
{ 
    errorHndl_t l_err = NO_ERROR;
    /*TRACDCOMP( ENTER_MRK"setupAddrLPC2AHB> i_addr=0x%X", i_addr ); */

    do {
        /* Select logical device D (iLPC2AHB) */
        l_err = writeRegSIO( i_sfc, 0x07, 0x0D );
        if( l_err ) { break; }

        /* Push 4 address bytes into SIO regs 0xF0-0xF3 */
        size_t i;
        for( i=sizeof(i_addr); i>0; i-- )
        {
            l_err = writeRegSIO( i_sfc, 0xF3-(i-1), /*F0,F1,F2,F3 */
                                 (uint8_t)(i_addr >> ((i-1)*8)) );
            if( l_err ) { break; }
        }
        if( l_err ) { break; }

        /* Configure 4 byte length */
        l_err = writeRegSIO( i_sfc, 0xF8, 0x02 );
        if( l_err ) { break; }

    } while(0);

    /*TRACDCOMP( EXIT_MRK"setupAddrLPC2AHB> err=%.8X", ERRL_GETEID_SAFE(l_err) ); */
    return l_err;
}

/**
 * @brief Write SPI Controller Register
 */
errorHndl_t writeRegSPIC( Sfc_t* i_sfc,
                         SpicReg_t i_reg,
                                     uint32_t i_data )
{
    errorHndl_t l_err = NO_ERROR;
    //TRACFCOMP( "writeRegSPIC> i_reg=0x%.2X, i_data=0x%.8X", i_reg, i_data );

    do {
        /* Compute the full LPC address */
        uint32_t lpc_addr = i_reg | SPIC_BASE_ADDR_AHB;

        /* Setup the logic for the write */
        l_err = setupAddrLPC2AHB( i_sfc, lpc_addr );
        if( l_err ) { break; }

        /* Push 4 data bytes into SIO regs 0xF4-0xF7 */
        uint8_t* ptr8 = (uint8_t*)(&i_data);
        size_t i;
        for( i=0; i<sizeof(i_data); i++ )
        {
            l_err = writeRegSIO( i_sfc, 0xF4+i, /*F4,F5,F6,F7 */
                                 ptr8[i] );
            if( l_err ) { break; }
        }
        if( l_err ) { break; }

        /* Trigger the write operation by writing the magic 0xCF value */
        l_err = writeRegSIO( i_sfc, 0xFE, 0xCF );
        if( l_err ) { break; }

    } while(0);

    /*TRACDCOMP( EXIT_MRK"writeRegSPIC> err=%.8X", ERRL_GETEID_SAFE(l_err) ); */
    return l_err;
}

/**
 * @brief Read SPI Controller Register
 */
errorHndl_t readRegSPIC( Sfc_t* i_sfc,
                        SpicReg_t i_reg,
                                    uint32_t* o_data )
{
    //TRACFCOMP( "readRegSPIC> i_reg=0x%.2X", i_reg );
    errorHndl_t l_err = NO_ERROR;

    do {
        /* Compute the full LPC address */
        uint32_t lpc_addr = i_reg | SPIC_BASE_ADDR_AHB;

        /* Setup the logic for the write */
        l_err = setupAddrLPC2AHB( i_sfc, lpc_addr );
        if( l_err ) { break; }

        /* Trigger the write operation by reading the magic register */
        uint8_t ignored = 0;
        l_err = readRegSIO( i_sfc, 0xFE, &ignored );
        if( l_err ) { break; }

        /* Read 4 data bytes into SIO regs 0xF4-0xF7 */
        uint8_t* ptr8 = (uint8_t*)(o_data);
        size_t i;
        for( i=0; i<sizeof(*o_data); i++ )
        {
            l_err = readRegSIO( i_sfc, 0xF4+i, /*F4,F5,F6,F7 */
                                &(ptr8[i]) );
            if( l_err ) { break; }
        }
        if( l_err ) { break; }

    } while(0);

    /*TRACDCOMP( EXIT_MRK"readRegSPIC> o_data=0x%.8X, l_err=%.8X", *o_data, ERRL_GETEID_SAFE(l_err) ); */
    return l_err;
}

/**
 * @brief Poll for completion of SPI operation
 */
errorHndl_t pollOpComplete( Sfc_t* i_sfc )
{
    errorHndl_t l_err = NO_ERROR;
    /* TRACDCOMP( "pollOpComplete>" ); */
int trace_lpc=TRACE_LPC; TRACE_LPC=0;
int trace_sfc=TRACE_SFC; TRACE_SFC=0;

    do {
        /* Send RDSR command until write-in-progress clears */
        NorStatusReg_t status;
        uint64_t poll_time = 0;
        uint64_t loop = 0;
        while( poll_time < MAX_WRITE_TIME_NS )
        {
            uint8_t opcode = SPI_JEDEC_READ_STATUS;
            l_err = sendSpiCmd( i_sfc, opcode,
                                NO_ADDRESS,
                                0, NULL,
                                1, &(status.data8) );
            if( l_err ) { break; }

            /* check if any op is still going */
            if( !status.writeInProgress )
            {
                break;
            }

            /* want to start out incrementing by small numbers then get bigger
               to avoid a really tight loop in an error case so we'll increase
               the wait each time through */
            ++loop;
            sleep( 100*loop );
            poll_time += 100*loop;
        }
        if( l_err ) { break; }

        /*TRACDCOMP(g_trac_pnor,"pollOpComplete> command took %d ns", poll_time); */

        /* No status regs to check so just look for timeout */
        if( status.writeInProgress )
        {
            TRACFCOMP( "pollOpComplete> Timeout during write or erase" );
            l_err = -1;
            break;
        }
    } while(0);

TRACE_LPC=trace_lpc;
TRACE_SFC=trace_sfc;
    /*TRACDCOMP( EXIT_MRK"pollOpComplete> err=%.8X", ERRL_GETEID_SAFE(l_err) ); */
    return l_err;
}

