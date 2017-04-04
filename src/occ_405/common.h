/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/common.h $                                            */
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
#ifndef _common_h
#define _common_h

#include <rtls.h>

// Reasons why the OCCMISC external interrupt was triggered
typedef enum
{
    // defined as individual bits (since multiple interrupts could be queued)
    INTR_REASON_HTMGT_SERVICE_REQUIRED  = 0x04,
    INTR_REASON_I2C_OWNERSHIP_CHANGE    = 0x02,
    INTR_REASON_OPAL_SHARED_MEM_CHANGE  = 0x01
} ext_intr_reason_t;


// Miscellaneous checks to be done by 405
// - check for pending host notifications
// - check for checkstop
void task_misc_405_checks(task_t *i_self);

// Trigger interrupt to the host with the specified reason (OCCMISC/core_ext_intr)
// Returns true if notification was sent, false if interrupt already outstanding
bool notify_host(const ext_intr_reason_t i_reason);

#endif // _common_h
