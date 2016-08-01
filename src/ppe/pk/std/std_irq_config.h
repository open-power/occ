/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/pk/std/std_irq_config.h $                             */
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
#ifndef __STD_IRQ_CONFIG_H__
#define __STD_IRQ_CONFIG_H__

/// \file std_irq_config.h
/// \brief Contains data and macros pertaining to external interrupt
/// configuration for a standard PPE.
///

#include "pk_app_cfg.h"

/// This constant is used to define the size of the table of interrupt handler
/// structures as well as a limit for error checking. 
#define EXTERNAL_IRQS 64

// Standard interrupt type values (level or edge)
#define STD_IRQ_TYPE_LEVEL            0
#define STD_IRQ_TYPE_EDGE             1

// Standard interrupt polarity values (high or low, rising falling)
#define STD_IRQ_POLARITY_LO           0
#define STD_IRQ_POLARITY_FALLING      0
#define STD_IRQ_POLARITY_HI           1
#define STD_IRQ_POLARITY_RISING       1

// Standard interrupt mask values (masked or enabled)
#define STD_IRQ_MASKED                0
#define STD_IRQ_ENABLED               1

// Fail to compile if the application does not define this
#ifndef APPCFG_EXT_IRQS_CONFIG
#error "APPCFG_EXT_IRQS_CONFIG must be defined in pk_app_cfg.h"
#endif

// Fail to compile if the application does not define this
#ifndef APPCFG_IRQ_INVALID_MASK
#error "APPCFG_IRQ_INVALID_MASK must be defined in pk_app_cfg.h"
#endif

#ifndef __ASSEMBLER__

/// This expression recognizes only those IRQ numbers that have named
/// (non-reserved) interrupts in the standard PPE interrupt controller.
#define STD_IRQ_VALID(irq) \
    ({unsigned __irq = (unsigned)(irq);         \
        ((__irq < EXTERNAL_IRQS) &&             \
         ((STD_IRQ_MASK64(__irq) &              \
            APPCFG_IRQ_INVALID_MASK) == 0));})

/// This is a 64-bit mask, with big-endian bit 'irq' set.
#define STD_IRQ_MASK64(irq) (0x8000000000000000ull >> (irq))

#else

//Untyped assembler version of STD_IRQ_MASK64
#define STD_IRQ_MASK64(irq) (0x8000000000000000 >> (irq))

#endif  /* __ASSEMBLER__ */

#ifndef __ASSEMBLER__
/// These globals are statically initialized elsewhere
extern uint64_t g_ext_irqs_type;
extern uint64_t g_ext_irqs_valid;
extern uint64_t g_ext_irqs_polarity;
extern uint64_t g_ext_irqs_enable;
#endif

#ifdef __ASSEMBLER__
/// These macros aid in the initialization of the external interrupt globals.  I would
/// prefer to use CPP macros, but they don't support recursive macros which I use to
/// convert the variable number of interrupts that a processor can control into static
/// bitmaps used by __hwmacro_setup() at runtime.


    //helper macro for setting up the irq configuration bitmaps for a standard PPE
    .macro .std_irq_config irq_num=-1 irq_type=-1 irq_polarity=-1 irq_mask=-1 parms:vararg
        .if (( \irq_num == -1 ) && ( \irq_type == -1 ) && ( \irq_polarity == -1 ) && ( \irq_mask == -1 ))
#.if ( .ext_irqs_defd != .ext_irqs_valid )
#.error "###### .std_irq_config: Missing configuration for one or more interrupts ######"
#.endif

            .section .sdata
            .align 3
            .global g_ext_irqs_type
            .global g_ext_irqs_polarity
            .global g_ext_irqs_enable
            g_ext_irqs_polarity:
            .quad .ext_irqs_polarity
            g_ext_irqs_type:
            .quad .ext_irqs_type
            g_ext_irqs_enable:
            .quad .ext_irqs_enable
        .else
            .if (( \irq_num < 0 ) || ( \irq_num > (EXTERNAL_IRQS - 1)))
                .error "###### .std_irq_config: invalid irq number \irq_num ######"
            .elseif ((.ext_irqs_valid & (1 << ( EXTERNAL_IRQS - 1 - \irq_num ))) == 0 )
                .error "###### .std_irq_config: Attempt to configure invalid irq number \irq_num ######" 
            .elseif (.ext_irqs_defd & (1 << ( EXTERNAL_IRQS - 1 - \irq_num )))
                .error "###### .std_irq_config: duplicate definition for irq \irq_num ######"
            .else
                .ext_irqs_defd = .ext_irqs_defd | (1 << ( EXTERNAL_IRQS - 1 - \irq_num ))
            .endif

            .if (( \irq_type < 0 ) || ( \irq_type > 1 ))
                .error "###### .std_irq_config: invalid/unspecified irq type \irq_type for irq \irq_num ######" 
            .else
                .ext_irqs_type = .ext_irqs_type | ( \irq_type << ( EXTERNAL_IRQS - 1 - \irq_num ))
            .endif

            .if (( \irq_polarity < 0 ) || ( \irq_polarity > 1 ))
                .error "###### .std_irq_config: invalid/unspecified irq polarity ( \irq_polarity ) for irq \irq_num ######" 
            .else
                .ext_irqs_polarity = .ext_irqs_polarity | ( \irq_polarity << ( EXTERNAL_IRQS - 1 - \irq_num ))
            .endif

            .if (( \irq_mask < 0 ) || ( \irq_mask > 1 ))
                .error "###### .std_irq_config: invalid/unspecified irq mask ( \irq_mask ) for irq \irq_num ######" 
            .else
                .ext_irqs_enable = .ext_irqs_enable | ( \irq_mask << ( EXTERNAL_IRQS - 1 - \irq_num ))
            .endif

            .std_irq_config \parms
        .endif
    .endm

    //Top level macro for generating interrupt configuration globals for a standard PPE
    .macro .std_irq_cfg_bitmaps
        .ext_irqs_valid = ~(APPCFG_IRQ_INVALID_MASK)
        .ext_irqs_type = 0
        .ext_irqs_polarity = 0
        .ext_irqs_enable = 0
        .irq_mask = 0
        .ext_irqs_defd = 0
        .std_irq_config APPCFG_EXT_IRQS_CONFIG
    .endm

#endif /*__ASSEMBLER__*/

#endif /*__STD_IRQ_CONFIG_H__*/ 
