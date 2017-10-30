#if !defined(__CENTAUR_MEM_DATA_H__)
#define __CENTAUR_MEM_DATA_H__
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
    MemDataSensorCache scache;              // OCC Centaur Sensor Cache Line (128 bytes)
} CentaurMemData;

/// \defgroup gpe_mem_data_rc gpe_get_mem_data() Error Return Codes
///
/// The gpe_get_mem_data() procedure deposits a non-0 return code into the \a
/// rc field of its parameter structure in the event of failure. Note that the
/// procedure stops on the first failure, and in particular the TOD timestamp
/// is not valid in the event of failure.
///
/// @{

/// The global G_centaurConfiguration is not valid
#define CENTAUR_GET_MEM_DATA_NOT_CONFIGURED 4

/// The workaround for HW256773 failed.  To diagnose the failure look at the
/// 'rc' field of the global variable G_hw256773.
#define CENTAUR_GET_MEM_DATA_HW256773_FAILED 5

/// This code is established in the RC field prior to collecting the Centaur
/// sensor cache data. If this RC is observed on a hard failure it most likely
/// indicates an error assiciated with the Centaur whose data was being
/// collected.
#define CENTAUR_GET_MEM_DATA_SENSOR_CACHE_FAILED 6

/// This code is established in the RC field prior to "poking" the Centaur (if
/// any) that is being updated this pass. If this RC is observed on a hard
/// failure it most likely indicates an error associated with the Centaur
/// being updated.
#define CENTAUR_GET_MEM_DATA_UPDATE_FAILED 7

#endif
