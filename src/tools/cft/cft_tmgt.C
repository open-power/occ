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
    uint8_t reserved[3];
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
    uint8_t reserved;
    uint8_t safe_mode;
    uint32_t safe_rc;
    uint32_t safe_occ;
    occ_info_t occ[8];
};



uint32_t send_fsp_command(const char *i_cmd);

uint32_t send_hbrt_command(const uint8_t i_cmd,
                           const uint8_t* i_cmd_data,
                           const uint16_t i_len);

void parse_htmgt_response(const uint8_t i_cmd,
                          const uint8_t *i_rsp_data,
                          const uint16_t i_rsp_len);

void parse_tmgt_info(const uint8_t *i_rsp_data,
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
        if (G_verbose)
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
        if (G_verbose)
            dumpHex((uint8_t*)&l_packet, l_packet_length);
        uint8_t *pData = reinterpret_cast<uint8_t *>(&l_packet);
        l_data.memCopyIn(pData, l_packet_length);

        ecmdChipTarget l_target;
        ecmdLooperData looper;

        if (G_verbose)
            printf("occ_cmd_via_htmgt: calling ecmdTargetInit()\n");
        ecmdTargetInit(l_target);
        l_target.posState = ECMD_TARGET_FIELD_VALID;
        l_target.pos      = i_occNum;

        if (G_verbose)
            printf("occ_cmd_via_htmgt: calling ecmdConfigLooperInit()\n");
        l_rc = ecmdConfigLooperInit(l_target, ECMD_SELECTED_TARGETS_LOOP_DEFALL, looper);
        if (l_rc)
        {
            cmtOutputError("%s : ecmdConfigLooperInit failed. RC=0x%08X\n", __FUNCTION__, l_rc);
            l_rc = CMT_INIT_FAILURE;
            break;
        }

        if (G_verbose)
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

    printf("%s returning %d\n", __FUNCTION__, l_rc);
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

    l_rc = send_fsp_command(command);

    if (G_verbose)
        printf("%s returning %d\n", __FUNCTION__, l_rc);

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
        dumpHex(i_cmd_data, i_len);
    }

    if (isFsp())
    {
        if (i_cmd == 0x01)
        {
            l_rc = send_fsp_command("tmgtclient -I");
        }
        else
        {
            cmtOutputError("send_tmgt_command: unsupported TMGT/FSP command 0x%02X", i_cmd);
            l_rc = 6;
        }
    }
    else
    {
       l_rc = send_hbrt_command(i_cmd, i_cmd_data, i_len);
    }

    return l_rc;

} // end send_tmgt_command()


uint32_t send_fsp_command(const char *i_cmd)
{
    uint32_t l_rc = CMT_SUCCESS;

    if ((i_cmd != NULL) && (i_cmd[0] != '\0'))
    {
        ecmdChipTarget  l_target;
        std::string     l_output;
        l_rc = makeSPSystemCall(l_target, i_cmd, l_output);
        if (l_rc)
        {
            printf("ERROR: send_fsp_command(%s) failed with rc=%d\n", i_cmd, l_rc);
            cmtOutputError(l_output.c_str());
        }
        else
        {
            printf("==> %s\n%s\n", i_cmd, l_output.c_str());
        }
    }
    else
    {
        cmtOutputError("send_fsp_command: No FSP command specified!");
        l_rc = 5;
    }

    return l_rc;

} // send_fsp_command()


uint32_t send_hbrt_command(const uint8_t i_cmd,
                           const uint8_t* i_cmd_data,
                           const uint16_t i_len)
{
    uint32_t l_rc = CMT_SUCCESS;

    ecmdDataBuffer l_data;
    uint8_t *l_responseData = NULL;
    uint32_t l_responseSize = 0;

    do
    {
        cmtHtmgtCmd l_packet;
        uint32_t l_packet_length = sizeof(cmtHeader)+1;
        if ((i_cmd_data != 0) && (i_len > 0))
        {
            l_packet_length += i_len;
        }
        initHeader(l_packet.header, l_packet_length);
        l_packet.header.cmdClass    = htons(HTMGT_CMD_CLASS);
        l_packet.header.cmdCode     = htons(HTMGT_CMD_CODE);
        l_packet.htmgtCmd    = i_cmd;
        // command data
        l_data.setByteLength(l_packet_length);
        if ((i_cmd_data != 0) && (i_len > 0))
        {
            memcpy(l_packet.data, i_cmd_data, i_len);
        }
        if (G_verbose)
            dumpHex((uint8_t*)&l_packet, l_packet_length);
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

        l_rc = cmtOCCSendReceive(l_target,l_data,l_responseData,l_responseSize);
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
        else
        {
            printf("WARNING: No data was returned");
        }

    } while (0);

    return l_rc;

} // end send_hbrt_command()


void parse_htmgt_response(const uint8_t i_cmd,
                          const uint8_t *i_rsp_data,
                          const uint16_t i_rsp_len)
{
    if ((i_rsp_data != NULL) && (i_rsp_len > 0))
    {
        switch (i_cmd)
        {
            case 0x01:
                parse_tmgt_info(i_rsp_data, i_rsp_len);
                break;

            default:
                dumpHex(i_rsp_data, i_rsp_len);
                break;
        }
    }
} // end parse_htmgt_response()


void parse_tmgt_info(const uint8_t *i_rsp_data, const uint16_t i_rsp_len)
{
    htmgt_info_t *data = (htmgt_info_t*)i_rsp_data;
    printf("HTMGT State: %s (0x%02X), %d OCC (master:OCC%d), resetCount:%d,",
           getStateString(data->state), data->state, data->num_occs, data->master, data->system_reset_count);
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
        occ_info_t * occd = &data->occ[occ];
        printf("OCC%d: %s %s (0x%02X) resetCount:%d wofResets:%d flags pollRsp:0x%02X%02X%02X%02X... %s\n",
               occd->instance, (occd->instance == data->master)?"Master":"Slave",
               getStateString(occd->state), occd->state,
               occd->reset_count, occd->wof_reset_count, occd->last_poll[0], occd->last_poll[1], occd->last_poll[2], occd->last_poll[3],
               getPollExtStatus(occd->last_poll[1]));
    }
} // end parse_tmgt_info()

