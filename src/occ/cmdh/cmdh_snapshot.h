/**
 * @file cmdh_snapshot.h
 * @brief Header file for snapshot buffer interface.
 *
 *
*/
/**
 *      @page ChangeLogs Change Logs
 *      @section _cmdh_snapshot_h cmdh_snapshot.h
 *      @verbatim
 *
 *  Flag     Def/Fea    Userid    Date        Description
 *  -------- ---------- --------  ---------   ----------------------------------
 *  @fk004   907588     fmkassem  11/25/2013  Created
 *
 *  @endverbatim
 */
#ifndef CMDH_SNAPSHOT_H
#define CMDH_SNAPSHOT_H

//*************************************************************************
// Includes
//*************************************************************************
#include "cmdh_fsp_cmds.h"

//*************************************************************************
// Defines/Enums
//*************************************************************************

/* snapshot buffer timer */
extern SsxTimer G_snapshotTimer;


//@fk004a
/**************************************************************************
 * @struct cmdh_get_ssbuffer_req_t
 * @brief Struct used to parse get snapshot buffer request Command
 */
typedef struct __attribute__ ((packed)) 
{
    struct    cmdh_fsp_cmd_header;  // Standard TMGT
    uint8_t   version;              // Get snapshot request Version
    uint8_t   reserved;             // Reserved.
    uint8_t   requested_id;         // ID of snapshot buffer to be returned.
    uint8_t   getnewest;            // 0x01 => return newest snapshot, 0x00 => return snapshot of ID given.
}cmdh_get_snapshot_query_t;


#define CMDH_GET_SNAPSHOT_NONITE_VERSION 0
#define CMDH_GET_SNAPSHOT_ITE_VERSION 1
#define CMDH_GET_SNAPSHOT_QUERY_DATALEN sizeof(cmdh_get_snapshot_query_t) - sizeof(cmdh_fsp_cmd_header_t)
#define CMDH_SNAPSHOT_MAX 6
#define CMDH_CIMP_MAX 20
#define CMDH_SNAPSHOT_DEFAULT_CUR_INDEX 0xFF    //Default value of g_cmdh_snapshot_cur_index
#define CMDH_SNAPSHOT_MAX_INDEX (CMDH_SNAPSHOT_MAX - 1)
#define CMDH_SNAPSHOT_SYNC_VERSION 0

/**************************************************************************
 * @struct cmdh_snapshot_entry_t;
 * @brief Used by OCC to store sequenced snapshot entries. 16 
 *        bytes each
 */
typedef struct __attribute__ ((packed))
{
    uint8_t                 seq_number;  //Snapshot Sequence number; 30 second time slice.
    uint8_t                 avg_dc[4];   //Average total node DC power in milliwatts.
    uint8_t                 max_dc[4];   //Maximum total node DC power in milliwatts.
    uint8_t                 min_dc[4];   //Minimum total node DC power in milliwatts.
    uint8_t                 avg_cpu_freq[2]; //Average Node CPU Frequency in MHz.
    uint8_t                 reserved;    //reserved.
}cmdh_snapshot_cimp_entry_t;

/**************************************************************************
 * @struct cmdh_snapshot_entry_t;
 * @brief Contains all entries in a snapshot.
 */
typedef struct __attribute__ ((packed))
{
    uint8_t  current_id;  // Snapshot buffer ID number of buffer data to follow, typically this should match id in request data, unless it's not available.
    cmdh_snapshot_cimp_entry_t   cim[CMDH_CIMP_MAX]; // Data collected for cimp
}cmdh_snapshot_buffer_t;

/**************************************************************************
 * @struct cmdh_get_snapshot_resp_v0_t;
 * @brief Used by OCC to respond to "GET_SNAPSHOT_BUFFER" nonite(version 0) cmd. 323 bytes
 */
typedef struct __attribute__ ((packed))
{
    struct                  cmdh_fsp_rsp_header;
    uint8_t                 oldest_id;   // Oldest available snapshot buffer ID number
    uint8_t                 newest_id;   // Newest available snapshot buffer ID number
    cmdh_snapshot_buffer_t   snapshot;    // Snapshot.
    uint8_t                 checksum[2]; // Checksum
}cmdh_get_snapshot_resp_v0_t;

/**************************************************************************
 * @struct cmdh_snapshot_sync_resp_t;
 * @brief Used by OCC to respond to "SNAPSHOT_SYNC" cmd.
 */
typedef struct __attribute__ ((packed))
{
    struct                  cmdh_fsp_rsp_header;
    UINT8                   checksum[2];
}cmdh_snapshot_sync_resp_t;

/**************************************************************************
 * @struct cmdh_snapshot_sync_query_t;
 * @brief structure of cmd packet received from tmgt.
 */
typedef struct __attribute__ ((packed))
{
    struct                  cmdh_fsp_cmd_header;  // Standard TMGT
    UINT8                   version;
} cmdh_snapshot_sync_query_t;

/**************************************************************************
 * @struct pwr250us_over30sec_t
 * @brief structure of data that need to be maintained between snapshots.
 */
typedef struct __attribute__ ((packed))
{
    uint32_t count;	 	//Count of number of samples taken.
    uint16_t max;		//Max value of pwr250us 
    uint16_t min;		//Min value of pwr250us
    uint32_t freqaAccum;//Accumulated AvgFreq over the past 30seconds.
} pwr250us_over30sec_t;

extern pwr250us_over30sec_t g_pwr250us_over30sec;
//*************************************************************************
// Function Declarations
//*************************************************************************

errlHndl_t cmdh_get_snapshot_buffer(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                          cmdh_fsp_rsp_t * i_rsp_ptr);

errlHndl_t cmdh_snapshot_sync(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                    cmdh_fsp_rsp_t * i_rsp_ptr);

//Called by a timer that is triggered every 30seconds.
void cmdh_snapshot_callback(void * arg);


#endif
