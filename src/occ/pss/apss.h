/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/pss/apss.h $                                          */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2014                        */
/* [+] Google Inc.                                                        */
/* [+] International Business Machines Corp.                              */
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

#ifndef _APSS_H
#define _APSS_H

#include <occ_common.h>
#include <trac_interface.h>
#include <errl.h>
#include <gpe_export.h>
#include <rtls.h>

#define NUM_OF_APSS_GPIO_PORTS   2
#define NUM_OF_APSS_PINS_PER_GPIO_PORT 8
#define MAX_APSS_ADC_CHANNELS   16
#define MAX_APSS_GPIO_PORTS      NUM_OF_APSS_GPIO_PORTS
#define MEAS_PADDING_REQUIRED (28-MAX_APSS_ADC_CHANNELS+MAX_APSS_GPIO_PORTS)
#define APSS_12BIT_ADC_MASK   0x0fff

#if ( (!defined(NO_TRAC_STRINGS)) && defined(TRAC_TO_SIMICS) )
void dumpHexString(const void *i_data, const unsigned int len, const char *string);
#endif

#define APSS_DATA_FAIL_MAX     32  //Number of steps we reach before reseting OCC.  This should allow for 4ms/16ticks with no APSS data.
#define APSS_DATA_FAILURE_STEP 2   //Number of steps to increment FAIL_COUNT due to a failed APSS data collection.
#define APSS_DATA_SUCCESS_STEP 1   //Number of steps to decrement the FAIL_COUNT due to successful APSS data collection.
#define APSS_ERRORLOG_RESET_THRESHOLD 16 //When to allow apss tasks to log another error if count goes back to 0 again.

extern uint8_t G_apss_fail_updown_count;           //Used to keep count of number of APSS data collection fails.

//Decrement APSS_FAIL_COUNT by APSS_DATA_SUCCESS_STEP to a minimum of 0.
#define APSS_SUCCESS() {(G_apss_fail_updown_count >= APSS_DATA_SUCCESS_STEP)? \
                        (G_apss_fail_updown_count -= APSS_DATA_SUCCESS_STEP): \
                        (G_apss_fail_updown_count = 0);}

// Increment APSS_FAIL_COUNT by APSS_DATA_FAILURE_STEP to a maximum of APSS_DATA_FAIL_MAX.
#define APSS_FAIL()   {((APSS_DATA_FAIL_MAX - G_apss_fail_updown_count) >= APSS_DATA_FAILURE_STEP)? \
                        (G_apss_fail_updown_count += APSS_DATA_FAILURE_STEP): \
                        (G_apss_fail_updown_count = APSS_DATA_FAIL_MAX);}

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

struct apssPwrMeasStruct
{
  uint16_t adc[MAX_APSS_ADC_CHANNELS];
  uint16_t gpio[MAX_APSS_GPIO_PORTS];
  uint16_t pad[MEAS_PADDING_REQUIRED]; // padding to allow TOD to be 8 byte aligned
  uint64_t tod;      // Time of Day that the ADC Collection Completed
} __attribute__ ((__packed__));
typedef struct apssPwrMeasStruct apssPwrMeasStruct_t;

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

// @TODO - Does G_gpio_config and G_apss_composite_config need to be used outside of APSS?  If not I will remove from .h
// G_gpio_config: configuration for APSS GPIO pins (default all input, all 1's, not int)
extern const apssGpioConfigStruct_t G_gpio_config[NUM_OF_APSS_GPIO_PORTS];
// G_apss_composite_config: system parms needed to select correct composite command options
extern const apssCompositeConfigStruct_t G_apss_composite_config;

// G_apss_pwr_meas: power, temp and GPIO readings that OCC gathers from APSS every tick
extern apssPwrMeasStruct_t G_apss_pwr_meas;

// Used to tell slave inbox that pwr meas is complete
extern volatile bool G_ApssPwrMeasCompleted;

extern initGpioArgs_t G_gpe_apss_initialize_gpio_args;
extern setCompositeModeArgs_t G_gpe_apss_set_composite_mode_args;

extern uint64_t G_gpe_apss_time_start;
extern uint64_t G_gpe_apss_time_end;

// apss_initialize is product applet
void task_apss_start_pwr_meas(task_t *i_self);
void task_apss_continue_pwr_meas(task_t *i_self);
void task_apss_complete_pwr_meas(task_t *i_self);

void apss_test_pwr_meas(); // @temp cc - used to test measurements
void reformat_meas_data();
bool apss_gpio_get(uint8_t i_pin_number, uint8_t *o_pin_value);

#endif //_APSS_H
