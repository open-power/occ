/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/dcmcom/dcmcom.c $                                 */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2021,2024                        */
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
#include <errl.h>
#include <trac.h>
#include "dcmcom.h"
#include "ocb_register_addresses.h"
#include "ocb_firmware_registers.h"
#include <occ_sys_config.h>
#include <sensor.h>
#include "wof.h"

// #define DCM_DEBUG

extern amec_wof_t * g_wof;

icc_msg_t g_current_icc_msg = {0};
uint8_t g_icc_seq = 0;
uint8_t g_icc_not_changing = 0;

void dcmicc_update()
{
    icc_msg_t icc_msg;
    ocb_woficstat_t status0;
    ocb_woficstat_t status1;

    // The DCM ICC (WOFIC) will only be configured by HCODE on DCM configurations
    // To manually enable this interface: putscom pu -all 0006c780 8024e80000000000
    // 
    // OCC will disable it if errors are encountered.
    if(in32(OCB_WOFICCTRL) != 0) // Interface configured
    {
        do
        {
            status0.value = in32(OCB_WOFICSTAT);
            icc_msg.value = in64(OCB_WOFICRD); // not autonomous
            status1.value = in32(OCB_WOFICSTAT);

            if(status0.fields.interchip_rx_ongoing == 1)
            {
                // if just finished get a fresh copy of the data
                if(status1.fields.interchip_rx_ongoing == 0)
                {
                    icc_msg.value = in64(OCB_WOFICRD);
                }
                else //  (status1.fields.interchip_rx_ongoing == 1)
                {
                    ssx_sleep(SSX_MICROSECONDS(1));

                    status0.value = in32(OCB_WOFICSTAT);
                    if(status0.fields.interchip_rx_ongoing == 1)
                    {
                        // something is wrong should not take so long
                        TRAC_ERR("task_dcm_icc: Timeout waiting for RX to complete");
                        out32(OCB_WOFICCTRL,0);  // Disable interface
                        break;
                    }
                    else
                    {
                        icc_msg.value = in64(OCB_WOFICRD);
                    }
                }
            }

            if(status1.fields.interchip_ecc_ue)
            {
                // Currently ECC checking is not enabled.
                TRAC_ERR("task_dcm_icc: ECC uncorrectable error encountered");
                // disble interface.
                out32(OCB_WOFICCTRL,0);
                break;
            }
            else if(g_current_icc_msg.seq == icc_msg.seq)
            {
                if(g_icc_not_changing < ICC_SEQ_UNCHANGED_MAX_COUNT)
                {
#ifdef DCM_DEBUG
                    TRAC_INFO("task_dcm_icc: Message sequence didn't change");
#endif
                    ++g_icc_not_changing;
                }
            }
            else
            {
                g_current_icc_msg.value = icc_msg.value;
                g_icc_not_changing = 0;
            }

            status0.value = in32(OCB_WOFICSTAT);
            if(status0.fields.interchip_tx_ongoing)
            {
                // Should never see this if last send was a tick ago
                // Device is already busy sending
                TRAC_ERR("task_dcm_icc: Attempting to send ICC data when already busy");
                out32(OCB_WOFICCTRL,0);  // Disable interface
                break;
            }
            else
            {
                icc_msg.value = 0;
                ++g_icc_seq;
                icc_msg.seq = g_icc_seq;
                icc_msg.average_freq = g_wof->avg_freq_mhz;
                icc_msg.clip_pstate = g_wof->f_clip_ps;

                sensor_t  *sensor = getSensorByGsid(TEMPPROCTHRM);

                icc_msg.proc_temperature = sensor->sample * 10;

                // High word must be written last - triggers TX
                out32(OCB_WOFICWD+4,icc_msg.low_word);
                out32(OCB_WOFICWD,icc_msg.high_word);
            }
        } while(0);
    }
}
