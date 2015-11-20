/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe0/gpe_util.h $                                     */
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

#ifndef _APSS_UTIL_H
#define _APSS_UTIL_H

#include <apss_structs.h>
#include <common_types.h>
#include <ipc_structs.h>
#include <ipc_async_cmd.h>

void gpe_set_ffdc(GpeErrorStruct *o_error, uint32_t i_addr, uint32_t i_rc, uint64_t i_ffdc);

int wait_spi_completion(GpeErrorStruct *error, uint32_t reg, uint8_t timeout);

// Read decrementer register
#define MFDEC(reg_var) \
asm volatile \
    ( \
    " mfdec %[dec_var] \n" \
    : [dec_var]"=r"(reg_var) \
    );

void busy_wait(uint32_t t_microseconds);

#endif //_APSS_UTIL_H
