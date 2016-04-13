/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/pgp/pgp_pore.h $                                      */
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
#ifndef __PGP_PORE_H__
#define __PGP_PORE_H__

/// \file pgp_pore.h
/// \brief PORE unit header.  Local and mechanically generated macros.

#include "pore_register_addresses.h"
#include "pore_firmware_registers.h"

/// The parameter for GPE-protocol triggering is the low-order word of the
/// EXE_TRIGGER register
#define PORE_EXE_PARAMETER_OFFSET (PORE_EXE_TRIGGER_OFFSET + 4)

/// The PORE OCI address space descriptor 
#define PORE_ADDRESS_SPACE_OCI 0x8000

/// The PORE BRAI opcode
#define PORE_BRAI 0xa2000000

/// The PORE BRAD D0 opcode
#define PORE_BRAD_D0 0x38500000


#ifndef __ASSEMBLER__

/// The putative type of PORE program entry points - to make GCC happy
typedef void *(PoreEntryPoint)(void);

#endif  /* __ASSEMBLER__ */

#endif  /* __PGP_PORE_H__ */
