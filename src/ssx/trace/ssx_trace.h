/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/trace/ssx_trace.h $                                   */
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
#ifndef __SSX_TRACE_H__
#define __SSX_TRACE_H__
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2014
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file ssx_trace.h
/// \brief Macros and declarations for the SSX Firmware Tracing Facility.
/// 

#include <stdint.h>

#define SSX_TRACE_VERSION 2

#ifndef SSX_TRACE_SZ
#define SSX_TRACE_SZ 256
#endif

//Fail compilation if size is not a power of 2
#if ((SSX_TRACE_SZ - 1) & SSX_TRACE_SZ)
#error "SSX_TRACE_SZ is not a power of two!!!"
#endif

//Fail compilation if size is smaller than 64 bytes
#if (SSX_TRACE_SZ < 64)
#error "SSX_TRACE_SZ must be at least 64 bytes!!!"
#endif

//Mask for calculating offsets into the trace circular buffer
#define SSX_TRACE_CB_MASK (SSX_TRACE_SZ - 1)

#define STRINGIFY_HELPER(x) #x
#define STRINGIFY(x) STRINGIFY_HELPER(x)

#define PPC_IMG_STRING STRINGIFY(IMAGE_NAME)

#ifdef SSX_TRACE_HASH_PREFIX
#if (SSX_TRACE_HASH_PREFIX > 0xffff)
#error SSX_TRACE_HASH_PREFIX must be defined as a 16 bit constant value
#endif
#endif //SSX_TRACE_HASH_PREFIX

//This provides a 128ns tick (assuming a 32ns clock period)
//and 4 different format values
#define SSX_TRACE_TS_BITS 30

#define SSX_TRACE_FORMAT_BITS (32 - SSX_TRACE_TS_BITS)

#define SSX_TRACE_TS_MASK (0xfffffffful << SSX_TRACE_FORMAT_BITS)
#define SSX_TRACE_FORMAT_MASK (~SSX_TRACE_TS_MASK)

#define SSX_GET_TRACE_FORMAT(w32) (SSX_TRACE_FORMAT_MASK & w32)
#define SSX_GET_TRACE_TIME(w32) (SSX_TRACE_TS_MASK & w32)

//Set the trace timer period to be the maximum
//32 bit time minus 2 seconds (assuming a 32ns tick)
//This allows for up to 1 second of interrupt latency +
//1 second for SSX_TRACE_MTBT while only requiring a trace
//every 135 seconds in order to maintain the 64bit timebase.
#define SSX_TRACE_TIMER_PERIOD (0xfffffffful - 62500000)

//The Maximum Time Between Traces. In order to reduce the time that interrupts
//are disabled for tracing, reading of the time stamp is not done atomically
//with alocating an entry in the circular buffer.  This means that the
//timestamps might not appear in order in the trace buffer. This is a
//problem because our calculation of the 64 bit timebase uses the unsigned
//difference of the current 32bit timestamp and the previous one and if they
//are out of order it will result in a very large difference.  To solve this
//problem, any time that the parser code sees a very large difference (larger
//than SSX_TRACE_MTBT) it will treat it as a negative number.
#define SSX_TRACE_MTBT (0xfffffffful - 31250000) 

#define SSX_TRACE_MAX_PARMS  4

//This is the maximum number of bytes allowed to be traced in a binary trace
//entry.
//The trace version needs to change if this changes.
#define SSX_TRACE_MAX_BINARY 256

//clip the largest binary trace according to the trace buffer size.
//(The trace version does not need to change if this changes as long
// as it remains less than SSX_TRACE_MAX_BINARY)
#if (SSX_TRACE_SZ <= 256)
#define SSX_TRACE_CLIPPED_BINARY_SZ SSX_TRACE_SZ / 2
#else
#define SSX_TRACE_CLIPPED_BINARY_SZ SSX_TRACE_MAX_BINARY
#endif

//Trace formats that are supported
typedef enum
{
    SSX_TRACE_FORMAT_EMPTY,
    SSX_TRACE_FORMAT_TINY,
    SSX_TRACE_FORMAT_BIG,
    SSX_TRACE_FORMAT_BINARY,
}SsxTraceFormat;

//This combines the timestamp and the format bits into a
//single 32 bit word.
typedef union
{
    struct
    {
        uint32_t    timestamp   : SSX_TRACE_TS_BITS;
        uint32_t    format      : SSX_TRACE_FORMAT_BITS;
    };
    uint32_t word32;
}SsxTraceTime;

//SSX trace uses a 16 bit string format hash value
typedef uint16_t SsxTraceHash;

//The constant 16 bit hash value is combined with a
//16 bit parameter value when doing a tiny trace
typedef union
{
    struct
    {
        SsxTraceHash        string_id;
        uint16_t            parm;
    };
    uint32_t    word32;
}SsxTraceTinyParms;

//A tiny trace fits within a single 8 byte word. This includes
//the timestamp, format bits, hash id, and a 16 bit parameter.
typedef union
{
    struct
    {
        SsxTraceTinyParms   parms;
        SsxTraceTime        time_format; 
    };
    uint64_t    word64;
}SsxTraceTiny;

//Larger traces that require a 32 bit parameter or more than one
//parameter use the big trace format.  The number of parms and
//the 'complete' flag are combined with the hash id. 'complete'
//is set to 0 initially and set to one only after all of the trace
//data has been written.
typedef union
{
    struct
    {
        SsxTraceHash        string_id;
        uint8_t             complete;
        uint8_t             num_parms;
    };
    uint32_t    word32;
}SsxTraceBigParms;

typedef union
{
    struct
    {
        SsxTraceBigParms    parms;
        SsxTraceTime        time_format; 
    };
    uint64_t    word64;
}SsxTraceBig;

//Binary traces are handled in a similar fashion to big traces, except
//that instead of having a number of parameters, we have number of bytes.
typedef union
{
    struct
    {
        SsxTraceHash        string_id;
        uint8_t             complete;
        uint8_t             num_bytes;
    };
    uint32_t    word32;
}SsxTraceBinaryParms;

typedef union
{
    struct
    {
        SsxTraceBinaryParms  parms;
        SsxTraceTime         time_format;
    };
    uint64_t    word64;
}SsxTraceBinary;

//This is a generic structure that can be used to retrieve data
//for tiny, big, and binary formatted entries.
typedef union
{
    struct
    {
        SsxTraceHash        string_id;
        union
        {
            uint16_t        parm16;
            struct
            {
                uint8_t     complete;
                uint8_t     bytes_or_parms_count;
            };
        };
        SsxTraceTime        time_format;
    };
    uint64_t    word64;
}SsxTraceGeneric;

//This is a format that might be used in the future for tracing
//a 64 bit timestamp so that we don't fill up the buffer with periodic
//timer traces.  It is not currently used.
#if 0
typedef union
{
    struct
    {
        uint32_t            upper32;
        SsxTraceTime        time_format;
    };
    uint64_t    word64;
}SsxTraceTime64;
#endif

//It would probably be more accurate to call this a footer since it
//actually resides at the highest address of each trace entry. These eight
//bytes contain information that allow us to walk the trace buffer from the
//most recent entry to the oldest entry.
typedef union
{
    SsxTraceGeneric      generic;
    SsxTraceBinary       binary;
    SsxTraceBig          big;
    SsxTraceTiny         small;
}SsxTraceEntryFooter;


//This is the data that is updated (in the buffer header) every time we add
//a new entry to the buffer.
typedef union
{
    struct
    {
        uint32_t  tbu32;
        uint32_t  offset;
    };
    uint64_t word64;
}SsxTraceState;

#define SSX_TRACE_IMG_STR_SZ 16

//Header data for the trace buffer that is used for parsing the data.
//Note: SsxTraceState contains a uint64_t which is required to be
//placed on an 8-byte boundary according to the EABI Spec.  This also
//causes cb to start on an 8-byte boundary.
typedef struct
{
    //these values are needed by the parser
    uint16_t            version;
    uint16_t            rsvd;
    char                image_str[SSX_TRACE_IMG_STR_SZ];
    uint16_t            instance_id;
    uint16_t            partial_trace_hash;
    uint16_t            hash_prefix;
    uint16_t            size;
    uint32_t            max_time_change;
    uint32_t            hz;
    uint32_t            pad;
    uint64_t            time_adj64;

    //updated with each new trace entry
    SsxTraceState       state;

    //circular trace buffer
    uint8_t             cb[SSX_TRACE_SZ];
}SsxTraceBuffer;

extern SsxTraceBuffer g_ssx_trace_buf;

#endif /* __SSX_TRACE_H__ */
