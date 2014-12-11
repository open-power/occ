/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/amec/amec_analytics.c $                               */
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

//*************************************************************************
// Includes
//*************************************************************************
#include <amec_amester.h>
#include <amec_sys.h>
#include <proc_data.h>
#include <ssx.h>

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************

//*************************************************************************
// Structures
//*************************************************************************

//*************************************************************************
// Globals
//*************************************************************************

//*************************************************************************
// Function Prototypes
//*************************************************************************

//*************************************************************************
// Functions
//*************************************************************************

void amec_analytics_sb_recording(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    uint16_t                    temp16 = 0;
    uint8_t                     k = 0;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    // Increment the internal counter here per 250us tick
    g_amec->r_cnt++;

    // Need to have reached modulo time interval to record output and also just
    // after the analytics_slot was reached.
    temp16 = g_amec->r_cnt - g_amec->analytics_slot;
    temp16 = ((1<<g_amec->stream_vector_rate)-1) & temp16;

    if ((temp16 == 0) && (g_amec->recordflag == 1))
    {
        if (g_amec->stream_vector_mode == 0)  // If zero, see if timer running prior to beginning a capture
        {
            if (g_amec->stream_vector_delay > 1)
            {
                g_amec->stream_vector_delay = g_amec->stream_vector_delay - 1;
            } else
            {
                if (g_amec->stream_vector_delay == 1)
                {
                    g_amec->stream_vector_mode = 1;  // Turn on 1 shot recording
                    g_amec->write_stream_index = 0;  // Reset to start of buffer
                    g_amec->stream_vector_delay = 0; // Disable any further delays

                    // support L4 state machine and tracing being synchronized
                    if (g_amec->reset_prep != 0)
                    {
                        g_amec->cent_l4_state[g_amec->probe_l4_centaur] = 0;      // Start with L4 state machine set to first state (L4_S0)
                        g_amec->cent_l4_ipl_state[g_amec->probe_l4_centaur] = 0;  // Start with L4 IPL state machine set to first state (IPL_L4_S0)
                        g_amec->reset_prep = 0;             // Turn off indicator of TMGT wanting to reset the OCC, which will start L4 state machine
                        g_amec->l4_powerdown_requestm = 1;  // Raise indicator that the master OCC wants to carry out an L4 power down
                    }
                }
            }
        } else
        {
            // Check is discarding initial frames due to analytics data getting averaged
            if (g_amec->analytics_bad_output_count == 0)
            {
                // Stream buffer recording function done every 250usec * 2^(stream_vector_rate)
                g_amec->ptr_stream_buffer[g_amec->write_stream_index] = (uint16_t)g_amec->r_cnt;
                g_amec->write_stream_index++;
                // WARNING -> The size of the vector recorded must be a precise multiple
                // of the size of the entire stream buffer.
                for (k = 1; k < STREAM_VECTOR_SIZE_EX; k++)
                {
                    if (g_amec->stream_vector_map[k] == (void *)0xffffffff)
                    {
                        k = STREAM_VECTOR_SIZE_EX;  // Terminate as partial vector complete
                    } else
                    {
                        temp16 = *((uint16_t * )(g_amec->stream_vector_map[k]));
                        g_amec->ptr_stream_buffer[g_amec->write_stream_index] = (uint16_t)temp16;
                        g_amec->write_stream_index++;
                    }
                }
                if (g_amec->write_stream_index >= STREAM_BUFFER_SIZE)
                {
                    g_amec->write_stream_index = 0;  // Reset to start of buffer
                    if (g_amec->stream_vector_mode == 1)
                    {
                        // If single shot, just rotate write ptr in last record
                        g_amec->write_stream_index = STREAM_BUFFER_SIZE - STREAM_VECTOR_SIZE_EX;
                    }
                }
            } else
            {
                g_amec->analytics_bad_output_count--; // decrement bad output counter
            }
        }
    }
}

void amec_analytics_main(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    uint8_t                     i = 0;
    uint8_t                     j = 0;
    uint8_t                     k = 0;
    uint8_t                     l = 0;
    uint8_t                     m = 0;
    uint16_t                    temp16 = 0;
    uint16_t                    tempreg = 0;
    uint32_t                    temp32 = 0;
    uint32_t                    tempaccum = 0;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    g_amec->packednapsleep[0] = (g_amec->proc[0].winkcnt2ms.sample<<8) +
        g_amec->proc[0].sleepcnt2ms.sample;
    g_amec->packednapsleep[1] = (g_amec->proc[1].winkcnt2ms.sample<<8) +
        g_amec->proc[1].sleepcnt2ms.sample;
    g_amec->packednapsleep[2] = (g_amec->proc[2].winkcnt2ms.sample<<8) +
        g_amec->proc[2].sleepcnt2ms.sample;
    g_amec->packednapsleep[3] = (g_amec->proc[3].winkcnt2ms.sample<<8) +
        g_amec->proc[3].sleepcnt2ms.sample;

    switch (g_amec->analytics_group)
    {
        case 44:  // Group 44

            // Every 2msec (250usec * 2^stream_rate, default stream_rate=3), perform averaging of sensors.
            // Averaging is required because many sensors are updated every
            // 2msec and if they aren't properly averaged, those updates
            // are lost in the final analytics output.
            // The analytics group should be a correct average of the higher
            // frequency sensor updates.
            // (wait until OCC master collects all chips data)
            for (i=0; i<1; i++)
            {
                g_amec->g44_avg[(i*MSA)+0] = (UINT32)g_amec->sys.todclock0.sample;   // ptr to high 16 bits of 48bit TOD clock
                g_amec->g44_avg[(i*MSA)+2] = (UINT32)g_amec->sys.todclock1.sample;   // ptr to middle 16 bits of 48 bit TOD clock
                g_amec->g44_avg[(i*MSA)+4] = (UINT32)g_amec->sys.todclock2.sample;   // ptr to low 16 bits of 48 bit TOD clock

                tempaccum=g_amec->sys.pwr250us.src_accum_snapshot;           // load pwr250us accum from last 2msec
                g_amec->sys.pwr250us.src_accum_snapshot = g_amec->sys.pwr250us.accumulator;  // save current accum state for next 2msec
                tempaccum=g_amec->sys.pwr250us.accumulator - tempaccum;    // total accumulation over 2msec
                tempaccum=tempaccum>>3;                                      // divide by 8
                g_amec->g44_avg[(i*MSA)+6] = g_amec->g44_avg[(i*MSA)+6] +
                    tempaccum;

                tempaccum=g_amec->sys.pwr250usgpu.src_accum_snapshot;       // load pwr250usgpu accum from last 2msec
                g_amec->sys.pwr250usgpu.src_accum_snapshot = g_amec->sys.pwr250usgpu.accumulator;  // save current accum state for next 2msec
                tempaccum=g_amec->sys.pwr250usgpu.accumulator - tempaccum;  // total accumulation over 2msec
                tempaccum=tempaccum>>3;                                     // divide by 8
                g_amec->g44_avg[(i*MSA)+8] = g_amec->g44_avg[(i*MSA)+8] +
                    tempaccum;

                tempaccum=g_amec->proc[i].pwr250us.src_accum_snapshot;     // load accumulator from last 2msec
                g_amec->proc[i].pwr250us.src_accum_snapshot = g_amec->proc[i].pwr250us.accumulator;  // save current accum state for next 2msec
                tempaccum=g_amec->proc[i].pwr250us.accumulator - tempaccum;    // total accumulation over 2msec
                tempaccum=tempaccum>>3;                                      // divide by 8
                g_amec->g44_avg[(i*MSA)+10] = g_amec->g44_avg[(i*MSA)+10] +
                    tempaccum;

                tempaccum=g_amec->proc[i].pwr250usvdd.src_accum_snapshot;     // load accumulator from last 2msec
                g_amec->proc[i].pwr250usvdd.src_accum_snapshot = g_amec->proc[i].pwr250usvdd.accumulator;  // save current accum state for next 2msec
                tempaccum=g_amec->proc[i].pwr250usvdd.accumulator - tempaccum;    // total accumulation over 2msec
                tempaccum=tempaccum>>3;
                g_amec->g44_avg[(i*MSA)+11] = g_amec->g44_avg[(i*MSA)+11] +
                    tempaccum;

                tempaccum=g_amec->proc[i].vrm[0].volt250us.src_accum_snapshot;     // load accumulator from last 2msec
                g_amec->proc[i].vrm[0].volt250us.src_accum_snapshot = g_amec->proc[i].vrm[0].volt250us.accumulator;  // save current accum state for next 2msec
                tempaccum=g_amec->proc[i].vrm[0].volt250us.accumulator - tempaccum;    // total accumulation over 2msec
                temp32 = tempaccum<<3;  // Pi, Vdd
                tempreg = 4000;
                // Convert voltage from 100uV resolution to 6.25mV resolution
                tempreg = (UINT16)(UTIL_DIV32(temp32, tempreg));
                g_amec->g44_avg[(i*MSA)+12] = g_amec->g44_avg[(i*MSA)+12] + (UINT32)tempreg;

                tempaccum=g_amec->proc[i].vrm[1].volt250us.src_accum_snapshot;     // load accumulator from last 2msec
                g_amec->proc[i].vrm[1].volt250us.src_accum_snapshot = g_amec->proc[i].vrm[1].volt250us.accumulator;  // save current accum state for next 2msec
                tempaccum=g_amec->proc[i].vrm[1].volt250us.accumulator - tempaccum;    // total accumulation over 2msec
                temp32 = tempaccum<<3;  // Pi, Vcs
                tempreg = 4000;
                // Convert voltage from 100uV resolution to 6.25mV resolution
                tempreg = (UINT16)(UTIL_DIV32(temp32, tempreg));
                g_amec->g44_avg[(i*MSA)+13] = g_amec->g44_avg[(i*MSA)+13] +
                    (UINT32)tempreg;

                tempaccum=g_amec->proc[i].cur250usvdd.src_accum_snapshot;     // load accumulator from last 2msec
                g_amec->proc[i].cur250usvdd.src_accum_snapshot = g_amec->proc[i].cur250usvdd.accumulator;  // save current accum state for next 2msec
                tempaccum=g_amec->proc[i].cur250usvdd.accumulator - tempaccum;    // total accumulation over 2msec
                tempaccum=tempaccum>>3;
                g_amec->g44_avg[(i*MSA)+14] = g_amec->g44_avg[(i*MSA)+14] +
                    tempaccum/100;
                g_amec->g44_avg[(i*MSA)+15] = g_amec->g44_avg[(i*MSA)+15] +
                    (UINT32)g_amec->proc[i].temp2ms.sample;    // hottest processor core temperature (average??)
                g_amec->g44_avg[(i*MSA)+16] = g_amec->g44_avg[(i*MSA)+16] +
                    (UINT32)(g_amec->proc[i].memctl[4].mrd2ms.sample/78); // memory read bandwidth
                g_amec->g44_avg[(i*MSA)+17] = g_amec->g44_avg[(i*MSA)+17] +
                    (UINT32)(g_amec->proc[i].memctl[4].mwr2ms.sample/78); // memory write bandwidth
                g_amec->g44_avg[(i*MSA)+18] = g_amec->g44_avg[(i*MSA)+18] +
                    (UINT32)(g_amec->proc[i].memctl[5].mrd2ms.sample/78); // memory read bandwidth
                g_amec->g44_avg[(i*MSA)+19] = g_amec->g44_avg[(i*MSA)+19] +
                    (UINT32)(g_amec->proc[i].memctl[5].mwr2ms.sample/78); // memory write bandwidth

                g_amec->g44_avg[(i*MSA)+20] = g_amec->g44_avg[(i*MSA)+20] |
                    (UINT32)g_amec->proc[i].winkcnt2ms.sample;    // winkle bitmap for all cores (OR of all 2msec)
                g_amec->g44_avg[(i*MSA)+21] = g_amec->g44_avg[(i*MSA)+21] |
                    (UINT32)g_amec->proc[i].sleepcnt2ms.sample;   // sleep bitmap for all cores (OR of all 2msec)

                g_amec->g44_avg[(i*MSA)+22] = g_amec->g44_avg[(i*MSA)+22] +
                    (UINT32)(g_amec->proc[i].memctl[6].mrd2ms.sample/78); // memory read bandwidth
                g_amec->g44_avg[(i*MSA)+23] = g_amec->g44_avg[(i*MSA)+23] +
                    (UINT32)(g_amec->proc[i].memctl[6].mwr2ms.sample/78); // memory write bandwidth
                g_amec->g44_avg[(i*MSA)+24] = g_amec->g44_avg[(i*MSA)+24] +
                    (UINT32)(g_amec->proc[i].memctl[7].mrd2ms.sample/78); // memory read bandwidth
                g_amec->g44_avg[(i*MSA)+25] = g_amec->g44_avg[(i*MSA)+25] +
                    (UINT32)(g_amec->proc[i].memctl[7].mwr2ms.sample/78); // memory write bandwidth

                m=0;                     // counter for actual configured # of cores - 1.
                for (j=0; j<12; j++)     // WARNING: Only 8 cores are allowed to be configured per OCC chip
                {
                    if (CORE_PRESENT(j))
                    {
                        //average frequency for this core
                        g_amec->g44_avg[(i*MSA)+30+m] = g_amec->g44_avg[(i*MSA)+30+m] +
                            (UINT32)g_amec->proc[i].core[j].freqa2ms.sample/28;
                        m++;              // increment configured core counter
                        if (m > 7) j=12;  // safeguard in case more than 8 configured cores.
                    }
                }

                m=0;                     // counter for actual configured # of cores - 1.
                for (j=0; j<12; j++)     // WARNING: Only 8 cores are allowed to be configured per OCC chip
                {
                    if (CORE_PRESENT(j))
                    {
                        tempreg = 0;     // keeps track of maximum thread utilization for this core
                        temp32 = 0;      // keeps track of average thread utilization for this core for non-zero threads (threadmode=0) or all threads (threadmode=1) or no average (threadmode=2)
                        temp16 = 0;      // keeps track of non-zero threads
                        for (k=0; k < g_amec->analytics_threadcountmax; k++)
                        {
                        if (tempreg < g_amec->proc[i].core[j].thread[k].util2ms_thread)
                            {
                            tempreg = g_amec->proc[i].core[j].thread[k].util2ms_thread;
                            }
                        if ((0 < g_amec->proc[i].core[j].thread[k].util2ms_thread) ||
                            (g_amec->analytics_threadmode !=  0))
                        {
                            // accumulate for computing average
                            temp32 = temp32 + g_amec->proc[i].core[j].thread[k].util2ms_thread;
                            // increment counter of threads
                            temp16 = temp16 + 1;
                        }
                        }
                        g_amec->g44_avg[(i*MSA)+38+m] = g_amec->g44_avg[(i*MSA)+38+m] +
                            (UINT32)(g_amec->proc[i].core[j].util2ms.sample/50);  // accumulate util sensor that feeds IPS and DPS algorithms for this core

                        if (g_amec->analytics_threadmode == 2)
                        {
                            temp16 = tempreg;          // Store maximum of all the threads on this core
                        }
                        if (g_amec->analytics_threadmode < 2)
                        {
                            if (temp16 > 0)
                            {
                                temp16 = (UINT16)(UTIL_DIV32(temp32, temp16));  // compute average utilization of all non-zero threads (threadmode=0) or all threads (threadmode=1)
                            }
                        }
                        if (g_amec->analytics_threadmode == 3)
                        {
                            // accumulate average finish latency counter for this core
                            temp16 = ((g_amec->proc[i].core[j].mcpifi2ms.sample) >>1);
                        }
                        if (g_amec->analytics_threadmode == 4)
                        {
                            if ((m&1) == 0)       // Capture L4 read traffic
                            {
                                temp16 = g_amec->proc[0].memctl[(m/2)+4].centaur.portpair[0].perf.l4rd2ms;
                                temp16 = temp16 + g_amec->proc[0].memctl[(m/2)+4].centaur.portpair[1].perf.l4rd2ms;
                                temp16 = temp16 >> 1;
                            }
                            else
                            {
                                temp16 = g_amec->proc[0].memctl[(m/2)+4].centaur.portpair[0].perf.l4wr2ms;
                                temp16 = temp16 + g_amec->proc[0].memctl[(m/2)+4].centaur.portpair[1].perf.l4wr2ms;
                                temp16 = temp16 >> 1;
                            }
                        }

                        g_amec->g44_avg[(i * MSA) + 46 + m] = g_amec->g44_avg[(i * MSA) + 46 + m] +
                            (UINT32)(temp16 / 50);   // accumulate average utilization or individual threads for this core or finish latency counter or L4 read/write counters
                        g_amec->g44_avg[(i * MSA) + 54 + m] = g_amec->g44_avg[(i * MSA) + 54 + m] +
                            (UINT32)(g_amec->proc[i].core[j].ips2ms.sample / 50);  // accumulate average MIPS for this core
                        g_amec->g44_avg[(i * MSA) + 62 + m] = g_amec->g44_avg[(i * MSA) + 62 + m] +
                            (UINT32)g_amec->proc[i].core[j].temp2ms.sample; // accumulate average temperature for this core
                        g_amec->g44_avg[(i * MSA) + 70 + m] = g_amec->g44_avg[(i * MSA) + 70 + m] +
                            (UINT32)((g_amec->proc[i].core[j].cmbw2ms.sample) / 78); // accumulate average memory bandwidth for this core
                        temp16 = ((g_amec->proc[i].core[j].mcpifd2ms.sample) / 100);    // accumulate average busy latency counter for this core
                        g_amec->g44_avg[(i * MSA) + 78 + m] = g_amec->g44_avg[(i * MSA) + 78 + m] + (UINT32)temp16;
                        m++;              // increment configured core counter
                        if (m > 7) j = 12;   // safeguard in case more than 8 configured cores.
                    }
                }  // End loop processing each core
            } // End loop processing each chip

            // Determine when to update final analytics_array
            temp16 = g_amec->r_cnt - g_amec->analytics_slot;
            temp16 = ((1<<g_amec->stream_vector_rate)-1) & temp16;

            // Have we completed this interval so that we can output?
            if (temp16 == 0)
            {
                // Now, update Group 44 analytics packed array
                switch (g_amec->analytics_thermal_offset)
                {
                    case 0:
                        tempreg = (g_amec->sys.tempambient.sample) << 8;   // upper byte
                        tempreg = tempreg | 0x8000;    // Turn on MSBit for temporal frame sync
                        break;

                    case 1:
                         if (g_amec->mst_ips_parms.active == 0)
                         {
                             tempreg = 0;   // If not in IPS mode, return 0
                         }
                         else
                         {
                             tempreg = 127; // If in IPS, return constant indicating in IPS mode
                         }
                        if (tempreg > 127) tempreg = 127;  // Saturate at 7 bit limit (508 seconds)
                        tempreg = (tempreg) << 8;   // upper byte
                        break;

                    case 2:
                        tempreg=(g_amec->mst_ips_parms.active)<<8;   // upper byte
                        break;

                    case 3:
                        tempreg = (g_amec->fan.fanspeedavg.sample / 100) << 8;   // upper byte (100 RPM resolution)
                        break;

                    case 4:
                        tempreg = (g_amec->proc[0].temp2msdimm.sample) << 8;   // upper byte
                        break;

                    case 5:
                        tempreg = (g_amec->proc[0].temp2mscent.sample) << 8;   // upper byte
                        break;

                    case 6:
                        // tempreg=(g_amec->proc[2].temp2msdimm.sample)<<8;   // upper byte
                        tempreg = 0;
                        break;

                    case 7:
                        // tempreg=(g_amec->proc[2].temp2mscent.sample)<<8;   // upper byte
                        tempreg = 0;
                        break;

                    default:
                        break;

                }
                g_amec->analytics_thermal_offset = 0x7 &
                        (g_amec->analytics_thermal_offset + 1); // modulo 8

                tempaccum = g_amec->fan.pwr250usfan.src_accum_snapshot;     // load accumulator from last 2msec
                g_amec->fan.pwr250usfan.src_accum_snapshot = g_amec->fan.pwr250usfan.accumulator;  // save current accum state for next 2msec
                tempaccum = g_amec->fan.pwr250usfan.accumulator - tempaccum;    // total accumulation over 2msec
                tempaccum = tempaccum >> g_amec->stream_vector_rate;

                tempreg = tempreg | (0xff & ((UINT16)tempaccum));
                g_amec->analytics_array[5] = tempreg;

                tempaccum = g_amec->io.pwr250usio.src_accum_snapshot;     // load accumulator from last 2msec
                g_amec->io.pwr250usio.src_accum_snapshot = g_amec->io.pwr250usio.accumulator;  // save current accum state for next 2msec
                tempaccum = g_amec->io.pwr250usio.accumulator - tempaccum;    // total accumulation over 2msec
                tempaccum = tempaccum >> g_amec->stream_vector_rate;

                tempreg = ((UINT16)tempaccum) << 8;   // upper byte

                tempaccum = g_amec->storage.pwr250usstore.src_accum_snapshot;     // load accumulator from last 2msec
                g_amec->storage.pwr250usstore.src_accum_snapshot = g_amec->storage.pwr250usstore.accumulator;  // save current accum state for next 2msec
                tempaccum = g_amec->storage.pwr250usstore.accumulator - tempaccum;    // total accumulation over 2msec
                tempaccum = tempaccum >> g_amec->stream_vector_rate;

                tempreg = tempreg | (0xff & ((UINT16)tempaccum));
                g_amec->analytics_array[6] = tempreg;

                tempaccum = g_amec->proc[0].pwr250usmem.src_accum_snapshot;     // load accumulator from last 2msec
                g_amec->proc[0].pwr250usmem.src_accum_snapshot = g_amec->proc[0].pwr250usmem.accumulator;  // save current accum state for next 2msec
                tempaccum = g_amec->proc[0].pwr250usmem.accumulator - tempaccum;    // total accumulation over 2msec
                tempaccum = tempaccum >> g_amec->stream_vector_rate;

                tempreg = ((UINT16)tempaccum) << 8;   // upper byte

                tempaccum = g_amec->proc[1].pwr250usmem.src_accum_snapshot;     // load accumulator from last 2msec
                g_amec->proc[1].pwr250usmem.src_accum_snapshot = g_amec->proc[1].pwr250usmem.accumulator;  // save current accum state for next 2msec
                tempaccum = g_amec->proc[1].pwr250usmem.accumulator - tempaccum;    // total accumulation over 2msec
                tempaccum = tempaccum >> g_amec->stream_vector_rate;
                tempaccum = 0;                                                  // force to 0 until master OCC collects

                tempreg = tempreg | (0xff & ((UINT16)tempaccum));
                g_amec->analytics_array[7] = tempreg;

                tempaccum = g_amec->proc[2].pwr250usmem.src_accum_snapshot;     // load accumulator from last 2msec
                g_amec->proc[2].pwr250usmem.src_accum_snapshot = g_amec->proc[2].pwr250usmem.accumulator;  // save current accum state for next 2msec
                tempaccum = g_amec->proc[2].pwr250usmem.accumulator - tempaccum;    // total accumulation over 2msec
                tempaccum = tempaccum >> g_amec->stream_vector_rate;
                tempaccum = 0;                                                  // force to 0 until master OCC collects

                tempreg = ((UINT16)tempaccum) << 8;   // upper byte

                tempaccum = g_amec->proc[3].pwr250usmem.src_accum_snapshot;     // load accumulator from last 2msec
                g_amec->proc[3].pwr250usmem.src_accum_snapshot = g_amec->proc[3].pwr250usmem.accumulator;  // save current accum state for next 2msec
                tempaccum = g_amec->proc[3].pwr250usmem.accumulator - tempaccum;    // total accumulation over 2msec
                tempaccum = tempaccum >> g_amec->stream_vector_rate;
                tempaccum = 0;                                                  // force to 0 until master OCC collects

                tempreg = tempreg | (0xff & ((UINT16)tempaccum));
                g_amec->analytics_array[8] = tempreg;

                // Now begins the per processor unique data
                tempreg = (g_amec->analytics_total_chips) << 8;   // upper byte
                tempreg = tempreg | (0xff & (g_amec->analytics_chip));  // which chip is outputting this interval?
                g_amec->analytics_array[9] = tempreg;
                j = g_amec->analytics_chip;    // select which chip to process

                if (g_amec->analytics_option == 0)
                {
                    k = 0;   // Default to no shift, if user didn't enter analytics_total_chips (set to 0)
                    if (g_amec->analytics_total_chips == 1) k = g_amec->stream_vector_rate - 3;   // (2msec * 2^k) is shift for averaging interval (16msec)
                    if (g_amec->analytics_total_chips == 2) k = g_amec->stream_vector_rate - 2;   // (2msec * 2^k) is shift for averaging interval (32msec)
                    if (g_amec->analytics_total_chips == 4) k = g_amec->stream_vector_rate - 1;   // (2msec * 2^k) is shift for averaging interval (64msec)
                    if (g_amec->analytics_total_chips == 8) k = g_amec->stream_vector_rate;     // (2msec * 2^k) is shift for averaging interval (128msec)
                }
                if (g_amec->analytics_option == 1)
                {
                    k = g_amec->stream_vector_rate - 3;                                     // (2msec * 2^k) is shift for averaging interval (16msec)
                }

                l = 12;     // index offset

                g_amec->analytics_array[0] = (UINT16)g_amec->g44_avg[(j * MSA) + 0];          // todclock1 (hi 16 bits, no averaging)
                g_amec->analytics_array[1] = (UINT16)g_amec->g44_avg[(j * MSA) + 2];          // todclock1 (medium 16 bits, no averaging)
                g_amec->analytics_array[2] = (UINT16)g_amec->g44_avg[(j * MSA) + 4];          // todclock1 (lo 16 bits, no averaging)

                g_amec->analytics_array[3] = (UINT16)(g_amec->g44_avg[(j * MSA) + 6] >> k);    // the first two averages are 16 bits
                g_amec->g44_avg[(j * MSA) + 6] = 0;                                  // reset average for this sensor to 0
                g_amec->analytics_array[4] = (UINT16)(g_amec->g44_avg[(j * MSA) + 8] >> k);    // the first two averages are 16 bits
                g_amec->g44_avg[(j * MSA) + 8] = 0;                                  // reset average for this sensor to 0
                g_amec->analytics_array[10] = (UINT16)(g_amec->g44_avg[(j * MSA) + 10] >> k);    // the first two averages are 16 bits
                g_amec->g44_avg[(j * MSA) + 10] = 0;                                  // reset average for this sensor to 0
                g_amec->analytics_array[11] = (UINT16)(g_amec->g44_avg[(j * MSA) + 11] >> k);  // the first two averages are 16 bits
                g_amec->g44_avg[(j * MSA) + 11] = 0;                                // reset average for this sensor to 0

                for (i = 12; i <= 15; i++)
                {
                    temp16 = (UINT16)(g_amec->g44_avg[(j * MSA) + l] >> k);
                    tempreg = temp16 << 8;                 // upper byte
                    temp16 = (UINT16)(g_amec->g44_avg[(j * MSA) + l + 1] >> k);
                    tempreg = tempreg | (0xff & temp16);
                    g_amec->analytics_array[i] = tempreg;
                    g_amec->g44_avg[(j * MSA) + l] = 0; // Reset average for this sensor to 0
                    g_amec->g44_avg[(j * MSA) + l + 1] = 0; // Reset average for this sensor to 0

                    l = l + 2;
                }

                // Now copy over sleep and nap sensors.
                // WARNING--> no division as these sensors are "OR"ed together
                // every 32msec: "OR" keeps deepest state reached
                temp16 = (UINT16)g_amec->g44_avg[(j * MSA) + l];
                tempreg = temp16 << 8;                 // upper byte
                temp16 = (UINT16)g_amec->g44_avg[(j * MSA) + l + 1];
                tempreg = tempreg | (0xff & temp16);
                g_amec->analytics_array[i] = tempreg;
                g_amec->g44_avg[(j * MSA) + l] = 0;   // Reset all indicators of cores napping to begin next N intervals
                g_amec->g44_avg[(j * MSA) + l + 1] = 0; // Reset all indicators of cores sleeping to begin next N intervals
                i = i + 1;
                l = l + 2;

                // Now process CPM data...instead of computing the average, the
                // worst case (minimum) CPM value was maintained. Just copy it
                // over. Then reset each CPM to be 15 (maximum value) to start next
                // monitoring interval
                // for (i=17; i<=18; i++)
                // {
                //    tempreg=(UINT16)g_amec->g44_avg[(j*MSA)+l]<<12;                        // hi nybble
                //    tempreg=tempreg | ((0xf & ((UINT16)g_amec->g44_avg[(j*MSA)+l+1]))<<8); // mid-hi nybble
                //    tempreg=tempreg | ((0xf & ((UINT16)g_amec->g44_avg[(j*MSA)+l+2]))<<4); // mid-lo nybble
                //    tempreg=tempreg | ((0xf & ((UINT16)g_amec->g44_avg[(j*MSA)+l+3])));    // lo nybble
                //    g_amec->analytics_array[i]=tempreg;
                //    g_amec->g44_avg[(j*MSA)+l] = 15;     // Reset all CPM indicators to 15
                //    g_amec->g44_avg[(j*MSA)+l+1] = 15;   // Reset all CPM indicators to 15
                //    g_amec->g44_avg[(j*MSA)+l+2] = 15;   // Reset all CPM indicators to 15
                //    g_amec->g44_avg[(j*MSA)+l+3] = 15;   // Reset all CPM indicators to 15
                //    l=l+4;
                // }

                // process last 2 Centaurs' memory bandwidth sensors
                for (i = 17; i <= 18; i++)
                {
                    temp16 = (UINT16)(g_amec->g44_avg[(j * MSA) + l] >> k);
                    tempreg = temp16 << 8;                 // upper byte
                    temp16 = (UINT16)(g_amec->g44_avg[(j * MSA) + l + 1] >> k);
                    tempreg = tempreg | (0xff & temp16);
                    g_amec->analytics_array[i] = tempreg;
                    g_amec->g44_avg[(j * MSA) + l] = 0;   // Reset average for this sensor to 0
                    g_amec->g44_avg[(j * MSA) + l + 1] = 0; // Reset average for this sensor to 0
                    l = l + 2;
                }

                for (i = 19; i <= 46; i++)
                {
                    temp16 = (UINT16)(g_amec->g44_avg[(j * MSA) + l] >> k);
                    tempreg = temp16 << 8;                 // upper byte
                    temp16 = (UINT16)(g_amec->g44_avg[(j * MSA) + l + 1] >> k);
                    tempreg = tempreg | (0xff & temp16);
                    g_amec->analytics_array[i] = tempreg;
                    g_amec->g44_avg[(j * MSA) + l] = 0;   // Reset average for this sensor to 0
                    g_amec->g44_avg[(j * MSA) + l + 1] = 0; // Reset average for this sensor to 0

                    l = l + 2;
                }

                // Final processing for Group 44: determine if cycling through all
                // chips or just monitoring one chip
                if (g_amec->analytics_option == 0)
                {
                    g_amec->analytics_chip++;

                    if (g_amec->analytics_chip >= g_amec->analytics_total_chips)
                    {
                        g_amec->analytics_chip = 0;  // loop back to chip 0 again
                    }
                }

            }
            break;

        default:
            break;
    }
}

/*----------------------------------------------------------------------------*/
/* End                                                                        */
/*----------------------------------------------------------------------------*/
