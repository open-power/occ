/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/tools/cft/cftocctool.C $                                  */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2020                             */
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

#include <ecmdClientCapi.H>
#include "cft_occ.H"
#include "cft_tmgt.H"


struct occ_poll_rsp_t
{
    // BYTE  1: Status
    union
    {
        struct
        {
            uint8_t master_occ     : 1;   // 1 => master, 0 => slave
            uint8_t _reserved_6    : 1;
            uint8_t _reserved_5    : 1;
            uint8_t pmcr_owner     : 1;   // 1 => Indicates OCC sets the PMCR / Pstates
            uint8_t simics         : 1;   // 1 => running in SIMICS environment
            uint8_t _reserved_2    : 1;
            uint8_t obs_ready      : 1;   // 1 => OCC received all data to support obs state.
            uint8_t active_ready   : 1;   // 1 => OCC received all data to support active state.
        };
        uint8_t word;
    } status;
    // BYTE  2: Extended Status
    union
    {
        struct
        {
            uint8_t dvfs_due_to_ot     : 1;   // 1 => OCC clipped max Pstate due to a processor over temp.
            uint8_t dvfs_due_to_pwr    : 1;   // 1 => OCC clipped max Psate due to reaching pcap limit.
            uint8_t mthrot_due_to_ot   : 1;   // 1 => OCC throttled memory due to an over temp.
            uint8_t n_power            : 1;   // 1 => Server running without redundant power.
            uint8_t dvfs_due_to_vdd_ot : 1;   // 1 => OCC clipped max Pstate due to VRM Vdd over temp.
            uint8_t gpu_throttle       : 3;   // 1 => Bitmask of GPUs being throttled. LSB = GPU0. MSB = GPU2.
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
            uint8_t _reserved_7     : 1;
            uint8_t _reserved_6     : 1;
            uint8_t _reserved_5     : 1;
            uint8_t _reserved_4     : 1;
            uint8_t _reserved_3     : 1;
            uint8_t _reserved_2     : 1;
            uint8_t ips_active      : 1;
            uint8_t ips_enabled     : 1;
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



int parse_occ_response(const uint8_t i_cmd,
                       const uint8_t *i_rsp_data,
                       const uint16_t i_rsp_len);
void parse_occ_poll(const uint8_t *i_rsp_data,
                    const uint16_t i_rsp_len);



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


const char * getPollStatus(const uint8_t i_flags)
{
    static char string[256] = "";
    if (i_flags & 0x80) strcat(string, "Master ");
    if (i_flags & 0x40) strcat(string, "CollectFIR ");
    if (i_flags & 0x10) strcat(string, "OCCPmcrOwner ");
    if (i_flags & 0x08) strcat(string, "SIMICS ");
    if (i_flags & 0x02) strcat(string, "ObsReady ");
    if (i_flags & 0x01) strcat(string, "ActReady ");
    if (i_flags & 0x24) strcat(string, "UNKNOWN ");
    unsigned int len = strlen(string);
    if ((len > 0) && (string[len-1] == ' ')) string[len-1]='\0';
   return string;
}


const char * getPollExtStatus(const uint8_t i_flags)
{
    static char string[256] = "";
    if (i_flags & 0x80) strcat(string, "Throttle-ProcOverTemp ");
    if (i_flags & 0x40) strcat(string, "Throttle-Power ");
    if (i_flags & 0x20) strcat(string, "MemThrot-OverTemp ");
    if (i_flags & 0x10) strcat(string, "QuickPowerDrop ");
    if (i_flags & 0x08) strcat(string, "Throttle-VddOverTemp ");
    if (i_flags & 0x07) strcat(string, "UNKNOWN ");
    unsigned int len = strlen(string);
    if ((len > 0) && (string[len-1] == ' ')) string[len-1]='\0';
   return string;
}


const char * getFruString(const uint8_t i_fru)
{
    const char * string = "";
    switch(i_fru)
    {
        case 0: string="core"; break;
        case 1: string="centaur"; break;
        case 2: string="dimm"; break;
        case 3: string="vrm-ot"; break;
        case 4: string="gpu"; break;
        case 5: string="gpu-mem"; break;
        case 6: string="vrm-vdd"; break;
    }
    return string;
}


// Return function name for APSS function IDs
const char * getApssFuncName(uint8_t i_funcId)
{
    switch (i_funcId)
    {
        case 0x01: return("Mem Proc 0"); break;
        case 0x02: return("Mem Proc 1"); break;
        case 0x03: return("Mem Proc 2"); break;
        case 0x04: return("Mem Proc 3"); break;
        case 0x05: return("Proc 0"); break;
        case 0x06: return("Proc 1"); break;
        case 0x07: return("Proc 2"); break;
        case 0x08: return("Proc 3"); break;
        case 0x09: return("Proc 0 cache/io/pcie"); break;
        case 0x0A: return("Proc 1 cache/io/pcie"); break;
        case 0x0B: return("Proc 2 cache/io/pcie"); break;
        case 0x0C: return("Proc 3 cache/io/pcie"); break;
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
        case 0x17: return("Memory Cache (Centaur)"); break;
        case 0x18: return("Proc 0 GPU 0"); break;
        case 0x19: return("Mem Proc 0-0"); break;
        case 0x1A: return("Mem Proc 0-1"); break;
        case 0x1B: return("Mem Proc 0-2"); break;
        case 0x1C: return("(12V standby current)"); break;
        case 0x1D: return("Proc 0 GPU 1"); break;
        case 0x1E: return("Proc 0 GPU 2"); break;
        case 0x1F: return("Proc 1 GPU 0"); break;
        case 0x20: return("Proc 1 GPU 1"); break;
        case 0x21: return("Proc 1 GPU 2"); break;
        default: return(""); break;
    }
}


const char * getHistoryName(uint8_t id)
{
    const char *string = "";
    switch (id)
    {
        case  1: string="VddCurrent"; break;
        case  2: string="VddVoltage"; break;
        case  3: string="VdnCurrent"; break;
        case  4: string="VdnVoltage"; break;
        case  5: string="DimmI2cPort0"; break;
        case  6: string="DimmI2cPort1"; break;
        case  7: string="VddOverTemp"; break;
        case  8: string="VdnOverTemp"; break;
        case  9: string="VddOverCurrent"; break;
        case 10: string="VdnOverCurrent"; break;
        case 11: string="ApssData"; break;
        case 12: string="ApssComplete"; break;
        case 13: string="ApssTimeout"; break;
        case 14: string="DcomTxSlvInbox"; break;
        case 15: string="DcomRxSlvInbox"; break;
        case 16: string="DcomTxSlvOutbox"; break;
        case 17: string="DcomRxSlvOutbox"; break;
        case 18: string="DcomMstPbaxSend"; break;
        case 19: string="DcomSlvPbaxSend"; break;
        case 20: string="DcomMstPbaxRead"; break;
        case 21: string="DcomSlvPbaxRead"; break;
        case 22: string="Gpe0NotIdle"; break;
        case 23: string="Gpe1NotIdle"; break;
        case 24: string="24x7Disabled"; break;
        case 25: string="CeffRatioVdd"; break;
        case 26: string="VddTemp"; break;
        case 27: string="OverPcapIgn"; break;
        case 28: string="VFRTTimeoutIgn"; break;
        case 29: string="WOFControlTimeoutIgn"; break;
        case 30: string="PstateChangeIngored"; break;
        case 31: string="VddCurrentRolloverMax"; break;
        case 32: string="CoreSmallDroop"; break;
        case 33: string="CoreLargeDroop"; break;
    }
    return string;
}


uint32_t send_occ_command(const uint8_t i_occ,
                          const uint8_t i_cmd,
                          const uint8_t* i_cmd_data,
                          const uint16_t i_len)
{
    uint32_t rc = CMT_SUCCESS;
    printf("Sending 0x%02X command to OCC%d\n", i_cmd, i_occ);
    if (G_verbose)
    {
        printf("send_occ_command(OCC%d, command=0x%02X, isFSP=%c)\n", i_occ, i_cmd, isFsp()?'y':'n');
        dumpHex(i_cmd_data, i_len);
    }

    uint8_t *rsp_ptr = NULL;
    uint32_t rsp_length = 0;
    if (isFsp())
    {
        rc = occ_cmd_via_tmgt(i_occ, i_cmd, i_cmd_data, i_len, rsp_ptr, rsp_length);
    }
    else
    {
        rc = occ_cmd_via_htmgt(i_occ, i_cmd, i_cmd_data, i_len, rsp_ptr, rsp_length);
        if (rc == CMT_SUCCESS)
        {
            printf("Poll returned %d bytes (0x%04X)\n", rsp_length, rsp_length);
            if ((rsp_length > 0) && (rsp_ptr != NULL))
            {
                parse_occ_response(i_cmd, rsp_ptr, rsp_length);
                free(rsp_ptr);
            }
            else
            {
                printf("WARNING: No data was returned");
            }
        }
    }

    if (rc != CMT_SUCCESS)
    {
        cmtOutputError("**** ERROR: Attempt to send POLL command returned 0x%02X\n", rc);
    }
    return rc;
}


int parse_occ_response(const uint8_t i_cmd, const uint8_t *i_rsp_data, const uint16_t i_rsp_len)
{
    int rc = 0;

    // response header already removed
    //printf("  Sequence: 0x%02X\n", i_rsp_data[0]);
    //printf("   Command: 0x%02X\n", i_rsp_data[1]);
    //printf("    Status: 0x%02X\n", i_rsp_data[2]);
    //uint16_t data_len = ((i_rsp_data[3])<<8) + i_rsp_data[4];
    //printf("  Data Len: 0x%04X  (%d)\n", data_len, data_len);
    switch(i_cmd)
    {
        case 0x00:
            parse_occ_poll(i_rsp_data, i_rsp_len);
            break;

        default:
            printf("      Data:\n");
            dumpHex(i_rsp_data, i_rsp_len);
    }

    return rc;
}


void parse_occ_poll(const uint8_t *i_rsp_data, const uint16_t i_rsp_len)
{
    occ_poll_rsp_t *data = (occ_poll_rsp_t*)i_rsp_data;

    printf("    Status: 0x%02X  %s\n", data->status.word, getPollStatus(data->status.word));
    printf("Ext Status: 0x%02X  %s\n", data->ext_status.word, getPollExtStatus(data->ext_status.word));
    printf("OCCs Prsnt: 0x%02X\n", data->occ_pres_mask);
    printf("Confg Reqd: 0x%02X\n", data->config_data);
    printf("     State: 0x%02X  %s\n", data->state, getStateString(data->state));
    printf("      Mode: 0x%02X\n", data->mode);
    printf("IPS Status: 0x%02X  %s  %s\n", data->ips_status.word,
           (data->ips_status.ips_enabled) ? "Enabled" : "Disabled",
           (data->ips_status.ips_active) ? "Active" : "");
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
    while (num_blocks > 0)
    {
        uint8_t sensorLen = dblock[index+6];
        uint8_t numSensors = dblock[index+7];
        unsigned int sindex = index+8;
        printf("    Sensor: %.4s - format:%d, %d sensors (%d bytes/sensor)\n",
               &dblock[index],  dblock[index+4], numSensors, sensorLen);
        if (strncmp((char *)&dblock[index], "TEMP", 4) == 0)
        {
            printf("                   SSSSSSSS FF TT  (SSSS = Sensor ID, FF is FRU type, TT is temp in C)\n");
            while (numSensors > 0)
            {
                printf("                   %02X%02X%02X%02X %02X %02X  (%dC %s)\n",
                       dblock[sindex], dblock[sindex+1], dblock[sindex+2], dblock[sindex+3],
                       dblock[sindex+4], dblock[sindex+5], dblock[sindex+5], getFruString(dblock[sindex+4]));
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
                    if ((name == ntohl(0x464D494E)) || (name == ntohl(0x464E4F4D)) || // FMIN/FNOM
                        (name == ntohl(0x46540000)) || (name == ntohl(0x46555400))) // FT/FUT
                    {
                        printf("\"%-4.4s\" %02X %02X %08X%04X    pSstate: %3d / %d MHz", (char*)&name, flags, dblock[sindex+5],
                                          htonl(*(uint32_t*)&dblock[sindex+6]), htons(*(uint16_t*)&dblock[sindex+10]),
                                          dblock[sindex+6], UINT16_GET(&dblock[sindex+7]));
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

}
