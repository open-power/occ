/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/powmanlib/stop_sgpe_cme_api.h $                       */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2019                             */
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
#ifndef __STOP_SGPE_CME_API_H__
#define __STOP_SGPE_CME_API_H__


enum SGPE_STOP_IRQ_PAYLOAD_MASKS
{
    TYPE2_PAYLOAD_STOP_LEVEL               = 0x00F,
    TYPE2_PAYLOAD_ENTRY_EVENT              = 0x000, // 0000
    TYPE0_PAYLOAD_ENTRY_RCLK               = 0x100, // 0001
    // 0010 reserved
    // 0011 reserved

    TYPE2_PAYLOAD_EXIT_EVENT               = 0xC00, // 1100
    TYPE2_PAYLOAD_HARDWARE_WAKEUP          = 0x800, // 1000
    TYPE2_PAYLOAD_SOFTWARE_WAKEUP          = 0x400, // 0100
    TYPE0_PAYLOAD_EXIT_RCLK                = 0x500, // 0101
    // 0110 reserved
    TYPE2_PAYLOAD_DECREMENTER_WAKEUP       = 0x700, // 0111

    // These bit1-4 is aligned with the bit0-3 of DB1 below
    TYPE2_PAYLOAD_SUSPEND_ACTION_MASK      = 0x400,
    TYPE2_PAYLOAD_SUSPEND_EXIT_MASK        = 0x200,
    TYPE2_PAYLOAD_SUSPEND_ENTRY_MASK       = 0x100,
    TYPE2_PAYLOAD_SUSPEND_SELECT_MASK      = 0x080,
    TYPE2_PAYLOAD_SUSPEND_ACK_MASK         = 0x040,

    TYPE6_PAYLOAD_EXIT_EVENT               = 0x00F
};


/// PIG TYPEs used by CME
enum CME_STOP_PIG_TYPES
{
    PIG_TYPE0                              = 0,
    PIG_TYPE2                              = 2,
    PIG_TYPE3                              = 3
};

/// Numberical Doorbell Message IDs(used by CME check)
enum CME_DOORBELL_MESSAGE_IDS
{
    // Bit0:Suspend(1)/Unsuspend(0) | Bit1:Exit | Bit2:Entry | Bit3:Suspend(1)/Block(0)

    // Illegal as Unblock without Entry/Exit   00, // 0000
    MSGID_DB1_UNBLOCK_STOP_ENTRIES         = 0x02, // 0010
    MSGID_DB1_UNBLOCK_STOP_EXITS           = 0x04, // 0100
    MSGID_DB1_UNBLOCK_STOP_ENTRIES_EXITS   = 0x06, // 0110

    // Illegal as Block without Entry/Exit     08, // 1000
    MSGID_DB1_BLOCK_STOP_ENTRIES           = 0x0A, // 1010
    MSGID_DB1_BLOCK_STOP_EXITS             = 0x0C, // 1100
    MSGID_DB1_BLOCK_STOP_ENTRIES_EXITS     = 0x0E, // 1110

    // Illegal as Unsuspend without Entry/Exit 01, // 0001
    MSGID_DB1_UNSUSPEND_STOP_ENTRIES       = 0x03, // 0011
    MSGID_DB1_UNSUSPEND_STOP_EXITS         = 0x05, // 0101
    MSGID_DB1_UNSUSPEND_STOP_ENTRIES_EXITS = 0x07, // 0111

    // Illegal as Suspend without Entry/Exit   09, // 1001
    MSGID_DB1_SUSPEND_STOP_ENTRIES         = 0x0B, // 1011
    MSGID_DB1_SUSPEND_STOP_EXITS           = 0x0D, // 1101
    MSGID_DB1_SUSPEND_STOP_ENTRIES_EXITS   = 0x0F, // 1111

    MSGID_DB2_DECREMENTER_WAKEUP           = 0x01,
    MSGID_DB2_RESONANT_CLOCK_DISABLE       = 0x02,
    MSGID_DB2_RESONANT_CLOCK_ENABLE        = 0x03
};

#endif  /* __STOP_SGPE_CME_API_H__ */
