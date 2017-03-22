/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/gpe_err.h $                                               */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2016                        */
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

/* This header file is used by all gpes                                   */
/* Contains common gpe return codes                                       */

#ifndef _GPE_ERR_H
#define _GPE_ERR_H

// List of general gpe Return Codes
#define GPE_RC_SUCCESS               0x00     // Success: No Errors
#define GPE_RC_SPI_TIMEOUT           0x01     // Timeout on previous SPI transaction
#define GPE_RC_SCOM_GET_FAILED       0x02     // Error on a SCOM read
#define GPE_RC_SCOM_PUT_FAILED       0x03     // Error on a SCOM write
#define GPE_RC_INVALID_REG           0x04     // Invalid SCOM Register used
#define GPE_RC_IPC_SEND_FAILED       0x05     // Failed to send an IPC message
#define GPE_RC_I2C_ERROR             0x06     // I2C error occurred
#define GPE_RC_INVALID_STATE         0x07     // Invalid state for requested operation
#define GPE_RC_NOT_COMPLETE          0x08     // Last operation did not complete

// Memory Power Control Error
#define GPE_RC_INVALID_MEM_PWR_CTL   0x20     // Invalid value for the memory power control command

// APSS Specific gpe return Codes
#define GPE_RC_INVALID_APSS_MODE     0x40     // OCC requested undefined APSS mode

// Core Data Errors
#define GPE_RC_GET_CORE_DATA_FAILED  0x60     // Failed to collect core data
#define GPE_RC_GET_NEST_DTS_FAILED   0x61     // Failed to collect nest DTS temperatures


#endif //_GPE_ERR_H
