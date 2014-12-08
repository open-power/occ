/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/cmdh/cmdh_service_codes.h $                           */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* COPYRIGHT International Business Machines Corp. 2011,2014              */
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

#ifndef CMDH_SERVICE_CODES_H
#define CMDH_SERVICE_CODES_H

#include <comp_ids.h>

enum occCmdhModuleId
{
    DATA_STORE_GENERIC_DATA             =  CMDH_COMP_ID | 0x00,
    DATA_STORE_FREQ_DATA                =  CMDH_COMP_ID | 0x01,
    DATA_STORE_PCAP_DATA                =  CMDH_COMP_ID | 0x02,
    CMDH_FSI2HOST_MBOX_UNAVAIL          =  CMDH_COMP_ID | 0x03,
    CMDH_GENERIC_CMD_FAILURE            =  CMDH_COMP_ID | 0x04,
    DATA_STORE_SYS_DATA                 =  CMDH_COMP_ID | 0x05,
    DATA_STORE_APSS_DATA                =  CMDH_COMP_ID | 0x06,
    CMDH_FSP_FSI2HOST_MBOX_WAIT4FREE    =  CMDH_COMP_ID | 0x07,
    DATA_GET_THRM_THRESHOLDS            =  CMDH_COMP_ID | 0x08,
    DATA_STORE_IPS_DATA                 =  CMDH_COMP_ID | 0x09,
    DATA_GET_IPS_DATA                   =  CMDH_COMP_ID | 0x0A,
    DATA_GET_RESET_PREP_ERRL            =  CMDH_COMP_ID | 0x0B,
};
#endif
