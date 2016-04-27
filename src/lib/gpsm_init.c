/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/gpsm_init.c $                                         */
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
/// \file gpsm_init.c
/// \brief Global Pstate Machine procedures only required to initialize Pstate
/// and Pstate modes.
///
/// In OCC product firmware this code is only needed immediately after IPL to
/// set up Pstate tables and enable Pstates.  This code could be run from an
/// applet or otherwise removed once Pstates are initialized. Note that this
/// file cointains code only; some global variables referenced here are
/// defined in gpsm.c, along with the run-time APIs for GPSM.
///
/// The following sequence of procedures is required to initialize the GPSM
/// mechanism, enable Pstate mode, and further enable Hardware Pstate Mode.
///
/// \code
///
/// PstateSuperStructure* pss;
/// GlobalPstateTable *gpst;
/// GpsmEnablePstatesMasterInfo info;
/// Pstate voltage_pstate, frequency_pstate;
///
/// gpsm_initialize(pss, gpst);
/// gpsm_enable_pstates_master(&info, &voltage_pstate, &frequency_pstate);
/// gpsm_enable_pstates_slave(&info, voltage_pstate, frequency_pstate);
/// gpsm_hw_mode();
///
/// \endcode
///
/// Executing these procedures enables Pstate control of voltage and
/// frequency, and leaves the chip quiesced in Firmware Pstate mode. These
/// procedures initializes both central (PMC) and remote (PCB Slave) Pstate
/// hardware to fully enable Pstate control of voltage and frequency.
/// Carefully note the procedure sequence preconditions and postconditions.
///
/// In the case of a DCM environment, the DCM master must execute the sequence
/// as follows:
///
/// \code
///
/// PstateSuperStructure* pss;
/// GlobalPstateTable *gpst;
/// GpsmEnablePstatesMasterInfo info;
/// Pstate voltage_pstate, frequency_pstate;
///
/// gpsm_initialize(pss, gpst);
/// gpsm_enable_pstates_master(&info, &voltage_pstate, &frequency_pstate);
///
/// <em> Send voltage_pstate and frequency_pstate to the slave and wait for
/// confirmation that the procedure has completed. </em>
///
/// gpsm_enable_pstates_slave(&info, voltage_pstate, frequency_pstate);
/// gpsm_hw_mode();
///
/// <em> Send command to the slave to execute gpsm_hw_mode() </em>
///
/// \endcode
///
/// The DCM slave executes the following sequence
/// \code
/// PstateSuperStructure* pss;
/// GlobalPstateTable *gpst;
/// Pstate voltage_pstate, frequency_pstate;
///
/// gpsm_initialize(pss, gpst);
///
/// <em> Receive voltage_pstate and frequency_pstate from the masterand wait for
/// confirmation that the procedure has completed. </em>
///
/// gpsm_enable_pstates_slave(0, voltage_pstate, frequency_pstate);
///
/// <em> Wait for a command from the master to execute gpsm_hw_mode(). </em>
///
/// \endcode
///
/// <b> Preconditions </b>
///
/// - This sequence must be called from a thread as it must be able to block
/// for the completion of GPSM events.
///
/// - The fundamental precondition is the assumption that any snapshot of the
/// voltage/frequency state of the system yields a legal state. As long as
/// this is true, then this sequence should never take the system to an
/// illegal V/F state.
///
/// - CPM-DPLL mode should be disabled, and all undervolting controls are
/// assumed to be at their nominal values.  Correctness can not guaranteed
/// otherwise.
///
/// - iVRM mode should be disabled prior to calling this procedure.
/// Correctness can not be guaranteed otherwise.
///
///
/// <b> Standard/Benign Postconditions after executing
/// gpsm_enable_pstates_slave() </b>
///
/// - The system will be in Firmware Pstate Mode, using the Local and Global
/// Pstate tables installed by gpsm_initialize().  Pstate 0 will be mapped to
/// the nominal frequency (modulo rounding down) of the
/// 'nominal_frequency_khz' field of the Global Pstate table.
///
/// - All core chiplet frequencies will be under the control of the PMCR Local
/// Pstate.
///
/// - The Global Actual Pstate immediately prior to the final entry of Pstate
/// mode is the Pstate that most closely matches an arbitrary snapshot of the
/// system voltage during the execution of this procedure.
///
/// - The PMCR and PMICR are not modified, therefore the Global Pstate
/// subsequent to the release of Hardware Pstate mode is arbitrary.
///
/// - The PMC_RAIL_BOUNDS register is set to the maximum legal bounds allowed
/// by Global Pstate Table
///
/// - IVRM is setup and enabled if Local Pstate Table is installed.
///
/// - Resonant Clock is setup and enabled if Resonant Clock is installed.
///
/// <b> Side-Effects </b>
///
/// - The dpll_fmax and dpll_fmin fields of the FREQ_CTRL_REG in each core are
/// set to an arbitrary value. The dpll_fmax_bias is set to the value of the
/// 'dpll_fmax_bias' field of the \a gpst.
///
/// - The core-level PCBS_POWER_MANAGEMENT_BOUNDS_REG registers are
/// set to the maximum legal bounds.  This is required due to Pstate table
/// constraints.
///
/// \todo - How to handle redundant SPIVID interfaces?  Here we get the
/// current volatge from interface 0.
///
/// \bug Check to make sure that the PMC_CORE_DECONFIGURATION_REG matches
/// multicast group 1.
///
/// \bug Code marked with ** VBU ** is necessary for VBU/EPO simulation, needs
/// to be scrubbed once this is working in VBU.

#include "ssx.h"
#include "ssx_io.h"
#include "gpsm.h"
#include "vrm.h"
#include "heartbeat.h"
#include "special_wakeup.h"

// Debugging support

#if 0
#define _BREAK                                                          \
    {                                                                   \
        fprintf(stderr, "%s:%d: _BREAK trapped error rc = 0x%08x (-0x%08x)\n", \
                __FILE__, __LINE__, rc, -rc);                           \
        SSX_PANIC(GPSM_ERROR_BREAK);                                    \
    }
#else
#define _BREAK break;
#endif


/// Flag set once gpsm_initialize() has been successfully completed.
uint8_t G_gpsm_initialized = 0;


/// Install a (new) Global Pstate Table
///
/// \param[out] o_gpst A pointer to a properly-aligned GlobalPstateTable
///
/// \param[in] i_source A pointer to an initialized source GlobalPstateTable
/// that will be copied to \a gpst (if not in fact the same as \a gpst).
///
/// This procedure will likely only be called once, at initialization, and
/// then only as part of the gpsm_initialize() procedure.  The procedure:
///
/// - Copies the \a source to the \a gpst if required.
///
/// - Installs a pointer to the Global Pstate Table in the PMC hardware.
///
/// - Sets up the PMC Pstate clipping bounds.
///
/// - Sets up the Pstate stepping parameters from the GlobalPstateTable
///
/// - Clears the local undervolting register (via multicast) and sets the
/// default undervolting bounds to the entire Local Pstate Table.
///
/// - Broadcasts the safe mode Pstate Psafe to the cores and clears the other
/// fields of the PCBS_OCC_HEARTBEAT_REG, disabling the heartbeat timer.
/// However the heartbeat timer must not have been active anyway.
///
/// \note This procedure does modify the rail bounds.
///
/// \note The caller is responsible for the mode-correctness of this
/// procedure.  This procedure must only be called when the PMC is in Firmware
/// Pstate Mode.
///
/// \retval 0 Success
///
/// \retval -GPSM_INVALID_ARGUMENT_GPST_INSTALL The Global Pstate table argument
/// was either NULL (0) or improperly aligned, or the \a source was NULL (0).
///
/// \retval -GPSM_ILLEGAL_MODE_GPST_INSTALL The PMC does not indicate that the
//  system is in Firmware Pstate Mode, or the heartbeat is enabled.

int
gpsm_gpst_install(GlobalPstateTable* o_gpst,
                  const GlobalPstateTable* i_source)
{
    pmc_occ_heartbeat_reg_t pohr;
    pmc_parameter_reg0_t ppr0;
    pmc_parameter_reg1_t ppr1;
    pmc_global_pstate_bounds_reg_t pgpbr;
    pmc_rail_bounds_register_t prbr;
    pmc_undervolting_reg_t pur;
    pcbs_occ_heartbeat_reg_t pcbsohr;

    int rc;

    TRACE_GPSM(TRACE_GPSM_GPST_INSTALL);

    do {

        // Optional bypass of the procedure

        if (i_source->options.options & PSTATE_NO_INSTALL_GPST) {

            rc = 0;
            break;
        }

        // Check presence and alignment of the Pstate table, and proper Pstate
        // and heartbeat modes.

        if ((o_gpst == 0) ||
            ((unsigned long)o_gpst % POW2_32(GLOBAL_PSTATE_TABLE_ALIGNMENT))) {
            rc = -GPSM_INVALID_ARGUMENT_GPST_INSTALL;
            _BREAK;
        }

        pohr.value = in32(PMC_OCC_HEARTBEAT_REG);
        rc = getscom(MC_ADDRESS(PCBS_OCC_HEARTBEAT_REG,
                                MC_GROUP_EX, PCB_MULTICAST_OR),
                     &(pcbsohr.value));
        if (rc) _BREAK;

        if (!gpsm_fw_mode_p() || pohr.fields.pmc_occ_heartbeat_en
            || pcbsohr.fields.occ_heartbeat_enable) {
            rc = -GPSM_ILLEGAL_MODE_GPST_INSTALL;
            _BREAK;
        }


        // Copy \a source to \a gpst if required, then install the Pstate
        // table, Pvsafe and Pstate stepping parameters, and set the clipping
        // bounds as well as the rail bounds

        if ((o_gpst != i_source) &&
            !(i_source->options.options & PSTATE_NO_COPY_GPST)) {

            memcpy(o_gpst, i_source, sizeof(*o_gpst));
        }

        ppr1.value = in32(PMC_PARAMETER_REG1);
        ppr1.fields.ba_sram_pstate_table =
            (unsigned long)o_gpst >> GLOBAL_PSTATE_TABLE_ALIGNMENT;
        ppr1.fields.pvsafe = i_source->pvsafe;

        // This fix is added per SW260911
        // Minimum Frequency in the system is given by MRW attribute
        // PState Datablock procedure will read the attribute then
        // convert it into pstate _pfloor_ and put it into
        // Global Pstate Table. GPSM here consumes the value
        // and set both lower bounds: pmin_rail(PMC) and pmin_clip(PCBS)
        // and two safe pstates: pvsafe(PMc) and psafe(PCBS) to be
        // _pfloor_ if _pfloor_ is higher than their default(gpst_pmin)
        // so that we should never run with frequency below the floor
        // even in safe mode
        if (ppr1.fields.pvsafe < i_source->pfloor && i_source->pfloor != 0)
            ppr1.fields.pvsafe = i_source->pfloor;

        out32(PMC_PARAMETER_REG1, ppr1.value);

        pgpbr.value = 0;
        pgpbr.fields.gpsi_min = gpst_pmin(i_source) - PSTATE_MIN;
        pgpbr.fields.gpst_number_of_entries_minus_one = i_source->entries - 1;
        out32(PMC_GLOBAL_PSTATE_BOUNDS_REG, pgpbr.value);

        ppr0.value = in32(PMC_PARAMETER_REG0);
        ppr0.fields.pstate_stepsize = i_source->pstate_stepsize;
        ppr0.fields.vrm_stepdelay_range = i_source->vrm_stepdelay_range;
        ppr0.fields.vrm_stepdelay_value = i_source->vrm_stepdelay_value;
        ppr0.fields.gpsa_timeout_value_sel = 1;
        out32(PMC_PARAMETER_REG0, ppr0.value);

        prbr.value = 0;
        prbr.fields.pmin_rail = gpst_pmin(i_source)+1;
        prbr.fields.pmax_rail = gpst_pmax(i_source);

        // This fix is added per SW260911
        // Minimum Frequency in the system is given by MRW attribute
        // PState Datablock procedure will read the attribute then
        // convert it into pstate _pfloor_ and put it into
        // Global Pstate Table. GPSM here consumes the value
        // and set both lower bounds: pmin_rail(PMC) and pmin_clip(PCBS)
        // and two safe pstates: pvsafe(PMc) and psafe(PCBS) to be
        // _pfloor_ if _pfloor_ is higher than their default(gpst_pmin)
        // so that we should never run with frequency below the floor
        // even in safe mode
        if (prbr.fields.pmin_rail < i_source->pfloor && i_source->pfloor != 0)
            prbr.fields.pmin_rail = i_source->pfloor;

        out32(PMC_RAIL_BOUNDS_REGISTER, prbr.value);

        // Clear the undervolting control, and set the undervolting range to
        // the entire Global Pstate Table range.

        pur.value = 0;
        pur.fields.puv_min = gpst_pmin(i_source);
        pur.fields.puv_max = gpst_pmax(i_source);
        pur.fields.kuv_request = 0;
        out32(PMC_UNDERVOLTING_REG, pur.value);


        // Broadcast the safe mode Pstate Psafe to the cores, disabling the
        // heartbeat (which must have been disabled anyway) and clearing any
        // other heartbeat setup.

        pcbsohr.value = 0;
        pcbsohr.fields.psafe = i_source->psafe;

        // This fix is added per SW260911
        // Minimum Frequency in the system is given by MRW attribute
        // PState Datablock procedure will read the attribute then
        // convert it into pstate _pfloor_ and put it into
        // Global Pstate Table. GPSM here consumes the value
        // and set both lower bounds: pmin_rail(PMC) and pmin_clip(PCBS)
        // and two safe pstates: pvsafe(PMc) and psafe(PCBS) to be
        // _pfloor_ if _pfloor_ is higher than their default(gpst_pmin)
        // so that we should never run with frequency below the floor
        // even in safe mode
        if (pcbsohr.fields.psafe < i_source->pfloor && i_source->pfloor != 0)
            pcbsohr.fields.psafe = i_source->pfloor;

        rc = putscom(MC_ADDRESS(PCBS_OCC_HEARTBEAT_REG,
                                MC_GROUP_EX, PCB_MULTICAST_WRITE),
                     pcbsohr.value);
        if (rc) _BREAK;

        rc = 0;

    } while(0);

    return rc;
}


/// Install a (new) Local Pstate Array
///
/// \param[in] i_lpsa A pointer to a LocalPstateArray to install in every
/// configured core.
///
/// \param[in] i_options Options controlling the installation, or a NULL (0)
/// pointer to indicate fully default behavior.
///
/// This procedure will likely only be called once, at initialization, and
/// then only as part of the gpsm_initialize() procedure.  The procedure:
///
/// - Power on PFET Voltage Reference Circuit
///
/// - Perform the binary search for IVRM Calibration
///
/// - Uploads the LocalPstateArray to every core using multicast.
///
/// - Sets up the Local Pstate table bounds in every core using multicast.
///
/// - Sets the step delay parameters for every core using multicast.
///
/// - Clears the local undervolting register (via multicast) and sets the
/// default undervolting bounds to the entire Local Pstate Table.
///
/// - Setup IVRM delay parameters
///
/// - Enable IVRM
///
/// \note This procedure \e does \e not modify the rail bounds.
///
/// \note The caller is responsible for the mode-correctness of this
/// procedure.  This procedure must only be called when the iVRM are
/// disabled, the DPLL is in "normal" (not CPM-DPLL) mode, and core heartbeats
/// are disabled.
///
/// \retval 0 Success
///
/// \retval -GPSM_INVALID_ARGUMENT_LPST_INSTALL The Local Pstate array argument
/// was NULL (0).
///
/// \retval -GPSM_ILLEGAL_MODE_LPST_INSTALL iVRM mode, CPM-DPLL mode, or the
/// local heartbeat appears to be enabled in at least one core.
///
/// \retval -GPSM_IVRM_CALIBRATION_TIMEOUT, if IVRM Calibration does not
/// complete in time.
///
/// \retval -GPSM_IVRM_GROSS_OR_FINE, if ivrm_gross_or_fine_err is set
///
/// \retval -GPSM_PSTATE_ENABLED, if pstate is enabled before enabling IVRM
///
/// \retval others This API may also return non-0 codes from
/// getscom()/putscom()

int
gpsm_lpsa_install(const LocalPstateArray* i_lpsa,
                  const PstateOptions* i_options)
{
    pcbs_ivrm_control_status_reg_t picsr;
    pcbs_dpll_cpm_parm_reg_t pdcpr;
    pcbs_pstate_index_bound_reg_t ppibr;
    pcbs_ivrm_vid_control_reg0_t pivcr0;
    pcbs_ivrm_vid_control_reg1_t pivcr1;
    pcbs_undervolting_reg_t pur;
    pcbs_pmerr_reg_t ppr;
    pcbs_pcbspm_mode_reg_t ppmr;
    pmc_core_deconfiguration_reg_t pcdr;
    ChipConfigCores cores, wakedup;
    SsxTimebase timeout;
    int i, rc, timeout_rc = 0;
    uint32_t configured_cores;
    int flag, core;

    TRACE_GPSM(TRACE_GPSM_LPSA_INSTALL);

    do {

        // Optional bypass of this procedure

        if ((i_options != 0) &&
            (i_options->options & PSTATE_NO_INSTALL_LPSA)) {

            rc = 0;
            break;
        }

        // No LPST Install and IVRM Enable if there is no configued cores

        configured_cores = ~in32(PMC_CORE_DECONFIGURATION_REG);
        flag = 1;
        for (core = 0; core < PGP_NCORES; core++, configured_cores <<= 1) {
            if (!(configured_cores & 0x80000000)) continue;
            flag = 0;
        }
        if (flag == 1) {
            rc = 0;
            break;
        }

        // Check the array for existence.  Do an OR-combining multicast read
        // to see if any of the cores have iVRM enabled, have the heartbeat
        // enabled, or any cores are running in CPM-DPLL mode.

        if (i_lpsa == 0) {
            rc = -GPSM_INVALID_ARGUMENT_LPST_INSTALL;
            _BREAK;
        }
        rc = getscom(MC_ADDRESS(PCBS_IVRM_CONTROL_STATUS_REG,
                                MC_GROUP_EX, PCB_MULTICAST_OR),
                     &(picsr.value));
        if (rc) _BREAK;
        rc = getscom(MC_ADDRESS(PCBS_DPLL_CPM_PARM_REG,
                                MC_GROUP_EX, PCB_MULTICAST_OR),
                     &(pdcpr.value));
        if (rc) _BREAK;

        if (picsr.fields.ivrm_fsm_enable ||
            pdcpr.fields.cpm_filter_enable) {
            rc = -GPSM_ILLEGAL_MODE_LPST_INSTALL;
            _BREAK;
        }

        // In case cores are in deep winkle so that ivrm caliburation
        // will fail, insert special wakeup first
        pcdr.value = in32(PMC_CORE_DECONFIGURATION_REG);
        cores = ~pcdr.fields.core_chiplet_deconf_vector;
        rc = occ_special_wakeup(1, cores, 25, &wakedup);
        if (rc) _BREAK;

        // Power on PFET Voltage Reference Circuit
        picsr.fields.pvref_en = 1;
        rc = putscom(MC_ADDRESS(PCBS_IVRM_CONTROL_STATUS_REG,
                                MC_GROUP_EX, PCB_MULTICAST_WRITE),
                     picsr.value);
        if (rc) _BREAK;

        // Wait 10us for circuit to power on
        timeout = ssx_timebase_get() + SSX_MICROSECONDS(10);
        while (ssx_timebase_get() < timeout) {;}

        // Perform the binary search
        picsr.fields.binsearch_cal_ena = 1;
        rc = putscom(MC_ADDRESS(PCBS_IVRM_CONTROL_STATUS_REG,
                                MC_GROUP_EX, PCB_MULTICAST_WRITE),
                     picsr.value);
        if (rc) _BREAK;

        // Check IVRM Calibration is completed
        // Poll for up to 100us for done before erroring out
        timeout_rc = -GPSM_IVRM_CALIBRATION_TIMEOUT;
        timeout = ssx_timebase_get() + SSX_MICROSECONDS(100);
        while (ssx_timebase_get() < timeout) {
            rc = getscom(MC_ADDRESS(PCBS_IVRM_CONTROL_STATUS_REG,
                                    MC_GROUP_EX, PCB_MULTICAST_AND),
                         &(picsr.value));
            if (rc) _BREAK;
            if (picsr.fields.binsearch_cal_done) {
                timeout_rc=0;
                break;
            }
        }
        if (timeout_rc||rc) _BREAK;

        // IVRM Calibration complete, Clear binary search enable
        picsr.fields.binsearch_cal_ena = 0;
        rc = putscom(MC_ADDRESS(PCBS_IVRM_CONTROL_STATUS_REG,
                                MC_GROUP_EX, PCB_MULTICAST_WRITE),
                     picsr.value);
        if (rc) _BREAK;

        // Check if IVRM Gross or Fine Error is set after calibration!
        rc = getscom(MC_ADDRESS(PCBS_PMERR_REG,
                                MC_GROUP_EX, PCB_MULTICAST_OR),
                     &(ppr.value));
        if (rc) _BREAK;
        if (ppr.fields.pcbs_ivrm_gross_or_fine_err) {
            rc = -GPSM_IVRM_GROSS_OR_FINE;
            _BREAK;
        }

        // Deassert Special Wakeup
        rc = occ_special_wakeup(0, cores, 25, &wakedup);
        if (rc) _BREAK;

        // Upload the Local Pstate Array.  The array is loaded via multicast,
        // using the built-in auto-increment mechanism.  Then upload the
        // Pstate bounds register via multicast. Pstate clipping is not
        // modified.

        rc = putscom(MC_ADDRESS(PCBS_PSTATE_TABLE_CTRL_REG,
                                MC_GROUP_EX, PCB_MULTICAST_WRITE),
                     0);
        if (rc) _BREAK;

        for (i = 0; i < LOCAL_PSTATE_ARRAY_ENTRIES+VDSVIN_ARRAY_ENTRIES; i++) {

            if (i < LOCAL_PSTATE_ARRAY_ENTRIES) {
                rc = putscom(MC_ADDRESS(PCBS_PSTATE_TABLE_REG,
                                        MC_GROUP_EX, PCB_MULTICAST_WRITE),
                i_lpsa->pstate[i].value);
                if (rc) _BREAK;
            } else {
                rc = putscom(MC_ADDRESS(PCBS_PSTATE_TABLE_REG,
                                        MC_GROUP_EX, PCB_MULTICAST_WRITE),
                i_lpsa->vdsvin[i-LOCAL_PSTATE_ARRAY_ENTRIES].value);
                if (rc) _BREAK;
            }

        }

        ppibr.value = 0;
        ppibr.fields.lpsi_min = lpst_pmin(i_lpsa) - PSTATE_MIN;
        ppibr.fields.lpsi_entries_minus_1 = i_lpsa->entries - 1;
        rc = putscom(MC_ADDRESS(PCBS_PSTATE_INDEX_BOUND_REG,
                                MC_GROUP_EX, PCB_MULTICAST_WRITE),
                     ppibr.value);
        if (rc) _BREAK;


        // Install the step delay parameters, then clear the undervolting
        // control (applicable to the entire range) via multicast.

        pivcr0.value = 0;
        if (i_lpsa->stepdelay_rising)
            pivcr0.fields.ivrm_req_pstate_stepdelay_rising =
                                         i_lpsa->stepdelay_rising;
        else
            pivcr0.fields.ivrm_req_pstate_stepdelay_rising = 0xFF;
        if (i_lpsa->stepdelay_lowering)
            pivcr0.fields.ivrm_req_pstate_stepdelay_lowering =
                                         i_lpsa->stepdelay_lowering;
        else
            pivcr0.fields.ivrm_req_pstate_stepdelay_lowering = 0XFF;
        rc = putscom(MC_ADDRESS(PCBS_IVRM_VID_CONTROL_REG0,
                                MC_GROUP_EX, PCB_MULTICAST_WRITE),
                     pivcr0.value);
        if (rc) _BREAK;

        ///bug need to determine where these values come from
        pivcr1.value = 0;
        pivcr1.fields.ivrm_stabilize_delay_run   = 0x40;
        pivcr1.fields.ivrm_stabilize_delay_idle  = 0x40;
        pivcr1.fields.ivrm_pfstr_prop_delay      = 0x1E;
        pivcr1.fields.ivrm_pfstrvalid_prop_delay = 0xFF;
        pivcr1.fields.ivrm_vpump_poweron_time    = 0xFF;
        pivcr1.fields.ivrm_bypass_delay          = 0x1E;
        pivcr1.fields.pfet_vpump_enable_delay    = 0x4E;
        pivcr1.fields.ivrm_vid_vout_threshold    = 0x00;
        rc = putscom(MC_ADDRESS(PCBS_IVRM_VID_CONTROL_REG1,
                                MC_GROUP_EX, PCB_MULTICAST_WRITE),
                     pivcr1.value);
        if (rc) _BREAK;

        pur.value = 0;
        pur.fields.puv_min = lpst_pmin(i_lpsa);
        pur.fields.puv_max = lpst_pmax(i_lpsa);
        pur.fields.kuv = 0;
        rc = putscom(MC_ADDRESS(PCBS_UNDERVOLTING_REG,
                                MC_GROUP_EX, PCB_MULTICAST_WRITE),
                     pur.value);
        if (rc) _BREAK;

        // Set pre_vret_pstate to Non-Functional Pstate
        // \bug currently set to pmin, is it always same as psafe?
        rc = getscom(MC_ADDRESS(PCBS_DPLL_CPM_PARM_REG,
                                MC_GROUP_EX, PCB_MULTICAST_OR),
                     &(pdcpr.value));
        if (rc) _BREAK;
        pdcpr.fields.pre_vret_pstate = lpst_pmin(i_lpsa);
        rc = putscom(MC_ADDRESS(PCBS_DPLL_CPM_PARM_REG,
                                MC_GROUP_EX, PCB_MULTICAST_WRITE),
                     pdcpr.value);
        if (rc) _BREAK;

        // Checking that PStates are NOT enabled
        rc = getscom(MC_ADDRESS(PCBS_PCBSPM_MODE_REG,
                                MC_GROUP_EX, PCB_MULTICAST_OR),
                     &(ppmr.value));
        if (rc) _BREAK;
        if (ppmr.fields.enable_pstate_mode) {
            rc = -GPSM_PSTATE_ENABLED;
            _BREAK;
        }

        // Enable I-VRM FSM
        rc = getscom(MC_ADDRESS(PCBS_IVRM_CONTROL_STATUS_REG,
                                MC_GROUP_EX, PCB_MULTICAST_OR),
                     &(picsr.value));
        if (rc) _BREAK;
        picsr.fields.ivrm_fsm_enable = 1;
        rc = putscom(MC_ADDRESS(PCBS_IVRM_CONTROL_STATUS_REG,
                                MC_GROUP_EX, PCB_MULTICAST_WRITE),
                     picsr.value);
        if (rc) _BREAK;

    } while (0);

    if (timeout_rc && !rc)
        return timeout_rc;
    else
        return rc;
}


/// Install (new) Resonant Clocking Setup
///
/// \param[in] i_resclk A pointer to a ResonantClockingSetup to install in
/// every configured core.
///
/// \param[in] i_options Options controlling the installation, or a NULL (0)
/// pointer to indicate fully default behavior.
///
/// This procedure will likely only be called once, at initialization, and
/// then only as part of the gpsm_initialize() procedure.  The procedure:
///
/// - Initializes the Pstate resonance range limits in the register
/// PCBS_RESONANT_CLOCK_CONTROL_REG1 by multicast.
///
/// - Setup parameters in PCBS_RESONANT_CLOCK_CONTROL_REG0 and turn on
/// Resonant Clock in Hardware Pstate Mode.
///
/// \note The caller is responsible for the mode-correctness of this
/// procedure.  This procedure must only be called when resonant clocking is
/// disabled and the controls are set for manual mode. Because of the way the
/// resonant clocking controls are designed we must enable resonant clocking
/// to update the Pstate bounds! After the bounds are updated resonant
/// clocking is disabled again.
///
/// \retval 0 Success
///
/// \retval -GPSM_INVALID_ARGUMENT_RCLK_INSTALL The ResonantClockingSetup
/// argument was NULL (0).
///
/// \retval -GPSM_ILLEGAL_MODE_RCLK_INSTALL Resonant clocking appears to be
/// enabled or not in manual mode in at least one configured core.
///
/// \retval others This API may also return non-0 codes from
/// getscom()/putscom()

int
gpsm_resclk_install(const ResonantClockingSetup* i_resclk,
                    const GlobalPstateTable* i_gpst,
                    const PstateOptions* i_options)
{
    ResonantClockingSetup d_resclk;
    pcbs_resonant_clock_control_reg0_t prccr0;
    pcbs_resonant_clock_control_reg1_t prccr1;
    int rc;
    uint32_t configured_cores;
    int flag, core;

    TRACE_GPSM(TRACE_GPSM_RESCLK_INSTALL);

    do {

        // Optional bypass of this procedure

        if ((i_options != 0) &&
            (i_options->options & PSTATE_NO_INSTALL_RESCLK)) {

            rc = 0;
            break;
        }

        // No Resonant Clock Install and Enable if there is no configued cores

        configured_cores = ~in32(PMC_CORE_DECONFIGURATION_REG);
        flag = 1;
        for (core = 0; core < PGP_NCORES; core++, configured_cores <<= 1) {
            if (!(configured_cores & 0x80000000)) continue;
            flag = 0;
        }
        if (flag == 1) {
            rc = 0;
            break;
        }

        // Check the setup for existence.  Do an AND-combining multicast read
        // to see if any of the cores have resonant clocking enabled, or are
        // not in manual mode.

        if (i_resclk == 0) {
            rc = -GPSM_INVALID_ARGUMENT_RCLK_INSTALL;
            _BREAK;
        }
        rc = getscom(MC_ADDRESS(PCBS_RESONANT_CLOCK_CONTROL_REG0,
                                MC_GROUP_EX, PCB_MULTICAST_AND),
                     &(prccr0.value));
        if (rc) _BREAK;

        if (!prccr0.fields.resclk_dis || !prccr0.fields.resclk_control_mode) {
            rc = -GPSM_ILLEGAL_MODE_RCLK_INSTALL;
            _BREAK;
        }


        // Resonant clocking is specified such that it must be enabled (in a
        // benign manual mode) in order to be set up.

        // Enable resonant clocking in the GP3 register (AND), bit 22. Our
        // Simics environment does not model the GP3->PRCCR0 connection
        // currently, and does not enforce the register locks.

        if (!SIMICS_ENVIRONMENT) {

            rc = putscom(MC_ADDRESS(0x100f0013, MC_GROUP_EX,
                                    PCB_MULTICAST_WRITE),
                         ~0x0000020000000000ull);
            if (rc) _BREAK;
        }

        // Write the PCBS_RESONANT_CLOCK_CONTROL_REG1 with the
        // Pstate setup, clearing all manual fields.


        // Make some defaults
        gpst_frequency2pstate(i_gpst, 0,       &(d_resclk.full_csb_ps));
        gpst_frequency2pstate(i_gpst, 2000000, &(d_resclk.res_low_lower_ps));
        gpst_frequency2pstate(i_gpst, 3200000, &(d_resclk.res_low_upper_ps));
        gpst_frequency2pstate(i_gpst, 3200000, &(d_resclk.res_high_lower_ps));
        gpst_frequency2pstate(i_gpst, 9999999, &(d_resclk.res_high_upper_ps));

        prccr1.value = 0;
        
        // SW348603 - The highest Pstate WILL be 0 so this check is not actually
        // valid.       
        // If all resonant clocking parms are 0 in PstateSuperStructure
        // write the register with default values
        // Low Band  : 2 GHZ to 3.2 GHz
        // High Band : 3.2 GHZ - Up
        
        // By implication, if ANY of the fields are not zero indicates that they
        // are all valid.

        if (((i_resclk->full_csb_ps == 0) && 
             (i_resclk->res_low_lower_ps == 0) &&
             (i_resclk->res_low_upper_ps == 0) && 
             (i_resclk->res_high_lower_ps == 0) &&
             (i_resclk->res_high_upper_ps == 0))) {     // SW348603          
            prccr1.fields.full_csb_ps       = d_resclk.full_csb_ps;
            prccr1.fields.res_low_lower_ps  = d_resclk.res_low_lower_ps;
            prccr1.fields.res_low_upper_ps  = d_resclk.res_low_upper_ps;
            prccr1.fields.res_high_lower_ps = d_resclk.res_high_lower_ps;
            prccr1.fields.res_high_upper_ps = d_resclk.res_high_upper_ps;
        } else {
            prccr1.fields.full_csb_ps       = i_resclk->full_csb_ps;
            prccr1.fields.res_low_lower_ps  = i_resclk->res_low_lower_ps;
            prccr1.fields.res_low_upper_ps  = i_resclk->res_low_upper_ps;
            prccr1.fields.res_high_lower_ps = i_resclk->res_high_lower_ps;
            prccr1.fields.res_high_upper_ps = i_resclk->res_high_upper_ps;
        }         


        ///bug need to determine where these values come from
        prccr1.fields.nonres_csb_value_ti  = 0xC;
        prccr1.fields.full_csb_value_ti    = 0xF;

        rc = putscom(MC_ADDRESS(PCBS_RESONANT_CLOCK_CONTROL_REG1,
                                MC_GROUP_EX, PCB_MULTICAST_WRITE),
                     prccr1.value);
        if (rc) _BREAK;


        // Disable resonant clocking in the GP3 register (OR), bit 22.

        if (!SIMICS_ENVIRONMENT) {

            rc = putscom(MC_ADDRESS(0x100f0014, MC_GROUP_EX,
                                    PCB_MULTICAST_WRITE),
                         0x0000020000000000ull);
            if (rc) _BREAK;
        }

        // Enable resonant clock pstate hardware mode(control_mode = 0)
        // Sync Pulse Width maximum value : 0x7  of nest/4 cycles
        // Sync Delay       maximum value : 0x7F of nest/4 cycles
        // Sector Buffer Strength Instruction . Low Band  : 0xAAA
        // Sector Buffer Strength Instruction . High Band : 0xAAA

        prccr0.fields.resclk_control_mode  = 0;
        prccr0.fields.resclk_sync_pw       = 0x7;
        prccr0.fields.res_sync_delay_cnt   = 0x7F;
        prccr0.fields.res_csb_str_instr_lo = 0xAAA;
        prccr0.fields.res_csb_str_instr_hi = 0x1FF;

        rc = putscom(MC_ADDRESS(PCBS_RESONANT_CLOCK_CONTROL_REG0,
                                MC_GROUP_EX, PCB_MULTICAST_WRITE),
                     prccr0.value);
        if (rc) break;

        // Enable resonant clocking in the GP3 register (AND), bit 22.
        if (!SIMICS_ENVIRONMENT) {

            rc = putscom(MC_ADDRESS(0x100f0013, MC_GROUP_EX,
                                PCB_MULTICAST_WRITE),
                     ~0x0000020000000000ull);
            if (rc) break;
        }

    } while (0);

    return rc;
}


/// Initialize the GPSM procedure mechanism
///
/// \param[in] i_pss A pointer to the PstateSuperStructure containing the
/// Global and Local Pstate tables, plus resonant clocking setup and other
/// options.
///
/// \param[out] o_gpst A pointer to a 1-KB aligned GlobalPstateTable which
/// will be updated with a copy of the GlobalPstateTable from the
/// PstateSuperStructure.
///
/// This API is designed to be called once at system initialization, to set up
/// GPSM mechanisms, install the Global and Local Pstate tables, and set up
/// resonant clocking from the PstateSuperStructure.  At the entry of this
/// procedure it is assumed that the system firmware has initialized the PMC
/// mode register to either no mode, or to indicate Firmware Pstate Mode.  It
/// is further assumed that the core chiplets are in a state which will allow
/// the Local Pstate tables and resonant clocking setup to be installed
/// without affecting system stability.  Such a state must be guaranteed at
/// system initialization and after any OCC reset.  If called from any other
/// context the caller is responsible for ensuring that the system is in a
/// state that will allow the procedure to run correctly.
///
/// This procedure does not enable Pstates or enter any Pstate mode, and does
/// not alter any voltage or frequency settings.  After the Pstate tables have
/// been installed, Pstate mode is enabled by calls of
/// gpsm_enable_pstates_master() and gpsm_enable_pstates_slave() as described
/// in the commenst for gpsm_init.c.
///
/// The initialization of Pstates was split up into these three steps to best
/// handle the initialization of the slave chip in a DCM.  This procedure
/// (gpsm_initialize()) can be called by the DCM slave whenever a
/// PstateSuperStructure is available.  By requirememt and convention this
/// Pstate SuperStructure will be identical with the one installed by the DCM
/// master.
///
/// The GPSM driver makes few assumptions about how the system firmware has
/// set up the PMC, but does require some critical setup.
///
/// - It is assumed that for DCM configurations the system firmware will have
/// set the PMC_MODE_REG.enable_interchip_interface (to indicate a DCM
/// configuration), and set the PMC_MODE_REG.interchip_mode appropriately for
/// the master and the slave.
///
/// - It is assumed that the PMC Core Deconfiguration register implies the
/// same set of configured cores as the set included in the PCB multicast
/// group covering all cores.
///
/// All GPSM procedures use the same semaphore, which is set by the interrupt
/// handler for all GPSM interrupts.  The GPSM driver claims the PMC protocol
/// ongoing, voltage change ongoing, and PMC Sync interrupts.
///
/// Once the interrupts are set up, the GlobalPstateTable is copied from the
/// PstateSuperStructure to its proper location and installed.  Next, the
/// Local Pstate tables and resonant clocking setup are installed into all
/// cores by multicast SCOM.
///
/// \retval 0 Success
///
/// \retval -GPSM_INVALID_OBJECT Either the \a i_pss or \a o_gpst are NULL (0).
///
/// \retval -GPSM_INVALID_MAGIC The 'magic number' of the PstateSuperStructure
/// is different from that expected.
///
/// \retval -GPSM_ILLEGAL_MODE_GPSM_INIT Either the PMC indicates a Pstate mode
/// is active, one or more cores appear to have iVRM enabled, or one or more
/// cores appear to have resonant clocking enabled.
///
/// \retval others This API may also return codes from gpsm_gpst_install(),
/// gpsm_lpsa_install() and gpsm_resclk_install().

int
gpsm_initialize(const PstateSuperStructure* i_pss,
                GlobalPstateTable* o_gpst)
{
    pmc_mode_reg_t pmr;
    int rc;

    TRACE_GPSM(TRACE_GPSM_INITIALIZE);

    do {

        // Check for a valid PstateSuperStructure and GlobalPstateTable

        if ((i_pss == 0) || (o_gpst == 0)) {

            rc = -GPSM_INVALID_OBJECT;
            _BREAK;
        }

        if (i_pss->magic != PSTATE_SUPERSTRUCTURE_GOOD1 &&
            i_pss->magic != PSTATE_SUPERSTRUCTURE_GOOD2 &&
            i_pss->magic != PSTATE_SUPERSTRUCTURE_GOOD3 &&
            i_pss->magic != PSTATE_SUPERSTRUCTURE_GOOD4) {

            rc = -GPSM_INVALID_MAGIC;
            _BREAK;
        }


        // Check/set up the PMC mode register

        pmr.value = in32(PMC_MODE_REG);
        if (pmr.fields.enable_hw_pstate_mode ||
            pmr.fields.enable_fw_auction_pstate_mode) {
            rc = -GPSM_ILLEGAL_MODE_GPSM_INIT;
            _BREAK;
        }
        if (!pmr.fields.enable_fw_pstate_mode) {

            pmr.fields.enable_fw_pstate_mode = 1;
            out32(PMC_MODE_REG, pmr.value);
        }

        // ** VBU **
        pmr.fields.halt_pstate_master_fsm = 0;
        out32(PMC_MODE_REG, pmr.value);


        // Initialize interrupt handling

        ssx_semaphore_create(&G_gpsm_protocol_semaphore, 0, 1);

        ssx_irq_disable(PGP_IRQ_PMC_PROTOCOL_ONGOING);
        ssx_irq_disable(PGP_IRQ_PMC_VOLTAGE_CHANGE_ONGOING);
        ssx_irq_disable(PGP_IRQ_PMC_SYNC);

        ssx_irq_setup(PGP_IRQ_PMC_PROTOCOL_ONGOING,
                      SSX_IRQ_POLARITY_ACTIVE_LOW,
                      SSX_IRQ_TRIGGER_LEVEL_SENSITIVE);

        ssx_irq_setup(PGP_IRQ_PMC_VOLTAGE_CHANGE_ONGOING,
                      SSX_IRQ_POLARITY_ACTIVE_LOW,
                      SSX_IRQ_TRIGGER_LEVEL_SENSITIVE);

        ssx_irq_setup(PGP_IRQ_PMC_SYNC,
                      SSX_IRQ_POLARITY_ACTIVE_HIGH,
                      SSX_IRQ_TRIGGER_LEVEL_SENSITIVE);


        ssx_irq_handler_set(PGP_IRQ_PMC_PROTOCOL_ONGOING,
                            ssx_semaphore_post_handler,
                            (void *)(&G_gpsm_protocol_semaphore),
                            SSX_NONCRITICAL);

        ssx_irq_handler_set(PGP_IRQ_PMC_VOLTAGE_CHANGE_ONGOING,
                            ssx_semaphore_post_handler,
                            (void *)(&G_gpsm_protocol_semaphore),
                            SSX_NONCRITICAL);

        ssx_irq_handler_set(PGP_IRQ_PMC_SYNC,
                            ssx_semaphore_post_handler,
                            (void *)(&G_gpsm_protocol_semaphore),
                            SSX_NONCRITICAL);

        // Install the Global Pstate table, Local Pstate Array and resonant
        // clocking setup, using options contained in the PstateSuperStructure.

        rc = gpsm_gpst_install(o_gpst, &(i_pss->gpst));
        if (rc) _BREAK;

        rc = gpsm_lpsa_install(&i_pss->lpsa,
                               &(i_pss->gpst.options));
        if (rc) _BREAK;

        rc = gpsm_resclk_install(&i_pss->resclk,
                                 &(i_pss->gpst),
                                 &(i_pss->gpst.options));
        if (rc) _BREAK;

        G_gpsm_initialized = 1;

    } while (0);

    return rc;
}


// Step voltage manually
//
// This API is only (?) used by gpsm_enable_pstates_master[slave]().  It is
// used to make a (hopefully) minor adjustment between the current voltage and
// the target voltage associated with the initial Global Actual Pstate. In
// cases where the current voltage is not represented in the new Pstate table,
// this routine may take a long time as it will do many single-VID-code steps
// as it gradually moves between the current and target voltages.
//
// Voltage change direction is determined by the difference in the Vdd VIDs,
// and the alogorithm mimics the P7 PVID stepping protocol.  If voltage is
// going up, Vdd and Vcs slew together.  If voltage is going down, Vdd slews
// twice for every change in Vcs.  Note that given a Vdd differential we can't
// assume which way Vcs is moving.
//
// The use of Vcs offsets instead of straight-up VID codes in the hardware is
// extremely confusing, especially since the offsets are defined in normal
// order as opposed to VID codes which decrease as voltage increases.

// Racall that the inputs are VID codes (lower VID --> higher voltage)

static int
_manual_step_voltage(const uint8_t i_currentVdd,
                     const uint8_t i_currentVcs,
                     const uint8_t i_targetVdd,
                     const uint8_t i_targetVcs)
{
    int rc, parity;
    pmc_global_actual_voltage_reg_t pgavr;
    uint8_t currentVdd, currentVcs;

    TRACE_GPSM(TRACE_MANUAL_STEP_VOLTAGE);

    do {

        rc = 0;
        currentVdd = i_currentVdd;
        currentVcs = i_currentVcs;
        parity = 1;

        while ((currentVdd != i_targetVdd) &&
               (currentVcs != i_targetVcs)) {

            if (currentVdd > i_targetVdd) {

                // Voltage going up, slew Vdd and Vcs together. Parity remains
                // 1.

                currentVdd--;

            } else if (currentVdd > i_targetVdd) {

                // Voltage going down, only slew Vcs every other time. Parity
                // is inverted.

                currentVdd++;
                parity = 1 - parity;

            } else {

                // Vdd not moving, set parity so Vcs will move every time.

                parity = 1;
            }

            if (parity) {
                if (currentVcs < i_targetVcs) {
                    currentVcs++;
                } else if (currentVcs > i_targetVcs) {
                    currentVcs--;
                }
            }

            rc = gpsm_quiesce();
            if (rc) _BREAK;

            pgavr.value = 0;
            pgavr.fields.evid_vdd = currentVdd;
            pgavr.fields.evid_vcs = -((int)currentVcs - (int)currentVdd);
            out32(PMC_GLOBAL_ACTUAL_VOLTAGE_REG, pgavr.value);
        }
        if (rc) _BREAK;

        rc = gpsm_quiesce();
        if (rc) _BREAK;

    } while (0);

    return rc;
}


// This is a 'prologue' sequence executed in each core chiplet during the
// initialization of Pstates.  The set of cores to operate on is taken from
// the current value of the PMC_CORE_DECONFIGURATION_REG.
//
// At entry it assumed that iVRM and CPM-DPLL are disabled. It also clears
// possible safe mode dails before enable pstate.
//
// At exit, the following will be true for all configured cores:
//
// - The core will be in DPLL frequency override mode with Fmin and Fmax set
// to the frequency implied by the given Pstate in the given Pstate table with
// 0 undervolting.
//
// - The Fmax bias of the core is set from the Pstate table.
//
// - The Fnom of the core is set from the Pstate table.
//
// - Pstate mode is enabled in the core and global requests are enabled.
//
// - The Local Actual Pstate is being controlled by the Pstate mechanism.
//
// - The PMCR will have been updated to the \a frequencyPstate (both global
// and local) and the global bids (should be) consistent. Auto-override modes
// in the PMCR are not modified.

static int
_enable_pstates_core_prologue(const GlobalPstateTable* i_gpst,
                              const Pstate i_frequency_pstate,
                              const gpst_entry_t i_entry)
{
    int rc, core;
    unsigned int bogus;
    uint32_t configured_cores;
    DpllCode fNom, fPstate;
    pcbs_pmgp1_reg_t pmgp1;
    pcbs_freq_ctrl_reg_t pfcr;
    pcbs_pcbspm_mode_reg_t ppmr;
    pcbs_power_management_control_reg_t pmcr;
    pcbs_power_management_bounds_reg_t ppmbr;
    pcbs_pmc_vf_ctrl_reg_t ppvcr;

    TRACE_GPSM(TRACE_ENABLE_PSTATES_CORE_PROLOGUE);

    do {

        /* In the event of no configured cores, FW requested to not error out */
        //rc = -GPSM_CONFIGURATION_ERROR;
        rc = 0;

        // Do for each core chiplet...
        configured_cores = ~in32(PMC_CORE_DECONFIGURATION_REG);

        // Turn off possible safe mode so we can move pstate
        pcbs_hb_config(0, configured_cores, 0, 0, 0, &bogus);
        pmc_hb_config(0, 0, 0, &bogus);

        for (core = 0; core < PGP_NCORES; core++, configured_cores <<= 1) {

            if (!(configured_cores & 0x80000000)) continue;

            // The 'nominal' frequency code may be biased per core. This
            // should not under/over-flow.

            fNom = i_gpst->pstate0_frequency_code[core];
            rc = bias_frequency(fNom, i_frequency_pstate, &fPstate);
            if (rc) _BREAK;


            /// \bug HW Bug: Chicken-and-egg problem with frequency override
            /// mode.  We need a different HW control structure here.  This
            /// may glitch frequency.

            // Initial PMGP1_REG setup
            //
            // - Force OCC control of the PM SPRS.  This may have to be
            // rethought if PHYP ever controls Pstates.
            //
            // - Enable DPLL frequency overrides

            pmgp1.value = 0;
            pmgp1.fields.pm_spr_override_en = 1;
            pmgp1.fields.dpll_freq_override_enable = 1;

            rc = putscom(CORE_CHIPLET_ADDRESS(PCBS_PMGP1_REG_OR, core),
                         pmgp1.value);
            if (rc) _BREAK;


            // Update Fmin, Fmax, Fmax bias and Pstate0 frequency.

            pfcr.value = 0;
            pfcr.fields.dpll_fmin = fPstate;
            pfcr.fields.dpll_fmax = fPstate;
            pfcr.fields.dpll_fmax_bias = i_gpst->dpll_fmax_bias[core];
            pfcr.fields.frequ_at_pstate0 = fNom;

            rc = putscom(CORE_CHIPLET_ADDRESS(PCBS_FREQ_CTRL_REG, core),
                         pfcr.value);
            if (rc) _BREAK;


            /// \bug HW BUG : PCBS_Power_Management_Bounds_reg hardware reset
            /// is whack, violating the Pstate constraints. (HW216565).
            /// Deferred to Venice.  Not a problem for us, we always set up
            /// this register.

            // The PCBS clipping is initialized to the limits present in the
            // _global_ Pstate table.  This is necessary for correctness of
            // the PCBS state machines. If fast-idle modes with retention are
            // enabled this is also necessary to protect against trying to
            // drop into non-functional Pstates required to be present in the
            // _local_ pstate table.

            // \bug Workaround, since pre_vret_pstate is set to pmin currently
            // until pstate super structure and pstate data block procedure
            // support an entry as non-functional pstate, need to set lower
            // clip bound to be the pstate one above pmin to make pmin
            // essentially a non-functional pstate for now

            ppmbr.value = 0;
            ppmbr.fields.pmin_clip = gpst_pmin(i_gpst)+1;
            ppmbr.fields.pmax_clip = gpst_pmax(i_gpst);

            // This fix is added per SW260911
            // Minimum Frequency in the system is given by MRW attribute
            // PState Datablock procedure will read the attribute then
            // convert it into pstate _pfloor_ and put it into
            // Global Pstate Table. GPSM here consumes the value
            // and set both lower bounds: pmin_rail(PMC) and pmin_clip(PCBS)
            // and two safe pstates: pvsafe(PMc) and psafe(PCBS) to be
            // _pfloor_ if _pfloor_ is higher than their default(gpst_pmin)
            // so that we should never run with frequency below the floor
            // even in safe mode
            if (ppmbr.fields.pmin_clip < i_gpst->pfloor && i_gpst->pfloor != 0)
                ppmbr.fields.pmin_clip = i_gpst->pfloor;

            rc = putscom(CORE_CHIPLET_ADDRESS(PCBS_POWER_MANAGEMENT_BOUNDS_REG,
                                              core),
                         ppmbr.value);
            if (rc) _BREAK;


            // Now that we've locked the frequency and set valid clipping
            // bounds, disable the local Pstate override and allow Global Acks
            // and Pmax-Sync to propogate.

            rc = getscom(CORE_CHIPLET_ADDRESS(PCBS_PMGP1_REG, core),
                         &(pmgp1.value));
            if (rc) _BREAK;

            pmgp1.value = 0;
            pmgp1.fields.enable_occ_ctrl_for_local_pstate_eff_req = 1;

            rc = putscom(CORE_CHIPLET_ADDRESS(PCBS_PMGP1_REG_AND, core),
                         ~pmgp1.value);
            if (rc) _BREAK;


            // Setup PCBS_PMC_VF_CTRL_REG before enable Pstate

            rc = getscom(CORE_CHIPLET_ADDRESS(PCBS_PMC_VF_CTRL_REG, core),
                         &(ppvcr.value));
            if (rc) _BREAK;

            ppvcr.fields.pglobal_actual = i_frequency_pstate;
            ppvcr.fields.maxregvcs = i_entry.fields.maxreg_vdd;
            ppvcr.fields.maxregvdd = i_entry.fields.maxreg_vcs;
            ppvcr.fields.evidvcs_eff = i_entry.fields.evid_vdd_eff;
            ppvcr.fields.evidvdd_eff = i_entry.fields.evid_vcs_eff;

            rc = putscom(CORE_CHIPLET_ADDRESS(PCBS_PMC_VF_CTRL_REG, core),
                         ppvcr.value);
            if (rc) _BREAK;


            // Enable Pstate in PCB Slave

            rc = getscom(CORE_CHIPLET_ADDRESS(PCBS_PCBSPM_MODE_REG, core),
                         &(ppmr.value));
            if (rc) _BREAK;

            ppmr.fields.enable_pstate_mode = 1;
            ppmr.fields.enable_global_pstate_req = 1;
            ppmr.fields.enable_pmc_pmax_sync_notification = 1;

            // ** VBU **
            ppmr.fields.dpll_lock_replacement_timer_mode_en = 1;

            rc = putscom(CORE_CHIPLET_ADDRESS(PCBS_PCBSPM_MODE_REG, core),
                         ppmr.value);
            if (rc) _BREAK;

            // Update the PMCR to propagate the global bids

            rc = getscom(CORE_CHIPLET_ADDRESS(PCBS_POWER_MANAGEMENT_CONTROL_REG,
                                              core),
                         &(pmcr.value));
            if (rc) _BREAK;

            pmcr.fields.global_pstate_req = i_frequency_pstate;
            pmcr.fields.local_pstate_req = i_frequency_pstate;

            rc = putscom(CORE_CHIPLET_ADDRESS(PCBS_POWER_MANAGEMENT_CONTROL_REG,
                                              core),
                         pmcr.value);
            if (rc) _BREAK;
        }
        if (rc) _BREAK;

    } while (0);

    return rc;
}


// This is an 'epilogue' sequence executed in each core chiplet during the
// enablement of Pstate mode.  When this code is executed, the core is in
// frequency override mode at (or below) the frequency of the Global Pstate
// Actual. This procedure releases frequency override mode and core-level
// Pstate operations commence.
//
// retval -GPSM_BABYSTEPPER_SYNC_TIMEOUT, if baby stepper sync
// local_pstate_actual times out
//
static int
_enable_pstates_core_epilogue(void)
{
    int rc = 0, timeout_rc = 0, core;
    uint32_t configured_cores;
    pcbs_pmgp1_reg_t pmgp1;
    pcbs_power_management_status_reg_t ppmsr;
    SsxTimebase timeout;

    TRACE_GPSM(TRACE_ENABLE_PSTATES_CORE_EPILOGUE);

    do {

        configured_cores = ~in32(PMC_CORE_DECONFIGURATION_REG);
        for (core = 0; core < PGP_NCORES; core++, configured_cores <<= 1) {

            if (!(configured_cores & 0x80000000)) continue;

            pmgp1.value = 0;
            pmgp1.fields.dpll_freq_override_enable = 1;

            rc = putscom(CORE_CHIPLET_ADDRESS(PCBS_PMGP1_REG_AND, core),
                         ~pmgp1.value);
            if (rc) _BREAK;
        }
        if (rc) _BREAK;

        // For Babystepper to catch up sync the local_pstate_actual
        // Poll for up to 300us for done before erroring out

        timeout_rc = -GPSM_BABYSTEPPER_SYNC_TIMEOUT;
        timeout = ssx_timebase_get() + SSX_MICROSECONDS(300);
        while (ssx_timebase_get() < timeout) {
            rc = getscom(MC_ADDRESS(PCBS_POWER_MANAGEMENT_STATUS_REG,
                                    MC_GROUP_EX, PCB_MULTICAST_AND),
                         &(ppmsr.value));
            if (rc) _BREAK;
            if (ppmsr.fields.local_pstate_actual ==
                ppmsr.fields.global_pstate_actual) {
                timeout_rc = 0;
                break;
            }
        }
        if (timeout_rc||rc) _BREAK;

    } while(0);

    if (timeout_rc && !rc)
        return timeout_rc;
    else
        return rc;

}


/// Enable Pstates in Firmware Mode, initial Master-only phase
///
/// \param[out] o_info This structure is populated by this API for use
/// by a DCM master in gpsm_enable_pstates_slave.  The DCM slave does not
/// require this information.
///
/// \param[out] o_voltage_pstate This parameter returns the Pstate
/// corresponding to the current system voltage (or the closest safe
/// approximation).  This parameter must be communicated to the slave before
/// the slave can call gpsm_enable_pstates_slave().
///
/// \param[out] o_frequency_pstate *DEPRECATED* This parameter returns the
/// Pstate corresponding to the current system voltage (or the closest safe
/// approximation).  This parameter must be communicated to the slave before
/// the slave can call gpsm_enable_pstates_slave().
///
/// \note This procedure is only called on an SCM or a DCM master.  It will
/// fail if called on a DCM slave.
///
/// \retval 0 Success
///
/// \returns All other return codes indicate an error.

int
gpsm_enable_pstates_master(GpsmEnablePstatesMasterInfo* o_info,
                           Pstate* o_voltage_pstate,
                           Pstate* o_frequency_pstate)
{
    int rc, search_rc;
    GlobalPstateTable* gpst;
    gpst_entry_t voltage_entry;

    TRACE_GPSM(TRACE_GPSM_ENABLE_PSTATES_MASTER);

    do {

        if (gpsm_dcm_slave_p()) {
            rc = -GPSM_ILLEGAL_MODE_EPSM;
            _BREAK;
        }

        // Enter Firmware Pstate Mode.  The gpsm_fw_mode() procedure
        // guarantees that the GPSM is quiesced at this point.  Recover a
        // pointer to the Pstate table from PMC.

        rc = gpsm_fw_mode();
        if (rc) _BREAK;

        gpst = gpsm_gpst();

        // Map the current Vdd VID to a pstate in the new Pstate table.
        //
        // As an option (workaround, simulation hack), force the assumption
        // that the current voltage corresponds to PMIN.  This will not move
        // the external voltage, however it will force the frequency down to
        // the PMIN frequency prior to starting Pstate operations.  It is
        // always safe to change the Pstate from "PMIN", regardless of the
        // actual external voltage, since the PMIN frequency is safe at any
        // voltage.

        if (!(gpst->options.options & PSTATE_FORCE_INITIAL_PMIN)) {

            rc = vrm_voltage_read(SPIVRM_PORT(0),
                                  VRM_RD_VDD_RAIL,
                                  &(o_info->currentVdd));
            if (rc) _BREAK;
            rc = vrm_voltage_read(SPIVRM_PORT(0),
                                  VRM_RD_VCS_RAIL,
                                  &(o_info->currentVcs));
            if (rc) _BREAK;

        } else {

            rc = gpst_entry(gpst, gpst_pmin(gpst), 0, &voltage_entry);
            if (rc) {
                SSX_PANIC(GPSM_BUG); /* This can't happen */
            }

            o_info->currentVdd = voltage_entry.fields.evid_vdd;
            o_info->currentVcs = voltage_entry.fields.evid_vcs;
        }

        search_rc = gpst_vdd2pstate(gpst, o_info->currentVdd,
                                    o_voltage_pstate, &voltage_entry);
        if (search_rc &&
            (search_rc != -GPST_PSTATE_CLIPPED_LOW_GPST_V2P) &&
            (search_rc != -GPST_PSTATE_CLIPPED_HIGH_GPST_V2P)) {
            rc = search_rc;
            break;
        }

        o_info->targetVdd = voltage_entry.fields.evid_vdd;
        o_info->targetVcs = voltage_entry.fields.evid_vcs;


        // If the Pstate was 'clipped low', it indicates that the current
        // voltage is lower than the lowest new Pstate.  Therefore we need to
        // manually step voltage up before locking in the Pmin frequency.  If
        // the Pstate was 'clipped high' it means that the current voltage is
        // higher than the highest Pstate, and we need to lock frequency at
        // the Pmax frequency prior to stepping voltage down. The unclipped
        // case is lumped with the 'clipped low' case as this case might
        // entail a slight rise of voltage. V/F stepping must be split across
        // the calls of gpsm_enable_pstates_master[slave].

        if ((search_rc == 0)||(search_rc = -GPST_PSTATE_CLIPPED_LOW_GPST_V2P)) {

            rc = _manual_step_voltage(o_info->currentVdd, o_info->currentVcs,
                                      o_info->targetVdd, o_info->targetVcs);
            if (rc) _BREAK;

            o_info->move_voltage = 0;

        } else {

            o_info->move_voltage = 1;

        }
    } while (0);

    /// \todo The o_frequency_pstate parameter is no longer needed.  It was
    /// originally needed when the Pstate table had an undervolting bias.
    *o_frequency_pstate = *o_voltage_pstate;

    return rc;
}


/// Enable Pstates in Firmware Pstate Mode, final Master/Slave phase
///
/// \param[in] i_info This structure is populated by
/// gpsm_enable_pstates_master(), and only required in an SCM or DCM master.
/// When this API is called on a DCM slave the parameter may be passed as NULL
/// (0).
///
/// \param[in] i_voltage_pstate This parameter is computed by
/// gpsm_enable_pstates_master(), and is required in every case.
///
/// \param[in] i_frequency_pstate This parameter is computed by
/// gpsm_enable_pstates_master(), and is required in every case.
///
/// \note This procedure is called in all cases as the final step in enabling
/// Pstate mode: SCM, DCM master, DCM slave.
///
/// \retval 0 Success
///
/// \returns All other return codes indicate an error.

int
gpsm_enable_pstates_slave(const GpsmEnablePstatesMasterInfo* i_info,
                          const Pstate i_voltage_pstate,
                          const Pstate i_frequency_pstate)
{
    int rc;
    GlobalPstateTable* gpst;
    pmc_mode_reg_t pmr;
    gpst_entry_t voltage_entry;

    TRACE_GPSM(TRACE_GPSM_ENABLE_PSTATES_SLAVE);

    do {

        // Enter Firmware Pstate Mode.  The gpsm_fw_mode() procedure
        // guarantees that the GPSM is quiesced at this point for the slave;
        // the master must already be quiesced.  Recover a pointer to the
        // Pstate table from PMC.

        if (gpsm_dcm_slave_p()) {

            rc = gpsm_fw_mode();
            if (rc) _BREAK;

        } else {

            if (!i_info) {
                rc = -GPSM_INVALID_ARGUMENT_EPSS;
                _BREAK;

            } else if (!gpsm_fw_mode_p() || !gpsm_quiesced_p()) {

                rc = -GPSM_ILLEGAL_MODE_EPSS;
                _BREAK;

            }
        }

        gpst = gpsm_gpst();
        gpst_entry(gpst, i_voltage_pstate, 0, &voltage_entry);

        // Execute the core prologue.  An SCM or DCM master may need to move
        // voltage after the frequency move.  Since this is guaranteed to be a
        // safe downward move (otherwise we would have moved voltage already),
        // it is safe for the DCM slave to go ahead and finish its Pstate
        // setup before the master has moved the voltage.

        rc = _enable_pstates_core_prologue(gpst, i_frequency_pstate,
                                                 voltage_entry);
        if (rc) _BREAK;

        if (!gpsm_dcm_slave_p()) {

            rc = _manual_step_voltage(i_info->currentVdd, i_info->currentVcs,
                                      i_info->targetVdd, i_info->targetVcs);
            if (rc) _BREAK;
        }


        // The Voltage and Frequency state is now consistent in the cores and
        // in PMC. Make sure that PMC modes are set correctly for Hardware
        // Pstate Mode.

        pmr.value = in32(PMC_MODE_REG);
        pmr.fields.enable_pstate_voltage_changes = 1;
        pmr.fields.enable_global_actual_pstate_forwarding = 1;
        //pmr.fields.enable_pstate_stepping = 1;
        out32(PMC_MODE_REG, pmr.value);


        // Since we're in Firmware Pstate mode and all cores are
        // frequency-locked, we can set the Global Actual without stepping -
        // under the assumption that the caller has disabled iVRM prior to the
        // call.  The master has already computed the volatge_pstate.  We
        // allow the GPSM to quiesce before unlocking the core frequencies.

        _gpsm_broadcast_global_actual(i_frequency_pstate, voltage_entry);

        rc = gpsm_quiesce();
        if (rc) _BREAK;

        rc = _enable_pstates_core_epilogue();
        if (rc) _BREAK;

    } while (0);

    return rc;
}

