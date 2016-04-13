/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/heartbeat.h $                                         */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2014,2016                        */
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
#ifndef __HEARTBEAT_H__
#define __HEARTBEAT_H__
/// \file heartbeat.h
/// \brief PgP PMC/PCBS heartbeat configuration procedures

#ifndef __ASSEMBLER__

#include <stdint.h>

#define HB_INVALID_ARGUMENT_PMC  0x00482801
#define HB_INVALID_ARGUMENT_PCBS 0x00482802
#define HB_UNDERFLOW_DIVIDER_PMC 0x00482803
#define HB_OVERFLOW_DIVIDER_PMC  0x00482804
#define HB_OVERFLOW_PULSES_PMC   0x00482805
#define HB_OVERFLOW_PULSES_PCBS  0x00482806
#define HB_UNDERFLOW_PULSES_PCBS 0x00482807

int
pmc_hb_config(unsigned int enable,
              unsigned int req_time_us,
              unsigned int force,
              unsigned int *o_time_us);

int
pcbs_hb_config(unsigned int enable,
               ChipConfigCores cores,
               uint32_t hb_reg,
               unsigned int req_time_us,
               unsigned int force,
               unsigned int *o_time_us);



#endif  /* __ASEMBLER__ */


#endif  /* __HEARTBEAT_H__ */
