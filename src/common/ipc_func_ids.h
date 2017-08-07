/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/common/ipc_func_ids.h $                                   */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2017                        */
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
#ifndef __IPC_FUNC_IDS_H__
#define __IPC_FUNC_IDS_H__

/// \file ipc_func_ids.h
/// \brief This file contains the definition of function ID used for
/// interprocessor communications (IPC) in the OCC.
///

// This is a global file that defines all of the valid IPC function ID's.

// comment this out if you do not want to use the ping utility provided by the IPC library
#define IPC_ENABLE_PING

// NOTE: MT Multi Target, ST = Single Target
IPC_FUNCIDS_TABLE_START

    //This is where common function ID's that any instance can support should be
    //defined.
    IPC_FUNCIDS_MT_START
#ifdef IPC_ENABLE_PING
        IPC_FUNC_ID(IPC_MT_PING) //anyone can ping anyone else
#endif
    IPC_FUNCIDS_MT_END

    //Functions that are only supported by GPE0 should be defined here
    //These function ID's can only be sent to GPE0
    IPC_FUNCIDS_ST_START(OCCHW_INST_ID_GPE0)
        IPC_FUNC_ID(IPC_ST_TEST_FUNC0)
        IPC_FUNC_ID(IPC_ST_APSS_INIT_GPIO_FUNCID)
        IPC_FUNC_ID(IPC_ST_APSS_INIT_MODE_FUNCID)
        IPC_FUNC_ID(IPC_ST_APSS_START_PWR_MEAS_READ_FUNCID)
        IPC_FUNC_ID(IPC_ST_APSS_CONTINUE_PWR_MEAS_READ_FUNCID)
        IPC_FUNC_ID(IPC_ST_APSS_COMPLETE_PWR_MEAS_READ_FUNCID)
        IPC_FUNC_ID(IPC_ST_GET_CORE_DATA_FUNCID)
        IPC_FUNC_ID(IPC_ST_SCOM_OPERATION)
        IPC_FUNC_ID(IPC_ST_GPE0_NOP)
        IPC_FUNC_ID(IPC_ST_GET_NEST_DTS_FUNCID)
        IPC_FUNC_ID(IPC_ST_FIR_COLLECTION)
        IPC_FUNC_ID(IPC_ST_GET_TOD_FUNCID)
    IPC_FUNCIDS_ST_END(OCCHW_INST_ID_GPE0)

    //Functions that are only supported by GPE1 should be defined here
    //These function ID's can only be sent to GPE1
    IPC_FUNCIDS_ST_START(OCCHW_INST_ID_GPE1)
        IPC_FUNC_ID(IPC_ST_DIMM_SM_FUNCID)
        IPC_FUNC_ID(IPC_ST_DIMM_CONTROL_FUNCID)
        IPC_FUNC_ID(IPC_ST_GPE1_NOP)
        IPC_FUNC_ID(IPC_ST_RESET_MEM_DEADMAN)
        IPC_FUNC_ID(IPC_ST_24_X_7_FUNCID)
        IPC_FUNC_ID(IPC_ST_MEM_POWER_CONTROL_FUNCID)
        IPC_FUNC_ID(IPC_ST_GPU_SM_FUNCID)
    IPC_FUNCIDS_ST_END(OCCHW_INST_ID_GPE1)

    //Functions that are only supported by GPE2 should be defined here
    //These function ID's can only be sent to GPE2 (PGPE)
    IPC_FUNCIDS_ST_START(OCCHW_INST_ID_GPE2)
        IPC_FUNC_ID(IPC_MSGID_405_INVALID)
        IPC_FUNC_ID(IPC_MSGID_405_START_SUSPEND)
        IPC_FUNC_ID(IPC_MSGID_405_CLIPS)
        IPC_FUNC_ID(IPC_MSGID_405_SET_PMCR)
        IPC_FUNC_ID(IPC_MSGID_405_WOF_CONTROL)
        IPC_FUNC_ID(IPC_MSGID_405_WOF_VFRT)
    IPC_FUNCIDS_ST_END(OCCHW_INST_ID_GPE2)

    //Functions that are only supported by GPE3 should be defined here
    //These function ID's can only be sent to GPE3
    IPC_FUNCIDS_ST_START(OCCHW_INST_ID_GPE3)
    IPC_FUNCIDS_ST_END(OCCHW_INST_ID_GPE3)

    //Functions that are only supported by PPC should be defined here
    //These function ID's can only be sent to the PPC
    IPC_FUNCIDS_ST_START(OCCHW_INST_ID_PPC)
    IPC_FUNCIDS_ST_END(OCCHW_INST_ID_PPC)

IPC_FUNCIDS_TABLE_END

#endif /*__IPC_FUNC_IDS_H__*/
