/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/special_wakeup.h $                                    */
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
#ifndef __SPECIAL_WAKEUP_H__
#define __SPECIAL_WAKEUP_H__

/// \file special_wakeup.h
/// \brief Container for special wakeup related procedures

#ifndef __ASSEMBLER__

#include <stdint.h>
#include "pgp_config.h"
#include "ssx.h"

#define SPWU_INVALID_ARGUMENT 0x00779801

extern uint32_t G_special_wakeup_count[PGP_NCORES];

int
occ_special_wakeup(int set,
                   ChipConfigCores cores,
                   int timeout_ms,
                   ChipConfigCores *o_timeouts);

#endif  /* __ASEMBLER__ */
#endif  /* __SPECIAL_WAKEUP_H__ */
