/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/importtemp/fapi2/include/target.H $                   */
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
/**
 * @file target.H
 * @brief platform specializations for fapi2 targets
 */

#ifndef __FAPI2_TARGET__
#define __FAPI2_TARGET__

#include <plat_target.H>
#include <fapi2_target.H>
#include <stdio.h>

namespace fapi2
{

    ///
    /// @brief Assignment Operator.
    /// @param[in] i_right Reference to Target to assign from.
    /// @return Reference to 'this' Target
    ///
    template<TargetType K, typename V>
    Target<K, V>& Target<K, V>::operator=(const Target& i_right)
    { iv_handle = i_right.iv_handle; return *this; }

    ///
    /// @brief Equality Comparison Operator
    /// @param[in] i_right Reference to Target to compare.
    /// @return bool. True if equal.
    /// @note Platforms need to define this so that the physical
    /// targets are determined to be equivilent rather than just the handles
    ///
    template<TargetType K, typename V>
    bool Target<K, V>::operator==(const Target& i_right) const
    { return i_right.iv_handle == iv_handle; }

    ///
    /// @brief Inquality Comparison Operator
    /// @param[in] i_right Reference to Target to compare.
    /// @return bool. True if not equal.
    /// @note Platforms need to define this so that the physical
    /// targets are determined to be equivilent rather than just the handles
    ///
    template<TargetType K, typename V>
    bool Target<K, V>::operator!=(const Target& i_right) const
    { return i_right.iv_handle != iv_handle; }

    ///
    /// @brief Get this target's immediate parent
    /// @tparam T The type of the parent
    /// @return Target<T> a target representing the parent
    ///
    template<TargetType K, typename V>
    template<TargetType T>
    inline Target<T> Target<K, V>::getParent(void) const
    {
        // For testing
        return Target<T>(iv_handle);
    }

    ///
    /// @brief Get this target's children
    /// @tparam T The type of the parent
    /// @param[in] i_state The desired TargetState of the children
    /// @return std::vector<Target<T> > a vector of present/functional
    /// children
    /// @warning The children of EX's (cores) are expected to be returned
    /// in order. That is, core 0 is std::vector[0].
    ///
    template<TargetType K, typename V>
    template< TargetType T>
    inline std::vector<Target<T> >
    Target<K, V>::getChildren(const TargetState i_state) const
    {
        // To keep the compiler quiet about unused variables
        static_cast<void>(i_state);
        // For testing
        return {Target<T>(), Target<T>()};
    }

    ///
    /// @brief Get the target at the other end of a bus - dimm included
    /// @tparam T The type of the parent
    /// @param[in] i_state The desired TargetState of the children
    /// @return Target<T> a target representing the thing on the other end
    /// @note Can be easily changed to a vector if needed
    ///
    template<TargetType K, typename V>
    template<TargetType T>
    inline Target<T>
    Target<K, V>::getOtherEnd(const TargetState i_state) const
    {
        // Implementation note: cast to a composite of
        // bus types and the compiler will check if this is
        // a good function at compile time
        return Target<T>();
    }


    ///
    /// @brief Return the string interpretation of this target
    /// @tparam T The type of the target
    /// @param[in] i_target Target<T>
    /// @param[in] i_buffer buffer to write in to
    /// @param[in] i_bsize size of the buffer
    /// @return void
    /// @post The contents of the buffer is replaced with the string
    /// representation of the target
    ///
    template< TargetType T >
    inline void toString(const Target<T>& i_target, char* i_buffer, size_t i_bsize)
    {
        snprintf(i_buffer, i_bsize, "Target 0x%lx/0x%x", i_target.get(), T);
    }

    ///
    /// @brief Return the string interpretation of this target
    /// @tparam T The type of the target
    /// @tparam B The type of the buffer
    /// @param[in] A pointer to the Target<T>
    /// @param[in] i_buffer buffer to write in to
    /// @param[in] i_bsize size of the buffer
    /// @return void
    /// @post The contents of the buffer is replaced with the string
    /// representation of the target
    ///
    template< TargetType T >
    inline void toString(const Target<T>* i_target, char* i_buffer, size_t i_bsize)
    {
        snprintf(i_buffer, i_bsize, "Target 0x%lx/0x%x", i_target->get(), T);
    }

    ///
    /// @brief Get an enumerated target of a specific type
    /// @tparam T The type of the target
    /// @param[in] Ordinal representing the ordinal number of
    /// the desired target
    /// @return Target<T> the target requested
    ///
    template<TargetType T>
    inline Target<T> getTarget(uint64_t Ordinal)
    {
        // For testing
        return Target<T>(Ordinal);
    }
}

#endif
