/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/pgp/pgp_centaur.h $                                   */
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
#ifndef __PGP_CENTAUR_H__
#define __PGP_CENTAUR_H__

#include "gpe_pba.h"

#ifndef __ASSEMBLER__

#include <stdint.h>

/// Compute the address of an MCS unit register from an index
///
/// The MCS units have a bizarre PIB addressing scheme.  This macro generates
/// MCS unit PIB addresses from a register name (w/o unit/index prefix),
/// assuming a valid index in the range 0:7. In the big sceheme of things it
/// probably saves space and time to do this with a table lookup rather than
/// generating the code to compute the address modification. (If we ever need
/// these in assembler code we'll have to implement a macro).

#define MCS_ADDRESS(reg, index) (MCS0_##reg | _pgp_mcs_offset[index])

extern const uint16_t _pgp_mcs_offset[PGP_NMCS];


/// A description of the current Centaur configuration
///
/// \note Because this structure is read by the GPE engine it is strongly
/// recommended to allocate instances of this structure in non-cacheable data
/// sections, with the caveat that data structures assigned to non-default
/// data sections must always be initialized. For example:
///
/// \code
///
/// CentaurConfiguration G_centaurConfiguration
/// SECTION_ATTRIBUTE(".noncacheable_ro") = 
/// {.configRc = CENTAUR_NOT_CONFIGURED};
///
/// \endcode

typedef struct {

    /// Centaur base addresses for in-band operations
    ///
    /// These base addresses are used by GPE programs so it is most convenient
    /// to store the entire 64 bits, even though only bits 23:26 of the base
    /// address can be manipulated through the PBA BARs and BAR masks. A 0
    /// value indicates an unconfigured Centaur (MCS).
    uint64_t baseAddress[PGP_NCENTAUR];

    /// Contents of Centaur device id registers
    ///
    /// These are the device ID SCOMs (0x000f000f) read from configured
    /// Centaur during initialization. A 0 value indicates an unconfigured
    /// Centaur.  These values are deconstructed by the memory buffer (mb)
    /// APIs mb_id(), mb_chip_type() and mb_ec_level().
    uint64_t deviceId[PGP_NCENTAUR];

    /// A "chip configuration" bit mask denoting valid Centaur
    ///
    /// It shoud always be true that a bit denoting a configured Centaur is
    /// associated with a non-0 \a baseAddress and vice-versa.
    ChipConfig config;

    /// The image of the PBA slave control register to use for the SYNC command
    ///
    /// The PowerBus address used to accomplish a Centaur SYNC is
    /// constant. To simplify the procedures the PBA slave control register
    /// (containing the extended address portion of the address) is
    /// pre-computed and stored here.
    ///
    /// \note One and Only one of the MCS units can be targeted with SYNC
    /// commands. The design includes a private bus connecting all MCS on the
    /// chip that allows this "SYNC master" to broadcast the SYNC to all other
    /// MCS on the chip.
    uint64_t syncSlaveControl;

    /// A GpePbaParms parameter block for gpe_mem_data()
    ///
    /// This parameter block is set up in advance and used by the GPE
    /// procedure gpe_mem_data(). Given the complexity of accessing Centaur
    /// sensors and SCOM through the PBA it is simpler to set these up ahead
    /// of time and simply have the GPE procedures access preconfigured global
    /// data. The \a dataParms and \a scomParms members are distinguished by
    /// the different way the PBA slave needs to be configured to access
    /// either the Centaur sensor cache or Centaur SCOMs.
    GpePbaParms dataParms;

    /// A GpePbaParms parameter block for gpe_scom_centaur().
    GpePbaParms scomParms;

    /// The final return code from centaur_configuration_create().
    ///
    /// If initialization fails then this value can be used to diagnose what
    /// happend. This field should be statically initialized to a non-0 value
    /// (CENTAUR_NOT_CONFIGURED) and can then be checked against 0 to
    /// determine if the structure has been correctly initialized.
    int64_t configRc;

} CentaurConfiguration;

/// The global CentaurConfiguration created during initialization
extern CentaurConfiguration G_centaurConfiguration;

#else // __ASSEMBLER__

        .set    CENTAUR_CONFIGURATION_BASE_ADDRESS, 0x0

        .set    CENTAUR_CONFIGURATION_DEVICE_ID, \
                (CENTAUR_CONFIGURATION_BASE_ADDRESS + (8 * PGP_NCENTAUR))

        .set    CENTAUR_CONFIGURATION_CONFIG, \
                (CENTAUR_CONFIGURATION_DEVICE_ID + (8 * PGP_NCENTAUR))

        .set    CENTAUR_CONFIGURATION_SYNC_SLAVE_CONTROL, \
                (CENTAUR_CONFIGURATION_CONFIG + 8)  

        .set    CENTAUR_CONFIGURATION_DATA_PARMS, \
                (CENTAUR_CONFIGURATION_SYNC_SLAVE_CONTROL + 8)  

        .set     CENTAUR_CONFIGURATION_SCOM_PARMS, \
                (CENTAUR_CONFIGURATION_DATA_PARMS + SIZEOF_GPEPBAPARMS)

        .set     CENTAUR_CONFIGURATION_CONFIG_RC, \
                (CENTAUR_CONFIGURATION_SCOM_PARMS + SIZEOF_GPEPBAPARMS)

        .set    SIZEOF_CENTAUR_CONFIGURATION, \
                (CENTAUR_CONFIGURATION_CONFIG_RC + 8)

#endif // __ASSEMBLER__


#ifndef __ASSEMBLER__

/// Error return codes set/returned by centaur_configuration_create()

enum CentaurConfigurationCreateRc{

    CENTAUR_INVALID_ARGUMENT         = 0x007ccc01,
    CENTAUR_MCSMODE0_SCOM_FAILURE    = 0x007ccc02,
    CENTAUR_MCSMODE0_19_FAILURE      = 0x007ccc03,
    CENTAUR_ADDRESS_27_FAILURE       = 0x007ccc04,
    CENTAUR_MULTIPLE_DESIGNATED_SYNC = 0x007ccc05,
    CENTAUR_NO_DESIGNATED_SYNC       = 0x007ccc06,
    CENTAUR_BAR_MASK_ERROR           = 0x007ccc07,
    CENTAUR_CONFIGURATION_FAILED     = 0x007ccc08,
    CENTAUR_DATA_SETUP_ERROR         = 0x007ccc09,
    CENTAUR_SCOM_SETUP_ERROR         = 0x007ccc0a,
    CENTAUR_NOT_CONFIGURED           = 0x007ccc0b,
    CENTAUR_MASK_ERROR               = 0x007ccc0c,
    CENTAUR_READ_TPC_ID_FAILURE      = 0x007ccc0d,
    CENTAUR_BARMSKN_PUTSCOM_FAILURE  = 0x007ccc0e,   
    CENTAUR_BARN_PUTSCOM_FAILURE     = 0x007ccc0f,
    CENTAUR_BARMSKN_GETSCOM_FAILURE  = 0x007ccc10,   
    CENTAUR_BARN_GETSCOM_FAILURE     = 0x007ccc11,
};


/// Create (initialize) G_centaurConfiguration
///
/// G_centaurConfiguration is a global structure used by GPE procedures to
/// access Centaur, and the mb_*() APIs to return CFAM-id type information
/// about the Centaurs.
///
/// To complete Centaur configuration requires running the GPE program
/// gpe_scom_centaur() on PORE-GPE1 to collect the TPC device Ids of the
/// Centaur chips. This means that the "async" drivers must be set up prior to
/// the call. We assume this API will be called before threads have started,
/// thus it will poll the async request for completion. Assuming no other GPE
/// programs are scheduled this should take a few microseconds at most.
///
/// \returns Either 0 for success or an element of the
/// CentaurConfigurationCreateRc enumeration.
int
centaur_configuration_create(void);


/// Create (initialize) G_centaurConfiguration (Internal API)
///
/// \param[in] i_bar The index of the PBA BAR reserved for access to
/// Centaur. This will normally be passed as the constant PBA_BAR_CENTAUR but
/// is allowed to be variable for special cases. 
///
/// \param[in] i_slave The index of the PBA slave reserved for access from the
/// GPE complex. This will normally be passed as the constant
/// PBA_SLAVE_PORE_GPE but is allowed to be variable for special cases.
///
/// \param[in] i_setup If non-0, then this procedure will set up the PBA BAR
/// correctly for access to Centaur. If > 1, then the procedure will also
/// designate an MCS to recieve the Centaur SYNC if the firmware failed to do
/// so. 
///
/// This API must be run early in the initialization flow, likely before the
/// real-time loop is activated. The API first scans the MBS configuration for
/// correctness and (optionally) sets up the PBA BAR and mask for access to
/// Centaur. The API then runs the gpe_scom_centaur() procedure to get the
/// CFAM Id from each configured Centaur.
///
/// \note Normally we would implement test/bringup workarounds like the \a
/// i_setup parameter separately, however the setup of Centaur is at a level
/// of complexity where it makes sense to implement this override in a
/// mainline procedure.
int
_centaur_configuration_create(int i_bar, int i_slave, int i_setup);


/// Get a Centaur (MB) CFAM Chip Id
/// 
/// \param[in] i_mb The index (0..PGP_NCENTAUR - 1) of the memory buffer being
/// queried.
///
/// \returns A 32-bit value to be compared against the enumeration of known
/// CFAM ids.  See \ref pgp_cfam_chip_ids.  If the \a i_mb is invalid or the
/// Centaur is not configured or the G_centaurConfiguration is not valid then
/// (uint32_t)-1 is returned.
uint32_t mb_id(int i_mb);


/// Get a Centaur (MB) Chip Type
/// 
/// \param[in] i_mb The index (0..PGP_NCENTAUR - 1) of the memory buffer being
/// queried.
///
/// \returns An 8-bit value to be compared against the enumeration of known
/// CFAM chip types.  See \ref pgp_cfam_chip_types. If the \a i_mb is invalid
/// or the Centaur is not configured or the G_centaurConfiguration is not
/// valid then (uint8_t)-1 is returned.
uint8_t mb_chip_type(int i_mb);


/// Get a Centaur (MB) CFAM Chip EC Level
/// 
/// \param[in] i_mb The index (0..PGP_NCENTAUR - 1) of the memory buffer being
/// queried.
///
/// \returns An 8-bit value; The high-order nibble is the major EC level and
/// the low-order nibble is the minor EC level. For example a value of 0x21
/// indicates DD 2.1. If the \a i_mb is invalid or the Centaur is not
/// configured or the G_centaurConfiguration is not valid then (uint8_t)-1 is
/// returned.
uint8_t mb_ec_level(int i_mb);

#endif // __ASSEMBLER

#endif // __PGP_CENTAUR_H__
