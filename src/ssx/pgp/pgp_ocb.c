/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/pgp/pgp_ocb.c $                                       */
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
/// \file pgp_ocb.c
/// \brief OCB-related drivers for PgP

#include "ssx.h"

//use compile-time default in case the OCB timer is never used -- grm
unsigned int g_ocb_timer_divider = OCB_TIMER_DIVIDER_DEFAULT;

/// Reset an OCB timer
///
/// \param timer A valid OCB timer index
///
/// \param auto_reload A non-0 value indicates to run the timer in auto-reload
/// mode. 
///
/// \param timeout_ns The timeout specified in nanoseconds. The actual timeout
/// will be rounded down to the underlying timer tick, with a minimum 1 tick
/// timeout. However if the \a timeout_ns argument is explicity 0, then the
/// timer wil be initialized with a 0 timeout, effectively disabling the
/// timer.
///
/// Reseting an OCB timer means rewriting the timer control register with a
/// potentially new auto-reload enable and new timeout value. This API also
/// clears any pending timer interrupt status.
///
/// \retval 0 Success
///
/// \retval -OCB_INVALID_ARGUMENT_TIMER Causes include illegal timer 
/// numbers and illegal or unrepresntable timeouts.

// Note that OCB_TIMER_FREQUENCY_HZ is in the range of 1-2 MHz.

int
ocb_timer_reset(int timer,
                int auto_reload,
                int timeout_ns)
{
    ocb_otrn_t otr;
    int ticks;

    //printk("ocb_timer_reset(%d, %d, %d)\n", 
    //       timer, auto_reload, timeout_ns);

    if (timeout_ns != 0) {
        ticks = MAX(1, timeout_ns / (1000000000 / OCB_TIMER_FREQUENCY_HZ));
    } else {
        ticks = 0;
    }

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF((timer < 0) ||
                     (timer >= OCB_TIMERS) ||
                     (timeout_ns < 0) ||
                     (ticks != ((uint16_t)ticks)),
                     OCB_INVALID_ARGUMENT_TIMER);
    }

    otr.value = 0;

    otr.fields.timeout = 1;
    otr.fields.control = 1;
    otr.fields.auto_reload = (auto_reload != 0);
    otr.fields.timer = ticks;

    out32(OCB_OTRN(timer), otr.value);

    return 0;
}


/// Set up an OCB timer and interrupt handler
///
/// \param timer A valid OCB timer index
///
/// \param auto_reload A non-0 value indicates to run the timer in auto-reload
/// mode. 
///
/// \param timeout_ns The timeout specified in nanoseconds. The actual timeout
/// will rounded down to the underlying timer tick, with a minimum 1 tick
/// timeout. However if the \a timeout_ns argument is explicity 0, then the
/// timer wil be initialized with a 0 timeout, effectively disabling the
/// timer.
///
/// \param handler The interrupt handler for the timer interrupt
///
/// \param arg The private argument of the interrupt handler
///
/// \param priority The SSX/PPC405 interrupt priority to assign to the
/// interrupt. 
///
/// This API sets up and starts the timer and unmasks the timer
/// interrupt. Once set up, the timer can be reset using ocb_timer_reset(). As
/// a fine point of the specification, if a timer interrupt is already pending
/// when this API is invoked then that interrupt will be cleared.  Only the
/// next interrupt (corresponding to the new programming) will be serviced by
/// the newly installed handler. 
///
/// Note that the interrupt handler is responsible for clearing the timer
/// interrupt status. An API ocb_timer_status_clear() is available for this.
///
/// \retval 0 Success
///
/// \retval -OCB_INVALID_ARGUMENT_TIMER Causes include illegal timer 
/// numbers and illegal or unrepresntable timeouts.
///
/// Other errors may be returned by the underlying calls to SSX routines that
/// set up interrupts.

int
ocb_timer_setup(int timer, 
                int auto_reload,
                int timeout_ns,
                SsxIrqHandler handler,
                void *arg,
                int priority)
{
    int rc;
    tpc_hpr2_t l_hpr2;

    do
    {
        //Read Hang Pulse Register 2 to get the log base 2 of the ocb clock divider -- grm
        rc = getscom(TPC_HPR2, &l_hpr2.value);
        if(rc)
        {
            break;
        }

        g_ocb_timer_divider = 1 << l_hpr2.fields.hang_pulse_reg;

        //printk("ocb_timer_setup(%d, %d, %d, %p, %p, %d)\n",
        //       timer, auto_reload, timeout_ns,
        //       handler, arg, priority);
    
        ssx_irq_disable(PGP_IRQ_OCC_TIMER0 + timer);

        ssx_irq_setup(PGP_IRQ_OCC_TIMER0 + timer,
                  SSX_IRQ_POLARITY_ACTIVE_HIGH,
                  SSX_IRQ_TRIGGER_LEVEL_SENSITIVE);

        ssx_irq_handler_set(PGP_IRQ_OCC_TIMER0 + timer,
                        handler,
                        arg,
                        priority);

        rc = ocb_timer_reset(timer, auto_reload, timeout_ns);

        ssx_irq_enable(PGP_IRQ_OCC_TIMER0 + timer);
    }while(0);

    return rc;
}


/// Generate an core interrupt via the PSI Host Bridge
///
/// Setting OCB_OCCMISC.core_ext_int to 1 causes a wire to pulse to the PSI
/// Host Bridge to allow the presentation of an external interrupt to a core
/// thread.  The core thread to be interrupted is controlled by the XIVR - OCC
/// register, SCOM 02010916.  Normally the hypervisor will set up the PSI Host
/// Bridge. This procedure allows OCC to send an interrupt to the hypervisor.
///
/// \retval 0 Success

// The interrupt is generated by causing a 0->1 transation on
// OCB_OCCMISC.core_ext_intr. This is implemented here using the WAND/WOR
// forms of the register.

int
ocb_core_interrupt()
{
    ocb_occmisc_t oo;

    oo.value = 0;
    oo.fields.core_ext_intr = 1;
    out32(OCB_OCCMISC_AND, ~oo.value);
    out32(OCB_OCCMISC_OR, oo.value);

    return 0;
}


/// Procedure to setup a linear window on an indirect channel
///
///
/// Since the linear window access is restricted to the SRAM region of
/// OCI space, Bits 0:4 of the base parameter are don't care and will be
/// overwritten with b'11000'
///
///
/// \todo double check SRAM region restriction 
///
/// \param channel The indirect channel to use, in the range 0..2.
///
/// \param base The 32-bit PowerBus base address where the block starts.  This
/// address must be aligned to the \a log_size.
///
/// \param log_size The base 2 logarithm of the block size, in bytes. The
/// minimum size is 8B (2**3), the maximum size is 32KB (2**15)
///
///
/// \retval 0 Success
///
/// \retval OCB_INVALID_ARGUMENT_LW_INIT One or more of the parameter 
/// restrictions were violated.
///
/// \retval OCB_SCOM_ERROR An attempt to write a PBA SCOM register to set up
/// the BARs produced a non-zero return code.

int
ocb_linear_window_initialize(int channel, uint32_t base, int log_size)
{
    uint32_t mask ;
    ocb_ocblwcrn_t ocblwcrn;
    ocb_ocblwsbrn_t ocblwsbrn;

    // create mask for checking
    mask = (0x1ull << log_size) - 1;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF((channel < 0) || 
                     (channel > 2) ||
                     (log_size < OCB_LW_LOG_SIZE_MIN) || 
                     (log_size > OCB_LW_LOG_SIZE_MAX) ||
                     ((base & mask) != 0),
                     OCB_INVALID_ARGUMENT_LW_INIT);
    }

    // now invert mask for use in ocb linear window setup
    mask = ~mask;


    // Configure OCB Linear Write Control Register    
    ocblwcrn.fields.linear_window_enable = 1;
    // base 13:28 (16 bits) 
    ocblwcrn.fields.linear_window_bar = (base >> 3) & 0xFFFF;
    // mask 17:28 (12 bits)
    ocblwcrn.fields.linear_window_mask = (mask >> 3) & 0xFFF;
    out32(OCB_OCBLWCRN(channel), ocblwcrn.value);

    // Configure OCB Linear Window Write Base Register
    ocblwsbrn.fields.linear_window_region = 3;  // SRAM only
    // \todo:  Are there constants for the OCI regions?
        // base 2:9 (8 bits)
    ocblwsbrn.fields.linear_window_base = (base >> 19) & 0xFF;
    out32(OCB_OCBLWSBRN(channel), ocblwsbrn.value);

    return 0 ;
}

/// Procedure to disable a linear window on an indirect channel
///
/// This procedure will disable the linear window while maintaining
/// the linear_window_bar and linear_window_mask settings
///
/// \param channel The indirect channel to disable, in the range 0..2.
///
/// \retval 0 Success
///
/// \retval OCB_INVALID_ARGUMENT_LW_DISABLE One or more of the parameter 
/// restrictions were violated.
///

int
ocb_linear_window_disable(int channel)
{
    ocb_ocblwcrn_t ocblwcrn;
 
    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF((channel < 0) || 
                     (channel > 2),
                     OCB_INVALID_ARGUMENT_LW_DISABLE);
    }

    ocblwcrn.value = in32(OCB_OCBLWCRN(channel));
    // Configure OCB Linear Write Control Register    
    ocblwcrn.fields.linear_window_enable = 0;
    out32(OCB_OCBLWCRN(channel), ocblwcrn.value);

    return 0 ;
}


/// Procedure for setting up untrusted mode for an indirect channel
///
/// Note that the OCC FW is expected to enable the channel for FSP
/// access.  As an untrusted master, the FSP cannot configure this
/// in a chip running in trusted mode.  The SBE is considered a trusted
/// master.
///
///
/// \param channel The indirect channel to use, in the range 0..2
/// Note that this bit is not used for indirect channel 3.
///
///
/// \param allow_untrusted Enable untrusted PIB masters 
/// access to the indirect channel being configured.  If allow_untrusted is
/// not enabled and the chip is running in trusted mode, then any untrusted
/// PIB master will get an offline return code when attempting to write
/// the indirect channel.  0 = Disable, 1 = Enable
///
/// \retval 0 Success
///
/// \retval OCB_INVALID_ARGUMENT_UNTRUST One or more of the parameter 
/// restrictions were violated.
///


int
ocb_allow_untrusted_initialize(int channel, int allow_untrusted)
{
    ocb_ocbicrn_t ocbicrn;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF((channel < 0) || 
                     (channel > 2) ||
                     (allow_untrusted < 0) ||
                     (allow_untrusted > 1),
                     OCB_INVALID_ARGUMENT_UNTRUST);
    }

    // Configure allow_unsecure_pib_masters bit
    ocbicrn.fields.allow_unsecure_pib_masters = allow_untrusted;
    out32(OCB_OCBICRN(channel), ocbicrn.value);  

    return 0 ;
}
