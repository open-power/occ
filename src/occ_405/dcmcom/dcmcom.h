#ifndef _DCMCOM_H
#define DCMCOM_H

#include "rtls.h"

#define ICC_SEQ_UNCHANGED_MAX_COUNT 5

typedef union icc_msg
{
    uint64_t value;
    struct
    {
        uint32_t high_word;
        uint32_t low_word;
    };

    struct
    {
        uint8_t seq;
        uint8_t flags;
        uint16_t average_freq; // WOF average frequency
        uint8_t  clip_pstate;
        uint8_t  proc_temperature; // current proc temp.
        uint16_t reserved;
    };
} icc_msg_t;

extern icc_msg_t g_current_icc_msg;

void dcmicc_update( void );


#endif
