/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/common/membuf_configuration.h $                           */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2016,2019                        */
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

/* This header file is used by both occ_405 and occ_gpe1.                 */
/* Contains common structures and globals.                                */
#if !defined(__MEMBUF_CONFIGURATION_H__)
#define __MEMBUF_CONFIGURATION_H__

//#include "occhw_common.h"
#include "gpe_pba_parms.h"
#include "ocmb_firmware_registers.h"

#define OCCHW_N_MC_PORT           4
#define OCCHW_N_MC_CHANNEL        8
#define OCCHW_N_MEMBUF           16
#define OCCWH_MEMBUF_PER_CHANNEL (OCCHW_N_MEMBUF/OCCHW_N_MC_CHANNEL)

#define MEMTYPE_OCMB             2

/// Error return codes set/returned by gpe_*_configuration_create()

typedef enum
{

    MEMBUF_INVALID_ARGUMENT         = 0x007ccc01,
    MEMBUF_BAR_MASK_ERROR           = 0x007ccc07,
    MEMBUF_DATA_SETUP_ERROR         = 0x007ccc09,
    MEMBUF_SCOM_SETUP_ERROR         = 0x007ccc0a,
    MEMBUF_NOT_CONFIGURED           = 0x007ccc0b,
    MEMBUF_MASK_ERROR               = 0x007ccc0c,
    MEMBUF_BARN_GETSCOM_FAILURE     = 0x007ccc0d,
    MEMBUF_BARMSKN_PUTSCOM_FAILURE  = 0x007ccc0e,
    MEMBUF_BARN_PUTSCOM_FAILURE     = 0x007ccc0f,
    MEMBUF_BARMSKN_GETSCOM_FAILURE  = 0x007ccc10,
} MemBufConfigurationCreateRc;

/**
 * Definition of the MemBufConfiguration data.
 * @note location must be 8-byte aligned, size must be multiple of 8 bytes.
 */
typedef struct
{

    /// Membuf base addresses for in-band MMIO/SCOM operations
    ///
    uint64_t baseAddress[OCCHW_N_MEMBUF];

    /// Designated sync scom address
    /// \note One and Only one of the MCS units can be targeted with SYNC
    /// commands. The design includes a private bus connecting all MCS on the
    /// chip that allows this "SYNC master" to broadcast the SYNC to all other
    /// MCS on the chip.
    uint32_t mcSyncAddr;

    // Membuf_type is MEMTYPE_OCMB | etc.
    uint32_t membuf_type; // keep 8 byte aligned

    /// A GpePbaParms parameter block for gpe_mem_data()
    ///
    /// This parameter block is set up in advance and used by the GPE
    /// procedure gpe_membuf_data(). Given the complexity of accessing memory
    /// sensors and SCOM through the PBA it is simpler to set these up ahead
    /// of time and simply have the GPE procedures access preconfigured global
    /// data. The \a dataParms and \a scomParms members are distinguished by
    /// the different way the PBA slave needs to be configured to access
    /// either the memory sensor cache or inband SCOMs.
    GpePbaParms dataParms;

    /// A GpePbaParms parameter block for inband scom.
    GpePbaParms scomParms;

    // Digital Thermal Sensor configuration bitmap.
    // use CONFIG_UBDTS0(n) CONFIG_MEMDTS0(n) CONFIG_MEMDTS1(n) to set/test
    uint64_t dts_config;


    /// A "chip configuration" bit mask denoting valid memory buffer.
    ///
    /// It shoud always be true that a bit denoting a configured memory buffer
    //  is associated with a non-0 \a baseAddress and vice-versa.
    uint32_t config;

    /// The final return code from gpe_*_configuration_create().
    /// @see MemBufConfigurationCreateRc
    /// If initialization fails then this value can be used to diagnose what
    /// happend. This field should be statically initialized to a non-0 value
    /// (MEMBUF_NOT_CONFIGURED) and can then be checked against 0 to
    /// determine if the structure has been correctly initialized.
    int32_t configRc;

} MemBufConfiguration_t __attribute__((aligned(8)));


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


// Inband Scom Return Codes
enum
{
    // rc 1 - 7 reserved from scom rc
    INBAND_ACCESS_INACTIVE = 98,
    INBAND_ACCESS_IN_PROGRESS = 99,
    MEMBUF_INVALID_SCOM = 100,
    MEMBUF_GET_MEM_DATA_COLLECT_INVALID = 200,
    MEMBUF_GET_MEM_DATA_UPDATE_INVALID = 201,
    MEMBUF_GET_MEM_DATA_DIED = 202,
    MEMBUF_CHANNEL_CHECKSTOP = 203,
    MEMBUF_SCACHE_ERROR = 204,
    MEMBUF_PBA_BUSY = 205,
    MEMBUF_INVALID_ADDRESS = 206,
};

// Inband Scom Operations
typedef enum
{
    /// No operation
    MEMBUF_SCOM_NOP,

    /// Read from SCOM, depositing read data into the \a data field of the
    /// scomList_t.
    MEMBUF_SCOM_READ,

    /// Write to SCOM, taking write data from the \a data field of the scomList_t.
    MEMBUF_SCOM_WRITE,

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
    MEMBUF_SCOM_RMW,

    /// The \a data field of the scomList_t contains a
    /// 32-bit pointer (cast to a uint64_t) to an array of # of MEMBUF uint64_t
    /// values. SCOM read data for each configured memory buffer module is
    //  deposited in this array.  Array entries for unconfigured memory
    //  modules are zeroed.
    MEMBUF_SCOM_READ_VECTOR,

    /// Write the \a data field of the scomList_t to
    /// all configured MEMBUFs.
    MEMBUF_SCOM_WRITE_ALL,

    /// Perform read-modify write for all configured
    /// MEMBUF.
    MEMBUF_SCOM_RMW_ALL,

    // Send SYNC to broadcast new throttle values.
    MEMBUF_SCOM_MEMBUF_SYNC,

    // Send SYNC to broadcast OCC_TOUCH cmd and reset the deadman timer
    MEMBUF_SCOM_MEMBUF_RESET_DEADMAN,

}  membuf_scom_operation_t;

#define MCS_MCSYNC_SYNC_TYPE_SYNC      0x00000000ul
#define MCS_MCSYNC_SYNC_TYPE_OCC_TOUCH 0x00700000ul
#define MCS_MCSYNC_SYNC_TYPE_FIELD     0x00f0000000000000ull
#define MCS_MCSYNC_SYNC_GO             0x0000800000000000ull
#define MCS_MCSYNC_EN_SYNC_IN          0x0000400000000000ull

// BAR and PBA_SLAVE assigned to gpe1 membuf
//  - @see POWER Energy Management Hcode/HWP spec
#define PBA_BAR_MEMBUF  1
#define PBA_SLAVE_MEMBUF 1
#define PBA_WRB_EMPTY 1

// These are used to setup MemBufConfiguration.config field
#define CHIP_CONFIG_MCS_BASE 0
#define CHIP_CONFIG_MCS(n) \
    ((0x80000000ul >> CHIP_CONFIG_MCS_BASE) >> (n))

#define CHIP_CONFIG_MEMBUF_BASE 16
#define CHIP_CONFIG_MEMBUF(n) \
    ((0x80000000ul >> CHIP_CONFIG_MEMBUF_BASE) >> (n))

//  These are used to setup the dts_config fields
#define CONFIG_UBDTS0(n) \
    (0x8000000000000000ull >> (4*n))

#define CONFIG_MEMDTS0(n) \
    (0x4000000000000000ull >> (4*n))

#define CONFIG_MEMDTS1(n) \
    (0x2000000000000000ull >> (4*n))

#endif


