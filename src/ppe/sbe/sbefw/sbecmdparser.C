/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/sbe/sbefw/sbecmdparser.C $                            */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015                             */
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
/*
 * @file: ppe/sbe/sbefw/sbecmdparser.C
 *
 * @brief This file contains the SBE FIFO Commands
 *
 */

#include "sbecmdparser.H"
#include "sbecmdscomaccess.H"
#include "sbecmdiplcontrol.H"
#include "sbetrace.H"
#include "sbe_sp_intf.H"


////////////////////////////////////////////////////////////////
// @brief g_sbeScomCmdArray
////////////////////////////////////////////////////////////////
static sbeCmdStruct_t g_sbeScomCmdArray [] =
{
    {sbeGetScom,
     SBE_CMD_GETSCOM,
     SBE_FENCE_AT_CONTINUOUS_IPL,
     },

    {sbePutScom,
     SBE_CMD_PUTSCOM,
     SBE_FENCE_AT_CONTINUOUS_IPL,
     },
};

////////////////////////////////////////////////////////////////
// @brief g_sbeScomCmdArray
//
////////////////////////////////////////////////////////////////
static sbeCmdStruct_t g_sbeIplControlCmdArray [] =
{
    {sbeHandleIstep,
     SBE_CMD_EXECUTE_ISTEP,
     SBE_FENCE_AT_CONTINUOUS_IPL|SBE_FENCE_AT_RUNTIME|SBE_FENCE_AT_MPIPL,
     },

    {sbeWaitForSbeIplDone,
     SBE_CMD_IS_SBE_IPL_DONE,
     SBE_FENCE_AT_ISTEP|SBE_FENCE_AT_RUNTIME|SBE_FENCE_AT_MPIPL,
     },
};



////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
uint8_t sbeGetCmdStructAttr (const uint8_t  i_cmdClass,
                                   sbeCmdStruct_t **o_ppCmd)
{
    #define SBE_FUNC " sbeGetCmdStructAttr "
    SBE_DEBUG(SBE_FUNC);
    uint8_t l_numCmds  = 0;
    *o_ppCmd = NULL;

    switch(i_cmdClass)
    {
        case SBE_CMD_CLASS_IPL_CONTROL:
            // @TODO via RTC : 128655
            //       Use C++ style typecase
            l_numCmds = sizeof(g_sbeIplControlCmdArray) /
                        sizeof(sbeCmdStruct_t);
            *o_ppCmd    = (sbeCmdStruct_t*)g_sbeIplControlCmdArray;
            break;
        case SBE_CMD_CLASS_SCOM_ACCESS:
            l_numCmds = sizeof(g_sbeScomCmdArray) /
                        sizeof(sbeCmdStruct_t);
            *o_ppCmd    = (sbeCmdStruct_t*)g_sbeScomCmdArray;
            break;

        // This will grow with each class of chipOp in future
        default:
                break;
    }
    return l_numCmds;
    #undef SBE_FUNC
}


////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
uint8_t sbeValidateCmdClass (const uint8_t i_cmdClass,
                             const uint8_t i_cmdOpcode)
{
    #define SBE_FUNC " sbeValidateCmdClass "
    uint8_t l_rc           = SBE_SEC_COMMAND_NOT_SUPPORTED;

    SBE_DEBUG(SBE_FUNC"i_cmdClass[0x%02X], "
              "i_cmdOpcode[0x%02X]", i_cmdClass, i_cmdOpcode);

    do
    {
        uint8_t l_numCmds      = 0;
        sbeCmdStruct_t *l_pCmd = NULL;

        l_numCmds = sbeGetCmdStructAttr (i_cmdClass, &l_pCmd);
        if (!l_numCmds)
        {
            // Command class not supported
            l_rc = SBE_SEC_COMMAND_CLASS_NOT_SUPPORTED;
            break;
        }

        // @TODO via RTC : 128654
        //       Analyze on merging the validation functions into one
        //       and also on using loop vs switch case performance
        for (uint8_t l_cnt = 0; l_cnt < l_numCmds; ++l_cnt, ++l_pCmd)
        {
            if (i_cmdOpcode == l_pCmd->cmd_opcode)
            {
                // Command found
                l_rc = SBE_SEC_OPERATION_SUCCESSFUL;
                break;
            }
        }
    } while (false);

    return l_rc;
    #undef SBE_FUNC
}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
bool sbeIsCmdAllowedAtState (const uint8_t i_cmdClass,
                             const uint8_t i_cmdOpcode)
{
    // @TODO via RTC : 126146
    //       SBE state management
    return 0;
}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
sbeCmdFunc_t sbeFindCmdFunc (const uint8_t i_cmdClass,
                             const uint8_t i_cmdOpcode)

{
    #define SBE_FUNC " sbeFindCmdFunc "
    uint8_t l_numCmds      = 0;
    sbeCmdStruct_t *l_pCmd = NULL;

    l_numCmds = sbeGetCmdStructAttr (i_cmdClass, &l_pCmd);

    SBE_DEBUG(SBE_FUNC"i_cmdClass[0x%02X], "
              "i_cmdOpcode[0x%02X], l_numCmds[0x%02X]",
               i_cmdClass, i_cmdOpcode, l_numCmds);

    for (uint8_t l_cnt = 0; l_cnt < l_numCmds; ++l_cnt, ++l_pCmd)
    {
        if (i_cmdOpcode == l_pCmd->cmd_opcode)
        {
            break;
        }
    }

    return l_pCmd ? (l_pCmd->cmd_func) : NULL;
    #undef SBE_FUNC
}
