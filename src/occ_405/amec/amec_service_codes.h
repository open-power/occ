/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/amec/amec_service_codes.h $                       */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2017                        */
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

#ifndef _AMEC_SERVICE_CODES_H_
#define _AMEC_SERVICE_CODES_H_

/*----------------------------------------------------------------------------*/
/* Includes                                                                   */
/*----------------------------------------------------------------------------*/
#include <comp_ids.h>

/*----------------------------------------------------------------------------*/
/* Constants                                                                  */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Globals                                                                    */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Defines                                                                    */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Typedef / Enum                                                             */
/*----------------------------------------------------------------------------*/
enum occAmecModuleId
{
    AMEC_INITIALIZE_FW_SENSORS        =    AMEC_COMP_ID | 0x00,
    AMEC_UPDATE_FW_SENSORS            =    AMEC_COMP_ID | 0x01,
    AMEC_VECTORIZE_FW_SENSORS         =    AMEC_COMP_ID | 0x02,
    AMEC_AMESTER_INTERFACE            =    AMEC_COMP_ID | 0x03,
    AMEC_PCAP_CONN_OC_CONTROLLER      =    AMEC_COMP_ID | 0x04,
    AMEC_MST_CHECK_PCAPS_MATCH        =    AMEC_COMP_ID | 0x05,
    AMEC_MST_CHECK_UNDER_PCAP         =    AMEC_COMP_ID | 0x06,
    AMEC_SLAVE_CHECK_PERFORMANCE      =    AMEC_COMP_ID | 0x07,
    AMEC_HEALTH_CHECK_PROC_TEMP       =    AMEC_COMP_ID | 0x08,
    AMEC_HEALTH_CHECK_DIMM_TEMP       =    AMEC_COMP_ID | 0x09,
    AMEC_HEALTH_CHECK_CENT_TEMP       =    AMEC_COMP_ID | 0x10,
    AMEC_HEALTH_CHECK_DIMM_TIMEOUT    =    AMEC_COMP_ID | 0x11,
    AMEC_HEALTH_CHECK_CENT_TIMEOUT    =    AMEC_COMP_ID | 0x12,
    AMEC_HEALTH_CHECK_VRFAN_TIMEOUT   =    AMEC_COMP_ID | 0x13,
    AMEC_HEALTH_CHECK_PROC_TIMEOUT    =    AMEC_COMP_ID | 0x14,
    AMEC_CALC_DTS_SENSORS             =    AMEC_COMP_ID | 0x16,
    AMEC_SET_FREQ_RANGE               =    AMEC_COMP_ID | 0x17,
    AMEC_UPDATE_APSS_GPIO             =    AMEC_COMP_ID | 0x18,
    AMEC_GPU_PCAP_MID                 =    AMEC_COMP_ID | 0x19,
    AMEC_HEALTH_CHECK_VRM_VDD_TEMP    =    AMEC_COMP_ID | 0x1A,
    AMEC_HEALTH_CHECK_VRM_VDD_TIMEOUT =    AMEC_COMP_ID | 0x1B,
};

/*----------------------------------------------------------------------------*/
/* Function Prototypes                                                        */
/*----------------------------------------------------------------------------*/

#endif /* #ifndef _AMEC_SERVICE_CODES_H_ */
