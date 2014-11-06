/******************************************************************************
// @file amec_freq.c
// @brief Amec Frequency Handling
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section _amec_freq_c amec_freq.c
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *   @th015             thallet   08/03/2012  New file
 *   @gs001             gsilva    08/03/2012  New file
 *   @th017             thallet   08/30/2012  Added the sensor update in votebox
 *   @ai001  853751     ailutsar  09/12/2012  Support for per-core frequency votes
 *   @ry002  862116     ronda     11/16/2012  Support thermal controller for processor
 *   @ry003  870734     ronda     02/20/2013  Thermal controller for memory 
 *   @ry004             ronda     02/27/2013  Added slave memory voting box
 *   @th040  887069     thallet   06/11/2013  Support Nom & FFO Freq Setting for Mnfg 
 *   @fk001  879727     fmkassem  04/16/2013  Power capping support.
 *   @gs007  888247     gjsilva   06/19/2013  OCC mnfg support for frequency distribution
 *   @cl001             lefurgy   07/24/2013  Fix overflow in l_throttle
 *   @gm004  892961     milesg    07/25/2013  Support memory auto slewing
 *   @gs008  894661     gjsilva   08/08/2013  Initial support for DPS-FP mode
 *   @at016  891144     alvinwan  06/10/2013  OCC Power Cap Testing
 *   @gs009  897228     gjsilva   08/28/2013  Enablement of DPS-FP Mode
 *   @rt001  897459     tapiar    08/21/2013  health monitor changes
 *   @rt002  901927     tapiar    10/01/2013  fix src tags 
 *   @gs014  903552     gjsilva   10/22/2013  Support for Amester parameter interface
 *   @gm012  905097     milesg    10/31/2013  enhanced power cap tracing
 *   @gs015  905166     gjsilva   11/04/2013  Full support for IPS function
 *   @rt003  905677     tapiar    11/07/2013  revamp trace/error log for frequency limited due
 *                                            system pcap scenario
 *   @gs017  905990     gjsilva   11/13/2013  Full support for tunable parameters
 *   @gs018  907196     gjsilva   11/20/2013  Base support for soft frequency boundaries
 *   @at019  908390     alvinwan  12/05/2013  Disable DPS algorithms from running in Sapphire
 *   @gm016  909061     milesg    12/10/2013  Support memory throttling due to temperature
 *   @at020  908666     alvinwan  12/16/2013  Oversubscription Error Handling
 *   @at022  910758     alvinwan  01/08/2014  B1812A33 logged while in DPS mode
 *   @gs023  912003     gjsilva   01/16/2014  Generate VRHOT signal and control loop
 *   @gs025  913663     gjsilva   01/30/2014  Full fupport for soft frequency boundaries
 *   @gs026  915840     gjsilva   02/13/2014  Support for Nvidia GPU power measurement
 *   @gm025  915973     milesg    02/14/2014  Full support for sapphire (KVM) mode
 *   @wb003  920760     wilbryan  03/24/2014  Update SRCs to match TPMD SRCs
 *   @wb004  922138     wilbryan  04/03/2014  Ensure timely pstate completion
 *   @gs036  931408     gjsilva   07/09/2014  Do not log 2A61 in DPS-FP mode
 *
 *  @endverbatim
 *
 *///*************************************************************************/
 
//*************************************************************************
// Includes
//*************************************************************************
#include <occ_common.h>
#include <ssx.h>         
#include <errl.h> 
#include "sensor.h"
#include "rtls.h"
#include "occ_sys_config.h"
#include "occ_service_codes.h"  // for SSX_GENERIC_FAILURE
#include "dcom.h"
#include "proc_data.h"
#include "proc_data_control.h"
#include "amec_smh.h"
#include "amec_slave_smh.h"
#include <trac.h>
#include "amec_sys.h"
#include "sensor_enum.h"
#include "amec_service_codes.h"
#include <amec_sensors_core.h>
#include <amec_sensors_power.h>
#include <amec_sensors_fw.h>
#include <amec_data.h>
#include <amec_freq.h>

//*************************************************************************
// Externs
//*************************************************************************
extern uint8_t G_cent_temp_expired_bitmap;
extern uint8_t G_dimm_temp_expired_bitmap;

//*************************************************************************
// Defines/Enums
//*************************************************************************

//*************************************************************************
// Structures
//*************************************************************************

//*************************************************************************
// Globals
//*************************************************************************
BOOLEAN G_non_dps_power_limited = FALSE;        //@rt001a
uint8_t G_amec_kvm_throt_reason = NO_THROTTLE; //gm025
uint16_t G_time_until_freq_check = FREQ_CHG_CHECK_TIME; // @wb004

// >> gitprep
// Moved following two structures because of multiple definitions error

//FFDC SCOM addresses as requested by Greg Still in defect SW247927 -- gm031
const uint32_t G_pmc_ffdc_scom_addrs[] =
{
    PMC_LFIR_ERR_REG,
    PMC_LFIR_ERR_MASK_REG,
    OCB_OCCLFIR,
    PBA_FIR
};

//FFDC OCI addresses as requested by Greg Still in defect SW247927 -- gm031
const uint32_t G_pmc_ffdc_oci_addrs[] =
{
    PMC_MODE_REG,
    PMC_PSTATE_MONITOR_AND_CTRL_REG,
    PMC_RAIL_BOUNDS_REGISTER,
    PMC_PARAMETER_REG0,
    PMC_PARAMETER_REG1,
    PMC_EFF_GLOBAL_ACTUAL_VOLTAGE_REG,
    PMC_STATUS_REG,
    PMC_INTCHP_CTRL_REG1,
    PMC_INTCHP_CTRL_REG4,
    PMC_INTCHP_STATUS_REG,
    PMC_INTCHP_COMMAND_REG,
    PMC_INTCHP_PSTATE_REG,
    PMC_SPIV_CTRL_REG0A,
    PMC_SPIV_CTRL_REG0B,
    PMC_SPIV_CTRL_REG1,
    PMC_SPIV_CTRL_REG2,
    PMC_SPIV_CTRL_REG3,
    PMC_SPIV_CTRL_REG4,
    PMC_SPIV_STATUS_REG,
    PMC_SPIV_COMMAND_REG,
    PMC_O2S_CTRL_REG0A,
    PMC_O2S_CTRL_REG0B,
    PMC_O2S_CTRL_REG1,
    PMC_O2S_CTRL_REG2,
    PMC_O2S_CTRL_REG4,
    PMC_O2S_STATUS_REG,
    PMC_O2S_COMMAND_REG,
    PMC_O2S_WDATA_REG,
    0                           //0 marks last OCI address
};
// << gitprep

//*************************************************************************
// Function Prototypes
//*************************************************************************

//*************************************************************************
// Functions
//*************************************************************************


// Function Specification
//
// Name: amec_set_freq_range
//
// Description: Set the frequency range for AMEC
//              This function will run on mode changes and cnfg_data changes
//              
//
// Flow:              FN=
//
// Thread: RealTime Loop
//
// Task Flags: 
//
// End Function Specification
errlHndl_t amec_set_freq_range(const OCC_MODE i_mode)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    errlHndl_t                  l_err = NULL;
    uint16_t                    l_freq_min  = 0;
    uint16_t                    l_freq_max  = 0;
    uint32_t                    l_temp = 0;
    amec_mode_freq_t            l_ppm_freq[OCC_INTERNAL_MODE_MAX_NUM] = {{0}};
    //uint16_t l_freq_nom  = 0;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    // First set to Max Freq Range for this mode
    if( VALID_MODE(i_mode) )
    { 
      l_freq_min = G_sysConfigData.sys_mode_freq.table[OCC_MODE_MIN_FREQUENCY];
      l_freq_max = G_sysConfigData.sys_mode_freq.table[i_mode];
    }

    // If SMS is set then TMGT wants us to pin to frequency which
    // corresponds to input mode.  They will use this function
    // when powering off and they wish to have us bring the system
    // back up to real nominal frequency (without being impacted
    // by power caps or thermal actuations)
    if(CURRENT_SMS() == SMGR_SMS_STATIC_VF_CHANGE_REQ)
    {
        // I>amec_control_set_freq_range: Request that we pin freq to %u for mode 0x%X
        //TRACE2(g_trac_amec,1775283132,l_freq_max,i_mode);

        l_freq_min = l_freq_max;
    }

    g_amec->sys.fmin = l_freq_min;
    g_amec->sys.fmax = l_freq_max;

    TRAC_INFO("amec_set_freq_range: Mode[0x%02x] Fmin[%u] Fmax[%u]",
              i_mode,
              l_freq_min,
              l_freq_max);

    // Now determine the max frequency for the PPM structure
    l_ppm_freq[OCC_INTERNAL_MODE_NOM].fmax    = G_sysConfigData.sys_mode_freq.table[OCC_MODE_NOMINAL];
    l_ppm_freq[OCC_INTERNAL_MODE_DPS].fmax    = G_sysConfigData.sys_mode_freq.table[OCC_MODE_DYN_POWER_SAVE];
    l_ppm_freq[OCC_INTERNAL_MODE_DPS_MP].fmax = G_sysConfigData.sys_mode_freq.table[OCC_MODE_DYN_POWER_SAVE_FP];

    // Determine the min frequency for the PPM structure. This Fmin should
    // always be set to the system Fmin
    l_ppm_freq[OCC_INTERNAL_MODE_NOM].fmin    = G_sysConfigData.sys_mode_freq.table[OCC_MODE_MIN_FREQUENCY];
    l_ppm_freq[OCC_INTERNAL_MODE_DPS].fmin    = G_sysConfigData.sys_mode_freq.table[OCC_MODE_MIN_FREQUENCY];
    l_ppm_freq[OCC_INTERNAL_MODE_DPS_MP].fmin = G_sysConfigData.sys_mode_freq.table[OCC_MODE_MIN_FREQUENCY];

    // Determine the min speed allowed for DPS power policies (this is needed
    // by the DPS algorithms)
    l_temp = (l_ppm_freq[OCC_INTERNAL_MODE_DPS].fmin * 1000)/l_ppm_freq[OCC_INTERNAL_MODE_DPS].fmax;
    l_ppm_freq[OCC_INTERNAL_MODE_DPS].min_speed = l_temp;

    l_temp = (l_ppm_freq[OCC_INTERNAL_MODE_DPS_MP].fmin * 1000)/l_ppm_freq[OCC_INTERNAL_MODE_DPS_MP].fmax;
    l_ppm_freq[OCC_INTERNAL_MODE_DPS_MP].min_speed = l_temp;

    // Copy the PPM frequency information into g_amec
    memcpy(g_amec->part_mode_freq, l_ppm_freq, sizeof(l_ppm_freq));

    TRAC_INFO("amec_set_freq_range: PPM Fmin[%u] Fnom[%u] Fmax[%u] min_speed[%u]",
              l_ppm_freq[OCC_INTERNAL_MODE_NOM].fmin,
              l_ppm_freq[OCC_INTERNAL_MODE_NOM].fmax,
              l_ppm_freq[OCC_INTERNAL_MODE_DPS].fmax,
              l_ppm_freq[OCC_INTERNAL_MODE_DPS_MP].min_speed);

    return l_err;
}



// Function Specification
//
// Name: amec_slv_voting_box
//
// Description: Slave OCC's voting box that decides the frequency request.
//              This function will run every tick.
//              
//
// Flow: 10/22/13      FN= amec_slv_voting_box.odg
//
// Thread: RealTime Loop
//
// Changedby: @fk001c
//
// Task Flags: 
//
// End Function Specification
void amec_slv_voting_box(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    uint16_t                        k = 0;
    uint16_t                        l_chip_fmax = g_amec->sys.fmax;
    uint16_t                        l_core_freq = 0;
    uint32_t                        l_chip_reason = 0; //@rt001c
    uint32_t                        l_core_reason = 0; //@rt001c
    uint8_t                         l_kvm_throt_reason = NO_THROTTLE; //gm025
    amec_part_t                     *l_part = NULL;
    bool                            l_freq_req_changed = FALSE; // @wb004

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    // Voting Box for CPU speed.
    // This function implements the voting box to decide which input gets the right
    // to actuate the system.

    //Reset the maximum core frequency requested prior to recalculation.
    g_amec->proc[0].core_max_freq = 0;

    // PPB_FMAX
    if(g_amec->proc[0].pwr_votes.ppb_fmax < l_chip_fmax)
    {
        l_chip_fmax = g_amec->proc[0].pwr_votes.ppb_fmax;
        l_chip_reason = AMEC_VOTING_REASON_PPB;
        l_kvm_throt_reason = POWERCAP; //gm025
    }

    // PMAX_CLIP_FREQ
    if(g_amec->proc[0].pwr_votes.pmax_clip_freq < l_chip_fmax)
    {
        l_chip_fmax = g_amec->proc[0].pwr_votes.pmax_clip_freq;
        l_chip_reason = AMEC_VOTING_REASON_PMAX;
        l_kvm_throt_reason = POWER_SUPPLY_FAILURE; //gm025
    }

    //THERMALPROC.FREQ_REQUEST
    //Thermal controller input based on processor temperature
    if(g_amec->thermalproc.freq_request < l_chip_fmax)
    {
        l_chip_fmax = g_amec->thermalproc.freq_request;
        l_chip_reason = AMEC_VOTING_REASON_PROC_THRM;
        l_kvm_throt_reason = CPU_OVERTEMP; //gm025
    }

    // Controller request based on VRHOT signal from processor regulator
    if(g_amec->vrhotproc.freq_request < l_chip_fmax)
    {
        l_chip_fmax = g_amec->vrhotproc.freq_request;
        l_chip_reason = AMEC_VOTING_REASON_VRHOT_THRM;
        l_kvm_throt_reason = CPU_OVERTEMP; //gm025
    }

    // CONN_OC_VOTE
    if(g_amec->proc[0].pwr_votes.conn_oc_vote < l_chip_fmax)
    {
        l_chip_fmax = g_amec->proc[0].pwr_votes.conn_oc_vote;
        l_chip_reason = AMEC_VOTING_REASON_CONN_OC;
        l_kvm_throt_reason = OVERCURRENT; //gm025
    }

    for (k=0; k<MAX_NUM_CORES; k++)
    {
        if(CORE_PRESENT(k))
        {
            l_core_freq = l_chip_fmax;
            l_core_reason = l_chip_reason;

            // Disable DPS in KVM // @at019a
            if(!G_sysConfigData.system_type.kvm)
            {
                l_part = amec_part_find_by_core(&g_amec->part_config, k);

                // Check frequency request generated by DPS algorithms
                if(g_amec->proc[0].core[k].core_perf.dps_freq_request < l_core_freq)
                {
                    l_core_freq = g_amec->proc[0].core[k].core_perf.dps_freq_request;
                    l_core_reason = AMEC_VOTING_REASON_UTIL;
                }

                // Adjust frequency based on soft frequency boundaries
                if(l_part != NULL)
                {
                    if(l_core_freq < l_part->soft_fmin)
                    {
                        // Before enforcing a soft Fmin, make sure we don't
                        // have a thermal or power emergency
                        if(!(l_chip_reason & (AMEC_VOTING_REASON_PROC_THRM |
                                              AMEC_VOTING_REASON_VRHOT_THRM |
                                              AMEC_VOTING_REASON_PPB |
                                              AMEC_VOTING_REASON_PMAX |
                                              AMEC_VOTING_REASON_CONN_OC))) //gm025
                        {
                            l_core_freq = l_part->soft_fmin;
                            l_core_reason = AMEC_VOTING_REASON_SOFT_MIN;
                        }
                    }
                    else if(l_core_freq > l_part->soft_fmax)
                    {
                        l_core_freq = l_part->soft_fmax;
                        l_core_reason = AMEC_VOTING_REASON_SOFT_MAX;
                    }
                }
            }

            if(CURRENT_MODE() == OCC_MODE_NOMINAL)
            {
                // PROC_PCAP_NOM_VOTE
                if(g_amec->proc[0].pwr_votes.proc_pcap_nom_vote < l_core_freq)
                {
                    l_core_freq = g_amec->proc[0].pwr_votes.proc_pcap_nom_vote;
                    l_core_reason = AMEC_VOTING_REASON_PWR;
                    l_kvm_throt_reason = POWERCAP; //gm025
                }
            }
            else
            {
                // PROC_PCAP_VOTE
                if(g_amec->proc[0].pwr_votes.proc_pcap_vote < l_core_freq)
                {
                    l_core_freq = g_amec->proc[0].pwr_votes.proc_pcap_vote;
                    l_core_reason = AMEC_VOTING_REASON_PWR;
                    l_kvm_throt_reason = POWERCAP; //gm025
                }
            }

            // Check IPS frequency request sent by Master OCC
            if(g_amec->slv_ips_freq_request != 0)
            {
                if(g_amec->slv_ips_freq_request < l_core_freq)
                {
                    l_core_freq = g_amec->slv_ips_freq_request;
                    l_core_reason = AMEC_VOTING_REASON_IPS;
                }
            }

            // Override frequency with request from Master OCC
            if(g_amec->foverride_enable)
            {
                if(g_amec->foverride != 0)
                {
                    // Override the frequency on all cores if Master OCC sends
                    // a non-zero request
                    l_core_freq = g_amec->foverride;
                    l_core_reason = AMEC_VOTING_REASON_OVERRIDE;
                }
            }

            //Make sure the frequency is not less then the system min
            if(l_core_freq < g_amec->sys.fmin)
            {
                l_core_freq = g_amec->sys.fmin;
            }

            // Override frequency via Amester parameter interface
            if (g_amec->proc[0].parm_f_override_enable &&
                g_amec->proc[0].parm_f_override[k] > 0)
            {
                l_core_freq = g_amec->proc[0].parm_f_override[k];
                l_core_reason = AMEC_VOTING_REASON_OVERRIDE_CORE;
            }

            // @wb004 -- If frequency has changed, set the flag
            if ( (l_core_freq != g_amec->proc[0].core[k].f_request) ||
                    (l_core_freq != g_amec->sys.fmax))
            {
                l_freq_req_changed = TRUE;
            }

            //STORE core frequency and reason
            g_amec->proc[0].core[k].f_request = l_core_freq;
            g_amec->proc[0].core[k].f_reason = l_core_reason;

            // Update the Amester parameter telling us the reason. Needed for
            // parameter array.
	        g_amec->proc[0].parm_f_reason[k] = l_core_reason;
    
            //@rt001a @at022c
            //CURRENT_MODE() may be OCC_MODE_NOCHANGE because STATE change is processed
            //before MODE change 
            if ((CURRENT_MODE() != OCC_MODE_DYN_POWER_SAVE) &&
                (CURRENT_MODE() != OCC_MODE_DYN_POWER_SAVE_FP) &&
                (CURRENT_MODE() != OCC_MODE_NOCHANGE) &&
                (l_core_reason & NON_DPS_POWER_LIMITED))
            {
                G_non_dps_power_limited = TRUE;
            }
            else
            {
                G_non_dps_power_limited = FALSE;
            }

            // Update the sensor telling us what the requested frequency is
            sensor_update( AMECSENSOR_ARRAY_PTR(FREQ250USP0C0,k), 
                    (uint16_t) g_amec->proc[0].core[k].f_request);  // @th017

#if 0
            /// TODO: This can be deleted if deemed useless
            /// This trace that can be used to debug the voting
            /// box an control loops.  It will trace the reason why a 
            /// controller is lowering the freq, but will only do it once in a 
            /// row for the specific freq it wants to control to.  It assumes
            /// that all cores will be controlled to same freq.
            if(l_chip_fmax != g_amec->sys.fmax){
                static uint16_t L_trace = 0;
                if(l_chip_fmax != L_trace){
                    L_trace = l_chip_fmax;
                    TRAC_INFO("Core: %d, Freq: %d, Reason: %d",k,l_core_freq,l_core_reason);
                }
            }
#endif

            if(l_core_freq > g_amec->proc[0].core_max_freq)
            {
                g_amec->proc[0].core_max_freq = l_core_freq;
            }
        }
        else
        {
            l_core_freq = 0;
            l_core_reason = 0;
        }
    }//End of for loop

    // @wb004 -- Check if the frequency is going to be changing
    if( l_freq_req_changed == TRUE )
    {
        G_time_until_freq_check = FREQ_CHG_CHECK_TIME;
    }
    else if (G_time_until_freq_check != 0)
    {
        G_time_until_freq_check--;
    }

    //convert POWERCAP reason to POWER_SUPPLY_FAILURE if ovs/failsafe is asserted -- gm025
    if((l_kvm_throt_reason == POWERCAP) &&
        (AMEC_INTF_GET_FAILSAFE() || AMEC_INTF_GET_OVERSUBSCRIPTION()))
    {
        l_kvm_throt_reason = POWER_SUPPLY_FAILURE;
    }

    //check if we need to update the throttle reason in homer -- gm025
    if(G_sysConfigData.system_type.kvm &&
       (l_kvm_throt_reason != G_amec_kvm_throt_reason))
    {
        //Notify dcom thread to update the table
        G_amec_kvm_throt_reason = l_kvm_throt_reason;
        ssx_semaphore_post(&G_dcomThreadWakeupSem);
    }

}

// Function Specification
//
// Name: amec_slv_freq_smh
//
// Description: Slave OCC's frequency state machine.
//              This function will run every tick.
//              
//
// Flow:              FN=
//
// Thread: RealTime Loop
//
// Task Flags: 
//
// End Function Specification
void amec_slv_freq_smh(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    uint16_t                        k = 0;
    int8_t                          l_pstate = 0;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    for (k=0; k<MAX_NUM_CORES; k++)
    {
        switch (g_amec->proc[0].core[k].f_sms)
        {
            case AMEC_CORE_FREQ_IDLE_STATE:
                // Translate frequency request into a Pstate
                l_pstate = proc_freq2pstate(g_amec->proc[0].core[k].f_request);

                // Fall through
            case AMEC_CORE_FREQ_PROCESS_STATE:
                if(G_sysConfigData.system_type.kvm) //gm025
                {
                    // update core bounds on kvm systems
                    proc_set_core_bounds(gpst_pmin(&G_global_pstate_table) + 1, (Pstate) l_pstate, k);
                }
                else
                {
                    // update core pstate request on non-kvm systems
                    proc_set_core_pstate((Pstate) l_pstate, k);
                }
                break;
        }
    }

}

// Function Specification
//
// Name: amec_slv_freq_smh
//
// Description: Slave OCC's voting box that decides the memory speed request.
//              This function will run every tick.
//              
//
// Flow:    2-20-12     FN=amec_slv_mem_voting_box.odg
//
// Thread: RealTime Loop
//
// Task Flags: 
//
// End Function Specification
void amec_slv_mem_voting_box(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    UINT16                  l_vote;
    UINT8                   l_reason;
    static INT16            l_slew_step = AMEC_MEMORY_STEP_SIZE;
    static bool             L_throttle_traced = FALSE;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    // Start with max allowed speed
    l_vote = AMEC_MEMORY_MAX_STEP;
    l_reason = AMEC_MEM_VOTING_REASON_INIT;

    // Check vote from Centaur thermal control loop
    if (l_vote > g_amec->thermalcent.speed_request)
    {
        l_vote = g_amec->thermalcent.speed_request;
        l_reason = AMEC_MEM_VOTING_REASON_CENT;
    }

    // Check vote from DIMM thermal control loop
    if (l_vote > g_amec->thermaldimm.speed_request)
    {
        l_vote = g_amec->thermaldimm.speed_request;
        l_reason = AMEC_MEM_VOTING_REASON_DIMM;
    }

    // Check if memory autoslewing is enabled - gm004
    if (g_amec->mnfg_parms.mem_autoslew)
    {
        //check if we've reached the max setting and need to start going down
        if(g_amec->mem_speed_request >= AMEC_MEMORY_MAX_STEP)
        {
            g_amec->mnfg_parms.mem_slew_counter++;
            l_slew_step = -AMEC_MEMORY_STEP_SIZE;
        }

        //check if we've reached the min setting and need to start going up
        else if(g_amec->mem_speed_request <= AMEC_MEMORY_MIN_STEP)
        {
            g_amec->mnfg_parms.mem_slew_counter++;
            l_slew_step = AMEC_MEMORY_STEP_SIZE;
        }

        l_vote = g_amec->mem_speed_request + l_slew_step;
        l_reason = AMEC_MEM_VOTING_REASON_SLEW;
    }

    // Store final vote and vote reason in g_amec
    g_amec->mem_throttle_reason = l_reason;
    g_amec->mem_speed_request = l_vote;

    //trace changes in memory throttling -- gm016
    if(l_reason != AMEC_MEM_VOTING_REASON_INIT)
    {
        if(!L_throttle_traced)
        {
            L_throttle_traced = TRUE;
            TRAC_INFO("Memory is being throttled. reason[%d] vote[%d] cent_expired[0x%02x] dimm_expired[0x%02x]",
                       l_reason, 
                       l_vote, 
                       G_cent_temp_expired_bitmap, 
                       G_dimm_temp_expired_bitmap); 
        }
    }
    else
    {
        if(L_throttle_traced)
        {
            L_throttle_traced = FALSE;
            TRAC_INFO("Memory is no longer being throttled");
        }
    }
    return;
}

// @rt001a
// Function Specification
//
// Name: amec_slv_check_perf
//
// Description: Slave OCC's Detect and log degraded performance errors
//              This function will run every tick.
//              
//
// Flow:    4-16-13     FN=amec_slv_check_perf.odg
//
// Thread: RealTime Loop
//
// Task Flags: 
//
// End Function Specification
void amec_slv_check_perf(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    static BOOLEAN          l_prev_failsafe_state = FALSE; 
    static BOOLEAN          l_prev_ovs_state = FALSE; 
    static BOOLEAN          l_prev_pcap_state = FALSE; 
    static ERRL_SEVERITY    l_pcap_sev =  ERRL_SEV_PREDICTIVE; 
    static BOOLEAN          l_throttle_traced = FALSE;
    static uint64_t         l_time = 0; //@rt003a

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    // @wb004 -- Verify that cores are at proper frequency
    amec_verify_pstate();

    do
    {
        // was frequency limited by power ?
        if ( G_non_dps_power_limited != TRUE )
        {
            if(l_throttle_traced)
            {
                TRAC_INFO("Frequency not limited by power algorithms anymore");
                l_throttle_traced = FALSE;
            }
            // we are done break and return
            break;
        }

        // frequency limited due to failsafe condition ?
        if ( AMEC_INTF_GET_FAILSAFE() == TRUE )
        {
            if ( l_prev_failsafe_state == TRUE)
            {
                // we are done break and return
                break;
            }
            else
            {
                // log this error ONLY ONCE per IPL
                l_prev_failsafe_state = TRUE;

                TRAC_ERR("Frequency limited due to failsafe condition(mode:%d, state:%d)",
                          CURRENT_MODE(), CURRENT_STATE()); // @at022c
                l_throttle_traced = TRUE;
                l_time = ssx_timebase_get(); //rt003a

                // log error that calls out OVS procedure 
                // set error severity to RRL_SEV_PREDICTIVE 

                /* @
                 * @errortype
                 * @moduleid    AMEC_SLAVE_CHECK_PERFORMANCE
                 * @reasoncode  INTERNAL_FAILURE
                 * @userdata1   Previous FailSafe State
                 * @userdata4   ERC_AMEC_SLAVE_FAILSAFE_STATE
                 * @devdesc     Frequency limited due to failsafe condition
                 */
                errlHndl_t l_errl = createErrl(AMEC_SLAVE_CHECK_PERFORMANCE, //modId
                                              INTERNAL_FAILURE,             //reasoncode
                                              ERC_AMEC_SLAVE_FAILSAFE_STATE,//Extended reason code
                                              ERRL_SEV_PREDICTIVE,          //Severity // @at020a
                                              NULL,                         //Trace Buf
                                              DEFAULT_TRACE_SIZE,           //Trace Size
                                              l_prev_failsafe_state,        //userdata1
                                              0);                           //userdata2


                addCalloutToErrl(   l_errl,
                                    ERRL_CALLOUT_TYPE_COMPONENT_ID,
                                    ERRL_COMPONENT_ID_OVERSUBSCRIPTION,
                                    ERRL_CALLOUT_PRIORITY_HIGH
                                );


                // and sets the consolidate action flag
                setErrlActions( l_errl, ERRL_ACTIONS_CONSOLIDATE_ERRORS );
 
                // Commit Error
                commitErrl(&l_errl);

                // we are done lets break
                break;
            }
        }

        
        // frequency limited due to oversubscription condition ?
        if ( AMEC_INTF_GET_OVERSUBSCRIPTION() == TRUE )
        {
            if ( l_prev_ovs_state == TRUE)
            {
                // we are done break and return
                break;
            }
            else
            {
                // log this error ONLY ONCE per IPL
                l_prev_ovs_state = TRUE;

                TRAC_ERR("Frequency limited due to oversubscription condition(mode:%d, state:%d)",
                          CURRENT_MODE(), CURRENT_STATE()); // @at022c
                l_throttle_traced = TRUE;
                l_time = ssx_timebase_get(); //@rt003a

                // log error that calls out OVS procedure
                // set error severity to RRL_SEV_PREDICTIVE

                // @wb003 -- Updated the RC to match the actual RC passed to createErrl()
                /* @
                 * @errortype
                 * @moduleid    AMEC_SLAVE_CHECK_PERFORMANCE
                 * @reasoncode  OVERSUB_LIMIT_ALERT
                 * @userdata1   Previous OVS State
                 * @userdata4   ERC_AMEC_SLAVE_OVS_STATE
                 * @devdesc     Frequency limited due to oversubscription condition
                 */
                errlHndl_t l_errl = createErrl(AMEC_SLAVE_CHECK_PERFORMANCE, //modId
                                              OVERSUB_LIMIT_ALERT,           //reasoncode  // @at020c
                                              ERC_AMEC_SLAVE_OVS_STATE,      //Extended reason code
                                              ERRL_SEV_PREDICTIVE,           //Severity  // @at020c
                                              NULL,                          //Trace Buf
                                              DEFAULT_TRACE_SIZE,            //Trace Size
                                              l_prev_ovs_state,              //userdata1
                                              0);                            //userdata2


                addCalloutToErrl(   l_errl,
                                    ERRL_CALLOUT_TYPE_COMPONENT_ID,
                                    ERRL_COMPONENT_ID_OVERSUBSCRIPTION,
                                    ERRL_CALLOUT_PRIORITY_HIGH
                                );


                // and sets the consolidate action flag
                setErrlActions( l_errl, ERRL_ACTIONS_CONSOLIDATE_ERRORS );

                // Commit Error
                commitErrl(&l_errl);

                // we are done lets break
                break;
            }
        }

        uint16_t l_snrBulkPwr = AMECSENSOR_PTR(PWR250US)->sample; //@rt003a

        // frequency limited due to system power cap condition ?
        if (( l_snrBulkPwr > (G_sysConfigData.pcap.system_pcap - PDROP_THRESH) ) //@rt003c
            && 
            ( G_sysConfigData.pcap.current_pcap == 0 ))
        {
            if ( l_prev_pcap_state == TRUE)
            {
                // we are done break and return
                break;
            }
            else
            {
                //@rt003c log this error ONLY ONCE per IPL 
                l_prev_pcap_state = TRUE;
                
                TRAC_ERR("Frequency limited due to power cap condition(mode:%d, state:%d)",
                         CURRENT_MODE(), CURRENT_STATE()); // @at022c

                TRAC_ERR("SnrBulkPwr %d > Sys Pcap %d ",l_snrBulkPwr,
                         G_sysConfigData.pcap.system_pcap ); //@rt003c @at022c

                TRAC_ERR("SnrFanPwr %d, SnrIOPwr %d, SnrStoragePwr %d, SnrGpuPrw %d ",
                        AMECSENSOR_PTR(PWR250USFAN)->sample,
                        AMECSENSOR_PTR(PWR250USIO)->sample,
                        AMECSENSOR_PTR(PWR250USSTORE)->sample,
                        AMECSENSOR_PTR(PWR250USGPU)->sample );

                TRAC_ERR("SnrProcPwr 0 %d, SnrProcPwr 1 %d, SnrProcPwr 2 %d, SnrProcPwr 3 %d",
                        g_amec->proc_snr_pwr[0],
                        g_amec->proc_snr_pwr[1],
                        g_amec->proc_snr_pwr[2],
                        g_amec->proc_snr_pwr[3] ); //@rt003a

                TRAC_ERR("SnrMemPwr 0 %d, SnrMemPwr 1 %d, SnrMemPwr 2 %d, SnrMemPwr 3 %d",
                        g_amec->mem_snr_pwr[0],
                        g_amec->mem_snr_pwr[1],
                        g_amec->mem_snr_pwr[2],
                        g_amec->mem_snr_pwr[3] ); //@rt003a


                l_throttle_traced = TRUE;
                l_time = ssx_timebase_get();

                // log error that calls out firmware and APSS procedure 
                // set error severity to l_pcap_sev 

                /* @
                 * @errortype
                 * @moduleid    AMEC_SLAVE_CHECK_PERFORMANCE
                 * @reasoncode  PCAP_THROTTLE_POWER_LIMIT
                 * @userdata1   Current Sensor Bulk Power
                 * @userdata2   System PCAP
                 * @userdata4   ERC_AMEC_SLAVE_POWERCAP
                 * @devdesc     Frequency limited due to PowerCap  condition
                 */
                errlHndl_t l_errl = createErrl(AMEC_SLAVE_CHECK_PERFORMANCE, //modId
                                              PCAP_THROTTLE_POWER_LIMIT,     //reasoncode @rt003c
                                              ERC_AMEC_SLAVE_POWERCAP,      //Extended reason code
                                              l_pcap_sev,                   //Severity
                                              NULL,                         //Trace Buf
                                              DEFAULT_TRACE_SIZE,           //Trace Size
                                              l_snrBulkPwr,                 //userdata1 @rt003c
                                              G_sysConfigData.pcap.system_pcap);//userdata2 @rt003c


                addCalloutToErrl(   l_errl,
                                    ERRL_CALLOUT_TYPE_COMPONENT_ID,
                                    ERRL_COMPONENT_ID_FIRMWARE,
                                    ERRL_CALLOUT_PRIORITY_HIGH
                                );
                
                addCalloutToErrl(   l_errl,
                                    ERRL_CALLOUT_TYPE_HUID,
                                    G_sysConfigData.apss_huid,
                                    ERRL_CALLOUT_PRIORITY_HIGH
                                );


                // and sets the consolidate action flag
                setErrlActions( l_errl, ERRL_ACTIONS_CONSOLIDATE_ERRORS );
 
                // then l_pcap_sev to informational
                l_pcap_sev = ERRL_SEV_INFORMATIONAL;
                
                // Commit Error
                commitErrl(&l_errl);

                // we are done lets break
                break;
            }
        }
        
        // trottle trace to every 3600 seconds (1hr = 3600000) @rt003c
        if(!l_throttle_traced && ( DURATION_IN_MS_UNTIL_NOW_FROM(l_time) > 3600000 ) )
        {
            TRAC_INFO("Frequency power limited due to transient condition: PowerLimited=%x, FailSafe=%x, OverSubScription=%x CurrentBulkPwr=%x",
            G_non_dps_power_limited, AMEC_INTF_GET_FAILSAFE(), AMEC_INTF_GET_OVERSUBSCRIPTION(), l_snrBulkPwr );
            l_throttle_traced = TRUE;
            
            l_time = ssx_timebase_get();
        }
    }
    while( 0 );

    return;
}

// @wb004
// Verifies that each core is at the correct frequency
// after they have had time to stabilize
void amec_verify_pstate()
{
    uint8_t                             l_core = 0;
    int8_t                              l_pstate_from_fmax = 0;
    gpe_bulk_core_data_t *              l_core_data_ptr;
    pmc_pmsr_ffcdc_data_t               l_pmc_pmsr_ffdc;
    errlHndl_t                          l_err = NULL;

    if ( (G_time_until_freq_check == 0) &&
            ( CURRENT_MODE() != OCC_MODE_DYN_POWER_SAVE ) &&
            ( CURRENT_MODE() != OCC_MODE_DYN_POWER_SAVE_FP ) &&
            (!G_sysConfigData.system_type.kvm))
    {
        // Reset the counter
        G_time_until_freq_check = FREQ_CHG_CHECK_TIME;

        // Convert fmax to the corresponding pstate
        l_pstate_from_fmax = proc_freq2pstate(g_amec->sys.fmax);

        for( l_core = 0; l_core < MAX_NUM_CORES; l_core++ )
        {
            // If the core isn't present, skip it
            if(!CORE_PRESENT(l_core))
            {
                l_pmc_pmsr_ffdc.pmsr_ffdc_data.data[l_core].value = 0;
                continue;
            }

            // Get pointer to core data
            l_core_data_ptr = proc_get_bulk_core_data_ptr(l_core);

            // Get the core's pmsr data
            l_pmc_pmsr_ffdc.pmsr_ffdc_data.data[l_core] = l_core_data_ptr->pcb_slave.pmsr;

            // Verify that the core is running at the correct frequency
            // If not, log an error
            if( (l_pstate_from_fmax != l_pmc_pmsr_ffdc.pmsr_ffdc_data.data[l_core].fields.local_pstate_actual) &&
                (l_pstate_from_fmax > l_pmc_pmsr_ffdc.pmsr_ffdc_data.data[l_core].fields.pv_min) &&
                (l_err == NULL) )
            {
                TRAC_ERR("Frequency mismatch in core %d: actual_ps[%d] req_ps[%d] fmax[%d] mode[%d].",
                    l_core,
                    l_pmc_pmsr_ffdc.pmsr_ffdc_data.data[l_core].fields.local_pstate_actual,
                    l_pstate_from_fmax,
                    g_amec->sys.fmax,
                    CURRENT_MODE());

                fill_pmc_ffdc_buffer(&l_pmc_pmsr_ffdc.pmc_ffcdc_data);

                /* @
                 * @moduleid   AMEC_VERIFY_FREQ_MID
                 * @reasonCode TARGET_FREQ_FAILURE
                 * @severity   ERRL_SEV_PREDICTIVE
                 * @userdata1  0
                 * @userdata2  0
                 * @userdata4  OCC_NO_EXTENDED_RC
                 * @devdesc    A core is not running at the expected frequency
                 */
                l_err = createErrl( AMEC_VERIFY_FREQ_MID,      // i_modId,
                                    TARGET_FREQ_FAILURE,       // i_reasonCode,
                                    OCC_NO_EXTENDED_RC,
                                    ERRL_SEV_UNRECOVERABLE,
                                    NULL,                      // tracDesc_t i_trace,
                                    DEFAULT_TRACE_SIZE,        // i_traceSz,
                                    0,                         // i_userData1,
                                    0);                        // i_userData2

                //Add firmware callout
                addCalloutToErrl(l_err,
                        ERRL_CALLOUT_TYPE_COMPONENT_ID,
                        ERRL_COMPONENT_ID_FIRMWARE,
                        ERRL_CALLOUT_PRIORITY_HIGH);

                //Add processor callout
                addCalloutToErrl(l_err,
                        ERRL_CALLOUT_TYPE_HUID,
                        G_sysConfigData.proc_huid,
                        ERRL_CALLOUT_PRIORITY_MED);

            }
        }

        if( l_err != NULL)
        {
            //Add our register dump to the error log
            addUsrDtlsToErrl(l_err,
                    (uint8_t*) &l_pmc_pmsr_ffdc,
                    sizeof(l_pmc_pmsr_ffdc),
                    ERRL_USR_DTL_STRUCT_VERSION_1,
                    ERRL_USR_DTL_BINARY_DATA);

            REQUEST_RESET(l_err);
        }
    }
}

// @wb004 -- Moved from main.c
// gm031
// Fills in a pmc ffdc buffer with lots of PMC related OCI and SCOM registers
void fill_pmc_ffdc_buffer(pmc_ffdc_data_t* i_ffdc_ptr)
{
    int i;
    uint32_t    l_rc, l_addr, l_data32;
    uint64_t    l_data64;

    //clear out the entire buffer
    memset(i_ffdc_ptr, 0, sizeof(pmc_ffdc_data_t));

    //first get the OCI accessible FFDC data
    for(i = 0; i < sizeof(G_pmc_ffdc_oci_addrs)/sizeof(uint32_t); i++)
    {
        l_addr = G_pmc_ffdc_oci_addrs[i];
        if(l_addr)
        {
            l_data32 = in32(l_addr);
        }
        else
        {
            //leave an entry with all zero address and data for eye catcher
            break;
        }

        //store address along with data for easier parsing
        i_ffdc_ptr->oci_regs[i].addr = l_addr;
        i_ffdc_ptr->oci_regs[i].data = l_data32;
    }

    //then get the SCOM accessible FFDC data
    for(i = 0; i < sizeof(G_pmc_ffdc_scom_addrs)/sizeof(uint32_t); i++)
    {
        l_addr = G_pmc_ffdc_scom_addrs[i];
        l_rc = (uint32_t)_getscom(l_addr, &l_data64, SCOM_TIMEOUT);
        if(l_rc)
        {
            //indicate there was a scom failure in collecting the data
            l_data64 = 0xFEEDB0B000000000ull;

            //store rc in lower word
            l_data64 |= l_rc;
        }

        //store address along with data for easier parsing
        i_ffdc_ptr->scom_regs[i].addr = l_addr;
        i_ffdc_ptr->scom_regs[i].data = l_data64;
    }
}
