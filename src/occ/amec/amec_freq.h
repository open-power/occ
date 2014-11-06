/******************************************************************************
// @file amec_freq.h
// @brief AMEC frequency header file
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section _amec_freq_h amec_freq.h
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *   @th015             thallet   08/03/2012  New file
 *   @gs001             gsilva    08/03/2012  New file
 *   @ry002  862116     ronda     12/06/2012  Added enum to store voting box reason codes 
 *   @ry003  870734     ronda     02/20/2013  Thermal controller for memory 
 *   @ry004  872358     ronda     02/27/2013  Added memory voting box
 *   @fk001  879727     fmkassem  04/16/2013  Power capping support.  
 *   @gm004  892961     milesg    07/25/2013  Support memory auto slewing
 *   @rt001  897459     tapiar    08/21/2013  Update amec voting enums to a bitmaskSupport memory auto slewing
 *   @gs014  903552     gjsilva   10/22/2013  Support for Amester parameter interface
 *   @gs015  905166     gjsilva   11/04/2013  Full support for IPS function
 *   @wb004  922138     wilbryan  04/03/2014  Ensure timely pstate completion
 *
 *  @endverbatim
 *
 *///*************************************************************************/
#ifndef _AMEC_FREQ_H
#define _AMEC_FREQ_H

//*************************************************************************
// Includes
//*************************************************************************
#include <occ_common.h>
#include <ssx.h>
#include "occ_sys_config.h"
#include <ssx_app_cfg.h>
#include <amec_smh.h>
#include <mode.h>
#include <errl.h>

//*************************************************************************
// Externs
//*************************************************************************
// >> gitprep
extern const uint32_t G_pmc_ffdc_scom_addrs[4];
extern const uint32_t G_pmc_ffdc_oci_addrs[29];
// << gitprep
//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************
#define FREQ_CHG_CHECK_TIME 4000 // @wb004

// This is used by the Frequency State Machine
typedef enum
{
    AMEC_CORE_FREQ_IDLE_STATE    = 0x00,
    AMEC_CORE_FREQ_PROCESS_STATE = 0x01,
}amec_freq_write_state_t;

// This is reason code used by Voting box amec_slv_voting_box
typedef enum
{
    // @rt001c
    AMEC_VOTING_REASON_INIT             = 0x00000000,
    AMEC_VOTING_REASON_PHYP             = 0x00000001,
    AMEC_VOTING_REASON_PLPM             = 0x00000002,
    AMEC_VOTING_REASON_LEGACY           = 0x00000004,
    AMEC_VOTING_REASON_SOFT_MIN         = 0x00000008,
    AMEC_VOTING_REASON_SOFT_MAX         = 0x00000010,   
    AMEC_VOTING_REASON_CORE_CAP         = 0x00000020,
    AMEC_VOTING_REASON_PROC_THRM        = 0x00000040,
    AMEC_VOTING_REASON_GXHB_THRM        = 0x00000080,
    AMEC_VOTING_REASON_VRHOT_THRM       = 0x00000100,
    AMEC_VOTING_REASON_OVER_CURRENT     = 0x00000200,
    AMEC_VOTING_REASON_OVERRIDE         = 0x00000400,
    AMEC_VOTING_REASON_CORE_GRP_MIN     = 0x00000800,    
    AMEC_VOTING_REASON_PWR              = 0x00001000, // @fk001a
    AMEC_VOTING_REASON_PPB              = 0x00002000,
    AMEC_VOTING_REASON_PMAX             = 0x00004000,
    AMEC_VOTING_REASON_UTIL             = 0x00008000,
    AMEC_VOTING_REASON_CONN_OC          = 0x00010000,
    AMEC_VOTING_REASON_OVERRIDE_CORE    = 0x00020000,
    AMEC_VOTING_REASON_IPS              = 0x00040000,
}amec_freq_voting_reason_t;

// rt001a
#define NON_DPS_POWER_LIMITED ( AMEC_VOTING_REASON_PWR | \
                                AMEC_VOTING_REASON_PPB | \
                                AMEC_VOTING_REASON_PMAX  \
                              )


// rt001a
extern BOOLEAN G_non_dps_power_limited;

// This is reason code used by Voting box amec_slv_mem_voting_box
typedef enum
{
    AMEC_MEM_VOTING_REASON_INIT         = 0,
    AMEC_MEM_VOTING_REASON_CENT         = 1,
    AMEC_MEM_VOTING_REASON_DIMM         = 2,
    AMEC_MEM_VOTING_REASON_SLEW         = 3,   //gm004
}amec_mem_voting_reason_t;

//*************************************************************************
// Structures
//*************************************************************************

#define PMC_FFDC_OCI_ADDRS_SIZE sizeof(G_pmc_ffdc_oci_addrs) // @wb004
#define PMC_FFDC_SCOM_ADDRS_SIZE sizeof(G_pmc_ffdc_scom_addrs) // @wb004

// @wb004 -- Moved the following three structures for accessibility

// scom ffdc format
typedef struct __attribute__ ((packed))
{
    uint32_t    addr;
    uint64_t    data;
} pmc_ffdc_scom_entry_t;

// OCI ffdc format
typedef struct
{
    uint32_t    addr;
    uint32_t    data;
} pmc_ffdc_oci_entry_t;

//PMC FFDC format for user detail section of error log -- gm031
typedef struct
{
    pmc_ffdc_oci_entry_t oci_regs[PMC_FFDC_OCI_ADDRS_SIZE/sizeof(uint32_t)];
    pmc_ffdc_scom_entry_t scom_regs[PMC_FFDC_SCOM_ADDRS_SIZE/sizeof(uint32_t)];
} pmc_ffdc_data_t;

// @wb004
typedef struct
{
    pcbs_power_management_status_reg_t    data[MAX_NUM_CORES];
} pmsr_ffdc_data_t;

// @wb004
typedef struct
{
    pmc_ffdc_data_t     pmc_ffcdc_data;
    pmsr_ffdc_data_t     pmsr_ffdc_data;
} pmc_pmsr_ffcdc_data_t;

//*************************************************************************
// Globals
//*************************************************************************

//*************************************************************************
// Function Prototypes
//*************************************************************************
void fill_pmc_ffdc_buffer(pmc_ffdc_data_t* i_ffdc_ptr); // @wb004

//*************************************************************************
// Functions
//*************************************************************************

// Used to set the freq range that amec can control between.
errlHndl_t amec_set_freq_range(const OCC_MODE i_mode);

// Voting box for handling slave freq votes
void amec_slv_voting_box(void);

// Amec Frequency State Machine
void amec_slv_freq_smh(void);

// Voting box for handling slave memory freq votes
void amec_slv_mem_voting_box(void);

// Amec Detect and log degraded performance errors
void amec_slv_check_perf(void);

// Verifies that each core is at the correct frequency
void amec_verify_pstate();


#endif

