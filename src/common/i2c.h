/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/common/i2c.h $                                            */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2024                        */
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

#ifndef _I2C_H
#define _I2C_H

// I2C SCOM Addresses:
// (There are unique constants/addresses per engine, but to make the calls generic,
//  these constants are defined instead of using the base constants)
#define PIB_BASE                    0x000A0000
// Engine B 0x00A0000
// Engine C 0x00A1000
// Engine D 0x00A2000
// Engine E 0x00A3000

// Fast mode I2C registers
#define I2C_FM_CTRL_REG             0x000A0000 // Write only
#define I2C_FM_DATA_8TO15_REG       0x000A0001 // Read-only
#define I2C_FM_RESET_I2C_REG        0x000A0001 // Write
#define I2C_FM_STATUS_REG           0x000A0002
#define I2C_FM_DATA_0TO7_REG        0x000A0003 // First 8 bytes of read, 5th to 12th bytes of write

// Typical I2C registers
#define I2C_FIFO1_REG_READ          0x000A0004
#define I2C_COMMAND_REG             0x000A0005
#define I2C_MODE_REG                0x000A0006
#define I2C_INTERRUPT_MASK_REG      0x000A0008
#define I2C_STATUS_REG              0x000A000B // read
#define I2C_IMM_RESET_I2C           0x000A000B // write
#define I2C_BUSY_REGISTER           0x000A000E
#define I2C_FIFO4_REG_READ          0x000A0012

#define SCOM_ENGINE_OFFSET(engine) (engine << 12)

// Fast mode I2C status register masks
#define FM_STATUS_ERROR_MASK            0x000000000007E400
#define FM_ERROR_OR_COMPLETE_MASK       0x000000000007FC00
#define FM_STATUS_COMPLETE_MASK         0x0000000000000800
#define FM_STATUS_DATA_REQ_MASK         0x0000000000001000

// I2C Status Register masks
#define STATUS_ERROR_MASK               0xFC80000000000000
#define STATUS_ERROR_OR_COMPLETE_MASK   0xFF80000000000000
#define STATUS_COMPLETE_MASK            0x0100000000000000
#define PEEK_ERROR_MASK                 0x00000000FC000000
#define PEEK_MORE_DATA                  0x0000000002000000

// P10 : localbus = pib / 2 = (pau/4) / 2 = (2133/4) / 2 = 266.625
// 0-15: Bit Rate Divisor - 0x00AE gives approx 400kHz (and allows margin for clock variation)
// 16-21: Port Number (0-5)
// 22-26: reserved (0s)
// 28: fgat mode. Must be set to allow stop only with secure boot enabled
#define I2C_MODE_REG_DIVISOR    0x00AE000800000000

typedef enum
{
    PIB_I2C_ENGINE_C = 0x01,
    PIB_I2C_ENGINE_D = 0x02,
    PIB_I2C_ENGINE_E = 0x03,
    PIB_I2C_ENGINE_ALL = 0xFF,
} PIB_I2C_ENGINE;

#endif // _I2C_H
