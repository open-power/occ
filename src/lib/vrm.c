// $Id: vrm.c,v 1.2 2014/02/03 01:30:26 daviddu Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/lib/vrm.c,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file vrm.c
/// \brief PgP SPIVRM procedures

#include "vrm.h"

// The semaphore used to block threads waiting for o2s operations to complete

static SsxSemaphore o2s_protocol_semaphore;


/// o2s_initialize follows the steps for setting up the o2s bridge as outlined
/// In the Energy Management spec (PMC section)
int 
o2s_initialize()
{
    pmc_o2s_ctrl_reg0a_t pocr0a;
    pmc_o2s_ctrl_reg0b_t pocr0b;
    pmc_o2s_ctrl_reg1_t pocr1;


    ssx_semaphore_create(&o2s_protocol_semaphore, 0, 1);

    ssx_irq_disable(PGP_IRQ_OCI2SPIVID_ONGOING);
    
    ssx_irq_setup(PGP_IRQ_OCI2SPIVID_ONGOING,
                  SSX_IRQ_POLARITY_ACTIVE_LOW,
                  SSX_IRQ_TRIGGER_LEVEL_SENSITIVE);

    ssx_irq_handler_set(PGP_IRQ_OCI2SPIVID_ONGOING,
                        ssx_semaphore_post_handler,
                        (void *)(&o2s_protocol_semaphore),
                        SSX_NONCRITICAL);

    pocr0a.value = in32(PMC_O2S_CTRL_REG0A);
    pocr0a.fields.o2s_frame_size = 32;
    pocr0a.fields.o2s_out_count1 = 32;
    pocr0a.fields.o2s_in_delay1 = 0x3F;
    pocr0a.fields.o2s_in_count1 = 0;
    out32(PMC_O2S_CTRL_REG0A, pocr0a.value);

    pocr0b.value = in32(PMC_O2S_CTRL_REG0B);
    pocr0b.fields.o2s_out_count2 = 0;
    pocr0b.fields.o2s_in_delay2 = 0;
    pocr0b.fields.o2s_in_count2 = 32;
    out32(PMC_O2S_CTRL_REG0B, pocr0b.value);

    pocr1.value = in32(PMC_O2S_CTRL_REG1);
    pocr1.fields.o2s_bridge_enable = 1;
    pocr1.fields.o2s_cpol = 0;
    pocr1.fields.o2s_cpha = 0;
    pocr1.fields.o2s_clock_divider = 29; // 10 MHz(?)
    pocr1.fields.o2s_nr_of_frames = 1;  // 2 frames
    out32(PMC_O2S_CTRL_REG1, pocr1.value);

    return 0;

}

/// similar to o2s_intialize, but for the automated spivid fsm
/// \param vrm_select A 3-bit vector of VRM selected for the operation.
///
/// NOTE:  The spivid is normally initialized by Host Boot
///
/// \retval 0 Success
int
spivid_initialize(int vrm_select)
{
    pmc_spiv_ctrl_reg0a_t pocr0a;
    pmc_spiv_ctrl_reg0b_t pocr0b;
    pmc_spiv_ctrl_reg1_t pocr1;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF((vrm_select <= 0) ||
                     (vrm_select > 0x7),
                     VRM_INVALID_ARGUMENT_INIT);
    }


    pocr0a.value = in32(PMC_SPIV_CTRL_REG0A);
    pocr0a.fields.spivid_frame_size = 32;
    pocr0a.fields.spivid_out_count1 = 32;
    pocr0a.fields.spivid_in_delay1 = 0x3F;
    pocr0a.fields.spivid_in_count1 = 0;
    out32(PMC_SPIV_CTRL_REG0A, pocr0a.value);

    pocr0b.value = in32(PMC_SPIV_CTRL_REG0B);
    pocr0b.fields.spivid_out_count2 = 0;
    pocr0b.fields.spivid_in_delay2 = 0;
    pocr0b.fields.spivid_in_count2 = 32;
    out32(PMC_SPIV_CTRL_REG0B, pocr0b.value);

    pocr1.value = in32(PMC_SPIV_CTRL_REG1);
    pocr1.fields.spivid_fsm_enable = 1;
    pocr1.fields.spivid_cpol = 0;
    pocr1.fields.spivid_cpha = 0;
    pocr1.fields.spivid_clock_divider = 29; // 10 MHz(?)
    pocr1.fields.spivid_port_enable = vrm_select; 
    out32(PMC_SPIV_CTRL_REG1, pocr1.value);

    return 0;

}


// Start an O2S transaction and poll for completion.  Optionally return the
// input data.

static void
o2s_start_poll(uint64_t out, uint64_t *in)
{

    out32(PMC_O2S_WDATA_REG, out >> 32);

    if (!ssx_irq_status_get(PGP_IRQ_OCI2SPIVID_ONGOING)) {
        ssx_irq_enable(PGP_IRQ_OCI2SPIVID_ONGOING);
        ssx_semaphore_pend(&o2s_protocol_semaphore, SSX_WAIT_FOREVER);
    }


    if (in != 0) {
        *in = in32(PMC_O2S_RDATA_REG);
    }
}


/// Write a voltage using the O2S bridge
///
/// \param vrm_select A 3-bit vector of VRM selected for the operation.
///
/// \param vdd_vid The VRM-11 VID code for Vdd.
///
/// \param vcs_offset The signed offset (Vdiff) equal to Vcs - Vdd expressed
/// in VRM-11 VID units.
///
/// \param phases The number of phases enabled.
///
/// This is a polling CPU procedure that writes a new voltage to a set of one
/// or more VRM then does a status read to make sure it succeeded.
///
/// \retval 0 Success
///
/// \retval -VRM_INVALID_ARGUMENT_VWRITE One of the arguments was invalid in 
/// some way.
///
/// \retval -O2S_BUSY_VRM_VOLTAGE_WRITE The O2S bridge is currently busy
///
/// \retval -O2S_READ_NOT_READY A 'read not ready' condition occurred on the
/// status read.
///
/// \retval -O2S_WRITE_NOT_VALID The voltage write was invalid
///
/// \retval -O2S_ECC_ERROR An ECC error occurred
///
/// \todo We need to understand what the firmware is expected to do when the
/// 'read not ready' or other error responses come back.  Here they will
/// likely panic.

int
vrm_voltage_write(int vrm_select, 
                  uint8_t vdd_vid, 
                  int8_t vcs_offset, 
                  int phases)
{
    int i, port;
    uint64_t result;
    pmc_o2s_ctrl_reg1_t pocr;
    pmc_o2s_status_reg_t posr;
    vrm_write_transaction_t vwt;
    vrm_write_resp_t vwr;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF((vrm_select <= 0) ||
                     (vrm_select > 0x7) ||
                     (phases < 0) ||
                     (phases > 0xf),
                     VRM_INVALID_ARGUMENT_VWRITE);
    }

    // Check for O2S busy

    posr.value = in32(PMC_O2S_STATUS_REG);

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF(posr.fields.o2s_ongoing, O2S_BUSY_VRM_VOLTAGE_WRITE);
    }

    // Use O2S to set voltage and read status, one port at a time.

    for (i = 0; i < SPIVRM_NPORTS; i++) {
        
        port = vrm_select & SPIVRM_PORT(i);
        if (port != 0) {

            // Set the (singular) O2S port

            pocr.value = in32(PMC_O2S_CTRL_REG1);
            pocr.fields.o2s_port_enable = port;
            out32(PMC_O2S_CTRL_REG1, pocr.value);

            // Create and initiate a voltage write command

            vwt.value = 0;
            vwt.fields.command = VRM_WRITE_VOLTAGE;
            vwt.fields.vdd_vid = vdd_vid;
            vwt.fields.vcs_offset = vcs_offset;
            vwt.fields.phase_enable = phases;


            o2s_start_poll(vwt.value, &result);
            // Check the status
            vwr.value = result << 32;

            // results are duplicated 3x, using first byte for checking

            SSX_ERROR_IF(vwr.fields.write_status0 == 0x00, O2S_READ_NOT_READY);
            SSX_ERROR_IF(vwr.fields.write_status0 == 0x55, O2S_WRITE_ECC_ERROR);
            SSX_ERROR_IF(vwr.fields.write_status0 != 0xAA, O2S_WRITE_NOT_VALID);
        }
    }
    return 0;
}

/// Read VRM state using the O2S bridge
///
/// \param vrm_select A 3-bit vector of VRM selected for the operation.
/// This procedure only allows 1 VRM to be selected.
///
/// \param vrail A 4-bit value for selecting a voltage rail
///
/// \param[out] o_vid The resulting 8-bit VRM-11 encoded voltage ID
///
///
/// \retval 0 Success
///
/// \retval -VRM_INVALID_ARGUMENT_SREAD One of the arguments was invalid in some
/// way.
///
/// \retval -O2S_BUSY_VRM_READ_STATE The O2S bridge is currently busy
///

int
vrm_read_state(int vrm_select, 
                  int  *mnp1,
                  int  *mn,
                  int  *vfan,
                  int  *vovertmp)
{
    int i, port;
    uint64_t result;
    pmc_o2s_status_reg_t posr;
    pmc_o2s_ctrl_reg1_t pocr;
    vrm_read_state_t vrs;
    vrm_read_state_resp_t vrsr;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF((!((vrm_select == SPIVRM_PORT(0)) ||
                        (vrm_select == SPIVRM_PORT(1)) || 
                        (vrm_select == SPIVRM_PORT(2)))),
                        VRM_INVALID_ARGUMENT_SREAD);
    }


    // Check for O2S busy

    posr.value = in32(PMC_O2S_STATUS_REG);

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF(posr.fields.o2s_ongoing, O2S_BUSY_VRM_READ_STATE);
    }

    // Use O2S to read voltage for selected rail.

    for (i = 0; i < SPIVRM_NPORTS; i++) {
        
        port = vrm_select & SPIVRM_PORT(i);
        if (port != 0) {

            // Set the (singular) O2S port

            pocr.value = in32(PMC_O2S_CTRL_REG1);
            pocr.fields.o2s_port_enable = port;
            out32(PMC_O2S_CTRL_REG1, pocr.value);

            // Create and initiate a voltage read command

            vrs.value = 0;
            vrs.fields.command = VRM_READ_STATE;


            o2s_start_poll(vrs.value, &result);
            // Check the status
            vrsr.value = result << 32;

            // results are duplicated 3x, returning first byte
            *mnp1     = vrsr.fields.minus_nplus1_0;
            *mn       = vrsr.fields.minus_n0;
            *vfan     = vrsr.fields.vrm_fan0; 
            *vovertmp = vrsr.fields.vrm_overtemp0;

        }
    }
    return 0;
}


/// Read a voltage using the O2S bridge
///
/// \param vrm_select A 3-bit vector of VRM selected for the operation.
/// This procedure only allows 1 VRM to be selected.
///
/// \param vrail A 4-bit value for selecting a voltage rail
///
/// \param[out] o_vid The resulting 8-bit VRM-11 encoded voltage ID
///
///
/// \retval 0 Success
///
/// \retval -VRM_INVALID_ARGUMENT_VREAD One of the arguments was invalid in some
/// way.
///
/// \retval -O2S_BUSY_VRM_VOLTAGE_READ The O2S bridge is currently busy
///

int
vrm_voltage_read(int vrm_select, 
                  uint8_t vrail, 
                  uint8_t *o_vid)
{
    int i, port;
    uint64_t result;
    pmc_o2s_status_reg_t posr;
    pmc_o2s_ctrl_reg1_t pocr;
    vrm_read_voltage_t vrv;
    vrm_read_voltage_resp_t vrvr;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF((!((vrm_select == SPIVRM_PORT(0)) ||
                        (vrm_select == SPIVRM_PORT(1)) || 
                        (vrm_select == SPIVRM_PORT(2)))) ||
                        (vrail > (SPIVRM_NRAILS - 1)),
                        VRM_INVALID_ARGUMENT_VREAD);
    }


    // Check for O2S busy

    posr.value = in32(PMC_O2S_STATUS_REG);

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF(posr.fields.o2s_ongoing, O2S_BUSY_VRM_VOLTAGE_READ);
    }

    // Use O2S to read voltage for selected rail.

    for (i = 0; i < SPIVRM_NPORTS; i++) {
        
        port = vrm_select & SPIVRM_PORT(i);
        if (port != 0) {

            // Set the (singular) O2S port

            pocr.value = in32(PMC_O2S_CTRL_REG1);
            pocr.fields.o2s_port_enable = port;
            out32(PMC_O2S_CTRL_REG1, pocr.value);

            // Create and initiate a voltage read command

            vrv.value = 0;
            vrv.fields.command = VRM_READ_VOLTAGE;
            vrv.fields.rail    = vrail;


            o2s_start_poll(vrv.value, &result);
            // Check the status
            vrvr.value = result << 32;

            // results are duplicated 3x, returning first byte
            *o_vid = vrvr.fields.vid0;
        }
    }
    return 0;
}




