/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: chips/p9/procedures/lib/pm/core_data.h $                      */
/*                                                                        */
/* IBM CONFIDENTIAL                                                       */
/*                                                                        */
/* EKB Project                                                            */
/*                                                                        */
/* COPYRIGHT 2015,2016                                                    */
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
#define PC_OCC_SPRC         0x00010A82
#define PC_OCC_SPRD         0x00010A83
#define TOD_VALUE_REG       0x00040020

#define CORE_RAW_CYCLES             0x200
#define CORE_RUN_CYCLES             0x208
#define CORE_WORKRATE_BUSY          0x210
#define CORE_WORKRATE_FINISH        0x218
#define CORE_MEM_HIER_A_LATENCY     0x220
#define CORE_MEM_HIER_B_LATENCY     0x228
#define CORE_MEM_HIER_C_ACCESS      0x230
#define THREAD0_RUN_CYCLES          0x238
#define THREAD0_INST_DISP_UTIL      0x240
#define THREAD0_INST_COMP_UTIL      0x248
#define THREAD0_MEM_HIER_C_ACCESS   0x250
#define THREAD1_RUN_CYCLES          0x258
#define THREAD1_INST_DISP_UTIL      0x260
#define THREAD1_INST_COMP_UTIL      0x268
#define THREAD1_MEM_HEIR_C_ACCESS   0x270
#define THREAD2_RUN_CYCLES          0x278
#define THREAD2_INST_DISP_UTIL      0x280
#define THREAD2_INST_COMP_UTIL      0x288
#define THREAD2_MEM_HEIR_C_ACCESS   0x290
#define THREAD3_RUN_CYCLES          0x298
#define THREAD3_INST_DISP_UTIL      0x2A0
#define THREAD3_INST_COMP_UTIL      0x2A8
#define THREAD3_MEM_HEIR_C_ACCESS   0x2B0
#define IFU_THROTTLE_BLOCK_FETCH    0x2B8
#define IFU_THROTTLE_ACTIVE         0x2C0
#define VOLT_DROOP_THROTTLE_ACTIVE  0x2C8

#define EMPATH_CORE_THREADS 4

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


typedef struct
{
    uint32_t tod_2mhz;
    uint32_t raw_cycles;       // 0x200
    uint32_t run_cycles;       // 0x208
    uint32_t freq_sens_busy;   // 0x210 Core workrate busy counter
    uint32_t freq_sens_finish; // 0x218 Core workrate finish counter
    uint32_t mem_latency_a;
    uint32_t mem_latency_b;
    uint32_t mem_access_c;

} CoreDataEmpath;

typedef struct
{
    uint32_t ifu_throttle;
    //uint32_t isu_throttle;  // No longer exists
    uint32_t ifu_active;
    uint32_t undefined;
    uint32_t v_droop;         // new for p9
} CoreDataThrottle;

typedef struct
{
    uint32_t run_cycles;
    uint32_t dispatch;      //  new for p9
    uint32_t completion;
    uint32_t mem_c;         // was mem_a
    //    uint32_t mem_b;       // No longer exists in p9
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
    sensor_result_t core[2];
    sensor_result_t cache;
    sensor_result_t reserved;
} CoreDataDts;


//
// The instance of this data object must be 8 byte aligned
//
typedef struct // 120
{
    CoreDataEmpath             empath;    //32
    CoreDataThrottle           throttle;  //16
    CoreDataPerThread          per_thread[EMPATH_CORE_THREADS]; // 64
    CoreDataDts                dts;  //8
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
