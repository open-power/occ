/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/tools/occutil/cft_occ.C $                                 */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2020,2022                        */
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
/**
  @file cftocctool.C
  @brief Main Program entry point for cmtDllClient Application
*/

#include "cft_utils.H"
#include "cft_occ.H"
#include "cft_tmgt.H"
#include <unistd.h> // sleep

char G_occ_string_file[512] = "/afs/rch/usr2/cjcain/public/occStringFile.p10";
uint16_t G_sensor_type = 0xFFFF;
uint16_t G_sensor_loc  = 0xFFFF;
uint16_t G_sensor_guid = 0;

#define MAX_SENSOR_NAME_SZ          16 // including NULL
#define MAX_SENSOR_UNIT_SZ          4 // including NULL

struct occ_poll_rsp_t
{
    // BYTE  1: Status
    union
    {
        struct
        {
#ifdef _BIG_ENDIAN
            uint8_t master_occ     : 1;   // 1 => master, 0 => slave
            uint8_t _reserved_6    : 1;
            uint8_t _reserved_5    : 1;
            uint8_t pmcr_owner     : 1;   // 1 => Indicates OCC sets the PMCR / Pstates
            uint8_t simics         : 1;   // 1 => running in SIMICS environment
            uint8_t _reserved_2    : 1;
            uint8_t obs_ready      : 1;   // 1 => OCC received all data to support obs state.
            uint8_t active_ready   : 1;   // 1 => OCC received all data to support active state.
#else
            uint8_t active_ready   : 1;   // 1 => OCC received all data to support active state.
            uint8_t obs_ready      : 1;   // 1 => OCC received all data to support obs state.
            uint8_t _reserved_2    : 1;
            uint8_t simics         : 1;   // 1 => running in SIMICS environment
            uint8_t pmcr_owner     : 1;   // 1 => Indicates OCC sets the PMCR / Pstates
            uint8_t _reserved_5    : 1;
            uint8_t _reserved_6    : 1;
            uint8_t master_occ     : 1;   // 1 => master, 0 => slave
#endif
        };
        uint8_t word;
    } status;
    // BYTE  2: Extended Status
    union
    {
        struct
        {
#ifdef _BIG_ENDIAN
            uint8_t dvfs_due_to_ot     : 1;   // 1 => OCC clipped max Pstate due to a processor over temp.
            uint8_t dvfs_due_to_pwr    : 1;   // 1 => OCC clipped max Psate due to reaching pcap limit.
            uint8_t mthrot_due_to_ot   : 1;   // 1 => OCC throttled memory due to an over temp.
            uint8_t n_power            : 1;   // 1 => Server running without redundant power.
            uint8_t dvfs_due_to_vdd_ot : 1;   // 1 => OCC clipped max Pstate due to VRM Vdd over temp.
            uint8_t gpu_throttle       : 3;   // 1 => Bitmask of GPUs being throttled. LSB = GPU0. MSB = GPU2.
#else
            uint8_t gpu_throttle       : 3;   // 1 => Bitmask of GPUs being throttled. LSB = GPU0. MSB = GPU2.
            uint8_t dvfs_due_to_vdd_ot : 1;   // 1 => OCC clipped max Pstate due to VRM Vdd over temp.
            uint8_t n_power            : 1;   // 1 => Server running without redundant power.
            uint8_t mthrot_due_to_ot   : 1;   // 1 => OCC throttled memory due to an over temp.
            uint8_t dvfs_due_to_pwr    : 1;   // 1 => OCC clipped max Psate due to reaching pcap limit.
            uint8_t dvfs_due_to_ot     : 1;   // 1 => OCC clipped max Pstate due to a processor over temp.
#endif
        };
        uint8_t word;
    } ext_status;
    // BYTE  3: OCCs Present
    uint8_t   occ_pres_mask;
    // BYTE  4: Config Data Requested
    uint8_t   config_data;
    // BYTE  5: Current OCC State
    uint8_t   state;
    // BYTE  6: Current OCC Mode
    uint8_t   mode;
    // BYTE  7: Current Idle Power Saver Status
    union
    {
        struct
        {
#ifdef _BIG_ENDIAN
            uint8_t _reserved_7     : 1;
            uint8_t _reserved_6     : 1;
            uint8_t _reserved_5     : 1;
            uint8_t _reserved_4     : 1;
            uint8_t _reserved_3     : 1;
            uint8_t _reserved_2     : 1;
            uint8_t ips_active      : 1;
            uint8_t ips_enabled     : 1;
#else
            uint8_t ips_enabled     : 1;
            uint8_t ips_active      : 1;
            uint8_t _reserved_2     : 1;
            uint8_t _reserved_3     : 1;
            uint8_t _reserved_4     : 1;
            uint8_t _reserved_5     : 1;
            uint8_t _reserved_6     : 1;
            uint8_t _reserved_7     : 1;
#endif
        };
        uint8_t word;
    } ips_status;
    // BYTE  8: Error Log ID
    uint8_t   errl_id;
    // BYTES  9 - 12: Error Log Start Address
    uint32_t  errl_address;
    // BYTES 13 - 14: Error Log Length
    uint16_t  errl_length;
    // BYTE 15: Error Log Source
    uint8_t   errl_source;
    // BYTE 16: GPU Configuration
    uint8_t   gpu_presence;
    // BYTES 17 - 32 (16 bytes): OCC Code Level - ASCII string of OCC build level currently running.
    uint8_t   occ_level[16];
    // BYTES 33 - 38 (6 bytes):  ASCII eye catcher "SENSOR"
    uint8_t   sensor_ec[6];
    // BYTE  39: Number of sensor data blocks
    uint8_t   sensor_dblock_count;
    // BYTE  40: Sensor Data Block Header Version
    uint8_t   sensor_dblock_version;
    // BYTE  41-
    uint8_t   sensor_dblock;
};



// Parse OCC response that include response header
int parse_occ_response_header(const uint8_t  i_cmd,
                              const uint8_t* i_cmd_data,
                              const uint8_t *i_rsp_data,
                              const uint16_t i_rsp_len);
// Parse OCC response that does not include response header
int parse_occ_response(const uint8_t  i_cmd,
                       const uint8_t* i_cmd_data,
                       const uint8_t *i_rsp_data,
                       const uint16_t i_rsp_len);

int parse_occ_poll(const uint8_t *i_rsp_data,
                   const uint16_t i_rsp_len);

int parse_occ_debug_passthru(const uint8_t *i_cmd_data,
                             const uint8_t *i_rsp_data,
                             const uint16_t i_rsp_len);

int parse_occ_mfg_test(const uint8_t *i_cmd_data,
                       const uint8_t *i_rsp_data,
                       const uint16_t i_rsp_len);



const char * getCmdString(const uint8_t i_cmd)
{
    // from src/occ_405/cmdh/cmdh_fsp_cmds.h
    const char *string = "UNKNOWN";
    switch(i_cmd)
    {
        case 0x00: string = "POLL"; break;
        case 0x12: string = "CLEAR_ELOG"; break;
        case 0x20: string = "SET_MODE_STATE"; break;
        case 0x21: string = "SET_CFG_DATA"; break;
        case 0x22: string = "SET_USER_CAP"; break;
        case 0x25: string = "RESET_PREP"; break;
        case 0x30: string = "SEND_AMBIENT"; break;
        case 0x40: string = "DEBUG_PT"; break;
        case 0x41: string = "AME_PT"; break;
        case 0x42: string = "GET_FIELD_DEBUG"; break;
        case 0x53: string = "MFG_TEST"; break;
    }
    return string;
}


const char * getStateString(const uint8_t i_state)
{
    const char *string = "UNKNOWN";
    switch(i_state)
    {
        case 0x01: string = "STANDBY"; break;
        case 0x02: string = "OBSERVATION"; break;
        case 0x03: string = "ACTIVE"; break;
        case 0x04: string = "SAFE"; break;
        case 0x05: string = "CHARACTERISTIC"; break;
        case 0x85: string = "RESET"; break;
        case 0x87: string = "TRANSITION"; break;
        case 0x88: string = "LOADING"; break;
        case 0x89: string = ""; break;
    }
    return string;
}


const char * getModeString(const uint8_t i_mode)
{
    const char *string = "UNKNOWN";
    switch(i_mode)
    {
        case 0x01: string = "STATIC"; break;
        case 0x03: string = "STATIC FREQ POINT"; break;
        case 0x04: string = "SAFE"; break;
        case 0x05: string = "STATIC POWER SAVE"; break;
        case 0x09: string = "MAXIMUM FREQUENCY"; break;
        case 0x0A: string = "DYNAMIC PERFORMANCE"; break;
        case 0x0B: string = "FIXED FREQ OVERRIDE"; break;
        case 0x0C: string = "MAXIMUM PERFORMANCE"; break;
    }
    return string;
}


const char * getPollStatus(const uint8_t i_flags)
{
    // from src/occ_405/cmdh/cmdh_fsp_cmds.h
    static char string[256] = "";
    string[0] = '\0';
    if (i_flags & 0x80) strcat(string, "Master ");
    if (i_flags & 0x10) strcat(string, "OCCPmcrOwner ");
    if (i_flags & 0x08) strcat(string, "SIMICS ");
    if (i_flags & 0x02) strcat(string, "ObsReady ");
    if (i_flags & 0x01) strcat(string, "ActReady ");
    if (i_flags & 0x64) strcat(string, "UNKNOWN ");
    unsigned int len = strlen(string);
    if ((len > 0) && (string[len-1] == ' ')) string[len-1]='\0';
   return string;
}


const char * getPollExtStatus(const uint8_t i_flags)
{
    // from src/occ_405/cmdh/cmdh_fsp_cmds.h
    static char string[256] = "";
    string[0] = '\0';
    if (i_flags != 0xFF)
    {
        if (i_flags & 0x80) strcat(string, "Throttle-ProcOverTemp ");
        if (i_flags & 0x40) strcat(string, "Throttle-Power ");
        if (i_flags & 0x20) strcat(string, "MemThrot-OverTemp ");
        if (i_flags & 0x10) strcat(string, "QuickPowerDrop ");
        if (i_flags & 0x08) strcat(string, "Throttle-VddOverTemp ");
        if (i_flags & 0x04) strcat(string, "GPU2-Throttled ");
        if (i_flags & 0x02) strcat(string, "GPU1-Throttled ");
        if (i_flags & 0x01) strcat(string, "GPU0-Throttled ");
        if (i_flags & 0x00) strcat(string, "UNKNOWN ");
        unsigned int len = strlen(string);
        if ((len > 0) && (string[len-1] == ' ')) string[len-1]='\0';
    }
    return string;
}


const char * getRspStatusString(uint8_t status)
{
    // from src/occ_405/cmdh/cmdh_fsp.h
    const char *string = "";
    switch (status)
    {
        case 0x00: string="SUCCESS"; break;
        case 0x01: string="CONDITIONAL_SUCCESS"; break;
        case 0x11: string="INVALID_COMMAND"; break;
        case 0x12: string="INVALID_CMD_LENGTH"; break;
        case 0x13: string="INVALID_DATA"; break;
        case 0x14: string="CHECKSUM_FAIL"; break;
        case 0x15: string="INTERNAL_FAIL"; break;
        case 0x16: string="INVALID_STATE"; break;
        case 0x17: string="NO_SUPPORT_IN_SMF_MODE"; break;
        case 0xE0: string="EXCEPTION-PANIC"; break;
        case 0xE1: string="EXCEPTION-INIT_CHECKPOINT"; break;
        case 0xE2: string="EXCEPTION-WATCHDOG_TIMER"; break;
        case 0xE3: string="EXCEPTION-OCB_TIMER"; break;
        case 0xE5: string="EXCEPTION-INIT_FAILURE"; break;
        case 0xFF: string="CMD_IN_PROGRESS"; break;
    }
    return string;
}


const char * getFruString(const uint8_t i_fru)
{
    // from src/occ_405/cmdh/cmdh_fsp_cmds_datacnfg.h
    const char * string = "";
    switch(i_fru)
    {
        case 0: string="core"; break;
        case 1: string="membuf"; break;
        case 2: string="dimm"; break;
        case 3: string="memctrl-dram"; break;
        case 4: string="gpu"; break;
        case 5: string="gpu-mem"; break;
        case 6: string="vrm-vdd"; break;
        case 7: string="pmic"; break;
        case 8: string="memctrl-ext"; break;
        case 9: string="proc-io"; break;
    }
    return string;
}


// Return function name for APSS function IDs
const char * getApssFuncName(uint8_t i_funcId)
{
    // from src/occ_405/occ_sys_config.h
    switch (i_funcId)
    {
        case 0x01: return("Mem Proc 0"); break;
        case 0x02: return("Mem Proc 1"); break;
        case 0x03: return("Mem Proc 2"); break;
        case 0x04: return("Mem Proc 3"); break;
        case 0x05: return("Proc 0 Vdd"); break;
        case 0x06: return("Proc 1 Vdd"); break;
        case 0x07: return("Proc 2 Vdd"); break;
        case 0x08: return("Proc 3 Vdd"); break;
        case 0x09: return("Proc 0 Vcs/io/pcie"); break;
        case 0x0A: return("Proc 1 Vcs/io/pcie"); break;
        case 0x0B: return("Proc 2 Vcs/io/pcie"); break;
        case 0x0C: return("Proc 3 Vcs/io/pcie"); break;
        case 0x0D: return("IO A"); break;
        case 0x0E: return("IO B"); break;
        case 0x0F: return("IO C"); break;
        case 0x10: return("Fans A"); break;
        case 0x11: return("Fans B"); break;
        case 0x12: return("Storage A"); break;
        case 0x13: return("Storage B"); break;
        case 0x14: return("(12V voltage sense)"); break;
        case 0x15: return("(ground remote sense)"); break;
        case 0x16: return("Total System Power"); break;
        case 0x17: return("Memory Cache"); break;
        case 0x18: return("Proc 0 GPU 0"); break;
        case 0x19: return("Proc 0 Mem 0"); break;
        case 0x1A: return("Proc 0 Mem 1"); break;
        case 0x1B: return("Proc 0 Mem 2"); break;
        case 0x1C: return("(12V standby current)"); break;
        case 0x1D: return("Proc 0 GPU 1"); break;
        case 0x1E: return("Proc 0 GPU 2"); break;
        case 0x1F: return("Proc 1 GPU 0"); break;
        case 0x20: return("Proc 1 GPU 1"); break;
        case 0x21: return("Proc 1 GPU 2"); break;
        case 0x22: return("PCIe Total"); break;
        case 0x23: return("PCIe DCM0"); break;
        case 0x24: return("PCIe DCM1"); break;
        case 0x25: return("PCIe DCM2"); break;
        case 0x26: return("PCIe DCM3"); break;
        case 0x27: return("IO DCM0"); break;
        case 0x28: return("IO DCM1"); break;
        case 0x29: return("IO DCM2"); break;
        case 0x2A: return("IO DCM3"); break;
        case 0x2B: return("AVdd Total"); break;
        default: return(""); break;
    }
}

const char * getWofReason(const uint32_t i_flags)
{
    // from src/occ_405/cmdh/cmdh_fsp_cmds.h
    static char string[256] = "";
    string[0] = '\0';
    if (i_flags & 0x80) strcat(string, "Throttle-ProcOverTemp ");

    if (i_flags & 0x00000001) strcat(string, "NO_WOF_HEADER_MASK ");
    if (i_flags & 0x00000004) strcat(string, "INVALID_VDD_VDN ");
    if (i_flags & 0x00000010) strcat(string, "PGPE_WOF_DISABLED ");
    if (i_flags & 0x00000020) strcat(string, "PSTATE_PROTOCOL_OFF ");
    if (i_flags & 0x00000040) strcat(string, "VRT_REQ_TIMEOUT ");
    if (i_flags & 0x00000080) strcat(string, "CONTROL_REQ_TIMEOUT ");
    if (i_flags & 0x00000100) strcat(string, "STATE_CHANGE ");
    if (i_flags & 0x00000400) strcat(string, "MODE_NO_SUPPORT_MASK ");
    if (i_flags & 0x00020000) strcat(string, "OCC_WOF_DISABLED ");
    if (i_flags & 0x00040000) strcat(string, "OPPB_WOF_DISABLED ");
    if (i_flags & 0x00080000) strcat(string, "SYSTEM_WOF_DISABLE ");
    if (i_flags & 0x00100000) strcat(string, "RESET_LIMIT_REACHED ");
    unsigned int len = strlen(string);
    if ((len > 0) && (string[len-1] == ' ')) string[len-1]='\0';
    return string;
}


const char * getHistoryName(uint8_t id)
{
    // from src/occ_405/errl/errl.h
    const char *string = "unknown";
    switch (id)
    {
        case 0x01: string="PhantomBCE"; break;
        case 0x02: string="EmpathFail"; break;
        case 0x03: string="NUTIL0Freq"; break;
        case 0x07: string="VddReadFail"; break;
        case 0x09: string="VddOverCurrent"; break;
        case 0x0B: string="InvalidApssData"; break;
        case 0x0C: string="ApssCompleteErr"; break;
        case 0x0D: string="ApssTimeout"; break;
        case 0x0E: string="DcomTxSlvInbox"; break;
        case 0x0F: string="DcomRxSlvInbox"; break;
        case 0x10: string="DcomTxSlvOutbox"; break;
        case 0x11: string="DcomRxSlvOutbox"; break;
        case 0x12: string="DcomMstPbaxSend"; break;
        case 0x13: string="DcomSlvPbaxSend"; break;
        case 0x14: string="DcomMstPbaxRead"; break;
        case 0x15: string="DcomSlvPbaxRead"; break;
        case 0x16: string="Gpe0NotIdle"; break;
        case 0x17: string="Gpe1NotIdle"; break;
        case 0x18: string="24x7Disabled"; break;
        case 0x19: string="CeffRatioVdd"; break;
        case 0x1A: string="VddTemp"; break;
        case 0x1B: string="OverPcapIgnored"; break;
        case 0x1C: string="VFRTTimeoutIgn"; break;
        case 0x1D: string="WOFControlTimeoutIgn"; break;
        case 0x1E: string="PstateChangeIgnored"; break;
        case 0x1F: string="VddCurrentRolloverMax"; break;
        case 0x20: string="CoreSmallDroop"; break;
        case 0x21: string="CoreLargeDroop"; break;
        case 0x22: string="OCSDirtyBlock"; break;
        case 0x23: string="RtlTimeExceeded"; break;
        case 0x24: string="DcomSlvInIncomplete"; break;
        case 0x25: string="DcomSlvLostConn"; break;
        case 0x26: string="24x7NotIdle"; break;
    }
    return string;
}


uint32_t send_occ_command(const uint8_t i_occ,
                          const uint8_t i_cmd,
                          const uint8_t* i_cmd_data,
                          const uint16_t i_len,
                          const uint8_t i_node)
{
    uint32_t rc = CMT_SUCCESS;

    uint8_t *rsp_ptr = NULL;
    uint32_t rsp_length = 0;
    char subcmd[8] = "";
    if ((i_cmd == 0x40) || (i_cmd == 0x53))
    {
        sprintf(subcmd, " / 0x%02X", i_cmd_data[0]);
    }

    if (isFsp())
    {
        printf("Sending %s (0x%02X%s) command to OCC%d in node %d\n",
               getCmdString(i_cmd), i_cmd, subcmd, i_occ, i_node);
        rc = occ_cmd_via_tmgt(i_occ, i_cmd, i_cmd_data, i_len, rsp_ptr, rsp_length, i_node);
    }
    else
    {
        if (G_cmdViaBmc)
        {
            printf("Sending %s (0x%02X%s) command to OCC%d via BMC\n",
                   getCmdString(i_cmd), i_cmd, subcmd, i_occ);
            char command[5300];
            sprintf(command, "busctl call org.open_power.OCC.Control /org/open_power/control/occ%d"
                    " org.open_power.OCC.PassThrough Send ai %d %d %d %d",
                    i_occ, i_len+3, i_cmd, i_len >> 8, i_len & 0xFF);
            for (int i = 0; i < i_len; ++i)
            {
                char append[8] = "";
                sprintf(append, " %d", i_cmd_data[i]);
                strcat(command, append);
            }
            std::string results;
            rc = send_bmc_command(command, results);
            if ((rc == CMT_SUCCESS) && (results.compare(0, 3, "ai ") != 0))
            {
                cmtOutputError("send_occ_command: BMC request failed: %s\n", results.c_str());
                rc = CMT_REQUEST_FAILED;
            }
            else
            {
                rsp_length = bmc_to_array(results, rsp_ptr);
            }
        }
        else
        {
            printf("Sending %s (0x%02X) command to OCC%d via HTMGT\n",
                   getCmdString(i_cmd), i_cmd, i_occ);
            rc = occ_cmd_via_htmgt(i_occ, i_cmd, i_cmd_data, i_len, rsp_ptr, rsp_length);
        }
    }

    if (rc == CMT_SUCCESS)
    {
        printf("%s (0x%02X) response has %d bytes (0x%04X)\n", getCmdString(i_cmd), i_cmd, rsp_length, rsp_length);
        if ((rsp_length > 0) && (rsp_ptr != NULL))
        {
            if (G_cmdViaBmc)
                parse_occ_response_header(i_cmd, i_cmd_data, rsp_ptr, rsp_length);
            else
                parse_occ_response(i_cmd, i_cmd_data, rsp_ptr, rsp_length);
            if ((rsp_ptr != NULL) && (rsp_length > 0)) free(rsp_ptr);
        }
        else
        {
            if (!isFsp()) // FSP could have already parsed data
                printf("WARNING: No data was returned\n");
        }
    }
    else
    {
        cmtOutputError("**** ERROR: Attempt to send %s (0x%02X) command returned 0x%02X\n",
                       getCmdString(i_cmd), i_cmd, rc);
    }

    return rc;
}


int parse_occ_response_header(const uint8_t  i_cmd,
                              const uint8_t* i_cmd_data,
                              const uint8_t *i_rsp_data,
                              const uint16_t i_rsp_len)
{
    printf("  Sequence: 0x%02X\n", i_rsp_data[0]);
    printf("   Command: 0x%02X\n", i_rsp_data[1]);
    printf("    Status: 0x%02X  %s\n", i_rsp_data[2], getRspStatusString(i_rsp_data[2]));
    uint16_t data_len = ((i_rsp_data[3])<<8) + i_rsp_data[4];
    printf("  Data Len: 0x%04X  (%d)\n", data_len, data_len);

    return parse_occ_response(i_cmd, i_cmd_data, &i_rsp_data[5], i_rsp_len - 5);
}

int parse_occ_response(const uint8_t  i_cmd,
                       const uint8_t* i_cmd_data,
                       const uint8_t *i_rsp_data,
                       const uint16_t i_rsp_len)
{
    int rc = 0;

    // response header already removed
    switch(i_cmd)
    {
        case 0x00:
            rc = parse_occ_poll(i_rsp_data, i_rsp_len);
            break;

        case 0x40:
            rc = parse_occ_debug_passthru(i_cmd_data, i_rsp_data, i_rsp_len);
            break;

        case 0x53:
            rc = parse_occ_mfg_test(i_cmd_data, i_rsp_data, i_rsp_len);
            break;

        default:
            rc = -1;
    }

    if (rc != 0)
    {
        // Failed to parse, just dump raw data
        printf("      Data:\n");
        dumpHex(i_rsp_data, i_rsp_len);
    }

    return rc;

} // end parse_occ_response()


int parse_occ_poll(const uint8_t *i_rsp_data, const uint16_t i_rsp_len)
{
    int rc = 0;
    occ_poll_rsp_t *data = (occ_poll_rsp_t*)i_rsp_data;

    if (i_rsp_len < 40)
    {
        printf("ERROR: OCC Response too short! (%d bytes, expected at least 40)\n", i_rsp_len);
        return CMT_DATA_TRUNCATED;
    }
    printf("    Status: 0x%02X  %s\n", data->status.word, getPollStatus(data->status.word));
    printf("Ext Status: 0x%02X  %s\n", data->ext_status.word, getPollExtStatus(data->ext_status.word));
    printf("OCCs Prsnt: 0x%02X\n", data->occ_pres_mask);
    printf("Confg Reqd: 0x%02X\n", data->config_data);
    printf("     State: 0x%02X  %s\n", data->state, getStateString(data->state));
    printf("      Mode: 0x%02X  %s\n", data->mode, getModeString(data->mode));
    if (data->status.master_occ)
        printf("IPS Status: 0x%02X  %s %s\n", data->ips_status.word,
               (data->ips_status.ips_enabled) ? "ENABLED" : "DISABLED",
               (data->ips_status.ips_active) ? "and ACTIVE" : "");
    else
        printf("IPS Status: 0x%02X  N/A\n", data->ips_status.word);
    printf("   Elog ID: 0x%02X  %s\n", data->errl_id, data->errl_id ? "" : "(no error)");
    printf(" Elog Addr: 0x%08X\n", htonl(data->errl_address));
    printf("  Elog Len: 0x%04X\n", htonl(data->errl_length));
    printf("  Elog Src: 0x%02X\n", data->errl_source);
    printf("   GPU Cfg: 0x%02X\n", data->gpu_presence);
    printf("Code Level: \"%.16s\"\n", data->occ_level);
    printf("Sensor Tag: \"%.6s\"\n", data->sensor_ec);
    uint8_t num_blocks = data->sensor_dblock_count;
    printf("  # Blocks: 0x%02X\n", num_blocks);
    printf(" Sens Vers: 0x%02X\n", data->sensor_dblock_version);
    uint8_t *dblock = (uint8_t*)&data->sensor_dblock;
    unsigned int index=0;
    while ((num_blocks > 0) && (40+index+8 < i_rsp_len))
    {
        uint8_t sensorLen = dblock[index+6];
        uint8_t numSensors = dblock[index+7];
        unsigned int sindex = index+8;
        printf("    Sensor: %.4s - format:%d, %d sensors (%d bytes/sensor)\n",
               &dblock[index],  dblock[index+4], numSensors, sensorLen);
        if (strncmp((char *)&dblock[index], "TEMP", 4) == 0)
        {
            printf("                   SSSSSSSS FF TT LL -- (SSSS = Sensor ID, FF is FRU type, TT is temp in C, LL is throttle temp)\n");
            while ((numSensors > 0) && (40+sindex+sensorLen < i_rsp_len))
            {
                if (dblock[sindex+4] != 0xFF) // Valid FRU
                    if (dblock[sindex+5] != 0xFF) // Valid temperature
                        printf("                   %02X%02X%02X%02X %02X %02X %02X %02X (%dC %s)\n",
                               dblock[sindex], dblock[sindex+1], dblock[sindex+2], dblock[sindex+3],
                               dblock[sindex+4], dblock[sindex+5], dblock[sindex+6], dblock[sindex+7],
                               dblock[sindex+5], getFruString(dblock[sindex+4]));
                    else // Error temperature
                        printf("                   %02X%02X%02X%02X %02X %02X %02X %02X  (error)  %s\n",
                               dblock[sindex], dblock[sindex+1], dblock[sindex+2], dblock[sindex+3],
                               dblock[sindex+4], dblock[sindex+5], dblock[sindex+6], dblock[sindex+7],
                               getFruString(dblock[sindex+4]));
                else // Unsupported FRU
                    printf("                   %02X%02X%02X%02X %02X %02X %02X %02X (N/A)\n",
                           dblock[sindex], dblock[sindex+1], dblock[sindex+2], dblock[sindex+3],
                           dblock[sindex+4], dblock[sindex+5], dblock[sindex+6], dblock[sindex+7]);
                sindex += sensorLen;
                --numSensors;
            }
        }
        else if (strncmp((char *)&dblock[index], "FREQ", 4) == 0)
        {
            printf("                   SSSSSSSS FFFF  (SSSS = Sensor ID, FFFF is freq in MHz)\n");
            for (uint8_t jj = 0; jj < numSensors; ++jj)
            {
                printf("                   %08X %04X  ",
                       UINT32_GET(&dblock[sindex]), UINT16_GET(&dblock[sindex+4]));
                if (UINT16_GET(&dblock[sindex+4]))
                    printf("(%4d MHz)\n", UINT16_GET(&dblock[sindex+4]));
                else
                    printf("\n");
                sindex += sensorLen;
            }
        }
        else if (strncmp((char *)&dblock[index], "POWR", 4) == 0)
        {
            printf("                   SSSSSSSS FF CH rrrr TTTTTTTT AAAAAAAAAAAAAAAA CCCC  (SS=Sensor ID, FF=Function ID,\n");
            printf("                               CH=APSS Channel, TT=Update Tag, AA=Accumulator, CC=current reading (W)\n");
            for (uint8_t jj = 0; jj < numSensors; ++jj)
            {
                printf("                   ");
                const char * functionName = getApssFuncName(dblock[sindex+4]);
                printf("%08X %02X %02X %04X %08X %08X%08X %04X  (%5d W)  %s\n", UINT32_GET(&dblock[sindex]),
                                  dblock[sindex+4], dblock[sindex+5],
                                  UINT16_GET(&dblock[sindex+6]), UINT32_GET(&dblock[sindex+8]),
                                  UINT32_GET(&dblock[sindex+12]), UINT32_GET(&dblock[sindex+16]),
                                  UINT16_GET(&dblock[sindex+20]), UINT16_GET(&dblock[sindex+20]), functionName);
                sindex += sensorLen;
            }
        }
        else if (strncmp((char *)&dblock[index], "CAPS", 4) == 0)
        {
            uint16_t cap = UINT16_GET(&dblock[sindex]);
            printf("                      Current Power Cap: %04X  (%5d W)\n", cap, cap);
            cap = UINT16_GET(&dblock[sindex+2]);
            printf("                   Current System Power: %04X  (%5d W)\n", cap, cap);
            cap = UINT16_GET(&dblock[sindex+4]);
            printf("                            N Power Cap: %04X  (%5d W)\n", cap, cap);
            cap = UINT16_GET(&dblock[sindex+6]);
            printf("                   Max System Power Cap: %04X  (%5d W)\n", cap, cap);
            cap = UINT16_GET(&dblock[sindex+8]);
            printf("                     Hard Min Power Cap: %04X  (%5d W)\n", cap, cap);
            if (dblock[sindex+5] > 2)
            {
                cap = UINT16_GET(&dblock[sindex+10]);
                printf("                     Soft Min Power Cap: %04X  (%5d W)\n", cap, cap);
            }
            cap = UINT16_GET(&dblock[sindex+12]);
            if (cap != 0)
            {
                printf("                       User Power Limit: %04X  (%5d W)\n", cap, cap);
            }
            else
            {
                printf("                       User Power Limit: %04X  (DISABLED)\n", cap);
            }
            cap = dblock[sindex+14];
            const char * capName = "";
            if (cap == 1) capName = "(TMGT/BMC)";
            else if (cap == 2) capName = "(OPAL)";
            printf("                   User Power Limit Src: %02X  %s\n", cap, capName);
            sindex += sensorLen;
        }
        else if (strncmp((char *)&dblock[index], "EXTN", 4) == 0)
        {
            printf("                   NNNNNN FF 00 DDDDDDDDDDDD (NNNN = Name/Sensor ID, FF is flags, DDDD is value)\n");
            for (uint8_t jj = 0; jj < numSensors; ++jj)
            {
                const uint32_t name = htonl(UINT32_GET(&dblock[sindex]));
                const uint8_t flags = dblock[sindex+4];
                printf("                   ");
                if (flags & 1)
                {
                    printf(" %08X  %02X %02X %08X%04X", name, flags, dblock[sindex+5],
                                      htonl(*(uint32_t*)&dblock[sindex+6]), htonl(*(uint16_t*)&dblock[sindex+10]));
                }
                else
                {
                    if ((name == ntohl(0x464D494E)) || (name == ntohl(0x46444953)) || // FMIN/FDIS
                        (name == ntohl(0x46424153)) || // FBAS
                        (name == ntohl(0x46555400)) || (name == ntohl(0x464D4158))) // FUT/FMAX
                    {
                        printf("\"%-4.4s\" %02X %02X %08X%04X    pState: %3d / %d MHz", (char*)&name, flags, dblock[sindex+5],
                                          htonl(*(uint32_t*)&dblock[sindex+6]), htons(*(uint16_t*)&dblock[sindex+10]),
                                          dblock[sindex+6], htons(*(uint16_t*)&dblock[sindex+7]));
                    }
                    else if (name == ntohl(0x434C4950)) // CLIP
                    {
                        const char *clip_string = "(OCC NOT clipping)";
                        if (dblock[sindex+7] != 0x00) clip_string = "(OCC clipping)";
                        printf("\"%-4.4s\" %02X %02X %02X %02X %08X  pState: %3d, count: %d, history  %s",
                               (char*)&name, flags, dblock[sindex+5],
                               dblock[sindex+6], dblock[sindex+7], htonl(*(uint32_t*)&dblock[sindex+8]),
                               dblock[sindex+6], dblock[sindex+7], clip_string);
                    }
                    else if (name == ntohl(0x574F4643)) // WOFC
                    {
                        if (dblock[sindex+6] != 0xFF)
                            printf("\"%-4.4s\" %02X %02X %02X%04X %04X %02X  pState: %3d, vRatio: 0x%04X",
                                   (char*)&name, flags, dblock[sindex+5],
                                   dblock[sindex+6], htons(*(uint16_t*)&dblock[sindex+7]),
                                   htons(*(uint16_t*)&dblock[sindex+9]), dblock[sindex+11],
                                   dblock[sindex+6], htons(*(uint16_t*)&dblock[sindex+9]));
                        else
                        {
                            const uint32_t reason = UINT32_GET(&dblock[sindex+8]);
                            printf("\"%-4.4s\" %02X %02X %02X%02X %08X   WOF disabled: %s",
                                   (char*)&name, flags, dblock[sindex+5],
                                   dblock[sindex+6], dblock[sindex+7],
                                   htonl(*(uint32_t*)&dblock[sindex+8]), getWofReason(reason));
                        }
                    }
                    else if (name == ntohl(0x45525248)) // ERRH
                    {
                        printf("\"%-4.4s\" %02X %02X ", (char*)&name, flags, dblock[sindex+5]);
                        if (dblock[sindex+ 6]) printf("0x%02X=%s:%d  ", dblock[sindex+ 6], getHistoryName(dblock[sindex+ 6]), dblock[sindex+7]);
                        if (dblock[sindex+ 8]) printf("0x%02X=%s:%d  ", dblock[sindex+ 8], getHistoryName(dblock[sindex+ 8]), dblock[sindex+9]);
                        if (dblock[sindex+10]) printf("0x%02X=%s:%d  ", dblock[sindex+10], getHistoryName(dblock[sindex+10]), dblock[sindex+11]);
                    }
                    else
                    {
                        printf("\"%-4.4s\" %02X %02X %08X%04X", (char*)&name, flags, dblock[sindex+5],
                                          htonl(*(uint32_t*)&dblock[sindex+6]), htons(*(uint16_t*)&dblock[sindex+10]));
                    }
                }
                printf("\n");
                sindex += sensorLen;
            }
        }
        else
        {
            dumpHex(&i_rsp_data[index+8], sensorLen*numSensors);
            sindex += sensorLen*numSensors;
        }
        index = sindex;
        --num_blocks;
    }
    if ((rc == 0) && (index > i_rsp_len))
    {
        printf("ERROR: OCC Response too short! Data was truncated (received %d bytes)\n", i_rsp_len);
        return CMT_DATA_TRUNCATED;
    }

    return rc;

} // end parse_occ_poll()


int parse_occ_debug_passthru(const uint8_t *i_cmd_data,
                             const uint8_t *i_rsp_data,
                             const uint16_t i_rsp_len)
{
    int rc = -1;

    if (i_cmd_data != NULL)
    {
        switch(i_cmd_data[0])
        {
            case 0x07: // Get Multiple Sensor Data
            case 0x0C: // Get Multiple Sensor Data (with clear)
                {
                    struct sensor_t
                    {
                        char name[16];
                        uint16_t guid;
                        uint16_t current;
                        uint16_t min;
                        uint16_t max;
                    };
                    const uint16_t num_sensors = htons(*(uint16_t*)i_rsp_data);
                    unsigned int sensor= 0;
                    unsigned int offset = 2;
                    printf("Number of sensors retrieved: %d\n", num_sensors);
                    printf("Sensor              GSID  Current    Min     Max\n");
                    printf("------------------------------------------------\n");
                    while ((sensor < num_sensors) && (offset + 14 <= i_rsp_len))
                    {
                        const struct sensor_t *sensorPtr = (struct sensor_t*)(i_rsp_data+offset);
                        printf("%-16.16s  0x%04X  %6d  %6d  %6d\n",
                               sensorPtr->name, htons(sensorPtr->guid),
                               htons(sensorPtr->current), htons(sensorPtr->min), htons(sensorPtr->max));
                        offset += 24;
                        ++sensor;
                    }
                    if (sensor < num_sensors)
                    {
                        printf("WARNING: rsp data too short for %d sensors\n", num_sensors);
                    }
                    rc = 0;
                }
                break;

            default:
                rc = -1;
                break;
        }
    }

    return rc;

} // end parse_occ_debug_passthru()


int parse_occ_mfg_test(const uint8_t *i_cmd_data,
                       const uint8_t *i_rsp_data,
                       const uint16_t i_rsp_len)
{
    int rc = -1;

    if (i_cmd_data != NULL)
    {
        switch(i_cmd_data[0])
        {
            case 0x02: // Frequency Slew
                if (i_rsp_len == 6)
                {
                    printf("    Slew Count: 0x%04X (%3d)\n", UINT16_GET(&i_rsp_data[0]), UINT16_GET(&i_rsp_data[0]));
                    printf("  Start Pstate: 0x%04X (%3d)  (lowest frequency)\n", UINT16_GET(&i_rsp_data[2]), UINT16_GET(&i_rsp_data[2]));
                    printf("   Stop Pstate: 0x%04X (%3d)  (highest frequency)\n", UINT16_GET(&i_rsp_data[4]), UINT16_GET(&i_rsp_data[4]));
                    rc = 0;
                }
                break;

            case 0x06: // Get Sensor Details
                {
                    printf("  MFG Sub Cmd: 0x%02X  (Get Sensor Details)\n\n", i_cmd_data[0]);
                    typedef struct __attribute__ ((packed))
                    {
                        uint16_t                gsid;
                        uint16_t                sample;
                        uint8_t                 status;
                        uint32_t                accumulator;
                        uint16_t                min;
                        uint16_t                max;
                        char                    name[MAX_SENSOR_NAME_SZ];
                        char                    units[MAX_SENSOR_UNIT_SZ];
                        uint32_t                freq;
                        uint32_t                scalefactor;
                        uint16_t                location;
                        uint16_t                type;
                        uint16_t                checksum;
                    } cmdh_mfg_get_sensor_resp_t;
                    unsigned int offset = 0;
                    const cmdh_mfg_get_sensor_resp_t *sensorPtr = (cmdh_mfg_get_sensor_resp_t*)(i_rsp_data+offset);
                    const char *units = sensorPtr->units;
                    printf("         GUID: 0x%04X\n", htons(sensorPtr->gsid));
                    printf("Latest sample: %6d %s (0x%04X)\n", htons(sensorPtr->sample), units, htons(sensorPtr->sample));
                    printf("       Status: 0x%02X\n", sensorPtr->status);
                    printf("  Accumulator: %u (0x%08X)\n", htonl(sensorPtr->accumulator), htonl(sensorPtr->accumulator));
                    printf("   Min Sample: %6d %s (0x%04X)\n", htons(sensorPtr->min), units, htons(sensorPtr->min));
                    printf("   Max Sample: %6d %s (0x%04X)\n", htons(sensorPtr->max), units, htons(sensorPtr->max));
                    printf("         Name: %s\n", sensorPtr->name);
                    printf("        Units: %s\n", units);
                    printf("  Update freq: %u (0x%08X)\n", htonl(sensorPtr->freq), htonl(sensorPtr->freq));
                    uint16_t mantissa=htonl(sensorPtr->scalefactor)>>8;
                    uint16_t exponent=htonl(sensorPtr->scalefactor)&0xFF;
                    if (exponent > 128) exponent -= 256;
                    printf(" Scale Factor: 0x%08X  (%dx10^%d)\n", htonl(sensorPtr->scalefactor), mantissa, exponent);
                    printf(" Sen Location: 0x%04X\n", htons(sensorPtr->location));
                    printf("  Sensor Type: 0x%04X\n", htons(sensorPtr->type));
                    rc = 0;
                }
                break;

            case 0x09: // Memory Slew
                if (i_rsp_len == 2)
                {
                    printf("    Slew Count: 0x%04X (%3d)\n", UINT16_GET(&i_rsp_data[0]), UINT16_GET(&i_rsp_data[0]));
                    rc = 0;
                }
                break;

            default:
                printf("  MFG Sub Cmd: 0x%02X\n\n", i_cmd_data[0]);
                rc = -1;
                break;
        }
    }

    return rc;

} // end parse_occ_mfg_test()


uint32_t get_occ_trace(const uint8_t i_occ, const uint8_t i_node)
{
    uint32_t rc = CMT_SUCCESS;

    if (isFsp())
    {
        printf("Collecting OCC%d trace on node %d\n", i_occ, i_node);
        char command[128];
        sprintf(command, "tmgt trace occsram %d %d", i_occ, i_node);
        rc = send_fsp_command(command);
    }
    else
    {
        printf("Collecting OCC%d trace\n", i_occ);
        char command[1024];
        sprintf(command, "/afs/rch/usr2/cjcain/bin/occtoolp10 -trace -s %s -o %d",
                G_occ_string_file, i_occ);

        bool l_modifiedConfig = false;
        ecmdChipTarget l_target;
#if 0
        // enable SBE FIFO for occtoolp10 for better performance
        ecmdTargetInit(l_target);
        l_modifiedConfig = update_sbe_fifo(l_target, CMT_SBE_FIFO_ON);
        // Does not change it for this system() call - so had to add to command:
#else
        sprintf(command, "setconfig USE_SBE_FIFO on; "
                "/afs/rch/usr2/cjcain/bin/occtoolp10 -trace -s %s -o %d",
                G_occ_string_file, i_occ);
#endif
        printf("==> %s\n", command);
        system(command);

        if ( l_modifiedConfig == true )
            update_sbe_fifo(l_target); // restore old value
    }

    return rc;
}



int get_occ_sensors(const uint8_t i_occ,
                    const uint8_t i_node,
                    const bool i_clear)
{
    int rc = 0;

    if (G_sensor_guid != 0)
    {
        uint8_t cmd_data[] = { 0x06, 0x00, uint8_t(G_sensor_guid >> 8), uint8_t(G_sensor_guid & 0xFF) };
        send_occ_command(i_occ, 0x53, cmd_data, sizeof(cmd_data), i_node);
    }
    else
    {
        uint8_t subcmd = 0x07;
        if (i_clear) subcmd = 0x0C;
        uint8_t cmd_data[] = { subcmd, uint8_t(G_sensor_type >> 8), uint8_t(G_sensor_type & 0xFF),
            uint8_t(G_sensor_loc >> 8), uint8_t(G_sensor_loc & 0xFF) };
        send_occ_command(i_occ, 0x40, cmd_data, sizeof(cmd_data), i_node);
    }

    return rc;

} // end get_occ_sensors
