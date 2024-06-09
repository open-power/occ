/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/errl/errl.h $                                     */
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

#ifndef _ERRL_H
#define _ERRL_H

#include <occ_common.h>
#include <trac_interface.h>
#include <occ_hcode_errldefs.h>
#include <pstate_pgpe_occ_api.h>

// Used as default for invalid slot number
static const uint8_t  ERRL_INVALID_SLOT = 0xFF;

// Used for shifting slot bits
static const uint32_t ERRL_SLOT_SHIFT = 0x80000000;

// Used for defaulting handle to invalid
static const uint32_t INVALID_ERR = 0xFFFFFFFF;

// Max size of non call home data logs (3616 bytes)
#define MAX_ERRL_ENTRY_SZ 0xE20

// Max size of call home data log (8192 bytes)
#define MAX_ERRL_CALL_HOME_SZ 0x2000

// Max number of error logs (6 normal and 1 call-home)
#define ERRL_MAX_SLOTS 7

// Used to default a old/bad error handle
#define INVALID_ERR_HNDL (errlHndl_t)INVALID_ERR

// USED to determine the number of all trace buffer types. Now have (INF/IMP/ERR)
#define NUM_OF_TRACE_TYPE 3

// maximum size of PGPE debug data will be added for ERRL_USR_DTL_PGPE_PK_TRACE
#define MAX_PGPE_DBUG_DATA 0x3E0

/* Slot Masks */
// These bits are used to acquire a slot number.  When used with the global
// slot bit mask, we are able to get 6 slots for any OCC/PGPE errors and
// one slot for call home data log
typedef enum
{
    ERRL_SLOT_MASK_DEFAULT        = 0xFFFFFFFF,
    ERRL_SLOT_MASK_OCC_ERROR      = 0x03FFFFFF, // first 6 slots
    ERRL_SLOT_MASK_CALL_HOME_DATA = 0xFDFFFFFF  // last slot
} ERRL_SLOT_MASK;


extern uint32_t     G_occErrSlotBits;
extern uint8_t      G_occErrIdCounter;

extern errlHndl_t   G_occErrSlots[ERRL_MAX_SLOTS];

extern hcode_elog_entry_t *G_hcode_elog_table;
extern uint32_t            G_hcode_elog_table_slots;

typedef enum {
    ERRH_NO_VALID_OCMB_DTS           =  0x01,
    // unused -  0x02,
    // unused -  0x03,
    // unused -  0x04,
    // unused - 0x05
    // unused - 0x06
    ERRH_AVSBUS_VDD_STATUS_READ_FAIL =  0x07,
    // unused -  0x08,
    ERRH_AVSBUS_VDD_OVER_CURRENT     =  0x09,
    // unused -  0x0A,
    ERRH_INVALID_APSS_DATA           =  0x0B,
    ERRH_APSS_COMPLETE_ERROR         =  0x0C,
    ERRH_APSS_COMPLETE_TIMEOUT       =  0x0D,
    ERRH_DCOM_TX_SLV_INBOX           =  0x0E,
    ERRH_DCOM_RX_SLV_INBOX           =  0x0F,
    ERRH_DCOM_TX_SLV_OUTBOX          =  0x10,
    ERRH_DCOM_RX_SLV_OUTBOX          =  0x11,
    ERRH_DCOM_MASTER_PBAX_SEND_FAIL  =  0x12,
    ERRH_DCOM_SLAVE_PBAX_SEND_FAIL   =  0x13,
    ERRH_DCOM_MASTER_PBAX_READ_FAIL  =  0x14,
    ERRH_DCOM_SLAVE_PBAX_READ_FAIL   =  0x15,
    ERRH_GPE0_NOT_IDLE               =  0x16,
    ERRH_GPE1_NOT_IDLE               =  0x17,
    ERRH_24X7_DISABLED               =  0x18,
    ERRH_CEFF_RATIO_VDD_EXCURSION    =  0x19,  // OCS Dirty+type '1' (Act)
    ERRH_AVSBUS_VDD_TEMPERATURE      =  0x1A,
    ERRH_OVER_PCAP_IGNORED           =  0x1B,
    ERRH_VRT_TIMEOUT_IGNORED         =  0x1C,
    ERRH_WOF_CONTROL_TIMEOUT_IGNORED =  0x1D,
    ERRH_PSTATE_CHANGE_IGNORED       =  0x1E,
    ERRH_VDD_CURRENT_ROLLOVER_MAX    =  0x1F,
    ERRH_CORE_SMALL_DROOP            =  0x20,
    ERRH_CACHE_LARGE_DROOP           =  0x21,
    ERRH_OCS_DIRTY_BLOCK             =  0x22,  // OCS Dirty+type '0' (block)
    ERRH_RTL_TIME_EXCEEDED           =  0x23,
    ERRH_DCOM_RX_SLV_INBOX_INCOMPLETE =  0x24,
    ERRH_DCOM_RX_SLV_LOST_CONNECTION =  0x25,
    ERRH_24x7_NOT_IDLE               =  0x26,
    ERRH_RESERVED27                  =  0x27,
    ERRH_RESERVED28                  =  0x28,
    ERRH_RESERVED29                  =  0x29,
    ERRH_RESERVED2A                  =  0x2A,
    ERRH_RESERVED2B                  =  0x2B,
    ERRH_RESERVED2C                  =  0x2C,
    ERRH_RESERVED2D                  =  0x2D,
    ERRH_RESERVED2E                  =  0x2E,
    ERRH_RESERVED2F                  =  0x2F,
    ERR_HISTORY_SIZE                 =  0x30
} ERR_HISTORY_INDEX;

// Array of error counters that are only cleared on OCC reset
extern uint8_t G_error_history[ERR_HISTORY_SIZE];
#define INCREMENT_ERR_HISTORY(errorIndex) { \
    if ((errorIndex < ERR_HISTORY_SIZE) && (G_error_history[errorIndex] < 255)) { \
        ++G_error_history[errorIndex]; \
    } \
}

// Globals used by testcases
extern uint8_t      G_errslot1[MAX_ERRL_ENTRY_SZ];
extern uint8_t      G_errslot2[MAX_ERRL_ENTRY_SZ];
extern uint8_t      G_errslot3[MAX_ERRL_ENTRY_SZ];
extern uint8_t      G_errslot4[MAX_ERRL_ENTRY_SZ];
extern uint8_t      G_errslot5[MAX_ERRL_ENTRY_SZ];
extern uint8_t      G_errslot6[MAX_ERRL_ENTRY_SZ];
extern uint8_t      G_callslot[MAX_ERRL_CALL_HOME_SZ];

/* Create an Error Log */
errlHndl_t createErrl(
            const uint16_t i_modId,
            const uint8_t i_reasonCode,
            const uint16_t i_extReasonCode,
            const ERRL_SEVERITY i_sev,
            const trace_descriptor_array_t* i_trace,
            const uint16_t i_traceSz,
            const uint32_t i_userData1,
            const uint32_t i_userData2
            );

/* Create an Error Log due to a PGPE failure */
errlHndl_t createPgpeErrl(
            const uint16_t i_modId,
            const uint8_t i_reasonCode,
            const uint16_t i_extReasonCode,
            const ERRL_SEVERITY i_sev,
            const uint32_t i_userData1,
            const uint32_t i_userData2
            );

/* Add Trace Data to Error Log */
void addTraceToErrl(
            const trace_descriptor_array_t* i_trace,
            const uint16_t i_traceSz,
            errlHndl_t io_errl
            );

// Add Error history data to the Error Log
void addErrHistory(errlHndl_t io_err);

// Add PGPE specific data to the Error Log
void addPgpeDataToErrl(errlHndl_t io_err);

/* Commit the Error Log */
void commitErrl( errlHndl_t * io_err );

/* Delete the Error Log */
errlHndl_t deleteErrl( errlHndl_t * io_err);

/* Add Callout to Error Log */
void addCalloutToErrl(
            errlHndl_t io_err,
            const ERRL_CALLOUT_TYPE i_type,
            const uint64_t i_calloutValue,
            const ERRL_CALLOUT_PRIORITY i_priority);

/* Add User Details Data to the Error Log */
void addUsrDtlsToErrl(
            errlHndl_t io_err,
            uint8_t *i_dataPtr,
            const uint16_t i_size,
            const uint8_t i_version,
            const ERRL_USR_DETAIL_TYPE i_type);

/* Change Severity of Error Log to Informational */
void setErrlSevToInfo( errlHndl_t io_err );

/* Set Actions to an Errl */
void setErrlActions(errlHndl_t io_err, const uint8_t i_mask);

/* Return Error Log ID to report to TMGT */
uint8_t getErrlLogId( errlHndl_t io_err );

/* Get Error Slot Num By Error Id */
uint8_t getErrSlotNumByErrId( uint8_t  i_errlId);

/* Get Error Slot OCI address */
uint32_t getErrSlotOCIAddr(const uint8_t  i_SlotNum);

/* Get the oldest error log ID so that we can pass it to TMGT */
uint8_t getOldestErrlID();

/* Return Length of an Error Log based on ID, to send to TMGT */
uint16_t getErrlLengthByID(const uint8_t i_id);

/* Return Address of an Error Log based on ID, to send to TMGT */
uint32_t getErrlOCIAddrByID(const uint8_t i_id);


#endif //_ERRL_H
