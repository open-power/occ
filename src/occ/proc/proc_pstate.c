/******************************************************************************
// @file proc_pstate.c
// @brief OCC Handling of Pstates
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section _proc_pstate_c proc_pstate.c
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *   @th010             thallet   07/10/2012  Created
 *   @th014             thallet   08/02/2012  Updated PstateSS & TODO flags added
 *   @th015             thallet   08/03/2012  Bounds checking on freq2pstate
 *   @th025  857856     thallet   10/16/2012  Dcom Master Slave SMS part 2
 *   @th032             thallet   04/26/2013  Tuleta HW Bringup Changes   
 *   @th035  881654     thallet   05/06/2013  Tuleta Bringup Pstate Fixes
 *   @th036  881677     thallet   05/07/2013  Cleanup
 *   @th040  887069     thalllet  06/11/2013  Support Nom & FFO Freq Setting for Mnfg
 *   @fk001  879727     fmkassem  04/16/2013  PCAP support. 
 *   @ly009  895318     lychen    08/13/2013  OCC-Sapphire shared memory interface
 *   @gm006  SW224414   milesg    09/16/2013  Reset and FFDC improvements 
 *   @gm011  903410     milesg    10/22/2013  Fail on murano dd10 or dd11 chips
 *   @ly010  908832     lychen    12/09/2013  Sapphire update status for reset
 *   @gm022  908890     milesg    01/23/2014  Fixed pstate table traces
 *   @gm025  915973     milesg    02/14/2014  Full support for sapphire (KVM) mode
 *   @wb001  919163     wilbryan  03/06/2014  Updating error call outs, descriptions, and severities
 *   @wb003  920760     wilbryan  03/25/2014  Update SRCs to match TPMD SRCs
 *   @gm033  920448     milesg    03/26/2014  use getscom/putscom ffdc wrapper
 *   @gs032  925755     gjsilva   05/07/2014  Optimize freq2pstate function
 *   @gs033  929049     gjsilva   06/13/2014  Fix computation of pstate
 *   @gs044  943766     gjsilva   11/03/2014  Clear bit 11 of PCBS_PMGP1_REG during KVM setup
 *
 *  @endverbatim
 *
 *///*************************************************************************/
 
//*************************************************************************
// Includes
//*************************************************************************
#include "ssx.h"		
#include "proc_data_service_codes.h" 
#include "errl.h"             
#include "trac.h"
#include "rtls.h"
#include "dcom.h"  
#include "occ_common.h"
#include "state.h"
#include "cmdh_fsp_cmds.h"
#include "cmdhDbugCmd.h"
#include "appletManager.h"
#include "gpsm.h"
#include "pstates.h"
#include "proc_data.h"
#include "proc_pstate.h"
#include "scom.h"

//*************************************************************************
// Defines/Enums
//*************************************************************************
// GPSM DCM Synchronization States
typedef enum 
{
  PROC_GPSM_SYNC_NO_PSTATE_TABLE        = 0,
  PROC_GPSM_SYNC_PSTATE_TABLE_INSTALLED = 1,
  PROC_GPSM_SYNC_READY_TO_ENABLE_MASTER = 2,
  PROC_GPSM_SYNC_PSTATE_MASTER_ENABLED  = 3,
  PROC_GPSM_SYNC_READY_TO_ENABLE_SLAVE  = 4,
  PROC_GPSM_SYNC_PSTATE_SLAVE_ENABLED   = 5,
  PROC_GPSM_SYNC_PSTATE_HW_MODE         = 6,
  PROC_GPSM_SYNC_PSTATE_HW_MODE_ENABLED = 7,
  PROC_GPSM_SYNC_PSTATE_ERROR,
} eProcGpsmDcmSyncStates;


//*************************************************************************
// Macros
//*************************************************************************



//*************************************************************************
// Globals 
//*************************************************************************

// Instance of the PState Table in OCC SRAM.  Should be placed in RO section 
// so that OCC FW can't corrupt it
GLOBAL_PSTATE_TABLE(G_global_pstate_table);

// Used for passing DCM Master & Slave States to each other over MBOX
proc_gpsm_dcm_sync_occfw_t G_proc_dcm_sync_state = {0, PROC_GPSM_SYNC_NO_PSTATE_TABLE, PROC_GPSM_SYNC_NO_PSTATE_TABLE,0,0};

// Holds Fmax from GPST for ease of proc_freq2pstate calculation
uint32_t G_proc_gpst_fmax = 0;

// Holds Fmin from GPST for ease of proc_freq2pstate calculation
uint32_t G_proc_gpst_fmin = 0;

// Holds Pmax from GPST for ease of proc_freq2pstate calculation
int8_t   G_proc_gpst_pmax = 0;

// Remembers if we are a DCM, for DCOM's sake
bool G_isDcm      = FALSE;

// Used for Sapphire
DMA_BUFFER( sapphire_table_t G_sapphire_table ) = {{0}}; // @ly009a

//KVM throttle reason coming from the frequency voting box.
extern uint8_t G_amec_kvm_throt_reason;

//*************************************************************************
// Forward Declarations
//*************************************************************************

// Set DCM Sync State
void proc_gpsm_dcm_sync_set_state(eProcGpsmDcmSyncStates i_dcm_sync_state);

// Tracing out pstate table when it gets installed
void proc_trace_pstate_table_quick(void);

//*************************************************************************
// Functions
//*************************************************************************


// Function Specification
//
// Name:  proc_is_hwpstate_enabled
//
// Description:  Checks DCM Master (or SCM) state to see if Pstate HW Mode 
//               is enabled.  We can check the DCM master state, since DCM 
//               slave also knows the master and DCM master can't be in this 
//               state if DCM slave isn't in HW mode
//              
//
// Flow:  xx/xx/xx    FN=
//
// End Function Specification
bool proc_is_hwpstate_enabled(void)
{
    return ((PROC_GPSM_SYNC_PSTATE_HW_MODE_ENABLED == G_proc_dcm_sync_state.sync_state_master) ? TRUE : FALSE);
}


// Function Specification
//
// Name:  proc_gpsm_dcm_sync_update_from_mbox
//
// Description:  Updates the global variable used for DCM sync based on the 
//               data that was received via the master/slave mailbox.
//              
// Thread:  Interrupt; Callback when Slave Inbox is received 
//
// Flow:  xx/xx/xx    FN=
//
// End Function Specification
void proc_gpsm_dcm_sync_update_from_mbox(proc_gpsm_dcm_sync_occfw_t * i_dcm_sync_state)
{
    if(!gpsm_dcm_slave_p())
    {
        G_proc_dcm_sync_state.sync_state_slave = i_dcm_sync_state->sync_state_slave;
    }
    else
    {
        G_proc_dcm_sync_state.sync_state_master = i_dcm_sync_state->sync_state_master;
        G_proc_dcm_sync_state.pstate_v = i_dcm_sync_state->pstate_v;
        G_proc_dcm_sync_state.pstate_f = i_dcm_sync_state->pstate_f;
    }
}


// Function Specification
//
// Name:  proc_gpsm_dcm_sync_get_state
//
// Description:  Return the global variable used for DCM sync
//              
//              
//              
//
// Flow:  xx/xx/xx    FN=
//
// End Function Specification
inline proc_gpsm_dcm_sync_occfw_t proc_gpsm_dcm_sync_get_state(void)
{
  return G_proc_dcm_sync_state;
}


// Function Specification
//
// Name:  proc_is_dcm
//
// Description:  Return if we are a DCM or not
//              
//              
//              
//
// Flow:  xx/xx/xx    FN=
//
// End Function Specification
inline bool proc_is_dcm(void)
{
  return G_isDcm;  
}


// Function Specification
//
// Name:  proc_gpsm_dcm_sync_set_state
//
// Description:  Set the state of global variable used for DCM sync
//               Differnt nybble will get set depending on if we are 
//               DCM Master or DCM Slave
//              
//
// Flow:  xx/xx/xx    FN=
//
// End Function Specification
inline void proc_gpsm_dcm_sync_set_state(eProcGpsmDcmSyncStates i_dcm_sync_state)
{
    if(!gpsm_dcm_slave_p())
    {
        G_proc_dcm_sync_state.sync_state_master = i_dcm_sync_state;
    }
    else
    {
        G_proc_dcm_sync_state.sync_state_slave = i_dcm_sync_state;
    } 
}


// Function Specification
//
// Name:  proc_gpsm_dcm_sync_get_state
//
// Description:  Return the state of global variable used for DCM sync
//              
//
// Flow:  xx/xx/xx    FN=
//
// End Function Specification
eProcGpsmDcmSyncStates proc_gpsm_dcm_sync_get_my_state(void)
{
    if(!gpsm_dcm_slave_p())
    {
        return G_proc_dcm_sync_state.sync_state_master;
    }
    else
    {
        return G_proc_dcm_sync_state.sync_state_slave;
    } 
}  // Added @th025


// Function Specification
//
// Name:  proc_trace_pstate_table_quick
//
// Description:  Debug Function to Print portion of Pstate Table 
//               Eventually, this should trace key elements of Pstate
//               table to Trace Buffer.
//              
//
// Flow:  xx/xx/xx    FN=
//
// End Function Specification
void proc_trace_pstate_table_quick(void)
{
    GlobalPstateTable * l_gpst_ptr = NULL;

    l_gpst_ptr = gpsm_gpst();
    // Check the pointer since it may not have been installed on chips with 0 configured cores -- gm011
    if(l_gpst_ptr == &G_global_pstate_table)
    {
        TRAC_IMP("GPST Installed:  Pstate[0]: %d kHz, Step: %d kHz, Entries: %d, Pvsafe[%d], Psafe[%d]",
                l_gpst_ptr->pstate0_frequency_khz, 
                l_gpst_ptr->frequency_step_khz,
                (int8_t) l_gpst_ptr->entries,
                (int8_t) l_gpst_ptr->pvsafe, 
                (int8_t) l_gpst_ptr->psafe 
                );

        TRAC_IMP("Pmin[%d]: %d kHz, Pmax[%d]: %d kHz",
                (int8_t) l_gpst_ptr->pmin, 
                (l_gpst_ptr->pstate0_frequency_khz + ((int8_t) l_gpst_ptr->pmin) * l_gpst_ptr->frequency_step_khz),
                ((int8_t) l_gpst_ptr->pmin + l_gpst_ptr->entries - 1),
                (l_gpst_ptr->pstate0_frequency_khz + ((int8_t) l_gpst_ptr->pmin + l_gpst_ptr->entries - 1) * l_gpst_ptr->frequency_step_khz)
                );
    }
    else //gm011
    {
        //This likely means that the processor has no configured cores (may not be an error scenario)
        TRAC_IMP("GPST not installed.  hw pointer= 0x%08x, present cores= 0x%08x", (uint32_t)l_gpst_ptr, G_present_cores);
    }
}


// Function Specification
//
// Name:  proc_pstate2freq
//
// Description:  Convert Pstate to Frequency
//              
//              
//              
//
// Flow:  xx/xx/xx    FN=
//
// End Function Specification
uint32_t proc_pstate2freq(Pstate i_pstate)
{
    uint32_t l_freq = 0;
    int8_t l_pmax = 0;
    GlobalPstateTable * l_gpst_ptr = NULL;

    do
    {
        // Get pointer to Pstate table
        l_gpst_ptr = gpsm_gpst();

        // Return the zero frequency if we don't have a PstateTable installed
        if(&G_global_pstate_table != l_gpst_ptr)
        {
            l_freq = 0;
            break;
        }

        // Calculate Pmax, since it is derived instead of being a explicit member
        l_pmax = ((int8_t) l_gpst_ptr->pmin + l_gpst_ptr->entries - 1);

        if(i_pstate < l_gpst_ptr->pmin)
        {
            // If passed in Pstate is lower than Pmin, just use Pmin
            i_pstate = l_gpst_ptr->pmin;
        }
        else if (i_pstate > l_pmax)
        {
            // If passed in Pstate is greater than Pmax, just use Pmax
            i_pstate = l_pmax;
        }

        // Calculate Frequency based on Pstate 
        l_freq = (l_gpst_ptr->pstate0_frequency_khz + ((int8_t) i_pstate) * l_gpst_ptr->frequency_step_khz);
        l_freq /= 1000;  // Convert to MHz
    }
    while(0);

    return l_freq;  // MHz
}


// Function Specification
//
// Name:  proc_freq2pstate
//
// Description:  Convert Frequency to Nearest Pstate
//              
//              
//              
//
// Flow:  xx/xx/xx    FN=
//
// End Function Specification
Pstate proc_freq2pstate(uint32_t i_freq_mhz)
{
    GlobalPstateTable * l_gpst_ptr = NULL;
    int8_t   l_pstate = PSTATE_MIN;
    int8_t   l_temp_pstate = 0;
    int32_t  l_temp_freq = 0;
    uint32_t l_freq_khz = 0;

    do
    {
        // Get pointer to Pstate table
        l_gpst_ptr = gpsm_gpst();

        // Return the minimum PState if we don't have a PstateTable installed
        if(&G_global_pstate_table != l_gpst_ptr)
        {
            break;
        }

        // Freq Units need to be in kHz, not Mhz for the following calculations
        l_freq_khz = i_freq_mhz * 1000;

        // Make sure that we don't ever get a frequency below the min Freq from
        // def file
        if(i_freq_mhz < G_sysConfigData.sys_mode_freq.table[OCC_MODE_MIN_FREQUENCY])
        {
            l_freq_khz =  G_sysConfigData.sys_mode_freq.table[OCC_MODE_MIN_FREQUENCY] * 1000;
        }

        if(l_freq_khz < G_proc_gpst_fmax)
        {
            // First, calculate the delta between passed in freq, and Pmin
            l_temp_freq = l_freq_khz - G_proc_gpst_fmin;

            // Check if the passed in frequency is smaller than anything in the
            // table
            if(l_freq_khz <= G_proc_gpst_fmin)
            {
                // We need to substract a full step (minus 1) to make sure we
                // are keeping things safe
                l_temp_freq -= (l_gpst_ptr->frequency_step_khz - 1); 
            }

            // Next, calculate how many Pstate steps there are in that delta
            l_temp_pstate = l_temp_freq / (int32_t)l_gpst_ptr->frequency_step_khz;

            // Lastly, calculate Pstate, by adding delta Pstate steps to Pmin
            l_pstate = l_gpst_ptr->pmin + l_temp_pstate;
        }
        else
        {
            // Freq is bigger than anything in table -- return Pmax
            l_pstate = G_proc_gpst_pmax;
        }
    }
    while(0);

    return (Pstate) l_pstate;
}


// Function Specification
//
// Name:   proc_gpsm_pstate_initialize
//
// Description:  Initialize Pstate Table (and the rest of the Pstate
//               SuperStructure).  Also, initialize Global variables 
//               that will speed up the proc_freq2pstate function.
//              
//
// Flow:  xx/xx/xx    FN=
//
// End Function Specification
errlHndl_t proc_gpsm_pstate_initialize(const PstateSuperStructure* i_pss)
{
    errlHndl_t l_errlHndl = NULL;
    GlobalPstateTable * l_gpst_ptr = NULL;
    int l_rc = 0;

    do
    {
        /// Because early EC's of the Murano chip did not have valid #V data,
        /// we need to exclude them from loading a pstate table created by a
        /// hardware procedure.  If we run a table created from a #V on these 
        /// chips, we could crash the box (or worse, burn something up!)
        if ( (cfam_id() == CFAM_CHIP_ID_MURANO_10) 
                || (cfam_id() == CFAM_CHIP_ID_MURANO_11) )
        {
            TRAC_ERR("OCC not supported on murano dd10 or dd11 due to bad #V data.  chip id = 0x%08x"); //gm011
            // Create Error Log and return to caller
            /* @
             * @errortype
             * @moduleid    PROC_GPST_INIT_FAILURE_MOD
             * @reasoncode  INTERNAL_FAILURE
             * @userdata1   chip id
             * @userdata2   0
             * @userdata4   OCC_NO_EXTENDED_RC
             * @devdesc     OCC not supported on Murano DD10 or DD11
             */
            l_errlHndl = createErrl(
                    PROC_GPST_INIT_FAILURE_MOD,         //modId
                    INTERNAL_FAILURE,                   //reasoncode // @wb003
                    OCC_NO_EXTENDED_RC,                 //Extended reason code
                    ERRL_SEV_UNRECOVERABLE,             //Severity
                    NULL,                               //Trace Buf
                    DEFAULT_TRACE_SIZE,                 //Trace Size
                    cfam_id(),                          //userdata1
                    0                                   //userdata2
                    );

            //callout the processor
            addCalloutToErrl(l_errlHndl,
                    ERRL_CALLOUT_TYPE_HUID,
                    G_sysConfigData.proc_huid,
                    ERRL_CALLOUT_PRIORITY_HIGH);
            break;
        }

        l_rc =  gpsm_initialize(i_pss, &G_global_pstate_table);

        // Print key elements of table for debug
        proc_trace_pstate_table_quick();

        // Get Pstate Table Ptr
        l_gpst_ptr = gpsm_gpst();

        if(l_rc || (l_gpst_ptr != &G_global_pstate_table)) //gm011
        {
            TRAC_ERR("gpsm_initialize failed with rc=0x%08x or  l_gpstr_ptr=0x%08x", l_rc, l_gpst_ptr); 

            // Create Error Log and return to caller
            /* @
             * @errortype
             * @moduleid    PROC_GPST_INIT_FAILURE_MOD
             * @reasoncode  INTERNAL_FAILURE
             * @userdata1   SRAM Address of the Pstate Table
             * @userdata2   Return Code of call that failed
             * @userdata4   ERC_PROC_PSTATE_INSTALL_FAILURE
             * @devdesc     Failed to install Pstate Table
             */
            l_errlHndl = createErrl(
                    PROC_GPST_INIT_FAILURE_MOD,         //modId
                    INTERNAL_FAILURE,                   //reasoncode // @wb003
                    ERC_PROC_PSTATE_INSTALL_FAILURE,    //Extended reason code
                    ERRL_SEV_UNRECOVERABLE,             //Severity
                    NULL,                               //Trace Buf
                    DEFAULT_TRACE_SIZE,                 //Trace Size
                    (uint32_t) &G_global_pstate_table,  //userdata1
                    l_rc                                //userdata2
                    );

            addCalloutToErrl(l_errlHndl,
                    ERRL_CALLOUT_TYPE_HUID,
                    G_sysConfigData.proc_huid,
                    ERRL_CALLOUT_PRIORITY_HIGH);
            addCalloutToErrl(l_errlHndl,
                    ERRL_CALLOUT_TYPE_COMPONENT_ID,
                    ERRL_COMPONENT_ID_FIRMWARE,
                    ERRL_CALLOUT_PRIORITY_MED);
            break;
        }


        // set up key globals based on the pstate table.  
        
        // Set the pstate state (state machine will start enabling pstates
        // when it sees this)
        proc_gpsm_dcm_sync_set_state(PROC_GPSM_SYNC_PSTATE_TABLE_INSTALLED);

        // Set up Key Globals for use by proc_freq2pstate functions
        G_proc_gpst_fmax = l_gpst_ptr->pstate0_frequency_khz 
            + (((int8_t) l_gpst_ptr->pmin + l_gpst_ptr->entries - 1) 
                    * l_gpst_ptr->frequency_step_khz);
        G_proc_gpst_fmin = l_gpst_ptr->pstate0_frequency_khz 
            + (((int8_t) l_gpst_ptr->pmin) 
                    * l_gpst_ptr->frequency_step_khz);
        G_proc_gpst_pmax = l_gpst_ptr->pmin + l_gpst_ptr->entries - 1;

        // Dcom uses this to know whether to pass DCM msgs or not.
        G_isDcm      = gpsm_dcm_mode_p();

        // Set globals used by amec for pcap calculation
        G_mhz_per_pstate = (l_gpst_ptr->frequency_step_khz)/1000;

    }while(0);
                                                                                                                               
    return l_errlHndl;
}


// gm025
// Function Specification
//
// Name:  proc_pstate_kvm_setup
//
// Description: Get everything set up for KVM mode
//
// Flow:  xx/xx/xx    FN=
//
// End Function Specification
void proc_pstate_kvm_setup()
{
    int                                 l_core;
    int                                 l_rc = 0;
    uint32_t                            l_configured_cores;
    pcbs_pcbspm_mode_reg_t              l_ppmr;
    pcbs_pmgp1_reg_t                    l_pmgp1;
    pcbs_power_management_bounds_reg_t  l_pmbr;
    errlHndl_t                          l_errlHndl;

    do
    {
        //only run this in KVM mode
        if(!G_sysConfigData.system_type.kvm)
        {
            break;
        }

        l_configured_cores = ~in32(PMC_CORE_DECONFIGURATION_REG);

        // Do per-core configuration
        for(l_core = 0; l_core < PGP_NCORES; l_core++, l_configured_cores <<= 1)
        {
            if(!(l_configured_cores & 0x80000000)) continue;

            //do read-modify-write to allow pmax clip to also clip voltage (not just frequency)
            l_rc = getscom_ffdc(CORE_CHIPLET_ADDRESS(PCBS_PCBSPM_MODE_REG, l_core),
                       &(l_ppmr.value), NULL); //commit errors internally -- gm033
            if(l_rc)
            {
                TRAC_ERR("proc_pstate_kvm_setup: getscom(PCBS_PCBSPM_MODE_REG) failed. rc=%d, hw_core=%d",
                         l_rc, l_core);
                break;
            }
            l_ppmr.fields.enable_clipping_of_global_pstate_req = 1;
            l_rc = putscom_ffdc(CORE_CHIPLET_ADDRESS(PCBS_PCBSPM_MODE_REG, l_core),
                 l_ppmr.value, NULL); //commit errors internally -- gm033
            if(l_rc)
            {
                TRAC_ERR("proc_pstate_kvm_setup: putscom(PCBS_PCBSPM_MODE_REG) failed. rc=%d, hw_core=%d",
                         l_rc, l_core);
                break;
            }

            //per Vaidy Srinivasan, clear bit 11 in the Power Management GP1 register
            l_pmgp1.value = 0;
            l_pmgp1.fields.pm_spr_override_en = 1;
            l_rc = putscom_ffdc(CORE_CHIPLET_ADDRESS(PCBS_PMGP1_REG_AND, l_core),
                           ~l_pmgp1.value, NULL); //commit errors internally
            if(l_rc)
            {
                TRAC_ERR("proc_pstate_kvm_setup: putscom(PCBS_PMGB1_REG_OR) failed. rc=0x%08x, hw_core=%d",
                         l_rc, l_core);
                break;
            }

            //set pmax/pmin clip initial settings
            l_pmbr.value = 0;
            l_pmbr.fields.pmin_clip = gpst_pmin(&G_global_pstate_table)+1; //Per David Du, we must use pmin+1 to avoid gpsa hang
            l_pmbr.fields.pmax_clip = gpst_pmax(&G_global_pstate_table);
            l_rc = putscom_ffdc(CORE_CHIPLET_ADDRESS(PCBS_POWER_MANAGEMENT_BOUNDS_REG, l_core),
                           l_pmbr.value, NULL); //commit errors internally -- gm033
            if(l_rc)
            {
                TRAC_ERR("proc_pstate_kvm_setup: putscom(PCBS_POWER_MANAGEMENT_BOUNDS_REG) failed. rc=0x%08x, hw_core=%d",
                         l_rc, l_core);
                break;
            }

        }// end of per-core config

        if(l_rc)
        {
            break;
        }

        // Set the voltage clipping register to match the pmax/pmin clip values set above.
        pmc_rail_bounds_register_t prbr;
        prbr.value = in32(PMC_RAIL_BOUNDS_REGISTER);
        prbr.fields.pmin_rail = gpst_pmin(&G_global_pstate_table);
        prbr.fields.pmax_rail = gpst_pmax(&G_global_pstate_table);
        TRAC_IMP("pmin clip pstate = %d, pmax clip pstate = %d", prbr.fields.pmin_rail, prbr.fields.pmax_rail);
        out32(PMC_RAIL_BOUNDS_REGISTER, prbr.value);

        // Initialize the sapphire table in SRAM (sets valid bit)
        populate_pstate_to_sapphire_tbl();

        // copy sram image into mainstore HOMER
        populate_sapphire_tbl_to_mem();
        TRAC_IMP("proc_pstate_kvm_setup: RUNNING IN KVM MODE");
    }while(0);

    if(l_rc)
    {
        // Create Error Log and request reset
        /* @
         * @errortype
         * @moduleid    PROC_PSTATE_KVM_SETUP_MOD
         * @reasoncode  PROC_SCOM_ERROR
         * @userdata1   l_configured_cores
         * @userdata2   Return Code of call that failed
         * @userdata4   OCC_NO_EXTENDED_RC
         * @devdesc     OCC failed to scom a core register
         */
        l_errlHndl = createErrl(
                PROC_PSTATE_KVM_SETUP_MOD,          //modId
                PROC_SCOM_ERROR,                    //reasoncode
                OCC_NO_EXTENDED_RC,                 //Extended reason code
                ERRL_SEV_PREDICTIVE,                //Severity
                NULL,                               //Trace Buf
                DEFAULT_TRACE_SIZE,                 //Trace Size
                l_configured_cores,                 //userdata1
                l_rc                                //userdata2
                );

        addCalloutToErrl(l_errlHndl,
                ERRL_CALLOUT_TYPE_HUID,
                G_sysConfigData.proc_huid,
                ERRL_CALLOUT_PRIORITY_HIGH);
        addCalloutToErrl(l_errlHndl,
                ERRL_CALLOUT_TYPE_COMPONENT_ID,
                ERRL_COMPONENT_ID_FIRMWARE,
                ERRL_CALLOUT_PRIORITY_MED);

        REQUEST_RESET(l_errlHndl);
    }
}

// Function Specification
//
// Name:  proc_gpsm_dcm_sync_enable_pstates_smh
//
// Description:  Step through all the states & synch needed to enable
//               Pstates on both master & slave on a DCM.  This also 
//               works for a SCM, which will act as DCM master (as far
//               as this function is concerned.)
//
// Flow:  xx/xx/xx    FN=
//
// End Function Specification
void proc_gpsm_dcm_sync_enable_pstates_smh(void)
{
    // Static Locals
    static GpsmEnablePstatesMasterInfo l_master_info;
    static Pstate l_voltage_pstate, l_freq_pstate;

    // Local Variables
    int l_rc = 0; 
    errlHndl_t l_errlHndl = NULL;

    if(!gpsm_dcm_slave_p())
    {
        // ---------------------------------------
        // SCM or DCM Master
        // ---------------------------------------
        switch( G_proc_dcm_sync_state.sync_state_master )
        {
            case PROC_GPSM_SYNC_NO_PSTATE_TABLE:
                // Waiting for Pstate Table from TMGT
                break;

            case PROC_GPSM_SYNC_PSTATE_TABLE_INSTALLED:
                PROC_DBG("GPST DCM Master State %d\n",G_proc_dcm_sync_state.sync_state_master);
                // DCM SYNC (MasterWaitForSlave):  Wait for slave to install Pstate table
                if(gpsm_dcm_mode_p()){
                    if( G_proc_dcm_sync_state.sync_state_slave == PROC_GPSM_SYNC_PSTATE_TABLE_INSTALLED)
                    {
                        // Move to next state in state machine
                        G_proc_dcm_sync_state.sync_state_master = PROC_GPSM_SYNC_READY_TO_ENABLE_MASTER;               
                    }
                }
                else
                {
                    // Move to next state in state machine
                    G_proc_dcm_sync_state.sync_state_master = PROC_GPSM_SYNC_READY_TO_ENABLE_MASTER;
                }
                break;

            case  PROC_GPSM_SYNC_READY_TO_ENABLE_MASTER:
                PROC_DBG("GPST DCM Master State %d\n",G_proc_dcm_sync_state.sync_state_master);

                // Pstate tables has been installed, so now Master can start to enable Pstates
                l_rc = gpsm_enable_pstates_master(&l_master_info, &l_voltage_pstate, &l_freq_pstate);
                if(l_rc)
                {
                    // Error
                    TRAC_ERR("MSTR: gpsm_enable_pstates_master failed with rc=0x%08x", l_rc);
                    G_proc_dcm_sync_state.sync_state_master =  PROC_GPSM_SYNC_PSTATE_ERROR;
                    break;
                }
                TRAC_IMP("MSTR: Initial Pstates: V: %d, F: %d\n",l_voltage_pstate, l_freq_pstate);

                // DCM SYNC (Master2Slave):  Send V & F Pstate to slave 
                G_proc_dcm_sync_state.dcm_pair_id = G_pob_id.chip_id;   
                G_proc_dcm_sync_state.pstate_v = l_voltage_pstate;
                G_proc_dcm_sync_state.pstate_f = l_freq_pstate;

                // Move to next state in state machine
                G_proc_dcm_sync_state.sync_state_master = PROC_GPSM_SYNC_PSTATE_MASTER_ENABLED;
                break;

            case  PROC_GPSM_SYNC_PSTATE_MASTER_ENABLED:
                PROC_DBG("GPST DCM Master State %d\n",G_proc_dcm_sync_state.sync_state_master);
                // DCM SYNC (MasterWaitForSlave):  Wait for slave to complete gpsm_enable_pstates_slave()
                if(gpsm_dcm_mode_p()){
                    if( G_proc_dcm_sync_state.sync_state_slave == PROC_GPSM_SYNC_PSTATE_SLAVE_ENABLED)
                    {
                        // Move to next state in state machine
                        G_proc_dcm_sync_state.sync_state_master = PROC_GPSM_SYNC_READY_TO_ENABLE_SLAVE;
                    }
                }
                else
                {
                    G_proc_dcm_sync_state.sync_state_master = PROC_GPSM_SYNC_READY_TO_ENABLE_SLAVE;
                }
                break;


            case PROC_GPSM_SYNC_READY_TO_ENABLE_SLAVE:
                PROC_DBG("GPST DCM Master State %d\n",G_proc_dcm_sync_state.sync_state_master);

                // Master does next step of enabling Pstates, now that slave has done it's enable
                l_rc = gpsm_enable_pstates_slave(&l_master_info, l_voltage_pstate, l_freq_pstate);
                if(l_rc)
                {
                    // Error
                    TRAC_ERR("MSTR: gpsm_enable_pstates_slave failed with rc=0x%08x", l_rc);
                    G_proc_dcm_sync_state.sync_state_master =  PROC_GPSM_SYNC_PSTATE_ERROR;
                    break;
                }
                TRAC_INFO("MSTR: Completed DCM Pstate Slave Init\n");
                G_proc_dcm_sync_state.sync_state_master = PROC_GPSM_SYNC_PSTATE_SLAVE_ENABLED;
                break;

            case PROC_GPSM_SYNC_PSTATE_SLAVE_ENABLED:
                PROC_DBG("GPST DCM Master State %d\n",G_proc_dcm_sync_state.sync_state_master);
                // Master puts this chip in Pstate HW mode
                l_rc = gpsm_hw_mode();
                if(l_rc)
                {
                    // Error
                    TRAC_ERR("MSTR: gpsm_hw_mode failed with rc=0x%08x", l_rc);
                    G_proc_dcm_sync_state.sync_state_master =  PROC_GPSM_SYNC_PSTATE_ERROR;
                    break;
                }
                // DCM SYNC (Master2Slave):  Tell Slave Master has entered HW mmode
                G_proc_dcm_sync_state.sync_state_master = PROC_GPSM_SYNC_PSTATE_HW_MODE;
                break;

            case PROC_GPSM_SYNC_PSTATE_HW_MODE:
                PROC_DBG("GPST DCM Master State %d\n",G_proc_dcm_sync_state.sync_state_master);
                // DCM SYNC (Master2Slave): Wait for Slave to Enter HW Mode 
                if(gpsm_dcm_mode_p()){
                    if( G_proc_dcm_sync_state.sync_state_slave == PROC_GPSM_SYNC_PSTATE_HW_MODE)
                    {
                        TRAC_INFO("MSTR: Completed DCM Pstate Enable");
                        G_proc_dcm_sync_state.sync_state_master = PROC_GPSM_SYNC_PSTATE_HW_MODE_ENABLED;

                        //do additional setup if in kvm mode -- gm025
                        proc_pstate_kvm_setup();
                    }
                }
                else
                {
                    G_proc_dcm_sync_state.sync_state_master = PROC_GPSM_SYNC_PSTATE_HW_MODE_ENABLED;
                    TRAC_INFO("MSTR: Completed SCM Pstate Enable");

                    //do additional setup if in kvm mode -- gm025
                    proc_pstate_kvm_setup();
                }
                break;

            case PROC_GPSM_SYNC_PSTATE_HW_MODE_ENABLED:
                // Final State
                // Pstates Enabled on both modules in DCM
                break;

           case PROC_GPSM_SYNC_PSTATE_ERROR:
               // Do nothing, something will have to come and kick us out of this state
               break;

            default:
                G_proc_dcm_sync_state.sync_state_master = PROC_GPSM_SYNC_NO_PSTATE_TABLE;
                break;
        }
    }
    else if (gpsm_dcm_slave_p())
    {
        // ---------------------------------------
        // DCM Slave
        //  - Don't need to check if DCM, since we can't come in here unless DCM
        // ---------------------------------------

       switch( G_proc_dcm_sync_state.sync_state_slave)
       {
           case PROC_GPSM_SYNC_NO_PSTATE_TABLE:
               // Waiting for Pstate Table from TMGT
               break;

           case PROC_GPSM_SYNC_PSTATE_TABLE_INSTALLED:
               // Pstate table has been installed, but slave needs to wait 
               // for master before it can do anything else.

               // DCM SYNC (SlaveWaitForMaster):  Send V & F Pstate to slave 
               // Wait for Master to complete gpsm_enable_pstates_master() 
               // before running gpsm_enable_pstates_slave()
               if( G_proc_dcm_sync_state.sync_state_master == PROC_GPSM_SYNC_PSTATE_MASTER_ENABLED)
               {
                   // Go to next state
                   G_proc_dcm_sync_state.sync_state_slave = PROC_GPSM_SYNC_PSTATE_MASTER_ENABLED;
               }
               break;


           case PROC_GPSM_SYNC_PSTATE_MASTER_ENABLED:
               PROC_DBG("GPST DCM Slave State %d\n",G_proc_dcm_sync_state.sync_state_slave);
               // Read the initial Pstates from the data DCM master sent  
               l_voltage_pstate = G_proc_dcm_sync_state.pstate_v;
               l_freq_pstate = G_proc_dcm_sync_state.pstate_f;

               // NULL is passed to this function when run on dcm slave   
               l_rc = gpsm_enable_pstates_slave(NULL, l_voltage_pstate, l_freq_pstate);
               if(l_rc)
               {
                   // Error
                   TRAC_ERR("SLV: gpsm_enable_pstates_slave failed with rc=0x%08x", l_rc);
                   G_proc_dcm_sync_state.sync_state_slave =  PROC_GPSM_SYNC_PSTATE_ERROR;
                   break;
               }
               TRAC_INFO("SLV: Completed DCM Pstate Slave Init\n");

               // DCM SYNC (Slave2Master):  
               // Tell Master that slave has run gpsm_enable_pstates_slave()
               
               // Go to next state
               G_proc_dcm_sync_state.sync_state_slave = PROC_GPSM_SYNC_PSTATE_SLAVE_ENABLED;
               break;

           case PROC_GPSM_SYNC_PSTATE_SLAVE_ENABLED:
               // DCM SYNC (SlaveWaitForMaster):  Wait for Master to run gpsm_hw_mode
               if( G_proc_dcm_sync_state.sync_state_master == PROC_GPSM_SYNC_PSTATE_HW_MODE)
               {
                   // Enter Pstate HW mode
                   l_rc = gpsm_hw_mode();
                   if(l_rc)
                   {
                       // Error
                       TRAC_ERR("SLV: gpsm_hw_mode failed with rc=0x%08x", l_rc);
                       G_proc_dcm_sync_state.sync_state_slave =  PROC_GPSM_SYNC_PSTATE_ERROR;
                       break;
                   }

                   // DCM SYNC (Slave2Master): Tell master that DCM slave made it to HW mode
                   
                   // Go to next state
                   G_proc_dcm_sync_state.sync_state_slave = PROC_GPSM_SYNC_PSTATE_HW_MODE;
               }
               break;

           case PROC_GPSM_SYNC_PSTATE_HW_MODE:
               // Slave & Master now both know each other has HW mode enabled
               if( G_proc_dcm_sync_state.sync_state_master == PROC_GPSM_SYNC_PSTATE_HW_MODE_ENABLED)
               {
                   G_proc_dcm_sync_state.sync_state_slave = PROC_GPSM_SYNC_PSTATE_HW_MODE_ENABLED;
                   TRAC_INFO("SLV: Completed DCM Pstate Enable");

                   //do additional setup if in kvm mode -- gm025
                   proc_pstate_kvm_setup();
               }
               break;

           case PROC_GPSM_SYNC_PSTATE_HW_MODE_ENABLED:
               // Final State
               // Pstates Enabled on both modules in DCM
               break;

           case PROC_GPSM_SYNC_PSTATE_ERROR:
               // Do nothing, something will have to come and kick us out of this state
               break;

           default:
               G_proc_dcm_sync_state.sync_state_slave = PROC_GPSM_SYNC_NO_PSTATE_TABLE;
               break;

       }
    }

    // If we are in the process of running through the state machine,
    // we will do a sem_post to speed up the DCOM Thread and step us 
    // through faster.
    if( PROC_GPSM_SYNC_NO_PSTATE_TABLE != proc_gpsm_dcm_sync_get_my_state()
        && !proc_is_hwpstate_enabled() )
    {
       ssx_semaphore_post(&G_dcomThreadWakeupSem);   // @th025
    }

    // If we broke out of loops above because of an error, create an
    // error log and return it to caller.
    if(l_rc)
    {
        /* @
         * @errortype
         * @moduleid    PROC_ENABLE_PSTATES_SMH_MOD
         * @reasoncode  SSX_GENERIC_FAILURE
         * @userdata1   SRAM Address of the Pstate Table
         * @userdata2   Return Code of call that failed
         * @userdata4   OCC_NO_EXTENDED_RC
         * @devdesc     Failed to install Pstate Table
         */
        l_errlHndl = createErrl(   //@gm006
                PROC_ENABLE_PSTATES_SMH_MOD,        //modId
                SSX_GENERIC_FAILURE,                //reasoncode
                OCC_NO_EXTENDED_RC,                 //Extended reason code
                ERRL_SEV_PREDICTIVE,                //Severity
                NULL,    //TODO: create trace       //Trace Buf
                DEFAULT_TRACE_SIZE,                 //Trace Size
                (uint32_t) &G_global_pstate_table,  //userdata1
                l_rc);                              //userdata2
        addCalloutToErrl(l_errlHndl,
                         ERRL_CALLOUT_TYPE_COMPONENT_ID,
                         ERRL_COMPONENT_ID_FIRMWARE,
                         ERRL_CALLOUT_PRIORITY_HIGH);
        addCalloutToErrl(l_errlHndl,
                         ERRL_CALLOUT_TYPE_HUID,
                         G_sysConfigData.proc_huid,
                         ERRL_CALLOUT_PRIORITY_LOW);
        REQUEST_RESET(l_errlHndl);
    }

    return;
}


// @ly010a
// Function Specification
//
// Name:  populate_pstate_to_sapphire_tbl
//
// Description:
//              
// Flow:  xx/xx/xx    FN=
//
// End Function Specification
void populate_pstate_to_sapphire_tbl()
{
    uint8_t i = 0;
    GlobalPstateTable * l_gpst_ptr = NULL;

    memset(&G_sapphire_table, 0, sizeof(sapphire_table_t));

    l_gpst_ptr = gpsm_gpst();
    const int8_t l_pmax = (int8_t) l_gpst_ptr->pmin + l_gpst_ptr->entries - 1;
    G_sapphire_table.config.valid = 1; // default 0x01
    G_sapphire_table.config.version = 1; // default 0x01
    G_sapphire_table.config.throttle = NO_THROTTLE; // defult 0x00
    G_sapphire_table.config.pmin = gpst_pmin(&G_global_pstate_table)+1; //Per David Du, we must use pmin+1 to avoid gpsa hang -- gm025
    G_sapphire_table.config.pnominal = (int8_t)proc_freq2pstate(G_sysConfigData.sys_mode_freq.table[OCC_MODE_NOMINAL]);
    G_sapphire_table.config.pmax = gpst_pmax(&G_global_pstate_table);
    const uint16_t l_entries = G_sapphire_table.config.pmax - G_sapphire_table.config.pmin + 1;
    const uint8_t l_idx = l_gpst_ptr->entries-1;

    for (i = 0; i < l_entries; i++)
    {
        G_sapphire_table.data[i].pstate = (int8_t) l_pmax - i;
        G_sapphire_table.data[i].flag = 0; // defult 0x00
        if (i < l_gpst_ptr->entries)
        {
            G_sapphire_table.data[i].evid_vdd = l_gpst_ptr->pstate[i].fields.evid_vdd;
            G_sapphire_table.data[i].evid_vcs = l_gpst_ptr->pstate[i].fields.evid_vcs;
        }
        else
        {
            // leave the VDD & VCS Vids the same as the "Pstate Table Pmin"
            G_sapphire_table.data[i].evid_vdd = l_gpst_ptr->pstate[l_idx].fields.evid_vdd;
            G_sapphire_table.data[i].evid_vcs = l_gpst_ptr->pstate[l_idx].fields.evid_vcs;     
        }
        // extrapolate the frequency
        G_sapphire_table.data[i].freq_khz = l_gpst_ptr->pstate0_frequency_khz + (G_sapphire_table.data[i].pstate * l_gpst_ptr->frequency_step_khz);
    }
}


// @ly009a @ly010c
// Function Specification
//
// Name:  populate_sapphire_tbl_to_mem
//
// Description:
//              
// Flow:  xx/xx/xx    FN=
//
// End Function Specification
void populate_sapphire_tbl_to_mem()
{
    int l_ssxrc = SSX_OK;
    uint32_t l_reasonCode = 0;
    uint32_t l_extReasonCode = 0;

    do
    {
#define SAPPHIRE_OFFSET_IN_HOMER 0x001F8000
        BceRequest pba_copy;
        // Set up copy request
        l_ssxrc = bce_request_create(&pba_copy,                          // block copy object
                                     &G_pba_bcue_queue,                  // sram to mainstore copy engine 
                                     SAPPHIRE_OFFSET_IN_HOMER,           // mainstore address
                                     (uint32_t) &G_sapphire_table,       // sram starting address
                                     (size_t) sizeof(G_sapphire_table),  // size of copy 
                                     SSX_WAIT_FOREVER,                   // no timeout
                                     NULL,                               // call back 
                                     NULL,                               // call back arguments
                                     ASYNC_REQUEST_BLOCKING              // callback mask
                                     );

        if(l_ssxrc != SSX_OK)
        {
            TRAC_ERR("populate_sapphire_tbl_to_mem: PBA request create failure rc=[%08X]", -l_ssxrc);
            /* 
             * @errortype
             * @moduleid    MAIN_STATE_TRANSITION_MID
             * @reasoncode  SSX_GENERIC_FAILURE
             * @userdata1   RC for PBA block-copy engine
             * @userdata4   ERC_BCE_REQUEST_CREATE_FAILURE
             * @devdesc     SSX BCE related failure
             */   
            l_reasonCode = SSX_GENERIC_FAILURE;
            l_extReasonCode = ERC_BCE_REQUEST_CREATE_FAILURE;
            break;
        }

        // Do actual copying
        l_ssxrc = bce_request_schedule(&pba_copy);

        if(l_ssxrc != SSX_OK)
        {
            TRAC_ERR("populate_sapphire_tbl_to_mem: PBA request schedule failure rc=[%08X]", -l_ssxrc);
            /* 
             * @errortype
             * @moduleid    MAIN_STATE_TRANSITION_MID
             * @reasoncode  SSX_GENERIC_FAILURE
             * @userdata1   RC for PBA block-copy engine
             * @userdata4   ERC_BCE_REQUEST_SCHEDULE_FAILURE
             * @devdesc     Failed to copy data by using DMA
             */
            l_reasonCode = SSX_GENERIC_FAILURE;
            l_extReasonCode = ERC_BCE_REQUEST_SCHEDULE_FAILURE;
            break;
        }
    } while(0);

    if ( l_ssxrc != SSX_OK )
    {
        errlHndl_t l_errl = createErrl(MAIN_STATE_TRANSITION_MID,    //modId
                                       l_reasonCode,                 //reasoncode
                                       l_extReasonCode,              //Extended reason code
                                       ERRL_SEV_UNRECOVERABLE,       //Severity
                                       NULL,                         //Trace Buf
                                       0,                            //Trace Size
                                       -l_ssxrc,                     //userdata1
                                       0);                           //userdata2
                                       
        // @wb001 -- Callout firmware
        addCalloutToErrl(l_errl,
                         ERRL_CALLOUT_TYPE_COMPONENT_ID,
                         ERRL_COMPONENT_ID_FIRMWARE,
                         ERRL_CALLOUT_PRIORITY_HIGH);

        commitErrl(&l_errl);
    }
}

// gm025
// Function Specification
//
// Name: proc_check_for_sapphire_updates 
//
// Description: Checks if the sapphire table needs an update
//              and updates if necessary.
//              
// Flow:  xx/xx/xx    FN=
//
// End Function Specification
void proc_check_for_sapphire_updates()
{
    uint8_t l_latest_throttle_reason;

    //If safe state is requested then that overrides anything from amec
    if(isSafeStateRequested())
    {
        l_latest_throttle_reason = OCC_RESET;
    }
    else
    {
        l_latest_throttle_reason = G_amec_kvm_throt_reason;
    }

    //If the throttle reason changed, update it in the HOMER
    if(G_sapphire_table.config.throttle != l_latest_throttle_reason)
    {
        TRAC_INFO("proc_check_for_sapphire_updates: throttle reason changed to %d", l_latest_throttle_reason);
        G_sapphire_table.config.throttle = l_latest_throttle_reason;
        G_sapphire_table.config.version = 1; // default 0x01
        G_sapphire_table.config.valid = 1; //default 0x01
        populate_sapphire_tbl_to_mem();
    }
}
