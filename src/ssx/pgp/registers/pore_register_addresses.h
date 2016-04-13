/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/pgp/registers/pore_register_addresses.h $             */
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
#ifndef __PORE_REGISTER_ADDRESSES_H__
#define __PORE_REGISTER_ADDRESSES_H__

/// \file pore_register_addresses.h
/// \brief Symbolic addresses for the PORE unit

// *** WARNING *** - This file is generated automatically, do not edit.


#define PORE_GPE0_OCI_BASE 0x40000000
#define PORE_GPE1_OCI_BASE 0x40000100
#define PORE_SLW_OCI_BASE 0x40040000
#define PORE_STATUS_OFFSET 0x00000000
#define PORE_GPE0_STATUS 0x40000000
#define PORE_GPE1_STATUS 0x40000100
#define PORE_SLW_STATUS 0x40040000
#define PORE_CONTROL_OFFSET 0x00000008
#define PORE_GPE0_CONTROL 0x40000008
#define PORE_GPE1_CONTROL 0x40000108
#define PORE_SLW_CONTROL 0x40040008
#define PORE_RESET_OFFSET 0x00000010
#define PORE_GPE0_RESET 0x40000010
#define PORE_GPE1_RESET 0x40000110
#define PORE_SLW_RESET 0x40040010
#define PORE_ERROR_MASK_OFFSET 0x00000018
#define PORE_GPE0_ERROR_MASK 0x40000018
#define PORE_GPE1_ERROR_MASK 0x40000118
#define PORE_SLW_ERROR_MASK 0x40040018
#define PORE_PRV_BASE_ADDRESS0_OFFSET 0x00000020
#define PORE_GPE0_PRV_BASE_ADDRESS0 0x40000020
#define PORE_GPE1_PRV_BASE_ADDRESS0 0x40000120
#define PORE_SLW_PRV_BASE_ADDRESS0 0x40040020
#define PORE_PRV_BASE_ADDRESS1_OFFSET 0x00000028
#define PORE_GPE0_PRV_BASE_ADDRESS1 0x40000028
#define PORE_GPE1_PRV_BASE_ADDRESS1 0x40000128
#define PORE_SLW_PRV_BASE_ADDRESS1 0x40040028
#define PORE_OCI_BASE_ADDRESS0_OFFSET 0x00000030
#define PORE_GPE0_OCI_BASE_ADDRESS0 0x40000030
#define PORE_GPE1_OCI_BASE_ADDRESS0 0x40000130
#define PORE_SLW_OCI_BASE_ADDRESS0 0x40040030
#define PORE_OCI_BASE_ADDRESS1_OFFSET 0x00000038
#define PORE_GPE0_OCI_BASE_ADDRESS1 0x40000038
#define PORE_GPE1_OCI_BASE_ADDRESS1 0x40000138
#define PORE_SLW_OCI_BASE_ADDRESS1 0x40040038
#define PORE_TABLE_BASE_ADDR_OFFSET 0x00000040
#define PORE_GPE0_TABLE_BASE_ADDR 0x40000040
#define PORE_GPE1_TABLE_BASE_ADDR 0x40000140
#define PORE_SLW_TABLE_BASE_ADDR 0x40040040
#define PORE_EXE_TRIGGER_OFFSET 0x00000048
#define PORE_GPE0_EXE_TRIGGER 0x40000048
#define PORE_GPE1_EXE_TRIGGER 0x40000148
#define PORE_SLW_EXE_TRIGGER 0x40040048
#define PORE_SCRATCH0_OFFSET 0x00000050
#define PORE_GPE0_SCRATCH0 0x40000050
#define PORE_GPE1_SCRATCH0 0x40000150
#define PORE_SLW_SCRATCH0 0x40040050
#define PORE_SCRATCH1_OFFSET 0x00000058
#define PORE_GPE0_SCRATCH1 0x40000058
#define PORE_GPE1_SCRATCH1 0x40000158
#define PORE_SLW_SCRATCH1 0x40040058
#define PORE_SCRATCH2_OFFSET 0x00000060
#define PORE_GPE0_SCRATCH2 0x40000060
#define PORE_GPE1_SCRATCH2 0x40000160
#define PORE_SLW_SCRATCH2 0x40040060
#define PORE_IBUF_01_OFFSET 0x00000068
#define PORE_GPE0_IBUF_01 0x40000068
#define PORE_GPE1_IBUF_01 0x40000168
#define PORE_SLW_IBUF_01 0x40040068
#define PORE_IBUF_2_OFFSET 0x00000070
#define PORE_GPE0_IBUF_2 0x40000070
#define PORE_GPE1_IBUF_2 0x40000170
#define PORE_SLW_IBUF_2 0x40040070
#define PORE_DBG0_OFFSET 0x00000078
#define PORE_GPE0_DBG0 0x40000078
#define PORE_GPE1_DBG0 0x40000178
#define PORE_SLW_DBG0 0x40040078
#define PORE_DBG1_OFFSET 0x00000080
#define PORE_GPE0_DBG1 0x40000080
#define PORE_GPE1_DBG1 0x40000180
#define PORE_SLW_DBG1 0x40040080
#define PORE_PC_STACK0_OFFSET 0x00000088
#define PORE_GPE0_PC_STACK0 0x40000088
#define PORE_GPE1_PC_STACK0 0x40000188
#define PORE_SLW_PC_STACK0 0x40040088
#define PORE_PC_STACK1_OFFSET 0x00000090
#define PORE_GPE0_PC_STACK1 0x40000090
#define PORE_GPE1_PC_STACK1 0x40000190
#define PORE_SLW_PC_STACK1 0x40040090
#define PORE_PC_STACK2_OFFSET 0x00000098
#define PORE_GPE0_PC_STACK2 0x40000098
#define PORE_GPE1_PC_STACK2 0x40000198
#define PORE_SLW_PC_STACK2 0x40040098
#define PORE_ID_FLAGS_OFFSET 0x000000a0
#define PORE_GPE0_ID_FLAGS 0x400000a0
#define PORE_GPE1_ID_FLAGS 0x400001a0
#define PORE_SLW_ID_FLAGS 0x400400a0
#define PORE_DATA0_OFFSET 0x000000a8
#define PORE_GPE0_DATA0 0x400000a8
#define PORE_GPE1_DATA0 0x400001a8
#define PORE_SLW_DATA0 0x400400a8
#define PORE_MEMORY_RELOC_OFFSET 0x000000b0
#define PORE_GPE0_MEMORY_RELOC 0x400000b0
#define PORE_GPE1_MEMORY_RELOC 0x400001b0
#define PORE_SLW_MEMORY_RELOC 0x400400b0
#define PORE_I2C_E0_PARAM_OFFSET 0x000000b8
#define PORE_I2C_E1_PARAM_OFFSET 0x000000c0
#define PORE_I2C_E2_PARAM_OFFSET 0x000000c8
#define PORE_GPE0_I2C_EN_PARAM(n) (PORE_GPE0_I2C_E0_PARAM + ((PORE_GPE0_I2C_E1_PARAM - PORE_GPE0_I2C_E0_PARAM) * (n)))
#define PORE_GPE1_I2C_EN_PARAM(n) (PORE_GPE1_I2C_E0_PARAM + ((PORE_GPE1_I2C_E1_PARAM - PORE_GPE1_I2C_E0_PARAM) * (n)))
#define PORE_SLW_I2C_EN_PARAM(n) (PORE_SLW_I2C_E0_PARAM + ((PORE_SLW_I2C_E1_PARAM - PORE_SLW_I2C_E0_PARAM) * (n)))
#define PORE_GPE0_I2C_E0_PARAM 0x400000b8
#define PORE_GPE1_I2C_E0_PARAM 0x400001b8
#define PORE_SLW_I2C_E0_PARAM 0x400400b8
#define PORE_GPE0_I2C_E1_PARAM 0x400000c0
#define PORE_GPE1_I2C_E1_PARAM 0x400001c0
#define PORE_SLW_I2C_E1_PARAM 0x400400c0
#define PORE_GPE0_I2C_E2_PARAM 0x400000c8
#define PORE_GPE1_I2C_E2_PARAM 0x400001c8
#define PORE_SLW_I2C_E2_PARAM 0x400400c8

#endif // __PORE_REGISTER_ADDRESSES_H__

