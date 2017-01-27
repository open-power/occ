/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/pgpe/pgpe_interface.h $                           */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2016                        */
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

#ifndef _PGPE_INTERFACE_H_
#define _PGPE_INTERFACE_H_

#include "errl.h"
#include "state.h"
#include "pstate_pgpe_occ_api.h"

void init_pgpe_ipcs(void);

errlHndl_t pgpe_init_clips(void);
errlHndl_t pgpe_init_pmcr(void);
errlHndl_t pgpe_init_start_suspend(void);
errlHndl_t pgpe_init_wof_control(void);
errlHndl_t pgpe_init_wof_vfrt(void);

int pgpe_widen_clip_ranges(OCC_STATE state);
int pgpe_widen_clip_blocking(OCC_STATE state);
int pgpe_clip_update(void);

int pgpe_pmcr_set(void);

int pgpe_start_suspend(uint8_t action, PMCR_OWNER owner);
void pgpe_start_suspend_callback(void);

#endif /* #ifndef _PGPE_INTERFACE_H_ */
