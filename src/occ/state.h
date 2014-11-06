/******************************************************************************
// @file state.h
// @brief OCC States (including Reset)
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section state.h STATE.H
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *   @th003             thallet   11/08/2011  New file
 *   @01                tapiar    11/10/2011  New state/mode macro/enums
 *   @rc003             rickylie  02/03/2012  Verify & Clean Up OCC Headers & Comments
 *   @pb00E             pbavari   03/11/2012  Added correct include file
 *   @th00d             thallet   04/23/2012  SMGR States, Structs & Functions
 *   @th011             thallet   07/12/2012  Split State/Mode/Reset into sep files
 *   @th022             thallet   10/03/2012  Changes to allow DCOM State/Mode setting
 *   @th036  881677     thallet   05/06/2013  Support for new poll command
 *   @th042  892056     thallet   07/19/2013  Send OCC to safe mode if first APSS GPE fails
 *   @gm006  SW224414   milesg    09/16/2013  Reset and FFDC improvements
 *   @gs011  900661     gjsilva   09/30/2013  Make data format 0x13 required to go active
 *   @gm013  907548     milesg    11/22/2013  Memory therm monitoring support
 *   @rt004  908817     tapiar    12/11/2013  Add APSS & THRM Threshold config data as required for OBS
 *                                            Add PCAP config data as required for Active 
 *   @gm016  909061     milesg    12/10/2013  Support memory throttling due to temperature
 *   @jh00b  910184     joshych   01/10/2014  Add check for checkstop
 *   @gm028  911670     milesg    02/27/2014  Immediate safe mode on checkstop
 *   @gm037  925908     milesg    05/07/2014  Redundant OCC/APSS support
 *
 *  @endverbatim
 *
 *///*************************************************************************/
#ifndef _state_h
#define _state_h

//*************************************************************************
// Includes
//*************************************************************************
//@pb00Ec - changed from common.h to occ_common.h for ODE support
#include <occ_common.h>
#include <common_types.h>
#include "rtls.h"
#include "errl.h"
#include "mode.h"

//*************************************************************************
// Externs
//*************************************************************************
extern uint32_t G_smgr_validate_data_active_mask;
extern uint32_t G_smgr_validate_data_observation_mask;
//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************
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
// <TULETA HW BRINGUP>
#define SMGR_VALIDATE_DATA_OBSERVATION_MASK_HARDCODES \
    (DATA_MASK_SYS_CNFG | \
     DATA_MASK_APSS_CONFIG | \
     DATA_MASK_SET_ROLE | \
     DATA_MASK_MEM_CFG | \
     DATA_MASK_THRM_THRESHOLDS )

// </TULETA HW BRINGUP>

#define SMGR_VALIDATE_DATA_ACTIVE_MASK  G_smgr_validate_data_active_mask
#define SMGR_VALIDATE_DATA_OBSERVATION_MASK G_smgr_validate_data_observation_mask

#define SMGR_VALIDATE_DATA_ACTIVE_MASK_HARDCODES \
    (SMGR_VALIDATE_DATA_OBSERVATION_MASK_HARDCODES | \
     DATA_MASK_PSTATE_SUPERSTRUCTURE | \
     DATA_MASK_FREQ_PRESENT | \
     DATA_MASK_PCAP_PRESENT )


//*************************************************************************
// Macros
//*************************************************************************

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


//*************************************************************************
// Structures
//*************************************************************************

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


//*************************************************************************
// Globals
//*************************************************************************
extern OCC_STATE          G_occ_internal_state;
extern OCC_STATE          G_occ_internal_req_state;
extern SMGR_SMS_CMD_TYPE  G_occ_internal_sms; 
extern OCC_STATE          G_occ_master_state;         // @th022
extern OCC_STATE          G_occ_external_req_state;   // @th022


//*************************************************************************
// Function Prototypes
//*************************************************************************
// Used by macro above to clear flag indicating to not halt OCC when a reset
// is requested.
inline void reset_disable_halt(void);

// Used to see if anyone has requested reset/safe state
bool isSafeStateRequested(void);   // @th042

// Used by macros to request reset states extenally
void reset_state_request(uint8_t i_request);

// Task that will check for checkstop // @jh00bc
void task_check_for_checkstop(task_t *i_self);

// Used to set OCC State
errlHndl_t SMGR_set_state(OCC_STATE i_state);

// Used to indicate which OCC States are valid, given the config data/system
// parms we currently know.
uint8_t SMGR_validate_get_valid_states(void);

#endif

