/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/common/apss_structs.h $                                   */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2017                        */
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

/* This header file is used by both occ_405 and occ_gpe0.                 */
/* Contains common structures and globals.                                */

#ifndef _APSS_STRUCTS_H
#define _APSS_STRUCTS_H

#include <gpe_export.h>
#include "gpe_err.h"

// List of supported APSS Modes set in occ_gpe0/apss_init.c
#define APSS_MODE_COMPOSITE     0
#define APSS_MODE_AUTO2         1
#define MAX_APSS_ADC_CHANNELS   16

/* This data structure holds the common args data structures between the  */
/* 405 and the GPE0, used in IPC communications.                          */
/* We started by adding these common apss data structures to test the     */
/* code the way it worked before using the GPE assembly. We will          */
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

struct apssModeConfigStruct
{
    uint8_t mode;
    uint8_t numAdcChannelsToRead;
    uint8_t numGpioPortsToRead;
} __attribute__ ((__packed__));
typedef struct apssModeConfigStruct apssModeConfigStruct_t;

typedef struct {
  GpeErrorStruct error;
  apssGpioConfigStruct_t config0; // G_gpio_config[0] (input to APSS)
  apssGpioConfigStruct_t config1; // G_gpio_config[1] (input to APSS)
} initGpioArgs_t;

typedef struct {
  GpeErrorStruct error;
  apssModeConfigStruct_t config; // G_apss_composite_config (input to APSS)
} setApssModeArgs_t;

typedef struct
{
  GpeErrorStruct error;
} apss_start_args_t;

typedef struct {
  GpeErrorStruct error;
  uint64_t meas_data[4]; // G_apss_pwr_meas (1st block of data) (output from APSS)
} apss_continue_args_t;

typedef struct {
  GpeErrorStruct error;
  uint64_t meas_data[4]; // G_apss_pwr_meas (2nd block of data) (output from APSS)
} apss_complete_args_t;

typedef struct {
  GpeErrorStruct error;
} apss_reset_args_t;

#endif //_APSS_STRUCTS_H
