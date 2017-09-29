/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/pk/ppe42/ppe42_core.c $                               */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2017                        */
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

/// \file ppe42_core.c
/// \brief The final bits of PK runtime code required to complete the PPE42
/// port.
///
/// The entry points in this file are considered 'core' routines that will
/// always be present during runtime in any PK application.

#define __PPE42_CORE_C__

#include "pk.h"

typedef union
{
    uint64_t    value;
    struct
    {
        uint32_t dec_start;
        uint32_t dec_change_tag;
    };
} ppe42_timebase_data_t;

ppe42_timebase_data_t ppe42_tb_data = {0};
PkTimebase  ppe42_64bit_timebase = 0;


#if PK_TIMER_SUPPORT

// The tickless kernel timer mechanism for PPE42
//
// This routine must be called from a critical section.
//
// Tickless timeouts are provided by programming the PIT timer based on when
// the next timeout will occur.  If the timeout is for the end of time there's
// nothing to do - PK does not use auto-reload mode so no more PIT interrupts
// will be arriving.  Otherwise, if the timeout is longer than the 32-bit PIT
// timer can handle, we simply schedule the timeout for 2**32 - 1 and
// __pk_timer_handler() will keep rescheduling it until it finally occurs.
// If the \a timeout is in the past, we schedule the PIT interrupt for 1 tick
// in the future in accordance with the PK specification.

#ifndef APPCFG_USE_EXT_TIMEBASE
void
__pk_schedule_hardware_timeout(PkTimebase timeout)
{
    PkTimebase       now;
    uint32_t         new_dec;
    uint32_t         dec;

    if (timeout != PK_TIMEBASE_MAX)
    {

        now = pk_timebase_get();

        if (timeout <= now)
        {
            new_dec = 1;
        }
        else if ((timeout - now) > 0xffff0000)
        {
            new_dec = 0xffff0000;
        }
        else
        {
            new_dec = timeout - now;
        }

        //read and write the DEC back-to-back so that we lose as little time
        //as possible
        dec = mfspr(SPRN_DEC);
        mtspr(SPRN_DEC, new_dec);

        //update our 64bit accumulator with how much time has advanced since
        //we last changed it.
        ppe42_64bit_timebase += ppe42_tb_data.dec_start - dec;

        //update our start time so we know how much time has advanced since
        //this update of the accumulator
        ppe42_tb_data.dec_start = new_dec;
        ppe42_tb_data.dec_change_tag++;
    }
}

#else

void
__pk_schedule_hardware_timeout(PkTimebase timeout)
{
    PkTimebase       now;
    PkTimebase       diff;
    uint32_t         new_dec;

    now = pk_timebase_get();
    //update the 64bit accumulator with the current
    ppe42_64bit_timebase =  now;

    if (timeout != PK_TIMEBASE_MAX)
    {
        if (timeout <= now)
        {
            new_dec = PK_DEC_MIN;
        }
        else
        {
            diff = (timeout - now);

            if (diff > 0xffff0000ull)
            {
                new_dec = 0xffff0000;
            }
            else
            {
                new_dec = diff;

                if(new_dec < PK_DEC_MIN)
                {
                    new_dec = PK_DEC_MIN;
                }
            }
        }

        mtspr(SPRN_DEC, new_dec);
    }
}

#endif  /* APPCFG_USE_EXT_TIMEBASE */

#endif  /* PK_TIMER_SUPPORT */

#undef __PPE42_CORE_C__
