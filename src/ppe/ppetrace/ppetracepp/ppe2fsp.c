/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/ppetrace/ppetracepp/ppe2fsp.c $                       */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2019                             */
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
#include "pk_trace.h"
#include "ppe2fsp.h"
#include "trac_interface.h"
#include <arpa/inet.h>
#include <string.h>
#include <stdint.h>

#define     TRACE_BUF_VERSION   0x01     /*!< Trace buffer version            */
#define     TRACE_FIELDTRACE    0x4654   /*!< Field Trace - "FT"              */
#define     TRACE_FIELDBIN      0x4644   /*!< Binary Field Trace - "FD"       */

#define TRAC_TIME_REAL   0 // upper 32 = seconds, lower 32 = nanoseconds
#define TRAC_TIME_50MHZ  1
#define TRAC_TIME_200MHZ 2
#define TRAC_TIME_167MHZ 3 // 166666667Hz

typedef struct
{
    trace_entry_stamp_t     stamp;
    trace_entry_head_t      head;
    union
    {
        uint8_t     data[PK_TRACE_MAX_BINARY + 1]; //add 1 byte for padding
        uint32_t    parms[PK_TRACE_MAX_PARMS];
    };
    uint32_t                size;
} largest_fsp_entry_t;

typedef struct
{
    union
    {
        uint8_t     binary_data[PK_TRACE_MAX_BINARY + 1];
        struct
        {
            uint8_t     rsvd[(PK_TRACE_MAX_BINARY + 1) - (PK_TRACE_MAX_PARMS * sizeof(uint32_t))];
            uint32_t    parms[PK_TRACE_MAX_PARMS];
        };
    };
    PkTraceEntryFooter footer;
} LargestPpeEntry;

//convert a ppe timestamp to an fsp trace timestamp
uint64_t ppe2fsp_time(uint64_t ppe_time, uint32_t hz)
{
    uint32_t    seconds;
    uint32_t    remainder;
    uint32_t    nseconds;

    //convert from ppe ticks to seconds and nanoseconds
    seconds = ppe_time / hz;
    remainder = ppe_time - (((uint64_t)seconds) * hz);
    nseconds = (((uint64_t)remainder) * 1000000000) / hz;
    return (((uint64_t)seconds) << 32) | nseconds;
}

//Writes an fsp trace entry to the fsp trace buffer
fsp_put_entry(trace_buf_head_t* tb, largest_fsp_entry_t* fte, size_t entry_size, uint32_t bytes_left)
{
    char*       buffer = ((char*)tb) + sizeof(trace_buf_head_t);
    char*       tb_start;
    char*       fte_start;
    uint32_t    copy_bytes;

    if(entry_size <= bytes_left)
    {
        tb_start = buffer + bytes_left - entry_size;
        fte_start = (char*)fte;
        copy_bytes = entry_size;
    }
    else
    {
        tb_start = buffer;
        fte_start = ((char*)fte) + (entry_size - bytes_left);
        copy_bytes = bytes_left;
    }

    memcpy(tb_start, fte_start, copy_bytes);
}


//convert a ppe trace entry to an fsp trace entry
size_t pte2fte(PkTraceBuffer* ptb,
               LargestPpeEntry* pte,
               size_t pte_size,
               largest_fsp_entry_t* fte,
               uint64_t ppe_time64)
{
    size_t              entry_size;
    PkTraceGeneric*     pte_footer = &pte->footer.generic;
    uint32_t            format;
    uint32_t            hash32;
    uint32_t            hash32_partial;
    uint32_t*           parm_start;
    uint32_t            parm_bytes;
    uint64_t            fsp_time64;

    //convert the ppe trace time to an fsp trace time
    fsp_time64 = ppe2fsp_time(ppe_time64, ntohl(ptb->hz));

    //fill in the 64 bit timestamp
    fte->stamp.tbh = htonl((uint32_t)(fsp_time64 >> 32));
    fte->stamp.tbl = htonl((uint32_t)(fsp_time64 & 0x00000000ffffffffull));

    //use the ppe instance id as the thread id.
    fte->stamp.tid = htonl((uint32_t)ntohs(ptb->instance_id));

    //merge the hash prefix and the string_id fields together for a 32 bit hash value
    hash32 = ((uint32_t)ntohs(ptb->hash_prefix)) << 16;
    hash32 |= pte_footer->string_id;
    fte->head.hash = htonl(hash32);

    //generate the 32bit hash value for a partial trace entry in case it's needed
    hash32_partial = ((uint32_t)ntohs(ptb->hash_prefix)) << 16;
    hash32_partial |= ntohs(ptb->partial_trace_hash);

    //set the line number to 1
    fte->head.line = htonl(1);

    //determine the FSP trace format
    format = PK_GET_TRACE_FORMAT(pte_footer->time_format.word32);

    if(format == PK_TRACE_FORMAT_BINARY)
    {
        fte->head.tag = htons(TRACE_FIELDBIN);
    }
    else
    {
        fte->head.tag = htons(TRACE_FIELDTRACE);
    }

    parm_start = (uint32_t*)(((char*)pte) + (sizeof(LargestPpeEntry) - pte_size));

    //fill in the parameters/binary data and size at the end
    switch(format)
    {

        case PK_TRACE_FORMAT_TINY:
            //one or 0 parameters
            entry_size = sizeof(trace_entry_stamp_t) +
                         sizeof(trace_entry_head_t) +
                         sizeof(uint32_t);
            fte->parms[0] = htonl((uint32_t)(pte_footer->parm16));
            fte->head.length = htons(sizeof(uint32_t));
            parm_bytes = 0;
            break;

        case PK_TRACE_FORMAT_BIG:

            //1 - 4 parameters
            //
            //If the trace entry data is incomplete (not all parm data
            //had been written at the time the trace was captured) then
            //we will write a trace to the fsp buffer that says
            //"PARTIAL TRACE ENTRY.  HASH_ID = %d"
            if(pte_footer->complete)
            {
                parm_bytes = pte_footer->bytes_or_parms_count * sizeof(uint32_t);
                fte->head.length = htons(parm_bytes + sizeof(uint32_t));
                entry_size = sizeof(trace_entry_stamp_t) +
                             sizeof(trace_entry_head_t) +
                             parm_bytes + sizeof(uint32_t);
            }
            else
            {
                parm_bytes = 0;
                entry_size = sizeof(trace_entry_stamp_t) +
                             sizeof(trace_entry_head_t) +
                             sizeof(uint32_t);
                fte->parms[0] = fte->head.hash; //already corrected for endianess
                fte->head.hash = htonl(hash32_partial);
                fte->head.length = htons(sizeof(uint32_t));
            }

            break;

        case PK_TRACE_FORMAT_BINARY:

            //If the trace entry data is incomplete (not all parm data
            //had been written at the time the trace was captured) then
            //we will write a trace to the fsp buffer that says
            //"PARTIAL TRACE ENTRY.  HASH_ID = %d"
            if(pte_footer->complete)
            {
                parm_bytes = pte_footer->bytes_or_parms_count;
                fte->head.length = htons((uint16_t)parm_bytes);
                entry_size = sizeof(trace_entry_stamp_t) +
                             sizeof(trace_entry_head_t) +
                             parm_bytes;

                //pad to 4 byte boundary
                entry_size = (entry_size + 3) & ~3;
            }
            else
            {
                parm_bytes = 0;
                entry_size = sizeof(trace_entry_stamp_t) +
                             sizeof(trace_entry_head_t) +
                             sizeof(uint32_t);
                fte->parms[0] = fte->head.hash;
                fte->head.hash = htonl(hash32_partial);
                fte->head.length = htons(sizeof(uint32_t));
                fte->head.tag = htons(TRACE_FIELDTRACE);
            }

            break;


        default:
            entry_size = 0;
            parm_bytes = 0;
            break;
    }

    //copy parameter bytes to the fsp entry if necessary
    if(parm_bytes)
    {
        memcpy(fte->data, parm_start, parm_bytes);
    }

    //add the entry size to the end
    if(entry_size)
    {
        uint32_t new_entry_size = entry_size + sizeof(uint32_t);
        *((uint32_t*)(((char*)fte) + entry_size)) = htonl(new_entry_size);
        entry_size = new_entry_size;
    }

    return entry_size;
}

//retrieve a ppe trace entry from a ppe trace buffer
size_t ppe_get_entry(PkTraceBuffer* tb, uint32_t offset, LargestPpeEntry* pte)
{
    uint32_t            mask = ntohs(tb->size) - 1;
    PkTraceEntryFooter* footer;
    size_t              entry_size;
    size_t              parm_size;
    char*               dest = (char*)pte;
    uint32_t            format;
    uint32_t            start_index;
    uint32_t            bytes_left;
    uint32_t            bytes_to_copy;

    //Find the footer in the circular buffer
    footer = (PkTraceEntryFooter*)(&tb->cb[(offset - sizeof(PkTraceEntryFooter)) & mask]);

    //always correct endianess for the time and string id words
    pte->footer.generic.time_format.word32 = ntohl(footer->generic.time_format.word32);
    pte->footer.generic.string_id = ntohs(footer->generic.string_id);

    //only need to byte swap the parm16 value if this is a tiny format
    pte->footer.generic.parm16 = footer->generic.parm16;

    //use footer data to determine the length of the binary data or parameters
    format = PK_GET_TRACE_FORMAT(pte->footer.generic.time_format.word32);

    switch(format)
    {
        case PK_TRACE_FORMAT_TINY:
            pte->footer.generic.parm16 = ntohs(pte->footer.generic.parm16);
            parm_size = 0;
            entry_size = sizeof(PkTraceEntryFooter);
            break;

        case PK_TRACE_FORMAT_BIG:
            parm_size = pte->footer.generic.bytes_or_parms_count * sizeof(uint32_t);
            entry_size = sizeof(PkTraceEntryFooter);
            break;

        case PK_TRACE_FORMAT_BINARY:
            parm_size = pte->footer.generic.bytes_or_parms_count;
            entry_size = sizeof(PkTraceEntryFooter);
            break;

        default:
            entry_size = 0;
            parm_size = 0;
            break;
    }

    //pad to 8 byte boundary
    parm_size = (parm_size + 7) & ~0x00000007ul;

    //add the parameter size to the total entry size
    entry_size += parm_size;

    //copy the entry from the circular buffer to pte
    start_index = (offset - entry_size) & mask;
    bytes_left = ntohs(tb->size) - start_index;

    //only copy up to the end of the circular buffer
    if(parm_size < bytes_left)
    {
        bytes_to_copy = parm_size;
    }
    else
    {
        bytes_to_copy = bytes_left;
    }

    dest += sizeof(LargestPpeEntry) - entry_size;
    memcpy(dest, &tb->cb[start_index], bytes_to_copy);

    //now copy the rest of the data starting from the beginning of the
    //circular buffer.
    if(bytes_to_copy < parm_size)
    {
        memcpy(dest + bytes_to_copy, tb->cb, parm_size - bytes_to_copy);
    }

    //return the size of the entry
    return entry_size;
}

//convert a ppe trace buffer to an fsp trace buffer
int ppe2fsp(void* in, unsigned long in_size, void* out, unsigned long* io_size)
{
    PkTraceBuffer*              ptb = (PkTraceBuffer*)in;
    trace_buf_head_t*           ftb = (trace_buf_head_t*)out;
    uint32_t                    ppe_bytes_left;
    uint32_t                    fsp_bytes_left;
    int                         rc = 0;
    uint32_t                    ptb_offset;
    PkTraceEntryFooter*         ptb_te;
    uint64_t                    ppe_time64;
    uint32_t                    fte_size, pte_size;
    uint32_t                    fsp_te_count = 0;
    uint32_t                    time_diff32, prev_time32, new_time32;
    PkTraceGeneric*             pte_footer;
    largest_fsp_entry_t         fte;
    LargestPpeEntry             pte;
    uint64_t                    time_adj64;

    do
    {
        if(!ptb || !ftb || !io_size)
        {
            rc = P2F_NULL_POINTER;
            break;
        }

        if(ntohs(ptb->version) != PK_TRACE_VERSION)
        {
            rc = P2F_INVALID_VERSION;
            break;
        }

        //check that the input buffer is large enough to have a ppe trace buffer
        if(in_size < (((uint32_t)(&ptb->cb[0])) - (uint32_t)(ptb)))
        {
            rc = P2F_INPUT_BUFFER_TOO_SMALL;
            break;
        }

        //initialize some locals
        fsp_bytes_left = *io_size - sizeof(trace_buf_head_t);
        ppe_bytes_left = ntohs(ptb->size);
        ptb_offset = ntohl(ptb->state.offset);

        if(htonl(1) == 1)
        {
            time_adj64 = ptb->time_adj64;
        }
        else
        {
            time_adj64 = ntohl((uint32_t)(ptb->time_adj64 >> 32));
            time_adj64 |= ((uint64_t)(ntohl((uint32_t)(ptb->time_adj64 &
                                            0x00000000ffffffff))))
                          << 32;
        }

        //make sure the ppe buffer size is a power of two
        if((ppe_bytes_left - 1) & ppe_bytes_left)
        {
            //size is not a power of two
            printf("size is not a power of two; ppe_bytes_left = %u\n", ppe_bytes_left);
            rc = P2F_INVALID_INPUT_SIZE;
            break;
        }

        //The ppe bytes field should always be a multiple of 8
        if(ptb_offset & 0x7)
        {
            printf("ppe bytes field should always be a multiple of 8; ptb_offset = %u\n", ptb_offset);
            rc = P2F_INVALID_PPE_OFFSET;
            break;
        }

        //make sure there is enough room for the fsp header
        if(*io_size < sizeof(trace_buf_head_t))
        {
            rc = P2F_OUTPUT_BUFFER_TOO_SMALL;
            break;
        }


        //initialize the fsp header
        ftb->ver = TRACE_BUF_VERSION;
        ftb->hdr_len = sizeof(trace_buf_head_t);
        ftb->time_flg = TRAC_TIME_REAL;
        ftb->endian_flg = 'B'; //big endian

        // Fix a problem when ptb->image_str fills the entire char array
        // and thus has no terminating NULL
        // First remove underscores then truncate if needed.
        if(strlen(ptb->image_str) > (sizeof(ftb->comp) - 1))
        {
            int p_idx = 0;
            int f_idx = 0;
            memset(ftb->comp, 0, sizeof(ftb->comp));

            for(; p_idx < sizeof(ftb->comp); ++p_idx)
            {
                char c = ptb->image_str[p_idx];

                if(c != '_')
                {
                    ftb->comp[f_idx] = c;
                    ++f_idx;

                    if(c == 0 || f_idx == sizeof(ftb->comp) - 1)
                    {
                        break;
                    }
                }
            }
        }
        else
        {
            memcpy(ftb->comp, ptb->image_str, sizeof(ftb->comp));
        }

        ftb->times_wrap = htonl(1);
        ftb->size = htonl(sizeof(trace_buf_head_t) + sizeof(uint32_t));
        ftb->next_free = htonl(sizeof(trace_buf_head_t));
        ftb->extracted = htonl(0);
        ftb->te_count = htonl(0);

        //find the latest timestamp so that we can work back from there
        ppe_time64 = ((uint64_t)(ntohl(ptb->state.tbu32) & 0xefffffff)) << 32;
        pte_size = ppe_get_entry(ptb, ptb_offset, &pte);
        prev_time32 = PK_GET_TRACE_TIME(pte.footer.generic.time_format.word32);
        ppe_time64 |= prev_time32;

        //process all of the input bytes one trace entry at a time
        //from newest to oldest (backwards) until we run out of input bytes or
        //we run out of output space.
        while(1)
        {
            //check if we have enough data for a ppe footer
            if(ppe_bytes_left < sizeof(PkTraceEntryFooter))
            {
                break;
            }

            //get the next ppe entry
            pte_size = ppe_get_entry(ptb, ptb_offset, &pte);

            //Stop if there are no more entries to retrieve from the ppe trace buffer
            if(!pte_size)
            {
                break;
            }

            pte_footer = &pte.footer.generic;

            //mark the entry as incomplete if we didn't have enough data
            //for the entire entry
            if(pte_size > ppe_bytes_left)
            {
                pte_footer->complete = 0;
                ppe_bytes_left = 0;
            }
            else
            {
                ppe_bytes_left -= pte_size;
                ptb_offset -= pte_size;
            }

            //Calculate the 64 bit timestamp for this entry....
            //On PPE, getting the timestamp is not done atomically with writing
            //the entry to the buffer.  This means that an entry with an older
            //timestamp could possibly be added to the buffer after an entry
            //with a newer timestamp.  Detect this condition by checking if the
            //time difference is bigger than the max difference.  The max
            //difference is enforced by the PPE having a trace added on a
            //shorter time boundary (using a timer).
            new_time32 = PK_GET_TRACE_TIME(pte_footer->time_format.word32);
            time_diff32 = prev_time32 - new_time32;

            if(time_diff32 > ntohl(ptb->max_time_change))
            {
                time_diff32 = new_time32 - prev_time32;
                ppe_time64 += time_diff32;
            }
            else
            {
                ppe_time64 -= time_diff32;
            }

            //save off the lower 32bit timestamp for the next iteration
            prev_time32 = new_time32;

            //convert the ppe trace entry to an fsp trace entry
            fte_size = pte2fte(ptb, &pte, pte_size, &fte, ppe_time64 + time_adj64);

            //fit as much of the entry into the fsp trace buffer as possible
            fsp_put_entry(ftb, &fte, fte_size, fsp_bytes_left);

            //update the fsp trace entry count
            fsp_te_count++;

            //stop if there is no more room left in the fsp trace buffer
            if(fte_size >= fsp_bytes_left)
            {
                fsp_bytes_left = 0;
                ftb->times_wrap = htonl(1);
                break;
            }
            else
            {
                fsp_bytes_left -= fte_size;
            }
        }//while(1)


        //shift the trace data up if there is space to do so
        if(fsp_bytes_left)
        {
            char* dest = ((char*)ftb) + sizeof(trace_buf_head_t);
            char* src = dest + fsp_bytes_left;
            size_t data_size = *io_size - sizeof(trace_buf_head_t) - fsp_bytes_left;
            memmove(dest, src, data_size);
        }

        //update the fsp header to reflect the true size and entry count
        ftb->te_count = htonl(fsp_te_count);

        //inform the caller of how many bytes were actually used
        *io_size -= fsp_bytes_left;

        //shrink the size field to what we actually ended up using
        ftb->size = htonl(*io_size);

    }
    while(0);

    return rc;
}



