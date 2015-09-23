/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/hwpf/plat/include/target.H $                          */
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
#include <plat_target_parms.H>
#include <fapi2_target.H>
#include <plat_trace.H>
#include <utils.H>
#include <stdio.h>
#include <stdint.h>
#include <vector>

extern "C"
{
    extern std::vector<fapi2::plat_target_handle_t> G_vec_targets;
}

namespace fapi2
{
    /// @brief Create a Target, with a value
    /// @param[in] Value the value (i.e., specific element this
    /// target represents, or pointer)
    /// @note Platforms can mangle the value and K to get a
    /// single uint64_t in value which represents all the information
    /// they might need. value( K | V ), for example
    ///
    template<TargetType K, typename V>
    Target<K, V>::Target(V Value)
    {
        static_assert( ((K == TARGET_TYPE_CORE) &
                            (K == TARGET_TYPE_EQ)       ) != true,
            "TARGET_TYPE_CORE and TARGET_TYPE_EQ cannot be specified at the same time");

        this->iv_handle.value = 0;
        if(K & TARGET_TYPE_PROC_CHIP)
        {
            this->iv_handle.fields.chiplet_num = 0;
            this->iv_handle.fields.type = TARGET_TYPE_PROC_CHIP;
            this->iv_handle.fields.type_target_num = 0;
        }

        if(K & TARGET_TYPE_PERV)
        {
            this->iv_handle.fields.chiplet_num = Value;
            this->iv_handle.fields.type = TARGET_TYPE_PERV;
            this->iv_handle.fields.type_target_num = Value;
        }

        if(K & TARGET_TYPE_CORE)
        {
            /*
            if (Value > CORE_CHIPLET_COUNT)
            {
               PK_TRACE("Chiplet number is greater than CORE_CHIPLET_COUNT");
               return -1;
            }
            */
            this->iv_handle.fields.chiplet_num = Value + CORE_CHIPLET_OFFSET;
            this->iv_handle.fields.type = TARGET_TYPE_CORE | TARGET_TYPE_PERV;
            this->iv_handle.fields.type_target_num = Value;
        }

        if(K & TARGET_TYPE_EQ)
        {
            this->iv_handle.fields.chiplet_num = Value + EQ_CHIPLET_OFFSET;
            this->iv_handle.fields.type = TARGET_TYPE_EQ | TARGET_TYPE_PERV;
            this->iv_handle.fields.type_target_num = Value;
        }

        if(K & TARGET_TYPE_EX)
        {

            this->iv_handle.fields.chiplet_num = (Value / 2) + EX_CHIPLET_OFFSET;
            this->iv_handle.fields.type = TARGET_TYPE_EX | TARGET_TYPE_PERV;
            this->iv_handle.fields.type_target_num = Value;
        }

        if(K & TARGET_TYPE_MCS)
        {

            this->iv_handle.fields.chiplet_num = Value + MCS_CHIPLET_OFFSET;
            this->iv_handle.fields.type = TARGET_TYPE_MCS | TARGET_TYPE_PERV;
            this->iv_handle.fields.type_target_num = Value;
        }

//         if(K & TARGET_TYPE_EQ_MC_WRITE)
//         {
//             this->iv_handle.fields.chiplet_num =
//                             (((MC_ENABLE) |
//                              ((MC_WRITE << 3) | (Value & 0x07))) &
//                               BITS(57,7));
//             this->iv_handle.fields.type = TARGET_TYPE_EQ_MC_WRITE;
//         }
//
//         if(K & TARGET_TYPE_EQ_MC_READOR)
//         {
//             this->iv_handle.fields.chiplet_num =
//                            (((MC_ENABLE) |
//                             ((MC_READ_OR << 3) | (Value & 0x07))) &
//                              BITS(57,7));
//             this->iv_handle.fields.type = TARGET_TYPE_EQ_MC_READOR;
//         }
//
//         if(K & TARGET_TYPE_EQ_MC_READAND)
//         {
//             this->iv_handle.fields.chiplet_num =
//                             (((MC_ENABLE) |
//                              ((MC_READ_AND << 3) | (Value & 0x07))) &
//                               BITS(57,7));
//             this->iv_handle.fields.type = TARGET_TYPE_EQ_MC_READAND;
//         }
//
//         if(K & TARGET_TYPE_CORE_MC_WRITE)
//         {
//             this->iv_handle.fields.chiplet_num =
//                             (((MC_ENABLE) |
//                              ((MC_WRITE << 3) | (Value & 0x07))) &
//                              BITS(57,7));
//             this->iv_handle.fields.type = TARGET_TYPE_CORE_MC_WRITE;
//         }
//
//         if(K & TARGET_TYPE_CORE_MC_READOR)
//         {
//             this->iv_handle.fields.chiplet_num =
//                             (((MC_ENABLE) |
//                              ((MC_READ_OR << 3) | (Value & 0x07))) &
//                              BITS(57,7));
//             this->iv_handle.fields.type = TARGET_TYPE_CORE_MC_READOR;
//         }
//
//         if(K & TARGET_TYPE_CORE_MC_READAND)
//         {
//             this->iv_handle.fields.chiplet_num =
//                             (((MC_ENABLE) |
//                              ((MC_READ_AND << 3) | (Value & 0x07))) &
//                               BITS(57,7));
//             this->iv_handle.fields.type = TARGET_TYPE_CORE_MC_READAND;
//         }

        if(K == TARGET_TYPE_ALL)
        {
            this->iv_handle.fields.chiplet_num = Value;
            this->iv_handle.fields.type = TARGET_TYPE_ALL;
        }
        this->iv_handle.fields.present = 1;
        this->iv_handle.fields.functional = 1;
        this->iv_handle.fields.address_overlay =
                (this->iv_handle.fields.chiplet_num << 24);

    }

    ///
    /// @brief Assignment Operator.
    /// @param[in] i_right Reference to Target to assign from.
    /// @return Reference to 'this' Target
    ///
    template<TargetType K, typename V>
    Target<K, V>& Target<K, V>::operator=(const Target& i_right)
        {
            this->iv_handle.value = i_right->iv_handle.value;
            return *this;
        }
    ///
    /// @brief Equality Comparison Operator
    /// @param[in] i_right Reference to Target to compare.
    /// @return bool. True if equal.
    /// @note Platforms need to define this so that the physical
    /// targets are determined to be equivilent rather than just the handles
    ///
    template<TargetType K, typename V>
    bool Target<K, V>::operator==(const Target& i_right) const
    {
        if (this->iv_handle.value == i_right->iv_handle.value)
            return true;
        else
            return false;
    }

    ///
    /// @brief Inquality Comparison Operator
    /// @param[in] i_right Reference to Target to compare.
    /// @return bool. True if not equal.
    /// @note Platforms need to define this so that the physical
    /// targets are determined to be equivilent rather than just the handles
    ///
    template<TargetType K, typename V>
    bool Target<K, V>::operator!=(const Target& i_right) const
    {
        if (this->iv_handle.value != i_right->iv_handle.value)
            return true;
        else
            return false;
    }

    ///
    /// @brief Get this target's immediate parent
    /// @tparam T The type of the parent
    /// @return Target<T> a target representing the parent
    ///
    template<TargetType K, typename V>
    template<TargetType T>
    inline Target<T> Target<K, V>::getParent(void) const
    {
        return this->iv_handle.value;
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
#define INVALID_CHILD(PARENT, CHILD) \
        static_assert(!((K == PARENT) && (T == CHILD)), \
                  #CHILD " is not a child of " #PARENT );

        // invalid children for proc
//        INVALID_CHILD(fapi2::TARGET_TYPE_PROC_CHIP, fapi2::TARGET_TYPE_NONE)
//        INVALID_CHILD(fapi2::TARGET_TYPE_PROC_CHIP, fapi2::TARGET_TYPE_SYSTEM)
//        INVALID_CHILD(fapi2::TARGET_TYPE_PROC_CHIP, fapi2::TARGET_TYPE_DIMM)
//        INVALID_CHILD(fapi2::TARGET_TYPE_PROC_CHIP, fapi2::TARGET_TYPE_PROC_CHIP)
//        INVALID_CHILD(fapi2::TARGET_TYPE_PROC_CHIP, fapi2::TARGET_TYPE_MEMBUF_CHIP)
//        INVALID_CHILD(fapi2::TARGET_TYPE_PROC_CHIP, fapi2::TARGET_TYPE_MBA)
#undef INVALID_CHILD

#define INVALID_PARENT(PARENT) \
        static_assert(!((K == PARENT)), \
                  #PARENT "is not supported on PPE platforms");

        // invalid parents
//         INVALID_PARENT(fapi2::TARGET_TYPE_SYSTEM)
//         INVALID_PARENT(fapi2::TARGET_TYPE_MEMBUF_CHIP)
//         INVALID_PARENT(fapi2::TARGET_TYPE_L4)
//         INVALID_PARENT(fapi2::TARGET_TYPE_DIMM)
//         INVALID_PARENT(fapi2::TARGET_TYPE_MCA)
//         INVALID_PARENT(fapi2::TARGET_TYPE_MBA)
//         INVALID_PARENT(fapi2::TARGET_TYPE_MI)
//         INVALID_PARENT(fapi2::TARGET_TYPE_MCBIST)
//         INVALID_PARENT(fapi2::TARGET_TYPE_DMI)
#undef INVALID_PARENT

        // valid children for EQ
        // EQ -> CORE
        // EQ -> EX
        static_assert(!((K == fapi2::TARGET_TYPE_EQ) &&
                        (T != fapi2::TARGET_TYPE_CORE) &&
                        (T != fapi2::TARGET_TYPE_EX)),
                      "improper child of fapi2::TARGET_TYPE_EQ");

        // valid children for EX
        // EX -> CORE
        static_assert(!((K == fapi2::TARGET_TYPE_EX) &&
                        (T != fapi2::TARGET_TYPE_CORE)),
                      "improper child of fapi2::TARGET_TYPE_EX");

        // Nimbus Memory
        // valid children for MCS
        // MCS -> MCA
//         static_assert(!((K == fapi2::TARGET_TYPE_MCS) &&
//                         (T != fapi2::TARGET_TYPE_MCA)),
//                       "improper child of fapi2::TARGET_TYPE_MCS");


        std::vector<fapi2::plat_target_handle_t>::iterator l_iter;
        FAPI_DBG("getChildren: initializing children vector");
        std::vector<Target<T> > l_children;
        
        
       
        uint32_t c = 0;
        for (l_iter = G_vec_targets.begin(); l_iter != G_vec_targets.end(); ++l_iter)
        {

            Target<T> * l_temp =  reinterpret_cast< Target<T>* >(l_iter);
            if (((*l_temp).getTargetType() & T) == T)
            {
                switch (i_state)
                {
                    case TARGET_STATE_PRESENT:                
                        if ((*l_temp).getPresent())
                        {
                            l_children.push_back((*l_temp));
//                            FAPI_DBG("Pushing getChildren present 0x%08X", (uint32_t)(((*l_temp)).get()>>32));
                        }
                        break;
                    case TARGET_STATE_FUNCTIONAL:
                        if ((*l_temp).getFunctional())
                        {
                            l_children.push_back((*l_temp));
//                            FAPI_DBG("Pushing getChildren functional 0x%08X", (uint32_t)(((*l_temp)).get()>>32));
                        }
                        break;
                    default:
                        FAPI_ERR("Coming error ASSERT for illegal i_state = %u", i_state);
                }
            }
            ++c;
        }

        return l_children;
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
//        static_assert( false, "getOtherEnd() is not supported on PPE platforms");
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
