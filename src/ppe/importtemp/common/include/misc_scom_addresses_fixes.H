/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/importtemp/common/include/misc_scom_addresses_fixes.H $ */
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
///
/// @file misc_scom_addresses_fixes.H
/// @brief The *scom_addresses.H files are  generated form figtree, but
///        the figree can be wrong.  This file is included at the end
///        of scom_addresses.H and allows incorrect constants to be
///        fixed manually.
///
// *HWP HWP Owner: Ben Gass <bgass@us.ibm.com>
// *HWP FW Owner: Thi Tran <thi@us.ibm.com>
// *HWP Team: Infrastructure
// *HWP Level: 1
// *HWP Consumed by: FSP:HB:HS:OCC:SBE:CME:SGPE:PGPE:FPPE:IPPE

#ifndef __MISC_SCOM_ADDRESSES_FIXES_H
#define __MISC_SCOM_ADDRESSES_FIXES_H


//Example,
//Copy the whole line from the *scom_addresses.H file.  Then add
//FIX in front of REG, and add another paramter that is the new
//corrected value.
//FIXREG64( PU_ALTD_ADDR_REG,
//          RULL(0x05022800), SH_UNT, SH_ACS_SCOM,
//          RULL(0x00090000)
//        );

// ADU registers
FIXREG64( PU_ALTD_ADDR_REG,
          RULL(0x05022800), SH_UNT, SH_ACS_SCOM,
          RULL(0x00090000)
        );
FIXREG64( PU_ALTD_CMD_REG,
          RULL(0x05022801), SH_UNT, SH_ACS_SCOM,
          RULL(0x00090001)
        );
FIXREG64( PU_ALTD_OPTION_REG,
          RULL(0x05022802), SH_UNT, SH_ACS_SCOM,
          RULL(0x00090002)
        );
FIXREG64( PU_ALTD_STATUS_REG,
          RULL(0x05022803), SH_UNT, SH_ACS_SCOM,
          RULL(0x00090003)
        );
FIXREG64( PU_ALTD_DATA_REG,
          RULL(0x05022804), SH_UNT, SH_ACS_SCOM_RW,
          RULL(0x00090004)
        );
FIXREG64( PU_FORCE_ECC_REG,
          RULL(0x0502280D), SH_UNT, SH_ACS_SCOM_RW,
          RULL(0x0009000D)
        );
FIXREG64( PU_XSCOM_BASE_REG,
          RULL(0x05022810), SH_UNT, SH_ACS_SCOM,
          RULL(0x00090010)
        );
FIXREG64( PU_XSCOM_MODE_REG,
          RULL(0x05022811), SH_UNT, SH_ACS_SCOM,
          RULL(0x00090011)
        );
FIXREG64( PU_XSCOM_LOG_REG,
          RULL(0x05022812), SH_UNT, SH_ACS_SCOM,
          RULL(0x00090012)
        );
FIXREG64( PU_XSCOM_ERR_REG,
          RULL(0x05022813), SH_UNT, SH_ACS_SCOM_WCLRREG,
          RULL(0x00090013)
        );
FIXREG64( PU_XSCOM_RCVED_STAT_REG,
          RULL(0x05022818), SH_UNT, SH_ACS_SCOM_WCLRREG,
          RULL(0x00090018)
        );
FIXREG64( PU_ADS_XSCOM_CMD_REG,
          RULL(0x0502281C), SH_UNT, SH_ACS_SCOM,
          RULL(0x0009001C)
        );
FIXREG64( PU_XSCOM_DAT0_REG,
          RULL(0x0502281E), SH_UNT, SH_ACS_SCOM_RW,
          RULL(0x0009001E)
        );
FIXREG64( PU_XSCOM_DAT1_REG,
          RULL(0x0502281F), SH_UNT, SH_ACS_SCOM_RW,
          RULL(0x0009001F)
        );
FIXREG64( PU_SND_STAT_REG,
          RULL(0x05022820), SH_UNT, SH_ACS_SCOM,
          RULL(0x00090020)
        );
FIXREG64( PU_SND_MODE_REG,
          RULL(0x05022821), SH_UNT, SH_ACS_SCOM,
          RULL(0x00090021)
        );
FIXREG64( PU_RCV_ERRLOG0_REG,
          RULL(0x05022822), SH_UNT, SH_ACS_SCOM_WAND,
          RULL(0x00090022)
        );
FIXREG64( PU_RCV_ERRLOG1_REG,
          RULL(0x05022823), SH_UNT, SH_ACS_SCOM_WAND,
          RULL(0x00090023)
        );
FIXREG64( PU_TOD_DATA_SND_REG,
          RULL(0x05022828), SH_UNT, SH_ACS_SCOM_RW,
          RULL(0x00090028)
        );
FIXREG64( PU_TOD_DATA_RCV_REG,
          RULL(0x05022829), SH_UNT, SH_ACS_SCOM_RO,
          RULL(0x00090029)
        );
FIXREG64( PU_TOD_CMD_REG,
          RULL(0x0502282A), SH_UNT, SH_ACS_SCOM,
          RULL(0x0009002A)
        );
FIXREG64( PU_IO_DATA_REG,
          RULL(0x05022830), SH_UNT, SH_ACS_SCOM_RO,
          RULL(0x00090030)
        );
FIXREG64( PU_PIB_CMD_REG,
          RULL(0x05022831), SH_UNT, SH_ACS_SCOM,
          RULL(0x00090031)
        );
FIXREG64( PU_PIB_DATA_REG,
          RULL(0x05022832), SH_UNT, SH_ACS_SCOM_RO,
          RULL(0x00090032)
        );
FIXREG64( PU_PIB_RESET_REG,
          RULL(0x05022833), SH_UNT, SH_ACS_SCOM,
          RULL(0x00090033)
        );
FIXREG64( PU_LPC_BASE_REG,
          RULL(0x05022840), SH_UNT, SH_ACS_SCOM,
          RULL(0x00090040)
        );
FIXREG64( PU_LPC_CMD_REG,
          RULL(0x05022841), SH_UNT, SH_ACS_SCOM,
          RULL(0x00090041)
        );
FIXREG64( PU_LPC_DATA_REG,
          RULL(0x05022842), SH_UNT, SH_ACS_SCOM_RO,
          RULL(0x00090042)
        );
FIXREG64( PU_ADU_HANG_DIV_REG,
          RULL(0x05022850), SH_UNT, SH_ACS_SCOM_RW,
          RULL(0x00090050)
        );

// PBA registers
FIXREG64( PU_PBAMODE_OCI,
          RULL(0x40020000), SH_UNT, SH_ACS_OCI,
          RULL(0xC0020000)
        );
FIXREG64( PU_PBAMODE_SCOM,
          RULL(0x05016840), SH_UNT, SH_ACS_SCOM,
          RULL(0x00068000)
        );
FIXREG64( PU_PBASLVRST_OCI,
          RULL(0x40020008), SH_UNT, SH_ACS_OCI,
          RULL(0xC0020008)
        );
FIXREG64( PU_PBASLVRST_SCOM,
          RULL(0x05016841), SH_UNT, SH_ACS_SCOM,
          RULL(0x00068001)
        );
FIXREG64( PU_PBASLVCTL0_OCI,
          RULL(0x40020020), SH_UNT, SH_ACS_OCI,
          RULL(0xC0020020)
        );
FIXREG64( PU_PBASLVCTL0_SCOM,
          RULL(0x05016844), SH_UNT, SH_ACS_SCOM_RW,
          RULL(0x00068004)
        );
FIXREG64( PU_PBASLVCTL1_OCI,
          RULL(0x40020028), SH_UNT, SH_ACS_OCI,
          RULL(0xC0020028)
        );
FIXREG64( PU_PBASLVCTL1_SCOM,
          RULL(0x05016845), SH_UNT, SH_ACS_SCOM_RW,
          RULL(0x00068005)
        );
FIXREG64( PU_PBASLVCTL2_OCI,
          RULL(0x40020030), SH_UNT, SH_ACS_OCI,
          RULL(0xC0020030)
        );
FIXREG64( PU_PBASLVCTL2_SCOM,
          RULL(0x05016846), SH_UNT, SH_ACS_SCOM_RW,
          RULL(0x00068006)
        );
FIXREG64( PU_PBASLVCTL3_OCI,
          RULL(0x40020038), SH_UNT, SH_ACS_OCI,
          RULL(0xC0020038)
        );
FIXREG64( PU_PBASLVCTL3_SCOM,
          RULL(0x05016847), SH_UNT, SH_ACS_SCOM_RW,
          RULL(0x00068007)
        );
FIXREG64( PU_BCDE_CTL_OCI,
          RULL(0x40020080), SH_UNT, SH_ACS_OCI,
          RULL(0xC0020080)
        );
FIXREG64( PU_BCDE_CTL_SCOM,
          RULL(0x05016850), SH_UNT, SH_ACS_SCOM,
          RULL(0x00068010)
        );
FIXREG64( PU_BCDE_SET_OCI,
          RULL(0x40020088), SH_UNT, SH_ACS_OCI,
          RULL(0xC0020088)
        );
FIXREG64( PU_BCDE_SET_SCOM,
          RULL(0x05016851), SH_UNT, SH_ACS_SCOM_RW,
          RULL(0x00068011)
        );
FIXREG64( PU_BCDE_STAT_OCI,
          RULL(0x40020090), SH_UNT, SH_ACS_OCI,
          RULL(0xC0020090)
        );
FIXREG64( PU_BCDE_STAT_SCOM,
          RULL(0x05016852), SH_UNT, SH_ACS_SCOM_RO,
          RULL(0x00068012)
        );
FIXREG64( PU_BCDE_PBADR_OCI,
          RULL(0x40020098), SH_UNT, SH_ACS_OCI,
          RULL(0xC0020098)
        );
FIXREG64( PU_BCDE_PBADR_SCOM,
          RULL(0x05016853), SH_UNT, SH_ACS_SCOM_RW,
          RULL(0x00068013)
        );
FIXREG64( PU_BCDE_OCIBAR_OCI,
          RULL(0x400200A0), SH_UNT, SH_ACS_OCI,
          RULL(0xC00200A0)
        );
FIXREG64( PU_BCDE_OCIBAR_SCOM,
          RULL(0x05016854), SH_UNT, SH_ACS_SCOM_RW,
          RULL(0x00068014)
        );
FIXREG64( PU_BCUE_CTL_OCI,
          RULL(0x400200A8), SH_UNT, SH_ACS_OCI,
          RULL(0xC00200A8)
        );
FIXREG64( PU_BCUE_CTL_SCOM,
          RULL(0x05016855), SH_UNT, SH_ACS_SCOM,
          RULL(0x00068015)
        );
FIXREG64( PU_BCUE_SET_OCI,
          RULL(0x400200B0), SH_UNT, SH_ACS_OCI,
          RULL(0xC00200B0)
        );
FIXREG64( PU_BCUE_SET_SCOM,
          RULL(0x05016856), SH_UNT, SH_ACS_SCOM_RW,
          RULL(0x00068016)
        );
FIXREG64( PU_BCUE_STAT_OCI,
          RULL(0x400200B8), SH_UNT, SH_ACS_OCI,
          RULL(0xC00200B8)
        );
FIXREG64( PU_BCUE_STAT_SCOM,
          RULL(0x05016857), SH_UNT, SH_ACS_SCOM_RO,
          RULL(0x00068017)
        );
FIXREG64( PU_BCUE_PBADR_OCI,
          RULL(0x400200C0), SH_UNT, SH_ACS_OCI,
          RULL(0xC00200C0)
        );
FIXREG64( PU_BCUE_PBADR_SCOM,
          RULL(0x05016858), SH_UNT, SH_ACS_SCOM_RW,
          RULL(0x00068018)
        );
FIXREG64( PU_BCUE_OCIBAR_OCI,
          RULL(0x400200C8), SH_UNT, SH_ACS_OCI,
          RULL(0xC00200C8)
        );
FIXREG64( PU_BCUE_OCIBAR_SCOM,
          RULL(0x05016859), SH_UNT, SH_ACS_SCOM_RW,
          RULL(0x00068019)
        );
FIXREG64( PU_PBAPBOCR0_OCI,
          RULL(0x400200D0), SH_UNT, SH_ACS_OCI,
          RULL(0xC00200D0)
        );
FIXREG64( PU_PBAPBOCR0_SCOM,
          RULL(0x0501685A), SH_UNT, SH_ACS_SCOM_RO,
          RULL(0x0006801A)
        );
FIXREG64( PU_PBAPBOCR1_OCI,
          RULL(0x400200D8), SH_UNT, SH_ACS_OCI,
          RULL(0xC00200D8)
        );
FIXREG64( PU_PBAPBOCR1_SCOM,
          RULL(0x0501685B), SH_UNT, SH_ACS_SCOM_RO,
          RULL(0x0006801B)
        );
FIXREG64( PU_PBAPBOCR2_OCI,
          RULL(0x400200E0), SH_UNT, SH_ACS_OCI,
          RULL(0xC00200E0)
        );
FIXREG64( PU_PBAPBOCR2_SCOM,
          RULL(0x0501685C), SH_UNT, SH_ACS_SCOM_RO,
          RULL(0x0006801C)
        );
FIXREG64( PU_PBAPBOCR3_OCI,
          RULL(0x400200E8), SH_UNT, SH_ACS_OCI,
          RULL(0xC00200E8)
        );
FIXREG64( PU_PBAPBOCR3_SCOM,
          RULL(0x0501685D), SH_UNT, SH_ACS_SCOM_RO,
          RULL(0x0006801D)
        );
FIXREG64( PU_PBAPBOCR4_OCI,
          RULL(0x400200F0), SH_UNT, SH_ACS_OCI,
          RULL(0xC00200F0)
        );
FIXREG64( PU_PBAPBOCR4_SCOM,
          RULL(0x0501685E), SH_UNT, SH_ACS_SCOM_RO,
          RULL(0x0006801E)
        );
FIXREG64( PU_PBAPBOCR5_OCI,
          RULL(0x400200F8), SH_UNT, SH_ACS_OCI,
          RULL(0xC00200F8)
        );
FIXREG64( PU_PBAPBOCR5_SCOM,
          RULL(0x0501685F), SH_UNT, SH_ACS_SCOM_RO,
          RULL(0x0006801F)
        );
FIXREG64( PU_PBAXSNDTX_OCI,
          RULL(0x40020100), SH_UNT, SH_ACS_OCI,
          RULL(0xC0020100)
        );
FIXREG64( PU_PBAXSNDTX_SCOM,
          RULL(0x05016860), SH_UNT, SH_ACS_SCOM_RW,
          RULL(0x00068020)
        );
FIXREG64( PU_PBAXCFG_OCI,
          RULL(0x40020108), SH_UNT, SH_ACS_OCI,
          RULL(0xC0020108)
        );
FIXREG64( PU_PBAXCFG_SCOM,
          RULL(0x05016861), SH_UNT, SH_ACS_SCOM,
          RULL(0x00068021)
        );
FIXREG64( PU_PBAXSNDSTAT_OCI,
          RULL(0x40020110), SH_UNT, SH_ACS_OCI,
          RULL(0xC0020110)
        );
FIXREG64( PU_PBAXSNDSTAT_SCOM,
          RULL(0x05016862), SH_UNT, SH_ACS_SCOM_RO,
          RULL(0x00068022)
        );
FIXREG64( PU_PBAXRCVSTAT_OCI,
          RULL(0x40020120), SH_UNT, SH_ACS_OCI,
          RULL(0xC0020120)
        );
FIXREG64( PU_PBAXRCVSTAT_SCOM,
          RULL(0x05016864), SH_UNT, SH_ACS_SCOM_RO,
          RULL(0x00068024)
        );
FIXREG64( PU_PBAXSHBR0_OCI,
          RULL(0x40020130), SH_UNT, SH_ACS_OCI,
          RULL(0xC0020130)
        );
FIXREG64( PU_PBAXSHBR0_SCOM,
          RULL(0x05016866), SH_UNT, SH_ACS_SCOM_RW,
          RULL(0x00068026)
        );
FIXREG64( PU_PBAXSHCS0_OCI,
          RULL(0x40020138), SH_UNT, SH_ACS_OCI,
          RULL(0xC0020138)
        );
FIXREG64( PU_PBAXSHCS0_SCOM,
          RULL(0x05016867), SH_UNT, SH_ACS_SCOM,
          RULL(0x00068027)
        );
FIXREG64( PU_PBAXSHBR1_OCI,
          RULL(0x40020150), SH_UNT, SH_ACS_OCI,
          RULL(0xC0020150)
        );
FIXREG64( PU_PBAXSHBR1_SCOM,
          RULL(0x0501686A), SH_UNT, SH_ACS_SCOM_RW,
          RULL(0x0006802A)
        );
FIXREG64( PU_PBAXSHCS1_OCI,
          RULL(0x40020158), SH_UNT, SH_ACS_OCI,
          RULL(0xC0020158)
        );
FIXREG64( PU_PBAXSHCS1_SCOM,
          RULL(0x0501686B), SH_UNT, SH_ACS_SCOM,
          RULL(0x0006802B)
        );


#endif
