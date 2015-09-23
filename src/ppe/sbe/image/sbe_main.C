/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/sbe/image/sbe_main.C $                                */
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
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2014
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file sample_main.c
/// \brief Sample program that creates and starts a thread
///
/// This file demonstrates how to create a thread and run it.  It also provides
/// an example of how to add traces to the code.
#include <fapi2.H>
//#include <p9_sbe_perv.H>

#include <vector>




extern "C"
{

#include "pk.h"
#include "pk_trace.h"
//#include "pk_trace_wrap.h"
//#include "common_scom_addresses.H"
//#include "p9_sbe_perv.H"
#include "p9_hcd_cache.H"
#include "p9_hcd_core.H"
#include "proc_sbe_fixed.H"
#include "trac_interface.h"

}

namespace fapi2attr
{

extern ProcChipAttributes_t* G_proc_chip_attributes asm("G_proc_chip_attributes")   __attribute__ ((section (".fixed")));
extern PervAttributes_t*     G_perv_attributes      asm("G_perv_attributes")        __attribute__ ((section (".fixed")));
extern CoreAttributes_t*     G_core_attributes      asm("G_core_attributes")        __attribute__ ((section (".fixed")));
extern EQAttributes_t*       G_eq_attributes        asm("G_eq_attributes")          __attribute__ ((section (".fixed")));
extern EXAttributes_t*       G_ex_attributes        asm("G_ex_attributes")          __attribute__ ((section (".fixed")));

}

extern "C" {


#define KERNEL_STACK_SIZE   256
#define MAIN_THREAD_STACK_SIZE  256

// Necessary Kernel Structures
uint8_t     G_kernel_stack[KERNEL_STACK_SIZE];
uint8_t     G_main_thread_stack[MAIN_THREAD_STACK_SIZE];
PkThread    G_main_thread;


fapi2::ReturnCode
hwp_chip(const fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP> & i_target);

fapi2::ReturnCode
hwp_chip2(const fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP> & i_target);


fapi2::ReturnCode
hwp_core(const fapi2::Target<fapi2::TARGET_TYPE_CORE> & i_target);

fapi2::ReturnCode
hwp_eq(const fapi2::Target<fapi2::TARGET_TYPE_EQ> & i_target);

fapi2::ReturnCode
hwp_perv(const fapi2::Target<fapi2::TARGET_TYPE_PERV> & i_target);


//---------------------------------------------------------------------------


void main_thread(void* arg)
{

        // This is workaround. Currently we do not have code to initialise
        // global objects. So initializing global objects against using local
        // initialized object
        FAPI_DBG("Workaround temporary allocation of Global Vector");
        std::vector<fapi2::plat_target_handle_t> targets1;
        G_vec_targets = std::move(targets1);

        // Intialize the targets
        fapi2::plat_TargetsInit();

        // Get a specific target
        fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP>chip_target_new =
                fapi2::plat_getChipTarget();
                
        FAPI_DBG("chip_target_new = 0x%08X", (uint32_t)(chip_target_new.get()>>32));

        FAPI_TRY(hwp_chip(chip_target_new));
        
        FAPI_TRY(hwp_chip2(chip_target_new));

        ///
#ifndef __noRC__
        // PIB Errors are masked for platforms like SBE where
        // explict error code checking is to occur
        fapi2::setPIBErrorMask(0b11111111);
#else
        // PIB Errors are unmaskd for platforms that desire to take machine
        // check interrupts
        fapi2::setPIBErrorMask(0b00000000);
#endif

//       FAPI_TRY(hwp_chip(chip_target_new));

//       FAPI_TRY(p9_sbe_attr_setup(chip_target));
//       FAPI_TRY(p9_sbe_check_master(chip_target));
//       FAPI_TRY(p9_sbe_setup_evid(chip_target));


fapi_try_exit:
    return;

}

// A Chip try
fapi2::ReturnCode
hwp_chip(const fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP> & i_target)
{

    FAPI_DBG("i_target = 0x%08X", (uint32_t)(i_target.get()>>32));

    auto l_perv_functional_vector =
		        i_target.getChildren<fapi2::TARGET_TYPE_PERV>
                (fapi2::TARGET_STATE_PRESENT);

    // Get the TPChiplet target
    uint32_t i = 0;
    for (auto it: l_perv_functional_vector)
	{

	    FAPI_DBG("Perv Functional Target %u value=%08X chiplet %02X",
                    i,
                    (uint32_t)(it.get()>>32),
                    (uint32_t)(it.getChipletNumber()));

        ++i;
	}

    auto l_core_functional_vector =
		        i_target.getChildren<fapi2::TARGET_TYPE_CORE>
                (fapi2::TARGET_STATE_PRESENT);

    // Get the Core Chiplet targets
    uint32_t j = 0;
    for (auto it: l_core_functional_vector)
	{

	    FAPI_DBG("Core Functional Target %u value=%08X chiplet %02X",
                    j,
                    (uint32_t)(it.get()>>32),
                    (uint32_t)(it.getChipletNumber()));

        ++j;
	}

    
     fapi2::buffer<uint64_t> data = 0;
     const uint32_t address = 0x0006d010;

     FAPI_INF("hwp_chip %u", address);

     uint64_t databuffer;
     getscom_abs(address, &databuffer);

     databuffer = 0xDEAD000000000000ull;

     putscom_abs(address, databuffer);

     data = 0xBADC0DE800000000ull;
     FAPI_TRY(fapi2::putScom(i_target, address, data));


     FAPI_TRY(fapi2::getScom(i_target, address, data));
     FAPI_DBG("First getSCOM: data = %016llX", revle64(data));

     data.setBit<0, 16>();
     FAPI_TRY(fapi2::putScom(i_target, 0x0006d010, data));

     return fapi2::FAPI2_RC_SUCCESS;

fapi_try_exit:
     return fapi2::FAPI2_RC_PLAT_ERR_SEE_DATA;
}

// A Chip try
fapi2::ReturnCode
hwp_chip2(const fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP> & i_target)
{

    FAPI_DBG("i_target = 0x%08X", (uint32_t)(i_target.get()>>32));


    auto l_eq_functional_vector =
		        i_target.getChildren<fapi2::TARGET_TYPE_EQ>
                (fapi2::TARGET_STATE_PRESENT);

    // Get the EQ Chiplet target
    uint32_t k = 0;
    for (auto it: l_eq_functional_vector)
	{

	    FAPI_DBG("EQ Functional Target %u value=%08X chiplet %02X",
                    k,
                    (uint32_t)(it.get()>>32),
                    (uint32_t)(it.getChipletNumber()));

        ++k;
	}
    
    
    auto l_mcs_functional_vector =
		        i_target.getChildren<fapi2::TARGET_TYPE_MCS>
                (fapi2::TARGET_STATE_PRESENT);
                
    // Get the MCS Chiplet target
    uint32_t m = 0;
    for (auto it: l_mcs_functional_vector)
	{

	    FAPI_DBG("MCS Functional Target %u value=%08X chiplet %02X",
                    m,
                    (uint32_t)(it.get()>>32),
                    (uint32_t)(it.getChipletNumber()));

        ++m;
	}
    

     return fapi2::FAPI2_RC_SUCCESS;

fapi_try_exit:
     return fapi2::FAPI2_RC_PLAT_ERR_SEE_DATA;
}

// A Core try
fapi2::ReturnCode
hwp_core(const fapi2::Target<fapi2::TARGET_TYPE_CORE> & i_target)
{

    // Temporary target that pulls out only the chiplet overly.  This keeps
    // from having to compute this for each SCOM operation.
//    fapi2::Target<fapi2::TARGET_TYPE_ADDRESS> iv_target (i_target.getAddressOverlay());

    fapi2::buffer<uint64_t> data = 0;
    fapi2::buffer<uint64_t> mask;

    uint32_t address = 0x200F5678;
    FAPI_TRY(fapi2::getScom(i_target, address, data));

    FAPI_TRY(fapi2::putScom(i_target, 0x20006789, data));

    data = 0xBADC0DEBADC0DEBAull;
    FAPI_TRY(fapi2::putScom(i_target, 0x0000AAAA, data));

    FAPI_TRY(fapi2::getScom(i_target, address, data));

    FAPI_TRY(fapi2::modifyScom(i_target, address, data, fapi2::CHIP_OP_MODIFY_MODE_OR));


    mask = BITS(4,4);
    FAPI_TRY(fapi2::putScomUnderMask(i_target, address, data, mask));

    return fapi2::FAPI2_RC_SUCCESS;

fapi_try_exit:

    return fapi2::FAPI2_RC_PLAT_ERR_SEE_DATA;
}

// An EQ try
fapi2::ReturnCode
hwp_eq(const fapi2::Target<fapi2::TARGET_TYPE_EQ> & i_target)
{
    fapi2::buffer<uint64_t> data = 0;

    uint64_t address = 0x1000F2222;
    FAPI_TRY(fapi2::getScom(i_target, address, data));


    FAPI_TRY(fapi2::putScom(i_target, 0x10006789, data));

    data = 0xDEADBEEFDEADBEEFull;
    FAPI_TRY(fapi2::putScom(i_target, 0x1000ABCD, data));

    return fapi2::FAPI2_RC_SUCCESS;

fapi_try_exit:

    return fapi2::FAPI2_RC_PLAT_ERR_SEE_DATA;
}

// A Perv try
fapi2::ReturnCode
hwp_perv(const fapi2::Target<fapi2::TARGET_TYPE_PERV> & i_target)
{
    fapi2::buffer<uint64_t> data = 0;;

    uint64_t address = 0x00005678;

    for (uint32_t i = 0; i < 5; i++)
    {
        FAPI_TRY(fapi2::getScom(i_target, address+i, data));

        data.setBit<4>();

        FAPI_TRY(fapi2::putScom(i_target, address+i, data));

        data = 0xDEADBEEFDEADBEEFull;
        FAPI_TRY(fapi2::putScom(i_target, address+(2*i), data));
    }

    return fapi2::FAPI2_RC_SUCCESS;

fapi_try_exit:

    return fapi2::FAPI2_RC_PLAT_ERR_SEE_DATA;
}



// The main function is called by the boot code (after initializing some
// registers)
int main(int argc, char **argv)
{
    // initializes kernel data (stack, threads, timebase, timers, etc.)
    pk_initialize((PkAddress)G_kernel_stack,
                  KERNEL_STACK_SIZE,
                  0,
                  500000000);

    PK_TRACE("Kernel init completed");

    //Initialize the thread control block for G_main_thread
    pk_thread_create(&G_main_thread,
                      (PkThreadRoutine)main_thread,
                      (void*)NULL,
                      (PkAddress)G_main_thread_stack,
                      (size_t)MAIN_THREAD_STACK_SIZE,
                      (PkThreadPriority)1);

    PK_TRACE_BIN("G_main_thread", &G_main_thread, sizeof(G_main_thread));

    //Make G_main_thread runnable
    pk_thread_resume(&G_main_thread);

    PK_TRACE("Starting thread(s)");

    // Start running the highest priority thread.
    // This function never returns
    pk_start_threads();

    return 0;
}

}  // extern C
