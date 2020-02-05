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

// Typedef of the various modes.
typedef enum
{
    OCC_MODE_NOCHANGE          = 0x00,
    OCC_MODE_DISABLED          = 0x01,  // previously known as "nominal".  ASM/HMC always called this "disabled" and never "nominal" 
    OCC_MODE_STATIC_FREQ_POINT = 0x03,
    OCC_MODE_SAFE              = 0x04,  // not user settable
    OCC_MODE_PWRSAVE           = 0x05,
    OCC_MODE_FMAX              = 0x09,
    OCC_MODE_DYN_PERF          = 0x0A,
    OCC_MODE_FFO               = 0x0B,
    OCC_MODE_MAX_PERF          = 0x0C,

    // These are used for mode transition table, and are not
    // a valid mode in and of itself.
    OCC_MODE_ALL              = 0xFE,
    OCC_MODE_INVALID          = 0xFF,
} OCC_MODE;

// Additional set mode parameter when setting static freq point mode
typedef enum
{
    OCC_MODE_PARM_NONE             = 0x0000,
    OCC_MODE_PARM_VPD_CF0_PT       = 0x1000,
    OCC_MODE_PARM_VPD_CF1_PT       = 0x1001,
    OCC_MODE_PARM_VPD_CF2_PT       = 0x1002,
    OCC_MODE_PARM_VPD_CF3_PT       = 0x1003,
    OCC_MODE_PARM_VPD_CF4_PT       = 0x1004,
    OCC_MODE_PARM_VPD_CF5_PT       = 0x1005,
    OCC_MODE_PARM_VPD_CF6_PT       = 0x1006,
    OCC_MODE_PARM_VPD_CF7_PT       = 0x1007,
    OCC_MODE_PARM_MIN_FREQ_PT      = 0x2000,
    OCC_MODE_PARM_WOF_BASE_FREQ_PT = 0x2001,
    OCC_MODE_PARM_UT_FREQ_PT       = 0x2002,
    OCC_MODE_PARM_FMAX_FREQ_PT     = 0x2003
} OCC_MODE_PARM;

// Used to index G_sysConfigData.sys_mode_freq.table used to store all frequency points
typedef enum
{
    OCC_FREQ_PT_VPD_CF0 = 0x00,     // Curve Fit points must be first and grouped together
    OCC_FREQ_PT_VPD_CF1 = 0x01,
    OCC_FREQ_PT_VPD_CF2 = 0x02,
    OCC_FREQ_PT_VPD_CF3 = 0x03,
    OCC_FREQ_PT_VPD_CF4 = 0x04,
    OCC_FREQ_PT_VPD_CF5 = 0x05,
    OCC_FREQ_PT_VPD_CF6 = 0x06,
    OCC_FREQ_PT_VPD_LAST_CF = 0x07,
    OCC_FREQ_PT_WOF_BASE = 0x08,
    OCC_FREQ_PT_VPD_UT      = 0x09,
    OCC_FREQ_PT_MAX_FREQ    = 0x0A,
    OCC_FREQ_PT_MIN_FREQ    = 0x0B,
    OCC_FREQ_PT_MODE_DISABLED = 0x0C,
    OCC_FREQ_PT_MODE_PWR_SAVE = 0x0D,
    OCC_FREQ_PT_MODE_DYN_PERF = 0x0E,
    OCC_FREQ_PT_MODE_MAX_PERF = 0x0F,
    OCC_FREQ_PT_MODE_FMAX = 0x10,
    OCC_FREQ_PT_MODE_USER = 0x11,   // used for FFO and static freq pt modes
    OCC_FREQ_PT_COUNT,
    OCC_FREQ_PT_INVALID  = 0xFF
} OCC_FREQ_POINT;

// These are the only modes that TMGT/HTMGT can send
#define OCC_MODE_IS_VALID(mode) ((mode == OCC_MODE_NOCHANGE) || \
                                 (mode == OCC_MODE_DISABLED) || \
                                 (mode == OCC_MODE_STATIC_FREQ_POINT) || \
                                 (mode == OCC_MODE_PWRSAVE) || \
                                 (mode == OCC_MODE_FMAX) || \
                                 (mode == OCC_MODE_DYN_PERF) || \
                                 (mode == OCC_MODE_MAX_PERF) || \
                                 (mode == OCC_MODE_FFO))

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
