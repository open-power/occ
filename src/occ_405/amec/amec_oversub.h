/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/amec/amec_oversub.h $                             */
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

#ifndef _AMEC_OVERSUB_H
#define _AMEC_OVERSUB_H

/*----------------------------------------------------------------------------*/
/* Includes                                                                   */
/*----------------------------------------------------------------------------*/
#include <occ_sys_config.h>     // Added for sys config access
#include <dcom.h>

/*----------------------------------------------------------------------------*/
/* Constants                                                                  */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Globals                                                                    */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Defines                                                                    */
/*----------------------------------------------------------------------------*/
#define AMEC_INTF_GET_OVERSUBSCRIPTION() \
(G_sysConfigData.failsafe_enabled ? g_amec->oversub_status.cmeThrottlePinLive : \
                                   (g_amec->oversub_status.oversubPinLive || G_dcom_slv_inbox_rx.emulate_oversub))

#define AMEC_INTF_GET_OVERSUBSCRIPTION_EMULATION() g_amec->oversub_status.oversubPinMnfg

#define AMEC_INTF_GET_FAILSAFE() \
(G_sysConfigData.failsafe_enabled ? g_amec->oversub_status.oversubPinLive : 0)

/*----------------------------------------------------------------------------*/
/* Typedef / Enum                                                             */
/*----------------------------------------------------------------------------*/
typedef enum oversub_reason
{
   PENDING_OR_INVALID = 0x00,
   FANS_FULL_SPEED    = 0x01,
   FAN_ERRROR         = 0x02,
   FAN_WARNING        = 0x03,
   ITE_FAILSAFE       = 0x04,

   INDETERMINATE      = 0xFF
}oversub_reason_t;

typedef struct oversub_status
{
    // Way to emulate oversub for MNFG or Developers
    uint32_t    oversubPinMnfg      : 1;

    // Live Status of oversub Pin
    uint32_t    oversubPinLive      : 1;

    // AMEC status of oversub pin, so it doesn't
    // change mid-RTL
    uint32_t    oversubLatchAmec    : 1;

    // Used for SRC logging of performance loss,
    // need to have countdown b/c we don't get
    // APSS gpio signals as quick as we get
    // oversub.
    uint8_t    oversubReasonLatchCount;
    oversub_reason_t    oversubReason;

    // For debug, tracks time oversub last went inactive
    SsxTimebase oversubInactiveTime;

    // For debug, tracks time oversub last went active
    SsxTimebase oversubActiveTime;

    // Live status of CME throttle pin, doesn't change mid-RTL
    uint32_t cmeThrottlePinLive     :1;

    // Status of CME Throttle, doesn't get cleared until MM/TMGT
    // tells us to clear it/un-throttle.
    uint32_t cmeThrottleLatchAmec   :1;

    // Way to emulate cmeThrottle for MNFG or Developers
    uint32_t cmeThrottlePinMnfg     :1;
}oversub_status_t;

/*----------------------------------------------------------------------------*/
/* Function Prototypes                                                        */
/*----------------------------------------------------------------------------*/

void amec_oversub_isr(void);

void amec_oversub_check(void);

void amec_oversub_pmax_clip(Pstate i_pstate);

bool apss_gpio_get(uint8_t i_pin_number, uint8_t *o_pin_value);

#endif //_AMEC_OVERSUB_H
