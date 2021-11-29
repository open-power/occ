/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/tools/occutil/occutil.C $                                 */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2020,2021                        */
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
  @file occutil.C
  @brief Main Program entry point for cmtDllClient Application
*/

#include <sys/stat.h>
#include "cft_utils.H"
#include "cft_occ.H"
#include "cft_tmgt.H"


void usage()
{
        printf("Usage: occutil <options> <commands>\n\n");
        printf("    Informational Commands:\n");
        printf("        -I             Query TMGT/OCC states\n");
        printf("        -p             Send POLL command to OCC\n");
        printf("        --driver       Display system driver level\n");
        printf("        --system       Query system state\n");
        printf("        --trace        Collect OCC trace (use -s to specify string file)\n");
        printf("           -s file     Specify occStringFile\n");
        printf("        --query        Query mode and function\n");
        printf("        --active_wait  Wait for the OCCs to get to active state\n");
        printf("        -S guid=XX     Dump speecified OCC sensor by GUID\n");
        printf("        -S type=XX,loc=XX Dump OCC sensors (type/loc are both optional)\n");
        printf("           types: 0x1=Generic, 0x2=Current, 0x4=Voltage, 0x8=Temperature,\n");
        printf("                  0x10=Utilization, 0x20=Time, 0x40=Frequency, 0x80=Power,\n");
        printf("                  0x200=Performance, 0x400=WOF\n");
        printf("           locs:  0x1=System, 0x2=Processor, 0x4=Partition, 0x8=Memory,\n");
        printf("                  0x10=VRM, 0x20=OCC, 0x40=Core, 0x80=GPU, 0x100=Quad\n");
        printf("    Commands:\n");
        printf("        --mode #       Set the Power Mode for the system\n");
        printf("                Modes: 1=STATIC, 3=STATIC FREQ POINT*, 5=STATIC POWER SAVE,\n"
               "                       9=MAX FREQ, 10=DYNAMIC PERF, 11=FIXED FREQ*, 12=MAX PERF\n");
        printf("           -f XXXX     Frequency point (required for * modes)\n");
        printf("                SFP points: 0x1000-0x1007=VPD Curve Fit Points, 0x2000=Static Power Save,\n"
               "                       0x2001=WOF Base, 0x2002=Ultra Turbo, 0x2003=Maximum Frequency,\n"
               "                       0x2004=Static, 0x4000=Bottom Throttle Space (most throttled),\n"
               "                       0xFFpp=Specific pstate: pp\n");
        printf("        --state #      Set the OCC state\n");
        printf("                States: 2=OBSERVATION, 3=ACTIVE, 5=CHARACTERIZATION\n");
        printf("        --reset        Reset the PM Complex (waits or OCC to go active)\n");
        printf("        --reset_clear  Reset the PM Complex (and clear reset counts)\n");
        printf("        -H XX          Send HTMGT cmd (use -D to specify data)\n");
        printf("        --IF           Query/set TMGT/HTMGT internal flags (use -D for set)\n");
        printf("        -X XX          Send OCC command XX (use -D to specify data)\n");
        printf("    Options:\n");
        printf("        -o #           Target specified OCC instance\n");
        printf("        -n #           Target specified node (FSP only)\n");
        printf("        -D XX...       Data for other commands (XX is a hex string)\n");
        printf("        -v|-v2|-v3     Verbose (-v2 includes ECMD packets)\n");
        printf("        --STO #        Use SBE timeout to # seconds (default: %d)\n", G_sbe_timeout);
        printf("\n    last update: 24-Nov-2021 - debug\n");
}


enum operation_e
{
    OP_UNDEFINED,
    OP_HELP,
    OP_OCC_POLL,
    OP_TMGT_INFO,
    OP_SEND_OCC_CMD,
    OP_HTMGT_CMD,
    OP_PMCOMPLEX_RESET,
    OP_PMCOMPLEX_RESET_CLEAR,
    OP_OCC_TRACE,
    OP_INTERNAL_FLAGS,
    OP_SYSTEM_STATE,
    OP_ACTIVE_WAIT,
    OP_DRIVER,
    OP_OCC_SENSORS,
    OP_SET_STATE,
    OP_SET_MODE,
    OP_QUERY_MODE_FUNCTION,
};


int main (int argc, char *argv[])
{
    uint32_t rc = CMT_SUCCESS;
    operation_e op = OP_UNDEFINED;
    unsigned int l_occ = 0;
    unsigned int l_fsp_node = 0;

    if (argc == 1)
    {
        op = OP_HELP;
    }

    unsigned int l_occ_cmd = 0xFF;
    uint8_t l_data[1024] = {0};
    unsigned int l_dataLen = 0;
    uint8_t l_state = 0;
    uint16_t l_mode = 0;
    uint16_t l_freq = 0;

    // Parse the parameters
    for (int ii = 1; ii < argc; ii++)
    {
        //printf("arg[%d]: %s\n", ii, argv[ii]);
        if (argv[ii][0] == '-')
        {
            // OPTIONS:

            if ((strcmp(argv[ii], "-h") == 0) || (strcmp(argv[ii], "--help") == 0))
            {
                op = OP_HELP;
            }
            else if ((strcmp(argv[ii], "-D") == 0) || (strcmp(argv[ii], "--data") == 0))
            {
                if ((ii+1 < argc) && (argv[ii+1][0] != '-'))
                {
                    ++ii;
                    if (strlen(argv[ii]) % 2 == 0)
                    {
                        unsigned int data_offset = 0;
                        if (argv[ii][1] == 'x') data_offset = 2;
                        char *ptr = &argv[ii][data_offset];
                        //printf("data\"%s\" is %lu characters long\n", argv[ii], strlen(argv[ii]));
                        while (ptr[0] != '\0')
                        {
                            if (sscanf(ptr, "%2hhX", &l_data[l_dataLen]) == 1)
                            {
                                //printf("--[%.3d]-->%02X\n", l_dataLen, l_data[l_dataLen]);
                                l_dataLen += 1;
                            }
                            else
                            {
                                cmtOutputError("ERROR: Unexpected hex data: \"%s\" (byte %d)\n", ptr, l_dataLen);
                                break;
                            }
                            ptr += 2;
                        }
                    }
                    else
                    {
                        cmtOutputError("ERROR: DATA (-D option) requires hex string \"%s\" (even number of digits)\n", argv[ii]);
                        rc = CMT_INVALID_PARAMETER;
                    }
                }
                else
                {
                    cmtOutputError("ERROR: DATA (-D option) requires hex string \"%s\"\n", argv[ii+1]);
                    rc = CMT_INVALID_PARAMETER;
                }
            }
            else if (strcmp(argv[ii], "-f") == 0)
            {
                if ((ii+1 < argc) && (argv[ii+1][0] != '-'))
                {
                    ++ii;
                    if (argv[ii][1] == 'x')
                        sscanf(&argv[ii][2], "%hX", &l_freq);
                    else
                        sscanf(argv[ii], "%hd", &l_freq);
                }
                else
                {
                    cmtOutputError("ERROR: Frequency point (-f option) requires number\n");
                    rc = CMT_INVALID_PARAMETER;
                }
            }
            else if ((strcmp(argv[ii], "-n") == 0) || (strcmp(argv[ii], "-N") == 0)) // FSP Node
            {
                if ((ii+1 < argc) && (argv[ii+1][0] != '-'))
                {
                    sscanf(argv[++ii], "%d", &l_fsp_node);
                }
                else
                {
                    cmtOutputError("ERROR: node (-n option) requires number\n");
                    rc = CMT_INVALID_PARAMETER;
                }
            }
            else if ((strcmp(argv[ii], "-o") == 0) || (strcmp(argv[ii], "-O") == 0)) // OCC Instance
            {
                if ((ii+1 < argc) && (argv[ii+1][0] != '-'))
                {
                    sscanf(argv[++ii], "%d", &l_occ);
                }
                else
                {
                    cmtOutputError("ERROR: OCC (-o option) requires number\n");
                    rc = CMT_INVALID_PARAMETER;
                }
            }
            else if (strcmp(argv[ii], "-s") == 0) // OCC String File
            {
                if ((ii+1 < argc) && (argv[ii+1][0] != '-'))
                {
                    ++ii;
                    struct stat buffer;
                    if (stat(argv[ii], &buffer) == 0)
                    {
                        strcpy(G_occ_string_file, argv[ii]);
                    }
                    else
                    {
                        cmtOutputError("ERROR: OCC string file not found: %s\n", argv[ii]);
                        rc = CMT_INVALID_PARAMETER;
                    }
                }
                else
                {
                    cmtOutputError("ERROR: -s requires OCC string filename\n");
                    rc = CMT_INVALID_PARAMETER;
                }
            }
            else if (strcmp(argv[ii], "-v") == 0)
            {
                G_verbose = 1;
            }
            else if (strcmp(argv[ii], "-v2") == 0)
            {
                G_verbose = 2;
            }
            else if (strcmp(argv[ii], "-v3") == 0)
            {
                G_verbose = 3;
            }

            // COMMANDS:

            else if (strcmp(argv[ii], "-p") == 0) { op = OP_OCC_POLL; }
            else if (strcmp(argv[ii], "--IF") == 0)
            {
                op = OP_INTERNAL_FLAGS;
            }
            else if (strcmp(argv[ii], "--mode") == 0)
            {
                op = OP_SET_MODE;
                if ((ii+1 < argc) && (argv[ii+1][0] != '-'))
                {
                    ++ii;
                    if (argv[ii][1] == 'x')
                        sscanf(&argv[ii][2], "%hX", &l_mode);
                    else
                        sscanf(argv[ii], "%hu", &l_mode);

                    if (!IS_VALID_MODE(l_mode))
                    {
                        cmtOutputError("ERROR: --mode command requires valid mode parameter\n");
                        rc = CMT_INVALID_PARAMETER;
                    }
                }
                else
                {
                    cmtOutputError("ERROR: --mode command requires valid mode parameter\n");
                    rc = CMT_INVALID_PARAMETER;
                }
            }
            else if (strcmp(argv[ii], "--state") == 0)
            {
                op = OP_SET_STATE;
                if ((ii+1 < argc) && (argv[ii+1][0] != '-'))
                {
                    sscanf(argv[++ii], "%hhu", &l_state);
                }
                if (!IS_VALID_STATE(l_state))
                {
                    cmtOutputError("ERROR: --state requires valid state parameter\n");
                    rc = CMT_INVALID_PARAMETER;
                }
            }
            else if (strcmp(argv[ii], "--system") == 0)
            {
                op = OP_SYSTEM_STATE;
            }
            else if (strcmp(argv[ii], "--driver") == 0)
            {
                op = OP_DRIVER;
            }
            else if (strcmp(argv[ii], "--trace") == 0)
            {
                op = OP_OCC_TRACE;
            }
            else if (strcmp(argv[ii], "--query") == 0)
            {
                op = OP_QUERY_MODE_FUNCTION;
            }
            else if (strcmp(argv[ii], "--reset") == 0)
            {
                op = OP_PMCOMPLEX_RESET;
            }
            else if (strcmp(argv[ii], "--reset_clear") == 0)
            {
                op = OP_PMCOMPLEX_RESET_CLEAR;
            }
            else if (strcmp(argv[ii], "--active_wait") == 0)
            {
                op = OP_ACTIVE_WAIT;
            }
            else if (strcmp(argv[ii], "-H") == 0)
            {
                op = OP_HTMGT_CMD;
                if ((ii+1 < argc) && (argv[ii+1][0] != '-'))
                {
                    ++ii;
                    if (argv[ii][1] == 'x')
                        sscanf(&argv[ii][2], "%X", &l_occ_cmd);
                    else
                        sscanf(argv[ii], "%d", &l_occ_cmd);
                }
                else
                {
                    cmtOutputError("ERROR: HTMGT command (-H option) requires number\n");
                    rc = CMT_INVALID_PARAMETER;
                }
            }
            else if (strcmp(argv[ii], "-I") == 0) { op = OP_TMGT_INFO; }
            else if (strcmp(argv[ii], "-S") == 0)
            {
                op = OP_OCC_SENSORS;
                if ((ii+1 < argc) && (argv[ii+1][0] != '-'))
                {
                    ++ii;
                    char *c = strstr(argv[ii], "loc=");
                    if (c)
                    {
                        if (c[5] == 'x') sscanf(c+6, "%hx", &G_sensor_loc);
                        else sscanf(c+4, "%hu", &G_sensor_loc);
                        printf("Sensor Loc:  0x%04X\n", G_sensor_loc);
                    }
                    c = strstr(argv[ii], "type=");
                    if (c)
                    {
                        if (c[6] == 'x') sscanf(c+7, "%hx", &G_sensor_type);
                        else sscanf(c+5, "%hu", &G_sensor_type);
                        printf("Sensor Type: 0x%04X\n", G_sensor_type);
                    }
                    c = strstr(argv[ii], "guid=");
                    if (c)
                    {
                        if (c[6] == 'x') sscanf(c+7, "%hx", &G_sensor_guid);
                        else sscanf(c+5, "%hu", &G_sensor_guid);
                        printf("Sensor GUID: 0x%04X\n", G_sensor_guid);
                    }
                }
                else
                {
                    G_sensor_type = 0xFFFF;
                    G_sensor_loc  = 0xFFFF;
                }
            }
            else if (strcmp(argv[ii], "-X") == 0)
            {
                op = OP_SEND_OCC_CMD;
                if ((ii+1 < argc) && (argv[ii+1][0] != '-'))
                {
                    ++ii;
                    if (argv[ii][1] == 'x')
                        sscanf(&argv[ii][2], "%X", &l_occ_cmd);
                    else
                        sscanf(argv[ii], "%d", &l_occ_cmd);
                }
                else
                {
                    cmtOutputError("ERROR: OCC command (-X option) requires number\n");
                    rc = CMT_INVALID_PARAMETER;
                }
            }
            else if (strcmp(argv[ii], "--STO") == 0) // SBE Timeout
            {
                if ((ii+1 < argc) && (argv[ii+1][0] != '-'))
                {
                    int newTimeout = 0;
                    sscanf(argv[++ii], "%d", &newTimeout);
                    if (newTimeout > 0)
                    {
                        G_sbe_timeout = newTimeout;
                    }
                    else
                    {
                        cmtOutputError("ERROR: SBE Timeout (--STO option) requires valid time in seconds\n");
                        rc = CMT_INVALID_PARAMETER;
                    }
                }
                else
                {
                    cmtOutputError("ERROR: SBE Timeout (--STO option) requires time in seconds\n");
                    rc = CMT_INVALID_PARAMETER;
                }
            }
            else
            {
                cmtOutputError("ERROR: Unknown command: %s\n", argv[ii]);
                rc = CMT_INVALID_PARAMETER;
            }
        }
        else
        {
            cmtOutputError("ERROR: Unexpected parameter: %s\n", argv[ii]);
            rc = CMT_INVALID_PARAMETER;
        }
    }

    // Execute the operation
    if (rc == CMT_SUCCESS)
    {
        // Load and initialize the eCMD Dll
        // Which DLL to load is determined by the ECMD_DLL_FILE environment variable
        rc = ecmdLoadDll("");
        if (rc)
        {
            cmtOutputError("**** ERROR : Problems loading eCMD Dll!\n");
            return rc;
        }

        switch (op)
        {
            case OP_OCC_POLL:
                {
                    uint8_t cmd_data[] = { 0x20 };
                    rc = send_occ_command(l_occ, 0x00, cmd_data, sizeof(cmd_data), l_fsp_node);
                }
                break;

            case OP_TMGT_INFO:
                {
                    rc = send_tmgt_command(HTMGT_QUERY_STATE, NULL, 0);
                }
                break;

            case OP_SET_STATE:
                {
                    rc = tmgt_set_state(l_state);
                }
                break;

            case OP_SET_MODE:
                {
                    rc = tmgt_set_mode(l_mode, l_freq);
                }
                break;

            case OP_QUERY_MODE_FUNCTION:
                {
                    rc = send_tmgt_command(HTMGT_QUERY_MODE_FUNC, NULL, 0);
                }
                break;

            case OP_HTMGT_CMD:
                {
                    rc = send_htmgt_command(l_occ_cmd, l_data, l_dataLen);
                }
                break;

            case OP_DRIVER:
                {
                    if (isFsp())
                    {
                        rc = send_fsp_command("tmgt driver");
                    }
                    else
                    {
                        printf("BMC Release:\n");
                        rc = send_bmc_command("strings /etc/os-release");
                        printf("PNOR Driver:\n");
                        rc |= send_bmc_command("strings /var/lib/phosphor-software-manager/pnor/ro/VERSION | grep -A99 VERSION");
                    }
                }
                break;

            case OP_SYSTEM_STATE:
                {
                    if (isFsp())
                    {
                        rc = send_fsp_command("tmgt");
                    }
                    else
                    {
                        rc = send_bmc_command("obmcutil state");
                    }
                }
                break;

            case OP_INTERNAL_FLAGS:
                {
                    if ((l_dataLen != 0) && (l_dataLen != 4))
                    {
                        cmtOutputError("ERROR: Invalid Internal Flags data length (data must be 4 bytes for set)");
                        rc = CMT_INVALID_DATA;
                    }
                    else
                    {
                        uint32_t l_flags = 0;
                        if (l_dataLen == 4)
                        {
                            l_flags = *((uint32_t*)l_data);
                        }
                        rc = tmgt_flags(l_flags, l_dataLen);
                    }
                }
                break;

            case OP_OCC_TRACE:
                {
                    rc = get_occ_trace(l_occ, l_fsp_node);
                }
                break;

            case OP_PMCOMPLEX_RESET:
                {
                    rc = send_tmgt_pmcomplex_reset(false);
                }
                break;

            case OP_PMCOMPLEX_RESET_CLEAR:
                {
                    rc = send_tmgt_pmcomplex_reset(true);
                }
                break;

            case OP_ACTIVE_WAIT:
                {
                    rc = tmgt_waitforstate(0x03);
                }
                break;

            case OP_SEND_OCC_CMD:
                {
                    rc = send_occ_command(l_occ, l_occ_cmd, l_data, l_dataLen, l_fsp_node);
                }
                break;

            case OP_OCC_SENSORS:
                {
                    rc = get_occ_sensors(l_occ, l_fsp_node);
                }
                break;

            case OP_HELP:
                usage();
                break;

            default:
                cmtOutputError("ERROR: Unrecognized command: %d\n", op);
                rc = CMT_INVALID_PARAMETER;
                break;
        }

        // Unload the eCMD Dll, this should always be the last thing you do
        ecmdUnloadDll();
    }


    if (rc)
    {
        if (op != OP_HELP)
            cmtOutputError("ERROR: Command (%d) failed with %s (rc=%d)\n",
                           op, getRcString(rc), rc);
    }

    exit(rc);
}


