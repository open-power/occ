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


#include <unistd.h> // sleep
#include "cft_utils.H"


unsigned int G_verbose = 0;

const uint32_t  SBE_HEADER_VERSION      = 0x00010000;
const uint32_t  COMMAND_HEADER_VERSION  = 0x00010000;

struct cmtOCCHeader
{
    uint32_t sbeHeaderVersion;
    uint32_t lengthOfMsg;
    uint32_t sequenceId;
    uint32_t cmdHeaderVersion;
    uint32_t status;
    uint32_t dataOffset;
    uint32_t dataSize;
} __attribute__((packed));

struct cmtOCCResponsePacket
{
    cmtOCCHeader responseHeader;
    uint16_t     cmdClass;
    uint16_t     cmdCode;
    uint8_t*     data;
} __attribute__((packed));

const uint32_t CMT_OCC_HEADER_STATUS_OFFSET_IN_BYTES    = offsetof(cmtOCCHeader, status);
const uint32_t CMT_OCC_MSG_LENGTH_OFFSET_IN_BYTES       = offsetof(cmtOCCHeader, lengthOfMsg);
const uint32_t CMT_OCC_HEADER_DATA_SIZE_OFFSET_IN_BYTES = offsetof(cmtOCCHeader, dataSize);
const uint32_t CMT_OCC_RESPONSE_DATA_OFFSET_IN_BYTES    = offsetof(cmtOCCResponsePacket, data);


#define   USE_SBE_FIFO_STR    "USE_SBE_FIFO"
#define   CMT_SBE_FIFO_OFF    "off"
#define   CMT_SBE_FIFO_ON     "on"
const uint64_t HOST_PASS_THROUGH_MEM_ADDRESS      = 0x0;
const uint64_t HOST_PASS_THROUGH_MEM_ADDRESS_128  = 0x80;
const uint32_t CFAM_REGISTER_ADDRESS              = 0x283B;
const uint32_t READ_CFAM_WAIT_TIME                = 2;  //in seconds
const uint32_t CMT_HBRT_CMD_COMPLETE_BIT                = 0x0;
const uint32_t CMT_HBRT_CMD_IN_PROGRESS_BIT             = 0x1;
const uint32_t DATA_128_BYTE_ALLIGN    = 128;


void dumpHex(const uint8_t *data, const unsigned int i_len)
{
    unsigned int i, j;
    char text[17];

    text[16] = '\0';

    unsigned int len = i_len;
    if (len > 0x1000)
        len = 0x1000;
    for(i = 0; i < len; i++)
    {
        if (i % 16 == 0)
        {
            if (i > 0) printf("   \"%s\"\n", text);
            printf("   %04X:",i);
        }
        if (i % 4 == 0) printf(" ");

        printf("%02X",(int)data[i]);
        if (isprint(data[i])) text[i%16] = data[i];
        else text[i%16] = '.';
    }
    if ((i%16) != 0) {
        text[i%16] = '\0';
        for(j = (i % 16); j < 16; ++j) {
            printf("  ");
            if (j % 4 == 0) printf(" ");
        }
    }
    printf("   \"%s\"\n", text);
    if (len < i_len)
    {
        printf("WARNING: data was truncated from %d to %d\n", i_len, len);
    }
    return;
}


void dumpHex(ecmdDataBuffer &i_data, const unsigned int i_length=0)
{
    unsigned length = i_data.getByteLength();
    if ((i_length > 0) && (i_length < length))
    {
        length = i_length;
    }

    uint8_t l_buffer[length];
    i_data.memCopyOut(l_buffer, length);
    dumpHex(l_buffer, length);

    return;
}


void initHeader(cmtHeader & i_header, uint32_t i_lengthOfMsg)
{
    i_header.sbeHeaderVersion   = htonl(SBE_HEADER_VERSION);
    i_header.lengthOfMsg        = htonl(i_lengthOfMsg);
    i_header.sequenceId         = 0x0;
    i_header.cmdHeaderVersion   = htonl(COMMAND_HEADER_VERSION);
    i_header.status             = 0x0;
    i_header.dataOffset         = htonl(sizeof(cmtHeader) - offsetof(cmtHeader,cmdHeaderVersion));
    i_header.dataSize           = htonl(i_lengthOfMsg - sizeof(cmtHeader));
}

void ecmdTargetInit(ecmdChipTarget &o_target)
{
    o_target.chipType          = "pu";
    o_target.chipTypeState     = ECMD_TARGET_FIELD_VALID;
    o_target.posState          = ECMD_TARGET_FIELD_WILDCARD;
    o_target.cageState         = ECMD_TARGET_FIELD_WILDCARD;
    o_target.nodeState         = ECMD_TARGET_FIELD_WILDCARD;
    o_target.slotState         = ECMD_TARGET_FIELD_WILDCARD;
    o_target.chipUnitTypeState = ECMD_TARGET_FIELD_UNUSED;
    o_target.chipUnitNumState  = ECMD_TARGET_FIELD_UNUSED;
    o_target.threadState       = ECMD_TARGET_FIELD_UNUSED;
}


uint32_t cmtOCCSendReceive(ecmdChipTarget &i_target,
                           ecmdDataBuffer &i_data,
                           uint8_t       *&o_responseData,
                           uint32_t       &o_responseSize,
                           const uint32_t  i_timeout)
{
    uint32_t l_rc = CMT_SUCCESS;

    o_responseData = NULL;
    o_responseSize = 0;

    //configure ecmd to use SBE FIFO for OCC pass through commands
    //
    ecmdConfigValid_t l_existingConfigValid;
    std::string       l_existingValueAlpha;
    uint32_t          l_existingValueNumeric;
    bool              l_modifiedConfig = false;
    if (G_verbose >= 2)
        printf("cmtOCCSendReceive: calling ecmdGetConfiguration()\n");
    l_rc = ecmdGetConfiguration(i_target, USE_SBE_FIFO_STR, l_existingConfigValid, l_existingValueAlpha, l_existingValueNumeric);
    if ( l_rc ) {
        cmtOutputError("%s : unable to determine whether SBE has been configured to use FIFO. Proceeding further... RC=0x%08X\n",
                       __FUNCTION__, l_rc);
        l_rc = CMT_SUCCESS;
    } else {

        //If SBE FIFO is not on, then update the configuration to set it to on
        if ( strcasecmp(l_existingValueAlpha.c_str(),CMT_SBE_FIFO_ON) != 0 )  {

            if (G_verbose >= 2)
                printf("cmtOCCSendReceive: calling ecmdGetConfiguration(ECMD_CONFIG_VALID_FIELD_ALPHA, CMT_SBE_FIFO_ON)\n");
            l_rc = ecmdSetConfiguration(i_target, USE_SBE_FIFO_STR, ECMD_CONFIG_VALID_FIELD_ALPHA, CMT_SBE_FIFO_ON, 0);
            if ( l_rc ) {
                cmtOutputError("%s : update of USE_SBE_FIFO to on failed. Exiting... RC=0x%08X\n",
                               __FUNCTION__, l_rc);
                return l_rc;
            }
            l_modifiedConfig = true;
        }
    }

    do
    {
        uint32_t l_bytes = i_data.getByteLength();

        if (G_verbose >= 2)
        {
            printf("cmtOCCSendReceive: calling putMemPba(0x%08X)\n", (unsigned int)HOST_PASS_THROUGH_MEM_ADDRESS);
            uint8_t l_buffer[l_bytes];
            i_data.memCopyOut(l_buffer, l_bytes);
            dumpHex(l_buffer, l_bytes);
        }
        l_rc = putMemPba(i_target, HOST_PASS_THROUGH_MEM_ADDRESS, l_bytes, i_data, (PBA_MODE_DMA|PBA_OPTION_PASSTHROUGH));
        if(l_rc)
        {
            cmtOutputError("%s : putMemPba() failed. RC=0x%08X\n", __FUNCTION__, l_rc);
            break;
        }

        uint32_t l_polling_interval = READ_CFAM_WAIT_TIME;
        if (i_timeout <= 60)
        {
            l_polling_interval = 1; // Check once per second
        }
        uint32_t l_timeout = i_timeout / l_polling_interval;
        if (i_timeout < l_polling_interval)
        {
            l_polling_interval = i_timeout;
        }
        uint32_t l_attempt = 0;
        ecmdDataBuffer l_regData;
        if (G_verbose)
            printf("cmtOCCSendReceive: Waiting for response (timeout=%d sec, %d sec between polls)\n",
                   l_timeout, l_polling_interval);
        do {
            //if (G_verbose)
            //    printf("cmtOCCSendReceive: sleeping %d sec (%d attempts remaining)\n",
            //           l_polling_interval, l_timeout-l_attempt);
            sleep(l_polling_interval);
            //if (G_verbose)
            //    printf("cmtOCCSendReceive: calling getCfamRegister(0x%04X)\n", CFAM_REGISTER_ADDRESS);
            l_rc = getCfamRegister(i_target,CFAM_REGISTER_ADDRESS,l_regData);
            if(l_rc) {
                cmtOutputError("%s : getCfamRegister() failed. RC=0x%08X\n", __FUNCTION__, l_rc);
                break;
            }
            l_attempt++;
        } while ((l_attempt < l_timeout) && (!l_regData.isBitSet(CMT_HBRT_CMD_COMPLETE_BIT)));


        if(l_rc) {
            break;
        }

        if ( (l_attempt == l_timeout) && (!l_regData.isBitSet(CMT_HBRT_CMD_COMPLETE_BIT)) ) {
            cmtOutputError("Host pass through commmand has timed out! (%d sec)\n", i_timeout);
            l_rc = 0x55; //CMT_HOST_PASSTHROUGH_TIMEOUT;
            break;
        }
        else if (G_verbose >= 2)
        {
            printf("cmtOCCSendReceive: request completed in %d sec\n",
                   l_attempt * l_polling_interval);
        }

        ecmdDataBuffer l_header;
        //Reading 128 bytes to get data size from HBRT header
        if (G_verbose >= 2)
            printf("cmtOCCSendReceive: calling getMemPbaHidden(0x%08X, %d)\n",
                   (unsigned int)HOST_PASS_THROUGH_MEM_ADDRESS, DATA_128_BYTE_ALLIGN);
        l_rc = getMemPbaHidden(i_target, HOST_PASS_THROUGH_MEM_ADDRESS, DATA_128_BYTE_ALLIGN, l_header,(PBA_MODE_LCO|PBA_OPTION_PASSTHROUGH));
        if(l_rc) {
            cmtOutputError("%s : getMemPbaHidden() failed. RC=0x%08X\n", __FUNCTION__, l_rc);
            break;
        }
        else if (G_verbose >= 2)
            dumpHex(l_header);

        //This l_rc indicates the Status returned by HBRT as a response of executing
        //the requested command
        l_rc = l_header.getWord(CMT_OCC_HEADER_STATUS_OFFSET_IN_BYTES/sizeof(uint32_t));
        if ( l_rc != CMT_SUCCESS ) {
            cmtOutputError("%s : Host passthrough command returned error. RC=0x%08X\n", __FUNCTION__, l_rc);
            break;
        }

        uint32_t l_dataSize = l_header.getWord(CMT_OCC_HEADER_DATA_SIZE_OFFSET_IN_BYTES/sizeof(uint32_t));
        //No response data for this command
        if ( l_dataSize == 0 ) {
            break;
        }

        uint32_t l_responseSize = l_header.getWord(CMT_OCC_MSG_LENGTH_OFFSET_IN_BYTES/sizeof(uint32_t));
        uint8_t *l_dataPtr = (uint8_t*)malloc(l_dataSize);

        ecmdDataBuffer l_tempData;
        ecmdDataBuffer l_data;

        if(l_responseSize > DATA_128_BYTE_ALLIGN)
        {
            uint32_t l_remainingRespSize = l_responseSize - DATA_128_BYTE_ALLIGN;
            l_bytes = ((l_remainingRespSize)%DATA_128_BYTE_ALLIGN)? (l_remainingRespSize-(l_remainingRespSize%DATA_128_BYTE_ALLIGN)+DATA_128_BYTE_ALLIGN) : l_remainingRespSize;
            if (G_verbose >= 2)
                printf("cmtOCCSendReceive: calling getMemPbaHidden(0x%08X, %d bytes)\n",
                       (unsigned int)HOST_PASS_THROUGH_MEM_ADDRESS_128, l_bytes);
            l_rc = getMemPbaHidden(i_target, HOST_PASS_THROUGH_MEM_ADDRESS_128, l_bytes, l_tempData,(PBA_MODE_LCO|PBA_OPTION_PASSTHROUGH));
            if(l_rc) {
                cmtOutputError("%s : getMemPbaHidden() failed. RC=0x%08X\n", __FUNCTION__, l_rc);
                break;
            }
            l_data.concat(l_header,l_tempData);
            l_data.shiftLeftAndResize((CMT_OCC_RESPONSE_DATA_OFFSET_IN_BYTES)*8);
            l_data.memCopyOut(l_dataPtr,l_dataSize);
        }
        else
        {
            l_header.shiftLeftAndResize((CMT_OCC_RESPONSE_DATA_OFFSET_IN_BYTES)*8);
            l_header.memCopyOut(l_dataPtr,l_dataSize);
        }
        o_responseSize = l_dataSize;
        o_responseData = l_dataPtr;
    } while (0);

    //In USE_SBE_FIFO was set to on, then set the original value back.
    //If this fails, then ignore the error
    if ( l_modifiedConfig == true ) {
        //No need to check the rc
        if (G_verbose >= 2)
            printf("cmtOCCSendReceive: calling ecmdSetConfiguration(restore prior)\n");
        ecmdSetConfiguration(i_target, USE_SBE_FIFO_STR, l_existingConfigValid,
                             l_existingValueAlpha, l_existingValueNumeric);
    }
    return l_rc;
}


bool isFsp()
{
    bool is_fsp = false;

    ecmdChipTarget l_target;
    std::string spType;
    unsigned int rc = ecmdQuerySP( l_target, "SPTYPE", spType );
    if (rc)
    {
        cmtOutputError("is_fsp: ecmdQuerySP failed. RC = 0x%08X \n", rc);
    }
    else
    {
        if (G_verbose >= 2)
            printf("isFSP: ecmdQuerySP returned %s\n", spType.c_str());
        if (spType == "FSP")
            is_fsp = true;
    }

    return is_fsp;
}


uint32_t send_fsp_command(const char *i_cmd)
{
    uint32_t l_rc = CMT_SUCCESS;

    if ((i_cmd != NULL) && (i_cmd[0] != '\0'))
    {
        if (isFsp())
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
                printf("-FSP-> %s\n", i_cmd);
                printf("%s\n", l_output.c_str());
                fflush(stdout);
            }
        }
        else
        {
            cmtOutputError("send_fsp_command: Not supported on BMC system!\n");
            l_rc = CMT_NOT_SUPPORTED;
        }
    }
    else
    {
        cmtOutputError("send_fsp_command: No FSP command specified!\n");
        l_rc = CMT_INVALID_DATA;
    }

    return l_rc;

} // send_fsp_command()


// Convert FSP output of OCC response buffer to binary data (to parse)
int convert_rsp_to_binary(const char *   i_output,
                          uint8_t *    & o_rspdata,
                          uint32_t     & o_rsplen)
{
    int rc = -1;
    o_rspdata = NULL;
    o_rsplen = 0;

    const char *s = strstr(i_output, "failure rc=");
    if (s == NULL)
    {
        s = strstr(i_output, "status:   0x00");
        if (s != NULL)
        {
            s = strstr(i_output, "data len:");
            if (s != NULL)
            {
                sscanf(s+12, "%04X", &o_rsplen);

                // print response header
                const unsigned int hlen = s+17 - i_output;
                printf("%.*s\n", hlen, i_output);

                if (o_rsplen > 0)
                {
                    char const *d = strstr(s, "0000:  ");
                    if (d != NULL)
                    {
                        unsigned int offset = 0;
                        uint8_t *buffer = (uint8_t*)malloc(o_rsplen);
                        while (offset < o_rsplen)
                        {
                            if (offset % 16 == 0)
                            {
                                d += 6;
                                if (offset > 0) ++d; // strip newline
                            }
                            if (offset % 4 == 0) ++d;
                            unsigned int hex;
                            sscanf(d, "%02X", &hex);
                            buffer[offset] = hex & 0xFF;
                            d += 2;
                            ++offset;
                        }
                        o_rspdata = buffer;
                        rc = 0;
                    }
                    else
                    {
                        if (G_verbose)
                            printf("convert_rsp_to_binary: \"0000:\" was not found - data already parsed?\n");
                    }
                }
            }
            else
            {
                printf("convert_rsp_to_binary: unable to find \"data len:\"\n");
            }
        }
        else
        {
            if (G_verbose)
                printf("convert_rsp_to_binary: OCC rsp status was not success\n");
        }
    }
    else
    {
        printf("TMGT returned ERROR\n");
    }

    return rc;

} // end convert_rsp_to_binary()


uint32_t send_fsp_command_rspdata(const char * i_cmd,
                                  uint8_t *  & o_rspdata,
                                  uint32_t   & o_rsplen)
{
    uint32_t l_rc = CMT_SUCCESS;
    o_rspdata = NULL;
    o_rsplen = 0;

    if ((i_cmd != NULL) && (i_cmd[0] != '\0'))
    {
        if (isFsp())
        {
            ecmdChipTarget  l_target;
            std::string     l_output;
            l_rc = makeSPSystemCall(l_target, i_cmd, l_output);
            if (l_rc)
            {
                printf("ERROR: send_fsp_command_rspdata(%s) failed with rc=%d (0x%02X)\n",
                       i_cmd, l_rc, l_rc);
                cmtOutputError(l_output.c_str());
            }
            else
            {
                printf("-FSP-> %s\n", i_cmd);
                if (convert_rsp_to_binary(l_output.c_str(), o_rspdata, o_rsplen) != 0)
                {
                    // Failed to convert, so just output raw output
                    printf("%s\n", l_output.c_str());
                    fflush(stdout);
                }
            }
        }
        else
        {
            cmtOutputError("send_fsp_command_rspdata: Not supported on BMC system!\n");
            l_rc = CMT_NOT_SUPPORTED;
        }
    }
    else
    {
        cmtOutputError("send_fsp_command_rspdata: No FSP command specified!\n");
        l_rc = CMT_INVALID_DATA;
    }

    return l_rc;

} // send_fsp_command_rspdata()


uint32_t send_bmc_command(const char *i_cmd)
{
    uint32_t l_rc = CMT_SUCCESS;

    if ((i_cmd != NULL) && (i_cmd[0] != '\0'))
    {
        if (!isFsp())
        {
            ecmdChipTarget  l_target;
            std::string     l_output;
            l_rc = makeSPSystemCall(l_target, i_cmd, l_output);
            if (l_rc)
            {
                printf("ERROR: send_bmc_command(%s) failed with rc=%d\n", i_cmd, l_rc);
                cmtOutputError(l_output.c_str());
            }
            else
            {
                printf("-BMC-> %s\n", i_cmd);
                printf("%s\n", l_output.c_str());
                fflush(stdout);
            }
        }
        else
        {
            cmtOutputError("send_bmc_command: Not supported on FSP system!\n");
            l_rc = CMT_NOT_SUPPORTED;
        }
    }
    else
    {
        cmtOutputError("send_bmc_command: No BMC command specified!\n");
        l_rc = CMT_INVALID_DATA;
    }

    return l_rc;

} // send_bmc_command()


