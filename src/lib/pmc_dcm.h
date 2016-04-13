/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/pmc_dcm.h $                                           */
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
#ifndef __PMC_DCM_H__
#define __PMC_DCM_H__

/// \file pmc_dcm.h
/// \brief Generic PMC Interhchip Communication Mechanism 


/// PMC-DCM Return Code
#define PMC_DCM_SUCCESS              0
#define PMC_DCM_ARG_NULL_OBJECT_SEND 0x00326501 //ssx panic
#define PMC_DCM_ARG_NULL_OBJECT_RECV 0x00326501 //ssx panic
#define PMC_DCM_INTCHP_DISABLED_SEND 0x00326502 //ssx panic
#define PMC_DCM_INTCHP_DISABLED_RECV 0x00326502 //ssx panic
#define PMC_DCM_OUTSTANDING_TRANSFER 0x00326503 //user handle
#define PMC_DCM_INTCHP_CMD_ONLY_MTOS 0x00326504 //ssx panic
#define PMC_DCM_INTCHP_CMD_ONLY_STOM 0x00326504 //ssx panic
#define PMC_DCM_INVALID_COMMAND_CODE 0x00326505 //ssx panic
#define PMC_DCM_RECEIVE_NOT_DETECTED 0x00326506 //user handle
#define PMC_DCM_RECEIVE_NOT_MSG_TYPE 0x00326507 //ssx panic
    
/// PMC Interchip Command Code
#define PMC_IC_GPA_CC           1  //0b0001
#define PMC_IC_GPA_ACK_CC       2  //0b0010
#define PMC_IC_GAR_CC           3  //0b0011
#define PMC_IC_PING_CC          4  //0b0100
#define PMC_IC_PING_ACK_CC      6  //0b0110
#define PMC_IC_MSG_CC           8  //0b1000
#define PMC_IC_MSG_NACK_CC      10 //0b1010
#define PMC_IC_MSG_ACK_CC       11 //0b1011
#define PMC_IC_ERROR_CC         15 //0b1111

#ifndef __ASSEMBLER__

/// PMC-DCM low-level (hardware) packet

typedef union PmcInterchipPacket { 
    uint32_t value;
    struct {
        /// Hardware command code
        uint8_t cmd_code : 4;
        /// Hardware command extension; GPSM-DCM command code
        uint8_t cmd_ext : 4;
        /// Payload. The plan of record is to use payload[2] as HW-generated ECC.
        uint8_t payload[3];
    } fields;
} PmcDcmPacket;

/// Macro to set and get payload field
#define SET_PAYLOAD_FIELD(value) (value << 8) & 0xFFFF00
#define GET_PAYLOAD_FIELD(value) (value & 0xFFFF00) >> 8


/// low-level PMC-DCM interchip communication methods

int
pmc_dcm_send(PmcDcmPacket* hwPacket);

int
pmc_dcm_receive(PmcDcmPacket* hwPacket);

void
_pmc_dcm_send(uint32_t *value);

void
_pmc_dcm_receive(uint32_t *value);

int
pmc_dcm_check_ic_command(int cmd_code);

int
pmc_dcm_if_dcm_master();

void
pmc_dcm_set_interchip_mode(int master);

int
pmc_dcm_if_interchip_interface_enabled();

void
pmc_dcm_set_interchip_interface(int enable);

int
pmc_dcm_if_channel_busy();

int
pmc_dcm_if_new_message();

void
pmc_dcm_init(int master_or_slace);

#endif  /* __ASSEMBLER__ */

#endif  /* __PMC_DCM_H__ */

