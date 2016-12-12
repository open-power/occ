/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/mode.h $                                          */
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
#ifndef _mode_h
#define _mode_h

#include <occ_common.h>
#include <common_types.h>
#include "rtls.h"
#include "errl.h"


// Returns the current OCC Mode
#define CURRENT_MODE() G_occ_internal_mode

// Returns the 'OCC Requested' OCC Mode
#define REQUESTED_MODE()  G_occ_internal_req_mode

// Returns the 'Requested' SMS Mode
#define VALID_MODE(mode)  ((mode < OCC_MODE_COUNT) ? 1 : 0)

// Typedef of the various modes that TMGT can put OCC into.
typedef enum
{
    OCC_MODE_NOCHANGE          = 0x00,
    OCC_MODE_NOMINAL           = 0x01,
    // reserved                = 0x02,
    OCC_MODE_TURBO             = 0x03,
    OCC_MODE_SAFE              = 0x04,
    OCC_MODE_PWRSAVE           = 0x05,
    OCC_MODE_DYN_POWER_SAVE    = 0x06,
    OCC_MODE_MIN_FREQUENCY     = 0x07,

    // Mode 0x08-0x0A reserved by TMGT
    // reserved                = 0x08,
    // reserved                = 0x09,
    OCC_MODE_DYN_POWER_SAVE_FP = 0x0A,
    OCC_MODE_FFO               = 0x0B,
    OCC_MODE_FMF               = 0x0C,

    OCC_MODE_UTURBO            = 0x0D,

    // Make sure this is after the last valid mode
    OCC_MODE_COUNT,

    // These are used for mode transition table, and are not
    // a valid mode in and of itself.
    OCC_MODE_ALL              = 0xFE,
    OCC_MODE_INVALID          = 0xFF,
} OCC_MODE;

// These are the only modes that TMGT/HTMGT can send
#define OCC_MODE_IS_VALID(mode) ((mode == OCC_MODE_NOCHANGE) || \
                                 (mode == OCC_MODE_NOMINAL) || \
                                 (mode == OCC_MODE_TURBO) || \
                                 (mode == OCC_MODE_PWRSAVE) || \
                                 (mode == OCC_MODE_DYN_POWER_SAVE) || \
                                 (mode == OCC_MODE_DYN_POWER_SAVE_FP) || \
                                 (mode == OCC_MODE_FFO))

// Typedef of the various internal modes that OCC can be in.
typedef enum
{
    OCC_INTERNAL_MODE_NOM          = 0x00,
    OCC_INTERNAL_MODE_SPS          = 0x01,
    OCC_INTERNAL_MODE_DPS          = 0x02,
    OCC_INTERNAL_MODE_DPS_MP       = 0x03,
    OCC_INTERNAL_MODE_FFO          = 0x04,
    OCC_INTERNAL_MODE_MAX_NUM,
    OCC_INTERNAL_MODE_UNDEFINED    = 0xFF
} OCC_INTERNAL_MODE;

extern OCC_MODE           G_occ_internal_mode;
extern OCC_MODE           G_occ_internal_req_mode;
extern OCC_MODE           G_occ_external_req_mode;
extern OCC_MODE           G_occ_external_req_mode_kvm;
extern OCC_MODE           G_occ_master_mode;

// Returns true if we are in the middle of a mode transition
inline bool SMGR_is_mode_transitioning(void);

// Used to get the OCC Mode
inline OCC_MODE SMGR_get_mode(void);

// Used to set OCC Mode
errlHndl_t SMGR_set_mode(const OCC_MODE i_mode);



#endif
