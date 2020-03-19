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
#include "cft_utils.H"


void usage()
{
        printf("Usage: cftocctool <options> <commands>\n");
        printf("        Options:\n");
        printf("          -o #   Target specified OCC instance\n");
        printf("          -D XX  Data for other commands (XX is a hex string\n");
        printf("          -v     Verbose\n");
        printf("        Commands:\n");
        printf("          -p     Send POLL command to OCC\n");
        printf("          -I     Query TMGT/OCC states\n");
        printf("          -X XX  Send OCC command XX (use -D to specify data)\n");
        printf("\n         last update: 19-May-2020\n");
}


enum operation_e
{
    OP_HELP,
    OP_OCC_POLL,
    OP_TMGT_INFO,
    OP_SEND_OCC_CMD,
};


int main (int argc, char *argv[])
{
    uint32_t rc = CMT_SUCCESS;
    operation_e op = OP_HELP;
    unsigned int l_occ = 0;

    // Load and initialize the eCMD Dll
    // Which DLL to load is determined by the ECMD_DLL_FILE environment variable
    rc = ecmdLoadDll("");
    if (rc)
    {
        cmtOutputError("**** ERROR : Problems loading eCMD Dll!\n");
        return rc;
    }

    if (argc > 1)
    {
        unsigned int l_occ_cmd = 0xFF;
        uint8_t l_data[1024] = {0};
        unsigned int l_dataLen = 0;

        for (int ii = 1; ii < argc; ii++)
        {
            //printf("arg[%d]: %s\n", ii, argv[ii]);
            if (argv[ii][0] == '-')
            {
                if (strcmp(argv[ii], "-h") == 0) { op = OP_HELP; }
                else if (strcmp(argv[ii], "-p") == 0) { op = OP_OCC_POLL; }
                else if (strcmp(argv[ii], "-I") == 0) { op = OP_TMGT_INFO; }
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
                        printf("ERROR: OCC command (-X option) requires number\n");
                        op = OP_HELP;
                    }
                }
                else if (strcmp(argv[ii], "-D") == 0)
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
                                    printf("ERROR: Unexpected hex data: \"%s\" (byte %d)\n", ptr, l_dataLen);
                                    break;
                                }
                                ptr += 2;
                            }
                        }
                        else
                        {
                            printf("ERROR: DATA (-D option) requires hex string \"%s\" (even number of digits)\n", argv[ii]);
                            op = OP_HELP;
                        }
                    }
                    else
                    {
                        printf("ERROR: DATA (-D option) requires hex string \"%s\"\n", argv[ii+1]);
                        op = OP_HELP;
                    }
                }
                else if (strcmp(argv[ii], "-o") == 0)
                {
                    if ((ii+1 < argc) && (argv[ii+1][0] != '-'))
                    {
                        sscanf(argv[++ii], "%d", &l_occ);
                    }
                    else
                    {
                        printf("ERROR: OCC (-o option) requires number\n");
                        op = OP_HELP;
                    }
                }
                else if (strcmp(argv[ii], "-v") == 0)
                {
                    G_verbose = 1;
                }
                else
                {
                    printf("WARNING: Ignoring unknown option: %s\n", argv[ii]);
                }
            }
            else
            {
                printf("WARNING: Ignoring unknown parameter: %s\n", argv[ii]);
            }
        }

        switch (op)
        {
            case OP_OCC_POLL:
                {
                    uint8_t cmd_data[] = { 0x20 };
                    rc = send_occ_command(l_occ, 0x00, cmd_data, sizeof(cmd_data));
                }
                break;

            case OP_TMGT_INFO:
                {
                    uint8_t cmd_data[] = { 0x20 };
                    rc = send_tmgt_command(0x01, cmd_data, sizeof(cmd_data));
                }
                break;

            case OP_SEND_OCC_CMD:
                {
                    rc = send_occ_command(l_occ, l_occ_cmd, l_data, l_dataLen);
                }
                break;

            case OP_HELP:
            default:
                usage();
        }
    }
    else
    {
        usage();
    }


    // Unload the eCMD Dll, this should always be the last thing you do
    ecmdUnloadDll();

    exit(rc);
}


