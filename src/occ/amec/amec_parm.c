/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/amec/amec_parm.c $                                    */
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

//*************************************************************************
// Includes
//*************************************************************************
#include <common_types.h>
#include <amec_parm.h>
#include <string.h>
#include <stdlib.h>
#include <occ_common.h>
#include <amec_amester.h>

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************

//*************************************************************************
// Globals
//*************************************************************************

///Array that maintains a list of all parameters built
extern amec_parm_t              g_amec_parm_list[];

//*************************************************************************
// Function Declarations
//*************************************************************************

//*************************************************************************
// Functions
//*************************************************************************

void amec_parm_get_number(const IPMIMsg_t *i_psMsg,
                          UINT8 *o_pu8Resp,
                          UINT16 *o_pu16RespLength,
                          UINT8 *o_retval)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    o_pu8Resp[0] = (UINT8)(AMEC_PARM_NUMBER_OF_PARAMETERS>>8);
    o_pu8Resp[1] = (UINT8)(AMEC_PARM_NUMBER_OF_PARAMETERS);

    *o_pu16RespLength=2;
    *o_retval=COMPCODE_NORMAL;

    return;
}


void amec_parm_get_config(const IPMIMsg_t *i_psMsg,
                          UINT8 *o_pu8Resp,
                          UINT16 *o_pu16RespLength,
                          UINT8 *o_retval)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    AMEC_PARM_GUID              l_id;   // parameter id
    UINT16                      l_j;    // index into return message
    UINT16                      l_length = 0; // response length
    CHAR                        *l_src; //pointer for copying name

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    l_id = (AMEC_PARM_GUID) CONVERT_UINT8_ARRAY_UINT16(
    i_psMsg->au8CmdData_ptr[1],
    i_psMsg->au8CmdData_ptr[2]);
    l_j = 0; // write index byte for response

    for (; l_id < AMEC_PARM_NUMBER_OF_PARAMETERS; l_id++)
    {
    if (l_j + strlen(g_amec_parm_list[l_id].name) + 1 + 10 >= IPMI_MAX_MSG_SIZE)
    {
        // +1 = null terminator in name.
        // +10 = type, mode, vector_length, length (optional)
        break; // hit end of response buffer
    }

    // Copy name into output buffer
    l_src = g_amec_parm_list[l_id].name;
    do
    {
        o_pu8Resp[l_j++] = *l_src;
    } while (*l_src++ != 0); /* copy string until \0 */

    o_pu8Resp[l_j++] = (UINT8)(g_amec_parm_list[l_id].type);
    o_pu8Resp[l_j++] = (UINT8)(g_amec_parm_list[l_id].mode);
    o_pu8Resp[l_j++] = (UINT8)(g_amec_parm_list[l_id].vector_length>>24);
    o_pu8Resp[l_j++] = (UINT8)(g_amec_parm_list[l_id].vector_length>>16);
    o_pu8Resp[l_j++] = (UINT8)(g_amec_parm_list[l_id].vector_length>>8);
    o_pu8Resp[l_j++] = (UINT8)(g_amec_parm_list[l_id].vector_length);

    // If base type is unstructured data or string, send length
    if (g_amec_parm_list[l_id].type == AMEC_PARM_TYPE_STRING ||
        g_amec_parm_list[l_id].type == AMEC_PARM_TYPE_RAW)
    {
        o_pu8Resp[l_j++] = (UINT8)(g_amec_parm_list[l_id].length>>24);
        o_pu8Resp[l_j++] = (UINT8)(g_amec_parm_list[l_id].length>>16);
        o_pu8Resp[l_j++] = (UINT8)(g_amec_parm_list[l_id].length>>8);
        o_pu8Resp[l_j++] = (UINT8)(g_amec_parm_list[l_id].length);
    }

    // update length of response parameter just copied
    l_length = l_j;
    }
    *o_pu16RespLength=l_length;
    *o_retval=COMPCODE_NORMAL;
    return;
}


void amec_parm_read(const IPMIMsg_t *const i_psMsg,
                    UINT8 *const o_pu8Resp,
                    UINT16 *const o_pu16RespLength,
                    UINT8 *const o_retval)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    AMEC_PARM_GUID              l_id;
    UINT16                      i=0;  // output index
    // l_bytesleft tracks the amount of space remaining in the response in bytes
    // The maximum data in bytes that can be returned is IPMI_MAX_MSG_SIZE - 1
    // The -1 is due to the mandatory returned error code taking 1 byte.
    UINT16                      l_bytesleft = IPMI_MAX_MSG_SIZE - 1;
    UINT8                       *l_src_ptr; // pointer to first byte of data
    UINT8                       *l_end_ptr; // mark end of data
    UINT32                      b;  // start byte
    UINT16                      l_in; // Input byte index
    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    do
    {
        *o_retval = COMPCODE_NORMAL; /* assume no error */

        // Parse input command
        // Get the byte offset for first parameter
        b = CONVERT_UINT8_ARRAY_UINT32(
            i_psMsg->au8CmdData_ptr[1],
            i_psMsg->au8CmdData_ptr[2],
            i_psMsg->au8CmdData_ptr[3],
            i_psMsg->au8CmdData_ptr[4]);

        // Process each parameter in turn
        for (l_in = 5; l_in + 1 < i_psMsg->u8CmdDataLen && l_bytesleft > 0;
             l_in += 2)
        {
            // Get next parameter id
            l_id = CONVERT_UINT8_ARRAY_UINT16(
                i_psMsg->au8CmdData_ptr[l_in],
                i_psMsg->au8CmdData_ptr[l_in+1]);

            if (l_id >= AMEC_PARM_NUMBER_OF_PARAMETERS)
            {
                // Mark which parameter number does not exist
                o_pu8Resp[0] = (uint8_t)(l_in >> 8);
                o_pu8Resp[1] = (uint8_t)(l_in);
                *o_retval = COMPCODE_PARAM_OUT_OF_RANGE;
                *o_pu16RespLength = 2;
                break;
            }

            if (g_amec_parm_list[l_id].preread)
            {
                amec_parm_preread(l_id);
            }

            // Copy value to output buffer
            // Set src to first byte to send back
            l_src_ptr = g_amec_parm_list[l_id].value_ptr + b;

            // Set end pointer 1 beyond last byte to send. It is limited either
            // on the value size, or the IPMI message size.
            l_end_ptr = g_amec_parm_list[l_id].value_ptr
                + (g_amec_parm_list[l_id].vector_length
                   * g_amec_parm_list[l_id].length);

            while ((UINT32)l_src_ptr < (UINT32)l_end_ptr)
            {
                //Copy next byte to output
                o_pu8Resp[i++] = (UINT8)*l_src_ptr++;
                l_bytesleft--;
                if (l_bytesleft == 0) break;
            }
            // Set b for following parameter, if any
            b = 0;
        }
    } while (FALSE);
    // If valid output, then mark the number of valid bytes returned
    if (*o_retval == COMPCODE_NORMAL) *o_pu16RespLength = i;
}


void amec_parm_write(const IPMIMsg_t *const i_psMsg,
                     UINT8 *const o_pu8Resp,
                     UINT16 *const o_pu16RespLength,
                     UINT8 *const o_retval)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    AMEC_PARM_GUID              l_id;
    UINT16                      i=0;  // output index
    UINT8                       *l_dest_ptr = NULL; // pointer to first byte of data
    UINT8                       *l_start_ptr; // mark end of data
    UINT8                       *l_end_ptr = NULL; // mark end of data
    UINT32                      b;  // start byte
    UINT32                      l_bytes = 0; // number of bytes written

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    do
    {
        *o_retval = COMPCODE_NORMAL; /* assume no error */

        // Parse input command
        // Get parameter id
        l_id = CONVERT_UINT8_ARRAY_UINT16(
            i_psMsg->au8CmdData_ptr[1],
            i_psMsg->au8CmdData_ptr[2]);

        // Get the starting byte of element
        b = CONVERT_UINT8_ARRAY_UINT32(
            i_psMsg->au8CmdData_ptr[3],
            i_psMsg->au8CmdData_ptr[4],
            i_psMsg->au8CmdData_ptr[5],
            i_psMsg->au8CmdData_ptr[6]);

        if (l_id >= AMEC_PARM_NUMBER_OF_PARAMETERS)
        {
            *o_retval = COMPCODE_PARAM_OUT_OF_RANGE;
            *o_pu16RespLength = 0;
            break;
        }

        i = 7; // start of data to write in input buffer

        // Check if read-only
        if (g_amec_parm_list[l_id].mode & AMEC_PARM_MODE_READONLY)
        {
            *o_retval = COMPCODE_WRONG_PRIV;
            *o_pu16RespLength = 0;
            break;
        }

        l_start_ptr = g_amec_parm_list[l_id].value_ptr + b;
        l_dest_ptr = l_start_ptr;
        // Set end pointer 1 beyond last byte to send. It is limited either
        // on the value size, or the IPMI message size.
        l_end_ptr = g_amec_parm_list[l_id].value_ptr
            + (g_amec_parm_list[l_id].vector_length * g_amec_parm_list[l_id].length);

        // Copy value from input buffer
        while ((UINT32)l_dest_ptr < (UINT32)l_end_ptr
               && i < i_psMsg->u8CmdDataLen)
        {
            *l_dest_ptr++ = i_psMsg->au8CmdData_ptr[i++];
        }

        l_bytes = l_dest_ptr - l_start_ptr;

        // Return number of bytes written
        *o_pu16RespLength = 4;
        o_pu8Resp[0] = (UINT8)(l_bytes >> 24);
        o_pu8Resp[1] = (UINT8)(l_bytes >> 16);
        o_pu8Resp[2] = (UINT8)(l_bytes >> 8);
        o_pu8Resp[3] = (UINT8)(l_bytes);

        // Run post-write routine only if last byte of parameter was written
        // Some long parameters require multiple write calls due to IPMI
        // message limits, so we only call the postwrite routine when the
        // last byte of the parameter is written.
        if (l_dest_ptr == l_end_ptr && g_amec_parm_list[l_id].postwrite)
        {
            amec_parm_postwrite(l_id);
        }

    } while (FALSE);
}

/*----------------------------------------------------------------------------*/
/* End                                                                        */
/*----------------------------------------------------------------------------*/
