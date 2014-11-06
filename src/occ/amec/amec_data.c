/******************************************************************************
// @file amec_data.c
// @brief Amec Data Import/Export Handling
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section _amec_data_c amec_data.c
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *   @th015             thallet   08/03/2012  New file
 *   @gs001             gsilva    08/03/2012  New file
 *   @fk001  879727     fmkassem  04/16/2013  OCC powercap support.
 *   @gs008  894661     gjsilva   08/08/2013  Initial support for DPS-FP mode
 *   @gs010  899888     gjsilva   09/24/2013  Process data format 0x13 from TMGT
 *   @gs012  903325     gjsilva   10/18/2013  Log Processor OT errors
 *   @gs015  905166     gjsilva   11/04/2013  Full support for IPS function
 *   @gm013  907548     milesg    11/22/2013  Memory therm monitoring support
 *   @gs019  908218     gjsilva   12/04/2013  Support cooling request architecture
 *   @rt004  908817     tapiar    12/11/2013  Update global data mask when master 
 *                                            code posts it to slave code (via data_write_pcap)
 *   @gs020  909320     gjsilva   12/12/2013  Support for VR_FAN thermal control
 *   @gm016  909061     milesg    12/10/2013  Support memory throttling due to temperature
 *   @gm017  909636     milesg    12/17/2013  Changes from mem throttle review
 *   @gs023  912003     gjsilva   01/16/2014  Generate VRHOT signal and control loop
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
#include <thrm_thread.h>

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************

//*************************************************************************
// Structures
//*************************************************************************

//*************************************************************************
// Globals
//*************************************************************************

//*************************************************************************
// Function Prototypes
//*************************************************************************

//*************************************************************************
// Functions
//*************************************************************************


// Function Specification
//
// Name:  AMEC_data_write_fcurr
//
// Description: 
//              
//
// Flow:              FN=
//
// Thread: RealTime Loop
//
// Task Flags: 
//
// End Function Specification
errlHndl_t AMEC_data_write_fcurr(const OCC_MODE i_mode)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    errlHndl_t                  l_err           = NULL;
    //UINT16                      l_freq_nom      = 0;
    OCC_MODE                   l_mode          = i_mode;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    // We'll never actually get a dynamic power save data package so just
    // use turbo as our mode if in dynamic power save since it
    // will have appropriate frequency.
    if((i_mode == OCC_MODE_DYN_POWER_SAVE) ||
       (i_mode == OCC_MODE_DYN_POWER_SAVE_FP))
    {
        l_mode = OCC_MODE_TURBO;
    }

    // If we're active we need to load this new range into DVFS MIN/MAX
    if(CURRENT_STATE() == OCC_STATE_ACTIVE)
    {
        // Use i_mode here since this function understands turbo
        l_err = amec_set_freq_range(i_mode);

        if(l_err)
        {
            // E>AMEC_data_write_fcurr: Error in call to amec_control_set_freq_range() i_mode:0x%X l_freq:%u
            //TRACE2(g_trac_amec,2118718763,i_mode,l_freq);
            //break;
        }
    }

    return l_err;
}

// Function Specification
//
// Name:  AMEC_data_write_thrm_thresholds
//
// Description: This function loads data from the Thermal Control Threshold
// data packet (format 0x13) into g_amec structure. This function should be
// called when OCC goes active or changes modes or goes in/out of Acoustic
// mode (ITE-only mode).
//              
//
// Flow:              FN=
//
// Thread: RealTime Loop
//
// Task Flags: 
//
// End Function Specification
errlHndl_t AMEC_data_write_thrm_thresholds(const OCC_MODE i_mode)
{

    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    errlHndl_t                  l_err = NULL;
    cmdh_thrm_thresholds_t      *l_data = NULL;
    cmdh_thrm_thresholds_set_t  *l_frudata = NULL;
    uint8_t                     l_dvfs_temp = 0;
    uint8_t                     l_error = 0;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    do
    {
        // Retrieve the thermal control threshold data
        l_err = DATA_get_thrm_thresholds(&l_data);
        if(l_err)
        {
            TRAC_ERR("AMEC_data_write_thrm_thresholds: Error retrieving thermal control threshold data");
            break;
        }

        // Notify thermal thread to update its local copy of the thermal thresholds
        THRM_thread_update_thresholds();

        l_frudata = l_data->data;

        // TODO: Need to check if acoustic mode has been enabled (ITE-only mode)

        // Store the processor thermal data
        if(i_mode == OCC_MODE_NOMINAL)
        {
            l_dvfs_temp = l_frudata[DATA_FRU_PROC].dvfs;
            l_error = l_frudata[DATA_FRU_PROC].error;
        }
        else
        {
            l_dvfs_temp = l_frudata[DATA_FRU_PROC].pm_dvfs;
            if(i_mode == OCC_MODE_TURBO) //gm016
            {
                //Need to log an error if we dvfs in static turbo mode (for mfg)
                l_error = l_dvfs_temp;
            }
            else
            {
                l_error = l_frudata[DATA_FRU_PROC].pm_error;
            }
        }
        // Store the DVFS thermal setpoint in 0.1 degrees C
        g_amec->thermalproc.setpoint = l_dvfs_temp * 10;
        // Store the error temperature for OT detection
        g_amec->thermalproc.ot_error = l_error;
        // Store the temperature timeout value
        g_amec->thermalproc.temp_timeout = l_frudata[DATA_FRU_PROC].max_read_timeout;

        TRAC_INFO("AMEC_data_write_thrm_thresholds: Setting %u as DVFS setpoint for processor",
                  l_dvfs_temp);

        // Store the Centaur thermal data
        if(i_mode == OCC_MODE_NOMINAL)
        {
            l_dvfs_temp = l_frudata[DATA_FRU_CENTAUR].dvfs;
            l_error = l_frudata[DATA_FRU_CENTAUR].error;
        }
        else
        {
            l_dvfs_temp = l_frudata[DATA_FRU_CENTAUR].pm_dvfs;
            if(i_mode == OCC_MODE_TURBO) //gm016
            {
                //Need to log an error if we throttle in static turbo mode (for mfg)
                l_error = l_dvfs_temp;
            }
            else
            {
                l_error = l_frudata[DATA_FRU_CENTAUR].pm_error;
            }
        }

        // Store the DVFS thermal setpoint in 0.1 degrees C
        g_amec->thermalcent.setpoint = l_dvfs_temp * 10;
        // Store the error temperature for OT detection
        g_amec->thermalcent.ot_error = l_error;
        // Store the temperature timeout value
        g_amec->thermalcent.temp_timeout = l_frudata[DATA_FRU_CENTAUR].max_read_timeout;

        TRAC_INFO("AMEC_data_write_thrm_thresholds: Setting %u as DVFS setpoint for Centaur",
                  l_dvfs_temp);

        // Store the DIMM thermal data
        if(i_mode == OCC_MODE_NOMINAL)
        {
            l_dvfs_temp = l_frudata[DATA_FRU_DIMM].dvfs;
            l_error = l_frudata[DATA_FRU_DIMM].error;
        }
        else
        {
            l_dvfs_temp = l_frudata[DATA_FRU_DIMM].pm_dvfs;
            if(i_mode == OCC_MODE_TURBO) //gm016
            {
                //Need to log an error if we throttle in static turbo mode (for mfg)
                l_error = l_dvfs_temp;
            }
            else
            {
                l_error = l_frudata[DATA_FRU_DIMM].pm_error;
            }
        }
        // Store the DVFS thermal setpoint in 0.1 degrees C
        g_amec->thermaldimm.setpoint = l_dvfs_temp * 10;
        // Store the error temperature for OT detection
        g_amec->thermaldimm.ot_error = l_error;
        // Store the temperature timeout value
        g_amec->thermaldimm.temp_timeout = l_frudata[DATA_FRU_DIMM].max_read_timeout;

        TRAC_INFO("AMEC_data_write_thrm_thresholds: Setting %u as DVFS setpoint for DIMM",
                  l_dvfs_temp);

        // Store the VRM thermal data
        g_amec->proc[0].vrfan_error_count = l_frudata[DATA_FRU_VRM].sample_error_count;
        g_amec->vrhotproc.setpoint = l_frudata[DATA_FRU_VRM].error_count;

        TRAC_INFO("AMEC_data_write_thrm_thresholds: Setting %u as DVFS setpoint for VRHOT",
                  g_amec->vrhotproc.setpoint);

    } while(0);

    return l_err;
}

// Function Specification
//
// Name:  AMEC_data_write_ips_config
//
// Description: This function loads data from the IPS Config data packet
// (format 0x11) into g_amec structure. This function should only by called by
// Master OCC firmware.
//              
//
// Thread: RealTime Loop
//
// End Function Specification
errlHndl_t AMEC_data_write_ips_cnfg(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    errlHndl_t                  l_err = NULL;
    cmdh_ips_config_data_t      *l_data = NULL;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    do
    {
        // Retrieve the thermal control threshold data
        l_err = DATA_get_ips_cnfg(&l_data);
        if(l_err)
        {
            TRAC_ERR("AMEC_data_write_ips_cnfg: Error retrieving IPS config data");
            break;
        }

        // Store Idle Power Saver settings for AMEC
        // Delay times should be stored in number of samples
        g_amec->mst_ips_parms.entry_delay = l_data->iv_delayTimeforEntry *
            AMEC_DPS_SAMPLING_RATE;
        g_amec->mst_ips_parms.exit_delay = l_data->iv_delayTimeforExit *
            AMEC_DPS_SAMPLING_RATE;

        // Utilization thresholds should be stored in hundredths of a percent
        g_amec->mst_ips_parms.entry_threshold = l_data->iv_utilizationForEntry * 100;
        g_amec->mst_ips_parms.exit_threshold = l_data->iv_utilizationForExit * 100;

        // Enable/disable Idle Power Saver
        g_amec->mst_ips_parms.enable = l_data->iv_ipsEnabled;

    } while(0);

    return l_err;
}

// Function Specification
//
// Name: AMEC_data_change
//
// Description: 
//              
//
// Flow: 03/04/13        FN= amec_data_change
//
// Thread: RealTime Loop
//
// Task Flags: 
//
// End Function Specification
errlHndl_t AMEC_data_change(const uint32_t i_data_mask)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    errlHndl_t                  l_err       = NULL;
    //    UINT32                      l_req_data  = 0;
    //    OCC_STATE                   l_cur_state = OCC_STATE_NOCHANGE;
    OCC_MODE                    l_cur_mode  = OCC_MODE_NOCHANGE;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    //l_cur_state = CURRENT_STATE();
    l_cur_mode = CURRENT_MODE();

    //If we received the frequency from TMGT 
    if(i_data_mask & DATA_MASK_FREQ_PRESENT)
    {
        // I>AMEC_data_change: We are active and receieved new freq packet!
        //TRACE0(g_trac_amec,1905158999);
        l_err = AMEC_data_write_fcurr(l_cur_mode);
        //if(l_err)
        //{
        //    E>AMEC_data_change: Error writing freq data! i_state_mask:0x%X i_mode:0x%X l_req_data:0x%X
        //    TRACE3(g_trac_amec,2446643683,
        //            l_cur_state,
        //            l_cur_mode,
        //            l_req_data);
        //}
    }
    else if(i_data_mask & DATA_MASK_THRM_THRESHOLDS)
    {
        l_err = AMEC_data_write_thrm_thresholds(l_cur_mode);

        if(l_err)
        {
            TRAC_ERR("AMEC_data_change: Error writing thermal threshold data!");
        }
    }
    else if(i_data_mask & DATA_MASK_IPS_CNFG)
    {
        l_err = AMEC_data_write_ips_cnfg();

        if(l_err)
        {
            TRAC_ERR("AMEC_data_change: Error writing IPS config data!");
        }
    }

    return l_err;
}

//////////////////////////
// Function Specification
//
// Name: amec_data_write_pcap
//
// Description: Function called by slave interrupt handler to collect pcap
//              data sent by Master OCC.
//              If a new packet is received from Master, it will be written to
//              G_sysConfigData.pcap, and g_amec->pcap will be updated too.
//              Otherwise nothing happens.
//              
//
// Flow:  04/16/13    FN= amec_data_write_pcap
//
// Thread: Interrupt Handler
//
// Changedby: @fk001a
//
// End Function Specification
void amec_data_write_pcap(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    static uint8_t  L_pcap_data_count = 0;
    uint16_t        l_customer = 0;
    uint16_t        l_system   = 0;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    //Check if Master sent a new packet of data.
    if(L_pcap_data_count != G_dcom_slv_inbox_doorbell_rx.pcap.pcap_data_count)
    {
        //Update counter
        L_pcap_data_count = G_dcom_slv_inbox_doorbell_rx.pcap.pcap_data_count;

        //Copy pcap data received from Master OCC to G_sysConfigData
        memcpy(&(G_sysConfigData.pcap),&(G_dcom_slv_inbox_doorbell_rx.pcap),
               sizeof(pcap_config_data_t)); 

        //Affects ITE ONLY: Check if it's ok (1) to exit the oversubscribed state
        if(1 == G_sysConfigData.pcap.unthrottle)
        {
            //Clear throttle flag
            g_amec->oversub_status.cmeThrottleLatchAmec = 0;
        }

        //Check node power cap requested by customer/system.  
        // 0 means there is no pcap for that parameter.
        if(0 == G_sysConfigData.pcap.current_pcap)
        {
            l_customer = 0xFFFF;
        }
        else
        {
            l_customer = G_sysConfigData.pcap.current_pcap;
        }

        //Check fixed node power cap required by the system.
        // 0 means there is no pcap for that parameter.
        if(0 == G_sysConfigData.pcap.system_pcap)
        {
            l_system = 0xFFFF;
        }
        else
        {
            l_system = G_sysConfigData.pcap.system_pcap;
        }

        //Set the normal node pcap to the least of both system and customer pcap.
        if(l_customer < l_system)
        {
            g_amec->pcap.norm_node_pcap = l_customer;
        }
        else
        {
            g_amec->pcap.norm_node_pcap = l_system;
        }

        ///////////////////////////////
        //Set the oversubscription pcap
        if(0 != G_sysConfigData.pcap.oversub_pcap)
        {
            g_amec->pcap.ovs_node_pcap = G_sysConfigData.pcap.oversub_pcap;
        }
        else
        { 
            g_amec->pcap.ovs_node_pcap = G_sysConfigData.pcap.hard_min_pcap;
        }

        //Oversubscription pcap can NOT be higher than a customer set pcap.
        if(g_amec->pcap.ovs_node_pcap > l_customer)
        {
            g_amec->pcap.ovs_node_pcap = l_customer;
        }

        // update data mask notifying we got pcap information @rt004a
        extern data_cnfg_t * G_data_cnfg;
        G_data_cnfg->data_mask |= DATA_MASK_PCAP_PRESENT;
        TRAC_IMP("amec_data_write: PCAP Config data: pcap[%d]: data_mask[%x]", g_amec->pcap.norm_node_pcap, G_data_cnfg->data_mask);
    }
}
