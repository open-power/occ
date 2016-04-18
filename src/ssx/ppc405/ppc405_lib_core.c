/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/ppc405/ppc405_lib_core.c $                            */
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
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2014
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file ppc405_lib_core.c
/// \brief PPC405-specific library procedures
///
/// The routines in this file are not part of SSX per se, but are included
/// with SSX because they may be considered OS services.
///
/// These are core routines that will present in all PPC405 SSX applications.

#include "ssx.h"

/// Cause a PPC405 core reset by an action on DBCR0

void
ppc405_core_reset()
{
    or_spr(SPRN_DBCR0, DBCR0_RST_CORE);
}

/// Cause a PPC405 chip reset by an action on DBCR0

void
ppc405_chip_reset()
{
    or_spr(SPRN_DBCR0, DBCR0_RST_CHIP);
}

/// Cause a PPC405 system reset by an action on DBCR0

void
ppc405_system_reset()
{
    or_spr(SPRN_DBCR0, DBCR0_RST_SYSTEM);
}

