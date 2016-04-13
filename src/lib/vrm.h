/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/vrm.h $                                               */
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
#ifndef __VRM_H__
#define __VRM_H__

/// \file vrm.h
/// \brief PgP SPIVRM procedures

#include "ssx.h"

#ifndef __ASSEMBLER__

int
o2s_initialize();

int
spivid_initialize();


int
vrm_voltage_write(int vrm_select, 
                  uint8_t vdd_vid, 
                  int8_t vcs_offset, 
                  int phases);

int
vrm_read_state(int vrm_select, 
                  int  *mnp1,
                  int  *mn,
                  int  *vfan,
                  int  *vovertmp);

int
vrm_voltage_read(int vrm_select,
                 uint8_t vrail,
                 uint8_t *o_vid);

#endif  /* __ASEMBLER__ */

// Error/panic codes

#define O2S_BUSY_VRM_VOLTAGE_READ   0x00627001
#define O2S_BUSY_VRM_VOLTAGE_WRITE  0x00627002
#define O2S_BUSY_VRM_READ_STATE     0x00627003
#define O2S_READ_NOT_READY          0x00627004
#define O2S_WRITE_NOT_VALID         0x00627005
#define O2S_WRITE_ECC_ERROR         0x00627006
#define VRM_INVALID_ARGUMENT_VREAD  0x00627007
#define VRM_INVALID_ARGUMENT_VWRITE 0x00627008 
#define VRM_INVALID_ARGUMENT_SREAD  0x00627009 
#define VRM_INVALID_ARGUMENT_INIT   0x0062700a

#endif  /* __VRM_H__ */
