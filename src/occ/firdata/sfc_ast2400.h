/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/firdata/sfc_ast2400.H $                               */
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
#ifndef __PNOR_SFCAST2400_H
#define __PNOR_SFCAST2400_H

#include <native.h>
#include <homerData_common.h>

/** @file sfc_ast2400.H
 *  @brief Provides the logic to access and configure the
 *         AST2400 BMC in order to access the PNOR
 */

/**
 * @brief SPI04 Control Register
 */
typedef union
{
    uint32_t data32;
    struct
    {                             /*Little-endian bit positions*/
        uint32_t rsvd            : 2; /*31:30*/
        uint32_t ioMode          : 2; /*29:28*/
        uint32_t pulseWidth      : 4; /*27:24*/
        uint32_t cmdData         : 8; /*23:16*/
        uint32_t dummyCycleCmd   : 1; /*15*/
        uint32_t dummyCycleRead1 : 1; /*14*/
        uint32_t fourByteMode    : 1; /*13*/
        uint32_t disableCmdMerge : 1; /*12*/
        uint32_t spiClkFreq      : 4; /*11:8*/
        uint32_t dummyCycleRead2 : 2; /*7:6*/
        uint32_t lsbFirst        : 1; /*5*/
        uint32_t useClkMode3     : 1; /*4*/
        uint32_t dualInputMode   : 1; /*3*/
        uint32_t stopActiveCtl   : 1; /*2*/
        uint32_t cmdMode         : 2; /*1:0*/
    };
} SpiControlReg04_t;


/**
 * @brief Structure to hold information about the SFC
 */
typedef struct
{
    /**
     * @brief Default value of SPI04 (saves a read)
     */
    SpiControlReg04_t iv_ctlRegDefault;

    /**
     * @brief Hardware workarounds
     */
    uint32_t iv_flashWorkarounds;

    /**
     * @brief LPC address that marks the beginning of flash
     */
    uint32_t iv_mmioOffset;
} Sfc_t;


/**
 * @brief Initialize the SFC Hardware
 * @param[in] Pointer to Sfc struct
 *
 * @return void
 */
errorHndl_t hwInit(Sfc_t* i_sfc);

/**
 * @brief Read data from the PNOR flash
 *
 * @param[in] Pointer to Sfc struct
 * @parm[in] i_addr  PNOR flash Address to read
 * @parm[in] i_size  Amount of data to read, in bytes.
 * @parm[out] o_data  Buffer to read data into
 *
 * @return Error from operation
 */
errorHndl_t readFlash(Sfc_t* i_sfc,
                      uint32_t i_addr,
                      size_t i_size,
                      void* o_data);

/**
 * @brief Write data to the PNOR flash
 *
 * @param[in] Pointer to Sfc struct
 * @parm i_addr  PNOR flash Address to write
 * @parm i_size  Amount of data to write, in bytes.
 * @parm i_data  Buffer containing data to write
 *
 * @return Error from operation
 */
errorHndl_t writeFlash(Sfc_t* i_sfc,
                       uint32_t i_addr,
                       size_t i_size,
                       void* i_data);

/**
 * @brief Erase a block of flash
 */
errorHndl_t eraseFlash( Sfc_t* i_sfc,
                        uint32_t i_addr );

/** @brief Constant for sendSpiCmd parameter */
static const uint32_t NO_ADDRESS = UINT32_MAX;


/**
 * @brief Send a SPI command
 *
 * @param[in] Pointer to Sfc struct
 * @parm[in] i_opCode: command to send into controller first
 * @parm[in] i_address: address for those commands that need it
 * @parm[in] i_writeCnt: number of bytes to write to device
 * @parm[in] i_writeData: write data buffer
 * @parm[in] i_readCnt: number of bytes to read from device
 * @parm[out] o_readData: read data buffer
 *
 * @return Error from operation
 */
errorHndl_t sendSpiCmd( Sfc_t* i_sfc,
                        uint8_t i_opCode,
                        uint32_t i_address,
                        size_t i_writeCnt,
                        const uint8_t* i_writeData,
                        size_t i_readCnt,
                        uint8_t* o_readData );

/**
 * @brief List of registers in the SPI Controller logic
 */
typedef enum
{
    CONFREG_00     = 0x00,
    CTLREG_04      = 0x04,
    MISCCTLREG_10  = 0x10,
    READTIMEREG_14 = 0x14
} SpicReg_t;

/**
 * @brief Write a SPI Controller register
 *
 * @param[in] Pointer to Sfc struct
 * @param[in] i_reg: Register to write
 * @param[in] i_data: Data to write
 *
 * @return Error from operation
 */
errorHndl_t writeRegSPIC( Sfc_t* i_sfc,
                          SpicReg_t i_reg,
                          uint32_t i_data );

/**
 * @brief Write a SPI Controller register
 *
 * @param[in] Pointer to Sfc struct
 * @param[in] i_reg: Register to write
 * @param[in] o_data: Data that was read
 *
 * @return Error from operation
 */
errorHndl_t readRegSPIC( Sfc_t* i_sfc,
                         SpicReg_t i_reg,
                         uint32_t* o_data );

/**
 * @brief Write a single byte into a SIO register
 *
 * @param[in] Pointer to Sfc struct
 * @param[in] i_reg: Register to write
 * @param[in] i_data: Data to write
 *
 * @return Error from operation
 */
errorHndl_t writeRegSIO( Sfc_t* i_sfc,
                         uint8_t i_regAddr,
                         uint8_t i_data );

/**
 * @brief Read a single byte from a SIO register
 *
 * @param[in] Pointer to Sfc struct
 * @param[in] i_reg: Register to read
 * @param[in] o_data: Data that was read
 *
 * @return Error from operation
 */
errorHndl_t readRegSIO( Sfc_t* i_sfc,
                        uint8_t i_regAddr,
                        uint8_t* o_data );

/**
 * @brief Enable write mode
 *
 * @param[in] Pointer to Sfc struct
 * @return Error from operation
 */
errorHndl_t enableWriteMode( Sfc_t* i_sfc );

/**
 * @brief Enter/exit command mode
 *
 * @param[in] Pointer to Sfc struct
 * @param[in] i_enter: true=enter cmd mode, false=exit cmd mode
 *
 * @return Error from operation
 */
errorHndl_t commandMode( Sfc_t* i_sfc,
                         bool i_enter );

/**
 * @brief Poll for completion of SPI operation
 *
 * @param[in] Pointer to Sfc struct
 * @return Error from operation
 */
errorHndl_t pollOpComplete( Sfc_t* i_sfc );

/**
 * @brief Prepare the iLPC2AHB address regs
 *
 * @param[in] Pointer to Sfc struct
 * @param[in] i_addr: LPC address to access
 *
 * @return Error from operation
 */
errorHndl_t setupAddrLPC2AHB( Sfc_t* i_sfc,
                              uint32_t i_addr );


/**
 * @brief SPI0 Configuration Register
 */
typedef union
{
    uint32_t data32;
    struct
    {                             /*Little-endian bit positions*/
        uint32_t rsvd            : 30; /*31:2*/
        uint32_t inactiveX2mode  : 1; /*1*/
        uint32_t enableWrite     : 1; /*0*/
    };
} SpiConfigReg00_t;

/** @brief General Constants */
enum
{
    LPC_TOP_OF_FLASH_OFFSET = 0x0FFFFFFF,

    /**< Offset to SPI Controller Register Space */
    LPC_SFC_CTLR_BASE   = 0x1E789000,

    /**< AHB address of SPI Flash controller */
    SPIC_BASE_ADDR_AHB = 0x1E630000,

    /**< AHB address of flash  */
    FLASH_BASE_ADDR_AHB = 0x30000000,

    /**< AHB address of LPC registers */
    LPC_CTLR_BASE_ADDR_AHB = 0x1E789000,

    /**< Maximum time to wait for a write/erase */
    MAX_WRITE_TIME_NS = NS_PER_SEC,

    /**< SuperIO Address Cycle */
    SIO_ADDR_2E = 0x2E,

    /**< SuperIO Data Cycle */
    SIO_DATA_2F = 0x2F,
};


#endif
