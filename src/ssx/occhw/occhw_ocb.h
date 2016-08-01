/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/occhw/occhw_ocb.h $                                   */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2016                        */
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
#ifndef __OCCHW_OCB_H__
#define __OCCHW_OCB_H__

/// \file occhw_ocb.h
/// \brief OCB unit header.  Local and mechanically generated macros and APIs.

#include "ssx.h"
#include "ppc32.h"

#include "occhw_common.h"
#include "ocb_register_addresses.h"
#include "ocb_firmware_registers.h"

#include "ppc405_irq.h"

#define OCB_TIMER0 0
#define OCB_TIMER1 1

#define OCB_TIMERS 2

#define OCB_TIMER_ONE_SHOT    0
#define OCB_TIMER_AUTO_RELOAD 1

#define OCB_LW_LOG_SIZE_MIN 3
#define OCB_LW_LOG_SIZE_MAX 15

#define OCB_INVALID_ARGUMENT_TIMER         0x00622001
#define OCB_INVALID_ARGUMENT_LW_INIT       0x00622002
#define OCB_INVALID_ARGUMENT_LW_DISABLE    0x00622003
#define OCB_INVALID_ARGUMENT_UNTRUST       0x00622004

#ifndef __ASSEMBLER__

int
ocb_timer_reset(int timer,
                int auto_reload,
                int timeout_ns);

#ifdef OCC
int
ocb_timer_setup(int timer,
                int auto_reload,
                int timeout_ns,
                SsxIrqHandler handler,
                void* arg,
                int priority) INIT_SECTION;
#else
int
ocb_timer_setup(int timer,
                int auto_reload,
                int timeout_ns,
                SsxIrqHandler handler,
                void* arg,
                int priority);
#endif

/// Clear OCB timer status based on the IRQ
///
/// This API can be called from OCB timer interrupt handlers, using the IRQ
/// provided to the handler.  No error checks are provided.

static inline void
ocb_timer_status_clear(SsxIrqId irq)
{
    ocb_otrn_t otrn_reg;
    otrn_reg.value = 0;
    otrn_reg.fields.timeout = 1;
    out32(OCB_OTRN(irq - OCCHW_IRQ_OCC_TIMER0), otrn_reg.value);
}

int
ocb_linear_window_initialize(int channel, uint32_t base, int log_size);

int
ocb_linear_window_disable(int channel);

int
ocb_allow_untrusted_initialize(int channel, int allow_untrusted);


#endif  /* __ASSEMBLER__ */

#endif  /* __OCCHW_OCB_H__ */
