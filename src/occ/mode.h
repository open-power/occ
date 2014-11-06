/******************************************************************************
// @file mode.h
// @brief OCC Modes
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section state.h STATE.H
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *   @th011             thallet   07/13/2011  New file
 *   @th015             thallet   08/03/2012  Added 2 macros
 *   @th022             thallet   10/03/2012  Dcom State/Mode changes
 *   @th036  881677     thallet   05/06/2013  Support for new Poll Command
 *   @jh004  889884     joshych   07/24/2013  Support CPM param and updated frequency packet
 *   @fk002  905632     fmkassem  11/05/2013  Remove CriticalPathMonitor code
 *   @at019  908390     alvinwan  12/05/2013  Disable DPS algorithms from running in Sapphire
 *
 *  @endverbatim
 *
 *///*************************************************************************/
#ifndef _mode_h
#define _mode_h

//*************************************************************************
// Includes
//*************************************************************************
#include <occ_common.h>
#include <common_types.h>
#include "rtls.h"
#include "errl.h"

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Macros
//*************************************************************************

// Returns the current OCC Mode 
#define CURRENT_MODE() G_occ_internal_mode

// Returns the 'OCC Requested' OCC Mode 
#define REQUESTED_MODE()  G_occ_internal_req_mode

// Returns the 'Requested' SMS Mode 
#define CURRENT_SMS()  G_occ_internal_sms

// Returns the 'Requested' SMS Mode 
#define VALID_MODE(mode)  ((mode < OCC_MODE_COUNT) ? 1 : 0)


//*************************************************************************
// Defines/Enums
//*************************************************************************

/**
 * @enum OCC_MODE
 * @brief Typedef of the various modes that TMGT can put OCC into.
 */
typedef enum
{
    OCC_MODE_NOCHANGE          = 0x00,
    OCC_MODE_NOMINAL           = 0x01,
    // OCC_MODE_BENCHMARK         = 0x02,
    OCC_MODE_TURBO             = 0x03,
    OCC_MODE_SAFE              = 0x04,
    OCC_MODE_PWRSAVE           = 0x05,
    OCC_MODE_DYN_POWER_SAVE    = 0x06,
    OCC_MODE_MIN_FREQUENCY     = 0x07,

    // Mode 0x08-0x0A reserved by TMGT
    // OCC_MODE_SAFE_LOW_POWER    = 0x09, 
    OCC_MODE_DYN_POWER_SAVE_FP = 0x0A, 
    OCC_MODE_FFO               = 0x0B,
    OCC_MODE_STURBO            = 0x0C,

    // Make sure this is after the last valid mode
    OCC_MODE_COUNT,

    // These are used for mode transition table, and are not
    // a valid mode in and of itself.
    OCC_MODE_ALL              = 0xFE,
    OCC_MODE_INVALID          = 0xFF,
} OCC_MODE;

/**
 * @enum OCC_INTERNAL_MODE
 * @brief Typedef of the various internal modes that OCC can be in.
 */
typedef enum
{
    OCC_INTERNAL_MODE_NOM          = 0x00,
    OCC_INTERNAL_MODE_SPS          = 0x01,
    OCC_INTERNAL_MODE_DPS          = 0x02,
    OCC_INTERNAL_MODE_DPS_MP       = 0x03,
    OCC_INTERNAL_MODE_FFO          = 0x04,
    OCC_INTERNAL_MODE_MAX_NUM,
    OCC_INTERNAL_MODE_UNDEFINED    = 0xFF
} OCC_INTERNAL_MODE;


//*************************************************************************
// Globals
//*************************************************************************
extern OCC_MODE           G_occ_internal_mode;
extern OCC_MODE           G_occ_internal_req_mode;
extern OCC_MODE           G_occ_external_req_mode;   // @th022
extern OCC_MODE           G_occ_external_req_mode_kvm; // @at019a
extern OCC_MODE           G_occ_master_mode;         // @th022

//*************************************************************************
// Functions
//*************************************************************************

// Returns true if we are in the middle of a mode transition
inline bool SMGR_is_mode_transitioning(void);

// Used to get the OCC Mode
inline OCC_MODE SMGR_get_mode(void);

// Used to set OCC Mode
errlHndl_t SMGR_set_mode(const OCC_MODE i_mode,
                         const uint8_t i_sms_type);





#endif 
