/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/occhw/occhw_scom.h $                                  */
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
#ifndef __OCCHW_SCOM_H__
#define __OCCHW_SCOM_H__

//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2014
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file occhw_scom.h
/// \brief procedures and support for SCOM operations

#include "ssx.h"

#include "occhw_common.h"
#include "pmc_register_addresses.h"
#include "pmc_firmware_registers.h"

////////////////////////////////////////////////////////////////////////////
// SCOM
////////////////////////////////////////////////////////////////////////////

#ifndef __ASSEMBLER__

    int
    _getscom(uint32_t address, uint64_t* data, SsxInterval timeout);

    int
    getscom(uint32_t address, uint64_t* data);

    int
    _putscom(uint32_t address, uint64_t data, SsxInterval timeout);

    int
    putscom(uint32_t address, uint64_t data);

#endif  /* __ASSEMBLER__ */

// Error/Panic Codes

#define SCOM_PCB_ERROR_1_GETSCOM         0x00726601
#define SCOM_PCB_ERROR_2_GETSCOM         0x00726602
#define SCOM_PCB_ERROR_3_GETSCOM         0x00726603
#define SCOM_PCB_ERROR_4_GETSCOM         0x00726604
#define SCOM_PCB_ERROR_5_GETSCOM         0x00726605
#define SCOM_PCB_ERROR_6_GETSCOM         0x00726606
#define SCOM_PCB_ERROR_7_GETSCOM         0x00726607
#define SCOM_PCB_ERROR_1_PUTSCOM         0x00726608
#define SCOM_PCB_ERROR_2_PUTSCOM         0x00726609
#define SCOM_PCB_ERROR_3_PUTSCOM         0x0072660a
#define SCOM_PCB_ERROR_4_PUTSCOM         0x0072660b
#define SCOM_PCB_ERROR_5_PUTSCOM         0x0072660c
#define SCOM_PCB_ERROR_6_PUTSCOM         0x0072660d
#define SCOM_PCB_ERROR_7_PUTSCOM         0x0072660e

#define SCOM_TIMEOUT_ERROR               0x00726610
#define SCOM_TIMEOUT_ERROR_GETSCOM       0x00726611
#define SCOM_TIMEOUT_ERROR_PUTSCOM       0x00726612
#define SCOM_PROTOCOL_ERROR_GETSCOM      0x00726613
#define SCOM_PROTOCOL_ERROR_PUTSCOM      0x00726614
#define SCOM_PROTOCOL_ERROR_GETSCOM_BUSY 0x00726615
#define SCOM_PROTOCOL_ERROR_PUTSCOM_BUSY 0x00726616
#define SCOM_PROTOCOL_ERROR_PUTSCOM_RST  0x00726617
#define SCOM_PROTOCOL_ERROR_GETSCOM_RST  0x00726618
#define SCOM_INVALID_ADDRESS             0x00726619


/// The default timeout for getscom()/putscom()
///
/// This timeout is enforced by the firmware to guarantee a timeout regardless
/// of the hardware setup.
///
/// The expectation is that the hardware will be set up to enforce a PCB
/// timeout of 8K cycles, or 16.384us @ 500 MHz. A timeout only occurs if
/// someone erroneously issues a SCOM for a chiplet that does not exist. If
/// this happens, then all other SCOMS waiting for the timed-out SCOM to
/// finish will have to wait as well.  Aside from the timeout error, the
/// hardware arbitration and hard latency calculations guarantee that a SCOM
/// from any master will complete in under 1us, regardless of PCB utilization.
///
/// Assuming that the PCB timeout is a rare event that will only happen when a
/// console user types an illegal address for getscom/putscom, then we should
/// be able to use 17us as a hard upper bound for a soft timeout. In practice
/// this software timeout should never be observed - if it is, then something
/// (either the setup, hardware or firmware) must be broken.

#ifndef SCOM_TIMEOUT
    #define SCOM_TIMEOUT SSX_MICROSECONDS(17)
#endif


/// The default getscom()/putscom() error limit.
///
/// This defines the maximum PCB error return code which will not cause
/// getscom()/putscom() to panic.  The choice of this error limit is
/// application dependent. For lab applications we should never try to SCOM a
/// partial-good chiplet, so the limit is set at PCB_ERROR_CHIPLET_OFFLINE.
/// For production applications it may be that notification of garded cores is
/// late, so production code may need to handle PCB_ERROR_PARTIAL_GOOD as
/// well.

#ifndef SCOM_ERROR_LIMIT
    #define SCOM_ERROR_LIMIT PCB_ERROR_CHIPLET_OFFLINE
#endif

#endif // __OCCHW_SCOM_H__
