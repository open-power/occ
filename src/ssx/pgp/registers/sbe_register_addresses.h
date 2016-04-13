/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/pgp/registers/sbe_register_addresses.h $              */
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
#ifndef __SBE_REGISTER_ADDRESSES_H__
#define __SBE_REGISTER_ADDRESSES_H__

/// \file sbe_register_addresses.h
/// \brief Symbolic addresses for the SBE unit

// *** WARNING *** - This file is generated automatically, do not edit.


#define PORE_SBE_PIB_BASE 0x000e0000
#define PORE_SBE_STATUS 0x000e0000
#define PORE_SBE_CONTROL 0x000e0001
#define PORE_SBE_RESET 0x000e0002
#define PORE_SBE_ERROR_MASK 0x000e0003
#define PORE_SBE_PRV_BASE_ADDRESS0 0x000e0004
#define PORE_SBE_PRV_BASE_ADDRESS1 0x000e0005
#define PORE_SBE_OCI_BASE_ADDRESS0 0x000e0006
#define PORE_SBE_OCI_BASE_ADDRESS1 0x000e0007
#define PORE_SBE_TABLE_BASE_ADDR 0x000e0008
#define PORE_SBE_EXE_TRIGGER 0x000e0009
#define PORE_SBE_SCRATCH0 0x000e000a
#define PORE_SBE_SCRATCH1 0x000e000b
#define PORE_SBE_SCRATCH2 0x000e000c
#define PORE_SBE_IBUF_01 0x000e000d
#define PORE_SBE_IBUF_2 0x000e000e
#define PORE_SBE_DBG0 0x000e000f
#define PORE_SBE_DBG1 0x000e0010
#define PORE_SBE_PC_STACK0 0x000e0011
#define PORE_SBE_PC_STACK1 0x000e0012
#define PORE_SBE_PC_STACK2 0x000e0013
#define PORE_SBE_ID_FLAGS 0x000e0014
#define PORE_SBE_DATA0 0x000e0015
#define PORE_SBE_MEMORY_RELOC 0x000e0016
#define PORE_SBE_I2C_EN_PARAM(n) (PORE_SBE_I2C_E0_PARAM + ((PORE_SBE_I2C_E1_PARAM - PORE_SBE_I2C_E0_PARAM) * (n)))
#define PORE_SBE_I2C_E0_PARAM 0x000e0017
#define PORE_SBE_I2C_E1_PARAM 0x000e0018
#define PORE_SBE_I2C_E2_PARAM 0x000e0019

#endif // __SBE_REGISTER_ADDRESSES_H__

