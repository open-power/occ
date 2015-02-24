/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/firdata/ecc.H $                                       */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015                             */
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
#ifndef __PNOR_ECC_H
#define __PNOR_ECC_H

#include <native.h>

/** @file ecc.H
 *  @brief Interfaces for the P8 8-byte ECC algorithm.
 */

/** Status for the ECC removal function. */
typedef uint8_t eccStatus;
#define ECC_CLEAN         0x00  /*< No ECC Error was detected. */
#define ECC_CORRECTED     0x01  /*< ECC error detected and corrected. */
#define ECC_UNCORRECTABLE 0x02  /*< ECC error detected and uncorrectable. */

/** Bit field identifiers for syndrome calculations. */
typedef uint8_t eccBitfields;
#define ECC_GD  0xff      /*< Good ECC matches. */
#define ECC_UE  0xfe      /*< Uncorrectable. */
#define ECC_E0  71        /*< Error in ECC bit 0 */
#define ECC_E1  70        /*< Error in ECC bit 1 */
#define ECC_E2  69        /*< Error in ECC bit 2 */
#define ECC_E3  68        /*< Error in ECC bit 3 */
#define ECC_E4  67        /*< Error in ECC bit 4 */
#define ECC_E5  66        /*< Error in ECC bit 5 */
#define ECC_E6  65        /*< Error in ECC bit 6 */
#define ECC_E7  64        /*< Error in ECC bit 7 */

/** Inject ECC into a data stream.
 *
 *  @param[in] i_src - Source data to create ECC on.
 *  @param[in] i_srcSz - Size in bytes of source data.
 *  @param[out] o_dst - Destination buffer of data+ECC.
 *
 *  @note i_srcSz must be a multiple of 8 bytes.
 */
void injectECC(const uint8_t* i_src, size_t i_srcSz,
               uint8_t* o_dst);

/** Remove ECC from a data stream.
 *
 *  @param[in,out] io_src - Source data+ECC stream.
 *  @param[out] o_dst - Destination buffer for data only.
 *  @param[in] i_dstSz - Size in bytes of destination ((srcSz / 9) * 8).
 *
 *  @note i_dstSz must be a multiple of 8 bytes.
 */
eccStatus removeECC(uint8_t* io_src,
                    uint8_t* o_dst, size_t i_dstSz);


#endif
