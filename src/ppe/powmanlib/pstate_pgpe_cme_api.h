/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/powmanlib/pstate_pgpe_cme_api.h $                     */
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
#ifndef __PSTATE_PGPE_CME_API_H__
#define __PSTATE_PGPE_CME_API_H__

#define DPLL_REFCLK_DIVIDER         16667

#define PMCR_UPPERPS_MASK  0xFF00000000000000
#define PMCR_LOWERPS_MASK  0x00FF000000000000
#define PMCR_PSTATE_SHIFT_AMOUNT SHIFT64(15)

#define PIG_PAYLOAD_MASK            0x0fff000000000000
#define PIG_INTR_FIELD_MASK         0x7000000000000000
#define PIG_INTR_GRANTED_MASK       0x0000000080000000

// PIR defines
#define PIR_INSTANCE_EVEN_ODD_MASK  (uint32_t)(0x00000001)
#define PIR_INSTANCE_NUM_MASK       (uint32_t)(0x0000000F)

//Bit0 WR: 0
//Bit1 Multicast: 1
//Bit 2:4 Multicast Type: 101
//Bit 5:7 Mutlicast Grp: 001
#define PCB_MULTICAST_GRP1          0x69000000
#define PCB_MULTICAST_GRP4          0x6c000000

#define QUAD_FROM_CORE(c) \
    ((c&0x1C) >> 2)

#define QUAD_FROM_CME_INSTANCE_NUM(num) \
    ((num&0xE) >> 1)


//PMCR GA1 Definition
enum PMCR_DEFS
{
    PMCR_UPPER_PS_START         = 0,
    PMCR_UPPER_PS_LENGTH        = 8,
    PMCR_LOWER_PS_START         = 8,
    PMCR_LOWER_PS_LENGTH        = 8,
    PMCR_VERSION_START          = 60,
    PMCR_VERSION_LENGTH         = 4
};

//PMSR GA1 Definition
enum PMSR_DEFS
{
    PMSR_GLOBAL_ACTUAL_PSTATE_START     = 0,
    PMSR_GLOBAl_ACTUAL_PSTATE_LENGTH    = 8,
    PMSR_LOCAL_ACTUAL_PSTATE_START      = 8,
    PMSR_LOCAL_ACTUAL_PSTATE_LENGTH     = 8,
    PMSR_PMIN_START                     = 16,
    PMSR_PMIN_LENGTH                    = 8,
    PMSR_PMAX_START                     = 24,
    PMSR_PMAX_LENGTH                    = 8,
    PMSR_PMCR_DISABLED                  = 32,
    PMSR_SAFE_MODE                      = 33,
    PMSR_IVRM_ALLOWED                   = 34,
    PMSR_IVRM_ENABLED                   = 35,
    PMSR_UPDATE_IN_PROGRESS             = 58,
    PMSR_INVALID_PMCR_VERSION           = 59,
    PMSR_VERSION_START                  = 60,
    PMSR_VERSION_LENGTH                 = 4
};

//
// CME<->PGPE API
//
enum MESSAGE_ID_DB0
{
    MSGID_DB0_VALID_START                   = 0, //This is for error checking
    //This is used when sending DB3 with payloads in DB0.
    //The reason is DB3 register has less bits than DB0.
    MSGID_DB0_DB3_PAYLOAD                   = 0,
    MSGID_DB0_RESERVED                      = 1,
    MSGID_DB0_GLOBAL_ACTUAL_BROADCAST       = 2,
    MSGID_DB0_START_PSTATE_BROADCAST        = 3,
    MSGID_DB0_STOP_PSTATE_BROADCAST         = 4,
    MSGID_DB0_CLIP_BROADCAST                = 5,
    MSGID_DB0_PMSR_UPDT                     = 6,
    MSGID_DB0_REGISTER_DONE                 = 7,
    MSGID_DB0_VALID_END                     = 7 //This is for error checking
};

enum MESSAGE_ID_DB3
{
    MSGID_DB3_INVALID                   = 0,
    MSGID_DB3_HIGH_PRIORITY_PSTATE      = 0x01,
    MSGID_DB3_ENTER_SAFE_MODE           = 0x02,
    MSGID_DB3_REPLAY_DB0                = 0x03,
    MSGID_DB3_DISABLE_SGPE_HANDOFF      = 0x04,
    MSGID_DB3_SUSPEND_STOP_ENTRY        = 0x05,
    MSGID_DB3_UNSUSPEND_STOP_ENTRY      = 0x06,
    MSGID_DB3_IMMEDIATE_HALT            = 0xF1,
    MSGID_DB3_RESTORE_STATE_AND_HALT    = 0xF2,
};

enum MESSAGEID_PCB_TYPE4_ACK_TYPES
{
    MSGID_PCB_TYPE4_ACK_ERROR                    = 0,
    MSGID_PCB_TYPE4_ACK_PSTATE_PROTO_ACK         = 1,
    MSGID_PCB_TYPE4_ACK_PSTATE_SUSPENDED         = 2,
    MSGID_PCB_TYPE4_QUAD_MGR_AVAILABLE           = 3,
    MSGID_PCB_TYPE4_NACK_DROOP_PRESENT           = 4,
    MSGID_PCB_TYPE4_SUSPEND_ENTRY_ACK            = 5,
    MSGID_PCB_TYPE4_UNSUSPEND_ENTRY_ACK          = 6
};

enum DB0_CLIP_BCAST_FIELDS
{
    DB0_CLIP_BCAST_TYPE_PMIN      = 0,
    DB0_CLIP_BCAST_TYPE_PMAX      = 1
};

enum DB0_PMSR_UPDT_COMMANDS
{
    DB0_PMSR_UPDT_SET_PSTATES_SUSPENDED     = 0x1,
    DB0_PMSR_UPDT_CLEAR_PSTATES_SUSPENDED   = 0x2
};

//
//PGPE-CME Doorbell0(Global Actual Broadcast)
//
typedef union pgpe_db0_glb_bcast_t
{
    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint64_t  msg_id : 8;
        uint64_t  global_actual : 8;
        uint64_t  quad0_ps: 8;
        uint64_t  quad1_ps: 8;
        uint64_t  quad2_ps: 8;
        uint64_t  quad3_ps: 8;
        uint64_t  quad4_ps: 8;
        uint64_t  quad5_ps: 8;
#else
        uint64_t  quad5_ps: 8;
        uint64_t  quad4_ps: 8;
        uint64_t  quad3_ps: 8;
        uint64_t  quad2_ps: 8;
        uint64_t  quad1_ps: 8;
        uint64_t  quad0_ps: 8;
        uint64_t  global_actual : 8;
        uint64_t  msg_id : 8;
#endif
    } fields;
} pgpe_db0_glb_bcast_t;

//
//PGPE-CME Doorbell0(Start Pstate Broaadcast)
//
typedef union pgpe_db0_start_ps_bcast
{
    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint64_t msg_id : 8;
        uint64_t global_actual : 8;
        uint64_t quad0_ps : 8;
        uint64_t quad1_ps : 8;
        uint64_t quad2_ps : 8;
        uint64_t quad3_ps : 8;
        uint64_t quad4_ps : 8;
        uint64_t quad5_ps : 8;
#else
        uint64_t quad5_ps : 8;
        uint64_t quad4_ps : 8;
        uint64_t quad3_ps : 8;
        uint64_t quad2_ps : 8;
        uint64_t quad1_ps : 8;
        uint64_t quad0_ps : 8;
        uint64_t global_actual : 8;
        uint64_t msg_id : 8;
#endif
    } fields;
} pgpe_db0_start_ps_bcast_t;

//
//PGPE-CME Doorbell0(Pstate Stop Broaadcast)
//
typedef union pgpe_db0_stop_ps_bcast
{
    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint64_t msg_id : 8;
        uint64_t reserved : 56;
#else
        uint64_t reserved : 56;
        uint64_t msg_id : 8;
#endif
    } fields;
} pgpe_db0_stop_ps_bcast_t;

//
//PGPE-CME Doorbell0(Clip Broadcast)
//
typedef union pgpe_db0_clip_bcast
{
    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint64_t msg_id : 8;
        uint64_t clip_type : 8;
        uint64_t quad0_clip : 8;
        uint64_t quad1_clip : 8;
        uint64_t quad2_clip : 8;
        uint64_t quad3_clip : 8;
        uint64_t quad4_clip : 8;
        uint64_t quad5_clip : 8;
#else
        uint64_t quad5_clip : 8;
        uint64_t quad4_clip : 8;
        uint64_t quad3_clip : 8;
        uint64_t quad2_clip : 8;
        uint64_t quad1_clip : 8;
        uint64_t quad0_clip : 8;
        uint64_t clip_type: 8;
        uint64_t msg_id : 8;
#endif
    } fields;
} pgpe_db0_clip_bcast_t;

//
//PGPE-CME Doorbell0(PMSR Update)
//
typedef union pgpe_db0_pmsr_updt
{
    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint64_t msg_id : 8;
        uint64_t command : 8;
        uint64_t reserved: 48;
#else
        uint64_t reserved: 48;
        uint64_t command : 8;
        uint64_t msg_id : 8;
#endif
    } fields;
} pgpe_db0_pmsr_updt_t;

//
//PGPE-CME Doorbell0(Pstate Abort)
//
typedef union pgpe_db0_pstate_register_done
{
    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint64_t msg_id : 8;
        uint64_t reserved: 56;
#else
        uint64_t reserved: 56;
        uint64_t msg_id : 8;
#endif
    } fields;
} pgpe_db0_pstate_register_done_t;


#endif //__PSTATE_PGPE_CME_API_H__
