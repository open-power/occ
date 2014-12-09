/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/state.h $                                             */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2014                        */
/* [+] Google Inc.                                                        */
/* [+] International Business Machines Corp.                              */
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
#ifndef _state_h
#define _state_h

#include <occ_common.h>
#include <common_types.h>
#include "rtls.h"
#include "errl.h"
#include "mode.h"

extern uint32_t G_smgr_validate_data_active_mask;
extern uint32_t G_smgr_validate_data_observation_mask;

enum eResetStates
{
  RESET_NOT_REQUESTED = 0,
  NOMINAL_REQUESTED_DUE_TO_ERROR,
  RESET_REQUESTED_DUE_TO_ERROR,
};

/**
 * @enum OCC_STATE
 * @brief Typedef of the various states that TMGT can put OCC into.
 */
typedef enum
{
    OCC_STATE_NOCHANGE           = 0x00,
    OCC_STATE_STANDBY            = 0x01,
    OCC_STATE_OBSERVATION        = 0x02,
    OCC_STATE_ACTIVE             = 0x03,
    OCC_STATE_SAFE               = 0x04,
    OCC_STATE_RESET              = 0x05,  // Invalid State, TPMD Legacy
    OCC_STATE_STANDBYOBSERVATION = 0x06,  // Invalid State, TPMD Legacy

    // These are used for state transition table, and are not
    // a valid state in and of itself.
    OCC_STATE_ALL                = 0xFE,
    OCC_STATE_INVALID            = 0xFF,
} OCC_STATE;


/**
 * @enum SMGR_SMS_CMD_TYPE
 * @brief SMGR set-mode-state commands version 0 contains a byte that indicates if
 * the TPMD must actually change the voltage / frequency during a mode change.
 */
typedef enum
{
    SMGR_SMS_VF_INFO_ONLY         = 0x00,
    SMGR_SMS_VF_CHANGE_REQ        = 0x01,
    SMGR_SMS_STATIC_VF_CHANGE_REQ = 0x02,
} SMGR_SMS_CMD_TYPE;
extern SMGR_SMS_CMD_TYPE   G_occ_internal_sms;  // TODO:  Move to state.c

/**
 * @enum SMGR_SMS_STATUS_TYPE
 * @brief TMGT Poll contains a byte that indicates status based on this
 * bitmask
 */
#define     SMGR_MASK_MASTER_OCC        0x80    ///This is the master OCC
#define     SMGR_MASK_RESERVED_6        0x40    ///Reserved
#define     SMGR_MASK_RESERVED_5        0x20    ///Reserved
#define     SMGR_MASK_STATUS_REG_CHANGE 0x10    ///Change in status register
#define     SMGR_MASK_ATTN_ENABLED      0x08    ///Attentions to FSP are enabled
#define     SMGR_MASK_RESERVED_2        0x04    ///Reserved
#define     SMGR_MASK_OBSERVATION_READY 0x02    ///Observation Ready
#define     SMGR_MASK_ACTIVE_READY      0x01    ///Active Ready

/**
 * @enum SMGR_VALIDATE_STATES
 * @brief Config Data Formats needed from TMGT to trans. between states
 *
 */

#define SMGR_VALIDATE_DATA_OBSERVATION_MASK_HARDCODES \
    (DATA_MASK_SYS_CNFG | \
     DATA_MASK_APSS_CONFIG | \
     DATA_MASK_SET_ROLE | \
     DATA_MASK_MEM_CFG | \
     DATA_MASK_THRM_THRESHOLDS )

#define SMGR_VALIDATE_DATA_ACTIVE_MASK  G_smgr_validate_data_active_mask
#define SMGR_VALIDATE_DATA_OBSERVATION_MASK G_smgr_validate_data_observation_mask

#define SMGR_VALIDATE_DATA_ACTIVE_MASK_HARDCODES \
    (SMGR_VALIDATE_DATA_OBSERVATION_MASK_HARDCODES | \
     DATA_MASK_PSTATE_SUPERSTRUCTURE | \
     DATA_MASK_FREQ_PRESENT | \
     DATA_MASK_PCAP_PRESENT )


// Used by OCC FW to request an OCC Reset because of an error.
// It's the action flag that actually requests the reset.
// Severity will be set to UNRECOVERABLE if it is INFORMATIONAL.
#define REQUEST_RESET(error_log) \
{\
    reset_state_request(RESET_REQUESTED_DUE_TO_ERROR);\
    setErrlActions(error_log, ERRL_ACTIONS_RESET_REQUIRED);\
    commitErrl(&error_log);\
}

#define REQUEST_SAFE_MODE(error_log) \
{\
    reset_state_request(RESET_REQUESTED_DUE_TO_ERROR);\
    setErrlActions(error_log, ERRL_ACTIONS_SAFE_MODE_REQUIRED);\
    commitErrl(&error_log);\
}


// Used by OCC FW to request that OCC go to Nominal because of an error
#define REQUEST_NOMINAL() reset_state_request(NOMINAL_REQUESTED_DUE_TO_ERROR);

// Used by OCC FW to signify that OCC can leave Nominal state because the
// error that caused OCC to go to Nominal has cleared.
#define CLEAR_NOMINAL()   reset_state_request(RESET_NOT_REQUESTED);

// Used to indicate that OCC has established TMGT Comm, and should no longer
// halt() on a reset request.
#define DISABLE_HALT_ON_RESET_REQUEST() reset_disable_halt();

// Returns the current OCC State
#define CURRENT_STATE()  G_occ_internal_state

// Returns the 'OCC Requested' OCC State
#define REQUESTED_STATE()  G_occ_internal_req_state

// Returns true if OCC State is active
#define IS_OCC_STATE_ACTIVE()  ( (OCC_STATE_ACTIVE == G_occ_internal_state)? 1 : 0 )

/**
 * @struct smgr_state_trans_t
 * @brief Used by the "Set State" command to call the correct transition
 * function, based on the current & new states.
 */
typedef struct
{
  uint8_t old_state;
  uint8_t new_state;
  errlHndl_t (*trans_func_ptr)(void);
} smgr_state_trans_t;


extern OCC_STATE          G_occ_internal_state;
extern OCC_STATE          G_occ_internal_req_state;
extern SMGR_SMS_CMD_TYPE  G_occ_internal_sms;
extern OCC_STATE          G_occ_master_state;
extern OCC_STATE          G_occ_external_req_state;

// Used by macro above to clear flag indicating to not halt OCC when a reset
// is requested.
inline void reset_disable_halt(void);

// Used to see if anyone has requested reset/safe state
bool isSafeStateRequested(void);

// Used by macros to request reset states extenally
void reset_state_request(uint8_t i_request);

// Task that will check for checkstop
void task_check_for_checkstop(task_t *i_self);

// Used to set OCC State
errlHndl_t SMGR_set_state(OCC_STATE i_state);

// Used to indicate which OCC States are valid, given the config data/system
// parms we currently know.
uint8_t SMGR_validate_get_valid_states(void);

#endif

