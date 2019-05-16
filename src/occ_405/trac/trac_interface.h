/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/trac/trac_interface.h $                           */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2019                        */
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

#ifndef _TRAC_INTERFACE_H
#define _TRAC_INTERFACE_H


//*************************************************************************
// Includes
//*************************************************************************
#include "ssx.h"
#include <occ_common.h>

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Macros
//*************************************************************************
/* Used to trace 0 - 5 arguments or a binary buffer when using a hash value. */


#define TRACE(i_td,i_string,args...)                  \
                trace_adal_write_all(i_td,trace_adal_hash(i_string,-1),__LINE__,0,##args)

#ifndef NO_TRAC_STRINGS

#define FIELD(a) \
        printf("%s",a)

#define FIELD1(a,b) \
        printf("%s%lx",a,(unsigned long)b)

#else  // NO_TRAC_STRINGS

#define FIELD(a)

#define FIELD1(a,b)

#endif  // NO_TRAC_STRINGS

#define SUCCESS 0


//*************************************************************************
// Defines/Enums
//*************************************************************************
#define     TRACE_MAX_ARGS      5        /* Maximum number of args to trace */

typedef uint32_t trace_hash_val;

// NOTE!  Increment this when new components are added!
#define TRAC_NUM_TRACE_COMPONENTS           1

#define TRACE_BUFFER_SIZE                   8192
#define NUMBER_TRACE_BUFFERS                3
#define ALL_TRACE_BUFFERS_SZ                (TRACE_BUFFER_SIZE * NUMBER_TRACE_BUFFERS)

#define CIRCULAR_BUFFER_SIZE                4

// These are indicies into g_des_array
#define INF_TRACE_DESCRIPTOR 0
#define ERR_TRACE_DESCRIPTOR 1
#define IMP_TRACE_DESCRIPTOR 2

//*************************************************************************
// Structures
//*************************************************************************
/*
 * Structure is put at beginning of all trace buffers
 */
typedef struct trace_buf_head {
    UCHAR ver;         /* version of this struct (1)                      */
    UCHAR hdr_len;     /* size of this struct in bytes                    */
    UCHAR time_flg;    /* meaning of timestamp entry field                */
    UCHAR endian_flg;  /* flag for big ('B') or little ('L') endian       */
    CHAR comp[16];     /* the buffer name as specified in init call       */
    UINT32 size;       /* size of buffer, including this struct           */
    UINT32 times_wrap; /* how often the buffer wrapped                    */
    UINT32 next_free;  /* offset of the byte behind the latest entry      */
    UINT32 te_count;   /* Updated each time a trace is done               */
    UINT32 extracted;  /* Not currently used                              */
}trace_buf_head_t;

/*
 * Timestamp and thread id for each trace entry.
 */
typedef struct trace_entry_stamp {
    UINT32 tbh;        /* timestamp upper part                            */
    UINT32 tbl;        /* timestamp lower part                            */
    UINT32 tid;        /* process/thread id                               */
}trace_entry_stamp_t;

/*
 * Structure is used by adal app. layer to fill in trace info.
 */
typedef struct trace_entry_head {
    UINT16 length;     /* size of trace entry                             */
    UINT16 tag;        /* type of entry: xTRACE xDUMP, (un)packed         */
    UINT32 hash;       /* a value for the (format) string                 */
    UINT32 line;       /* source file line number of trace call           */
}trace_entry_head_t;

/*
 * Parameter traces can be all contained in one write.
 */
typedef struct trace_entire_entry {
    trace_entry_stamp_t stamp;
    trace_entry_head_t head;
    UINT32 args[TRACE_MAX_ARGS + 1];
} trace_entire_entry_t;


/*
 * Binary first writes header and time stamp.
 */
typedef struct trace_bin_entry {
    trace_entry_stamp_t stamp;
    trace_entry_head_t head;
} trace_bin_entry_t;

/*
 * Used as input to traces to get to correct buffer.
 */
typedef trace_buf_head_t * tracDesc_t;

/*
 * Structure is used to hold array of all trace descriptors
 */
typedef struct trace_descriptor_array
{
    tracDesc_t      *entry; /* Pointer to trace descriptor */
    CHAR            *comp;  /* Pointer to component name   */
    SsxSemaphore    *sem;   /* Pointer to semaphore        */
}trace_descriptor_array_t;

typedef struct circular_buf_head
{
    UINT32 head;       // pointer to head
    UINT32 tail;       // pointer to tail
    UINT32 entryCount; // nums of entry
} circular_buf_header_t;


typedef struct circular_entire_data {
    UINT32 len;
    CHAR comp[4];
    trace_entire_entry_t entry;
} circular_entire_data_t;


//*************************************************************************
// Globals
//*************************************************************************
// All TPMF component trace descriptors.
extern tracDesc_t g_trac_inf;
extern tracDesc_t g_trac_err;
extern tracDesc_t g_trac_imp;

extern const trace_descriptor_array_t g_des_array[];

//*************************************************************************
// Function Prototypes
//*************************************************************************
/*
 *  Allocate and initialize all trace buffers in memory.
 *
 *  This function will allocate memory for each of the pre-defined trace
 *  buffers, initialize the buffers with starting data, and set up the
 *  trace descriptors which each component will use to trace.
 *
 *  This function must be called first before any components try to trace!
 *
 *  return Non-zero return code on error.
 */
UINT TRAC_init_buffers(void);

/*
 *  Retrieve full trace buffer for component i_comp
 *
 *  This function assumes memory has already been allocated for
 *  the full trace buffer in o_data.
 *
 *  param i_td_ptr Trace descriptor of buffer to retrieve.
 *  param o_data Pre-allocated pointer to where data will be stored.
 *
 *  return Non-zero return code on error
 */
UINT TRAC_get_buffer(const trace_descriptor_array_t *i_td_ptr,
                     void *o_data);

/*
 *  Retrieve partial trace buffer for component i_comp
 *
 *  This function assumes memory has already been allocated for
 *  the trace buffer (size io_size).  This function will copy
 *  in up to io_size in bytes to the buffer and set io_size
 *  to the exact size that is copied in.
 *
 *  param i_td_ptr Trace descriptor of buffer to retrieve.
 *  param o_data Pre-allocated pointer to where data will be stored.
 *  param io_size Size of trace data to retrieve (input)
 *                Actual size of trace data stored (output)
 *
 *  return Non-zero return code on error
 */
UINT TRAC_get_buffer_partial(const trace_descriptor_array_t *i_td_ptr,
                    void *o_data,
                    UINT *io_size);

/*
 *  Retrieve trace descriptor for input component name
 *
 *  param i_comp Component name to retrieve trace descriptor for.
 *
 *  return Valid trace descriptor on success, NULL on failure.
 */
const trace_descriptor_array_t* TRAC_get_td(const char *i_comp);

/*
 *  Reset all trace buffers
 *
 *  return Non-zero return code on error
 */
UINT TRAC_reset_buf(void);


/*
 *  Trace input integers to trace buffer.
 *
 *  This function assumes i_td has been initialized.
 *
 *  param io_td Initialized trace descriptor pointer to buffer to trace to.
 *  param i_hash Hash value to be recorded for this trace.
 *  param i_fmt Output format
 *  param i_line Line number trace is occurring on.
 *  param i_type trace type. field or debug.
 *  param ... params that are limited to a size of 4 bytes, i.e. int, uint32_t, nnn*
 *
 *  return Non-zero return code on error.
 */
UINT trace_adal_write_all(const trace_descriptor_array_t *io_td,const trace_hash_val i_hash,
                     const char *i_fmt,const ULONG i_line, const ULONG i_type,...);


/*
 *  Trace input integers to trace buffer.
 *
 *  This function assumes i_td has been initialized.
 *
 *  param io_td Initialized trace descriptor pointer to buffer to trace to.
 *  param i_hash Hash value to be recorded for this trace.
 *  param i_line Line number trace is occurring on.
 *  param i_num_args Number of arguments to trace.
 *  param i_1 Input Parameter 1
 *  param i_2 Input Parameter 2
 *  param i_3 Input Parameter 3
 *  param i_4 Input Parameter 4
 *  param i_5 Input Parameter 5
 *
 *  return Non-zero return code on error.
 */
UINT trac_write_int(const trace_descriptor_array_t *io_td,const trace_hash_val i_hash,
                    const ULONG i_line,
                    const UINT i_num_args,
                    const ULONG i_1,const ULONG i_2,const ULONG i_3,
                    const ULONG i_4,const ULONG i_5
                   );

//*************************************************************************
// Functions
//*************************************************************************

#endif //_TRAC_INTERFACE_H
