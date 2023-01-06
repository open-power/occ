/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe1/gpe1_main.c $                                    */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2023                        */
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

/// \file gpe1_main.c
/// \brief Sample program that creates and starts a thread
///
/// This file demonstrates how to create a thread and run it.  It also provides
/// an example of how to add traces to the code.

#include "pk.h"
#include "pk_trace.h"
#include "ipc_api.h"
#include "gpe_export.h"
#include "gpe_membuf.h"

#if defined(__OCMB_UNIT_TEST__)
#include "membuf_structs.h"
#include "ocmb_mem_data.h"
#endif

#define KERNEL_STACK_SIZE  1024

uint64_t        G_kernel_stack[KERNEL_STACK_SIZE/8];

//Point to the GPE shared structure
#define GPE_SHARED_DATA_ADDR 0xFFFF6000
#define GPE_SHARED_DATA_SIZE 256

gpe_shared_data_t * G_gpe_shared_data = (gpe_shared_data_t*) GPE_SHARED_DATA_ADDR;

extern PkTraceBuffer* g_pk_trace_buf_ptr;

#if defined(__OCMB_UNIT_TEST__)
MemBufGetMemDataParms_t G_dataParms;
MemBufConfiguration_t G_membufConfiguration;
OcmbMemData G_escache;

int gpe_ocmb_configuration_create(MemBufConfiguration_t* o_config, uint32_t i_max_dts);
int get_ocmb_sensorcache(MemBufConfiguration_t* i_config,
                         MemBufGetMemDataParms_t* i_parms);
#endif

// The main function is called by the boot code (after initializing some
// registers)
int main(int argc, char **argv)
{
    int rc;
    if(G_gpe_shared_data == NULL)
    {
        // This should never happen  If the trace is never seen after a while
        // then remove this "if" block.
        G_gpe_shared_data = (gpe_shared_data_t*) GPE_SHARED_DATA_ADDR;
        PK_TRACE("GPE1 main found G_gpe_shared_data ptr NULL!");
    }
    uint32_t l_timebase = G_gpe_shared_data->occ_freq_div;

    // Don't initialize with a 0
    if (!l_timebase)
    {
        l_timebase = PPE_TIMEBASE_HZ;
    }

    // Mark the location of the trace buffer in shared data
    G_gpe_shared_data->gpe1_tb_ptr = (uint32_t) g_pk_trace_buf_ptr;
    G_gpe_shared_data->gpe1_tb_sz = sizeof(PkTraceBuffer);

    // initializes kernel data (stack, threads, timebase, timers, etc.)
    pk_initialize((PkAddress)G_kernel_stack,
                  KERNEL_STACK_SIZE,
                  PK_TIMEBASE_CONTINUES,
                  l_timebase);

    PK_TRACE("Kernel init completed, timebase is %d Hz", l_timebase);

    // Disable IPC's and register the IPC interrupt handler
    rc = ipc_init();
    if(rc)
    {
        PK_TRACE("ipc_init failed with rc = 0x%08x", rc);
        pk_halt();
    }

    // enable IPC's
    rc = ipc_enable();
    if(rc)
    {
        PK_TRACE("ipc_enable failed with rc = 0x%08x", rc);
        pk_halt();
    }
#if defined(__OCMB_UNIT_TEST__)
    G_dataParms.error.rc = 0;
    G_dataParms.collect = 0;
    G_dataParms.update = -1;
    G_dataParms.data = (uint64_t*)(&G_escache);
    G_membuf_config = &G_membufConfiguration;

    G_membufConfiguration.config = 0xf000ff00;

    rc = gpe_ocmb_configuration_create(&G_membufConfiguration, 2);

    PK_TRACE("gpe_ocmb_configuration_create rc = %d",rc);

    rc = get_ocmb_sensorcache(&G_membufConfiguration, &G_dataParms);

    PK_TRACE("get_ocmb_sensorcache rc = %d",rc);
#endif
    return 0;
}


/*
 * Function Specification
 *
 * Name: gpe_set_ffdc
 *
 * Description: Fills up the error struct with the given data.
 *
 * End Function Specification
 */
void gpe_set_ffdc(GpeErrorStruct *o_error, uint32_t i_addr, uint32_t i_rc, uint64_t i_ffdc)
{

    o_error->addr = i_addr;
    //Return codes defined in gpe_err.h
    o_error->rc = i_rc;
    o_error->ffdc = i_ffdc;
}
