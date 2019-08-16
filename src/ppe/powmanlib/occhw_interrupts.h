/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/powmanlib/occhw_interrupts.h $                        */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2019                             */
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
#ifndef __OCCHW_INTERRUPTS_H__
#define __OCCHW_INTERRUPTS_H__

/// \file occ_interrupts.h
/// \brief Interrupt assignments and macros for the OCC
///

#ifndef __ASSEMBLER__
    #include <stdint.h>
#endif

////////////////////////////////////////////////////////////////////////////
// IRQ
////////////////////////////////////////////////////////////////////////////

// The OCB interrupt controller consists of 2 x 32-bit controllers.  Unlike
// PPC ASICs, the OCB controllers are _not_ cascaded.  The combined
// controllers are presented to the application as if there were a single
// 64-bit interrupt controller, while the code underlying the abstraction
// manipulates the 2 x 32-bit controllers independently.
//
// Note that the bits named *RESERVED* are actually implemented in the
// controller, but the interrupt input is tied low. That means they can also
// be used as IPI targets. Logical bits 32..63 are not implemented.

#define OCCHW_IRQ_TRACE_TRIGGER_0               0  /* 0x00 */
#define OCCHW_IRQ_OCC_ERROR                     1  /* 0x01 */
#define OCCHW_IRQ_GPE2_ERROR                    2  /* 0x02 */
#define OCCHW_IRQ_GPE3_ERROR                    3  /* 0x03 */
#define OCCHW_IRQ_CHECK_STOP_GPE2               4  /* 0x04 */
#define OCCHW_IRQ_CHECK_STOP_GPE3               5  /* 0x05 */
#define OCCHW_IRQ_OCC_MALF_ALERT                6  /* 0x06 */
#define OCCHW_IRQ_PVREF_ERROR                   7  /* 0x07 */
#define OCCHW_IRQ_IPI2_HI_PRIORITY              8  /* 0x08 */
#define OCCHW_IRQ_IPI3_HI_PRIORITY              9  /* 0x09 */
#define OCCHW_IRQ_DEBUG_TRIGGER                 10 /* 0x0a */
#define OCCHW_IRQ_RESERVED_11                   11 /* 0x0b */
#define OCCHW_IRQ_PBAX_PGPE_ATTN                12 /* 0x0c */
#define OCCHW_IRQ_PBAX_PGPE_PUSH0               13 /* 0x0d */
#define OCCHW_IRQ_PBAX_PGPE_PUSH1               14 /* 0x0e */
#define OCCHW_IRQ_PBA_OVERCURRENT_INDICATOR     15 /* 0x0f */
#define OCCHW_IRQ_PMC_PCB_INTR_TYPE0_PENDING    16 /* 0x10 */
#define OCCHW_IRQ_PMC_PCB_INTR_TYPE1_PENDING    17 /* 0x11 */
#define OCCHW_IRQ_PMC_PCB_INTR_TYPE2_PENDING    18 /* 0x12 */
#define OCCHW_IRQ_PMC_PCB_INTR_TYPE3_PENDING    19 /* 0x13 */
#define OCCHW_IRQ_PMC_PCB_INTR_TYPE4_PENDING    20 /* 0x14 */
#define OCCHW_IRQ_PMC_PCB_INTR_TYPE5_PENDING    21 /* 0x15 */
#define OCCHW_IRQ_PMC_PCB_INTR_TYPE6_PENDING    22 /* 0x16 */
#define OCCHW_IRQ_PMC_PCB_INTR_TYPE7_PENDING    23 /* 0x17 */
#define OCCHW_IRQ_PMC_PCB_INTR_TYPE8_PENDING    24 /* 0x18 */
#define OCCHW_IRQ_PMC_PCB_INTR_TYPE9_PENDING    25 /* 0x19 */
#define OCCHW_IRQ_PMC_PCB_INTR_TYPEA_PENDING    26 /* 0x1a */
#define OCCHW_IRQ_PMC_PCB_INTR_TYPEB_PENDING    27 /* 0x1b */
#define OCCHW_IRQ_PMC_PCB_INTR_TYPEC_PENDING    28 /* 0x1c */
#define OCCHW_IRQ_PMC_PCB_INTR_TYPED_PENDING    29 /* 0x1d */
#define OCCHW_IRQ_PMC_PCB_INTR_TYPEE_PENDING    30 /* 0x1e */
#define OCCHW_IRQ_PMC_PCB_INTR_TYPEF_PENDING    31 /* 0x1f */
#define OCCHW_IRQ_DEBUGGER                      32 /* 0x20 */
#define OCCHW_IRQ_TRACE_TRIGGER_33              33 /* 0x21 */
#define OCCHW_IRQ_RESERVED_34                   34 /* 0x22 */
#define OCCHW_IRQ_PBA_ERROR                     35 /* 0x23 */
#define OCCHW_IRQ_GPE0_ERROR                    36 /* 0x24 */
#define OCCHW_IRQ_GPE1_ERROR                    37 /* 0x25 */
#define OCCHW_IRQ_CHECK_STOP_PPC405             38 /* 0x26 */
#define OCCHW_IRQ_EXTERNAL_TRAP                 39 /* 0x27 */
#define OCCHW_IRQ_OCC_TIMER0                    40 /* 0x28 */
#define OCCHW_IRQ_OCC_TIMER1                    41 /* 0x29 */
#define OCCHW_IRQ_IPI0_HI_PRIORITY              42 /* 0x2a */
#define OCCHW_IRQ_IPI1_HI_PRIORITY              43 /* 0x2b */
#define OCCHW_IRQ_IPI4_HI_PRIORITY              44 /* 0x2c */
#define OCCHW_IRQ_I2CM_INTR                     45 /* 0x2d */
#define OCCHW_IRQ_IPI_SCOM                      46 /* 0x2e */
#define OCCHW_IRQ_DCM_INTF_ONGOING              47 /* 0x2f */
#define OCCHW_IRQ_PBAX_OCC_SEND                 48 /* 0x30 */
#define OCCHW_IRQ_PBAX_OCC_PUSH0                49 /* 0x31 */
#define OCCHW_IRQ_PBAX_OCC_PUSH1                50 /* 0x32 */
#define OCCHW_IRQ_PBA_BCDE_ATTN                 51 /* 0x33 */
#define OCCHW_IRQ_PBA_BCUE_ATTN                 52 /* 0x34 */
#define OCCHW_IRQ_STRM0_PULL                    53 /* 0x35 */
#define OCCHW_IRQ_STRM0_PUSH                    54 /* 0x36 */
#define OCCHW_IRQ_STRM1_PULL                    55 /* 0x37 */
#define OCCHW_IRQ_STRM1_PUSH                    56 /* 0x38 */
#define OCCHW_IRQ_STRM2_PULL                    57 /* 0x39 */
#define OCCHW_IRQ_STRM2_PUSH                    58 /* 0x3a */
#define OCCHW_IRQ_STRM3_PULL                    59 /* 0x3b */
#define OCCHW_IRQ_STRM3_PUSH                    60 /* 0x3c */
#define OCCHW_IRQ_IPI0_LO_PRIORITY              61 /* 0x3d */
#define OCCHW_IRQ_IPI1_LO_PRIORITY              62 /* 0x3e */
#define OCCHW_IRQ_IPI4_LO_PRIORITY              63 /* 0x3f */

/// This constant is used to define the size of the table of interrupt handler
/// structures as well as a limit for error checking.  The entire 64-bit
/// vector is now in use.

#define OCCHW_IRQS 64

// Please keep the string definitions up-to-date as they are used for
// reporting in the Simics simulation.

#define OCCHW_IRQ_STRINGS(var)                      \
    const char* var[OCCHW_IRQS] = {                 \
                                                    "OCCHW_IRQ_TRACE_TRIGGER_0",                \
                                                    "OCCHW_IRQ_OCC_ERROR",                      \
                                                    "OCCHW_IRQ_GPE2_ERROR",                     \
                                                    "OCCHW_IRQ_GPE3_ERROR",                     \
                                                    "OCCHW_IRQ_CHECK_STOP_GPE2",                \
                                                    "OCCHW_IRQ_CHECK_STOP_GPE3",                \
                                                    "OCCHW_IRQ_OCC_MALF_ALERT",                 \
                                                    "OCCHW_IRQ_PVREF_ERROR",                    \
                                                    "OCCHW_IRQ_IPI2_HI_PRIORITY",               \
                                                    "OCCHW_IRQ_IPI3_HI_PRIORITY",               \
                                                    "OCCHW_IRQ_DEBUG_TRIGGER",                  \
                                                    "OCCHW_IRQ_RESERVED_11",                    \
                                                    "OCCHW_IRQ_PBAX_PGPE_ATTN",                 \
                                                    "OCCHW_IRQ_PBAX_PGPE_PUSH0",                \
                                                    "OCCHW_IRQ_PBAX_PGPE_PUSH1",                \
                                                    "OCCHW_IRQ_PBA_OVERCURRENT_INDICATOR",      \
                                                    "OCCHW_IRQ_PMC_PCB_INTR_TYPE0_PENDING",     \
                                                    "OCCHW_IRQ_PMC_PCB_INTR_TYPE1_PENDING",     \
                                                    "OCCHW_IRQ_PMC_PCB_INTR_TYPE2_PENDING",     \
                                                    "OCCHW_IRQ_PMC_PCB_INTR_TYPE3_PENDING",     \
                                                    "OCCHW_IRQ_PMC_PCB_INTR_TYPE4_PENDING",     \
                                                    "OCCHW_IRQ_PMC_PCB_INTR_TYPE5_PENDING",     \
                                                    "OCCHW_IRQ_PMC_PCB_INTR_TYPE6_PENDING",     \
                                                    "OCCHW_IRQ_PMC_PCB_INTR_TYPE7_PENDING",     \
                                                    "OCCHW_IRQ_PMC_PCB_INTR_TYPE8_PENDING",     \
                                                    "OCCHW_IRQ_PMC_PCB_INTR_TYPE9_PENDING",     \
                                                    "OCCHW_IRQ_PMC_PCB_INTR_TYPEA_PENDING",     \
                                                    "OCCHW_IRQ_PMC_PCB_INTR_TYPEB_PENDING",     \
                                                    "OCCHW_IRQ_PMC_PCB_INTR_TYPEC_PENDING",     \
                                                    "OCCHW_IRQ_PMC_PCB_INTR_TYPED_PENDING",     \
                                                    "OCCHW_IRQ_PMC_PCB_INTR_TYPEE_PENDING",     \
                                                    "OCCHW_IRQ_PMC_PCB_INTR_TYPEF_PENDING",     \
                                                    "OCCHW_IRQ_DEBUGGER",                       \
                                                    "OCCHW_IRQ_TRACE_TRIGGER_33",               \
                                                    "OCCHW_IRQ_RESERVED_34",                    \
                                                    "OCCHW_IRQ_PBA_ERROR",                      \
                                                    "OCCHW_IRQ_GPE0_ERROR",                     \
                                                    "OCCHW_IRQ_GPE1_ERROR",                     \
                                                    "OCCHW_IRQ_CHECK_STOP_PPC405",              \
                                                    "OCCHW_IRQ_EXTERNAL_TRAP",                  \
                                                    "OCCHW_IRQ_OCC_TIMER0",                     \
                                                    "OCCHW_IRQ_OCC_TIMER1",                     \
                                                    "OCCHW_IRQ_IPI0_HI_PRIORITY",               \
                                                    "OCCHW_IRQ_IPI1_HI_PRIORITY",               \
                                                    "OCCHW_IRQ_IPI4_HI_PRIORITY",               \
                                                    "OCCHW_IRQ_I2CM_INTR",                      \
                                                    "OCCHW_IRQ_IPI_SCOM",                       \
                                                    "OCCHW_IRQ_DCM_INTF_ONGOING",               \
                                                    "OCCHW_IRQ_PBAX_OCC_SEND",                  \
                                                    "OCCHW_IRQ_PBAX_OCC_PUSH0",                 \
                                                    "OCCHW_IRQ_PBAX_OCC_PUSH1",                 \
                                                    "OCCHW_IRQ_PBA_BCDE_ATTN",                  \
                                                    "OCCHW_IRQ_PBA_BCUE_ATTN",                  \
                                                    "OCCHW_IRQ_STRM0_PULL",                     \
                                                    "OCCHW_IRQ_STRM0_PUSH",                     \
                                                    "OCCHW_IRQ_STRM1_PULL",                     \
                                                    "OCCHW_IRQ_STRM1_PUSH",                     \
                                                    "OCCHW_IRQ_STRM2_PULL",                     \
                                                    "OCCHW_IRQ_STRM2_PUSH",                     \
                                                    "OCCHW_IRQ_STRM3_PULL",                     \
                                                    "OCCHW_IRQ_STRM3_PUSH",                     \
                                                    "OCCHW_IRQ_IPI0_LO_PRIORITY",               \
                                                    "OCCHW_IRQ_IPI1_LO_PRIORITY",               \
                                                    "OCCHW_IRQ_IPI4_LO_PRIORITY",               \
                                  };


/// Routing codes for OCB interrupts
#define OCCHW_IRQ_TARGET_ID_405_NONCRIT     0
#define OCCHW_IRQ_TARGET_ID_405_CRIT        1
#define OCCHW_IRQ_TARGET_ID_405_UNCOND      2
#define OCCHW_IRQ_TARGET_ID_405_DEBUG       3
#define OCCHW_IRQ_TARGET_ID_GPE0            4
#define OCCHW_IRQ_TARGET_ID_GPE1            5
#define OCCHW_IRQ_TARGET_ID_GPE2            6
#define OCCHW_IRQ_TARGET_ID_GPE3            7

// OCB interrupt type values (level or edge)
#define OCCHW_IRQ_TYPE_LEVEL            0
#define OCCHW_IRQ_TYPE_EDGE             1

// OCB interrupt polarity values (high or low, rising falling)
#define OCCHW_IRQ_POLARITY_LO           0
#define OCCHW_IRQ_POLARITY_FALLING      0
#define OCCHW_IRQ_POLARITY_HI           1
#define OCCHW_IRQ_POLARITY_RISING       1

// OCB interrupt mask values (masked or enabled)
#define OCCHW_IRQ_MASKED                0
#define OCCHW_IRQ_ENABLED               1

// Note: All standard-product IPI uses are declared here to avoid conflicts
// Validation- and lab-only IPI uses are documented in validation.h

/// The deferred callback queue interrupt
///
/// This IPI is reserved for use of the async deferred callback mechanism.
/// This IPI is used by both critical and noncritical async handlers to
/// activate the deferred callback mechanism.
#define OCCHW_IRQ_ASYNC_IPI OCCHW_IRQ_IPI4_LO_PRIORITY


#ifndef __ASSEMBLER__

/// This expression recognizes only those IRQ numbers that have named
/// (non-reserved) interrupts in the OCB interrupt controller.

// There are so many invalid interrupts now that it's a slight improvement in
// code size to let the compiler optimize the invalid IRQs to a bit mask for
// the comparison.

#define OCCHW_IRQ_VALID(irq) \
    ({unsigned __irq = (unsigned)(irq); \
        ((__irq < OCCHW_IRQS) &&                                  \
         ((OCCHW_IRQ_MASK64(__irq) &                              \
           (OCCHW_IRQ_MASK64(OCCHW_IRQ_RESERVED_11) |               \
            OCCHW_IRQ_MASK64(OCCHW_IRQ_RESERVED_34))) == 0));})

/// This is a 32-bit mask, with big-endian bit (irq % 32) set.
#define OCCHW_IRQ_MASK32(irq) (((uint32_t)0x80000000) >> ((irq) % 32))

/// This is a 64-bit mask, with big-endian bit 'irq' set.
#define OCCHW_IRQ_MASK64(irq) (0x8000000000000000ull >> (irq))

#else

//assembler version of OCCHW_IRQ_MASK32
#define OCCHW_IRQ_MASK32(irq) ((0x80000000) >> ((irq) % 32))

#endif  /* __ASSEMBLER__ */

#ifndef __ASSEMBLER__

    // These macros select OCB interrupt controller registers based on the IRQ
    // number.

    #define OCCHW_OIMR_CLR(irq) (((irq) & 0x20) ? OCB_OIMR1_CLR : OCB_OIMR0_CLR)
    #define OCCHW_OIMR_OR(irq)  (((irq) & 0x20) ? OCB_OIMR1_OR  : OCB_OIMR0_OR)

    #define OCCHW_OISR(irq)     (((irq) & 0x20) ? OCB_OISR1     : OCB_OISR0)
    #define OCCHW_OISR_CLR(irq) (((irq) & 0x20) ? OCB_OISR1_CLR : OCB_OISR0_CLR)
    #define OCCHW_OISR_OR(irq)  (((irq) & 0x20) ? OCB_OISR1_OR  : OCB_OISR0_OR)

    #define OCCHW_OIEPR(irq)     (((irq) & 0x20) ? OCB_OIEPR1     : OCB_OIEPR0)
    #define OCCHW_OIEPR_OR(irq)  (((irq) & 0x20) ? OCB_OIEPR1_OR  : OCB_OIEPR0_OR)
    #define OCCHW_OIEPR_CLR(irq) (((irq) & 0x20) ? OCB_OIEPR1_CLR : OCB_OIEPR0_CLR)
    #define OCCHW_OITR(irq)      (((irq) & 0x20) ? OCB_OITR1      : OCB_OITR0)
    #define OCCHW_OITR_OR(irq)   (((irq) & 0x20) ? OCB_OITR1_OR   : OCB_OITR0_OR)
    #define OCCHW_OITR_CLR(irq)  (((irq) & 0x20) ? OCB_OITR1_CLR  : OCB_OITR0_CLR)

    #define OCCHW_OIRRA(irq)     (((irq) & 0x20) ? OCB_OIRR1A     : OCB_OIRR0A)
    #define OCCHW_OIRRA_OR(irq)  (((irq) & 0x20) ? OCB_OIRR1A_OR  : OCB_OIRR0A_OR)
    #define OCCHW_OIRRA_CLR(irq) (((irq) & 0x20) ? OCB_OIRR1A_CLR : OCB_OIRR0A_CLR)
    #define OCCHW_OIRRB(irq)     (((irq) & 0x20) ? OCB_OIRR1B     : OCB_OIRR0B)
    #define OCCHW_OIRRB_OR(irq)  (((irq) & 0x20) ? OCB_OIRR1B_OR  : OCB_OIRR0B_OR)
    #define OCCHW_OIRRB_CLR(irq) (((irq) & 0x20) ? OCB_OIRR1B_CLR : OCB_OIRR0B_CLR)
    #define OCCHW_OIRRC(irq)     (((irq) & 0x20) ? OCB_OIRR1C     : OCB_OIRR0C)
    #define OCCHW_OIRRC_OR(irq)  (((irq) & 0x20) ? OCB_OIRR1C_OR  : OCB_OIRR0C_OR)
    #define OCCHW_OIRRC_CLR(irq) (((irq) & 0x20) ? OCB_OIRR1C_CLR : OCB_OIRR0C_CLR)
#endif  /* __ASSEMBLER__ */

#endif  /* __OCCHW_INTERRUPTS_H__ */
