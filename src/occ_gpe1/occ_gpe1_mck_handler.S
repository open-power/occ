.nolist
#include "pk.h"
.list

.section .text, "ax", @progbits
.global __gpe1_machine_check_handler
.global __pk_ctx_pop

__gpe1_machine_check_handler:
#if !defined(__PPE42X__)
    stwu    %r1,    -PK_CTX_SIZE(%r1)
    stw     %r0,    PK_CTX_GPR0(%r1)
    stvd    %d3,    PK_CTX_GPR3(%r1)
    stvd    %d5,    PK_CTX_GPR5(%r1)
    stvd    %d7,    PK_CTX_GPR7(%r1)
    stvd    %d9,    PK_CTX_GPR9(%r1)
    stvd    %d28,   PK_CTX_GPR28(%r1)
    stvd    %d30,   PK_CTX_GPR30(%r1)
    mflr    %r3
    stw     %r3,    PK_CTX_LR(%r1)
    mfcr    %r3
    mfsprg0 %r4
    stvd    %d3,    PK_CTX_CR(%r1)
    mfxer   %r3
    mfctr   %r4
    stvd    %d3,    PK_CTX_XER(%r1)
    mfsrr0  %r3
    mfsrr1  %r4
    stvd    %d3,    PK_CTX_SRR0(%r1)
#else
    stcxtu  %r1,    -PK_CTX_SIZE(%r1)
    mfsrr0  %r3
    mfsrr1  %r4
#endif

    mfedr   %r5

    bl  gpe1_machine_check_handler
    stw %r3,    PK_CTX_SRR0(%r1)

#if !defined(__PPE42X__)
    lwz     %r0,    PK_CTX_GPR0(%r1)
    lvd     %d7,    PK_CTX_SRR0(%r1)
    mtsrr1  %r8
    mtsrr0  %r7
    lvd     %d5,    PK_CTX_XER(%r1)
    mtctr   %r6
    mtxer   %r5
    lvd     %d30,   PK_CTX_GPR30(%r1)
    lvd     %d28,   PK_CTX_GPR28(%r1)
    lvd     %d9,    PK_CTX_GPR9(%r1)
    lvd     %d7,    PK_CTX_GPR7(%r1)
    lvd     %d5,    PK_CTX_GPR5(%r1)
    lvd     %d3,    PK_CTX_CR(%r1)  ## CR,SPRG0
    mtcr0   %r3
    lwz     %r4,    PK_CTX_LR(%r1)
    mtlr    %r4
    lvd     %d3,    PK_CTX_GPR3(%r1)
    addi    %r1,    %r1, PK_CTX_SIZE
#else
    lcxt    %r1,%r1
#endif
    rfi

