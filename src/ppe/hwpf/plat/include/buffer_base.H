/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/hwpf/plat/include/buffer_base.H $                     */
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
 * @file buffer_base.H
 * @brief definitions for fapi2 buffer base class
 */

#ifndef __FAPI2_BUFFER_BASE__
#define __FAPI2_BUFFER_BASE__

#include <stdint.h>
#include <initializer_list>
#include <error_scope.H>
#include <buffer_parameters.H>
#include <buffer_traits.H>
#include <return_code.H>

namespace fapi2
{
    /// @brief Base class for buffers and variable buffers
    /// @tparam T is the type of iv_data (std::vector, etc)
    /// @tparam TT is the template trait, defaults to the trait for T
    ///
    /// Buffers can be of two styles; buffers made from an integral type and
    /// buffers made from a container. Integral type buffers, while limited
    /// in size, can be tightly controlled via the compiler by using c++
    /// templates.
    ///
    /// C++ templates allow for very explicit control, but yield a
    /// syntax different than the FAPI 1.x functional interface. For example,
    /// a fapi2::buffer is defined as having a type:
    /// @code
    /// fapi2::buffer<uint64_t> new_buffer;
    /// @endcode
    /// defines a buffer with exactly 64 bits, and can be manipulated by the
    /// compiler as a single intrgral value. These implementations result
    /// in concise instruction streams, and a platform may choose to implement
    /// all or some or none of the integral buffer types.
    ///
    /// Buffers which have containers as their underlying implementation
    /// are found in the class fapi2::variable_buffer. variable_buffer is little
    /// more than
    /// @code
    /// fapi2::buffer<fapi2::bits_container>
    /// @endcode
    /// where bits_container is the typedef of the underlying container (a
    /// vector of uint32_t, for example)
    ///
    /// Examples:<br>
    ///
    /// * Simple uint64_t buffer
    /// @code
    /// const uint32_t x = 2;
    ///
    /// // this data buffer will contain data in a uint64_t type
    /// fapi2::buffer<uint64_t> data;
    ///
    /// // Set using the template and a constant
    /// data.setBit<x>();
    ///
    /// // Set using the template and a value
    /// data.setBit<3>();
    ///
    /// // Set using the function interface, and a value
    /// data.setBit(1);
    ///
    /// // compiler gets smart.
    /// // movabs $0x7000000000000000,%rsi
    /// @endcode
    ///
    /// * variable_buffer, same thing
    /// @code
    ///
    /// const uint32_t x = 2;
    ///
    /// // Note: only 15 bits long
    /// fapi2::variable_buffer data(15);
    ///
    ///
    /// data.setBit(x);
    /// data.setBit(3);
    /// data.setBit(1);
    /// @endcode
    ///
    /// * method chaining
    /// Buffers support method chaining. So, rather than
    /// this
    /// @code
    /// buffer<T> mask;
    /// mask.setBit<B>();
    /// mask.invert();
    /// my_buffer &= mask;
    /// @endcode
    /// You can do
    /// @code
    /// my_buffer &= buffer<T>().setBit<B>.invert();
    /// @endcode
    ///
    /// * buffer operations
    /// @code
    ///
    /// // An 8 bit buffer, initialized with a value
    /// fapi2::buffer<uint8_t> eight_bits = 0xAA;
    /// fapi2::buffer<uint8_t> another_eight;
    /// fapi2::buffer<uint16_t> sixteen_bits;
    ///
    /// // You can't assign an 8 bit buffer to a 16 bit buffer.
    /// sixteen_bits = eight_bits; ERROR
    ///
    /// // But you can assign buffers of the same type
    /// another_eight = eight_bits;
    ///
    /// // You can assign constants (or other known values) directly:
    /// sixteen_bits = 0xAABB;
    /// @endcode
    ///
    /// * Variable buffer operations
    ///
    /// @code
    /// fapi2::variable_buffer data(16);
    ///
    /// // Very large buffers can be initialized rather than set bit by bit.
    /// const fapi2::variable_buffer bit_settings_known(
    ///     {0xFFFF0000, 0xAABBF0F0,
    ///      0xFFFF0000, 0xAABBF0F0,
    ///      0xFFFF0000, 0xAABBF0F0,});
    ///
    /// // Assignment will expand or shrink the size automatically.
    /// data = bit_settings_known;
    ///
    /// // You can assign directly to the buffer:
    /// fapi2::variable_buffer other_bits;
    /// const fapi2::container_unit x = 0xFF00AA55;
    /// other_bits = {x, 0xDEADBEEF};
    /// @endcode
    ///
    template <typename T, typename TT = bufferTraits<T> >
    class buffer_base
    {

    public:

        /// Shortcut typedef to get to our traits class
        typedef typename TT::bits_type bits_type;
        /// Shortcut typedef to get to our traits class
        typedef typename TT::unit_type unit_type;

        ///
        /// @brief Default constructor
        /// @note iv_data will get the "default" construction, which is
        /// correct - 0 for integral types, an empty container for the others.
        ///
        buffer_base(void):
            iv_data()
            {}

        virtual ~buffer_base(void)
            {}

#ifndef DOXYGEN
        /// @brief Print the contents of the buffer to stdout
        inline void print(void) const
        { TT::print(iv_data); }
#endif
        ///
        /// @brief Get the contents of the buffer
        /// @return The contents of the buffer
        ///
        inline operator T() const { return iv_data; }

        ///
        /// @brief Get the contents of the buffer
        /// @return The contents of the buffer
        ///
        inline operator T&() { return iv_data; }

        ///
        /// @brief Get the contents of the buffer
        /// @return The contents of the buffer
        ///
        inline T& operator()(void) { return iv_data; }

        ///
        /// @brief Get the contents of the buffer
        /// @return Reference to the contents of the buffer
        ///
        inline const T& operator()(void) const { return iv_data; }

        ///
        /// @brief Get a pointer to the buffer bits
        /// @return Pointer to the buffer itself
        ///
        inline T* pointer(void) { return &iv_data; }

        /// @name Buffer Manipulation Functions
        ///@{

        ///
        /// @brief Set an OT of data in buffer
        /// @param[in] i_value sizeof(OT) bits of data
        /// @param[in] i_offset Start OT (start word, for example) in buffer
        ///            - defaults to 0 (will by default write the left most element)
        /// @return FAPI2_RC_SUCCESS on success, FAPI2_RC_OVERFLOW otherwise
        /// @note This is is only available for integral types. To set a
        ///    variable_buffer into a variable_buffer, use insert()
        ///
        template< typename OT>
        inline fapi2::ReturnCode set(OT i_value, const bits_type i_offset = 0)
        {
            // Compile time check to make sure OT isn't a variable buffer
            static_assert( !std::is_same<bits_container, OT>::value,
                           "Can't use a variable_buffer as input to set()" );

            //
            // There's a gotcha in here. size<OT>() returns the size in the buffer
            // in OT units *rounded up*. This is the actual size of the buffer, not
            // the perceived size of a variable_buffer. This should be OK however,
            // as what we're trying to prevent is overflow, which this should do.
            //
            const uint32_t length = TT:: template size<OT>(iv_data);
            static const bits_type bits_in_value = parameterTraits<OT>::bit_length;
            const bits_type bit_length = TT::bit_length(iv_data);

            if (i_offset >= length)
            {
                return FAPI2_RC_OVERFLOW;
            }

            // Create mask if part of this byte is not in the valid part of the buffer,
            // Shift it left by the amount of unused bits,
            // Clear the unused bits
            if (((i_offset + 1) == length) && (bit_length % bits_in_value)) {
                i_value &= parameterTraits<OT>::mask << ((bits_in_value * length) - bit_length);
            }

            parameterTraits<OT>::template write_element<unit_type>(TT::get_address(iv_data), i_value, i_offset);
            return FAPI2_RC_SUCCESS;
        }

        ///
        /// @brief Set and entire buffer to X's
        /// @tparam X {0,1} depending if you want to clear (0)
        /// or fill (1) a buffer
        ///
        template< uint8_t X >
        inline void flush(void)
            {
                static_assert( (X == 1) || (X == 0), "bad argument to flush" );
                (0 == X) ? TT::clear(iv_data) : TT::set(iv_data);
            }

        ///
        /// @brief Invert entire buffer
        /// @return buffer_base&, Useful for method chaining
        ///
        inline buffer_base& invert(void)
            { TT::invert(iv_data); return *this; }

        ///
        /// @brief Bit reverse entire buffer
        /// @return buffer_base&, Useful for method chaining
        ///
        inline buffer_base& reverse(void)
            { TT::reverse(iv_data); return *this; }

        //@}
    protected:

        ///
        /// @brief Variable buffer constructor
        /// @param[in] i_value number of *bits* (sizeof(container_units) * 8)
        /// needed. Meaningless for integral types and thus protected.
        ///
        buffer_base(bits_type i_value);

        ///
        /// @brief Variable buffer construct from a list
        /// @param[in] i_value an initializer list to initialize the container.
        /// Meaningless for integral types and thus protected
        ///
        buffer_base(std::initializer_list<unit_type> i_value);

        ///
        /// @brief Clear the buffer
        ///
        inline void clear(void)
            { TT::clear(iv_data); }

        /// The contents of the buffer
        T iv_data;
    };

    template <typename T, typename TT>
    inline buffer_base<T, TT>::buffer_base(bits_type i_value):
        iv_data( std::max(bits_type(1),
                          bits_type(i_value / 8 / sizeof(bits_type))))
    {
    }

    template <typename T, typename TT>
    inline buffer_base<T, TT>::buffer_base(std::initializer_list<unit_type> i_value):
        iv_data(i_value)
    {
    }
};



#endif
