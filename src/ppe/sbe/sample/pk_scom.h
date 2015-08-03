/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/sbe/sample/pk_scom.h $                                */
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
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2014
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file sample_main.c
/// \brief Sample program that creates and starts a thread
///
/// This file demonstrates how to create a thread and run it.  It also provides
/// an example of how to add traces to the code.

#ifndef __PK_SCOM_H__
#define __PK_SCOM_H__

/// SCOM operations return non-zero error codes that may or may not indicate
/// an actual error, depending on which SCOM is begin accessed.  This error
/// code is returned as the value of get/putscom().  The error code value
/// increases with teh severity of the error.
#define CFAM_FSI_STATUS_0x00001007  0x00001007
typedef union cfam_fsi_status_reg {
  uint64_t value;
  struct  {
    uint64_t ignore_fields1   : 17  ;
    uint64_t pib_error_code   : 3   ;
    uint64_t igore_fields2    : 44  ;
  }  fields;
}  cfam_fsi_status_reg_t;

#define PCB_ERROR_NONE              0
#define PCB_ERROR_RESOURCE_OCCUPIED 1
#define PCB_ERROR_CHIPLET_OFFLINE   2
#define PCB_ERROR_PARTIAL_GOOD      3
#define PCB_ERROR_ADDRESS_ERROR     4
#define PCB_ERROR_CLOCK_ERROR       5
#define PCB_ERROR_PACKET_ERROR      6
#define PCB_ERROR_TIMEOUT           7

uint32_t putscom( uint32_t i_chiplet, uint32_t i_address, uint64_t i_data);

uint32_t getscom( uint32_t i_chiplet, uint32_t i_address, uint64_t *o_data);

#endif  // __PK_SCOM_H__
