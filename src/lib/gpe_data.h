#ifndef __GPE_DATA_H__
#define __GPE_DATA_H__

// $Id: gpe_data.h,v 820.1 2014/08/22 16:33:56 daviddu Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/fw820/procedures/lib/gpe_data.h,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file gpe_data.h
/// \brief Data structures for the GPE programs that collect raw data defined
/// in gpe_data.S.  The data structure layouts are also documented in the
/// spreadsheet "Pgp Procedures.ods" in lib/doc.
///
/// \todo Add SPURR Fraction update as an option

#include "ssx.h"
#include "gpe.h"
#include "pgp_config.h"

////////////////////////////////////////////////////////////////////////////
// gpe_get_core_data()
////////////////////////////////////////////////////////////////////////////

#ifndef __ASSEMBLER__

/// Paramaters for gpe_get_core_data() & gpe_get_per_core_data()

typedef struct {

    /// gpe_get_core_data() and gpe_get_per_core_data() only collects data for
    /// core chiplets configured in this mask.
    ChipConfig config;

    /// This mask, comprised of a logical OR of the GPE_GET_CORE_DATA_*
    /// macros, controls which data groups are collected.
    uint64_t select;

    /// This is the 32-bit pointer (cast to a uint64_t) to the core chiplet
    /// raw data area to be filled by this invocation of gpe_get_core_data().
    ///
    /// For gpe_get_core_data() (used by the lab thread coreData) this is a
    /// pointer to an array of CoreData structures, with one structure
    /// allocated for each possible core supported by the architecture.
    ///
    /// For gpe_get_per_core_data() (used by OCC FW) this is a pointer to a
    /// single CoreData structure to be filled in by the routine.
    uint64_t data;

} GpeGetCoreDataParms;

// Get data for all cores, placing data at an index into 
// GpeGetCoreDataParms->data pointer, depending on the core which got data.
PoreEntryPoint gpe_get_core_data;      

// Get data for first core in GpeGetCoreDataParms->config, placing data 
// directly into GpeGetCoreDataParms->data pointer
PoreEntryPoint gpe_get_per_core_data;

#endif  /* __ASSEMBLER__ */

// Parameter offsets for gpe_get_core_data()

#define GPEGETCOREDATAPARMS_CONFIG      0x00
#define GPEGETCOREDATAPARMS_SELECT      0x08
#define GPEGETCOREDATAPARMS_DATA        0x10

// Data group select masks for gpe_get_core_data()

#define GPE_GET_CORE_DATA_EMPATH            0x0001
#define GPE_GET_CORE_DATA_MEMORY            0x0002
#define GPE_GET_CORE_DATA_THROTTLE          0x0004
#define GPE_GET_CORE_DATA_THREAD            0x0008
#define GPE_GET_CORE_DATA_DTS_CPM           0x0010
#define GPE_GET_CORE_DATA_PCB_SLAVE         0x0020

#define GPE_GET_CORE_DATA_ALL 0x003f

// Per-core data area offsets

#define CORE_DATA_EMPATH_BASE 0
#define CORE_DATA_EMPATH_SIZE 40

#define CORE_DATA_MEMORY_BASE                           \
    (CORE_DATA_EMPATH_BASE + CORE_DATA_EMPATH_SIZE)
#define CORE_DATA_MEMORY_SIZE 24

#define CORE_DATA_THROTTLE_BASE \
    (CORE_DATA_MEMORY_BASE + CORE_DATA_MEMORY_SIZE)
#define CORE_DATA_THROTTLE_SIZE 24

#define CORE_DATA_THREAD_BASE(t)                                        \
    (CORE_DATA_THROTTLE_BASE + CORE_DATA_THROTTLE_SIZE + (24 * (t)))
#define CORE_DATA_THREAD_SIZE (24 * 8)

#define CORE_DATA_DTS_CPM_BASE                          \
    (CORE_DATA_THREAD_BASE(0) + CORE_DATA_THREAD_SIZE)
#define CORE_DATA_DTS_CPM_SIZE 40

#define CORE_DATA_PCB_SLAVE_BASE                        \
    (CORE_DATA_DTS_CPM_BASE + CORE_DATA_DTS_CPM_SIZE)
#define CORE_DATA_PCB_SLAVE_SIZE 32

#define CORE_DATA_OHA_BASE \
    (CORE_DATA_PCB_SLAVE_BASE + CORE_DATA_PCB_SLAVE_SIZE)
#define CORE_DATA_OHA_SIZE 8

#define CORE_DATA_SIZE (CORE_DATA_OHA_BASE + CORE_DATA_OHA_SIZE)

// Data area components.  Each data group is marked with the TOD captured just
// before each data group capture.  Data groups that may have some relation to
// frequency are also tagged with the current raw cycles reading.  The offsets
// are _byte_ offsets into a byte array.  The user needs to be aware of
// whether each datum represents a 32- or 64-bit integer.

#define CORE_DATA_EMPATH_UNUSED        (CORE_DATA_EMPATH_BASE + 0x00)
#define CORE_DATA_EMPATH_TOD           (CORE_DATA_EMPATH_BASE + 0x04)
#define CORE_DATA_DISPATCH             (CORE_DATA_EMPATH_BASE + 0x08)
#define CORE_DATA_COMPLETION           (CORE_DATA_EMPATH_BASE + 0x0c)
#define CORE_DATA_FREQ_SENS_BUSY       (CORE_DATA_EMPATH_BASE + 0x10)
#define CORE_DATA_FREQ_SENS_FINISH     (CORE_DATA_EMPATH_BASE + 0x14)
#define CORE_DATA_RUN_CYCLES           (CORE_DATA_EMPATH_BASE + 0x18)
#define CORE_DATA_RAW_CYCLES           (CORE_DATA_EMPATH_BASE + 0x1c)
#define CORE_DATA_MEM_A                (CORE_DATA_EMPATH_BASE + 0x20)
#define CORE_DATA_MEM_B                (CORE_DATA_EMPATH_BASE + 0x24)

#define CORE_DATA_MEMORY_RAW_CYCLES    (CORE_DATA_MEMORY_BASE + 0x00)
#define CORE_DATA_MEMORY_TOD           (CORE_DATA_MEMORY_BASE + 0x04)
#define CORE_DATA_MEMORY_COUNT(p)      (CORE_DATA_MEMORY_BASE + 0x08 + ((p) * 4))

#define CORE_DATA_THROTTLE_RAW_CYCLES   (CORE_DATA_THROTTLE_BASE + 0x00)
#define CORE_DATA_THROTTLE_TOD          (CORE_DATA_THROTTLE_BASE + 0x04)
#define CORE_DATA_THROTTLE_IFU_THROTTLE (CORE_DATA_THROTTLE_BASE + 0x08)
#define CORE_DATA_THROTTLE_ISU_THROTTLE (CORE_DATA_THROTTLE_BASE + 0x10)
#define CORE_DATA_THROTTLE_IFU_ACTIVE   (CORE_DATA_THROTTLE_BASE + 0x18)

#define CORE_DATA_THREAD_RAW_CYCLES(t) (CORE_DATA_THREAD_BASE(t) + 0x00)
#define CORE_DATA_THREAD_TOD(t)        (CORE_DATA_THREAD_BASE(t) + 0x04)
#define CORE_DATA_THREAD_RUN_CYCLES(t) (CORE_DATA_THREAD_BASE(t) + 0x08)
#define CORE_DATA_THREAD_COMPLETION(t) (CORE_DATA_THREAD_BASE(t) + 0x0c)
#define CORE_DATA_THREAD_MEM_A(t)      (CORE_DATA_THREAD_BASE(t) + 0x10)
#define CORE_DATA_THREAD_MEM_B(t)      (CORE_DATA_THREAD_BASE(t) + 0x14)

#define CORE_DATA_DTS_CPM_UNUSED       (CORE_DATA_DTS_CPM_BASE + 0x00)
#define CORE_DATA_DTS_CPM_TOD          (CORE_DATA_DTS_CPM_BASE + 0x04)
#define CORE_DATA_SENSOR_V0            (CORE_DATA_DTS_CPM_BASE + 0x08)
#define CORE_DATA_SENSOR_V1            (CORE_DATA_DTS_CPM_BASE + 0x10)
#define CORE_DATA_SENSOR_V8            (CORE_DATA_DTS_CPM_BASE + 0x18)
#define CORE_DATA_SENSOR_V9            (CORE_DATA_DTS_CPM_BASE + 0x20)

#define CORE_DATA_PCB_SLAVE_UNUSED     (CORE_DATA_PCB_SLAVE_BASE + 0x00)
#define CORE_DATA_PCB_SLAVE_TOD        (CORE_DATA_PCB_SLAVE_BASE + 0x04)
#define CORE_DATA_PMCR                 (CORE_DATA_PCB_SLAVE_BASE + 0x08)
#define CORE_DATA_PMSR                 (CORE_DATA_PCB_SLAVE_BASE + 0x10)
#define CORE_DATA_PM_HISTORY           (CORE_DATA_PCB_SLAVE_BASE + 0x18)

#define CORE_DATA_OHA_RO_STATUS_REG    (CORE_DATA_OHA_BASE + 0x00)


#ifndef __ASSEMBLER__

// The GPE routine requires that the structure of core data collected by
// gpe_get_core_data() be represented as the offsets defined above.  This set
// of structures represents the equivalent C-structure form of the data.  Note
// that the procedure formats the TOD as a 32-bit, 2 MHz timebase.

typedef struct {
    uint32_t unused;
    uint32_t tod_2mhz;
    uint32_t dispatch;
    uint32_t completion;
    uint32_t freq_sens_busy;
    uint32_t freq_sens_finish;
    uint32_t run_cycles;
    uint32_t raw_cycles;
    uint32_t mem_a;
    uint32_t mem_b;
} CoreDataEmpath;

typedef struct {
    uint32_t raw_cycles;
    uint32_t tod_2mhz;
    uint32_t count[4];
} CoreDataPerPartitionMemory;

typedef struct {
    uint32_t raw_cycles;
    uint32_t tod_2mhz;
    uint32_t ifu_throttle;
    uint32_t isu_throttle;
    uint32_t ifu_active;
    uint32_t undefined;
} CoreDataThrottle;

typedef struct {
    uint32_t raw_cycles;
    uint32_t tod_2mhz;
    uint32_t run_cycles;
    uint32_t completion;
    uint32_t mem_a;
    uint32_t mem_b;
} CoreDataPerThread;

typedef struct {
    uint32_t unused;
    uint32_t tod_2mhz;
    sensors_v0_t sensors_v0;
    sensors_v1_t sensors_v1;
    sensors_v8_t sensors_v8;
    sensors_v9_t sensors_v9;
} CoreDataDtsCpm;
    
typedef struct {
    uint32_t unused;
    uint32_t tod_2mhz;
    pcbs_power_management_control_reg_t pmcr;
    pcbs_power_management_status_reg_t pmsr;
    pcbs_pmstatehistocc_reg_t pm_history;
} CoreDataPcbSlave;

typedef struct {
    oha_ro_status_reg_t oha_ro_status_reg;
} CoreDataOha;
    
typedef struct {
    CoreDataEmpath             empath;
    CoreDataPerPartitionMemory per_partition_memory;
    CoreDataThrottle           throttle;
    CoreDataPerThread          per_thread[8];
    CoreDataDtsCpm             dts_cpm;
    CoreDataPcbSlave           pcb_slave;
    CoreDataOha                oha;
} CoreData;

#endif // __ASSEMBLER__


/// \defgroup core_data_status_bits Core Data Status Bits
///
/// These bits are set (if appropriate) in the low-order reserved area of the
/// OHA_RO_STATUS_REG image stored in the CoreData structure.
///
/// @{

/// This bit is set if SCOM access to the OHA returns a non-0 PIB return code
/// when trying to write the OHA_CPM_HIST_RESET_REG to set up PC-only special
/// wakeup.
#define CORE_DATA_CPM_HIST_RESET_ACCESS_FAILED 0x01

/// This bit is set if access to the OHA returns a non-0 PIB return code when
/// trying to read the OHA_RO_STATUS_REG to determine core status.
#define CORE_DATA_OHA_RO_STATUS_ACCESS_FAILED 0x02

/// This bit is set if EMPATH data was requested to be collected and was
/// collected. If this bit is not set then any EMPATH data requested to be
/// collected will be 0. 
///
/// If EMPATH data was requested but was not collected, then one of the bits
/// CORE_DATA_EXPECTED_EMPATH_ERROR or CORE_DATA_UNEXPECTED_EMPATH_ERROR will
/// be set, and the error code is stored in the OHA_RO_STATUS register image.
#define CORE_DATA_EMPATH_COLLECTED 0x04

/// This bit is set if core sensor data (DTS/CPM) was collected. If this bit
/// is not set then core DTS/CPM data will be 0.
#define CORE_DATA_CORE_SENSORS_COLLECTED 0x08

/// This bit is set if L3 sensor data (DTS/CPM) was collected. If this bit is
/// not set then L3 DTS/CPM data will be 0.
#define CORE_DATA_L3_SENSORS_COLLECTED 0x10

/// If this bit is set, then an "expected" error was encountered while
/// collecting EMPATH data. Given that the procedure has gone through the
/// PC-only special wakeup protocol, the only "expected" error is the
/// intermittant PCB error code #4 due to HW280375.
#define CORE_DATA_EXPECTED_EMPATH_ERROR 0x20

/// If this bit is set, then an "unexpected" error was encountered while
/// collecting EMPATH data. Given that the procedure has gone through the
/// PC-only special wakeup protocol, the only "expected" error is the
/// intermittant PCB error code #4 due to HW280375. If this bit is set it
/// indicates a serious problem.
#define CORE_DATA_UNEXPECTED_EMPATH_ERROR 0x40

/// The first bit of the 4-bit PCB parity + error code, in the event a PCB
/// error is encountered during EMPATH processing.
#define CORE_DATA_EMPATH_ERROR_LOCATION 52

#define CORE_DATA_EMPATH_ERROR_BITS 4

/// @}


////////////////////////////////////////////////////////////////////////////
// gpe_get_core_data_fast()
////////////////////////////////////////////////////////////////////////////

#ifndef __ASSEMBLER__

/// Paramaters for gpe_get_chip_data_fast()

typedef struct {

    /// gpe_get_core_data_fast() only collects data for chiplets configured in
    /// this mask.
    ChipConfig config;

    /// This mask, comprised of a logical OR of the GPE_GET_CORE_DATA_FAST_*
    /// macros, controls which data groups are collected.
    uint64_t select;

    /// This is the 32-bit pointer (cast to a uint64_t) to the chiplet raw
    /// data area to be filled by this invocation of gpe_get_core_data_fast().
    uint64_t data;

} GpeGetChipDataFastParms;

PoreEntryPoint gpe_get_core_data_fast;

#endif  /* __ASSEMBLER__ */

// Parameter offsets for gpe_get_core_data()

#define GPEGETCOREDATAFASTPARMS_CONFIG  0x00
#define GPEGETCOREDATAFASTPARMS_SELECT  0x08
#define GPEGETCOREDATAFASTPARMS_DATA    0x10

// Data group select masks for gpe_get_core_data_fast()

#define GPE_GET_CORE_DATA_FAST_FREQ_TARGET 0x0001

#define CORE_DATA_FAST_FREQ_TARGET_BASE 0x0
#define CORE_DATA_FAST_FREQ_TARGET_SIZE (8 + (PGP_NCORES * 8))

#define CORE_DATA_FAST_SIZE \
    (CORE_DATA_FAST_FREQ_TARGET_BASE + CORE_DATA_FAST_FREQ_TARGET_SIZE)

#define CORE_DATA_FAST_FREQ_TARGET_UNUSED    (CORE_DATA_FAST_FREQ_TARGET_BASE + 0)
#define CORE_DATA_FAST_FREQ_TARGET_TOD       (CORE_DATA_FAST_FREQ_TARGET_BASE + 4)
#define CORE_DATA_FAST_FREQ_TARGET_LPFTSR(n) (CORE_DATA_FAST_FREQ_TARGET_BASE + 8 + ((n) * 8))

#ifndef __ASSEMBLER__

typedef struct {
    uint32_t unused;
    uint32_t tod_2mhz;
    pcbs_local_pstate_frequency_target_status_reg_t lpftsr[PGP_NCORES];
} CoreDataFast;

#endif // __ASSEMBLER__


////////////////////////////////////////////////////////////////////////////
// gpe_get_chip_data()
////////////////////////////////////////////////////////////////////////////

#ifndef __ASSEMBLER__

/// Paramaters for gpe_get_chip_data()

typedef struct {

    /// This mask, comprised of a logical OR of the GPE_GET_CHIP_DATA_*
    /// macros, controls which data groups are collected.
    uint64_t select;

    /// This is the 32-bit pointer (cast to a uint64_t) to the chiplet raw
    /// data area to be filled by this invocation of gpe_get_chip_data().
    uint64_t data;

} GpeGetChipDataParms;

PoreEntryPoint gpe_get_chip_data;

#endif  /* __ASSEMBLER__ */

// Parameter offsets for gpe_get_chip_data()

#define GPEGETCHIPDATAPARMS_SELECT 0x00
#define GPEGETCHIPDATAPARMS_DATA   0x08

// Data group select masks for gpe_get_chip_data()

#define GPE_GET_CHIP_DATA_OVERCOMMIT 0x0001

#define CHIP_DATA_OVERCOMMIT_BASE 0
#define CHIP_DATA_OVERCOMMIT_SIZE 56

#define CHIP_DATA_SIZE (CHIP_DATA_OVERCOMMIT_BASE + CHIP_DATA_OVERCOMMIT_SIZE)


////////////////////////////////////////////////////////////////////////////
// gpe_get_mem_data()
////////////////////////////////////////////////////////////////////////////

#ifndef __ASSEMBLER__

/// Paramaters for gpe_get_mem_data()

typedef struct {

    /// The index (0 .. PGP_NCENTAUR - 1) of the Centaur whose sensor cache
    /// data to collect, or -1 to bypass collection.
    uint64_t collect;

    /// The index (0 .. PGP_NCENTAUR - 1) of the Centaur to "poke" to cause it
    /// to begin collecting the next round of data into its sensor cache, or
    /// -1 to bypass updating.
    uint64_t update;

    /// This is the 32-bit pointer (cast to a uint64_t) to the chiplet raw
    /// data area to be filled by this invocation of gpe_get_mem_data(). This
    /// pointer need not be valid if the \a collect field of the structure is
    /// -1.
    uint64_t data;

    /// The return code returned by the last invocation of the procedure; See
    /// \ref gpe_get_mem_date_rc.
    uint64_t rc;
    
    /// The 'update' timestamp
    ///
    /// This is the value of the chip TOD at the time the 'update' phase of
    /// the procedure is run, as close as possible to the "poke" of the
    /// Centaur. This timestamp indicates the time that the Centaur sensor
    /// cache line collection was kicked off. The timestamp is collected even
    /// if the \a update field of the structure is -1. Consistent with
    /// gpe_get_core_data() the timestamp is reduced to a 32-bit, 2MHz
    /// timestamp, and stored in the low-order half of a doubleword.
    uint32_t pad;
    uint32_t tod_2mhz;

} GpeGetMemDataParms;

PoreEntryPoint gpe_get_mem_data;

#endif  /* __ASSEMBLER__ */

// Parameter offsets for gpe_get_mem_data()

#define GPEGETMEMDATAPARMS_COLLECT 0x00
#define GPEGETMEMDATAPARMS_UPDATE  0x08
#define GPEGETMEMDATAPARMS_DATA    0x10
#define GPEGETMEMDATAPARMS_RC      0x18
#define GPEGETMEMDATAPARMS_PAD_TOD 0x20
#define SIZEOF_GPEGETMEMDATAPARMS  0x28


/// \defgroup gpe_mem_data_rc gpe_get_mem_data() Error Return Codes
///
/// The gpe_get_mem_data() procedure deposits a non-0 return code into the \a
/// rc field of its parameter structure in the event of failure. Note that the
/// procedure stops on the first failure, and in particular the TOD timestamp
/// is not valid in the event of failure.
///
/// @{

/// The procedure died, but no other information is available. This would have
/// signalled an error interrupt and the PORE flex request will contain FFDC
/// about the failure.
#define GPE_GET_MEM_DATA_DIED 1

/// The \a collect parameter was invalid, i.e. it either was an illegal index
/// or the index of an unconfigured MCS or Centaur.
#define GPE_GET_MEM_DATA_COLLECT_INVALID 2

/// The \a update parameter was invalid, i.e. it either was an illegal index
/// or the index of an unconfigured MCS or Centaur.
#define GPE_GET_MEM_DATA_UPDATE_INVALID 3

/// The global G_centaurConfiguration is not valid
#define GPE_GET_MEM_DATA_NOT_CONFIGURED 4

/// The workaround for HW256773 failed.  To diagnose the failure look at the
/// 'rc' field of the global variable G_hw256773.
#define GPE_GET_MEM_DATA_HW256773_FAILED 5

/// This code is established in the RC field prior to collecting the Centaur
/// sensor cache data. If this RC is observed on a hard failure it most likely
/// indicates an error assiciated with the Centaur whose data was being
/// collected.
#define GPE_GET_MEM_DATA_SENSOR_CACHE_FAILED 6

/// This code is established in the RC field prior to "poking" the Centaur (if
/// any) that is being updated this pass. If this RC is observed on a hard
/// failure it most likely indicates an error associated with the Centaur
/// being updated.
#define GPE_GET_MEM_DATA_UPDATE_FAILED 7

/// @}        


#ifndef __ASSEMBLER__

// The GPE routine requires that the structure of centaur data collected by
// gpe_get_mem_data() be represented as the offsets defined above.  This set
// of structures represent the equivalent C-structure form of the data.  Note
// that the procedure formats the TOD as a 32-bit, 2 MHz timebase.

/// Layout of data collected from MCS
///
/// This is currently empty, however to avoid code rewrites if any data is
/// ever collected here the structure is declared and placed in the larger
/// MemData structure.  The fact that the structure is empty does not seem to
/// cause problems. 

typedef struct {
} MemDataMcs;

/// The layout of a Centaur chip thermal sensor
///
/// \todo Centaur spec. has no doc. on layout of these bits; Waiting for more
/// info from Centaur team.

typedef union {
    uint16_t value;
    struct {
        uint16_t value;
    } fields;
} centaur_sensor_t;

/// The layout of a Centaur DIMM sensor
///
/// Mnemonic macros for the 2-bit status codes (DIMM_SENSOR_STATUS_*) are
/// currently defined in ssx/pgp/pgp_common.h
///
/// \todo Waiting for more info from Centaur team on how to interpret

typedef union {
    uint16_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint16_t crit_trip : 1;
    uint16_t alarm_trip : 1;
    uint16_t below_trip : 1;
    uint16_t sign_bit : 1;
    uint16_t temperature : 8;
    uint16_t temp_fraction : 2;
    uint16_t status : 2;
#else
    uint16_t status : 2;
    uint16_t temp_fraction : 2;
    uint16_t temperature : 8;
    uint16_t sign_bit : 1;
    uint16_t below_trip : 1;
    uint16_t alarm_trip : 1;
    uint16_t crit_trip : 1;
#endif
    } fields;
} centaur_dimm_sensor_t;

/// The layout of the status bits of the sensor cache line
///
/// The sensor cache-line aggregator gets each element of the sensor cache
/// line by an internal SCOM.  The individual PCB return codes for each SCOM
/// are collected here (3 bits each) - note that many of the 32-bit registers
/// come back in a single 64-bit internal SCOM. Normally this register will
/// always read as 0 indicating all data was collected successfully.  The PCB
/// error codes (PCB_ERROR_*) are currently defined in ssx/pgp/pgp_common.h.

typedef union {
    uint64_t value;
    struct {
#ifdef _BIG_ENDIAN
        uint64_t mba01_rw : 3;         /// mba01_rd[+ wr]
        uint64_t mba01_ap : 3;         /// mba01_act[+ powerups]
        uint64_t mba23_rw : 3;         /// mba23_rd[+ wr]
        uint64_t mba23_ap : 3;         /// mba23_act[+ powerups]
        uint64_t mba_sc : 3;           /// mba01[+ 23]_spec_cancels
        uint64_t lp2_exits : 3;        /// lp2_exits
        uint64_t frame_count : 3;      /// frame_count
        uint64_t mba01_chrw : 3;       /// mba01_cache_hits_rd[+ wr]
        uint64_t mba23_chrw : 3;       /// mba23_cache_hits_rd[+ wr]
        uint64_t mba01_iac_bl : 3;     /// mba01_intreq_arr_cnt_base[+ low]
        uint64_t mba01_iac_mh : 3;     /// mba01_intreq_arr_cnt_med[+ high]
        uint64_t mba23_iac_bl : 3;     /// mba23_intreq_arr_cnt_base[+ low]
        uint64_t mba23_iac_mh : 3;     /// mba23_intreq_arr_cnt_med[+ high]
        uint64_t iac_high_latency : 3; /// intereq_arr_cnt_high_latency
        uint64_t centaur01 : 3;        /// centaur_thermal_sensor[0 - 1]
        uint64_t dimm03 : 3;           /// dimm_thermal_sensor[0 - 3]
        uint64_t dimm47 : 3;           /// dimm_thermal_sensor[4 - 7]
        uint64_t reserved : 13;
#else
        uint64_t reserved : 13;
        uint64_t dimm47 : 3;           /// dimm_thermal_sensor[4 - 7]
        uint64_t dimm03 : 3;           /// dimm_thermal_sensor[0 - 3]
        uint64_t centaur01 : 3;        /// centaur_thermal_sensor[0 - 1]
        uint64_t iac_high_latency : 3; /// intereq_arr_cnt_high_latency
        uint64_t mba23_iac_mh : 3;     /// mba23_intreq_arr_cnt_med[+ high]
        uint64_t mba23_iac_bl : 3;     /// mba23_intreq_arr_cnt_base[+ low]
        uint64_t mba01_iac_mh : 3;     /// mba01_intreq_arr_cnt_med[+ high]
        uint64_t mba01_iac_bl : 3;     /// mba01_intreq_arr_cnt_base[+ low]
        uint64_t mba23_chrw : 3;       /// mba23_cache_hits_rd[+ wr]
        uint64_t mba01_chrw : 3;       /// mba01_cache_hits_rd[+ wr]
        uint64_t frame_count : 3;      /// frame_count
        uint64_t lp2_exits : 3;        /// lp2_exits
        uint64_t mba_sc : 3;           /// mba01[+ 23]_spec_cancels
        uint64_t mba23_ap : 3;         /// mba23_act[+ powerups]
        uint64_t mba23_rw : 3;         /// mba23_rd[+ wr]
        uint64_t mba01_ap : 3;         /// mba01_act[+ powerups]
        uint64_t mba01_rw : 3;         /// mba01_rd[+ wr]
#endif
    } fields;
} centaur_scom_status_t;

/// The layout of the Centaur sensor cache line

typedef struct {
    uint32_t mba01_rd;                      // PP1/MBA01 Reads 
    uint32_t mba01_wr;                      // PP1/MBA01 Writes
    uint32_t mba01_act;                     // PP1/MBA01 Activations
    uint32_t mba01_powerups;                // PP1/MBA01 PowerUps
    
    uint32_t mba23_rd;                      // PP2/MBA23 Reads 
    uint32_t mba23_wr;                      // PP2/MBA23 Writes
    uint32_t mba23_act;                     // PP2/MBA23 Activations
    uint32_t mba23_powerups;                // PP2/MBA23 PowerUps

    uint32_t mba01_spec_cancels;            // PP1/MBA01 Speculative Cancels
    uint32_t mba23_spec_cancels;            // PP2/MBA23 Speculative Cancels
#ifdef _BIG_ENDIAN
    uint32_t eventn     :4;                 // EVENTN
    uint32_t reserved_0 :20;                // Reserved
    uint32_t lp2_exits  :8;                 // LP2 Exits
#else
    uint32_t lp2_exits  :8;                 // LP2 Exits
    uint32_t reserved_0 :20;                // Reserved
    uint32_t eventn     :4;                 // EVENTN
#endif
    uint32_t frame_count;                   // Frame Count (timestamp)

    uint32_t mba01_cache_hits_rd;           // PP1/MBA01 Cache Hits Reads
    uint32_t mba01_cache_hits_wr;           // PP1/MBA01 Cache Hits Writes
    uint32_t mba23_cache_hits_rd;           // PP2/MBA23 Cache Hits Reads
    uint32_t mba23_cache_hits_wr;           // PP2/MBA23 Cache Hits Writes
    
    uint32_t mba01_intreq_arr_cnt_base;     // PP1/MBA01 Inter-Req Arrival Count Base
    uint32_t mba01_intreq_arr_cnt_low;      // PP1/MBA01 Inter-Req Arrival Count Low
    uint32_t mba01_intreq_arr_cnt_med;      // PP1/MBA01 Inter-Req Arrival Count Med 
    uint32_t mba01_intreq_arr_cnt_high;     // PP1/MBA01 Inter-Req Arrival Count High

    uint32_t mba23_intreq_arr_cnt_base;     // PP2/MBA23 Inter-Req Arrival Count Base
    uint32_t mba23_intreq_arr_cnt_low;      // PP2/MBA23 Inter-Req Arrival Count Low
    uint32_t mba23_intreq_arr_cnt_med;      // PP2/MBA23 Inter-Req Arrival Count Med 
    uint32_t mba23_intreq_arr_cnt_high;     // PP2/MBA23 Inter-Req Arrival Count High

    uint32_t intreq_arr_cnt_high_latency;   // Inter-Req Arrival Count High Latency
    centaur_sensor_t centaur_thermal_sensor[2];   // Centaur Thermal Sensors 0-1
    centaur_dimm_sensor_t dimm_thermal_sensor[8]; // DIMM Thermal Sensors 0-7
    centaur_scom_status_t status;           // Aggregated internal SCOM status
} MemDataSensorCache;

typedef struct {
    MemDataMcs         mcs;                 // TODO:  Not collected yet
    MemDataSensorCache scache;              // OCC Centaur Sensor Cache Line (128 bytes)
} MemData;

#endif // __ASSEMBLER__


// Data offsets for gpe_get_mem_data()

#define MEM_DATA_MCS_BASE 0
#define MEM_DATA_MCS_SIZE 0

#define MEM_DATA_CENTAUR_BASE (MEM_DATA_MCS_BASE + MEM_DATA_MCS_SIZE)
#define MEM_DATA_CENTAUR_SIZE 128

#define MEM_DATA_SIZE (MEM_DATA_MCS_SIZE + MEM_DATA_CENTAUR_SIZE)

#endif  /* __GPE_DATA_H__ */
