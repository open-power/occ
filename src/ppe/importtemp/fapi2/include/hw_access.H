/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/importtemp/fapi2/include/hw_access.H $                */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2012,2015                        */
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

/// @file hw_access.H
///
/// @brief Hardware access functions that needs to be specialized for
/// platform implementation.
///

#ifndef _FAPI2_HWACCESS_H_
#define _FAPI2_HWACCESS_H_

    // variable_buffer isn't supported on PPE
#ifndef __PPE__
#include <variable_buffer.H>
#endif

#include <plat_hw_access.H>
#include <fapi2_hw_access.H>

namespace fapi2
{
    //--------------------------------------------------------------------------
    // PIB Error Functions
    //--------------------------------------------------------------------------

    /// @brief Sets the PIB error mask - platform dependant
    /// @param[in]  i_mask     The new error mask
    // note: this can be moved  to a C file if desired
    inline void setPIBErrorMask(uint8_t i_mask)
    {
        // Keeps the compiler from complaining about the unused i_mask
        static_cast<void>(i_mask);

        return;
    }

    /// @brief Gets the PIB error mask - platform dependant
    /// @return uint8_t The current PIB error mask
    // note: this can be moved  to a C file if desired
    inline uint8_t getPIBErrorMask(void)
    {
        return 0;
    }

    //--------------------------------------------------------------------------
    // Operational Mode Error Functions
    //--------------------------------------------------------------------------

    /// @brief Sets the operational mode
    /// @param[in]  i_mode     The new mode
    // note: this can be moved  to a C file if desired
    inline void setOpMode(const OpModes i_mode)
    {
        // Keeps the compiler from complaining about the unused i_mode
        static_cast<void>(i_mode);

        // No-op for now. Should set thread-local operational mode
        return;
    }

    /// @brief Gets the operational mode
    /// @return the operational mode
    // note: this can be moved  to a C file if desired
    inline OpModes getOpMode(void)
    {
        // No-op for now. Should read thread-local operational mode
        return NORMAL;
    }

    //------------------------------------------------------------------------------
    // HW Communication Functions to be implemented at the platform layer.
    //------------------------------------------------------------------------------

    ///
    /// @brief Platform-level implementation of getScom()
    /// @Tparam K template parameter, passed in target.
    /// @param[in]  i_target   HW target to operate on.
    /// @param[in]  i_address  SCOM register address to read from.
    /// @param[out] o_date     Buffer that holds data read from HW target.
    /// @return fapi::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
    ///
    template< TargetType K >
    inline ReturnCode getScom(const Target<K>& i_target,
                              const uint64_t i_address,
                              buffer<uint64_t>& o_data)
    {
        o_data = 0x0000FEEDFACE0000;
        std::cout << std::hex << "   getScom "
                  << "target: {" << i_target.getType() << ","
                  << uint64_t(i_target) << "}; "
                  << "address: " << i_address << "; "
                  << "output data: " << uint64_t(o_data)
                  << std::dec << std::endl;

        return FAPI2_RC_SUCCESS;
    }

    /// @brief Platform-level implementation of putScom()
    /// @Tparam K template parameter, passed in target.
    /// @param[in] i_target     HW target to operate on.
    /// @param[in] i_address    SCOM register address to write to.
    /// @param[in] i_data       Buffer that holds data to write into address.
    /// @return fapi::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
    template< TargetType K >
    inline ReturnCode putScom(const Target<K>& i_target,
                              const uint64_t i_address,
                              const buffer<uint64_t> i_data)
    {
        std::cout << std::hex << "   putScom "
                  << "target: {" << i_target.getType() << ","
                  << uint64_t(i_target) << "}; "
                  << "address: " << i_address << "; "
                  << "input data: " << uint64_t(i_data)
                  << std::dec << std::endl;
        return FAPI2_RC_SUCCESS;
    }

    /// @brief Platform-level implementation of putScomUnderMask()
    /// @tparam K template parameter, passed in target.
    /// @param[in] i_target     HW target to operate on.
    /// @param[in] i_address    SCOM register address to write to.
    /// @param[in] i_data       Buffer that holds data to write into address.
    /// @param[in] i_mask       Buffer that holds the mask value.
    /// @return fapi::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
    template< TargetType K >
    inline ReturnCode putScomUnderMask(const Target<K>& i_target,
                                       const uint64_t i_address,
                                       const buffer<uint64_t> i_data,
                                       const buffer<uint64_t> i_mask)
    {
        std::cout << std::hex << "   putScomUnderMask "
                  << "target: {" << i_target.getType() << ","
                  << uint64_t(i_target) << "}; "
                  << "address: " << i_address << "; "
                  << "input data: " << uint64_t(i_data) << "; "
                  << "input mask: " << uint64_t(i_mask)
                  << std::dec << std::endl;
        return FAPI2_RC_SUCCESS;
    }

    ///
    /// @brief Platform-level implementation called by getCfamRegister()
    /// Hardware procedures writers will not call this function.
    /// @Tparam K template parameter, passed in target.
    /// @param[in]  i_target   HW target to operate on.
    /// @param[in]  i_address  CFAM address to read from.
    /// @param[out] o_data     32-bit buffer that holds data read from HW target.
    /// @return fapi::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
    ///
    template< TargetType K >
    inline ReturnCode getCfamRegister(const Target<K>& i_target,
                                      const uint32_t i_address,
                                      buffer<uint32_t>& o_data)
    {
        o_data = 0xFEED0CFA;
        std::cout << std::hex << "   getCfamRegister "
                  << "target: {" << i_target.getType() << ","
                  << uint64_t(i_target) << "}; "
                  << "address: " << i_address << "; "
                  << "output data: " << uint32_t(o_data)
                  << std::dec << std::endl;
        return FAPI2_RC_SUCCESS;
    }

    ///
    /// @brief Platform-level implementation of putCfamRegister()
    /// Hardware procedures writers will not call this function.
    /// @Tparam K template parameter, passed in target.
    /// @param[in]  i_target   HW target to operate on.
    /// @param[in]  i_address  CFAM address to write to.
    /// @param[out] i_data     32-bit buffer that holds data to write into address.
    /// @return fapi::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
    ///
    template< TargetType K >
    inline ReturnCode putCfamRegister(const Target<K>& i_target,
                                      const uint32_t i_address,
                                      const buffer<uint32_t> i_data)
    {
        std::cout << std::hex << "   putCfamRegister "
                  << "target: {" << i_target.getType() << ","
                  << uint64_t(i_target) << "}; "
                  << "address: " << i_address << "; "
                  << "input data: " << uint32_t(i_data)
                  << std::dec << std::endl;
        return FAPI2_RC_SUCCESS;
    }


    ///
    /// @brief Platform-level implementation of modifyCfamRegister()
    /// Hardware procedures writers will not call this function.
    /// @Tparam K template parameter, passed in target.
    /// @param[in]  i_target     HW target to operate on.
    /// @param[in]  i_address    CFAM register address to modify.
    /// @param[out] i_data       32-bit buffer that holds data to modify.
    /// @param[in]  i_modifyMode The modify mode (or/and/xor).
    /// @return fapi::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
    ///
    template< TargetType K >
    inline ReturnCode modifyCfamRegister(const Target<K>& i_target,
                                         const uint32_t i_address,
                                         const buffer<uint32_t> i_data,
                                         const fapi2::ChipOpModifyMode i_modifyMode)
    {
        std::cout << std::hex << "   modifyCfamRegister "
                  << "target: {" << i_target.getType() << ","
                  << uint64_t(i_target) << "}; "
                  << "address: " << i_address << "; "
                  << "input modifying data: " << uint32_t(i_data) << "; "
                  << "input ChipOpModifyMode: " << i_modifyMode
                  << std::dec << std::endl;
        return FAPI2_RC_SUCCESS;
    }

    // variable_buffer isn't supported on PPE
#ifndef __PPE__
    ///
    /// @brief Platform-level implementation of getRing()
    /// Hardware procedures writers will not call this function.
    /// @Tparam K template parameter, passed in target.
    /// @param[in]  i_target   HW target to operate on.
    /// @param[in]  i_address  Ring address to read from.
    /// @param[out] o_data     Buffer that holds ring data read from HW target.
    /// @param[in]  i_ringMode Ring operation mode.
    /// @return fapi::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
    ///
    template< TargetType K >
    inline ReturnCode getRing(const Target<K>& i_target,
                              const scanRingId_t i_address,
                              variable_buffer& o_data,
                              const RingMode i_ringMode)
    {
        o_data.setBit(0);
        o_data.setBit(3);
        std::cout << std::hex << "   getRing "
                  << "target: {" << i_target.getType() << ","
                  << uint64_t(i_target) << "}; "
                  << "ring address: " << i_address << "; "
                  << "ring mode: " << i_ringMode << "; "
                  << "first element of output data: " << o_data()[0]
                  << std::endl;

        return FAPI2_RC_SUCCESS;
    }

    /// @brief Platform-level implementation  of putRing()
    /// Hardware procedures writers will not call this function.
    /// @tparam K template parameter, passed in target.
    /// @param[in] i_target   Target to operate on.
    /// @param[in] i_address  Ring address to write to.
    /// @param[in] i_data     Buffer that contains RS4 compressed ring data
    ///                       to write into address
    /// @param[in] i_ringMode Ring operation mode.
    /// @return fapi::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
    template< TargetType K >
    inline ReturnCode putRing(const Target<K>& i_target,
                              const scanRingId_t i_address,
                              const variable_buffer& i_data,
                              const RingMode i_ringMode)
    {
        std::cout << std::hex << "   putRing "
                  << "target: {" << i_target.getType() << ","
                  << uint64_t(i_target) << "}; "
                  << "address: " << i_address << "; "
                  << "ring mode: " << i_ringMode << "; "
                  << "first element of the input data: " << i_data()[0]
                  << std::endl;

        return FAPI2_RC_SUCCESS;
    }

    /// @brief Platform-level implementation of modifyRing()
    /// @tparam K template parameter, passed in target.
    /// @param[in] i_target     Target to operate on.
    /// @param[in] i_address    Ring address to modify.
    /// @param[in] i_data       Buffer that contains RS4 compressed ring data
    ///                         to be modified.
    /// @param[in] i_modifyMode The modify mode (or/and/xor)
    /// @param[in] i_ringMode   Ring operation mode.
    /// @return fapi::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
    template< TargetType K >
    inline ReturnCode modifyRing(const Target<K>& i_target,
                                 const scanRingId_t i_address,
                                 const variable_buffer& i_data,
                                 const ChipOpModifyMode i_modifyMode,
                                 const RingMode i_ringMode)
    {
        std::cout << std::hex << "   modifyRing "
                  << "target: {" << i_target.getType() << ","
                  << uint64_t(i_target) << "}; "
                  << "address: " << i_address << "; "
                  << "input ChipOpModifyMode: " << i_modifyMode << "; "
                  << "ring mode: " << i_ringMode << "; "
                  << "first element of the input data: " << i_data()[0]
                  << std::endl;

        return FAPI2_RC_SUCCESS;
    }
#endif

#ifdef FAPI_SUPPORT_MULTI_SCOM
    /// @brief Performs a multiple SCOM operation
    ///        This interface performs multiple SCOM operations on a chip in the
    ///        order specified by the input MultiScom object.
    ///        See fapiMultiScom.H for details of how to populate the MultiScom
    ///        object with SCOM operations.
    ///
    /// @tparam K template parameter, passed in target.
    /// @param[in]     i_target Target to operate on.
    /// @param[in,out] io_multiScomObj Reference to a MultiScom object,
    ///                                pre-populated with SingleScomInfo entries
    ///                                to perform multiple SCOMs on input target
    /// @return fapi2::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
    ///
    /// @note   This is a synchronous interface and would return after all the
    ///         SCOM operations are completed or on the first failed operation
    ///
    /// @note   SCOMs will be performed in the order they were added to the
    ///         input MultiScom object
    ///
    /// @note   In case of errors, the platform code is responsible to collect
    ///         and add all the required error info and FFDC into the error data
    ///         for debugging
    ///
    /// @note   If the SCOM operations added are specific to a processor chip,
    ///         then the FSI Shift Engine configured in scatter-gather DMA mode
    ///         extension would be used to execute the SCOM operations in a
    ///         performance optimize mode. In this mode, the special
    ///         SCOM_BULK_READ_MODE and SCOM_BULK_WRITE_MODE operations are
    ///         supported that allow a large bulk of SCOM access (in multiple of
    ///         64 bits) for targets that support auto-increment. The
    ///         SCOM_WRITE_UNDER_MASK operation is not supported in this mode
    ///
    /// @note   If the SCOM operations added are specific to a memory buffer
    ///         chip, then the regular SCOM engine is used to execute the SCOM
    ///         operations. SCOM_WRITE_UNDER_MASK operation is supported in
    ///         this mode, but the special SCOM_BULK_READ_MODE and
    ///         SCOM_BULK_WRITE_MODE operations are not supported due to
    ///         hardware limitations.
    ///
    template< TargetType K >
    fapi2::ReturnCode multiScom (const Target<K>& i_target,
                                 MultiScom&  io_multiScomObj)
    {
    }
#endif

    // --------------------------------------------------------------------------
    // NOTE:
    // Implement platform Spy access functions if platform supports them.
    // --------------------------------------------------------------------------

    // variable_buffer isn't supported on PPE
#ifndef __PPE__
    /// @brief Reads a spy from a chip.
    /// @tparam K template parameter, passed in target.
    /// @param[in]  i_target   Target to operate on.
    /// @param[in]  i_spyId    Id of the spy whose data to be read.
    /// @param[out] o_data     Buffer that holds data read from HW target.
    /// @return fapi2::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
    ///
    /// @note:  The string version is only supported for cronus.
    ///
    ///         The fapi design to support both FSP and cronus use of get and
    ///         put spy functions is dependant on the SPY names being expanded
    ///         to resemble a valid C identifier. This design places some
    ///         restrictions on the SPY names which can be used.
    ///
    ///         1. if the spy name contains a # procedure writers should replace
    ///         it with an __P__ for example -
    ///
    ///         ABUS.RX0.RXPACKS#0.RXPACK.RD.LC.LC.ACT_DIS
    ///         becomes
    ///         ABUS.RX0.RXPACKS__P__0.RXPACK.RD.LC.LC.ACT_DIS
    ///
    ///         2. if the spy name has a number following a "." it must have an
    ///         underscore prepended to the number.
    ///
    ///         EH.TPCHIP.2KX100_ARY_CLK_EDGES_DLY
    ///         becomes
    ///         EH.TPCHIP._2KX100_ARY_CLK_EDGES_DLY
    ///
    ///         Example SPY name:
    ///         The hardware procedure should call the function like:
    ///
    ///          ABUS.RX0.RXPACKS#0.RXPACK.RD.LC.LC.ACT_DIS
    ///
    ///          fapi2::ReturnCode rc = fapiGetSpy( targ,
    ///                  ABUS.RX0.RXPACKS__P__0.RXPACK.RD.LC.LC.ACT_DIS, data );
    ///
    /// @note The ID is not in quotes the fapi code will handle adding
    ///       the quotes for the cronus environment
    ///
#ifdef FAPI_SUPPORT_SPY_AS_ENUM
    template< TargetType K >
    inline ReturnCode getSpy(const Target<K>& i_target,
                             const spyId_t i_spyId,
                             variable_buffer& o_data)
    {
        static_assert(K == 0, "implement getSpy (string)");
        return ~FAPI2_RC_SUCCESS;
    }
#endif
#ifdef FAPI_SUPPORT_SPY_AS_STRING
    template< TargetType K >
    inline ReturnCode getSpy(const Target<K>& i_target,
                             const char * const i_spyId,
                             variable_buffer& o_data)
    {
        static_assert(K == 0, "implement getSpy (string)");
        return ~FAPI2_RC_SUCCESS;
    }
#endif
    /// @brief Writes a spy on a chip.
    /// @tparam K template parameter, passed in target.
    /// @param[in]  i_target   Target to operate on.
    /// @param[in]  i_spyId    Id of the spy to write data to.
    /// @param[out] i_data     Buffer that holds data to write into spy.
    /// @return fapi2::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
    ///
    /// @note:  The string version is only supported for cronus.
    ///
    ///         The fapi design to support both FSP and cronus use of get and
    ///         put spy functions is dependent on the SPY names being expanded
    ///         to resemble a valid C identifier. This design places some
    ///         restrictions on the SPY names which can be used.
    ///
    ///         1. if the spy name contains a # procedure writers should replace
    ///         is with an __P__ for example -
    ///
    ///         ABUS.RX0.RXPACKS#0.RXPACK.RD.LC.LC.ACT_DIS
    ///         becomes
    ///         ABUS.RX0.RXPACKS__P__0.RXPACK.RD.LC.LC.ACT_DIS
    ///
    ///         2. if the spy name has a number following a "." it must have an
    ///         underscore prepended to the number.
    ///
    ///         EH.TPCHIP.2KX100_ARY_CLK_EDGES_DLY
    ///         becomes
    ///         EH.TPCHIP._2KX100_ARY_CLK_EDGES_DLY
    ///
    ///         Example SPY name:
    ///         The hardware procedure should call the function like:
    ///
    ///          ABUS.RX0.RXPACKS#0.RXPACK.RD.LC.LC.ACT_DIS
    ///
    ///          fapi2::ReturnCode rc = fapiPutSpy( targ,
    ///                  ABUS.RX0.RXPACKS__P__0.RXPACK.RD.LC.LC.ACT_DIS, data );
    ///
    /// @note The ID is not in quotes the fapi code will handle adding
    ///         the quotes for the cronus environment
    ///
#ifdef FAPI_SUPPORT_SPY_AS_ENUM
    template< TargetType K >
    inline ReturnCode putSpy(const Target<K>& i_target,
                             const spyId_t i_spyId,
                             variable_buffer& i_data)
    {
        static_assert(K == 0, "implement putSpy (enum)");
        return ~FAPI2_RC_SUCCESS;
    }
#endif
#ifdef FAPI_SUPPORT_SPY_AS_STRING
    template< TargetType K >
    inline ReturnCode putSpy(const Target<K>& i_target,
                             const char* const i_spyId,
                             variable_buffer& i_data)
    {
        static_assert(K == 0, "implement putSpy (string)");
        return ~FAPI2_RC_SUCCESS;
    }
#endif
    /// @brief Writes spy data into a buffer holding ring data image
    ///        This API is used by L2/L3 repair to put column repair data
    ///        into a ring buffer image.
    /// @tparam K template parameter, passed in target.
    /// @param[in]  i_target Target to operate on.
    /// @param[in]  i_spyId  Id of the spy.
    /// @param[in]  i_data   Buffer that holds spy data to write into ring
    ///                      image.
    /// @param[out] o_data   Buffer that holds updated ring image.
    /// @return fapi2::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
    ///
    /// @note:  The string version is only supported for cronus.
    ///
    ///         The fapi design to support both FSP and cronus use of get and
    ///         put spy functions is dependent on the SPY names being expanded
    ///         to resemble a valid C identifier. This design places some
    ///         restrictions on the SPY names which can be used.
    ///
    ///         See fapiPutSpy for details on spy id specifics.
    ///
#ifdef FAPI_SUPPORT_SPY_AS_ENUM
    template< TargetType K >
    inline ReturnCode putSpyImage(const Target<K>& i_target,
                                  const spyId_t i_spyId,
                                  const variable_buffer& i_data,
                                  variable_buffer& o_imageData)
    {
        static_assert(K == 0, "implement putSpyImage (enum)");
        return ~FAPI2_RC_SUCCESS;
    }
#endif
#ifdef FAPI_SUPPORT_SPY_AS_STRING
    template< TargetType K >
    inline ReturnCode putSpyImage(const Target<K>& i_target,
                                  const char* const i_spyId,
                                  const variable_buffer& i_data,
                                  variable_buffer& o_imageData)
    {
        static_assert(K == 0, "implement putSpyImage (string)");
        return ~FAPI2_RC_SUCCESS;
    }
#endif
    /// @brief Reads spy data from a ring image buffer
    /// @param[in]   i_target      Target to operate on
    /// @param[in]   i_spyId       The spy's id
    /// @param[out]  o_data        Buffer that holds data read from ring image.
    /// @param[in]   i_imageData   Buffer that holds ring image to read data
    ///                            from.
    /// @return fapi2::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
    ///
    /// @note:  The string version is only supported for cronus.
    ///
    ///         The fapi design to support both FSP and cronus use of get and
    ///         put spy functions is dependent on the SPY names being expanded
    ///         to resemble a valid C identifier. This design places some
    ///         restrictions on the SPY names which can be used.
    ///
    ///         See fapiPutSpy for details on spy id specifics.
    ///
#ifdef FAPI_SUPPORT_SPY_AS_ENUM
    template< TargetType K >
    inline ReturnCode getSpyImage(const Target<K>& i_target,
                                  const spyId_t i_spyId,
                                  variable_buffer& o_data,
                                  const variable_buffer& i_imageData)
    {
        static_assert(K == 0, "implement getSpyImage (enum)");
        return ~FAPI2_RC_SUCCESS;
    }
#endif
#ifdef FAPI_SUPPORT_SPY_AS_STRING
    template< TargetType K >
    inline ReturnCode getSpyImage(const Target<K>& i_target,
                                  const char * const i_spyId,
                                  variable_buffer& o_data,
                                  const variable_buffer& i_imageData)
    {
        static_assert(K == 0, "implement getSpyImage (string)");
        return ~FAPI2_RC_SUCCESS;
    }
#endif

#endif // PPE

};

#endif // _FAPI2_HWACCESS_H_
