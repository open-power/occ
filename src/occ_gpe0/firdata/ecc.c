/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe0/firdata/ecc.c $                                  */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2017                        */
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

#include <native.h>
#include <ecc.h>

/** Matrix used for ECC calculation.
 *
 *  Each row of this is the set of data word bits that are used for
 *  the calculation of the corresponding ECC bit.  The parity of the
 *  bitset is the value of the ECC bit.
 *
 *  ie. ECC[n] = eccMatrix[n] & data
 *
 *  Note: To make the math easier (and less shifts in resulting code),
 *        row0 = ECC7.  HW numbering is MSB, order here is LSB.
 *
 *  These values come from the HW design of the ECC algorithm.
 */
static uint64_t eccMatrix[] = {
    /*0000000000000000111010000100001000111100000011111001100111111111 */
      0x0000e8423c0f99ff,
    /*0000000011101000010000100011110000001111100110011111111100000000 */
    0x00e8423c0f99ff00,
    /*1110100001000010001111000000111110011001111111110000000000000000 */
    0xe8423c0f99ff0000,
    /*0100001000111100000011111001100111111111000000000000000011101000 */
    0x423c0f99ff0000e8,
    /*0011110000001111100110011111111100000000000000001110100001000010 */
    0x3c0f99ff0000e842,
    /*0000111110011001111111110000000000000000111010000100001000111100 */
    0x0f99ff0000e8423c,
    /*1001100111111111000000000000000011101000010000100011110000001111 */
    0x99ff0000e8423c0f,
    /*1111111100000000000000001110100001000010001111000000111110011001 */
    0xff0000e8423c0f99
};

/** Syndrome calculation matrix.
 *
 *  Maps syndrome to flipped bit.
 *
 *  To perform ECC correction, this matrix is a look-up of the bit
 *  that is bad based on the binary difference of the good and bad
 *  ECC.  This difference is called the "syndrome".
 *
 *  When a particular bit is on in the data, it cause a column from
 *  eccMatrix being XOR'd into the ECC field.  This column is the
 *  "effect" of each bit.  If a bit is flipped in the data then its
 *  "effect" is missing from the ECC.  You can calculate ECC on unknown
 *  quality data and compare the ECC field between the calculated
 *  value and the stored value.  If the difference is zero, then the
 *  data is clean.  If the difference is non-zero, you look up the
 *  difference in the syndrome table to identify the "effect" that
 *  is missing, which is the bit that is flipped.
 *
 *  Notice that ECC bit flips are recorded by a single "effect"
 *  bit (ie. 0x1, 0x2, 0x4, 0x8 ...) and double bit flips are identified
 *  by the UE status in the table.
 *
 *  Bits are in MSB order.
 */
static uint8_t syndromeMatrix[] = {
    ECC_GD, ECC_E7, ECC_E6, ECC_UE, ECC_E5, ECC_UE, ECC_UE,     47,
    ECC_E4, ECC_UE, ECC_UE,     37, ECC_UE,     35,     39, ECC_UE,
    ECC_E3, ECC_UE, ECC_UE,     48, ECC_UE,     30,     29, ECC_UE,
    ECC_UE,     57,     27, ECC_UE,     31, ECC_UE, ECC_UE, ECC_UE,
    ECC_E2, ECC_UE, ECC_UE,     17, ECC_UE,     18,     40, ECC_UE,
    ECC_UE,     58,     22, ECC_UE,     21, ECC_UE, ECC_UE, ECC_UE,
    ECC_UE,     16,     49, ECC_UE,     19, ECC_UE, ECC_UE, ECC_UE,
        23, ECC_UE, ECC_UE, ECC_UE, ECC_UE,     20, ECC_UE, ECC_UE,
    ECC_E1, ECC_UE, ECC_UE,     51, ECC_UE,     46,      9, ECC_UE,
    ECC_UE,     34,     10, ECC_UE,     32, ECC_UE, ECC_UE,     36,
    ECC_UE,     62,     50, ECC_UE,     14, ECC_UE, ECC_UE, ECC_UE,
        13, ECC_UE, ECC_UE, ECC_UE, ECC_UE, ECC_UE, ECC_UE, ECC_UE,
    ECC_UE,     61,      8, ECC_UE,     41, ECC_UE, ECC_UE, ECC_UE,
        11, ECC_UE, ECC_UE, ECC_UE, ECC_UE, ECC_UE, ECC_UE, ECC_UE,
        15, ECC_UE, ECC_UE, ECC_UE, ECC_UE, ECC_UE, ECC_UE, ECC_UE,
    ECC_UE, ECC_UE,     12, ECC_UE, ECC_UE, ECC_UE, ECC_UE, ECC_UE,
    ECC_E0, ECC_UE, ECC_UE,     55, ECC_UE,     45,     43, ECC_UE,
    ECC_UE,     56,     38, ECC_UE,      1, ECC_UE, ECC_UE, ECC_UE,
    ECC_UE,     25,     26, ECC_UE,      2, ECC_UE, ECC_UE, ECC_UE,
       24,  ECC_UE, ECC_UE, ECC_UE, ECC_UE, ECC_UE,     28, ECC_UE,
    ECC_UE,     59,     54, ECC_UE,     42, ECC_UE, ECC_UE,     44,
         6, ECC_UE, ECC_UE, ECC_UE, ECC_UE, ECC_UE, ECC_UE, ECC_UE,
         5, ECC_UE, ECC_UE, ECC_UE, ECC_UE, ECC_UE, ECC_UE, ECC_UE,
    ECC_UE, ECC_UE, ECC_UE, ECC_UE, ECC_UE, ECC_UE, ECC_UE, ECC_UE,
    ECC_UE,     63,     53, ECC_UE,      0, ECC_UE, ECC_UE, ECC_UE,
        33, ECC_UE, ECC_UE, ECC_UE, ECC_UE, ECC_UE, ECC_UE, ECC_UE,
         3, ECC_UE, ECC_UE,     52, ECC_UE, ECC_UE, ECC_UE, ECC_UE,
    ECC_UE, ECC_UE, ECC_UE, ECC_UE, ECC_UE, ECC_UE, ECC_UE, ECC_UE,
         7, ECC_UE, ECC_UE, ECC_UE, ECC_UE, ECC_UE, ECC_UE, ECC_UE,
    ECC_UE,     60, ECC_UE, ECC_UE, ECC_UE, ECC_UE, ECC_UE, ECC_UE,
    ECC_UE, ECC_UE, ECC_UE, ECC_UE,      4, ECC_UE, ECC_UE, ECC_UE,
    ECC_UE, ECC_UE, ECC_UE, ECC_UE, ECC_UE, ECC_UE, ECC_UE, ECC_UE,
};

/** Returns the parity of x, i.e. the number of 1-bits in x modulo 2. 
 * Replacement for __builtin_parityl
 */
uint8_t parity_check( uint64_t i_data )
{
    int ones = 0;
    uint32_t x;
    for( x=0; x<(sizeof(i_data)*8); x++ )
    {
        if( i_data & (0x8000000000000000ull >> x) )
        {
            ones++;
        }
    }
    return ones%2;
}

/** Create the ECC field corresponding to a 8-byte data field
 *
 *  @param[in] i_data - The 8 byte data to generate ECC for.
 *  @return The 1 byte ECC corresponding to the data.
 */
uint8_t generateECC(uint64_t i_data)
{
    uint8_t result = 0;

    int i = 0;
    for (i = 0; i < 8; i++)
    {
        result |= (parity_check(eccMatrix[i] & i_data) << i);
    }

    return result;
}

/** Verify the data and ECC match or indicate how they are wrong.
 *
 * @param[in] i_data - The data to check ECC on.
 * @param[in] i_ecc - The [supposed] ECC for the data.
 *
 * @return eccBitfield or 0-64.
 *
 * @retval GD - Indicates the data is good (matches ECC).
 * @retval UE - Indicates the data is uncorrectable.
 * @retval all others - Indication of which bit is incorrect.
 */
uint8_t verifyECC(uint64_t i_data, uint8_t i_ecc)
{
    return syndromeMatrix[generateECC(i_data) ^ i_ecc];
}

/** Correct the data and/or ECC.
 *
 * @param[in,out] io_data - Data to check / correct.
 * @param[in,out] io_ecc - ECC to check / correct.
 *
 * @return eccBitfield or 0-64.
 *
 * @retval GD - Data is good.
 * @retval UE - Data is uncorrectable.
 * @retval all others - which bit was corrected.
 */
uint8_t correctECC(uint64_t* io_data, uint8_t* io_ecc)
{
    uint8_t badBit = verifyECC(*io_data, *io_ecc);

    if ((badBit != ECC_GD) && (badBit != ECC_UE))  /* Good is done, UE is hopeless. */
    {
        /* Determine if the ECC or data part is bad, do bit flip. */
        if (badBit >= ECC_E7)
        {
            *io_ecc ^= (1 << (badBit - ECC_E7));
        }
        else
        {
            *io_data ^= (1ul << (63 - badBit));
        }
    }
    return badBit;
}

void injectECC(const uint8_t* i_src,
               uint32_t i_srcSz,
               uint8_t* o_dst)
{
    if (0 != (i_srcSz % sizeof(uint64_t)))
    {
        return;
    }

    uint32_t i = 0;
    uint32_t o = 0;
    for(i = 0, o = 0;
        i < i_srcSz;
        i += sizeof(uint64_t), o += sizeof(uint64_t) + sizeof(uint8_t))
    {
        /* Read data word, copy to destination. */
        uint64_t data = *((const uint64_t*)(&i_src[i]));
        *((uint64_t*)(&o_dst[o])) = data;
        data = be64toh(data);

        /* Calculate ECC, copy to destination. */
        uint8_t ecc = generateECC(data);
        o_dst[o + sizeof(uint64_t)] = ecc;
    }
}

eccStatus removeECC(uint8_t* io_src,
                    uint8_t* o_dst, uint32_t i_dstSz)
{
    if (0 != (i_dstSz % sizeof(uint64_t)))
    {
        return -1;
    }

    eccStatus rc = ECC_CLEAN;

    uint32_t i = 0, o = 0;
    for(i = 0, o = 0;
        o < i_dstSz;
        i += sizeof(uint64_t) + sizeof(uint8_t), o += sizeof(uint64_t))
    {
        /* Read data and ECC parts. */
        uint64_t data = *((uint64_t*)(&io_src[i]));
        data = be64toh(data);
        uint8_t ecc = io_src[i + sizeof(uint64_t)];

        /* Calculate failing bit and fix data. */
        uint8_t badBit = correctECC(&data, &ecc);

        /* Return data to big endian. */
        data = htobe64(data);

        /* Perform correction and status update. */
        if (badBit == ECC_UE)
        {
            rc = ECC_UNCORRECTABLE;
        }
        else if (badBit != ECC_GD)
        {
            if (rc != ECC_UNCORRECTABLE)
            {
                rc = ECC_CORRECTED;
            }
            *((uint64_t*)(&io_src[i])) = data;
            io_src[i + sizeof(uint64_t)] = ecc;
        }

        /* Copy fixed data to destination buffer. */
        *((uint64_t*)(&o_dst[o])) = data;
    }

    return rc;
}

