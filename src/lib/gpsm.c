/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/gpsm.c $                                              */
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
/// \file gpsm.c
/// \brief Global Pstate Mechanism procedures
///
/// \todo : Should we initialize any/all iVRM delays in gpsm_lpsa_install()?

#include "ssx.h"
#include "pstates.h"
#include "gpe_control.h"
#include "gpsm.h"
#include "vrm.h"

/// The semaphore used to block threads waiting for GPSM protocol actions

SsxSemaphore G_gpsm_protocol_semaphore;


////////////////////////////////////////////////////////////////////////////
// Private Utilities
////////////////////////////////////////////////////////////////////////////

// The mechanical transition to Firmware Pstate Mode - Not a procedure

static void
_gpsm_fw_mode(void)
{
    pmc_mode_reg_t pmr;

    pmr.value = in32(PMC_MODE_REG);
    pmr.fields.enable_hw_pstate_mode = 0;
    pmr.fields.enable_fw_auction_pstate_mode = 0;
    pmr.fields.enable_fw_pstate_mode = 1;
    out32(PMC_MODE_REG, pmr.value);
}


// The mechanical transition to Firmware Auction Pstate Mode - Not a procedure

static void
_gpsm_fw_auction_mode(void)
{
    pmc_mode_reg_t pmr;

    pmr.value = in32(PMC_MODE_REG);
    pmr.fields.enable_hw_pstate_mode = 0;
    pmr.fields.enable_fw_auction_pstate_mode = 1;
    pmr.fields.enable_fw_pstate_mode = 0;
    out32(PMC_MODE_REG, pmr.value);
}


// The mechanical transition to Hardware Pstate Mode - Not a procedure.
// Disable voltage change via safe_mode_without_spivid 
// before enter hw mode to prevent possible glitch that 
// hw momentarily flush turbo to pstate actual, 
// After enter hw mode, we will enable back the spivid

static void
_gpsm_hw_mode(void)
{
    pmc_mode_reg_t pmr;

    if (!gpsm_dcm_slave_p()) {
        pmr.value = in32(PMC_MODE_REG);
        pmr.fields.safe_mode_without_spivid = 1;
        out32(PMC_MODE_REG, pmr.value);
    }

    pmr.value = in32(PMC_MODE_REG);
    pmr.fields.enable_hw_pstate_mode = 1;
    pmr.fields.enable_fw_auction_pstate_mode = 0;
    pmr.fields.enable_fw_pstate_mode = 0;
    out32(PMC_MODE_REG, pmr.value);

    if (!gpsm_dcm_slave_p()) {
        pmr.value = in32(PMC_MODE_REG);
        pmr.fields.safe_mode_without_spivid = 0;
        out32(PMC_MODE_REG, pmr.value);
    }

}

    
////////////////////////////////////////////////////////////////////////////
// Private Sub-Procedures
////////////////////////////////////////////////////////////////////////////

// By definition, quiescing the GPSM always leaves the system in firmware
// Pstate mode.  This is necessary for a consistent specification due to the
// fact that in general, Hardware Pstate mode can not be quiesced without
// leaving that mode.

// To quiesce the GPSM in firmware or firmware auction mode requires waiting
// for both the voltage and frequency changes to be complete.  Note that they
// will never be ongoing simultaneously. This predicate is used for all
// firmware-mode quiesce, even though normally only one part or the other
// (voltage/protocol) is active.
//
// Recall that PMC interrupts are level-low, so if ongoing status is clear,
// the operation is ongoing.

static int
gpsm_fw_quiesce(void)
{
    int rc = 0;
    
    if (!ssx_irq_status_get(PGP_IRQ_PMC_PROTOCOL_ONGOING)) {
        ssx_irq_enable(PGP_IRQ_PMC_PROTOCOL_ONGOING);
        rc = ssx_semaphore_pend(&G_gpsm_protocol_semaphore, SSX_WAIT_FOREVER);
    }

    if ((!rc) && !ssx_irq_status_get(PGP_IRQ_PMC_VOLTAGE_CHANGE_ONGOING)) {
        ssx_irq_enable(PGP_IRQ_PMC_VOLTAGE_CHANGE_ONGOING);
        rc = ssx_semaphore_pend(&G_gpsm_protocol_semaphore, SSX_WAIT_FOREVER);
    }

    if (!rc) {
        _gpsm_fw_mode();
    }

    return rc;
}


// To quiesce the GPSM in hardware mode requires waiting for any ongoing
// Pstate change to be complete.  Note that there is no guarantee that this
// condition will ever be true in general unless something external to PMC
// ensures that Global bids stop coming in to the GPSM.  An alternative used
// here is to 'lock' the GPSM temporarily by setting the rail bounds min and
// max to the current Global Pstate Actual.  The GPSM will eventually quiesce
// at the global actual, and we can safely move to Firmware Pstate mode and
// release the lock.
//
// Recall that PMC 'ongoing' interrupts are level-low, so if ongoing status is
// clear, the operation is ongoing.

static int
gpsm_hw_quiesce(void)
{
    int rc = 0;
    pmc_rail_bounds_register_t prbr, original_prbr;
    pmc_pstate_monitor_and_ctrl_reg_t ppmacr;

    ppmacr.value = in32(PMC_PSTATE_MONITOR_AND_CTRL_REG);
    
    original_prbr.value = prbr.value = in32(PMC_RAIL_BOUNDS_REGISTER);
    prbr.fields.pmin_rail = ppmacr.fields.gpsa;
    prbr.fields.pmax_rail = ppmacr.fields.gpsa;
    out32(PMC_RAIL_BOUNDS_REGISTER, prbr.value);

    rc = _gpsm_hw_quiesce();
    
    if (!rc) {
        _gpsm_fw_mode();
        out32(PMC_RAIL_BOUNDS_REGISTER, original_prbr.value);
    }

    return rc;
}

////////////////////////////////////////////////////////////////////////////
// Public Predicates
////////////////////////////////////////////////////////////////////////////

/// Is the Global Pstate Mechanism quiesced?
///
/// This predicate can only truly be answered 'true' if we are not in
/// hardware Pstate mode.  
///
/// \retval 0 Either we're in Hardware Pstate Mode, or a Voltage/Frequency
/// operation is ongoing.
///
/// \retval 1 We're not in Hardware Pstate Mode and no Voltage/Frequency
/// operation is ongoing.

int
gpsm_quiesced_p(void)
{
    return !(gpsm_hw_mode_p() ||
             !ssx_irq_status_get(PGP_IRQ_PMC_PROTOCOL_ONGOING) ||
             !ssx_irq_status_get(PGP_IRQ_PMC_VOLTAGE_CHANGE_ONGOING));
}

/// Predicate: Is the PMC in hardware Pstate mode?
///
/// \returns 0/1

int
gpsm_hw_mode_p(void)
{
    pmc_mode_reg_t pmr;

    pmr.value = in32(PMC_MODE_REG);
    return (pmr.fields.enable_hw_pstate_mode != 0);
}


/// Predicate: Is the PMC in firmware auction Pstate mode?
///
/// \returns 0/1

int
gpsm_fw_auction_mode_p(void)
{
    pmc_mode_reg_t pmr;

    pmr.value = in32(PMC_MODE_REG);
    return (pmr.fields.enable_fw_auction_pstate_mode != 0);
}


/// Predicate: Is the PMC in firmware Pstate mode?
///
/// \returns 0/1

int
gpsm_fw_mode_p(void)
{
    pmc_mode_reg_t pmr;

    pmr.value = in32(PMC_MODE_REG);
    return (pmr.fields.enable_fw_pstate_mode != 0);
}


/// Predicate: Is the chip configured as a DCM?
///
/// \returns 0/1

int
gpsm_dcm_mode_p(void)
{
    pmc_mode_reg_t pmc_mode_reg;
    pmc_mode_reg.value = in32(PMC_MODE_REG);
    return pmc_mode_reg.fields.enable_interchip_interface;
}


/// Predicate: Is the chip configured as a DCM Slave?
///
/// \returns 0/1

int
gpsm_dcm_master_p(void)
{
    pmc_mode_reg_t pmc_mode_reg;
    pmc_mode_reg.value = in32(PMC_MODE_REG);
    return 
        pmc_mode_reg.fields.enable_interchip_interface &&
        pmc_mode_reg.fields.interchip_mode;
}


/// Predicate: Is the chip configured as a DCM Slave?
///
/// \returns 0/1

int
gpsm_dcm_slave_p(void)
{
    pmc_mode_reg_t pmc_mode_reg;
    pmc_mode_reg.value = in32(PMC_MODE_REG);
    return 
        pmc_mode_reg.fields.enable_interchip_interface &&
        (pmc_mode_reg.fields.interchip_mode == 0);
}
    

        

////////////////////////////////////////////////////////////////////////////
// Procedures
////////////////////////////////////////////////////////////////////////////

/// Recover the GlobalPstateTable object from the PMC
///
/// \note It is assumed that the pointer to the Global Pstate table installed
/// in the PMC is actually a pointer to a complete GlobalPstateTable object
/// (which contains a Global Pstate table as its first element).
///
/// \returns A pointer to the currently active GlobalPstateTable object.

GlobalPstateTable*
gpsm_gpst(void)
{
    pmc_parameter_reg1_t ppr1;

    ppr1.value = in32(PMC_PARAMETER_REG1);
    return (GlobalPstateTable*)
        (ppr1.fields.ba_sram_pstate_table << GLOBAL_PSTATE_TABLE_ALIGNMENT);
}


/// Quiesce the GPSM to firmware mode from any other mode
///
/// At the exit of this procedure, the PMC will be in Firmware Pstate Mode and
/// there will be no ongoing voltage or frequency transitions.
int
gpsm_quiesce(void)
{
    int rc;

    if (gpsm_hw_mode_p()) {
        rc = gpsm_hw_quiesce();
    } else {
        rc = gpsm_fw_quiesce();
    }

    return rc;
}

/// Quiesce the GPSM in Hardware Pstate Mode
///
/// In general there is no guarantee that the GPSM will ever quiesce, or
/// remain quiesced in Hardware Pstate Mode unless something like the
/// procedure in gpsm_hw_quiesce() is used.  This procedure is provided for
/// the benefit of applications that are in complete control of Pstates
/// (including idle state Pstates) to simply wait for the Pstate protocol to
/// quiesce, without quiescing and entering Firmware Pstate mode like
/// gpsm_hw_quiesce(). 

int
_gpsm_hw_quiesce(void)
{
    int rc;

    do {
        rc = 0;

        if (!gpsm_hw_mode_p()) {
            rc = -GPSM_ILLEGAL_MODE_HW_QUIESCE;
            break;
        }

        if (!ssx_irq_status_get(PGP_IRQ_PMC_PROTOCOL_ONGOING)) {
            ssx_irq_enable(PGP_IRQ_PMC_PROTOCOL_ONGOING);
            rc = ssx_semaphore_pend(&G_gpsm_protocol_semaphore, 
                                    SSX_WAIT_FOREVER);
            if (rc) break;
        }
    } while (0);
    
    return rc;
}


/// Change to GPSM firmware mode from any mode

int
gpsm_fw_mode(void)
{
    return gpsm_quiesce();
}


/// Change to GPSM firmware auction mode from any mode

int
gpsm_fw_auction_mode(void)
{
    int rc;

    rc = gpsm_quiesce();
    if (!rc) {
        _gpsm_fw_auction_mode();
    }
    return rc;
}


/// Change to Hardware Pstate Mode
///
/// The (unchecked) assumption behind this procedure is that the caller has
/// run through Pstate intialization and enablement, and the system is in a
/// state where the entry to Hardware Pstate Mode is safe once the GPSM is
/// quiesced. 

int
gpsm_hw_mode(void)
{
    int rc;

    TRACE_GPSM(TRACE_GPSM_HW_MODE);

    rc = gpsm_quiesce();
    if (!rc) {
        _gpsm_hw_mode();
    }
    return rc;
}


/// The default GPSM auction procedure
///
/// The default auction returns the value of
/// PMC_HARDWARE_AUCTION_PSTATE_REG.haps.

Pstate
gpsm_default_auction(void)
{
    pmc_hardware_auction_pstate_reg_t phapr;

    phapr.value = in32(PMC_HARDWARE_AUCTION_PSTATE_REG);
    return phapr.fields.haps;
}


/// Update a user-supplied vector of Pstates with the current Global bid of
/// each core.
///
/// \param[out] o_bids A vector of Pstates; The vector must be large enough to
/// hold the bid of every possible core.
///
/// This routine is provided for use by non-default Global Pstate auction
/// procedures.

void
gpsm_get_global_bids(Pstate *o_bids)
{
    // This takes advantage of the implicit layout of the
    // PMC_CORE_PSTATE_REG<n>.

    uint32_t *bids32 = (uint32_t *)o_bids;

    bids32[0] = in32(PMC_CORE_PSTATE_REG0);
    bids32[1] = in32(PMC_CORE_PSTATE_REG1);
    bids32[2] = in32(PMC_CORE_PSTATE_REG2);
    bids32[3] = in32(PMC_CORE_PSTATE_REG3);
}


/// Update a current Global bid of each core from a user supplied vector.
///
/// \param[in] i_bids An array of Global Pstate bids.
///
/// This routine is provided for use by test procedures; there is likely no
/// product-level energy management application for this procedure.

void
gpsm_set_global_bids(const Pstate *i_bids)
{
    // This takes advantage of the implicit layout of the
    // PMC_CORE_PSTATE_REG<n>.

    uint32_t *bids32 = (uint32_t *)i_bids;

    out32(PMC_CORE_PSTATE_REG0, bids32[0]);
    out32(PMC_CORE_PSTATE_REG1, bids32[1]);
    out32(PMC_CORE_PSTATE_REG2, bids32[2]);
    out32(PMC_CORE_PSTATE_REG3, bids32[3]);
}

    
/// Application-controlled Global Actual Broadcast
///
/// \param[in] i_pstate The Global Actual Pstate to broadcast
///
/// \param[in] i_entry A gpst_entry_t containing the information to be used by
/// the iVRM. If iVRM are not enabled then \a entry can be initialized to 0.
///
/// This API is provided for advanced applications to have complete control
/// over a firmware-mode Global Actual broadcast.  There is no error
/// checking. Most applications in Firware Pstate mode will use the
/// higher-level gpsm_broadcast_global_actual() API.

void
_gpsm_broadcast_global_actual(const Pstate i_pstate, 
                              const gpst_entry_t i_entry)
{
    pmc_pstate_monitor_and_ctrl_reg_t ppmacr;
    pmc_eff_global_actual_voltage_reg_t pegavr;

    ppmacr.value = 0;
    ppmacr.fields.gpsa = i_pstate;
    out32(PMC_PSTATE_MONITOR_AND_CTRL_REG, ppmacr.value);

    pegavr.value = 0;
    pegavr.fields.maxreg_vdd = i_entry.fields.maxreg_vdd;
    pegavr.fields.maxreg_vcs = i_entry.fields.maxreg_vcs;
    pegavr.fields.eff_evid_vdd = i_entry.fields.evid_vdd_eff; 
    pegavr.fields.eff_evid_vcs = i_entry.fields.evid_vcs_eff; 
    out32(PMC_EFF_GLOBAL_ACTUAL_VOLTAGE_REG, pegavr.value);

    TRACE_GPSM(TRACE_GPSM_BROADCAST_GLOBAL_ACTUAL);
}


/// Broadcast the Global Actual Pstate in firmware Pstate mode.
///
/// \param[in] i_gpst An initialized GlobalPstateTable structure used to
/// define the legal Pstate range, and to provide the voltage settings
/// (maxreg_vxx and eff_evid_vxx) for the internal VRM.
///
/// \param[in] i_pstate The pstate specfiying the Global Actual Pstate to be
/// broadast to the core chiplets.
///
/// \param[in] i_bias This is a signed bias used to obtain the voltage Pstate,
/// <em> in addition to the \a undervolting_bias already built into the Pstate
/// table </em>. The iVRM information sent with the Global Actual Pstate comes
/// from the \a pstate - \a undervolting_bias + \a bias entry of the Pstate
/// table.
///
/// This API can be used in firware Pstate mode to broadcast a Global Actual
/// Pstate and iVRM settings to the core chiplets. The API also supports
/// optional under/over-volting.  The requested Pstate will be broadcast along
/// with the voltage information from the associated Pstate table entry. 
///
/// Under/over-volting is specified by setting the \a bias to a non-0
/// (signed) value.  For example, to undervfolt by one Pstate (if possible),
/// call the API with \a bias = -1.
///
/// This API always waits for the Global Pstate Machine to quiesce before
/// proceeding with the Global Actual broadcast. Therefore it can only be
/// called from thread mode, or from a non-thread mode guaranteed by the
/// caller to have quiesced.
///
/// \note The application can use the _gpsm_broadcast_global_actual() API for
/// complete control over the information transmitted to the cores.
///
/// The following return codes are not considered errors:
///
/// \retval 0 Success
///
/// \retval -GPST_PSTATE_CLIPPED_HIGH_GPSM_BGA The requested Pstate does not 
/// exist in the table. The maximum Pstate entry in the table has been 
/// broadcast as the voltage Pstate.
///
/// \retval -GPST_PSTATE_CLIPPED_LOW_GPSM_BGA The requested Pstate does not 
/// exist in the table. The minimum Pstate entry in the table has been 
/// broadcast as the voltage Pstate.
///
/// The following return codes are considered errors:
///
/// \retval -GPSM_INVALID_OBJECT The Global Pstate Table is either null (0) or
/// otherwise invalid.
///
/// \retval -GPSM_INVALID_ARGUMENT One or more arguments are invalid or
/// inconsistent in some way.
///
/// \retval -GPSM_ILLEGAL_MODE_BGA The PMC is not in firmware pstate mode.
///
/// This API may also return errors from the SSX semaphore operations that
/// implement the wait for quiescence.

int
gpsm_broadcast_global_actual(const GlobalPstateTable* i_gpst, 
                             const Pstate i_pstate, 
                             const int i_bias)
{
    int rc, bias_rc, entry_rc;
    gpst_entry_t entry;
    Pstate voltage_pstate;

    do {

        if (!gpsm_fw_mode_p()) {
            rc = GPSM_ILLEGAL_MODE_BGA;
            break;
        }

        // Bias the pstate, fetch the Pstate entry, quiesce and broadcast.
        // bias_pstate() only returns saturation warnings. These are turned
        // into bounds warnings if necessary (indicating that the Pstate
        // saturated but the PMAX or PMIN was also a legal entry in the
        // table).

        bias_rc = bias_pstate(i_pstate, i_bias, &voltage_pstate);
        entry_rc = gpst_entry(i_gpst, voltage_pstate, 0, &entry);
        if (entry_rc && 
            (entry_rc != -GPST_PSTATE_CLIPPED_LOW_GPST_ENTRY) &&
            (entry_rc != -GPST_PSTATE_CLIPPED_HIGH_GPST_ENTRY)) {
            rc = entry_rc;
            break;
        }

        rc = gpsm_quiesce();
        if (rc) break;

        _gpsm_broadcast_global_actual(i_pstate, entry);

        if (entry_rc != 0) {
            rc = entry_rc;
        } else if (bias_rc == -PSTATE_OVERFLOW_BIAS_PS) {
            rc = -GPST_PSTATE_CLIPPED_HIGH_GPSM_BGA;
        } else if (bias_rc == -PSTATE_UNDERFLOW_BIAS_PS) {
            rc = -GPST_PSTATE_CLIPPED_LOW_GPSM_BGA;
        }
    } while (0);

    return rc;
}


