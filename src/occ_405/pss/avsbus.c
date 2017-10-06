/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/pss/avsbus.c $                                    */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2017                        */
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
#include "cmdh_fsp_cmds_datacnfg.h"

//#define AVSDEBUG

#ifdef AVSDEBUG
#define DEBUG_TRACE_MAX  2
static bool G_trace_scoms = TRUE;
  #define AVS_DBG(frmt,args...)  TRAC_INFO(frmt,##args)
  #define DEBUG_IN32(reg, result, name) if (G_trace_scoms) { TRAC_INFO(" in32(%08X) returned 0x%08X  "name, reg, result); }
  #define DEBUG_OUT32(reg, value, name) if (G_trace_scoms) { TRAC_INFO("out32(%08X) = 0x%08X  "name, reg, value); }
#else
  #define AVS_DBG(frmt,args...)
  #define DEBUG_IN32(reg, result, name)
  #define DEBUG_OUT32(reg, value, name)
#endif

//
// 64 bit operations are only directly supported by the GPE.  On the 405 the
// opreations will get broken up into two 32 bit operations.
//
// FYI: Using out64 was not working for writing OCB registers.
// The problem was that when it breaks it down into two operations,
// it winds up writing the wdata register twice.

// AVS Bus usage will be determined after receiving config data from TMGT
bool G_avsbus_vdd_monitoring = FALSE;
bool G_avsbus_vdn_monitoring = FALSE;

extern uint32_t G_nest_frequency_mhz;
#define AVSBUS_FREQUENCY_MHZ 10
extern bool G_vrm_thermal_monitoring;

extern bool G_vrm_vdd_temp_expired;
void amec_health_check_vrm_vdd_temp(const sensor_t *i_sensor);

// Number of read failures allowed before elog is created and reset requested.
// This should be no longer than 4ms (or it will impact WOF calculations)
// (readings are taken every 500us => 500us * 8 = 4ms)
const uint8_t     MAX_READ_ATTEMPTS = 8;

const uint16_t    AVSBUS_STATUS_READ_ERROR = 0xFFFF;
extern data_cnfg_t * G_data_cnfg;

uint32_t avs_crc_calculate(const uint32_t i_avs_cmd);

// NOTE: OCC must use Bridge B, because Bridge A is reserved for PGPE


// Clear OT/OC bit in status reg for both busses
uint32_t deglitch_status_errors(const uint8_t i_bus, const uint32_t i_status_mask)
{
    // Determine register based on the bus number
    uint32_t o2scmd_reg = OCB_O2SCMD0B;
    uint32_t o2swd_reg = OCB_O2SWD0B;
    uint32_t o2sst_reg = OCB_O2SST0B;
    if (i_bus != 0)
    {
        o2scmd_reg = OCB_O2SCMD1B;
        o2swd_reg = OCB_O2SWD1B;
        o2sst_reg = OCB_O2SST1B;
    }

    // Write O2SCMD[a][n]
    //   o2s_clear_sticky_bits = 1
    uint32_t value = 0x40000000;
    DEBUG_OUT32(o2scmd_reg, value, "OCB_O2SCMDxB");
    out32(o2scmd_reg, value);

    // To deglitch the status bits, set the status bits you wish to clear with a 1. (in CmdData)
    // After the write completes, if the status is still active, then the
    // signal must be solid failure.

    TRAC_INFO("deglitch_status_errors(AVSBUS %d,0x%08X)", i_bus, i_status_mask);
    // AVS Bus command (write staus):
    //   0:1   StartCode = 0b01
    //   2:3   Cmd = 0b00 (write+commit)
    //   4     CmdGroup = 0b0 (AVSBus)
    //   5:8   CmdDataType (STATUS = 01110b)
    //   9:12  Select (All rails / broadcast = 01111b )
    //   13:28 CmdData (status bits to clear)
    //   29:31 CRC
    //   01000DDD DRRRRXXX XXXXXXXX XXXXXCCC
    //   01000111 01111--- -------- -----CCC
    uint32_t cmd_data = AVSBUS_STATUS_OVER_CURRENT_MASK | AVSBUS_STATUS_OVER_TEMPERATURE_MASK;
    value = 0x47780000 | (cmd_data << 3);
    // Calculate/add CRC
    value |= avs_crc_calculate(value);
    DEBUG_OUT32(o2swd_reg, value, "OCB_O2SWDxB");
    out32(o2swd_reg, value);

    // Wait for operation to complete (clear ongoing)
    uint32_t l_status = in32(o2sst_reg);
    DEBUG_IN32(o2sst_reg, l_status, "OCB_O2SSTxB");
    unsigned int loops = 0;
    while ((l_status & AVSBUS_STATUS_ONGOING) && (loops < 500))
    {
        // o2s_ongoing bit was still set (operation did not complete)
        l_status = in32(o2sst_reg);
        ++loops;
    }
    DEBUG_IN32(o2sst_reg, l_status, "OCB_O2SSTxB");
    if (0 != (l_status & AVSBUS_STATUS_ERRORS))
    {
        TRAC_ERR("deglitch_status_errors(): error in status register: 0x%08X", l_status);
    }
    else if (l_status & AVSBUS_STATUS_ONGOING) // o2s_ongoing
    {
        TRAC_ERR("deglitch_status_errors(): timeout waiting for ongoing bit to clear (%d loops) 0x%08X", loops, l_status);
    }

    // return the last read status
    return l_status;

} // end deglitch_status_errors()


// AVS Bus setup that must be done once (common between read/write operations)
void avsbus_init()
{
    uint32_t value;

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
    value = 0x820FC000;
    if (bus0_monitoring)
    {
        DEBUG_OUT32(OCB_O2SCTRLF0B, value, "OCB_O2SCTRLF0B");
        out32(OCB_O2SCTRLF0B, value);
    }
    if (bus1_monitoring)
    {
        DEBUG_OUT32(OCB_O2SCTRLF1B, value, "OCB_O2SCTRLF1B");
        out32(OCB_O2SCTRLF1B, value);
    }

    // Write O2SCTRLS_[a][n]
    //  0:5   o2s_out_count2 = 0 (no output)
    //  6:11  o2s_in_delay2 = 0 (no delay - immediate read data)
    //  12:17 o2s_in_count2 = 32 (bits captured)
    value = 0x00080000;
    if (bus0_monitoring)
    {
        DEBUG_OUT32(OCB_O2SCTRLS0B, value, "OCB_O2SCTRLS0B");
        out32(OCB_O2SCTRLS0B, value);
    }
    if (bus1_monitoring)
    {
        DEBUG_OUT32(OCB_O2SCTRLS1B, value, "OCB_O2SCTRLS1B");
        out32(OCB_O2SCTRLS1B, value);
    }

    // Write O2SCTRL1_[a][n]
    //  0     o2s_bridge_enable = 1 (make the O2S bridge active if not already)
    //  1     reserved
    //  2     o2s_cpol = 0 (positive active clock)
    //  3     o2s_cpha = 1 (second edge data sample)
    //  4:13  o2s_clock_divider = set based on the nest frequency for the desired frequency of 10MHz (assumed speed) per O2SCTRL1_[a][n] description.
    //  14:16 reserved
    //  17    o2s_nr_of_frames = 1 (2 frames to account for the first and second frames of an AVSBus command)
    //  18:63 reserved
    //  1r00DDDD DDDDDDrr r1rrrrrr rrrrrrrrr
    value = 0x90004000;
    // calculate o2s_clock_divider based on nest freq and target bus freq
    const uint32_t divider = (G_nest_frequency_mhz / (AVSBUS_FREQUENCY_MHZ * 8)) - 1;
    value |= (divider << 18);
    if (bus0_monitoring)
    {
        TRAC_INFO("avsbus_init: reg[OCB_O2SCTRL10B] = 0x%08X", value);
        out32(OCB_O2SCTRL10B, value);
    }
    if (bus1_monitoring)
    {
        TRAC_INFO("avsbus_init: reg[OCB_O2SCTRL11B] = 0x%08X", value);
        out32(OCB_O2SCTRL11B, value);
    }

    // Write O2SCTRL2_[a][n]
    //  o2s_inter_frame_delay = 0 (Wait 1 SPI clock). The AVSBus spec does not define any inter-frame delay so set this to the smallest value.
    //  Note: the value 0 is the hardware reset value and, thus, this step can be omitted if the value desired to to be left at 0.
    value = 0x00000000;
    if (bus0_monitoring)
    {
        DEBUG_OUT32(OCB_O2SCTRL20B, value, "OCB_O2SCTRL20B");
        out32(OCB_O2SCTRL20B, value);
    }
    if (bus1_monitoring)
    {
        DEBUG_OUT32(OCB_O2SCTRL21B, value, "OCB_O2SCTRL21B");
        out32(OCB_O2SCTRL21B, value);
    }

    // Clear OC/OT masks in the status reg
    const uint32_t error_mask = AVSBUS_STATUS_OVER_CURRENT_MASK | AVSBUS_STATUS_OVER_TEMPERATURE_MASK;
    deglitch_status_errors(0, error_mask);
    deglitch_status_errors(1, error_mask);

} // end avsbus_init()


// Calculate CRC for specified AVS Bus command
// Function which generates a 3 bit CRC value for 29 bit data
// from: ekb/chips/p9/procedures/hwp/lib/p9_avsbus_lib.C
// (CRC is bits 29:31 in the AVS bus command)
#define AVS_CRC_MASK      0x00000007
#define AVS_CRC_DATA_MASK 0xFFFFFFF8
uint32_t avs_crc_calculate(const uint32_t i_avs_cmd)
{
    //Polynomial= x^3 + x^1 + x^0 = 1*x^3 + 0*x^2 + 1*x^1 + 1*x^0 = divisor(1011)

    uint32_t o_crc_value = 0;
    uint32_t l_polynomial = 0xB0000000;
    uint32_t l_msb =        0x80000000;

    o_crc_value = i_avs_cmd & AVS_CRC_DATA_MASK;

    while (o_crc_value & AVS_CRC_DATA_MASK)
    {
        if (o_crc_value & l_msb)
        {
            //if l_msb is 1'b1, divide by l_polynomial and shift l_polynomial
            // to the right
            o_crc_value = o_crc_value ^ l_polynomial;
            l_polynomial = l_polynomial >> 1;
        }
        else
        {
            // if l_msb is zero, shift l_polynomial
            l_polynomial = l_polynomial >> 1;
        }

        l_msb = l_msb >> 1;
    }

    return o_crc_value;
}


// Initiate read for specified type (Vdd/Vdn) and cmd (Voltage/Current)
void avsbus_read_start(const avsbus_type_e i_type,
                       const avsbus_cmdtype_e i_cmdtype)
{
    if (isSafeStateRequested())
    {
        // No need to attempt read if OCC will be reset
        return;
    }

    avsbusData_t l_data;

    // Create error array for each type (Vdd/Vdn) and command (Voltage/Current)
    if (AVSBUS_VDD == i_type)
    {
        l_data = G_sysConfigData.avsbus_vdd;
    }
    else
    {
        l_data = G_sysConfigData.avsbus_vdn;
    }

#ifdef AVSDEBUG
    uint8_t l_cmd_index = 0;
    char l_trace_cmd = 'V';
    char l_trace_type = 'd';

    if (i_cmdtype == AVSBUS_CURRENT)
    {
        l_cmd_index = 1;
        l_trace_cmd = 'C';
    }
    else if (i_cmdtype == AVSBUS_TEMPERATURE)
    {
        l_cmd_index = 2;
        l_trace_cmd = 'T';
    }
    if (AVSBUS_VDD != i_type)
    {
        l_trace_type = 'n';
    }

    static uint32_t L_trace_count[AVSBUS_TYPE_MAX][AVSBUS_CMDS_MAX] = {{0}};
    uint32_t *      l_trace_count = &L_trace_count[i_type][l_cmd_index];
    if (*l_trace_count < DEBUG_TRACE_MAX)
    {
        TRAC_INFO("avsbus_read_start: Vd%c %c - bus[%d] rail[%d]",
                  l_trace_type, l_trace_cmd, l_data.bus, l_data.rail);
    }
#endif

    // Determine register based on the bus number
    uint32_t o2scmd_reg = OCB_O2SCMD0B;
    uint32_t o2swd_reg = OCB_O2SWD0B;
    if (1 == l_data.bus)
    {
        o2scmd_reg = OCB_O2SCMD1B;
        o2swd_reg = OCB_O2SWD1B;
    }

    // Write O2SCMD[a][n]
    //   o2s_clear_sticky_bits = 1
    uint32_t value = 0x40000000;
    DEBUG_OUT32(o2scmd_reg, value, "OCB_O2SCMDxB");
    out32(o2scmd_reg, value);

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
    value = 0x7007FFF8 | ((uint32_t) i_cmdtype << 23) | ((uint32_t)l_data.rail << 19);
    // Calculate/add CRC
    value |= avs_crc_calculate(value);
    DEBUG_OUT32(o2swd_reg, value, "OCB_O2SWDxB");
    out32(o2swd_reg, value);

    // Read has been started so now just wait for HW to complete

    // HW: Wait for bus op to complete
    // HW: arbitration between two bridges
    // HW: o2s_ongoing: 0 -> 1
    // HW: execution completes
    // HW: o2s_ongoing 1 -> 0

#ifdef AVSDEBUG
    ++*l_trace_count;
#endif

} // end avsbus_read_start()


// Read and return the voltage, current, or temperature for specified rail
// (voltage units are mV, current units are in 10mA, temperature in 0.1 C)
uint16_t avsbus_read(const avsbus_type_e i_type,
                     const avsbus_cmdtype_e i_cmdtype)
{
    if (isSafeStateRequested())
    {
        // No need to process data if OCC will be reset
        return 0;
    }

    uint16_t o_reading = 0;
    bool l_failure = FALSE;

    uint8_t l_cmd_index = 0;
    char l_trace_cmd = 'V';
    if (i_cmdtype == AVSBUS_CURRENT)
    {
        l_cmd_index = 1;
        l_trace_cmd = 'C';
    }
    else if (i_cmdtype == AVSBUS_TEMPERATURE)
    {
        l_cmd_index = 2;
        l_trace_cmd = 'T';
    }

    // Static error counters for each type (Vdd/Vdn) and command (Voltage/Current)
    static uint32_t L_error_count[AVSBUS_TYPE_MAX][AVSBUS_CMDS_MAX] = {{0}};
    uint32_t *      l_error_count = &L_error_count[i_type][l_cmd_index];

    char l_trace_type = 'd';
    avsbusData_t l_data = G_sysConfigData.avsbus_vdd;
    if (AVSBUS_VDN == i_type)
    {
        l_trace_type = 'n';
        l_data = G_sysConfigData.avsbus_vdn;
    }

#ifdef AVSDEBUG
    static uint32_t L_trace_count[AVSBUS_TYPE_MAX][AVSBUS_CMDS_MAX] = {{0}};
    uint32_t *      l_trace_count = &L_trace_count[i_type][l_cmd_index];
    if (*l_trace_count < DEBUG_TRACE_MAX)
    {
        TRAC_INFO("avsbus_read: Vd%c %c - bus[%d] rail[%d]",
                  l_trace_type, l_trace_cmd, l_data.bus, l_data.rail);
    }
#endif

    // Determine register based on the bus number
    uint32_t o2sst_reg = OCB_O2SST0B;
    uint32_t o2srd_reg = OCB_O2SRD0B;
    if (1 == l_data.bus)
    {
        o2sst_reg = OCB_O2SST1B;
        o2srd_reg = OCB_O2SRD1B;
    }

    // HW: Wait for bus op to complete
    // HW: arbitration between two bridges
    // HW: o2s_ongoing: 0 -> 1
    // HW: execution completes
    // HW: o2s_ongoing 1 -> 0

    // Since read was started in previous tick, it should have already completed
    // (no need to poll/wait on o2s_ongoing)
    enum occReasonCode rc = OCC_SUCCESS_REASON_CODE;
    uint32_t l_status = in32(o2sst_reg);
    DEBUG_IN32(o2sst_reg, l_status, "OCB_O2SSTxB");
    // OCC O2S Status Register
    //   0     o2s_ongoing
    //   1:4   reserved
    //   5     write_while_bridge_busy_error
    //   6     reserved
    //   7     FSM error
    //   8:63  reserved
    // GrrrrBrF rrrrrrrr rrrrrrrr rrrrrrrr

    if (0 != (l_status & AVSBUS_STATUS_ERRORS))
    {
        // error bit was set
        l_failure = TRUE;
        (*l_error_count)++;
        if ((*l_error_count == 1) || (*l_error_count == MAX_READ_ATTEMPTS))
        {
            TRAC_ERR("avsbus_read: Error found in Vd%c %c O2SST[0x%08X] = [0x%08X]",
                     l_trace_type, l_trace_cmd, o2sst_reg, l_status);
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
    else if (l_status & AVSBUS_STATUS_ONGOING) // o2s_ongoing
    {
        // o2s_ongoing bit was still set (operation did not complete)
        l_failure = TRUE;
        (*l_error_count)++;
        if ((*l_error_count == 1) || (*l_error_count == MAX_READ_ATTEMPTS))
        {
            TRAC_ERR("avsbus_read: Vd%c %c timeout waiting for o2s_ongoing change O2SST[0x%08X] = [0x%08X]",
                     l_trace_type, l_trace_cmd, o2sst_reg, l_status);
            /*
             * @errortype
             * @moduleid    PSS_MID_AVSBUS_READ
             * @reasoncode  AVSBUS_TIMEOUT
             * @userdata1   AVS Bus type/bus/rail
             * @userdata2   status
             * @devdesc     Timeout when reading AVS Bus
             */
            rc = AVSBUS_TIMEOUT;
        }
    }

    if (FALSE == l_failure)
    {
        // Read the response data
        uint32_t value = in32(o2srd_reg);
        DEBUG_IN32(o2srd_reg, value, "OCB_O2SRDxB");
        // AVS Bus response (read voltage, current, or temperature):
        //   0:1   SlaveAck (0b00 from slave indicates good CRC and action was taken)
        //   2     0
        //   3:7   StatusResp
        //   8:23  CmdData (LSB = 1mV or 10mA or 0.1C)
        //   24:28 Reserved (must be all 1s)
        //   29:31 CRC
        //   AA0SSSSS VVVVVVVV VVVVVVVV 11111CCC

        // Validate CRC
        const uint32_t crc = avs_crc_calculate(value);
        if (crc != (value & AVS_CRC_MASK))
        {
            l_failure = TRUE;
            (*l_error_count)++;
            if ((*l_error_count == 1) || (*l_error_count == MAX_READ_ATTEMPTS))
            {
                TRAC_ERR("avsbus_read: CRC mismatch in Vd%c %c rsp O2SRD[0x%08X] = [0x%08X] (calculated CRC 0x%08X)",
                         l_trace_type, l_trace_cmd, o2srd_reg, value, crc);
                /*
                 * @errortype
                 * @moduleid    PSS_MID_AVSBUS_READ
                 * @reasoncode  AVSBUS_CRC_ERROR
                 * @userdata1   AVS Bus type/bus/rail
                 * @userdata2   status
                 * @devdesc     CRC error reading AVS Bus
                 */
                rc = AVSBUS_CRC_ERROR;
            }
        }
        // Check for valid command operation and extract read data
        else if (0 == (value & 0xC0000000))
        {
            o_reading = (value >> 8) & 0x0000FFFF;
#ifdef AVSDEBUG
            if (*l_trace_count < DEBUG_TRACE_MAX)
            {
                if (i_cmdtype == AVSBUS_VOLTAGE)
                {
                    TRAC_INFO("avsbus_read: Successfully read Vd%c voltage %dmV [0x%08X]",
                              l_trace_type, o_reading, value);
                }
                else if (i_cmdtype == AVSBUS_CURRENT)
                {
                    TRAC_INFO("avsbus_read: Successfully read Vd%c current %dx10mA [0x%08X]",
                              l_trace_type, o_reading, value);
                }
            }
#endif

            if (i_cmdtype == AVSBUS_TEMPERATURE)
            {
#ifdef AVSDEBUG
                if (*l_trace_count < DEBUG_TRACE_MAX)
                {
                    TRAC_INFO("avsbus_read: Successfully read Vd%c temperature %d/10 C [0x%08X]",
                              l_trace_type, o_reading, value);
                }
#endif
                // Update sensor (convert to degrees C) and validate it
                sensor_t * l_sensor = AMECSENSOR_PTR(TEMPVDD);
                sensor_update(l_sensor, (uint16_t)o_reading/10);
                G_vrm_vdd_temp_expired = false;
                amec_health_check_vrm_vdd_temp(l_sensor);
            }

            if (*l_error_count)
            {
                // Trace and clear the error count
                TRAC_INFO("avsbus_read: Successfully read Vd%c %c [0x%08X] (error count=%d)",
                          l_trace_type, l_trace_cmd, value, *l_error_count);
                *l_error_count = 0;
            }
        }
        else
        {
            l_failure = TRUE;
            (*l_error_count)++;
            if ((*l_error_count == 1) || (*l_error_count == MAX_READ_ATTEMPTS))
            {
                TRAC_ERR("avsbus_read: SlaveAck reported no action taken[0x%08X]", value);
                rc = AVSBUS_ERROR;
            }
        }
    }

    if (l_failure)
    {
        enum occExtReasonCode exrc = ERC_AVSBUS_VDD_VOLTAGE_FAILURE;
        if (AVSBUS_VDD == i_type)
        {
            if (i_cmdtype == AVSBUS_CURRENT)
            {
                exrc = ERC_AVSBUS_VDD_CURRENT_FAILURE;
                INCREMENT_ERR_HISTORY(ERRH_AVSBUS_VDD_CURRENT);
            }
            else if (i_cmdtype == AVSBUS_VOLTAGE)
            {
                INCREMENT_ERR_HISTORY(ERRH_AVSBUS_VDD_VOLTAGE);
            }
            else if (i_cmdtype == AVSBUS_TEMPERATURE)
            {
                exrc = ERC_AVSBUS_VDD_TEMPERATURE_FAILURE;
                INCREMENT_ERR_HISTORY(ERRH_AVSBUS_VDD_TEMPERATURE);
            }
        }
        else
        {
            if (i_cmdtype == AVSBUS_CURRENT)
            {
                exrc = ERC_AVSBUS_VDN_CURRENT_FAILURE;
                INCREMENT_ERR_HISTORY(ERRH_AVSBUS_VDN_CURRENT);
            }
            else if (i_cmdtype == AVSBUS_VOLTAGE)
            {
                exrc = ERC_AVSBUS_VDN_VOLTAGE_FAILURE;
                INCREMENT_ERR_HISTORY(ERRH_AVSBUS_VDN_VOLTAGE);
            }
        }

        if (*l_error_count == MAX_READ_ATTEMPTS)
        {
            TRAC_ERR("avsbus_read: Reached %d consecutive Vd%c %c errors, requesting reset",
                     *l_error_count, l_trace_type, l_trace_cmd);
            G_avsbus_vdd_monitoring = FALSE;
            G_avsbus_vdn_monitoring = FALSE;
            G_vrm_thermal_monitoring = FALSE;
            errlHndl_t l_err = createErrl(PSS_MID_AVSBUS_READ,
                                          rc,
                                          exrc,
                                          ERRL_SEV_PREDICTIVE,
                                          NULL,
                                          DEFAULT_TRACE_SIZE,
                                          (i_type << 16) | (l_data.bus << 8) | l_data.rail,
                                          l_status);
            // add processor callout and request reset
            addCalloutToErrl(l_err,
                             ERRL_CALLOUT_TYPE_HUID,
                             G_sysConfigData.proc_huid,
                             ERRL_CALLOUT_PRIORITY_MED);
            REQUEST_RESET(l_err);
        }
    }

#ifdef AVSDEBUG
    ++*l_trace_count;
#endif

    return o_reading;

} // end avsbus_read()


// Start the AVS Bus read for both buses (if enabled)
void initiate_avsbus_reads(avsbus_cmdtype_e i_cmdType)
{
    if (G_avsbus_vdd_monitoring)
    {
        // AVS Bus Vdd config data was received:
        // Initiate AVS Bus read for Vdd (voltage or current)
        avsbus_read_start(AVSBUS_VDD, i_cmdType);
    }

    if (G_avsbus_vdn_monitoring)
    {
        // AVS Bus Vdn config data was received:
        // Initiate AVS Bus read for Vdn (voltage or current)
        avsbus_read_start(AVSBUS_VDN, i_cmdType);
    }

} // end initiate_avsbus_reads()


// Initiate read for error status bits (over-temperature, over-current)
void initiate_avsbus_read_status()
{
    if (isSafeStateRequested())
    {
        // No need to attempt read if OCC will be reset
        return;
    }

#ifdef AVSDEBUG
    static uint32_t L_trace_count = 0;
#endif

    unsigned int index;
    for (index = 0; index < AVSBUS_TYPE_MAX; ++index)
    {
        // Determine busses that are being monitored
        uint8_t bus = 0xFF;
        if ((index == 0) && G_avsbus_vdd_monitoring)
        {
            bus = G_sysConfigData.avsbus_vdd.bus;
        }
        else if ((index == 1) && G_avsbus_vdn_monitoring)
        {
            bus = G_sysConfigData.avsbus_vdn.bus;
        }
        if (bus != 0xFF)
        {
            // Determine register based on the bus number
            uint32_t o2scmd_reg = OCB_O2SCMD0B;
            uint32_t o2swd_reg = OCB_O2SWD0B;
            if (0 == bus)
            {
                o2scmd_reg = OCB_O2SCMD0B;
                o2swd_reg = OCB_O2SWD0B;
            }
            else
            {
                o2scmd_reg = OCB_O2SCMD1B;
                o2swd_reg = OCB_O2SWD1B;
            }

#ifdef AVSDEBUG
            if (L_trace_count < DEBUG_TRACE_MAX)
            {
                TRAC_INFO("initiate_avsbus_read_status: read Status - bus[%d], rail[broadcast]", bus);
            }
#endif

            // Write O2SCMD[a][n]
            //   o2s_clear_sticky_bits = 1
            uint32_t value = 0x40000000;
            DEBUG_OUT32(o2scmd_reg, value, "OCB_O2SCMDxB");
            out32(o2scmd_reg, value);

            // Write O2SWD[a][n] - write commands and initiate hardware operation
            //   o2s_wdata with content
            // AVS Bus command (read staus):
            //   0:1   StartCode = 0b01
            //   2:3   Cmd = 0b11 (read)
            //   4     CmdGroup = 0b0 (AVSBus)
            //   5:8   CmdDataType (STATUS = 01110b)
            //   9:12  Select (All rails / broadcast = 01111b )
            //   13:28 CmdData (reserved / must be 1s)
            //   29:31 CRC
            //   01110DDD DRRRR111 11111111 11111CCC
            //   01110111 01111111 11111111 11111CCC
            value = 0x777FFFF8;
            // Calculate/add CRC
            value |= avs_crc_calculate(value);
            DEBUG_OUT32(o2swd_reg, value, "OCB_O2SWDxB");
            out32(o2swd_reg, value);
        }
    }

    // Read has been started so now just wait for HW to complete

    // HW: Wait for bus op to complete
    // HW: arbitration between two bridges
    // HW: o2s_ongoing: 0 -> 1
    // HW: execution completes
    // HW: o2s_ongoing 1 -> 0

#ifdef AVSDEBUG
    ++L_trace_count;
#endif

} // end initiate_avsbus_read_status()


// Process AVS Bus read status results (or errors)
// Predictive error will be logged after "VRMs: max_read_timeout" failures
// and a mfg error will be committed.
// Returns the status data or AVSBUS_STATUS_READ_ERROR on error
uint16_t avsbus_read_status(const avsbus_type_e i_type)
{
    if (isSafeStateRequested() || (G_vrm_thermal_monitoring == FALSE))
    {
        // No need to process data if OCC will be reset
        return 0;
    }

    uint16_t o_reading = 0;
    bool l_failure = FALSE;
    const uint8_t max_read_attempts = G_data_cnfg->thrm_thresh.data[DATA_FRU_VRM_OT_STATUS].max_read_timeout;

    // Static error counters for each type (Vdd/Vdn)
    static uint32_t L_error_count[AVSBUS_TYPE_MAX] = {0};
    uint32_t *      l_error_count = &L_error_count[i_type];

    char l_trace_type = 'd';
    avsbusData_t l_data = G_sysConfigData.avsbus_vdd;
    if (AVSBUS_VDN == i_type)
    {
        l_trace_type = 'n';
        l_data = G_sysConfigData.avsbus_vdn;
    }

#ifdef AVSDEBUG
    static uint32_t L_trace_count = 0;
    if (L_trace_count < DEBUG_TRACE_MAX)
    {
        TRAC_INFO("avsbus_read_status: Vd%c - bus[%d] rail[%d]",
                  l_trace_type, l_data.bus, l_data.rail);
    }
#endif

    // Determine register based on the bus number
    uint32_t o2sst_reg = OCB_O2SST0B;
    uint32_t o2srd_reg = OCB_O2SRD0B;
    if (1 == l_data.bus)
    {
        o2sst_reg = OCB_O2SST1B;
        o2srd_reg = OCB_O2SRD1B;
    }

    // HW: Wait for bus op to complete
    // HW: arbitration between two bridges
    // HW: o2s_ongoing: 0 -> 1
    // HW: execution completes
    // HW: o2s_ongoing 1 -> 0

    // Since read was started in previous tick, it should have already completed
    // (no need to poll/wait on o2s_ongoing)
    enum occReasonCode rc = OCC_SUCCESS_REASON_CODE;
    uint32_t l_status = in32(o2sst_reg);
    DEBUG_IN32(o2sst_reg, l_status, "OCB_O2SSTxB");
    // OCC O2S Status Register
    //   0     o2s_ongoing
    //   1:4   reserved
    //   5     write_while_bridge_busy_error
    //   6     reserved
    //   7     FSM error
    //   8:63  reserved
    // GrrrrBrF rrrrrrrr rrrrrrrr rrrrrrrr

    if (0 != (l_status & AVSBUS_STATUS_ERRORS))
    {
        // error bit was set
        l_failure = TRUE;
        (*l_error_count)++;
        if ((*l_error_count == 1) || (*l_error_count == max_read_attempts))
        {
            TRAC_ERR("avsbus_read_status: Error found in Vd%c O2SST[0x%08X] = [0x%08X]",
                     l_trace_type, o2sst_reg, l_status);
            /*
             * @errortype
             * @moduleid    PSS_MID_AVSBUS_READ_STATUS
             * @reasoncode  AVSBUS_ERROR
             * @userdata1   AVS Bus type/bus/rail
             * @userdata2   status
             * @devdesc     Error encountered when reading AVS Bus
             */
            rc = AVSBUS_ERROR;
        }
    }
    else if (l_status & AVSBUS_STATUS_ONGOING) // o2s_ongoing
    {
        // o2s_ongoing bit was still set (operation did not complete)
        l_failure = TRUE;
        (*l_error_count)++;
        if ((*l_error_count == 1) || (*l_error_count == max_read_attempts))
        {
            TRAC_ERR("avsbus_read_status: Vd%c timeout waiting for o2s_ongoing change O2SST[0x%08X] = [0x%08X]",
                     l_trace_type, o2sst_reg, l_status);
            /*
             * @errortype
             * @moduleid    PSS_MID_AVSBUS_READ_STATUS
             * @reasoncode  AVSBUS_TIMEOUT
             * @userdata1   AVS Bus type/bus/rail
             * @userdata2   status
             * @devdesc     Timeout when reading AVS Bus
             */
            rc = AVSBUS_TIMEOUT;
        }
    }

    if (FALSE == l_failure)
    {
        // Read the response data
        uint32_t value = in32(o2srd_reg);
        DEBUG_IN32(o2srd_reg, value, "OCB_O2SRDxB");
        // AVS Bus response (read status):
        //   0:1   SlaveAck (0b00 from slave indicates good CRC and action was taken)
        //   2     0
        //   3:7   StatusResp
        //   8:23  CmdData (LSB = 1mV or 10mA)
        //   24:28 Reserved (must be all 1s)
        //   29:31 CRC
        //   AA0SSSSS VVVVVVVV VVVVVVVV 11111CCC

        // Validate CRC
        const uint32_t crc = avs_crc_calculate(value);
        if (crc != (value & AVS_CRC_MASK))
        {
            l_failure = TRUE;
            (*l_error_count)++;
            if ((*l_error_count == 1) || (*l_error_count == max_read_attempts))
            {
                TRAC_ERR("avsbus_read_status: CRC mismatch in Vd%c rsp O2SRD[0x%08X] = [0x%08X] (calculated CRC 0x%08X)",
                         l_trace_type, o2srd_reg, value, crc);
                /*
                 * @errortype
                 * @moduleid    PSS_MID_AVSBUS_READ_STATUS
                 * @reasoncode  AVSBUS_CRC_ERROR
                 * @userdata1   AVS Bus type/bus/rail
                 * @userdata2   status
                 * @devdesc     CRC error reading AVS Bus
                 */
                rc = AVSBUS_CRC_ERROR;
            }
        }
        // Check for valid command operation and extract read data
        else if (0 == (value & 0xC0000000))
        {
            // AVS Bus Status:
            //  0     VDone
            //  1     IOUT_OC_WARNING (over-current)
            //  2     VOUT_UV_WARNING (under-voltage)
            //  3     IOUT_OT_WARNING (over-temperature)
            //  4     POUT_OP_WARNING (over power)
            //  5-7   reserved
            //  8-15  reserved
            o_reading = (value >> 8) & 0x0000FFFF;


#ifdef AVSDEBUG
            static uint16_t L_lastReading = 0;
            if ((L_trace_count < DEBUG_TRACE_MAX) || (o_reading != L_lastReading))
            {
                TRAC_INFO("avsbus_read_status: Successfully read Vd%c status 0x%04X [0x%08X]",
                          l_trace_type, o_reading, value);
                L_lastReading = o_reading;
            }
#endif
            if (*l_error_count)
            {
                // Trace and clear the error count
                TRAC_INFO("avsbus_read_status: Successfully read Vd%c status [0x%08X] (error count=%d)",
                          l_trace_type, value, *l_error_count);
                *l_error_count = 0;
            }
        }
        else
        {
            l_failure = TRUE;
            (*l_error_count)++;
            if ((*l_error_count == 1) || (*l_error_count == max_read_attempts))
            {
                TRAC_ERR("avsbus_read_status: SlaveAck reported no action taken[0x%08X]", value);
                rc = AVSBUS_ERROR;
            }
        }
    }

    if (l_failure)
    {
        if (*l_error_count == max_read_attempts)
        {
            TRAC_ERR("avsbus_read_status: Reached %d consecutive Vd%c errors reading status",
                     *l_error_count, l_trace_type);
            G_avsbus_vdd_monitoring = FALSE;
            G_avsbus_vdn_monitoring = FALSE;
            G_vrm_thermal_monitoring = FALSE;
            errlHndl_t l_err = createErrl(PSS_MID_AVSBUS_READ_STATUS,
                                          rc,
                                          ERC_AVSBUS_STATUS_FAILURE,
                                          ERRL_SEV_PREDICTIVE,
                                          NULL,
                                          DEFAULT_TRACE_SIZE,
                                          (i_type << 16) | (l_data.bus << 8) | l_data.rail,
                                          l_status);
            setErrlActions(l_err, ERRL_ACTIONS_MANUFACTURING_ERROR);
            // add processor callout
            addCalloutToErrl(l_err,
                             ERRL_CALLOUT_TYPE_HUID,
                             G_sysConfigData.proc_huid,
                             ERRL_CALLOUT_PRIORITY_MED);
            commitErrl(&l_err);
        }
        o_reading = AVSBUS_STATUS_READ_ERROR;
    }

#ifdef AVSDEBUG
    ++L_trace_count;
    if (L_trace_count >= DEBUG_TRACE_MAX)
    {
        G_trace_scoms = FALSE;
    }
#endif

    return o_reading;

} // end avsbus_read_status()


// Read the status from AVS Bus and return 1 if over-temperature was found for either bus
// or 0 if no OT was found.  0xFF will be returned if there was an error reading status.
// Error history counters will be incremented for any over-temp/over-current condition.
// Mfg error will be logged for the first OT or first OC condition.
uint8_t process_avsbus_status()
{
    uint8_t  foundOT = 0;
    uint8_t  foundOC = 0;
    uint16_t vdd_status = 0;
    uint16_t vdn_status = 0;
    static bool L_vdd_ot_found = FALSE;
    static bool L_vdd_oc_found = FALSE;
    static bool L_vdn_ot_found = FALSE;
    static bool L_vdn_oc_found = FALSE;
    static bool L_vdd_ot_counted = FALSE;
    static bool L_vdd_oc_counted = FALSE;
    static bool L_vdn_ot_counted = FALSE;
    static bool L_vdn_oc_counted = FALSE;

    if (G_vrm_thermal_monitoring)
    {
        if (G_avsbus_vdd_monitoring)
        {
            vdd_status = avsbus_read_status(AVSBUS_VDD);
            if (vdd_status != AVSBUS_STATUS_READ_ERROR)
            {
                if ((vdd_status & (AVSBUS_STATUS_OVER_TEMPERATURE_MASK|AVSBUS_STATUS_OVER_CURRENT_MASK)) == 0)
                {
                    // No OT/OC errors found
                    if (L_vdd_ot_found) TRAC_INFO("process_avsbus_status: Vdd OT cleared");
                    L_vdd_ot_found = FALSE;
                    L_vdd_ot_counted = FALSE;
                    if (L_vdd_oc_found) TRAC_INFO("process_avsbus_status: Vdd OC cleared");
                    L_vdd_oc_found = FALSE;
                    L_vdd_oc_counted = FALSE;
                }
                else
                {
                    // Deglitch error if needed
                    uint32_t deglitch_mask = 0;
                    if ((vdd_status & AVSBUS_STATUS_OVER_TEMPERATURE_MASK) && (!L_vdd_ot_found))
                    {
                        deglitch_mask |= AVSBUS_STATUS_OVER_TEMPERATURE_MASK;
                        L_vdd_ot_found = TRUE;
                    }
                    if ((vdd_status & AVSBUS_STATUS_OVER_CURRENT_MASK) && (!L_vdd_oc_found))
                    {
                        deglitch_mask |= AVSBUS_STATUS_OVER_CURRENT_MASK;
                        L_vdd_oc_found = TRUE;
                    }
                    if (deglitch_mask)
                    {
                        // Deglitch error bit(s)
                        deglitch_status_errors(G_sysConfigData.avsbus_vdd.bus, deglitch_mask);

                        // Clear error bit(s) since could be false reading
                        vdd_status &= (~deglitch_mask);
                    }

                    // Process error (if already deglitched)
                    if (vdd_status & AVSBUS_STATUS_OVER_TEMPERATURE_MASK)
                    {
                        foundOT = 1;
                        if (!L_vdd_ot_counted)
                        {
                            INCREMENT_ERR_HISTORY(ERRH_AVSBUS_VDD_OVER_TEMPERATURE);
                            L_vdd_ot_counted = TRUE;
                        }
                    }
                    else
                    {
                        L_vdd_ot_counted = FALSE;
                    }

                    if (vdd_status & AVSBUS_STATUS_OVER_CURRENT_MASK)
                    {
                        foundOC = 1;
                        if (!L_vdd_oc_counted)
                        {
                            INCREMENT_ERR_HISTORY(ERRH_AVSBUS_VDD_OVER_CURRENT);
                            L_vdd_oc_counted = TRUE;
                        }
                    }
                    else
                    {
                        L_vdd_oc_counted = FALSE;
                    }
                }
            }
            else
            {
                // 0xFF indicates error reading status
                foundOT = 0xFF;
            }
        }
        if (G_avsbus_vdn_monitoring)
        {
            vdn_status = avsbus_read_status(AVSBUS_VDN);
            if (vdn_status != AVSBUS_STATUS_READ_ERROR)
            {
                if ((vdn_status & (AVSBUS_STATUS_OVER_TEMPERATURE_MASK|AVSBUS_STATUS_OVER_CURRENT_MASK)) == 0)
                {
                    // No OT/OC errors found
                    if (L_vdn_ot_found) TRAC_INFO("process_avsbus_status: Vdn OT cleared");
                    L_vdn_ot_found = FALSE;
                    L_vdn_ot_counted = FALSE;
                    if (L_vdn_oc_found) TRAC_INFO("process_avsbus_status: Vdn OC cleared");
                    L_vdn_oc_found = FALSE;
                    L_vdn_oc_counted = FALSE;
                }
                else
                {
                    // Deglitch error if needed
                    uint32_t deglitch_mask = 0;
                    if ((vdn_status & AVSBUS_STATUS_OVER_TEMPERATURE_MASK) && (!L_vdn_ot_found))
                    {
                        deglitch_mask |= AVSBUS_STATUS_OVER_TEMPERATURE_MASK;
                        L_vdn_ot_found = TRUE;
                    }
                    if ((vdn_status & AVSBUS_STATUS_OVER_CURRENT_MASK) && (!L_vdn_oc_found))
                    {
                        deglitch_mask |= AVSBUS_STATUS_OVER_CURRENT_MASK;
                        L_vdn_oc_found = TRUE;
                    }
                    if (deglitch_mask)
                    {
                        // Deglitch error bit(s)
                        deglitch_status_errors(G_sysConfigData.avsbus_vdn.bus, deglitch_mask);

                        // Clear error bit(s) since could be false reading
                        vdn_status &= (~deglitch_mask);
                    }

                    // Process error (if already deglitched)
                    if (vdn_status & AVSBUS_STATUS_OVER_TEMPERATURE_MASK)
                    {
                        // if no Vdd error, update OT result
                        if (foundOT != 0xFF)
                        {
                            foundOT = 1;
                        }
                        if (!L_vdn_ot_counted)
                        {
                            INCREMENT_ERR_HISTORY(ERRH_AVSBUS_VDN_OVER_TEMPERATURE);
                            L_vdn_ot_counted = TRUE;
                        }
                    }
                    else
                    {
                        L_vdn_ot_counted = FALSE;
                    }

                    if (vdn_status & AVSBUS_STATUS_OVER_CURRENT_MASK)
                    {
                        foundOC = 1;
                        if (!L_vdn_oc_counted)
                        {
                            INCREMENT_ERR_HISTORY(ERRH_AVSBUS_VDN_OVER_CURRENT);
                            L_vdn_oc_counted = TRUE;
                        }
                    }
                    else
                    {
                        L_vdn_oc_counted = FALSE;
                    }
                }
            }
            else
            {
                // 0xFF indicates error reading status
                foundOT = 0xFF;
            }
        }

        // Log an error the first time either condition is asserted
        static bool loggedOT = FALSE;
        static bool loggedOC = FALSE;
        errlHndl_t l_err;
        if ((foundOT == 1) && !loggedOT && G_vrm_thermal_monitoring)
        {
            loggedOT = TRUE;
            TRAC_ERR("process_avsbus_status: AVSBUS Over Temperature Warning (Vdd: 0x%08X, Vdn: 0x%08X)",
                     vdd_status, vdn_status);
            /* @
             * @errortype
             * @moduleid    PSS_MID_AVSBUS_READ
             * @reasoncode  VRM_VRFAN_WARNING
             * @userdata1   Vdd Status
             * @userdata2   Vdn Status
             * @devdesc     VRFAN / over-temperature asserted
             */
            l_err = createErrl(PSS_MID_AVSBUS_READ,
                               VRM_VRFAN_WARNING,
                               OCC_NO_EXTENDED_RC,
                               ERRL_SEV_INFORMATIONAL,
                               NULL,
                               DEFAULT_TRACE_SIZE,
                               vdd_status, vdn_status);
            setErrlActions(l_err, ERRL_ACTIONS_MANUFACTURING_ERROR);
            // add processor callout
            addCalloutToErrl(l_err,
                             ERRL_CALLOUT_TYPE_HUID,
                             G_sysConfigData.proc_huid,
                             ERRL_CALLOUT_PRIORITY_MED);
            commitErrl(&l_err);
        }
        if ((foundOC == 1) && !loggedOC)
        {
            loggedOC = TRUE;
            TRAC_ERR("process_avsbus_status: AVSBUS Over Current Warning (Vdd: 0x%08X, Vdn: 0x%08X)",
                     vdd_status, vdn_status);
            /* @
             * @errortype
             * @moduleid    PSS_MID_AVSBUS_READ
             * @reasoncode  VRM_OVER_CURRENT_WARNING
             * @userdata1   Vdd Status
             * @userdata2   Vdn Status
             * @devdesc     Output over-current asserted
             */
            l_err = createErrl(PSS_MID_AVSBUS_READ,
                               VRM_OVER_CURRENT_WARNING,
                               OCC_NO_EXTENDED_RC,
                               ERRL_SEV_INFORMATIONAL,
                               NULL,
                               DEFAULT_TRACE_SIZE,
                               vdd_status, vdn_status);
            setErrlActions(l_err, ERRL_ACTIONS_MANUFACTURING_ERROR);
            // add processor callout
            addCalloutToErrl(l_err,
                             ERRL_CALLOUT_TYPE_HUID,
                             G_sysConfigData.proc_huid,
                             ERRL_CALLOUT_PRIORITY_MED);
            commitErrl(&l_err);
        }
    }

    return foundOT;

} // end process_avsbus_status()


