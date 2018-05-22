/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/firdata/fir_data_collect.c $                      */
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


#include <fir_data_collect.h>
#include "tpc_firmware_registers.h"
#include "tpc_register_addresses.h"
#include <firData.h>
#include <gpe_export.h>

extern gpe_shared_data_t * G_gpe_shared_data;
extern void busy_wait(uint32_t t_microseconds);

/*
 * Function Specification
 *
 * Name: fir_data_collect
 *
 * Description: Collects FIR data on checkstop.
 *
 * End Function Specification
 */
void fir_data_collect(void)
{

    int32_t l_rc = 0;

    // Homer data section and size
    uint8_t *l_hBuf = (uint8_t*) G_gpe_shared_data->fir_params_buffer_ptr;
    uint32_t l_hBufSize = FIR_PARMS_SECTION_SIZE;

    // PNOR working buffer in SRAM and size
    uint8_t *l_pBuf = (uint8_t*) G_gpe_shared_data->fir_heap_buffer_ptr;
    uint32_t l_pBufSize = FIR_HEAP_SECTION_SIZE;

    busy_wait(2000000);  // wait two seconds

    l_rc = FirData_captureCsFirData(l_hBuf,
                                    l_hBufSize,
                                    l_pBuf,
                                    l_pBufSize);

    // Trace the rc only, error logs cannot be collected in this state
    TRAC_IMP("Checkstop FIR data capture completed with rc=%d", l_rc);
}
