/******************************************************************************
// @file occ_service_codes.h
// @brief Error codes for OCC.
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section _occ_service_codes_h occ_service_codes.h
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *                      TEAM      06/16/2010  Port
 *   @01                nguyenp   08/18/2011  Added new OCC reason code
 *                      pbavari   08/16/2011  Added new reason code
 *   @02                tapiar    08/06/2011  New failure reason code
 *   @pb007             pbavari   09/27/2011  Added timer reasonCode and moduleid
 *   @03                abagepa   09/06/2011  new dcom failure reason codes
 *   @04                nguyenp   10/10/2011  New proc failure reason code
 *   @05                tapiar    11/17/2011  new dcom failure reason codes
 *   @pb009             pbavari   10/20/2011  Main thread support
 *   @rc003             rickylie  02/03/2012  Verify & Clean Up OCC Headers & Comments
 *   @pb00E             pbavari   03/11/2012  Added correct include file
 *   @nh001             neilhsu   05/23/2012  Add missing error log tags
 *   @th010             thallet   07/14/2012  Added ERCs
 *   @ai002             ailutsar  07/24/2012  Fix data tlb miss in startApplet
 *   @th022             thallet   10/10/2012  Added Module IDs for State & Mode changes
 *   @ly003  861535     lychen    11/19/2012  Remove APSS configuration/gathering of Altitude & Temperature
 *   @th031  878471     thallet   04/15/2013  Centaur Throttles
 *   @th033             thallet   04/17/2013  Mnfg AVP Support
 *   @fk001  879727     fmkassem  04/16/2013  PCAP support.
 *   @at015  885884     alvinwan  06/10/2013  Support Observation/Active state change
 *   @db001  897459     deepthib  08/03/2013  AMEC pcap failure, mismatch changes
 *   @rt001  897459     tapiar    08/20/2013  AMEC Slave Performance Check
 *   @rt002  901927     tapiar    10/02/2013  Fix Extended Refcodes
 *   @gs012  903325     gjsilva   10/18/2013  Log Processor OT errors
 *   @gm011  903410     milesg    10/22/2013  Fail on murano dd10 or dd11 chips
 *   @gs015  905166     gjsilva   11/04/2013  Full support for IPS function
 *   @rt003  905677     tapiar    11/07/2013  Match over pcap reason code to P7 value
 *   @sb000  905504     sbroyles  11/06/2013  New extrefs for HOMER error.
 *   @gm013  907548     milesg    11/22/2013  Memory therm monitoring support
 *   @jh009  908383     joshych   12/04/2013  Generate error logs when required for Reset Prep command
 *   @gm015  907601     milesg    12/06/2013  L4 Bank Delete circumvention and centaur i2c recovery
 *   @gs021  909855     gjsilva   12/18/2013  Support for processor OT condition
 *   @at020  908666     alvinwan  12/16/2013  Oversubscription Error Handling
 *   @at021  910861     alvinwan  01/08/2014  Change SRCs back to original values
 *   @jh00b  910184     joshych   01/10/2014  Add check for checkstop
 *   @at023  910877     alvinwan  01/09/2014  Excessive fan increase requests error for mfg
 *   @gm022  908890     milesg    01/23/2014  Halt OCC on OCCLFIR[38]
 *   @fk005  911760     fmkassem  01/20/2014  APSS retry support.
 *   @gs025  913663     gjsilva   01/30/2014  Full fupport for soft frequency boundaries
 *   @fk006  914801     fmkassem  02/05/2014  Modify reasoncode list
 *   @gm025  915973     milesg    02/14/2014  Full support for sapphire (KVM) mode
 *   @sb055  911966     sbroyles  02/27/2014  Enable PBCS heartbeat
 *   @gm031  916489     milesg    03/07/2014  Additional handling for OISR0[2] and OISR0[9]
 *   @wb001  919163     wilbryan  03/06/2014  Updating error call outs, descriptions, and severities
 *   @wb003  920760     wilbryan  03/24/2014  Update SRCs to match TPMD SRCs
 *   @gm033  920448     milesg    03/26/2014  use getscom/putscom ffdc wrapper
 *   @sbpde  922027     sbroyles  04/04/2014  Add error check to GPE proc data
 *                                            collection.
 *   @gm036  917550     milesg    04/09/2014  Ignore phantom interrupts (log info error)
 *   @wb004  922138     wilbryan  04/07/2014  Ensure timely pstate completion
 *   @gm037  925908     milesg    05/07/2014  Redundant OCC/APSS support
 *   @gm041  928150     milesg    06/02/2014  log error in mfg if centaur lfir[6] bit is set
 *   @gs041  942203     gjsilva   10/17/2014  Support for HTMGT/BMC interface
 *   @gs042  942940     gjsilva   10/24/2014  Support for data packets in BMC-based systems
 *
 *  @endverbatim
 *
 *///*************************************************************************/

#ifndef _OCC_SERVICE_CODES_H_
#define _OCC_SERVICE_CODES_H_

//*************************************************************************
// Includes
//*************************************************************************
#include <comp_ids.h>

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************
enum occReasonCode        // @nh001c @fk006c
{
    /// Generic OCC firmware error log with extended srcs.
    INTERNAL_FAILURE                = 0x00,
    /// Informational periodic call home log
    GEN_CALLHOME_LOG                = 0x01,
    /// Failure within the OCC Complex of the processor
    PREP_FOR_RESET                  = 0x02, // @jh009a
    /// Invalid Input Data received from FSP
    INVALID_INPUT_DATA              = 0x03, // @wb003
    /// Oversubscription was asserted
    OVERSUB_ALERT                   = 0x05, // @at020a
    /// Failure to maintain a hard power cap
    POWER_CAP_FAILURE               = 0x06, // @wb003
    /// Failsafe State
    FAILSAFE_ALERT                  = 0x07, // @wb001
    /// Fans are in full speed
    FAN_FULL_SPEED                  = 0x08, // @at023a
    /// Timed out reading a FRU temperature
    FRU_TEMP_TIMEOUT                = 0x09,
    /// Processor reached error threshold
    PROC_ERROR_TEMP                 = 0x10,
    /// Timed out reading processor temperature
    PROC_TEMP_TIMEOUT               = 0x11,
    /// @wb003 -- Removed PROCESSOR_NOT_SUPPORTED and changed error to INTERNAL_FAILURE
    /// Processor SCOM failure -- gm025
    PROC_SCOM_ERROR                 = 0x16,
    /// Any failure coming from the SSX RTOS code
    SSX_GENERIC_FAILURE             = 0x17, // @wb003
    /// Failure to handshake with an external fw entity (HB, FSP, PHYP, etc)
    EXTERNAL_INTERFACE_FAILURE      = 0x18, // @wb003
    /// VRM reached error threshold (VR_HOT asserted)
    VRM_ERROR_TEMP                  = 0x20,
    /// Timed out reading VR_FAN signal from VRM
    VRM_VRFAN_TIMEOUT               = 0x21,
    /// VR_FAN signal from VRM has been asserted
    VRM_VRFAN_ASSERTED              = 0x22,
    /// DIMM reached error threshold
    DIMM_ERROR_TEMP                 = 0x30,
    /// Frequency limited due to oversubscription condition
    OVERSUB_LIMIT_ALERT             = 0x33,  // SRC the same with TPMD // @at020a
    /// Centaur reached error threshold
    CENT_ERROR_TEMP                 = 0x40,
    /// Centaur in-band scom failure
    CENT_SCOM_ERROR                 = 0x41,
    /// Centaur FIR bit set
    CENT_LFIR_ERROR                 = 0x42, //gm041
    /// Throttle in nominal or turbo mode due to the bulk power limit being reached with both power supplies good @rt003a
    PCAP_THROTTLE_POWER_LIMIT       = 0x61,
    /// Failure within the OCC Complex of the processor
    INTERNAL_HW_FAILURE             = 0xB0, //@01a @jh009c
    /// OCC GPE halted due to checkstop
    OCC_GPE_HALTED                  = 0xB1, // @jh00ba
    /// PMC Failure
    PMC_FAILURE                     = 0xB2, // gm031
    /// Data passed as an argument or returned from a function is invalid
    INTERNAL_INVALID_INPUT_DATA     = 0xB3, // @wb003
    /// A core was not at the expected frequency
    TARGET_FREQ_FAILURE             = 0xB4, // @wb004
    ///  Request to read APSS data failed.
    APSS_GPE_FAILURE                = 0xC0, // @fk005a
    /// Connector overcurrent pin still asserted.
    CONNECTOR_OC_PINS_WARNING       = 0xC1, // @wb001
    CONNECTOR_OC_PINS_FAILURE       = 0xC2, // @wb001
    ///  Request to read redundant APSS data failed -- gm037
    REDUNDANT_APSS_GPE_FAILURE      = 0xCB, 
    /// Success!
    OCC_SUCCESS_REASON_CODE         = 0xFF,
};


enum occExtReasonCode    // @nh001a
{
    OCC_NO_EXTENDED_RC                          = 0x00000000,

    ERC_GENERIC_TIMEOUT                         = 0x00000001,
    ERC_INVALID_INPUT_DATA                      = 0x00000002,
    ERC_MMU_MAP_FAILURE                         = 0x00000003,
    ERC_MMU_UNMAP_FAILURE                       = 0x00000004,
    ERC_BCE_REQUEST_CREATE_FAILURE              = 0x00000005,
    ERC_BCE_REQUEST_SCHEDULE_FAILURE            = 0x00000006,

    ERC_RUNNING_SEM_PENDING_FAILURE             = 0x00000007,
    ERC_RUNNING_SEM_POSTING_FAILURE             = 0x00000008,
    ERC_WAKEUP_SEM_PENDING_FAILURE              = 0x00000009,
    ERC_WAKEUP_SEM_POSTING_FAILURE              = 0x0000000a,
    ERC_FINISHED_SEM_PENDING_FAILURE            = 0x0000000b,
    ERC_FINISHED_SEM_POSTING_FAILURE            = 0x0000000c,
    ERC_CALLER_SEM_POSTING_FAILURE              = 0x0000000d,
    ERC_CREATE_SEM_FAILURE                      = 0x0000000e,

    ERC_LOW_CORE_PORE_FLEX_CREATE_FAILURE       = 0x0000000f,
    ERC_HIGH_CORE_PORE_FLEX_CREATE_FAILURE      = 0x00000010,
    ERC_FAST_CORE_PORE_FLEX_CREATE_FAILURE      = 0x00000011,

    ERC_SSX_IRQ_SETUP_FAILURE                   = 0x00000012,
    ERC_SSX_IRQ_HANDLER_SET_FAILURE             = 0x00000013,
    ERC_PPC405_WD_SETUP_FAILURE                 = 0x00000014,
    ERC_OCB_WD_SETUP_FAILURE                    = 0x00000015,
    ERC_ARG_POINTER_FAILURE                     = 0x00000016,

    ERC_PSS_GPIO_INIT_FAIL                      = 0x00000017,
    ERC_PSS_COMPOSITE_MODE_FAIL                 = 0x00000019,

    ERC_PROC_CONTROL_TASK_FAILURE               = 0x0000001a,   // @th010

    ERC_MMU_MAP_APPLET_READ_FAILURE             = 0x0000001b,   // @ai002
    ERC_MMU_MAP_APPLET_WRITE_FAILURE            = 0x0000001c,   // @ai002
    ERC_MMU_MAP_APPLET_OVERWRITE_FAILURE        = 0x0000001d,   // @ai002
    ERC_MMU_UNMAP_APPLET_READ_FAILURE           = 0x0000001e,   // @ai002
    ERC_MMU_UNMAP_APPLET_WRITE_FAILURE          = 0x0000001f,   // @ai002
    ERC_MMU_UNMAP_APPLET_OVERWRITE_FAILURE      = 0x00000020,   // @ai002

    ERC_CENTAUR_PORE_FLEX_CREATE_FAILURE        = 0x00000021,   // @th031
    ERC_CENTAUR_PORE_FLEX_SCHEDULE_FAILURE      = 0x00000022,   // @
    ERC_CENTAUR_INTERNAL_FAILURE                = 0x00000023,   // @

    ERC_APSS_GPIO_OUT_OF_RANGE_FAILURE          = 0x00000024,   // @at014
    ERC_APSS_GPIO_DUPLICATED_FAILURE            = 0x00000025,   // @at014
    ERC_APSS_ADC_OUT_OF_RANGE_FAILURE           = 0x00000026,   // @at014
    ERC_APSS_ADC_DUPLICATED_FAILURE             = 0x00000027,   // @at014

    ERC_STATE_FROM_OBS_TO_STB_FAILURE           = 0x00000028,   // @at015
    ERC_STATE_FROM_STB_TO_OBS_FAILURE           = 0x00000029,   // @at015
    ERC_STATE_HEARTBEAT_CFG_FAILURE             = 0x00000080,   // @sb055

    ERC_AMEC_PCAPS_MISMATCH_FAILURE             = 0x0000002A,   // @db001a
    ERC_AMEC_UNDER_PCAP_FAILURE                 = 0x0000002B,   // @db001a

    ERC_AMEC_SLAVE_FAILSAFE_STATE               = 0x0000002C,   // @rt001a
    ERC_AMEC_SLAVE_OVS_STATE                    = 0x0000002D,   // @rt001a
    ERC_AMEC_SLAVE_POWERCAP                     = 0x0000002E,   // @rt001a

    ERC_AMEC_PROC_ERROR_OVER_TEMPERATURE        = 0x0000002F,   // @gs012a

    ERC_APLT_INIT_FAILURE                       = 0x00000030,   //
    ERC_APLT_START_VERSION_MISMATCH             = 0x00000031,   //
    ERC_APLT_START_CHECKSUM_MISMATCH            = 0x00000032,   //

    ERC_CMDH_MBOX_REQST_FAILURE                 = 0x00000040,   //
    ERC_CMDH_INTERNAL_FAILURE                   = 0x00000041,   //
    ERC_CMDH_THRM_DATA_MISSING                  = 0x00000042,   //
    ERC_CMDH_IPS_DATA_MISSING                   = 0x00000043,
    ERC_CMDH_INVALID_ATTN_DATA                  = 0x00000044,   // @gs041

    ERC_CHIP_IDS_INVALID                        = 0x00000050,
    ERC_GETSCOM_FAILURE                         = 0x00000051,

    ERC_HOMER_MAIN_ACCESS_ERROR                 = 0x00000060,   // @sb000
    ERC_HOMER_MAIN_SSX_ERROR                    = 0x00000061,   // @sb000

    ERC_APSS_SCHEDULE_FAILURE                   = 0x00000062,   // @fk005a
    ERC_APSS_COMPLETE_FAILURE                   = 0x00000063,   // @fk005a

    ERC_PROC_CONTROL_INIT_FAILURE               = 0x00000064,   // @wb003 - Changed because it was previously a duplicate
    ERC_PROC_PSTATE_INSTALL_FAILURE             = 0x00000065,   // @wb003
    ERC_PROC_CORE_DATA_EMPATH_ERROR             = 0x00000066,   // @sbpde

    ERC_BCE_REQ_CREATE_READ_FAILURE             = 0x00000070,
    ERC_BCE_REQ_SCHED_READ_FAILURE              = 0x00000071,
    ERC_BCE_REQ_CREATE_INPROG_FAILURE           = 0x00000072,
    ERC_BCE_REQ_SCHED_INPROG_FAILURE            = 0x00000073,
    ERC_BCE_REQ_CREATE_WRITE_FAILURE            = 0x00000074,
    ERC_BCE_REQ_SCHED_WRITE_FAILURE             = 0x00000075,
};


enum occModuleId
{
    MAIN_MID                        =  MAIN_COMP_ID | 0x01,
    MAIN_THRD_ROUTINE_MID           =  MAIN_COMP_ID | 0x02,
    MAIN_THRD_TIMER_MID             =  MAIN_COMP_ID | 0x03,
    MAIN_THRD_SEM_INIT_MID          =  MAIN_COMP_ID | 0x04,
    MAIN_STATE_TRANSITION_MID       =  MAIN_COMP_ID | 0x05,     // @th022
    MAIN_MODE_TRANSITION_MID        =  MAIN_COMP_ID | 0x06,     // @th022
    MAIN_GPE_HALTED_MID             =  MAIN_COMP_ID | 0x07,     // @jh00ba
    OCC_IRQ_SETUP                   =  MAIN_COMP_ID | 0x08,     // @gm022
    PMC_HW_ERROR_ISR                =  MAIN_COMP_ID | 0x09,     // @gm031
    GETSCOM_FFDC_MID                =  MAIN_COMP_ID | 0x0a,
    PUTSCOM_FFDC_MID                =  MAIN_COMP_ID | 0x0b,
    HMON_ROUTINE_MID                =  MAIN_COMP_ID | 0x0c,     // @gm036
	AMEC_VERIFY_FREQ_MID            =  MAIN_COMP_ID | 0x0d,     // @wb004
};

enum occUserDataType
{
    OCC_FULL_ELOG_TYPE              =   0x0000,                 // complete error log data

};

enum occUserDataVersion
{
    OCC_FULL_ELOG_TYPE_VER1         =   0x0001,                 // complete error log data ver 1
};


//*************************************************************************
// Structures
//*************************************************************************

//*************************************************************************
// Globals
//*************************************************************************

//*************************************************************************
// Function Prototypes
//*************************************************************************

//*************************************************************************
// Functions
//*************************************************************************

#endif /* #ifndef _OCC_SERVICE_CODES_H_ */
