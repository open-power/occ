/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/gpe/apss_constants.h $                                */
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

#ifndef _APSS_CONSTANTS_H
#define _APSS_CONSTANTS_H

#define SPIPSS_REGISTER_BASE          0x00020000
#define SPIPSS_ADC_CTRL_REG0          (SPIPSS_REGISTER_BASE + 0x00)
#define SPIPSS_ADC_CTRL_REG1          (SPIPSS_REGISTER_BASE + 0x01)
#define SPIPSS_ADC_CTRL_REG2          (SPIPSS_REGISTER_BASE + 0x02)
#define SPIPSS_ADC_STATUS_REG         (SPIPSS_REGISTER_BASE + 0x03)
#define SPIPSS_ADC_COMMAND_REG        (SPIPSS_REGISTER_BASE + 0x04)
#define SPIPSS_ADC_WDATA_REG          (SPIPSS_REGISTER_BASE + 0x10)
#define SPIPSS_ADC_RDATA_REG0         (SPIPSS_REGISTER_BASE + 0x20)
#define SPIPSS_ADC_RDATA_REG1         (SPIPSS_REGISTER_BASE + 0x21)
#define SPIPSS_ADC_RDATA_REG2         (SPIPSS_REGISTER_BASE + 0x22)
#define SPIPSS_ADC_RDATA_REG3         (SPIPSS_REGISTER_BASE + 0x23)
#define SPIPSS_P2S_CTRL_REG0          (SPIPSS_REGISTER_BASE + 0x40)
#define SPIPSS_P2S_CTRL_REG1          (SPIPSS_REGISTER_BASE + 0x41)
#define SPIPSS_P2S_CTRL_REG2          (SPIPSS_REGISTER_BASE + 0x42)
#define SPIPSS_P2S_STATUS_REG         (SPIPSS_REGISTER_BASE + 0x43)
#define SPIPSS_P2S_COMMAND_REG        (SPIPSS_REGISTER_BASE + 0x44)
#define SPIPSS_P2S_WDATA_REG          (SPIPSS_REGISTER_BASE + 0x50)
#define SPIPSS_P2S_RDATA_REG          (SPIPSS_REGISTER_BASE + 0x60)

#endif //_APSS_CONSTANTS_H
