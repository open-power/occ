/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/mode.h $                                          */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2020                        */
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

// Typedef of the various modes
typedef enum
{
    OCC_MODE_NOCHANGE          = 0x00,
    OCC_MODE_NOMINAL           = 0x01,
    OCC_MODE_OVERSUB           = 0x02,  // not a settable mode, just used to store oversubscription max freq
    OCC_MODE_STATIC_FREQ_POINT = 0x03,
    OCC_MODE_SAFE              = 0x04,
    OCC_MODE_PWRSAVE           = 0x05,
    OCC_MODE_DYN_POWER_SAVE    = 0x06,
    OCC_MODE_MIN_FREQUENCY     = 0x07,  // not a settable mode, just used to store system min freq
    OCC_MODE_NOM_PERFORMANCE   = 0x08,
    OCC_MODE_MAX_PERFORMANCE   = 0x09,
    OCC_MODE_DYN_POWER_SAVE_FP = 0x0A,
    OCC_MODE_FFO               = 0x0B,
    OCC_MODE_FMF               = 0x0C,
    OCC_MODE_UTURBO            = 0x0D,  // not a settable mode, just used to store UT freq
    OCC_MODE_VRM_N             = 0x0E,  // not a settable mode, just used to store VRM N mode max freq
    OCC_MODE_WOF_BASE          = 0x0F,

    // Make sure this is after the last valid mode
    OCC_MODE_COUNT,

    // These are used for mode transition table, and are not
    // a valid mode in and of itself.
    OCC_MODE_ALL              = 0xFE,
    OCC_MODE_INVALID          = 0xFF,
} OCC_MODE;

typedef enum
{
    OCC_MODE_PARM_NONE    = 0x0000,
    VPD_CURVE_FIT_POINT_0 = 0x1000,
    VPD_CURVE_FIT_POINT_1 = 0x1001,
    VPD_CURVE_FIT_POINT_2 = 0x1002,
    VPD_CURVE_FIT_POINT_3 = 0x1003,
    VPD_CURVE_FIT_POINT_4 = 0x1004,
    VPD_CURVE_FIT_POINT_5 = 0x1005,
    VPD_CURVE_FIT_POINT_6 = 0x1006,
    VPD_CURVE_FIT_POINT_7 = 0x1007,
    MIN_FREQ_POINT        = 0x2000,
    WOF_BASE_POINT        = 0x2001,
    ULTRA_TURBO_POINT     = 0x2002,
    FMAX_FREQ_POINT       = 0x2003 
} OCC_MODE_PARM;

// Used to index the frequency point table
typedef enum
{
    OCC_FREQ_PT_VPD_CFT0 = 0x00,
    OCC_FREQ_PT_VPD_CFT1 = 0x01,
    OCC_FREQ_PT_VPD_CFT2 = 0x02,
    OCC_FREQ_PT_VPD_CFT3 = 0x03,
    OCC_FREQ_PT_VPD_CFT4 = 0x04,
    OCC_FREQ_PT_VPD_CFT5 = 0x05,
    OCC_FREQ_PT_VPD_CFT6 = 0x06,
    OCC_FREQ_PT_VPD_CFT7 = 0x07,
    OCC_FREQ_PT_PWR_SAVE = 0x08,
    OCC_FREQ_PT_WOF_BASE = 0x09,
    OCC_FREQ_PT_UT       = 0x0A,
    OCC_FREQ_PT_FMAX     = 0x0B,
    OCC_FREQ_PT_COUNT,
    OCC_FREQ_PT_INVALID  = 0xFF
} OCC_FREQ_POINT;

// These are the only modes that TMGT/HTMGT can send
#define OCC_MODE_IS_VALID(mode) ((mode == OCC_MODE_NOCHANGE) || \
                                 (mode == OCC_MODE_NOMINAL) || \
                                 (mode == OCC_MODE_STATIC_FREQ_POINT) || \
                                 (mode == OCC_MODE_PWRSAVE) || \
                                 (mode == OCC_MODE_DYN_POWER_SAVE) || \
                                 (mode == OCC_MODE_NOM_PERFORMANCE) || \
                                 (mode == OCC_MODE_MAX_PERFORMANCE) || \
                                 (mode == OCC_MODE_DYN_POWER_SAVE_FP) || \
                                 (mode == OCC_MODE_FFO) || \
                                 (mode == OCC_MODE_FMF))

// Typedef of the various internal modes that OCC can be in.
typedef enum
{
    OCC_INTERNAL_MODE_NOM          = 0x00,
    OCC_INTERNAL_MODE_SPS          = 0x01,
    OCC_INTERNAL_MODE_DPS          = 0x02,
    OCC_INTERNAL_MODE_DPS_MP       = 0x03,
    OCC_INTERNAL_MODE_FFO          = 0x04,
    OCC_INTERNAL_MODE_NOM_PERF     = 0x05,
    OCC_INTERNAL_MODE_MAX_PERF     = 0x06,
    OCC_INTERNAL_MODE_FMF          = 0x07,
    OCC_INTERNAL_MODE_MAX_NUM,
    OCC_INTERNAL_MODE_UNDEFINED    = 0xFF
} OCC_INTERNAL_MODE;

extern OCC_MODE           G_occ_internal_mode;
extern OCC_MODE           G_occ_internal_req_mode;
extern OCC_MODE           G_occ_external_req_mode;
extern OCC_MODE           G_occ_external_req_mode_kvm;
extern OCC_MODE           G_occ_master_mode;
extern uint16_t           G_occ_external_req_mode_parm;
// Returns true if we are in the middle of a mode transition
inline bool SMGR_is_mode_transitioning(void);

// Used to get the OCC Mode
inline OCC_MODE SMGR_get_mode(void);

// Used to set OCC Mode
errlHndl_t SMGR_set_mode(const OCC_MODE i_mode);



#endif
