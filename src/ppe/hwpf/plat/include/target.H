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
 * @brief definitions for fapi2 targets
 */

#ifndef __FAPI2_TARGET__
#define __FAPI2_TARGET__

#include <stdint.h>
//#include <vector>
#include <target_types.H>
#include <utils.H>
#include "plat_target_parms.H"

#define MC_ENABLE   0x1 << 6
#define MC_WRITE    5
#define MC_READ_OR  0
#define MC_READ_AND 1

#define EX_ADDRESS_MASK 0x0000FF00

#ifdef __ASSEMBLER__

#ifndef ULL
#define ULL(x) x
#endif

#else

#ifndef ULL
#define ULL(x) x##ull

#endif

#endif // __ASSEMBLER

/// Create a multi-bit mask of \a n bits starting at bit \a b
#define BITS(b, n) ((ULL(0xffffffffffffffff) << (64 - (n))) >> (b))

/// Create a single bit mask at bit \a b
#define BIT(b) BITS((b), 1)


namespace fapi2
{
    ///
    /// @brief Class representing a FAPI2 Target
    /// @tparam K the type (Kind) of target
    /// @tparam V the type of the target's Value
    /// @remark TargetLite targets are uint64_t, Targets
    /// are uintptr_t (void*).
    ///
    /// Assuming there are representations of a processor,
    /// a membuf and a system here are some examples:
    /// @code
    /// #define PROCESSOR_CHIP_A 0xFFFF0000
    /// #define MEMBUF_CHIP_B    0x0000FFFF
    /// #define SYSTEM_C         0x0000AAAA
    /// @endcode
    ///
    /// * To define a target:
    /// @code
    /// fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP> A(PROCESSOR_CHIP_A);
    /// fapi2::Target<fapi2::TARGET_TYPE_SYSTEM> C(SYSTEM_C);
    /// fapi2::Target<fapi2::TARGET_TYPE_MEMBUF_CHIP> B(MEMBUF_CHIP_B);
    /// @endcode
    ///
    /// * Functions which take composite target types
    /// @code
    /// void takesProcOrMembuf(
    /// const fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP |
    ///                     fapi2::TARGET_TYPE_MEMBUF_CHIP>& V );
    ///
    /// void takesAny(const fapi2::Target<fapi2::TARGET_TYPE_ALL>& V );
    ///
    /// @endcode
    ///
    /// * Traversing the target "tree"
    /// @code
    /// fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP> A(PROCESSOR_CHIP_A);
    ///
    /// // Get A's parent
    /// A.getParent<fapi2::TARGET_TYPE_SYSTEM>();
    ///
    /// // Get the 0x53'd core
    /// fapi2::getTarget<fapi2::TARGET_TYPE_CORE>(0x53);
    ///
    /// // Get all *my* present/functional children which are cores
    /// A.getChildren<fapi2::TARGET_TYPE_CORE>();
    ///
    /// // Get all of the the cores relative to my base target
    /// fapi2::getChildren<fapi2::TARGET_TYPE_CORE>();
    /// @endcode
    ///
    /// * Invalid casts
    /// @code
    /// // Can't cast to a specialized target
    /// fapi2::Target<fapi2::TARGET_TYPE_NONE> D(MEMBUF_CHIP_B);
    /// takesProcOrMembuf( D );
    ///
    /// // Not one of the shared types
    /// fapi2::Target<fapi2::TARGET_TYPE_ABUS_ENDPOINT> E;
    /// takesProcOrMembuf( E );
    /// @endcode
    template<TargetType K, typename V = uint64_t>
    class Target
    {
    public:     
       
        ///
        /// @brief Create a Target, with a value
        /// @param[in] V the value (i.e., specific element this
        /// target represents, or pointer)
        /// @note Platforms can mangle the value and K to get a
        /// single uint64_t in value which represents all the information
        /// they might need. value( K | V ), for example
        ///
        
        /// Default constructor is basically a TARGET_TYPE_ALL
        Target()
        {
            this->iv_handle.fields.chiplet_num = 0;
//            this->iv_address.fields.chiplet_num = this->iv_handle.fields.chiplet_num;             
        }
        
        Target( V Value )
        {
            static_assert( ((K == TARGET_TYPE_CORE) &
                                (K == TARGET_TYPE_EQ)       ) != true,
                "TARGET_TYPE_CORE and TARGET_TYPE_EQ cannot be specified at the same time");
            
            if(K & TARGET_TYPE_PROC_CHIP)
            {
                this->iv_handle.fields.chiplet_num = Value;
                this->iv_handle.fields.type = TARGET_TYPE_PROC_CHIP;
                this->iv_handle.fields.type_target_num = Value;  // TODO:  check this
//                this->iv_address.fields.chiplet_num = Value;
            }

            if(K & TARGET_TYPE_PERV)
            {
                this->iv_handle.fields.chiplet_num = Value;
                this->iv_handle.fields.type = TARGET_TYPE_PERV | TARGET_TYPE_PROC_CHIP; 
                this->iv_handle.fields.type_target_num = Value;  // TODO:  check this
//                this->iv_address.fields.chiplet_num = this->iv_handle.fields.chiplet_num;
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
                this->iv_handle.fields.type = TARGET_TYPE_CORE;
                this->iv_handle.fields.type_target_num = Value;
//                this->iv_address.fields.chiplet_num = this->iv_handle.fields.chiplet_num;
            }

            if(K & TARGET_TYPE_EQ)
            {
                this->iv_handle.fields.chiplet_num = Value + EQ_CHIPLET_OFFSET;
                this->iv_handle.fields.type = TARGET_TYPE_EQ;
                this->iv_handle.fields.type_target_num = Value;
//                this->iv_address.fields.chiplet_num = this->iv_handle.fields.chiplet_num;
            }
            
            if(K & TARGET_TYPE_EX)
            {                                           
                               
                this->iv_handle.fields.chiplet_num = (Value / 2) + EX_CHIPLET_OFFSET;               
                this->iv_handle.fields.type = TARGET_TYPE_EX;
                this->iv_handle.fields.type_target_num = Value;
//                this->iv_address.fields.chiplet_num = this->iv_handle.fields.chiplet_num;
            }
            
            if(K & TARGET_TYPE_EQ_MC_WRITE)
            {
                this->iv_handle.fields.chiplet_num =  
                                (((MC_ENABLE) | 
                                 ((MC_WRITE << 3) | (Value & 0x07))) &
                                  BITS(57,7));
                this->iv_handle.fields.type = TARGET_TYPE_EQ_MC_WRITE;
//                this->iv_address.fields.chiplet_num = this->iv_handle.fields.chiplet_num;              
            }
            
            if(K & TARGET_TYPE_EQ_MC_READOR)
            {
                this->iv_handle.fields.chiplet_num = 
                               (((MC_ENABLE) | 
                                ((MC_READ_OR << 3) | (Value & 0x07))) &
                                 BITS(57,7));
                this->iv_handle.fields.type = TARGET_TYPE_EQ_MC_READOR;
//                this->iv_address.fields.chiplet_num = this->iv_handle.fields.chiplet_num;                
            }
            
            if(K & TARGET_TYPE_EQ_MC_READAND)
            {
                this->iv_handle.fields.chiplet_num = 
                                (((MC_ENABLE) | 
                                 ((MC_READ_AND << 3) | (Value & 0x07))) &
                                  BITS(57,7));
                this->iv_handle.fields.type = TARGET_TYPE_EQ_MC_READAND;
//                this->iv_address.fields.chiplet_num = this->iv_handle.fields.chiplet_num;               
            }
            
            if(K & TARGET_TYPE_CORE_MC_WRITE)
            {
                this->iv_handle.fields.chiplet_num = 
                                (((MC_ENABLE) | 
                                 ((MC_WRITE << 3) | (Value & 0x07))) &
                                 BITS(57,7));
                this->iv_handle.fields.type = TARGET_TYPE_CORE_MC_WRITE;
//                this->iv_address.fields.chiplet_num = this->iv_handle.fields.chiplet_num;
            }
 
            if(K & TARGET_TYPE_CORE_MC_READOR)
            {
                this->iv_handle.fields.chiplet_num = 
                                (((MC_ENABLE) | 
                                 ((MC_READ_OR << 3) | (Value & 0x07))) &
                                 BITS(57,7));
                this->iv_handle.fields.type = TARGET_TYPE_CORE_MC_READOR;
//                this->iv_address.fields.chiplet_num = this->iv_handle.fields.chiplet_num;
            }
 
            if(K & TARGET_TYPE_CORE_MC_READAND)
            {
                this->iv_handle.fields.chiplet_num = 
                                (((MC_ENABLE) | 
                                 ((MC_READ_AND << 3) | (Value & 0x07))) &
                                  BITS(57,7));
                this->iv_handle.fields.type = TARGET_TYPE_CORE_MC_READAND;
//                this->iv_address.fields.chiplet_num = this->iv_handle.fields.chiplet_num;               
            }
            
            if(K == TARGET_TYPE_ALL)
            {
                this->iv_handle.fields.chiplet_num = Value;
                this->iv_handle.fields.type = TARGET_TYPE_ALL;
//                this->iv_address.fields.chiplet_num = this->iv_handle.fields.chiplet_num;
            }
            
//             if(K == TARGET_TYPE_ADDRESS)
//             {
//                 this->iv_handle.fields.chiplet_num = Value;
//                 this->iv_address.fields.chiplet_num = this->iv_handle.fields.chiplet_num;
//             }
        }          

        ///
        /// @brief Assignment Operator.
        /// @param[in] i_right Reference to Target to assign from.
        /// @return Reference to 'this' Target
        ///
        Target<K,V>& operator=(const Target<K,V>& i_right)
        {
            this->iv_handle.value = i_right->iv_handle.value;
            this->iv_addresss.value = i_right->iv_address.value;
            return *this;
        }     
        
        ///
        /// @brief Equality Comparison Operator
        /// @param[in] i_right Reference to Target to compare.
        /// @return bool. True if equal.
        ///
        bool operator==(const Target<K,V>& i_right) const
        {
            if (this->iv_handle.value == i_right->iv_handle.value)
                return true;
            else
                return false;
        }
        ///
        /// @brief Inquality Comparison Operator
        /// @param[in] i_right Reference to Target to compare.
        /// @return bool. True if equal.
        ///
        bool operator!=(const Target<K,V>& i_right) const
        {
            if (this->iv_handle.value != i_right->iv_handle.value)
                return true;
            else
                return false;
        }

        ///
        /// @brief Get the handle.
        /// @return V The target's handle, or value
        ///
        V get(void) const
         {
            return this->iv_handle.value;
        }
 
        ///
        /// @brief Get the handle as a V
        /// @return V The target's handle, or value
        ///
        operator V() const
        {
            return this->iv_handle.value;
        }
        
        ///
        /// @brief Get a target's value
        /// @return V The target's handle, or value
        ///
        V& operator()(void)
        {
            return *this->iv_handle;
        }

        ///
        /// @brief Get the target type
        /// @return The type of target represented by this target
        ///
        TargetType getType(void) const
        {
            return (TargetType)this->iv_handle.fields.type;
        }
        
        ///
        /// @brief Get address overlay to reduce runtime processing
        /// @return Overlay as a type V
        ///
        V getAddressOverlay(void) const
        {
            return this->iv_handle.fields.address_overlay;
        }
        
        
#if 0
        ///
        /// @brief Get this target's immediate parent
        /// @tparam T The type of the parent
        /// @return Target<T> a target representing the parent
        ///
        template<TargetType T>
        Target<T,V> getParent(void) const
            {
                static_assert((T & ~(TARGET_TYPE_PROC_CHIP | TARGET_TYPE_PERV) != 0),
                "Only TARGET_TYPE_PROC_CHIP and TARGET_TYPE_PERV parent types supported"); 
                
                static_assert( ((((T & TARGET_TYPE_PROC_CHIP) == 0) &
                                  (T & TARGET_TYPE_PERV     ) == 0)   ),
                "Either TARGET_TYPE_PROC_CHIP or TARGET_TYPE_PERV parent type must be specified");                 
            }

        ///
        /// @brief Get this target's children
        /// @tparam T The type of the parent
        /// @return std::vector<Target<T> > a vector of present/functional
        /// children
        ///
        template< TargetType T>
        std::vector<Target<T,V> > getChildren(void) const
            {
                static_assert( ((T & ~TARGET_TYPE_PROC_CHIP) != 0 ) != true,
                "Only TARGET_TYPE_CHIP and TARGET_TYPE_EQ child types supported");
                 
                if(K & TARGET_TYPE_PROC_CHIP)
                {
                    std::vector<Target<T,V> chip_children(CHIPLET_COUNT);
                    for (int i = 0; i < CHIPLET_COUNT; i++)
                    {
                        if (this->iv_handle.fields.type == TARGET_TYPE_PROC_CHIP  & 
                            this->iv_handle.fields.present = 1    &
                            this->iv_handle.fields.functional = 1  )
                        {                        
                            chip_children.push_back(this->iv_handle)
                        }
                    }
                    return chip_children;
                }

                if(K & TARGET_TYPE_PERV)
                {
                    this->iv_handle.fields.chiplet_num = Value;
                    // this->iv_handle.fields.type_perv = 1;
                    this->iv_handle.fields.type_target_num = 0;
                }    

           }

        ///
        /// @brief Get the target at the other end of a bus - dimm included
        /// @tparam T The type of the parent
        /// @return Target<T> a target representing the thing on the other end
        /// @note Can be easily changed to a vector if needed
        ///
        template<TargetType T>
        Target<T,K> getOtherEnd(void) const
            {
               static_assert( false, "getOtherEnd() is not supported on PPE platforms");
            }
#endif

        ///
        /// @brief Copy from a Target<O> to a Target<K>
        /// @tparam O the target type of the other
        ///
        template<TargetType O>
        Target<K,V>( const Target<O>& Other ):
            Target<K,V>(Other.get())
        {
            static_assert( (K & O) != 0,
                           "unable to cast Target, no shared types");

            static_assert( bitCount<K>::count >= bitCount<O>::count,
                           "unable to cast to specialized Target");
        }

    private:

//         union {
//             V value;
//             struct {
// #ifdef _BIG_ENDIAN  
//                 V chiplet_num       : 8;  
//                 V _reserved_b24     : 24;        
// #else       
//                 V _reserved_b24     : 24;        
//                 V chiplet_num       : 8;
// #endif                     
//             } fields;
//             
//         } iv_address;
//         
        union {
            V value;
            struct {
#ifdef _BIG_ENDIAN  
                V chiplet_num       : 8;  
                V type_target_num   : 8;      
                V type              : 8; 
                V _reserved_b6      : 6; 
                V present           : 1;
                V functional        : 1;
                V address_overlay   : 32;
#else
                V address_overlay   : 32;
                V functional        : 1;
                V present           : 1;
                V _reserved_b6      : 6;
                V type              : 8;      
                V type_target_num   : 8;
                V chiplet_num       : 8;
#endif                     
            } fields;
        } iv_handle ;
        
//      uint64_t iv_present_children;
//      uint64_t iv_functional_children;       

    };
    

#if 0
    ///
    /// @brief Return the string interpretation of this target
    /// @tparam T The type of the target
    /// @tparam B The type of the buffer
    /// @param[in] The Target<T>
    /// @param[in] The buffer
    /// @return void
    /// @post The contents of the buffer is replaced with the string
    /// representation of the target
    ///
    template<TargetType T, typename B>
    void toString(const Target<T>& i_target, B& i_buffer)
    {
        static_assert( false, "toString is not supported on PPE platforms");
    }

    ///
    /// @brief Get an enumerated target of a specific type
    /// @tparam T The type of the target
    /// @param[in] uint64_t representing the ordinal number of
    /// the desired target
    /// @return Target<T> the target requested
    ///
    template<TargetType T>
    Target<T> getTarget(uint64_t Ordinal)
    {
        // For testing
        return Target<T>(Ordinal);
    }
#endif
    // Why has the been removed? For starters, the API name
    // is probably wrong as it's already been confused with
    // Target::getChildren(). And if I'm going to change it
    // I really want to see if we need it. I'm still not
    // clear on whether we're alloing this traversal or not.
#if 0
    ///
    /// @brief Get the base target's children
    /// @tparam T The type of the target
    /// @return std::vector<Target<T> > a vector of present/functional
    /// children
    ///
    template<TargetType T>
    std::vector<Target<T> > getChildren()
    {
        // For testing
        return {Target<T>(), Target<T>()};
    }
#endif

    ///
    /// @brief Check if the target is of a type, or in a type subset.
    /// @tparam K the TargetType to check
    /// @tparam T TargetType or TargetType composite to check against
    /// @return True, iff K is a proper T
    ///
    template< TargetType K, TargetType T >
    constexpr bool is_same(void)
    { return (K & T) != 0; }
        
    


};
//#include "plat_ppe_targets_hold.H"
#endif
