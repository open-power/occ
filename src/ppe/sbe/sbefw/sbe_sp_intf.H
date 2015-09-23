/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/sbe/sbefw/sbe_sp_intf.H $                             */
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
/*
 *  @file sbe_sp_intf.H
 *
 *  @brief This file contains the SP - SBE interface protocol common details
 */

#ifndef __SBEFW_SBE_SP_INTF_H
#define __SBEFW_SBE_SP_INTF_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * $Version: Conforms to SP-SBE Interface Spec v0.8
 */

/**
 * @brief enums for SBE command classes
 *
*/
enum sbeCommandClass
{
    SBE_CMD_CLASS_UNKNOWN                = 0,
    SBE_CMD_CLASS_IPL_CONTROL            = 0xA1,
    SBE_CMD_CLASS_SCOM_ACCESS            = 0xA2,
    SBE_CMD_CLASS_RING_ACCESS            = 0xA3,
    SBE_CMD_CLASS_MEMORY_ACCESS          = 0xA4,
    SBE_CMD_CLASS_REGISTER_ACCESS        = 0xA5,
    SBE_CMD_CLASS_ARRAY_ACCESS           = 0xA6,
    SBE_CMD_CLASS_INSTRUCTION_CONTROL    = 0xA7,
    SBE_CMD_CLASS_GENERIC_MESSAGE        = 0xA8,
    SBE_CMD_CLASS_MPIPL_COMMANDS         = 0xA9,
};

/**
 * @brief enums for SBE Istep Control command class
 *
*/
enum sbeIplControlCommands
{
    SBE_CMD_EXECUTE_ISTEP      = 0x01, /* Execute istep */
    SBE_CMD_IS_SBE_IPL_DONE    = 0x02, /* Check if SBE IPL Done */
};

/**
 * @brief enums for SCOM Access Messages
 *
*/
enum sbeScomAccessCommands
{
    SBE_CMD_GETSCOM          = 0x01, /* Get SCOM */
    SBE_CMD_PUTSCOM          = 0x02, /* Put SCOM */
    SBE_CMD_MODIFYSCOM       = 0x03, /* Modify SCOM */
    SBE_CMD_PUTSCOM_MASK     = 0x04, /* Put SCOM under mask */
    SBE_CMD_MULTISCOM        = 0x05, /* Execute Multi SCOM */
};

/**
 * @brief enums for Ring Access Messages
 *
*/
enum sbeRingAccessCommands
{
    SBE_CMD_GETRING          = 0x01, /* Get Ring */
    SBE_CMD_PUTRING          = 0x02, /* Put Ring */
};

/**
 * @brief enums for Memory Access Messages
 *
*/
enum sbeMemoryAccesCommands
{
    SBE_CMD_GETMEM           = 0x01, /* Get Memory (Proc/PBA) */
    SBE_CMD_PUTMEM           = 0x02, /* Put Memory (Proc/PBA) */
    SBE_CMD_GETSRAM          = 0x03, /* Get Memory (SRAM) */
    SBE_CMD_PUTSRAM          = 0x04, /* Put Memory (SRAM) */
};

/**
 * @brief enums for GPR/SPR/FPR Access Messages
 *
*/
enum sbeRegisterAccessCommands
{
    SBE_CMD_GETREG           = 0x01, /* Get Register (GPR,SPR,FPR) */
    SBE_CMD_PUTREG           = 0x02, /* Put Register (GPR,SPR,FPR) */
};

/**
 * @brief enums for Trace Array Access Messages
 *
*/
enum sbeArrayAccessCommands
{
    SBE_CMD_GET_FAST_ARRAY        = 0x01, /* Get Fast Array */
    SBE_CMD_GET_TRACE_ARRAY       = 0x02, /* Get Trace Array */
    SBE_CMD_CONTROL_TRACE_ARRAY   = 0x03, /* Control Trace Array */
};

/**
 * @brief enums for Instruction Control Messages
 *
*/
enum sbeInstructionControlCommands
{
    SBE_CMD_CONTROL_INSTRUCTIONS  = 0x01, /* Control Instructions */
};

/**
 * @brief enums for Generic Messages
 *
*/
enum sbeGenericMessageCommands
{
    SBE_CMD_GET_SBE_FFDC          = 0x01, /* Get FFDC */
    SBE_CMD_GET_SBE_VER           = 0x02, /* GET SBE Version */
    SBE_CMD_GET_SBE_CAPABILITIES  = 0x03, /* GET SBE capabilities */
    SBE_CMD_GET_FREQ_SUPPORTED    = 0x04, /* Get Supported frequencies */
    SBE_CMD_GET_SBE_STATE         = 0x05, /* Get SBE State */
};

enum sbeMpIplCommands
{
    SBE_CMD_MPIPL_INVALID = 0x00,
};

/**
 * @brief enums for primary SBE response
 *
*/
enum sbePrimResponse
{
    SBE_PRI_OPERATION_SUCCESSFUL        = 0x00,
    SBE_PRI_INVALID_COMMAND             = 0x01,
    SBE_PRI_INVALID_DATA                = 0x02,
    SBE_PRI_SEQUENCE_ERROR              = 0x03,
    SBE_PRI_INTERNAL_ERROR              = 0x04,
    SBE_PRI_GENERIC_EXECUTION_FAILURE   = 0xFE,
};

/**
 * @brief enums for secondary SBE response
 *   @TODO via RTC: 129763
 *         Discuss on SBE_SEC_INVALID_TARGET_ID_PASSED
 *
*/
enum sbeSecondaryResponse
{
    SBE_SEC_OPERATION_SUCCESSFUL              = 0x00,
    SBE_SEC_COMMAND_CLASS_NOT_SUPPORTED       = 0x01,
    SBE_SEC_COMMAND_NOT_SUPPORTED             = 0x02,
    SBE_SEC_INVALID_ADDRESS_PASSED            = 0x03,
    SBE_SEC_INVALID_TARGET_TYPE_PASSED        = 0x04,
    SBE_SEC_INVALID_TARGET_ID_PASSED          = 0x05,
    SBE_SEC_SPECIFIED_TARGET_NOT_PRESENT      = 0x06,
    SBE_SEC_SPECIFIED_TARGET_NOT_FUNCTIONAL   = 0x07,
    SBE_SEC_COMMAND_NOT_ALLOWED_IN_THIS_STATE = 0x08,
    SBE_SEC_FUNCTIONALITY_NOT_SUPPORTED       = 0x09,
    SBE_SEC_GENERIC_FAILURE_IN_EXECUTION      = 0x0A,
    SBE_SEC_SECURITY_VALIDATION_FAILED        = 0x0B,
    SBE_SEC_OS_FAILURE                        = 0x0C,
    SBE_SEC_FIFO_ACCESS_FAILURE               = 0x0D,
};

/**
 * @brief enums for SBE command timeout values
 *
*/
enum sbeCmdRespTimeout
{
    SBE_CMD_TIMEOUT_SHORT_IN_MSEC = 100,
    SBE_CMD_TIMEOUT_LONG_IN_MSEC  = 30000,
};

/**
 * @brief enums for PCB-PIB Generic Error codes
 *
*/
enum sbePCBPIBErrorRC
{
    SBE_PCB_PIB_ERROR_NONE                = 0x00,
    SBE_PCB_PIB_ERROR_RESOURCE_OCCUPIED   = 0x01,
    SBE_PCB_PIB_ERROR_CHIPLET_OFFLINE     = 0x02,
    SBE_PCB_PIB_ERROR_PARTIAL_GOOD        = 0x03,
    SBE_PCB_PIB_ERROR_ADDRESS_ERROR       = 0x04,
    SBE_PCB_PIB_ERROR_CLOCK_ERROR         = 0x05,
    SBE_PCB_PIB_ERROR_PACKET_ERROR        = 0x06,
    SBE_PCB_PIB_ERROR_TIMEOUT             = 0x07,
};


#ifdef __cplusplus
}
#endif

#endif /* __SBEFW_SBE_SP_INTF_H */
