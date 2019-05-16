/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/pk/ppe42/ppe42_irq_init.c $                           */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2019                        */
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

/// \file ppe42_irq_init.c
/// \brief PPE42 IRQ initialization routines
///
/// The entry points in this file are routines that are typically used during
/// initialization, and their code space could be deallocated and recovered if
/// no longer needed by the application after initialization.

#include "pk.h"

/// Set up a PPE42 Fixed Interval Timer (FIT) handler
///
/// See the PK specification for full details on setting up a FIT handler.
///
/// Return values other then PK_OK (0) are errors; see \ref pk_errors
///
/// \retval 0 Successful completion
///

int
ppe42_fit_setup(PkIrqHandler handler, void* arg)
{
    PkMachineContext ctx;
    Ppe42TCR tcr;

    pk_critical_section_enter(&ctx);

    tcr.value = mfspr(SPRN_TCR);

    if (handler)
    {

        tcr.fields.fp  = 0;
        tcr.fields.fie = 1;

        __ppe42_fit_routine = handler;
        __ppe42_fit_arg = arg;

    }
    else
    {

        tcr.fields.fie = 0;
    }

    mtspr(SPRN_TCR, tcr.value);
    sync();

    pk_critical_section_exit(&ctx);

    return PK_OK;
}



