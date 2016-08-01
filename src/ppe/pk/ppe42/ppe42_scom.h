/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/pk/ppe42/ppe42_scom.h $                               */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2016                        */
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

/// \file  ppe42_scom.h
/// \brief Include file for PK SCOMs
///

#ifndef __PK_SCOM_H__
#define __PK_SCOM_H__

/// SCOM operations return non-zero error codes that may or may not indicate
/// an actual error, depending on which SCOM is begin accessed.  This error
/// code will appear in the MSR[SIBRC] field, bits[9:11] right after the 
/// SCOM OP returns.  The error code value increases with the severity of the 
/// error.
#define PCB_ERROR_NONE              0
#define PCB_ERROR_RESOURCE_OCCUPIED 1
#define PCB_ERROR_CHIPLET_OFFLINE   2
#define PCB_ERROR_PARTIAL_GOOD      3
#define PCB_ERROR_ADDRESS_ERROR     4
#define PCB_ERROR_CLOCK_ERROR       5
#define PCB_ERROR_PACKET_ERROR      6
#define PCB_ERROR_TIMEOUT           7

#ifdef __cplusplus 
extern "C" {
#endif


/// PPE Load Virtual Double operation
#define PPE_LVD(_m_address, _m_data) \
asm volatile \
    ( \
    "lvd %[data], 0(%[address]) \n" \
    :  [data]"=r"(_m_data) \
    :  [address]"b"(_m_address) \
    );


// PPE Store Virtual Double operation
#define PPE_STVD(_m_address, _m_data) \
asm volatile \
    ( \
        "stvd %[data], 0(%[address]) \n" \
        : [data]"=&r"(_m_data) \
        : "[data]"(_m_data), \
          [address]"b"(_m_address) \
        : "memory" \
     );

/// PPE Load Virtual Double Indexed operation
#define PPE_LVDX(_m_base, _m_offset, _m_data) \
asm volatile \
    ( \
    "lvdx %[data], %[base], %[offset] \n" \
    :  [data]"=r"(_m_data) \
    :  [base]"b"(_m_base), \
       [offset]"r"(_m_offset) \
    );


// PPE Store Virtual Double Indexed operation
#define PPE_STVDX(_m_base, _m_offset, _m_data) \
asm volatile \
    ( \
        "stvdx %[data], %[base], %[offset] \n" \
        : [data]"=&r"(_m_data) \
        : "[data]"(_m_data), \
          [base]"b"(_m_base), \
          [offset]"r"(_m_offset) \
        : "memory" \
     );

#define PPE_MFMSR(_m_data) \
asm volatile \
    ( \
    "mfmsr %[data] \n" \
    :  [data]"=&r"(*_m_data) \
    :  "[data]"(*_m_data) \
    );

/// @brief putscom with absolute address
/// @param [in] i_address    Fully formed SCOM address
/// @param [in] i_data       Pointer to uint64_t data to be written.  A pointer is used
///                          to optimize the underlying hardware execution
///
/// @retval     On PPE42 platform, unmasked errors will take machine check interrupts
uint32_t putscom_abs(const uint32_t i_address, uint64_t i_data);

/// @brief getscom with absolute address
/// @param [in]  i_address   Fully formed SCOM address
/// @param [in] *o_data      Pointer to uint64_t data read
///
/// @retval     On PPE42 platform, unmasked errors will take machine check interrupts

uint32_t getscom_abs( const uint32_t i_address, uint64_t *o_data);

/// @brief Implementation of PPE putscom functionality
/// @param [in] i_chiplet   Chiplet ID (@todo Should only be right justified)
/// @param [in] i_address   Base SCOM address
/// @param [in] i_data      Pointer to uint64_t data to be written.  A pointer is used
///                         to optimize the underlying hardware execution
///
/// @retval     On PPE42 platform, unmasked errors will take machine check interrupts
uint32_t _putscom( const uint32_t i_chiplet, const uint32_t i_address, uint64_t i_data);


/// @brief Implementation of PPE getscom functionality
/// @param [in] i_chiplet   Chiplet ID (@todo Should only be right justified)
/// @param [in] i_address   Base SCOM address
/// @param [in] i_data      Pointer to uint64_t data read
///
/// @retval     On PPE42 platform, unmasked errors will take machine check interrupts
uint32_t _getscom( uint32_t i_chiplet, uint32_t i_address, uint64_t *o_data);

extern inline uint32_t putscom(const uint32_t i_chiplet, const uint32_t i_address, uint64_t i_data)
{
    return _putscom(i_chiplet, i_address, i_data);
}


extern inline uint32_t getscom(const uint32_t i_chiplet, const uint32_t i_address, uint64_t *o_data)
{  
    return _getscom(i_chiplet, i_address, o_data);
}

#ifdef __cplusplus 
} // extern C
#endif

#endif  // __PK_SCOM_H__
