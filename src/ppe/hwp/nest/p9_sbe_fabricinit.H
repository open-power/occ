/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/hwp/nest/p9_sbe_fabricinit.H $                        */
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

///
/// @file p9_sbe_fabricinit.H
/// @brief Initialize island-mode fabric configuration (FAPI2)
///
/// The purpose of this procedure is to initalize the fabric.
///
/// In the post scan flush/init state, the fabric command/data init lines on
/// each chip are held de-asserted; this stops all local arbitration for
/// command/data requests from attached units.  At the conclusion of this
/// procedure, the fabric command/data init lines on the target chip
/// will be asserted.  Units on the target chip may make fabric requests that
/// will be processed and broadcast out to the SMP (consisting of the target
/// chip only), based upon the defined fabric broadcast protocols.
///
/// The initialization is accomplished by injecting an init command (special
/// ttype/tsize) into the fabric from the Alter Display Unit (ADU). This
/// command is permitted to be broadcast (even though the target chip is not
/// yet initialized), because the ADU is considered a trusted unit allowed
/// to broadcast commands at any time.
///
/// When the init reflected command is observed by the fabric snooper logic
/// (with target chip configured as fabric master), it will provide an
/// lpc_ack partial response that generates a clean combined response
/// (ack_done). Upon observation of the clean combined response broadcast,
/// the fabric snooper logic will assert its command/data init lines to
/// permit locally mastered requests to be arbitrated.
///
/// High-level procedure flow:
/// - Check state of tc_pb_stop (set by checkstop), which if
///   set would prohibit the init command from being broadcast
/// - Acquire the ADU lock to guarantee exclusive use of ADU resources
/// - Clear the ADU status register, reset ADU state machine
/// - Program the ADU to issue & launch the init command
/// - Check the status of the init command
/// - Release the ADU lock
/// - Confirm state of fabric init control
///
/// @author Joe McGill <jmcgill@us.ibm.com>
/// @author Christy Graves <clgraves@us.ibm.com>
///

//
// *HWP HWP Owner: Joe McGill <jmcgill@us.ibm.com>
// *HWP FW Owner: Thi Tran <thi@us.ibm.com>
// *HWP Team: Nest
// *HWP Level: 3
// *HWP Consumed by: SBE
//

#ifndef _P9_SBE_FABRICINIT_H_
#define _P9_SBE_FABRICINIT_H_


//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <fapi2.H>

//------------------------------------------------------------------------------
// Structure definitions
//------------------------------------------------------------------------------

/// function pointer typedef definition for HWP call support
typedef fapi2::ReturnCode (*p9_sbe_fabricinit_FP_t) (const fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP> &);


//------------------------------------------------------------------------------
// Function prototypes
//------------------------------------------------------------------------------

extern "C"
{

///
/// @brief Initialize fabric (in single chip 'island mode' configuration) by mastering
/// an init command (ttype=pbop.init_all) from the Alter Display Unit (ADU)
///
/// @param[in] i_target Reference to processor chip target
/// @return fapi::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
///
fapi2::ReturnCode p9_sbe_fabricinit(const fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP> & i_target);

} // extern "C"

#endif // _P9_SBE_FABRICINIT_H_
