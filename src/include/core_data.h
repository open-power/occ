/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: chips/p9/procedures/lib/pm/core_data.h $                      */
/*                                                                        */
/* IBM CONFIDENTIAL                                                       */
/*                                                                        */
/* EKB Project                                                            */
/*                                                                        */
/* COPYRIGHT 2015                                                         */
/* [+] International Business Machines Corp.                              */
/*                                                                        */
/*                                                                        */
/* The source code for this program is not published or otherwise         */
/* divested of its trade secrets, irrespective of what has been           */
/* deposited with the U.S. Copyright Office.                              */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */

/// \file core_data.h
/// \brief Data structures for the GPE programs that collect raw data defined
/// in core_data.C.  The data structure layouts are also documented in the
/// spreadsheet \todo (RTC 137031) location.
///
//  *HWP HWP Owner: Doug Gilbert <dgilbert@us.ibm.com>
//  *HWP FW Owner: Martha Broyles <mbroyles@us.ibm.com>
//  *HWP Team: PM
//  *HWP Level: 1
//  *HWP Consumed by: OCC


#ifndef __GPE_CORE_DATA_H__
#define __GPE_CORE_DATA_H__

#include <stdint.h>

#define THERM_DTS_RESULT    0x00050000


typedef union dts_sensor_result_reg
{
    uint64_t value;
    struct
    {
        uint16_t  reading[2];
        uint16_t  unused_hw2;
        uint16_t  unused_hw3;
    } half_words;
} dts_sensor_result_reg_t;


typedef struct   //40 bytes
{
    uint32_t unused;
    uint32_t tod_2mhz;
    uint32_t dispatch;
    uint32_t completion;
    uint32_t freq_sens_busy;
    uint32_t freq_sens_finish;
    uint32_t run_cycles;
    uint32_t raw_cycles;
    uint32_t mem_a;         // not used in P8
    uint32_t mem_b;         // not used in P8
} CoreDataEmpath;


// \todo (RTC 137031) :  should seriously question the need for this
typedef struct          //24 bytes
{
    uint32_t raw_cycles;
    uint32_t tod_2mhz;
    uint32_t count[4];          // research prototype use
} CoreDataPerPartitionMemory;

typedef struct
{
    uint32_t raw_cycles;
    uint32_t tod_2mhz;
    uint32_t ifu_throttle;
    uint32_t isu_throttle;
    uint32_t ifu_active;
    uint32_t undefined;
} CoreDataThrottle;         // not used in P8

typedef struct
{
    uint32_t raw_cycles;    // used in P8
    uint32_t tod_2mhz;
    uint32_t run_cycles;    // used in P8
    uint32_t completion;
    uint32_t mem_a;
    uint32_t mem_b;
} CoreDataPerThread;

typedef union sensor_result
{
    uint16_t result;
    struct
    {
        uint16_t reading : 12;
        uint16_t thermal_trip : 2;
        uint16_t spare : 1;
        uint16_t valid : 1;
    } fields;

} sensor_result_t;

typedef struct
{
    uint32_t unused;
    uint32_t tod_2mhz;
    sensor_result_t core[2];
    sensor_result_t cache;
    sensor_result_t reserved;
} CoreDataDts;


//
// The instance of this data object must be 8 byte aligned
//
typedef struct
{
    CoreDataEmpath             empath;                  //40
    CoreDataPerPartitionMemory per_partition_memory;    //24
    CoreDataThrottle           throttle;                //24
    CoreDataPerThread          per_thread[8];           //24 * 8
    CoreDataDts                dts;                     //16
} CoreData;

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Get core data
 * @param[in] The system core number [0-23]
 * @param[out] Data pointer for the result
 * @return result of scom operation
 */
uint32_t  get_core_data(uint32_t i_core, CoreData* o_data);

#ifdef __cplusplus
};
#endif
#endif  /* __GPE_CORE_DATA_H__ */

