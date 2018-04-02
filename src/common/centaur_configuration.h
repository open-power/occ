#if !defined(__CENTAUR_CONFIGURATION_H__)
#define __CENTAUR_CONFIGURATION_H__

//#include "occhw_common.h"
#include "gpe_pba_parms.h"
#include "centaur_firmware_registers.h"

#define OCCHW_NCENTAUR           8

/// Error return codes set/returned by centaur_configuration_create()

typedef enum
{

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
} CentaurConfigurationCreateRc;

/**
 * Definition of the CentaurConfiguration data.
 * @note location must be 8-byte aligned, size must be multiple of 8 bytes.
 */
typedef struct
{

    /// Centaur base addresses for in-band operations
    ///
    uint64_t baseAddress[OCCHW_NCENTAUR];

    /// Contents of Centaur device id registers
    ///
    /// These are the device ID SCOMs (0x000f000f) read from configured
    /// Centaur during initialization. A 0 value indicates an unconfigured
    /// Centaur.  These values are deconstructed by the memory buffer (mb)
    /// APIs mb_id(), mb_chip_type() and mb_ec_level().
    centaur_device_id_t deviceId[OCCHW_NCENTAUR];

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
    /// MCS on the chip.  Currently not used in P9
    pba_slvctln_t syncSlaveControl;

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

    /// A GpePbaParms parameter block for IPC_ST_CENTAUR_SCOM().
    GpePbaParms scomParms;

    /// A "chip configuration" bit mask denoting valid Centaur
    ///
    /// It shoud always be true that a bit denoting a configured Centaur is
    /// associated with a non-0 \a baseAddress and vice-versa.
    uint32_t config;


    /// The final return code from centaur_configuration_create().
    /// @see CentaurConfigurationCreateRc
    /// If initialization fails then this value can be used to diagnose what
    /// happend. This field should be statically initialized to a non-0 value
    /// (CENTAUR_NOT_CONFIGURED) and can then be checked against 0 to
    /// determine if the structure has been correctly initialized.
    int32_t configRc;

} CentaurConfiguration_t __attribute__((aligned(8)));

// Moved CentaurGetMemDataParms_t to centaur_structs.h

typedef struct
{
    union
    {
        struct
        {
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
        struct
        {
            uint32_t unused;
            uint64_t* pData;
        };
    };
}  scomList_t;


// Centaur Scom Return Codes
enum
{
    // rc 1 - 7 reserved from scom rc
    CENTAUR_INVALID_SCOM = 100,
    CENTAUR_GET_MEM_DATA_COLLECT_INVALID = 200,
    CENTAUR_GET_MEM_DATA_UPDATE_INVALID = 201,
    CENTAUR_GET_MEM_DATA_DIED = 202,
    CENTAUR_CHANNEL_CHECKSTOP = 203,
};

// Centaur Scom Operations
typedef enum
{
    /// No operation
    CENTAUR_SCOM_NOP,

    /// Read from SCOM, depositing read data into the \a data field of the
    /// scomList_t.
    CENTAUR_SCOM_READ,

    /// Write to SCOM, taking write data from the \a data field of the scomList_t.
    CENTAUR_SCOM_WRITE,

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
    CENTAUR_SCOM_RMW,

    /// For IPC_ST_CENTAUR_SCOM(), the \a data field of the scomList_t contains a
    /// 32-bit pointer (cast to a uint64_t) to an array of PGP_NCENTAUR uint64_t
    /// values. SCOM read data for each configured Centaur (MCS) is deposited in
    /// this array.  Array entries for unconfigured Centaur are zeroed.
    CENTAUR_SCOM_READ_VECTOR,

    /// For IPC_ST_CENTAUR_SCOM(), write the \a data field of the scomList_t to
    /// all configured Centaur. Currently unsupported for gpe_scom_p8().
    CENTAUR_SCOM_WRITE_ALL,

    /// For IPC_ST_CENTAUR_SCOM(), perform read-modify write for all configured
    /// Centaur. Currently unsupported for gpe_scom_p8().
    CENTAUR_SCOM_RMW_ALL,

}  centaur_scom_operation_t;


// BAR and PBA_SLAVE assigned to gpe1 centaur
//  - @see POWER Energy Management Hcode/HWP spec
#define PBA_BAR_CENTAUR  1
#define PBA_SLAVE_CENTAUR 2

// This is used to setup CentaurConfiguration.config field
#define CHIP_CONFIG_MCS_BASE 16
#define CHIP_CONFIG_MCS(n) \
    ((0x80000000ul >> CHIP_CONFIG_MCS_BASE) >> (n))

#define CHIP_CONFIG_CENTAUR_BASE 24
#define CHIP_CONFIG_CENTAUR(n) \
    ((0x80000000ul >> CHIP_CONFIG_CENTAUR_BASE) >> (n))

#endif


