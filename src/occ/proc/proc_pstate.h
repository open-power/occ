/******************************************************************************
// @file proc_pstate.h
// @brief OCC Firmware Handline of Pstates
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section _proc_pstate_h proc_pstate.h
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *   @th010             thallet   04/05/2012  Created
 *   @th015             thallet   08/03/2012  Removed inline from functions
 *   @at010  859992     alvinwan  11/07/2012  Added oversubscription feature
 *   @fk001  879727     fmkassem  04/16/2013  PCAP support. 
 *   @ly009  895318     lychen    08/13/2013  OCC-Sapphire shared memory interface
 *   @gm006  SW224414   milesg    09/16/2013  Reset and FFDC improvements 
 *   @ly010  908832     lychen    12/09/2013  Sapphire update status for reset
 *   @gm025  915973     milesg    02/14/2014  Full support for sapphire (KVM) mode
 *
 *  @endverbatim
 *
 *///*************************************************************************/

#ifndef PROC_PSTATE_H
#define PROC_PSTATE_H

//*************************************************************************
// Includes
//*************************************************************************
#include "ssx.h"		
#include "cmdh_service_codes.h" 
#include "errl.h"             
#include "trac.h"
#include "rtls.h"
#include "occ_common.h"
#include "state.h"
#include "cmdh_fsp_cmds.h"
#include "cmdhDbugCmd.h"
#include "appletManager.h"
#include "gpsm.h"
#include "pstates.h"


//*************************************************************************
// Defines/Enums
//*************************************************************************

// GPSM DCM Synchronization - used by MBOX to transfer between DCM M & S 
typedef struct
{
  uint8_t dcm_pair_id;
  uint8_t sync_state_master :4;
  uint8_t sync_state_slave :4;
  uint8_t pstate_v;
  uint8_t pstate_f;
} proc_gpsm_dcm_sync_occfw_t;

typedef struct __attribute__ ((packed))
{
    uint8_t              valid;
    uint8_t              version;
    // Sapphire_OCC_interface_v3.odp // @ly010c
    // throttle status 0x00 No throttle, 0x01 Powercap, 0x02 CPU overtemp, 0x03 power supply failure, 0x04 overcurrent, 0x05 OCC reset
    uint8_t              throttle;
    int8_t               pmin;
    int8_t               pnominal;
    int8_t               pmax;
    uint16_t             spare;
} sapphire_config_t; // @ly009a

typedef struct __attribute__ ((packed))
{
    int8_t               pstate;
    uint8_t              flag;
    uint8_t              evid_vdd;
    uint8_t              evid_vcs;
    uint32_t             freq_khz;
} sapphire_data_t; // @ly009a

#define PSTATE_ENTRY_NUMBER 256
// This size must be a multiple of 128
typedef struct __attribute__ ((packed))
{
    sapphire_config_t    config;
    uint64_t             reserved;
    sapphire_data_t      data[PSTATE_ENTRY_NUMBER];
    uint8_t              pad[112];
} sapphire_table_t __attribute__ ((aligned (128))); // @ly009a

enum {
    NO_THROTTLE = 0x00,
    POWERCAP = 0x01,
    CPU_OVERTEMP = 0x02,
    POWER_SUPPLY_FAILURE = 0x03,
    OVERCURRENT = 0x04,
    OCC_RESET = 0x05,
}; // @ly010a

//*************************************************************************
// Externs
//*************************************************************************
extern GlobalPstateTable G_global_pstate_table; // @at010a

extern uint32_t    G_mhz_per_pstate;            // @fk001a

extern sapphire_table_t G_sapphire_table;       // @ly009a

//*************************************************************************
// Functions
//*************************************************************************

// ---------------------------------
// Pstate Init Functions
// ---------------------------------

// Initialize PState Table
errlHndl_t proc_gpsm_pstate_initialize(const PstateSuperStructure* i_pss);

// Entry function for enabling Pstates once table is installed
void proc_gpsm_dcm_sync_enable_pstates_smh(void);


// ---------------------------------
// Pstate Init DCM Sync Functions
// ---------------------------------

// Get DCM Sync State 
proc_gpsm_dcm_sync_occfw_t proc_gpsm_dcm_sync_get_state(void);

// Pull down DCM pair's Sync State & Info via Mbox
void proc_gpsm_dcm_sync_update_from_mbox(proc_gpsm_dcm_sync_occfw_t * i_dcm_sync_state);

// ---------------------------------
// Pstate Related Helper Functions
// ---------------------------------

// Helper function to translate from Frequency to nearest Pstate
Pstate proc_freq2pstate(uint32_t i_freq_mhz);

// Helper function to translate from Pstate to nearest Frequency
uint32_t proc_pstate2freq(Pstate i_pstate);

// Helper function to determine if we are a DCM 
inline bool proc_is_dcm();

// Helper function to determine if we are in HW Pstate mode
inline bool proc_is_hwpstate_enabled(void);

// Copy pstate data to sapphire table
void populate_pstate_to_sapphire_tbl(); // @ly010a

// Copy sapphire table to mainstore memory at SAPPHIRE_OFFSET_IN_HOMER
void populate_sapphire_tbl_to_mem(); // @ly009a

// Check if sapphire table needs update
void proc_check_for_sapphire_updates();
#endif
