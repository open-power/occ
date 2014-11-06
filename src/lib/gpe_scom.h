#ifndef __GPE_SCOM_H__
#define __GPE_SCOM_H__

// $Id: gpe_scom.h,v 1.1.1.1 2013/12/11 20:49:20 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/lib/gpe_scom.h,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file gpe_scom.h
/// \brief Generic SCOM procedures for PORE-GPE
///
/// We provide 2 generic SCOM procedures for PORE-GPE, one for P8 SCOMs and
/// another for Centaur SCOMS. The setup and control of the procedures is
/// roughly modeled after the way the simple SCOM-ing programs were described
/// and implemented by the P7 OCA unit. This facility was written primarily to
/// support SCOM-ing Centaur from OCC (which requires a complex setup),
/// however for some core applications it may also be simpler to use a generic
/// procedure rather than creating a custom GPE program to read/write P8 SCOM
/// registers.
///
/// SCOM programs are set up and controlled through a GpeScomParms
/// structure. This structure contains overall control and status information,
/// as well as a pointer to an array of scomList_t structures which decribes
/// the program. Each entry of the scomList_t describes one of several
/// operations that can be performed on a SCOM address including read, write,
/// and read-modify-write. Special control-only entries are also supported
/// including a NOP, a programmable wait delay, timestamping with the TOD and
/// special "SYNC" commands for Centaur. For more on the commands and their
/// actions please see \ref gpe_scom_commands.
///
/// Each scomList_t entry also includes a data field and a mask field. The
/// data field contains the data to write for SCOM writes and
/// read-modify-writes, holds the data returned for SCOM reads, or contains a
/// pointer to a data vector for vector commands. The mask is used in the case
/// of read-modify-write to indicate which bits of the SCOM to modify. Control
/// commands may also interpret these fields in different ways.


#ifndef __ASSEMBLER__

/// A SCOM command descriptor for gpe_scom_centaur amd gpe_scom_p8.
///
/// For an introduction to the use of this structure and the procedures that
/// use please see the commants for the file gpe_scom.h
///
/// The \a scom field is the full 32-bit SCOM address of the targeted SCOM,
/// initialized by the caller.  SCOM addresses for P8 core SCOMs can be
/// created from the chiplet-0 address using the macro
/// CORE_CHIPLET_ADDRESS(). Multicast addresses for P8 can be generated using
/// the macro MC_ADDRESS(). The notions of internal "chiplets" and "multicast"
/// are not supported for Centaur SCOM addreses, so for Centaur this is always
/// a simple SCOM address. The procedure gpe_scom_centaur() does support an
/// iterative notion of multicast however. Some special control commands do
/// not use this field at all; see \ref gpe_scom_commands.
///
/// The \a errorFlags field contains error status associated with the
/// (attempted) SCOM access.  This field is set by the procedure. For futher
/// informaton on error handling please see each individual procedure.
///
/// The \a instanceNumber field is currently only used by
/// gpe_scom_centaur(). For further details please see the documentation for
/// gpe_scom_centaur(). 
///
/// The \a commandType field is initialized by the caller. For command
/// documentation see \ref gpe_scom_commands and the individual procedures.
///
/// The \a data field is used to hold write data for SCOM write and
/// read-modify-write commands (including the "all" forms), contains the
/// returned read data for scalar SCOM read commands, and contains a pointer
/// to a data vector for vector commands. Other commands may also use the \a
/// data field for other purposes as documented with each command.
///
/// The \a mask field contains a positive bit mask used to identify the fields
/// to update for SCOM read-modify-write. Other commands may also use the \a
/// mask field for other purposes as documented with each command.
///
/// \note Because this structure is read and written by the GPE engine it is
/// strongly recommended to allocate instances of this structure in
/// non-cacheable data sections, with the caveat that data structures assigned
/// to non-default data sections must always be initialized. For example:
///
/// \code
///
/// static scomList_t S_scomList[10] 
///     SECTION_ATTRIBUTE(".noncacheable") = {0};
///
/// \endcode

typedef struct {
    union
    {
        struct {
            uint32_t scom;
            uint8_t reserved;
            uint8_t errorFlags;
            uint8_t instanceNumber;
            uint8_t commandType;
        };
        uint64_t command;
    };
    uint64_t mask;
    union
    {
        uint64_t data;
        struct {
            uint32_t unused;
            uint64_t* pData;
        };
    };
}  scomList_t;

#else // __ASSEMBLER__

        // scomList_t structure offsets

        .set    SCOM_LIST_COMMAND,  0x0
        .set    SCOM_LIST_MASK,     0x8
        .set    SCOM_LIST_DATA,     0x10
        .set    SIZEOF_SCOM_LIST_T, 0x18

        // PGAS macros to extract fields of the scomList_t command. The source
        // and target must be data registers, and they can be the same data
        // register.

        .macro  scom_list_get_scom, target:req, source:req
        extrdi  \target, \source, 32, 0
        .endm

        .macro  scom_list_get_instance_number, target:req, source:req
        extrdi  \target, \source, 8, 48
        .endm

        .macro  scom_list_get_command_type, target:req, source:req
        extrdi  \target, \source, 8, 56
        .endm

        // PGAS macros to update fields of the scomList_t command. The source
        // and target must be different data registers. The target is the
        // current value and is updated with the new field held
        // right-justified in the source. The source register is effectively
        // destroyed by these operations.

        .macro  scom_list_set_error_flags, target:req, source:req
        insrdi  \target, \source, 8, 40
        .endm

        .macro  scom_list_set_instance_number, target:req, source:req
        insrdi  \target, \source, 8, 48
        .endm

#endif // __ASSEMBLER__

/// \defgroup gpe_scom_commands GPE SCOM Procedure Commands
///
/// \note Command 0 is not defined on purpose to trap errors.
/// @{

/// No operation
#define GPE_SCOM_NOP 1

/// Read from SCOM, depositing read data into the \a data field of the
/// scomList_t.
#define GPE_SCOM_READ 2

/// Write to SCOM, taking write data from the \a data field of the scomList_t.
#define GPE_SCOM_WRITE 3

/// Read-Modify-Write.
///
/// This operation first reads the SCOM. Bits under the \a mask field of the
/// scomList_t are then cleared in the read data. The masked read data is then
/// ORed with the contents of the \a data field of the scomList_t and the
/// result is written back to the SCOM address.
///
/// \note This command \e does \e not apply the mask to the data from the \a
/// data field of the scomList_t. The caller should do this (if necessary)
/// when setting up the scomList_t.
///
/// \note The procedures do not provide a way to distinguish errors that may
/// have occurred on the initial read vs. those that may have occurred on the
/// subsequenct write. 
#define GPE_SCOM_RMW 4

/// For gpe_scom_centaur(), the \a data field of the scomList_t contains a
/// 32-bit pointer (cast to a uint64_t) to an array of PGP_NCENTAUR uint64_t
/// values. SCOM read data for each configured Centaur (MCS) is deposited in
/// this array.  Array entries for unconfigured Centaur are zeroed.
#define GPE_SCOM_READ_VECTOR 5

/// For gpe_scom_centaur(), write the \a data field of the scomList_t to
/// all configured Centaur. Currently unsupported for gpe_scom_p8().
#define GPE_SCOM_WRITE_ALL 6

/// For gpe_scom_centaur(), perform read-modify write for all configured
/// Centaur. Currently unsupported for gpe_scom_p8().
#define GPE_SCOM_RMW_ALL 7

/// Programmable wait delay
///
/// This command simply waits for an interval of time specified by the \a data
/// field of the scomList_t. Use the macro GPE_SCOM_WAIT_DELAY() to convert
/// SSX (OCC timebase) ticks into the correct units for this command.  For
/// example use GPE_SCOM_WAIT_DELAY(SSX_MILLISECONDS(10)) to wait 10 ms.
///
/// \note This operation blocks the GPE from completing any other work until
/// the delay is finished.
///
/// \note This time delay can not be implemented with extreme precision due to
/// the lack of a programmable wait delay in the PORE architecture, plus
/// procedure overhead, bus and bus interface contention, etc. For
/// applications requiring extremely precise timing it will be best to code
/// those by hand in PORE assembler and run them in a dedicated lab-only
/// setting.
#define GPE_SCOM_WAIT 8

/// Issue a generic Centaur SYNC
///
/// This command is only valid for gpe_scom_centaur(). This command creates
/// and issues a generic SYNC command to Centaur.  The caller is completely
/// responsible for creating the contents of the data packet sent as part of
/// the Centaur SYNC. The data packet is taken verbatim from the \a data field
/// of the scomList_t, and sent to the MCS designated as the SYNC MCS in the
/// global G_centaurConfiguration.  For further details see the comments with
/// the procedure gpe_scom_centaur() and the CentaurConfiguration structure.
#define GPE_SCOM_CENTAUR_SYNC 9

/// Issue a Centaur SYNC to all configured Centaur
///
/// This command is only valid for gpe_scom_centaur(). This command creates
/// and issues a SYNC command to all configured Centaur. The data packet is
/// taken from the \a data field of the scomList_t, and sent to the MCS
/// designated as the SYNC MCS in the global G_centaurConfiguration. The
/// caller is responsible for setting the SYNC command bits (bits 8:N); The
/// procedure will fill bits 0:7 with a mask of all configured Centaur.  For
/// further details see the comments with the procedure gpe_scom_centaur() and
/// the CentaurConfiguration structure.
#define GPE_SCOM_CENTAUR_SYNC_ALL 10

/// Read the TOD clock
///
/// This command reads the TOD clock and deposits the value into the \a data
/// field of the scomList_t.
#define GPE_SCOM_TOD 11

/// @}


#ifndef __ASSEMBLER__

/// \defgroup centaur_sync_commands Centaur SYNC Command Bits
///
/// The Centaur SYNC command is an 8-byte word written to a specific in-band
/// address. SYNC commands are generated by the gpe_scom_centaur() procedure
/// in response to the GPE_SCOM_CENTAUR_SYNC and GPE_SCOM_CENTAUR_SYNC_ALL
/// commands (which see).
///
/// \note From the MCS Unit Workbook: Note that only the N/M Throttle sync
/// command will be used operationally in P-series, although if will be
/// possible to test all the sync commands in P-series lab testing. Z-series
/// will use all specified sync command types. ... Valid combinations of bits
/// (8:15) are: b00000000, bVVVVVV0V, and b00000010, where V = 0 or 1.
///
/// @{

#define CENTAUR_GENERATE_REFRESH_COUNTER_SYNC    0x0080000000000000ull
#define CENTAUR_RESET_CALIBRATION_COUNTER_1_SYNC 0x0040000000000000ull
#define CENTAUR_RESET_CALIBRATION_COUNTER_2_SYNC 0x0020000000000000ull
#define CENTAUR_RESET_CALIBRATION_COUNTER_3_SYNC 0x0010000000000000ull
#define CENTAUR_RESET_N_M_THROTTLE_COUNTER_SYNC  0x0008000000000000ull
#define CENTAUR_RESET_MB_TIMEBASE_SYNC           0x0004000000000000ull
#define CENTAUR_SUPER_SYNC                       0x0002000000000000ull
#define CENTAUR_MYSTERY_SYNC                     0x0001000000000000ull

/// \todo Figure out what is the "mystery sync"

/// @}


/// Convert an SsxInterval to a delay specification for gpe_scom_*()

// Yes, Virginia, the PORE engine takes 20 cycles to decrement and branch :(
#define GPE_SCOM_WAIT_DELAY(x) ((x) / 20)


/// Parameters for gpe_scom_centaur() and gpe_scom_p8().
///
/// A pointer to an initialized GpeScomParms structure is passed as the
/// parameter to the GPE procedures gpe_scom_centaur() and gpe_scom_p8.
///
/// \note Because this structure is read and written by the GPE engine it is
/// strongly recommended to allocate instances of this structure in
/// non-cacheable data sections, with the caveat that data structures assigned
/// to non-default data sections must always be initialized. For example:
///
/// \code
///
/// static GpeScomParms S_scomParms 
///     SECTION_ATTRIBUTE(".noncacheable") = {0};
///
/// \endcode

typedef struct {

    /// Input: The SCOM list
    ///
    /// This is a right-justfied pointer to an array of scomList_t structures
    /// describing the sequence of commands to execute.
    uint64_t scomList;

    /// Input: The number of entries in the scomList.
    ///
    /// \note It is considered an error if \a entries is 0, under the
    /// assumption that the caller must have neglected to initialize the
    /// structure.
    uint32_t entries;

    /// Input: Procedure options
    ///
    /// An OR-mask of option flags; See \ref gpe_scom_options;
    uint32_t options;

    /// Output: The procedure return code
    ///
    /// This field will contain 0 in the event of a successful return, and a
    /// non-zero value in the event of an error. See \ref gpe_scom_rc for
    /// documentation of the possible return codes.
    uint32_t rc;

    /// Output: The index of the entry that failed
    ///
    /// In the event that \a rc != 0, this field will contain the 0-based
    /// index of the \a scomList entry that was being processed at the time of
    /// the failure, or -1 for failures associated with the parameters or
    /// setup of the procedure.
    int32_t errorIndex;

} GpeScomParms;

#else // __ASSEMBLER__

        // Offsets into the GpeScomParms structure

        .set    GPE_SCOM_PARMS_SCOM_LIST,       0x00
        .set    GPE_SCOM_PARMS_ENTRIES_OPTIONS, 0x08
        .set    GPE_SCOM_PARMS_RC_ERROR_INDEX,  0x10

        // PGAS macros to extract fields of the GpeScomParms. The source
        // and target must be data registers, and they can be the same data
        // register.

        .macro  gpe_scom_parms_get_entries, target:req, source:req
        extrdi  \target, \source, 32, 0
        .endm

        .macro  gpe_scom_parms_get_options, target:req, source:req
        extrdi  \target, \source, 32, 32
        .endm

        .macro  gpe_scom_parms_get_rc, target:req, source:req
        extrdi  \target, \source, 32, 0
        .endm

        // PGAS macros to update fields of the GpeScomParms. The source
        // and target must be different data registers. The target is the
        // current value and is updated with the new field held
        // right-justified in the source. The source register is effectively
        // destroyed by these operations.

        .macro  gpe_scom_parms_set_rc, target:req, source:req
       insrdi   \target, \source, 32, 0
        .endm

        .macro  gpe_scom_parms_set_error_index, target:req, source:req
        insrdi  \target, \source, 32, 32
        .endm

#endif // __ASSEMBLER__

        
/// \defgroup gpe_scom_rc Return Codes From GPE SCOM Procedures
///
/// @{

/// Successful completion of a GPE SCOM program
#define GPE_SCOM_SUCCESS 0

/// An error occurred during setup of the PBA for Centaur access. If this
/// error code is returned then the \a errorIndex field of the GpeScomParms
/// structure will be set to -1.
#define GPE_SCOM_SETUP_ERROR 1

/// One of the fields of the GpeScomParms structure is invalid. In the case of
/// gpe_scom_centaur(), this code may also be returned if there is a problem
/// with the global structure G_centaurConfiguration.  If this error code is
/// returned then the \a errorIndex field of the GpeScomParms structure will
/// be set to -1 if the error occurred before command processing begins. 
#define GPE_SCOM_INVALID_ARGUMENT 2

/// The procedure died. Since GPE procedures do not trap errors by default
/// they will typically die on the first hardware-detected error, and GPE
/// error recovery procedures will clean up the failed job. If this error code
/// is returned then the \a errorIndex field of the GpeScomParms structure
/// will indicate the \a scomList entry being processed at the time of the
/// failure.
#define GPE_SCOM_DIED 3

/// The \a commandType field of the scomList_t was not valid for the procedure.
/// When this error is signalled then the \a errorIndex field of the
/// GpeScomParms structure contains the index of the failing entry.
#define GPE_SCOM_INVALID_COMMAND 4

/// Signalled only by gpe_scom_centaur(), the \a instanceNumber field of the
/// scomList_t did not index a valid (configured) Centaur. This error is only
/// signalled by the GPE_SCOM_READ, GPE_SCOM_WRITE and GPE_SCOM_RMW commands
/// that require a valid Centaur to be specified. When this error is signalled
/// then the \a errorIndex field of the GpeScomParms structure contains the
/// index of the failing entry.
#define GPE_SCOM_INVALID_CENTAUR 5

/// @}


/// Execute a SCOM program for Centaur SCOMs
///
/// \param[in,out] io_parms A pointer to an initialized GpeScomParms
/// structure. Since this structure is used both for input of parameterization
/// and output of return codes it is imperitive that this structure is
/// allocated in non-cacheable memory to avoid cache-related bugs.  See the
/// documentation for the fields of GpeScomParms for more information.
///
/// gpe_scom_centaur() is a GPE program that takes a pointer to an initialized
/// GpeScomParms structure as input and executes the list of SCOMs and other
/// commands. Return codes are returned in the GpeScomParms.
///
/// The following notes relate to the fields of the scomList_t structure when
/// used by gpe_scom_centaur().
///
/// - \a instanceNumber : This field must be set to the index (0 - 7) of the
/// Centaur (MCS) to access for the commands GPE_SCOM_READ, GPE_SCOM_WRITE and
/// GPE_SCOM_RMW. This field is ignored by other commands.
///
/// - \a commandType : gpe_scom_centaur() supports the special command types
/// GPE_SCOM_CENTAUR_SYNC and GPE_CENTAUR_SYNC_ALL as documented in \ref
/// gpe_scom_commands.
///
/// - \a data : The commands GPE_SCOM_CENTAUR_SYNC and
/// GPE_SCOM_CENTAUR_SYNC_ALL require a unique format for the \a data field as
/// documented with the command.
#ifdef DOXYGEN_ONLY
void gpe_scom_centaur(GpeScomParms *io_parms);
#endif

#ifndef __ASSEMBLER__

// Procedure entry points

PoreEntryPoint gpe_scom_centaur;
PoreEntryPoint gpe_scom_p8;

// Debugging symbols

extern uint64_t G_gsc_lastSlaveControl SECTION_ATTRIBUTE(".data.pore");
extern uint64_t G_gsc_lastScomAddress  SECTION_ATTRIBUTE(".data.pore");
extern uint64_t G_gsc_lastOciAddress   SECTION_ATTRIBUTE(".data.pore");

#endif // __ASSEMBLER__

#endif // __GPE_SCOM_H__
