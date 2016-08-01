/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/pk/ppe42/ppe42_timebase.S $                           */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2016                        */
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
/// \file ppe42_timebase.S
/// \brief PPE42-specific 64 bit timebase emulation
///
        .nolist
#include "pk.h"
        .list
        
/// \fn PkTimebase pk_timebase_get(void)
/// \brief Returns a 64 bit timebase
///
#ifdef DOXYGEN_ONLY
PkTimebase 
pk_timebase_get(void);
#endif
        
/// \cond
        .global ppe42_64bit_timebase
        .global ppe42_tb_data
        .global_function pk_timebase_get

#ifndef APPCFG_USE_EXT_TIMEBASE

        /// Note that it is ok to use this function in a fast interrupt
        /// context
        .align 5
        .global_function pk_timebase32_get
pk_timebase32_get:

        //load the decrementer start time and change tag
        lvd     %r4, ppe42_tb_data@sda21(0)

        //load the lower 32 bits of the 64bit timebase accumulator
        lwz     %r3, ppe42_64bit_timebase+4@sda21(0)

        //load the current decrementer value
        mfdec   %r0

        //load the change tag again (should already be in the cache)
        lwz     %r6, ppe42_tb_data+4@sda21(0)

        //loop until the change tag is the same (typically should be same)
        cmplwbne %r5, %r6, pk_timebase32_get

        //calculate how much time has passed since the decrementer was started and store in r6
        subf    %r5, %r0, %r4

        //add the 32bit difference to our 32bit timebase accumulator
        add     %r3, %r5, %r3

        blr


/// Use the DEC for our timebase until we have a real timebase register (uses
/// 9 instructions).
/// Note: It is not ok to use this function in a fast interrupt context due to
///       its use of r7
        .align 5
pk_timebase_get:

        //load the decrementer start time and change tag
        lvd     %r5, ppe42_tb_data@sda21(0)

        //load 64bit timebase accumulator
        lvd     %r3, ppe42_64bit_timebase@sda21(0)


        //load the current decrementer value
        mfdec   %r0

        //load the change tag again (should already be in the cache)
        lwz     %r7, ppe42_tb_data+4@sda21(0)

        //loop until the change tag is the same
        cmplwbne %r6, %r7, pk_timebase_get

        //calculate how much time has passed since the decrementer was started and store in r6
        subf    %r6, %r0, %r5

        //add the 32bit difference to the 64bit timebase accumulator
        addc    %r4, %r6, %r4
        addze   %r3, %r3

        blr

//enable this once we have a local timebase register in the model
#else

// use the local timebase register to keep more accurate time with just 6 instructions
// in the common case and 7 otherwise.
        .align 5
pk_timebase_get:

        //load the 64bit timebase accumulator
        lvd         r3, ppe42_64bit_timebase@sda21(0)

        //read the local timebase register (2 instructions)
        _pk_timebase32_get r5, r5

        //increment the upper 32 bits if the lower 32 bits have flipped
        cmplwbge r5, r4, update_lower_32

        //increment the upper 32 bits
        addi        r3, r3, 1

update_lower_32:
        //replace the lower 32bits with what we read from the local timebase register
        mr          r4, r5

        blr
#endif /* APPCFG_USE_EXT_TIMEBASE */
/// \endcond
