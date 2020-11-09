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


#include "cft_utils.H"
#include "cft_occ.H"
#include "cft_tmgt.H"


const uint16_t  HTMGT_CMD_CLASS   = 0x00E0;
const uint16_t  HTMGT_CMD_CODE    = 0x0001;
//const uint16_t  HBRT_CMD_CLASS    = 0x00E1;
//const uint16_t  HBRT_CMD_CODE     = 0x0002;


struct cmtHtmgtCmd
{
    cmtHeader   header;
    uint8_t     htmgtCmd;
    uint8_t     data[2048];
} __attribute__((packed));

struct cmtOccCmd
{
    cmtHeader   header;
    uint8_t     htmgtCmd;
    uint8_t     occNumber;
    uint8_t     commandType;
    uint8_t     data[2048];
} __attribute__((packed));

struct occ_info_t
{
    uint8_t instance;
    uint8_t state;
    uint8_t role;
    uint8_t master_capable;
    uint8_t comm_established;
    uint8_t mode;
    uint8_t reserved[2];
    uint8_t failed;
    uint8_t needs_reset;
    uint8_t reset_reason;
    uint8_t wof_reset_count:4;
    uint8_t reset_count:4;
    uint8_t last_poll[4];
};

struct htmgt_info_t
{
    uint8_t num_occs;
    uint8_t master;
    uint8_t state;
    uint8_t target_state;
    uint8_t system_reset_count;
    uint8_t reset_since_boot;
    uint8_t mode;
    uint8_t safe_mode;
    uint32_t safe_rc;
    uint32_t safe_occ;
    occ_info_t occ[8];
};

struct htmgt_query_mode_function_t
{
    uint8_t system_mode;
    uint8_t cust_mode;
    uint8_t current_mode;
    uint8_t state;
    uint8_t hftrading;
    uint16_t freq;
    uint8_t occ_status;
    uint8_t power_save;
    uint8_t power_cap;
    uint8_t reserved1;
    uint8_t dyn_perf;
    uint8_t ffo;
    uint8_t reserved2;
    uint8_t max_perf;
    uint8_t reserved3[5];
    uint32_t safe_rc;
    uint32_t safe_occ;
} __attribute__((packed));



uint32_t send_hbrt_command(const htmgt_command i_cmd,
                           const uint8_t* i_cmd_data,
                           const uint16_t i_len,
                           const uint32_t i_timeout = 60 /*seconds*/);

void parse_htmgt_response(const uint8_t i_cmd,
                          const uint8_t *i_rsp_data,
                          const uint16_t i_rsp_len);

int parse_tmgt_info(const uint8_t *i_rsp_data,
                    const uint16_t i_rsp_len);

int parse_query_mode_func(const uint8_t *i_rsp_data,
                          const uint16_t i_rsp_len);

uint32_t occ_cmd_via_htmgt(const uint8_t i_occNum,
                           const uint8_t i_cmd,
                           const uint8_t* i_cmd_data,
                           const uint16_t i_len,
                           uint8_t *& o_responseData,
                           uint32_t & o_responseSize)
{
    uint32_t    l_rc = CMT_SUCCESS;
    ecmdDataBuffer l_data;
    uint8_t *l_responseData = NULL;
    uint32_t l_responseSize = 0;

    do
    {
        cmtOccCmd l_packet;
        uint32_t l_packet_length = sizeof(cmtHeader)+3;
        if ((i_cmd_data != 0) && (i_len > 0))
        {
            l_packet_length += i_len;
        }
        if (G_verbose >= 2)
            printf("occ_cmd_via_htmgt: calling initHeader()\n");
        initHeader(l_packet.header, l_packet_length);
        l_packet.header.cmdClass = htons(HTMGT_CMD_CLASS);
        l_packet.header.cmdCode  = htons(HTMGT_CMD_CODE);
        l_packet.htmgtCmd    = HTMGT_SEND_OCC_CMD;
        l_packet.occNumber   = i_occNum;
        l_packet.commandType = i_cmd;
        l_data.setByteLength(l_packet_length);
        if ((i_cmd_data != 0) && (i_len > 0))
        {
            memcpy(l_packet.data, i_cmd_data, i_len);
        }
        if (G_verbose >= 2)
            dumpHex((uint8_t*)&l_packet, l_packet_length);
        else if (G_verbose)
        {
            printf("occ_cmd_via_htmgt: Sending %s (0x%02X) command to OCC%d (%d bytes of data)\n",
                   getCmdString(i_cmd), i_cmd, i_occNum, i_len);
            if (i_len)
                dumpHex(i_cmd_data, i_len);
        }
        uint8_t *pData = reinterpret_cast<uint8_t *>(&l_packet);
        l_data.memCopyIn(pData, l_packet_length);

        ecmdChipTarget l_target;
        ecmdLooperData looper;

        if (G_verbose >= 2)
            printf("occ_cmd_via_htmgt: calling ecmdTargetInit()\n");
        ecmdTargetInit(l_target);
        l_target.posState = ECMD_TARGET_FIELD_VALID;
        l_target.pos      = i_occNum;

        if (G_verbose >= 2)
            printf("occ_cmd_via_htmgt: calling ecmdConfigLooperInit()\n");
        l_rc = ecmdConfigLooperInit(l_target, ECMD_SELECTED_TARGETS_LOOP_DEFALL, looper);
        if (l_rc)
        {
            cmtOutputError("%s : ecmdConfigLooperInit failed. RC=0x%08X\n", __FUNCTION__, l_rc);
            l_rc = CMT_INIT_FAILURE;
            break;
        }

        if (G_verbose >= 2)
            printf("occ_cmd_via_htmgt: calling ecmdConfigLooperNext()\n");
        l_rc = ecmdConfigLooperNext(l_target, looper);
        if (!l_rc) {
            cmtOutputError("ecmdConfigLooperNext has not returned a valid target\n");
            l_rc = CMT_INVALID_TARGET;
            break;
        }

        if (G_verbose)
            printf("occ_cmd_via_htmgt: calling cmtOCCSendReceive()\n");
        l_rc = cmtOCCSendReceive(l_target,l_data,l_responseData,l_responseSize);
        if (l_rc)
        {
            cmtOutputError("%s : cmtOCCSendReceive() failed. RC=0x%08X\n", __FUNCTION__, l_rc);
            l_rc = CMT_SEND_RECEIVE_FAILURE;
            break;
        }
        o_responseData = l_responseData;
        o_responseSize = l_responseSize;
        if (G_verbose)
        {
            printf("occ_cmd_via_htmgt: response size=%d\n", o_responseSize);
            dumpHex((uint8_t*)o_responseData, o_responseSize);
        }

    } while (0);

    if (G_verbose)
        printf("occ_cmd_via_htmgt: returning %d\n", l_rc);
    return l_rc;

} // end occ_cmd_via_htmgt()


uint32_t occ_cmd_via_tmgt(const uint8_t i_occNum,
                           const uint8_t i_cmd,
                           const uint8_t* i_cmd_data,
                           const uint16_t i_len,
                           uint8_t *& o_responseData,
                           uint32_t & o_responseSize)
{
    uint32_t    l_rc = CMT_SUCCESS;
    char command[1024] = "";
    ecmdDataBuffer l_data;

    sprintf(command, "tmgtclient -X 0x%02X ", i_cmd);
    unsigned int cmd_index = strlen(command);
    if (i_len > 0)
    {
        strncat(command, "-D 0x", 5);
        cmd_index += 5;
        for (unsigned int ii = 0; ii < i_len; ++ii)
        {
            sprintf(&command[cmd_index], "%02X", i_cmd_data[ii]);
            cmd_index += 2;
        }
    }
    sprintf(&command[cmd_index], " -O %d", i_occNum);

    l_rc = send_fsp_command_rspdata(command, o_responseData, o_responseSize);

    if (G_verbose)
        printf("occ_cmd_via_tmgt returning %d\n", l_rc);

    return l_rc;

} // end occ_cmd_via_tmgt()


uint32_t send_tmgt_command(const uint8_t i_cmd,
                           const uint8_t* i_cmd_data,
                           const uint16_t i_len)
{
    uint32_t l_rc = CMT_SUCCESS;

    if (G_verbose)
    {
        printf("send_tmgt_command(command=0x%02X, FSP=%c)\n", i_cmd, isFsp()?'y':'n');
        if ((i_cmd_data != NULL) && (i_len > 0))
            dumpHex(i_cmd_data, i_len);
    }

    if (isFsp())
    {
        if (i_cmd == HTMGT_QUERY_STATE)
        {
            l_rc = send_fsp_command("tmgtclient -I");
        }
        else if (i_cmd == HTMGT_QUERY_MODE_FUNC)
        {
            l_rc = send_fsp_command("tmgtclient --query_mode_and_function");
        }
        else
        {
            cmtOutputError("send_tmgt_command: unsupported TMGT/FSP command 0x%02X", i_cmd);
            l_rc = 6;
        }
    }
    else
    {
       l_rc = send_hbrt_command((htmgt_command)i_cmd, i_cmd_data, i_len);
    }

    return l_rc;

} // end send_tmgt_command()


uint32_t send_htmgt_command(const uint8_t i_cmd,
                           const uint8_t* i_cmd_data,
                           const uint16_t i_len)
{
    uint32_t l_rc = CMT_SUCCESS;

    if (G_verbose)
    {
        printf("send_htmgt_command(command=0x%02X, FSP=%c)\n", i_cmd, isFsp()?'y':'n');
        dumpHex(i_cmd_data, i_len);
    }

    if (isFsp())
    {
        cmtOutputError("send_htmgt_command: Not supported on FSP");
        l_rc = 6;
    }
    else
    {
       l_rc = send_hbrt_command((htmgt_command)i_cmd, i_cmd_data, i_len);
    }

    return l_rc;

} // end send_htmgt_command()


uint32_t send_hbrt_command(const htmgt_command i_cmd,
                           const uint8_t* i_cmd_data,
                           const uint16_t i_len,
                           const uint32_t i_timeout)
{
    uint32_t l_rc = CMT_SUCCESS;

    ecmdDataBuffer l_data;
    uint8_t *l_responseData = NULL;
    uint32_t l_responseSize = 0;

    do
    {
        cmtHtmgtCmd l_packet;
        uint32_t l_packet_length = sizeof(cmtHeader)+1;
        if ((i_len > 0) && (i_cmd_data != 0))
        {
            l_packet_length += i_len;
        }
        initHeader(l_packet.header, l_packet_length);
        l_packet.header.cmdClass    = htons(HTMGT_CMD_CLASS);
        l_packet.header.cmdCode     = htons(HTMGT_CMD_CODE);
        l_packet.htmgtCmd    = i_cmd;
        if (G_verbose)
            printf("HTMGT command 0x%02X w/%d bytes of data\n", i_cmd, i_len);
        // command data
        l_data.setByteLength(l_packet_length);
        if ((i_len > 0) && (i_cmd_data != 0))
        {
            if (G_verbose)
                dumpHex(i_cmd_data, i_len);
            memcpy(l_packet.data, i_cmd_data, i_len);
        }
        uint8_t *pData = reinterpret_cast<uint8_t *>(&l_packet);
        l_data.memCopyIn(pData, l_packet_length);

        ecmdChipTarget l_target;
        ecmdLooperData looper;

        ecmdTargetInit(l_target);
        l_target.posState = ECMD_TARGET_FIELD_VALID;
        l_target.pos      = 0; // i_occNum;

        l_rc = ecmdConfigLooperInit(l_target, ECMD_SELECTED_TARGETS_LOOP_DEFALL, looper);
        if (l_rc)
        {
            cmtOutputError("%s : ecmdConfigLooperInit failed. RC=0x%08X\n", __FUNCTION__, l_rc);
            break;
        }

        l_rc = ecmdConfigLooperNext(l_target, looper);
        if (!l_rc) {
            cmtOutputError("ecmdConfigLooperNext has not returned a valid target\n");
            break;
        }

        l_rc = cmtOCCSendReceive(l_target,l_data,l_responseData,l_responseSize, i_timeout);
        if (l_rc)
        {
            cmtOutputError("%s : cmtOCCSendReceive() failed. RC=0x%08X\n", __FUNCTION__, l_rc);
            break;
        }
        if (G_verbose)
            printf("HTMGT returned %d bytes (0x%04X)\n", l_responseSize, l_responseSize);
        if ((l_responseSize > 0) && (l_responseData != NULL))
        {
            parse_htmgt_response(i_cmd, l_responseData, l_responseSize);
            free(l_responseData);
        }

    } while (0);

    return l_rc;

} // end send_hbrt_command()


void parse_htmgt_response(const uint8_t i_cmd,
                          const uint8_t *i_rsp_data,
                          const uint16_t i_rsp_len)
{
    int l_rc = 0;

    if ((i_rsp_data != NULL) && (i_rsp_len > 0))
    {
        if (G_verbose)
            dumpHex(i_rsp_data, i_rsp_len);
        switch (i_cmd)
        {
            case HTMGT_QUERY_STATE:
                l_rc = parse_tmgt_info(i_rsp_data, i_rsp_len);
                break;

            case HTMGT_QUERY_MODE_FUNC:
                l_rc = parse_query_mode_func(i_rsp_data, i_rsp_len);
                break;

            default:
                l_rc = -1;
                break;
        }

        if (l_rc != 0)
            dumpHex(i_rsp_data, i_rsp_len);
    }
} // end parse_htmgt_response()


int parse_tmgt_info(const uint8_t *i_rsp_data, const uint16_t i_rsp_len)
{
    int l_rc = 0;
    htmgt_info_t *data = (htmgt_info_t*)i_rsp_data;
    if (i_rsp_len >= 16)
    {
        printf("HTMGT State: %s (0x%02X), Mode: %s (0x%02X), %d OCC (master:OCC%d), resetCount:%d,",
               getStateString(data->state), data->state, getModeString(data->mode), data->mode,
               data->num_occs, data->master, data->system_reset_count);
        if (data->safe_mode)
        {
            printf(",  (SAFE MODE: rc=0x%04X/OCC%d)", htonl(data->safe_rc), htonl(data->safe_occ));
        }
        else if (data->safe_rc != 0)
        {
            printf(",  (last reset due to rc=0x%04X/OCC%d)", htonl(data->safe_rc), htonl(data->safe_occ));
        }
        printf("\n");
        if (data->reset_since_boot > 0)
        {
            printf("           : PM Complex Resets since boot: %d\n", data->reset_since_boot);
        }
        for (unsigned int occ = 0; occ < data->num_occs; ++occ)
        {
            if (i_rsp_len >= (16 + (sizeof(occ_info_t)*(occ+1))))
            {
                occ_info_t * occd = &data->occ[occ];
                printf("OCC%d: %s %s (0x%02X) %s (0x%02X) resetCount:%d wofResets:%d flags pollRsp:0x%02X%02X%02X%02X... %s\n",
                       occd->instance, (occd->instance == data->master)?"Master":"Slave ",
                       getStateString(occd->state), occd->state,
                       getModeString(occd->mode), occd->mode,
                       occd->reset_count, occd->wof_reset_count, occd->last_poll[0], occd->last_poll[1], occd->last_poll[2], occd->last_poll[3],
                       getPollExtStatus(occd->last_poll[1]));
            }
            else
            {
                printf("ERROR: parse_tmgt_info: Unable to parse OCC%d - rsp too short! (rcvd %d, exp > %ld)\n",
                       occ, i_rsp_len, (16 + (sizeof(occ_info_t)*(occ+1))));
                l_rc = -1;
            }
        }
    }
    else
    {
        printf("ERROR: parse_tmgt_info: Unable to parse - rsp too short! (rcvd %d, exp >= 16)\n",
               i_rsp_len);
        l_rc = -1;
    }
    return l_rc;

} // end parse_tmgt_info()


int parse_query_mode_func(const uint8_t *i_rsp_data, const uint16_t i_rsp_len)
{
    int l_rc = 0;
    htmgt_query_mode_function_t *data = (htmgt_query_mode_function_t*)i_rsp_data;
    if (i_rsp_len >= sizeof(htmgt_query_mode_function_t))
    {
        printf("            System Mode: 0x%02X  %s\n", data->system_mode, data->system_mode==1?"NORMAL":"SAFE");
        printf("Customer Requested Mode: 0x%02X  %s\n", data->cust_mode, getModeString(data->cust_mode));
        printf("           Current Mode: 0x%02X  %s\n", data->current_mode, getModeString(data->current_mode));
        printf("          Current State: 0x%02X  %s\n", data->state, getStateString(data->state));
        if (data->freq != 0)
            printf("      Desired Frequency: 0x%04X (%d)\n", data->freq, data->freq);
        else
            printf("      Desired Frequency: N/A\n");
        printf("             OCC Status: 0x%02X  %s\n", data->occ_status, data->occ_status==1?"ENABLED":"DISABLED");
        printf("             Power Save? %c\n", data->power_save?'Y':'N');
        printf("          Power Capping? %c\n", data->power_cap?'Y':'N');
        printf("    Dynamic Performance? %c\n", data->dyn_perf?'Y':'N');
        printf("  Fixed Frequency (FFO)? %c\n", data->ffo?'Y':'N');
        printf("    Maximum Performance? %c\n", data->max_perf?'Y':'N');
        if (data->safe_rc)
        {
            printf("  SAFE MODE RC: 0x%04X (OCC%d)", htonl(data->safe_rc), htonl(data->safe_occ));
        }
    }
    else
    {
        printf("ERROR: parse_query_mode_func: Unable to parse - rsp too short! (rcvd %d, exp >= 16)\n",
               i_rsp_len);
        l_rc = -1;
    }

    return l_rc;
} // end parse_query_mode_func()


uint32_t send_tmgt_pmcomplex_reset(const bool i_clear_reset_count)
{
    uint32_t l_rc = CMT_SUCCESS;

    if (G_verbose)
    {
        printf("send_tmgt_pmcomplex_reset(clear resets=%c)\n", i_clear_reset_count?'y':'n');
    }

    if (isFsp())
    {
        if (i_clear_reset_count)
        {
            l_rc = send_fsp_command("tmgtclient --reset_occ_clear 0");
        }
        else
        {
            l_rc = send_fsp_command("tmgtclient --reset_occ 0");
        }
        if (l_rc == CMT_SUCCESS)
        {
            l_rc = tmgt_waitforstate(0x03);
        }
    }
    else
    {
        if (i_clear_reset_count)
        {
            uint32_t l_clear_rc;
            l_clear_rc = send_hbrt_command(HTMGT_CLEAR_RESET_COUNTS, NULL, 0);
            if (l_clear_rc)
            {
                printf("send_tmgt_pmcomplex_reset: Clear of reset counts failed! (before reset) RC=0x%04X\n",
                       l_clear_rc);
            }

            printf("Waiting up to 5 minutes for reset to complete...\n");
            l_rc = send_hbrt_command(HTMGT_RESET_PM_COMPLEX, NULL, 0, 300);

            l_clear_rc = send_hbrt_command(HTMGT_CLEAR_RESET_COUNTS, NULL, 0);
            if (l_clear_rc)
            {
                printf("send_tmgt_pmcomplex_reset: Clear of reset counts failed! RC=0x%04X\n",
                       l_clear_rc);
            }
        }
        else
        {
            l_rc = send_hbrt_command(HTMGT_RESET_PM_COMPLEX, NULL, 0, 300);
        }
    }

    return l_rc;

} // end send_tmgt_pmcomplex_reset


uint32_t tmgt_flags(const uint32_t i_flag_data,
                    const uint16_t i_len)
{
    uint32_t l_rc = CMT_SUCCESS;
    uint32_t l_flag = htonl(i_flag_data);

    if (G_verbose)
    {
        if (i_len == 0)
            printf("tmgt_flags(query)\n");
        else
            printf("tmgt_flags(set 0x%08X)\n", l_flag);
    }

    if (isFsp())
    {
        if (i_len == 0)
            l_rc = send_fsp_command("registry -l tmgt/internalFlags");
        else
        {
            char command[128];
            sprintf(command, "registry -Hw tmgt/internalFlags 0x%08X", l_flag);
            l_rc = send_fsp_command(command);
            // query flags
            l_rc = send_fsp_command("registry -l tmgt/internalFlags");
        }
    }
    else
    {
       l_rc = send_hbrt_command(HTMGT_INTERNAL_FLAGS, (const uint8_t*)&i_flag_data, i_len);
       if (i_len > 0)
       {
           // re-query flags
           (void)send_hbrt_command(HTMGT_INTERNAL_FLAGS, NULL, 0);
       }
    }

    return l_rc;

} // end send_tmgt_command()


uint32_t tmgt_waitforstate(const uint8_t i_state)
{
    uint32_t l_rc = CMT_SUCCESS;

    if (isFsp())
    {
        const char *target_state = "";
        if (i_state == 0x03)
            target_state = "active";
        else
        {
            l_rc = CMT_INVALID_DATA;
        }
        if (target_state[0] != '\0')
        {
            printf("Waiting for OCC state %s...\n", target_state);
            char command[128];
            sprintf(command, "tmgt %s", target_state);
            l_rc = send_fsp_command(command);
        }
    }
    else
    {
        cmtOutputError("tmgt_waitforstate: Not supported!\n");
        l_rc = CMT_INVALID_DATA;
    }

    return l_rc;

} // end tmgt_waitforstate()


uint32_t tmgt_set_state(const uint8_t i_state)
{
    uint32_t l_rc = CMT_SUCCESS;

    if (IS_VALID_STATE(i_state))
    {
            if (isFsp())
            {
                char command[128];
                sprintf(command, "tmgtclient --change_occ_state %d", i_state);
                l_rc = send_fsp_command(command);
            }
            else
            {
                l_rc = send_hbrt_command(HTMGT_SET_OCC_STATE, &i_state, sizeof(i_state));
            }

            if (l_rc == CMT_SUCCESS)
            {
                // Display the resulting state/mode
                (void)send_tmgt_command(HTMGT_QUERY_STATE, NULL, 0);
            }
    }
    else
    {
        cmtOutputError("tmgt_set_state: State %d not supported!\n", i_state);
        l_rc = CMT_INVALID_DATA;
    }

    return l_rc;

} // end tmgt_set_state()


uint32_t tmgt_set_mode(const uint8_t  i_mode,
                       const uint16_t i_freq)
{
    uint32_t l_rc = CMT_SUCCESS;

    if (IS_VALID_MODE(i_mode))
    {

        if ((i_mode == POWERMODE_SFP) ||
            (i_mode == POWERMODE_FFO))
        {
            if (i_freq == 0)
            {
                cmtOutputError("tmgt_set_mode: Mode %d requires a non zero frequency point (-f option)!\n",
                               i_mode);
                l_rc = CMT_INVALID_DATA;
            }
            else
            {
                if (isFsp())
                {
                    char command[128];
                    sprintf(command, "tmgtclient --set_cust_requested_mode 0x%02X -f %d",
                            i_mode, i_freq);
                    l_rc = send_fsp_command(command);
                }
                else
                {
                    const uint8_t l_data[3] = { i_mode, uint8_t(i_freq>>8), uint8_t(i_freq & 0xFF) };
                    l_rc = send_hbrt_command(HTMGT_SET_OCC_MODE, l_data, sizeof(l_data));
                }
            }
        }
        else
        {
            if (isFsp())
            {
                char command[128];
                sprintf(command, "tmgtclient --set_cust_requested_mode 0x%02X", i_mode);
                l_rc = send_fsp_command(command);
            }
            else
            {
                l_rc = send_hbrt_command(HTMGT_SET_OCC_MODE, (uint8_t*)&i_mode, sizeof(i_mode));
            }
        }
        if (l_rc == CMT_SUCCESS)
        {
            // Display the resulting state/mode
            (void)send_tmgt_command(HTMGT_QUERY_STATE, NULL, 0);
        }
    }
    else
    {
        cmtOutputError("tmgt_set_mode: Mode %d not supported!\n", i_mode);
        l_rc = CMT_INVALID_DATA;
    }

    return l_rc;

} // end tmgt_set_mode()
