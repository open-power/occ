/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/dcom/dcom_service_codes.h $                           */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2014                        */
/* [+] Google Inc.                                                        */
/* [+] International Business Machines Corp.                              */
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

#ifndef _DCOM_SERVICE_CODES_H_
#define _DCOM_SERVICE_CODES_H_

#include <comp_ids.h>

enum dcomModuleId
{
    DCOM_MID_INIT_ROLES             =  DCOM_COMP_ID | 0x00,
    DCOM_MID_TASK_RX_SLV_INBOX      =  DCOM_COMP_ID | 0x01,
    DCOM_MID_TASK_TX_SLV_INBOX      =  DCOM_COMP_ID | 0x02,
    DCOM_MID_INIT_PBAX_QUEUES       =  DCOM_COMP_ID | 0x03,
    DCOM_MID_TASK_RX_SLV_OUTBOX     =  DCOM_COMP_ID | 0x04,
    DCOM_MID_TASK_TX_SLV_OUTBOX     =  DCOM_COMP_ID | 0x05,
    DCOM_MID_SLV_OUTBOX_TX_DOORBELL =  DCOM_COMP_ID | 0x06,
    DCOM_MID_TASK_WAIT_FOR_MASTER   =  DCOM_COMP_ID | 0x07,
    DCOM_MID_ERROR_CHECK            =  DCOM_COMP_ID | 0x08,
    DCOM_MID_WAIT_FOR_MASTER        =  DCOM_COMP_ID | 0x09,
    DCOM_MID_PBAX_ERROR_HANDLER     =  DCOM_COMP_ID | 0x0A,
    DCOM_MID_BUILD_SLV_INBOX        =  DCOM_COMP_ID | 0x0B,
};

#endif /* #ifndef _DCOM_SERVICE_CODES_H_ */
