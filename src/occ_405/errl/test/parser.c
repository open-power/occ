/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/errl/test/parser.c $                              */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2018                        */
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

#include <stdio.h>
#include <stdint.h>
#include <string.h>

typedef uint32_t      UINT32;
typedef int32_t       INT32;
typedef uint8_t       UCHAR;
typedef uint8_t       UINT8;
typedef int8_t        INT8;
typedef uint16_t      UINT16;
typedef int16_t       INT16;
typedef char          CHAR;
typedef unsigned int  UINT;
typedef unsigned long ULONG;
typedef int           INT;
typedef void          VOID;
typedef uint32_t      size_t ;


// Max size of non call home data logs (2048 bytes)
#define MAX_ERRL_ENTRY_SZ 0x800

// Max size of call home data log (3072 bytes)
#define MAX_ERRL_CALL_HOME_SZ 0xC00

// Max size of callouts
#define ERRL_MAX_CALLOUTS 6

// Max number of error logs
#define ERRL_MAX_SLOTS 9

// Used to default a old/bad error handle
#define INVALID_ERR_HNDL (errlHndl_t)INVALID_ERR

// These bits are used to acquire a slot number.  When used with the global
// slot bit mask, we are able to get 7 slots for predictive/unrecoverable errors,
// 1 slot for informational logs, and 1 slot for call home data log
/* Slot Masks */
typedef enum
{
    ERRL_SLOT_MASK_DEFAULT        = 0xFFFFFFFF,
    ERRL_SLOT_MASK_INFORMATIONAL  = 0xFEFFFFFF,
    ERRL_SLOT_MASK_PREDICTIVE     = 0x01FFFFFF,
    ERRL_SLOT_MASK_UNRECOVERABLE  = 0x01FFFFFF,
    ERRL_SLOT_MASK_CALL_HOME_DATA = 0xFF7FFFFF,
} ERRL_SLOT_MASK;

// These are the possible severities that an error log can have.
// Users must ONLY use these enum values for severity.
/* Error Severity */
typedef enum
{
    ERRL_SEV_INFORMATIONAL  = 0x00,
    ERRL_SEV_PREDICTIVE     = 0x01,
    ERRL_SEV_UNRECOVERABLE  = 0x02,
    ERRL_SEV_CALLHOME_DATA  = 0x03,
} ERRL_SEVERITY;

// These are the possible callout priorities that a callout can have.
// Users must ONLY use these enum values for callout priority
/* Callout Priority */
typedef enum
{
    ERRL_CALLOUT_PRIORITY_INVALID   = 0x00,
    ERRL_CALLOUT_PRIORITY_LOW       = 0x01,
    ERRL_CALLOUT_PRIORITY_MED       = 0x02,
    ERRL_CALLOUT_PRIORITY_HIGH      = 0x03,
} ERRL_CALLOUT_PRIORITY;

// These are the user detail types that a user details can have.
// Users must ONLY use these enum values for user detail type
/* User Detail Type */
typedef enum
{
    ERRL_USR_DTL_TRACE_DATA     = 0x01,
    ERRL_USR_DTL_CALLHOME_DATA  = 0x02,
} ERRL_USR_DETAIL_TYPE;

// These are the possible OCC States.
/* OCC States */
typedef enum
{
    ERRL_OCC_STATE_INVALID          = 0xFF,
} ERRL_OCC_STATE;

//Versions
/* Errl Structure Version */
typedef enum
{
    ERRL_STRUCT_VERSION_1       = 0x01,
} ERRL_STRUCT_VERSION;

/* Errl User Details Version */
typedef enum
{
    ERRL_USR_DTL_STRUCT_VERSION_1       = 0x01,
} ERRL_USR_DTL_STRUCT_VERSION;


/* Errl Trace Version */
typedef enum
{
    ERRL_TRACE_VERSION_1       = 0x00,
} ERRL_TRACE_VERSION;

/* Callout Structure */
struct ErrlCallout
{
    uint64_t    iv_homUnitId;   // HOM Unit ID
    uint8_t     iv_priority;    // Callout Priority
                                // NOTE: Users must use ERRL_CALLOUT_PRIORITY enum
    uint8_t     iv_reserved1;   // Reserved 1
    uint16_t    iv_reserved2;   // Reserved 2

} __attribute__ ((__packed__));

typedef struct ErrlCallout ErrlCallout_t;

// The User Detail Entry Structure consists of the fields below followed
// by the actual data the user is trying to collect.
// NOTE: A data pointer field is NOT defined but rather inferred here.  In the
//       error log contents, the user will see all the subsequent fields followed
//       by the actual data
/* User Detail Entry Structure */
struct ErrlUserDetailsEntry
{
    uint8_t     iv_version; // User Details Entry Version
    uint8_t     iv_type;    // User Details Entry Type
                            // Note: Users must use ERRL_USR_DETAIL_TYPE enum
    uint16_t    iv_size;    // User Details Entry Size
} __attribute__ ((__packed__));

typedef struct ErrlUserDetailsEntry ErrlUserDetailsEntry_t;

// The User Detail Structure consists of the fields below followed
// by each individual User Details Entry structure & data
// NOTE: A data pointer field is NOT defined but rather inferred here.  In the
//       error log contents, the user will see all the subsequent fields followed
//       by each User Details Entry structure and its data
/* User Detail Structure */
struct ErrlUserDetails
{
    uint8_t     iv_version;             // User Details Version
    uint8_t     iv_reserved;            // Reserved
    uint16_t    iv_modId;               // Module Id
    uint32_t    iv_fclipHistory;        // Frequency Clip History
    uint64_t    iv_timeStamp;           // Time Stamp
    uint8_t     iv_occId;               // OCC ID
    uint8_t     iv_occRole;             // OCC Role
    uint8_t     iv_operatingState;      // OCC State
    uint8_t     iv_committed:1;         // Log Committed?
    uint8_t     iv_reserved1:7;
    uint32_t    iv_userData1;           // User Data Word 1
    uint32_t    iv_userData2;           // User Data Word 2
    uint32_t    iv_userData3;           // User Data Word 3
    uint16_t    iv_reserved2;
    uint16_t    iv_extendedRC;          // Extended RC
    uint16_t    iv_entrySize;           // Log Size
    uint16_t    iv_userDetailEntrySize; // User Details Size
} __attribute__ ((__packed__));

typedef struct ErrlUserDetails ErrlUserDetails_t;

/* Error Log Structure */
struct ErrlEntry
{
    uint16_t            iv_checkSum;    // Log CheckSum
    uint8_t             iv_version;     // Log Version
    uint8_t             iv_entryId;     // Log Entry ID
    uint8_t             iv_reasonCode;  // Log Reason Code
    uint8_t             iv_severity;    // Log Severity
                                        // NOTE: Users must use ERRL_SEVERITY enum
    uint8_t             iv_reserved1;   // Reserved

    uint8_t             iv_numCallouts; // Log Callout Number
    ErrlCallout_t       iv_callouts[ERRL_MAX_CALLOUTS];
    ErrlUserDetails_t   iv_userDetails; // User Details section for Log

} __attribute__ ((__packed__));

typedef struct ErrlEntry ErrlEntry_t;

// Function Specification
//
// Name: main
//
// Description: main function
//
// End Function Specification
main()
{
    FILE *      l_fptr = fopen( "occ.log.bin", "r");
    uint32_t    l_sizeRead = 0;
    uint32_t     l_readBuf[3072] = {0};

    if ( l_fptr )
    {
        fseek( l_fptr, 0, SEEK_END);
        uint32_t l_len = ftell( l_fptr );
        fseek( l_fptr, 0, SEEK_SET);

        l_sizeRead = fread( l_readBuf, 1, l_len, l_fptr );

        printf("File read contains %d bytes read %d \n", l_len, l_sizeRead );
    }

    fclose( l_fptr );

    ErrlEntry_t l_log;

    memcpy( &l_log, l_readBuf, l_sizeRead );

    printf(" CheckSum   : 0x%04X \n", ntohs(l_log.iv_checkSum) );
    printf(" Version    : 0x%02X \n", l_log.iv_version);
    printf(" Entry Id   : 0x%02X \n", l_log.iv_entryId);
    printf(" Reason Code: 0x%02X \n", l_log.iv_reasonCode);
    printf(" Severity   : 0x%02X \n", l_log.iv_severity);
    printf(" Callouts   : 0x%02X \n", l_log.iv_numCallouts);
    uint8_t l_index=0;
    for( l_index=0; l_index < ERRL_MAX_CALLOUTS; l_index++)
    {
        printf(" Callout%d  : \n", l_index+1);
        printf(" HOM UID    : 0x%08X \n", ntohl(l_log.iv_callouts[l_index].iv_homUnitId));
        printf(" Priority   : 0x%02X \n", l_log.iv_callouts[l_index].iv_priority );
    }
    printf(" UserDetails    : \n" );
    printf(" Version        : 0x%02X \n", l_log.iv_userDetails.iv_version );
    printf(" Module Id      : 0x%04X \n", ntohs(l_log.iv_userDetails.iv_modId) );
    printf(" FClip History  : 0x%08X \n", l_log.iv_userDetails.iv_fclipHistory );
    printf(" Time           : 0x%08X \n", l_log.iv_userDetails.iv_timeStamp );
    printf(" OCC ID         : 0x%02X \n", l_log.iv_userDetails.iv_occId );
    printf(" OCC Role       : 0x%02X \n", l_log.iv_userDetails.iv_occRole );
    printf(" OCC State      : 0x%02X \n", l_log.iv_userDetails.iv_operatingState );
    printf(" Committed      : %s \n", (l_log.iv_userDetails.iv_committed != 0) ? "true":"false" );
    printf(" Word 1         : 0x%08X \n", ntohl(l_log.iv_userDetails.iv_userData1) );
    printf(" Word 2         : 0x%08X \n", ntohl(l_log.iv_userDetails.iv_userData2) );
    printf(" Word 3         : 0x%08X \n", ntohl(l_log.iv_userDetails.iv_userData3) );
    printf(" Word 4         : 0x%08X \n", ntohl(l_log.iv_userDetails.iv_userData4) );
    printf(" Entry Size     : 0x%04X \n", ntohs(l_log.iv_userDetails.iv_entrySize) );


    return 0;
}
