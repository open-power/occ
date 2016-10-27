/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/pss/avsbus.c $                                    */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2015                        */
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

#include "avsbus.h"
#include <trac.h>
#include <occ_common.h>
#include <comp_ids.h>
#include <occ_sys_config.h>
#include <trac_interface.h>
#include "ocb_register_addresses.h"
#include "occ_service_codes.h"
#include "pss_service_codes.h"
#include "ssx.h"
#include "occ_util.h"

//#define AVSDEBUG

#ifdef AVSDEBUG
  #define AVS_DBG(frmt,args...)  TRAC_INFO(frmt,##args)
#else
  #define AVS_DBG(frmt,args...)
#endif


// AVS Bus usage will be determined after receiving config data from TMGT
bool G_avsbus_vdd_monitoring = FALSE;
bool G_avsbus_vdn_monitoring = FALSE;

// The following constants are used after a command is put on the bus.
// The code will wait for the o2s_ongoing bit to change (which indicates cmd completion).
// Max wait time will be: O2S_ONGOING_CHECK_ATTEMPTS * O2S_ONGOING_CHECK_DELAY_NS (in nanoseconds)
const uint32_t    O2S_ONGOING_CHECK_ATTEMPTS = 10;
const uint32_t    O2S_ONGOING_CHECK_DELAY_NS = 100;

// Number of read failures allowed before elog is created and reset requested.
// This should be no longer than 4ms (or it will impact WOF calculations)
// (readings are taken every 500ms => 500us * 8 = 4ms)
const uint8_t     MAX_READ_ATTEMPTS = 8;

// NOTE: OCC must use Bridge B, because Bridge A is reserved for PGPE

// AVS Bus setup that must be done once (common between read/write operations)
void avsbus_init()
{
    uint64_t value;

    TRAC_INFO("avsbus_init: Vdd=%c Vdn=%c",
              G_avsbus_vdd_monitoring?'Y':'N', G_avsbus_vdn_monitoring?'Y':'N');

    bool bus0_monitoring = FALSE;
    bool bus1_monitoring = FALSE;
    if (G_avsbus_vdd_monitoring)
    {
        if (0 == G_sysConfigData.avsbus_vdd.bus)
        {
            bus0_monitoring = TRUE;
        }
        else
        {
            bus1_monitoring = TRUE;
        }
    }
    if (G_avsbus_vdn_monitoring)
    {
        if (0 == G_sysConfigData.avsbus_vdn.bus)
        {
            bus0_monitoring = TRUE;
        }
        else
        {
            bus1_monitoring = TRUE;
        }
    }

    // Write O2SCTRLF_[a][n]
    //  o2s_frame_size = 0x20 (32d)
    //  o2s_out_count1 = 0x20 (32d) - 5b header, 8b command type/select,16b info, 3b CRC
    //  o2s_in_delay1 = 0xFF (long delay - no read data)
    //  o2s_in_count1 = 0x0 (no read data)
    value = 0x820FC00000000000;
    if (bus0_monitoring)
    {
        out64(OCB_O2SCTRLF0B, value);
    }
    if (bus1_monitoring)
    {
        out64(OCB_O2SCTRLF1B, value);
    }

    // Write O2SCTRLS_[a][n]
    //  o2s_out_count2 = 0x0 (no output)
    //  o2s_in_delay2 = 0x0 (no delay - immediate read data)
    //  o2s_in_count2 = 0x20 - 3b Slave Ack, 5b StatusResp,16b info, 5b Reserved, 3b CRC
    value = 0x0000800000000000;
    if (bus0_monitoring)
    {
        out64(OCB_O2SCTRLS0B, value);
    }
    if (bus1_monitoring)
    {
        out64(OCB_O2SCTRLS1B, value);
    }

    // Write O2SCTRL2_[a][n]
    //  o2s_inter_frame_delay = 0 (Wait 1 SPI clock). The AVSBus spec does not define any inter-frame delay so set this to the smallest value.
    //  Note: the value 0 is the hardware reset value and, thus, this step can be omitted if the value desired to to be left at 0.
    value = 0x0000000000000000;
    if (bus0_monitoring)
    {
        out64(OCB_O2SCTRL20B, value);
    }
    if (bus1_monitoring)
    {
        out64(OCB_O2SCTRL21B, value);
    }

    // Write O2SCTRL1_[a][n]
    //  0     o2s_bridge_enable = 1 (make the O2S bridge active if not already)
    //  1     reserved
    //  2     o2s_cpol = 0 (positive active clock)
    //  3     o2s_cpha = 0 (first edge data launch/sample)
    //  4:13  o2s_clock_divider = set based on the nest frequency for the desired frequency of 10MHz (assumed speed) per O2SCTRL1_[a][n] description.
    //                          = (nest_freq / (SPI_freq*8))-1 = (500M/(10M*8))-1 = 5.25
    //  14:16 reserved
    //  17    o2s_nr_of_frames = 1 (2 frames to account for the first and second frames of an AVSBus command)
    //  18:63 reserved
    //  1r00DDDD DDDDDDrr r1rrrrrr rrrrrrrrr
    value = 0x8014400000000000;
    // TODO: RTC163992 - Determine correct o2s_clock_divider based on nest freq
    if (bus0_monitoring)
    {
        out64(OCB_O2SCTRL10B, value);
    }
    if (bus1_monitoring)
    {
        out64(OCB_O2SCTRL11B, value);
    }
}


// Calculate and update command with correct CRC
// Function which generates a 3 bit CRC value for 29 bit data
// from: ekb/chips/p9/procedures/hwp/lib/p9_avsbus_lib.C
#define AVS_CRC_DATA_MASK 0xFFFFFFF800000000
void avs_crc_update(uint64_t *io_avs_cmd)
{
    //Polynomial= x^3 + x^1 + x^0 = 1*x^3 + 0*x^2 + 1*x^1 + 1*x^0 = divisor(1011)

    uint64_t l_crc_value = 0;
    uint64_t l_polynomial = 0xB000000000000000;
    uint64_t l_msb =        0x8000000000000000;

    l_crc_value = *io_avs_cmd & AVS_CRC_DATA_MASK;

    while (l_crc_value & AVS_CRC_DATA_MASK)
    {
        if (l_crc_value & l_msb)
        {
            //if l_msb is 1'b1, divide by l_polynomial and shift l_polynomial
            // to the right
            l_crc_value = l_crc_value ^ l_polynomial;
            l_polynomial = l_polynomial >> 1;
        }
        else
        {
            // if l_msb is zero, shift l_polynomial
            l_polynomial = l_polynomial >> 1;
        }

        l_msb = l_msb >> 1;
    }

    // Add CRC to avs command (lower 3 bits of cmd)
    *io_avs_cmd |= l_crc_value;
}


#define ERRORCOUNT_MAXTYPES  2
#define ERRORCOUNT_MAXCMDS   2
// Read and return the voltage or current for specified rail
// (voltage units are mV, current units are in 10mA)
uint16_t avsbus_read(const avsbus_type_e i_type,
                     const avsbus_cmdtype_e i_cmdtype)
{
    if (isSafeStateRequested())
    {
        // No need to attempt read if OCC will be reset
        return 0;
    }

    uint16_t o_reading = 0;
    bool l_failure = FALSE;
    avsbusData_t l_data;

    // Create error array for each type (Vdd/Vdn) and command (Voltage/Current)
    uint8_t l_cmd_index = 0;
    char l_trace_cmd = 'V';
    if (i_cmdtype == AVSBUS_CURRENT)
    {
        l_cmd_index = 1;
        l_trace_cmd = 'C';
    }
    static uint32_t L_error_count[ERRORCOUNT_MAXTYPES][ERRORCOUNT_MAXCMDS] = {{0}};
    uint32_t *      l_error_count = &L_error_count[i_type][l_cmd_index];

    char l_trace_type = 'd';
    if (AVSBUS_VDD == i_type)
    {
        l_data = G_sysConfigData.avsbus_vdd;
    }
    else
    {
        l_data = G_sysConfigData.avsbus_vdn;
        l_trace_type = 'n';
    }

#ifdef AVSDEBUG
    static bool L_traced[ERRORCOUNT_MAXTYPES][ERRORCOUNT_MAXCMDS] = {{0}};
    bool *      l_traced = &L_traced[i_type][l_cmd_index];
    if (!*l_traced)
    {
        TRAC_INFO("avsbus_read: Vd%c %c - bus[%d] rail[%d]",
                  l_trace_type, l_trace_cmd, l_data.bus, l_data.rail);
    }
#endif

    // Determine register based on the bus number
    uint32_t o2scmd_reg = OCB_O2SCMD0B;
    uint32_t o2swd_reg = OCB_O2SWD0B;
    uint32_t o2sst_reg = OCB_O2SST0B;
    uint32_t o2srd_reg = OCB_O2SRD0B;
    if (1 == l_data.bus)
    {
        o2scmd_reg = OCB_O2SCMD1B;
        o2swd_reg = OCB_O2SWD1B;
        o2sst_reg = OCB_O2SST1B;
        o2srd_reg = OCB_O2SRD1B;
    }

    // Write O2SCMD[a][n]
    //   o2s_clear_sticky_bits = 1
    uint64_t value;
    value = 0x4000000000000000;
    out64(o2scmd_reg, value);

    // Write O2SWD[a][n] - write commands and initiate hardware operation
    //   o2s_wdata with content
    // AVS Bus command (read voltage/current):
    //   0:1   StartCode = 0b01
    //   2:3   Cmd = 0b11 (read)
    //   4     CmdGroup = 0b0 (AVSBus)
    //   5:8   CmdDataType (read/write voltage or read current)
    //   9:12  Select (Rail Select)
    //   13:28 CmdData (reserved / must be 1s)
    //   29:31 CRC
    //   01110DDD DRRRR111 11111111 11111CCC
    value = 0x7007FFF800000000 | ((uint64_t) i_cmdtype << 55) | ((uint64_t)l_data.rail << 51);
    avs_crc_update(&value);
    out64(o2swd_reg, value);

    // HW: Wait for bus op to complete
    // HW: arbitration between two bridges
    // HW: o2s_ongoning: 0 -> 1
    // HW: execution completes
    // HW: o2s_ongoing 1 -> 0

    // Poll waiting for o2s_ongoing to change to 0 or an error bit is set
    enum occReasonCode rc = OCC_SUCCESS_REASON_CODE;
    uint64_t l_status = in64(o2sst_reg);
    // OCC O2S Status Register
    //   0     o2s_ongoing
    //   1:4   reserved
    //   5     write_while_bridge_busy_error
    //   6     reserved
    //   7     FSM error
    //   8:63  reserved
    // GrrrrBrF rrrrrrrr rrrrrrrr rrrrrrrr
    uint32_t l_max_attempts = O2S_ONGOING_CHECK_ATTEMPTS;

    while ((1 == (l_status >> 63)) &&
           (0 == (l_status & 0x0500000000000000)))
    {
        if (l_max_attempts-- == 0)
        {
            l_failure = TRUE;
            (*l_error_count)++;
            if ((*l_error_count == 1) || (*l_error_count == MAX_READ_ATTEMPTS))
            {
                TRAC_ERR("avsbus_read: Vd%c %c timeout waiting for o2s_ongoing change O2SST[0x%08X] = [0x%08X] / Command[0x%08X]",
                         l_trace_type, l_trace_cmd, o2sst_reg, WORD_HIGH(l_status), WORD_HIGH(value));
                /*
                 * @errortype
                 * @moduleid    PSS_MID_AVSBUS_READ
                 * @reasoncode  AVSBUS_TIMEOUT
                 * @userdata1   AVS Bus type/bus/rail
                 * @userdata2   status
                 * @devdesc     task_apss_complete_pwr_meas schedule failed
                 * @devdesc     Timeout when reading AVS Bus
                 */
                rc = AVSBUS_TIMEOUT;
            }
            break;
        }
        // TODO: RTC 163992
        //ssx_sleep(SSX_NANOSECONDS(O2S_ONGOING_CHECK_DELAY_NS));
        l_status = in64(o2sst_reg);
    }

    if (0 != (l_status & 0x0500000000000000))
    {
        l_failure = TRUE;
        (*l_error_count)++;
        if ((*l_error_count == 1) || (*l_error_count == MAX_READ_ATTEMPTS))
        {
            // error bit was set
            TRAC_ERR("avsbus_read: Error found in Vd%c %c O2SST[0x%08X] = [0x%08X] / Command[0x%08X]",
                     l_trace_type, l_trace_cmd, o2sst_reg, WORD_HIGH(l_status), WORD_HIGH(value));
            /*
             * @errortype
             * @moduleid    PSS_MID_AVSBUS_READ
             * @reasoncode  AVSBUS_ERROR
             * @userdata1   AVS Bus type/bus/rail
             * @userdata2   status
             * @devdesc     Error encountered when reading AVS Bus
             */
            rc = AVSBUS_ERROR;
        }
    }

    if (FALSE == l_failure)
    {
        // Read the response data
        value = in64(o2srd_reg);

        // Check for valid command operation and extract read data
        // AVS Bus response (read voltage or current):
        //   0:1   SlaveAck (0b00 from slave indicates good CRC and action was taken)
        //   2     0
        //   3:7   StatusResp
        //   8:23  CmdData (LSB = 1mV or 10mA)
        //   24:28 Reserved (must be all 1s)
        //   29:31 CRC
        //   AA0SSSSS VVVVVVVV VVVVVVVV 11111CCC
        if (0 == (value & 0xC000000000000000))
        {
            o_reading = (value >> 40) & 0x0000FFFF;
#ifdef AVSDEBUG
            if (!*l_traced)
            {
                if (i_cmdtype == AVSBUS_VOLTAGE)
                {
                    TRAC_INFO("avsbus_read: Successfully read Vd%c voltage %dmV [0x%08X]",
                              l_trace_type, o_reading, WORD_HIGH(value));
                }
                else
                {
                    TRAC_INFO("avsbus_read: Successfully read Vd%c current %dx10mA [0x%08X]",
                              l_trace_type, o_reading, WORD_HIGH(value));
                }
            }
#endif
            if (*l_error_count)
            {
                // Trace and clear the error count
                TRAC_INFO("avsbus_read: Successfully read Vd%c %c [0x%08X] (error count=%d)",
                          l_trace_type, l_trace_cmd, WORD_HIGH(value), *l_error_count);
                *l_error_count = 0;
            }
        }
        else
        {
            l_failure = TRUE;
            (*l_error_count)++;
            if ((*l_error_count == 1) || (*l_error_count == MAX_READ_ATTEMPTS))
            {
                TRAC_ERR("avsbus_read: SlaveAck reported no action taken[0x%08X]", WORD_HIGH(value));
                rc = AVSBUS_ERROR;
            }
        }
    }

    if (l_failure)
    {
        if (*l_error_count == MAX_READ_ATTEMPTS)
        {
            TRAC_ERR("avsbus_read: Reached %d consecutive Vd%c %c errors, requesting reset",
                     *l_error_count, l_trace_type, l_trace_cmd);
            G_avsbus_vdd_monitoring = FALSE;
            G_avsbus_vdn_monitoring = FALSE;
            enum occExtReasonCode exrc = ERC_AVSBUS_VDD_VOLTAGE_FAILURE;
            if (AVSBUS_VDD == i_type)
            {
                if (i_cmdtype == AVSBUS_CURRENT)
                {
                    exrc = ERC_AVSBUS_VDD_CURRENT_FAILURE;
                }
            }
            else
            {
                if (i_cmdtype == AVSBUS_CURRENT)
                {
                    exrc = ERC_AVSBUS_VDN_CURRENT_FAILURE;
                }
                else
                {
                    exrc = ERC_AVSBUS_VDN_VOLTAGE_FAILURE;
                }
            }
            errlHndl_t l_err = createErrl(PSS_MID_AVSBUS_READ,
                                          rc,
                                          exrc,
                                          ERRL_SEV_PREDICTIVE,
                                          NULL,
                                          DEFAULT_TRACE_SIZE,
                                          (i_type << 16) | (l_data.bus << 8) | l_data.rail,
                                          WORD_HIGH(l_status));
            // add processor callout and request reset
            addCalloutToErrl(l_err,
                             ERRL_CALLOUT_TYPE_HUID,
                             G_sysConfigData.proc_huid,
                             ERRL_CALLOUT_PRIORITY_MED);
            REQUEST_RESET(l_err);
        }
    }
#ifdef AVSDEBUG
    *l_traced = TRUE; // DEBUG
#endif

    return o_reading;

} // end avsbus_read()




