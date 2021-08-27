/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/amec/amec_data.c $                                */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2020                        */
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
#include <pgpe_interface.h>
#include <pstates_occ.H>

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
extern uint8_t G_occ_interrupt_type;

//*************************************************************************
// Function Prototypes
//*************************************************************************

//*************************************************************************
// Functions
//*************************************************************************


// Function Specification
//
// Name:  AMEC_data_write_thrm_thresholds
//
// Description: This function loads data from the Thermal Control Threshold
// data packet (format 0x13) into g_amec structure. This function should be
// called when OCC goes active or changes modes
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
    errlHndl_t                      l_err = NULL;
    cmdh_thrm_thresholds_v30_t     *l_data = NULL;
    cmdh_thrm_thresholds_set_v30_t *l_frudata = NULL;
    uint8_t                         l_dvfs_temp = 0;
    uint8_t                         l_error = 0;

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

        l_frudata = l_data->data;

        //
        // Store the processor thermal data
        //
        l_dvfs_temp = l_frudata[DATA_FRU_PROC].dvfs;
        if(OCC_MODE_STATIC_FREQ_POINT == i_mode)
        {
            l_error = l_dvfs_temp;
        }
        else
        {
            l_error = l_frudata[DATA_FRU_PROC].error;
        }
        // Store the DVFS thermal setpoint in 0.1 degrees C
        g_amec->thermalproc.setpoint = l_dvfs_temp * 10;
        // Store the error temperature for OT detection
        g_amec->thermalproc.ot_error = l_error;
        // Store the temperature timeout value
        g_amec->thermalproc.temp_timeout = l_frudata[DATA_FRU_PROC].max_read_timeout;

        TRAC_INFO("AMEC_data_write_thrm_thresholds: Processor setpoints - DVFS: %u, Error: %u",
                  l_dvfs_temp, l_error);

        //
        // Store the membuf thermal data
        //

        // Store the DVFS thermal setpoint in 0.1 degrees C
        l_dvfs_temp = l_frudata[DATA_FRU_MEMBUF].dvfs;
        g_amec->thermalmembuf.setpoint = l_dvfs_temp * 10;
        // Store the error temperature for OT detection
        l_error = l_frudata[DATA_FRU_MEMBUF].error;
        g_amec->thermalmembuf.ot_error = l_error;
        // Store the temperature timeout value
        g_amec->thermalmembuf.temp_timeout = l_frudata[DATA_FRU_MEMBUF].max_read_timeout;

        TRAC_INFO("AMEC_data_write_thrm_thresholds: membuf setpoints - DVFS: %u, Error: %u",
                  l_dvfs_temp, l_error);

        //
        // Store the DIMM thermal data
        //

        // Store the DVFS thermal setpoint in 0.1 degrees C
        l_dvfs_temp = l_frudata[DATA_FRU_DIMM].dvfs;
        g_amec->thermaldimm.setpoint = l_dvfs_temp * 10;
        // Store the error temperature for OT detection
        l_error = l_frudata[DATA_FRU_DIMM].error;
        g_amec->thermaldimm.ot_error = l_error;
        // Store the temperature timeout value
        g_amec->thermaldimm.temp_timeout = l_frudata[DATA_FRU_DIMM].max_read_timeout;

        TRAC_INFO("AMEC_data_write_thrm_thresholds: DIMM setpoints - DVFS: %u, Error: %u",
                  l_dvfs_temp, l_error);

        //
        // Store the Memctrl+DIMM thermal dat
        //
        l_dvfs_temp = l_frudata[DATA_FRU_MEMCTRL_DRAM].dvfs;
        g_amec->thermalmcdimm.setpoint = l_dvfs_temp * 10;
        // Store the error temperature for OT detection
        l_error = l_frudata[DATA_FRU_MEMCTRL_DRAM].error;
        g_amec->thermalmcdimm.ot_error = l_error;
        // Store the temperature timeout value
        g_amec->thermalmcdimm.temp_timeout = l_frudata[DATA_FRU_DIMM].max_read_timeout;

        TRAC_INFO("AMEC_data_write_thrm_thresholds: MC+DIMM setpoints - DVFS: %u, Error: %u",
                  l_dvfs_temp, l_error);
        //
        // Store the PMIC thermal data
        //

        // Store the DVFS thermal setpoint in 0.1 degrees C
        l_dvfs_temp = l_frudata[DATA_FRU_PMIC].dvfs;
        g_amec->thermalpmic.setpoint = l_dvfs_temp * 10;
        // Store the error temperature for OT detection
        l_error = l_frudata[DATA_FRU_PMIC].error;
        g_amec->thermalpmic.ot_error = l_error;
        // Store the temperature timeout value
        g_amec->thermalpmic.temp_timeout = l_frudata[DATA_FRU_PMIC].max_read_timeout;

        TRAC_INFO("AMEC_data_write_thrm_thresholds: PMIC setpoints - DVFS: %u, Error: %u",
                  l_dvfs_temp, l_error);


        // Store the DVFS thermal setpoint in 0.1 degrees C
        l_dvfs_temp = l_frudata[DATA_FRU_MEMCTRL_EXT].dvfs;
        g_amec->thermalmcext.setpoint = l_dvfs_temp * 10;
        // Store the error temperature for OT detection
        l_error = l_frudata[DATA_FRU_MEMCTRL_EXT].error;
        g_amec->thermalmcext.ot_error = l_error;
        // Store the temperature timeout value
        g_amec->thermalmcext.temp_timeout = l_frudata[DATA_FRU_MEMCTRL_EXT].max_read_timeout;

        TRAC_INFO("AMEC_data_write_thrm_thresholds: External MC setpoints - DVFS: %u, Error: %u",
                  l_dvfs_temp, l_error);

        // Store the VRM Vdd thermal data
        //

        // Store the DVFS thermal setpoint in 0.1 degrees C
        l_dvfs_temp = l_frudata[DATA_FRU_VRM_VDD].dvfs;
        if(OCC_MODE_STATIC_FREQ_POINT == i_mode)
        {
            l_error = l_dvfs_temp;
        }
        else
        {
            l_error = l_frudata[DATA_FRU_VRM_VDD].error;
        }
        g_amec->thermalvdd.setpoint = l_dvfs_temp * 10;
        // Store the error temperature for OT detection
        g_amec->thermalvdd.ot_error = l_error;
        // Store the temperature timeout value
        g_amec->thermalvdd.temp_timeout = l_frudata[DATA_FRU_VRM_VDD].max_read_timeout;

        TRAC_INFO("AMEC_data_write_thrm_thresholds: VRM Vdd setpoints - DVFS: %u, Error: %u",
                  l_dvfs_temp, l_error);

        // Store the Processor IO thermal data
        //

        l_dvfs_temp = l_frudata[DATA_FRU_PROC_IO].dvfs;
        if(OCC_MODE_STATIC_FREQ_POINT == i_mode)
        {
            l_error = l_dvfs_temp;
        }
        else
        {
            l_error = l_frudata[DATA_FRU_PROC_IO].error;
        }
        // Store the DVFS thermal setpoint in 0.1 degrees C
        g_amec->thermalprocio.setpoint = l_dvfs_temp * 10;
        // Store the error temperature for OT detection
        g_amec->thermalprocio.ot_error = l_error;
        // Store the temperature timeout value
        g_amec->thermalprocio.temp_timeout = l_frudata[DATA_FRU_PROC_IO].max_read_timeout;

        TRAC_INFO("AMEC_data_write_thrm_thresholds: Processor IO setpoints - DVFS: %u, Error: %u",
                  l_dvfs_temp, l_error);

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
    OCC_MODE                    l_cur_mode  = OCC_MODE_NOCHANGE;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    //l_cur_state = CURRENT_STATE();
    l_cur_mode = CURRENT_MODE();

    if(i_data_mask & DATA_MASK_THRM_THRESHOLDS)
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
// Thread: Interrupt Handler
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
    //Set the slaves local copy of ppb_fmax to that received from Master OCC.
    g_amec->proc[0].pwr_votes.ppb_fmax = G_dcom_slv_inbox_doorbell_rx.ppb_fmax;

    //Check if Master sent a new packet of data.
    if(L_pcap_data_count != G_dcom_slv_inbox_doorbell_rx.pcap.pcap_data_count)
    {
        //Update counter
        L_pcap_data_count = G_dcom_slv_inbox_doorbell_rx.pcap.pcap_data_count;

        //Copy pcap data received from Master OCC to G_sysConfigData
        memcpy(&(G_sysConfigData.pcap),&(G_dcom_slv_inbox_doorbell_rx.pcap),
               sizeof(pcap_config_data_t));

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

        //for all new pcap data setting: If KVM, update the OPAL dynamic data
        if(G_sysConfigData.system_type.kvm)
        {
            ssx_semaphore_post(&G_dcomThreadWakeupSem);
        }

        // update data mask notifying we got pcap information
        extern data_cnfg_t * G_data_cnfg;
        G_data_cnfg->data_mask |= DATA_MASK_PCAP_PRESENT;
        TRAC_IMP("amec_data_write: PCAP Config data: pcap[%d]: data_mask[%x]",
                 g_amec->pcap.norm_node_pcap, G_data_cnfg->data_mask);
    }
}

/*----------------------------------------------------------------------------*/
/* End                                                                        */
/*----------------------------------------------------------------------------*/
