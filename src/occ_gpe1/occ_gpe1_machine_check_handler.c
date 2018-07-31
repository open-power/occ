#include "pk_panic_codes.h"
#include "gpe_centaur.h"
#include "pk.h"

#define OCI_ADDR_BAR_MASK 0xf0000000
#define OCI_ADDR_BAR1 0x90000000

extern uint32_t gpe1_machine_check_handler(uint32_t srr0,
                                           uint32_t srr1,
                                           uint32_t edr);
extern uint32_t g_centaur_access_state;

uint32_t gpe1_machine_check_handler(uint32_t srr0,
                                    uint32_t srr1,
                                    uint32_t edr)
{
    PK_TRACE("GPE1 Machine check! SRR0:%08x SRR1: %08x EDR:%08x",
             srr0,
             srr1,
             edr);

    // It's possible to get back-to-back machine checks for the same condition
    // so CENTAUR_CHANNEL_CHECKSTOP may already be set. Also check that the
    // machine check was due to a Centaur Access (PBABAR1)
    if((g_centaur_access_state == CENTAUR_ACCESS_IN_PROGRESS ||
       g_centaur_access_state == CENTAUR_CHANNEL_CHECKSTOP) &&
       ((edr & OCI_ADDR_BAR_MASK) == OCI_ADDR_BAR1))
    {
        // Returning this to OCC405 will cause sensor to be removed from
        // active list
        g_centaur_access_state = CENTAUR_CHANNEL_CHECKSTOP;

        // The instruction that caused the machine check should
        // be a double word load or store.
        // move the IAR to the instruction after the one that caused
        // the machine check.
        srr0 += 4;
    }
    else
    {
        PK_PANIC( PPE42_MACHINE_CHECK_PANIC );
    }
    return srr0;
}
