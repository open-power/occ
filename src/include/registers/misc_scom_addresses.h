/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/include/registers/misc_scom_addresses.h $                 */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2019                             */
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
#if !defined(_MISC_SCOM_ADDRESSES_H_)
#define _MISC_SCOM_ADDRESSES_H_

// GPE Timer Select Register
#define GPE_GPENTSEL(n) (TP_TPCHIP_OCC_OCI_GPE0_OCB_GPETSEL + ((TP_TPCHIP_OCC_OCI_GPE1_OCB_GPETSEL - TP_TPCHIP_OCC_OCI_GPE0_OCB_GPETSEL) * (n)))

// GPE Interrupt Vector Prefix Register (regs from chips/p10/common/include/p10_oci_proc_?.H)
static const uint32_t TP_TPCHIP_OCC_OCI_GPE0_OCB_GPEIVPR = 0xc0000008ull;
static const uint32_t TP_TPCHIP_OCC_OCI_GPE1_OCB_GPEIVPR = 0xc0010008ull;
#define GPE_GPENIVPR(n) (TP_TPCHIP_OCC_OCI_GPE0_OCB_GPEIVPR + ((TP_TPCHIP_OCC_OCI_GPE1_OCB_GPEIVPR - TP_TPCHIP_OCC_OCI_GPE0_OCB_GPEIVPR) * (n)))

// GPE External Interface XCR (regs from  chips/p10/common/include/p10_oci_proc_?.H)
static const uint32_t TP_TPCHIP_OCC_OCI_GPE0_OCB_GPEXIXCR = 0xc0000080ull;
static const uint32_t TP_TPCHIP_OCC_OCI_GPE1_OCB_GPEXIXCR = 0xc0010080ull;
#define GPE_GPENXIXCR(n) (TP_TPCHIP_OCC_OCI_GPE0_OCB_GPEXIXCR + ((TP_TPCHIP_OCC_OCI_GPE1_OCB_GPEXIXCR - TP_TPCHIP_OCC_OCI_GPE0_OCB_GPEXIXCR) * (n)))

// from: ./include/registers/gpe_register_addresses.h (P9A_*)
#define PU_GPIO_INPUT       0x000B0050
#define PU_GPIO_OUTPUT      0x000B0051
#define PU_GPIO_OUTPUT_OR   0x000B0052
#define PU_GPIO_OUTPUT_CLR  0x000B0053
#define PU_GPIO_OUTPUT_EN   0x000B0054


#if 0
// Found in Cumulus nest "MC Fault Isolation Register"
#define MCS_0_MCFIR         0x05010800
#define MCS_1_MCFIR         0x05010880
#define MCS_2_MCFIR         0x03010800
#define MCS_3_MCFIR         0x03010880

// found in Cumulus nest "MC Mode0 Register"
#define MCS_0_MCMODE0       0x05010811
#define MCS_1_MCMODE0       0x05010891
#define MCS_2_MCMODE0       0x03010811
#define MCS_3_MCMODE0       0x03010891

// found in Cumulus nest "MC Primary Memory Configuration Register"
#define    MCS_0_MCRSVDE    0x0501080E
#define    MCS_0_MCRSVDF    0x0501080F
#define    MCS_1_MCRSVDE    0x0501088E
#define    MCS_1_MCRSVDF    0x0501088F
#define    MCS_2_MCRSVDE    0x0301080E
#define    MCS_2_MCRSVDF    0x0301080F
#define    MCS_3_MCRSVDE    0x0301088E
#define    MCS_3_MCRSVDF    0x0301088F

#define    MCS_0_MCSYNC     0x05010815
#define    MCS_1_MCSYNC     0x05010895
#define    MCS_2_MCSYNC     0x03010815
#define    MCS_3_MCSYNC     0x03010895

// MC Memory Configuration Register FIR/CFG
#define    MCP_CHAN0_CHI_FIR        0x07010900
#define    MCP_CHAN1_CHI_FIR        0x07010940
#define    MCP_CHAN2_CHI_FIR        0x07010980
#define    MCP_CHAN3_CHI_FIR        0x070109C0
#define    MCP_CHAN4_CHI_FIR        0x08010900
#define    MCP_CHAN5_CHI_FIR        0x08010940
#define    MCP_CHAN6_CHI_FIR        0x08010980
#define    MCP_CHAN7_CHI_FIR        0x080109C0

#define    MCP_CHAN0_CHI_MCICFG1Q   0x0701090E
#define    MCP_CHAN1_CHI_MCICFG1Q   0x0701094E
#define    MCP_CHAN2_CHI_MCICFG1Q   0x0701098E
#define    MCP_CHAN3_CHI_MCICFG1Q   0x070109CE
#define    MCP_CHAN4_CHI_MCICFG1Q   0x0801090E
#define    MCP_CHAN5_CHI_MCICFG1Q   0x0801094E
#define    MCP_CHAN6_CHI_MCICFG1Q   0x0801098E
#define    MCP_CHAN7_CHI_MCICFG1Q   0x080109CE
#endif

#endif
