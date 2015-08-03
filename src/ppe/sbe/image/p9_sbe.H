/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/sbe/image/p9_sbe.H $                                  */
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
#ifndef __P9_SBE_H
#define __P9_SBE_H

/// \file p9_sbe.H
/// \brief A header to be included into all SBE assembler files and C++
/// hardware procedures that interact with SBE images.
///
/// This header #include-s all generally useful headers required for SBE code
/// development as well as defining other needed facilities.  Note that any
/// sub-includes must be done in an assembler-safe way.

#include "sbe_common.H"
//#include "p9_scom_addresses.H"
//#include "p9_istep_num.H"

// The #include of fapiHwpReturnCodes.H must be made here (instead of in
// sbe_common.H) to guarantee that the P9-local copy is used.
//#include "fapiHwpReturnCodes.H"

#ifdef __ASSEMBLER__

        // p9_sbe.H 'recursively' inserts itself into any hook code extracted
        // from an assembly context that includes p9_sbe.H.

	##`#include "p9_sbe.H"

#endif  // __ASSEMBLER__

#endif  // __P9_SBE_H
