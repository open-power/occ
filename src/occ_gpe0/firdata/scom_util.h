/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe0/firdata/scom_util.h $                            */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2018                        */
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

/* Interfaces to read SCOM registers */

/*#include <stdint.h> */
#include <native.h>
#include <scom_trgt.h>

#define SCOMFAIL 0xDEADBEEF

typedef enum
{
    N0_CHIPLET_ID   = 0x02,     ///< Nest0 (North) chiplet
    N1_CHIPLET_ID   = 0x03,     ///< Nest1 (East) chiplet
    N2_CHIPLET_ID   = 0x04,     ///< Nest2 (South) chiplet
    N3_CHIPLET_ID   = 0x05,     ///< Nest3 (West) chiplet
    MC01_CHIPLET_ID = 0x07,     ///< MC01 (West) chiplet
    MC23_CHIPLET_ID = 0x08,     ///< MC23 (East) chiplet
    OB0_CHIPLET_ID  = 0x09,     ///< OBus0 chiplet
    PCI0_CHIPLET_ID = 0x0D,     ///< PCIe0 chiplet
    EP00_CHIPLET_ID = 0x10,     ///< Quad0 chiplet (EX0/1)
    EP05_CHIPLET_ID = 0x15,     ///< Quad5 chiplet (EX10/11)
    EC00_CHIPLET_ID = 0x20,     ///< Core0 chiplet (Quad0, EX0, C0)
    EC23_CHIPLET_ID = 0x37      ///< Core23 chiplet (Quad5, EX11, C1)
} p9_chiplet_id_t;

typedef enum
{
    MC_MC01_0_RING_ID = 0x2,    ///< MC01_0 / MC23_0
    MC_IOM01_0_RING_ID = 0x4,   ///< IOM01_0 / IOM45_0
} p9_mc_ring_id_t;

typedef enum
{
    XB_IOX_0_RING_ID = 0x3,     ///< IOX_0
    XB_IOX_2_RING_ID = 0x5,     ///< IOX_2
    XB_PBIOX_0_RING_ID = 0x6,   ///< PBIOX_0
    XB_PBIOX_2_RING_ID = 0x8    ///< PBIOX_2
} p9_xb_ring_id_t;

typedef enum
{
    P9C_MC_CHAN_RING_ID = 0x2,
    P9C_MC_IO_RING_ID   = 0x4,
    P9C_MC_BIST_RING_ID = 0x8
} p9c_mc_ring_id_t;

typedef enum
{
    N2_PCIS0_0_RING_ID = 0x3,   ///< PCIS0_0
} p9_n2_ring_id_t;

typedef enum
{
    P9A_MC_OMIC0_RING_ID = 0x4,      ///< OMIC0
    P9A_MC_OMIC2_RING_ID = 0x6,      ///< OMIC2
    P9A_MC_OMIC0_PPE_RING_ID = 0x9,  ///< OMIC0 PPE
    P9A_MC_OMIC2_PPE_RING_ID = 0xB,  ///< OMIC2 PPE
} p9a_mc_ring_id_t;

typedef enum
{
    P9A_MC_DSTL_CHAN0_SAT_ID = 0x4,   ///< DSTL registers channel 0
    P9A_MC_DSTL_CHAN3_SAT_ID = 0x7,   ///< DSTL registers channel 3
    P9A_MC_USTL_CHAN0_SAT_ID = 0x8,   ///< USTL registers channel 0
    P9A_MC_USTL_CHAN3_SAT_ID = 0xB,   ///< USTL registers channel 3
} p9a_mc_sat_id_t;

typedef enum
{
    N3_NPU_0_RING_ID      = 0x5,
    P9A_NPU_2_RING_ID     = 0x7,
    P9A_NPU_2_FIR_RING_ID = 0x8,
    P9A_NPU_0_FIR_RING_ID = 0xF,
} p9a_npu_ring_id_t;

/** @brief  Performs a hardware scom on a regular register.
 *  @param  i_trgt The SCOM target.
 *  @param  i_addr 32-bit SCOM address.
 *  @param  o_val  64-bit returned value.
 *  @return Non-SUCCESS if the SCOM fails. SUCCESS otherwise.
 */
int32_t SCOM_getScom( SCOM_Trgt_t i_trgt, uint32_t i_addr, uint64_t * o_val );

/** @brief  Performs a hardware put scom on a regular register.
 *  @param  i_trgt The SCOM target.
 *  @param  i_addr 32-bit SCOM address.
 *  @param  i_val  64-bit value to write to the address.
 *  @return Non-SUCCESS if the SCOM fails. SUCCESS otherwise.
*/
int32_t SCOM_putScom( SCOM_Trgt_t i_trgt, uint32_t i_addr, uint64_t i_val );

/** @brief  Performs a hardware scom on an indirect-SCOM register.
 *  @param  i_trgt The SCOM target.
 *  @param  i_addr 64-bit SCOM address.
 *  @param  o_val  32-bit returned value.
 *  @return Non-SUCCESS if the SCOM fails. SUCCESS otherwise.
 */
int32_t SCOM_getIdScom( SCOM_Trgt_t i_trgt, uint64_t i_addr, uint32_t * o_val );
