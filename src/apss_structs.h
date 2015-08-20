/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/apss_structs.h $                                          */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2015                        */
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

#ifndef _APSS_STRUCTS_H
#define _APSS_STRUCTS_H

#include <gpe_export.h>

/* This data structure holds the common args data structures between the  */
/* 405 and the GPE0, used in IPC communications.                          */
/* We started by adding these common apss data structures to test the     */
/* code the way it worked before using the PORE GPE assembly. We will     */
/* probably have to add the apssPwrMeasStruct_t to include GPIO,          */
/* measurements, and to maintain a single data structure for all GPE0's   */
/* ASPSS routine (or may be consolidate the three routines in a single    */
/* function)                                                              */
/* this file is now kept in the main src directory. Will probably move to */
/* a common directory under src (e.g. src/occ_common/include?)            */


struct apssGpioConfigStruct
{
   uint8_t direction;
   uint8_t drive;
   uint8_t interrupt;
} __attribute__ ((__packed__));
typedef struct apssGpioConfigStruct apssGpioConfigStruct_t;

struct apssCompositeConfigStruct
{
   uint8_t numAdcChannelsToRead;
   uint8_t numGpioPortsToRead;
} __attribute__ ((__packed__));
typedef struct apssCompositeConfigStruct apssCompositeConfigStruct_t;

typedef struct {
  PoreGpeErrorStruct error;
  apssGpioConfigStruct_t config0; // G_gpio_config[0] (input to APSS)
  apssGpioConfigStruct_t config1; // G_gpio_config[1] (input to APSS)
} initGpioArgs_t;

typedef struct {
  PoreGpeErrorStruct error;
  apssCompositeConfigStruct_t config; // G_apss_composite_config (input to APSS)
} setCompositeModeArgs_t;

typedef struct
{
  PoreGpeErrorStruct error;
} apss_start_args_t;

typedef struct {
  PoreGpeErrorStruct error;
  uint64_t meas_data[4]; // G_apss_pwr_meas (1st block of data) (output from APSS)
} apss_continue_args_t;

typedef struct {
  PoreGpeErrorStruct error;
  uint64_t meas_data[4]; // G_apss_pwr_meas (2nd block of data) (output from APSS)
} apss_complete_args_t;

#endif //_APSS_STRUCTS_H
