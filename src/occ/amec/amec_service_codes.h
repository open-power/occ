/**
 * @file amec_service_codes.h
 * @brief Error codes for amec component.
 *
 *
*/
/**
 *      @page ChangeLogs Change Logs
 *      @section _amec_service_codes_h amec_service_codes.h
 *      @verbatim
 *
 *  Flag     Def/Fea    Userid    Date      Description
 *  -------- ---------- --------  --------  -----------------------------------
 *   @th00a             thallet   02/03/2012  Created
 *   @pb00E             pbavari   03/11/2012  Added correct include file
 *   @nh001             neilhsu   05/23/2012  Add missing error log tags 
 *   @at008             alvinwan  08/09/2012  Support AME Pass Thru command from TMGT
 *   @fk001  879727     fmkassem  04/16/2013  PCAP support. 
 *   @db001  897459     deepthib  08/20/2013  pcap mismatch/under pcap support
 *   @rt001  897459     tapiar    08/20/2013  slave performance check support
 *   @gs012  903325     gjsilva   10/18/2013  Log Processor OT errors
 *   @gm013  907548     milesg    11/22/2013  Memory therm monitoring support
 *   @gs020  909320     gjsilva   12/12/2013  Support for VR_FAN thermal control
 *   @gs021  909855     gjsilva   12/18/2013  Support for processor OT condition
 *   @gs023  912003     gjsilva   01/16/2014  Generate VRHOT signal and control loop
 *
 *  @endverbatim
 */

#ifndef _AMEC_SERVICE_CODES_H_
#define _AMEC_SERVICE_CODES_H_
/*----------------------------------------------------------------------------*/
/* Includes                                                                   */
/*----------------------------------------------------------------------------*/
#include <comp_ids.h>

/*----------------------------------------------------------------------------*/
/* Constants                                                                  */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Globals                                                                    */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Defines                                                                    */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Typedef / Enum                                                             */
/*----------------------------------------------------------------------------*/
enum occAmecModuleId
{
    AMEC_INITIALIZE_FW_SENSORS       =    AMEC_COMP_ID | 0x00,
    AMEC_UPDATE_FW_SENSORS           =    AMEC_COMP_ID | 0x01,      // @nh001c
    AMEC_VECTORIZE_FW_SENSORS        =    AMEC_COMP_ID | 0x02,      // @nh001a
    AMEC_AMESTER_INTERFACE           =    AMEC_COMP_ID | 0x03,      // @at008a
    AMEC_PCAP_CONN_OC_CONTROLLER     =    AMEC_COMP_ID | 0x04,      // @fk001a
    AMEC_MST_CHECK_PCAPS_MATCH       =    AMEC_COMP_ID | 0x05,      // @db001a
    AMEC_MST_CHECK_UNDER_PCAP  	     = 	  AMEC_COMP_ID | 0x06,      // @db001a
    AMEC_SLAVE_CHECK_PERFORMANCE     =    AMEC_COMP_ID | 0x07,      // @rt001a
    AMEC_HEALTH_CHECK_PROC_TEMP      =    AMEC_COMP_ID | 0x08,      // @gs012a
    AMEC_HEALTH_CHECK_DIMM_TEMP      =    AMEC_COMP_ID | 0x09,
    AMEC_HEALTH_CHECK_CENT_TEMP      =    AMEC_COMP_ID | 0x10,
    AMEC_HEALTH_CHECK_DIMM_TIMEOUT   =    AMEC_COMP_ID | 0x11,
    AMEC_HEALTH_CHECK_CENT_TIMEOUT   =    AMEC_COMP_ID | 0x12,
    AMEC_HEALTH_CHECK_VRFAN_TIMEOUT  =    AMEC_COMP_ID | 0x13,      // @gs020a
    AMEC_HEALTH_CHECK_PROC_TIMEOUT   =    AMEC_COMP_ID | 0x14,      // @gs021a
    AMEC_HEALTH_CHECK_PROC_VRHOT     =    AMEC_COMP_ID | 0x15,      // @gs023a
};

/*----------------------------------------------------------------------------*/
/* Function Prototypes                                                        */
/*----------------------------------------------------------------------------*/

#endif /* #ifndef _AMEC_SERVICE_CODES_H_ */

