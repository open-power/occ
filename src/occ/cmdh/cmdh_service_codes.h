
/**
 * @file cmdh_service_codes.h
 * @brief Error codes for cmdh component.
 *
 *
*/
/**
 *      @page ChangeLogs Change Logs
 *      @section _cmdh_service_codes_h cmdh_service_codes.h
 *      @verbatim
 *
 *  Flag     Def/Fea    Userid    Date        Description
 *  -------- ---------- --------  ---------   ----------------------------------
 *  @th022              thallet   10/03/2012  Added Generic Module ID
 *  @at013   878755     alvinwan  04/17/2013  OCC power capping implementation
 *  @th033   878894     thallet   04/17/2013  Mnfg AVP Support
 *  @th032              thallet   04/26/2013  Tuleta Mnfg Stage 2.5 Support
 *  @at014   882077     alvinwan  05/09/2013  Support APSS and System Config data from TMGT
 *  @jh004   889884     joshych   07/24/2013  Support CPM param and updated frequency packet
 *  @gs010   899888     gjsilva   09/24/2013  Process data format 0x13 from TMGT
 *  @rt001   902613     tapiar    10/14/2013  Process data format 0x11 from TMGT
 *  @gs015   905166     gjsilva   11/04/2013  Full support for IPS function
 *  @fk002   905632     fmkassem  11/05/2013  Remove CriticalPathMonitor code
 *  @jh009   908383    joshych   12/04/2013  Generate error logs when required for Reset Prep command
 *  @gm020   912611     milesg    01/21/2014  Add special wakeup for scom'ing core chiplets
 *
 *  @endverbatim
 */
#ifndef CMDH_SERVICE_CODES_H
#define CMDH_SERVICE_CODES_H
//*************************************************************************
// Includes
//*************************************************************************
#include <comp_ids.h>

//*************************************************************************
// Defines/Enums
//*************************************************************************
enum occCmdhModuleId
{
    DATA_STORE_GENERIC_DATA             =  CMDH_COMP_ID | 0x00,
    DATA_STORE_FREQ_DATA                =  CMDH_COMP_ID | 0x01,
    DATA_STORE_PCAP_DATA                =  CMDH_COMP_ID | 0x02, // @at013a
    CMDH_FSI2HOST_MBOX_UNAVAIL          =  CMDH_COMP_ID | 0x03, // @th033
    CMDH_GENERIC_CMD_FAILURE            =  CMDH_COMP_ID | 0x04, // @th032
    DATA_STORE_SYS_DATA                 =  CMDH_COMP_ID | 0x05, // @at014a
    DATA_STORE_APSS_DATA                =  CMDH_COMP_ID | 0x06, // @at014a
    CMDH_FSP_FSI2HOST_MBOX_WAIT4FREE    =  CMDH_COMP_ID | 0x07, // @gm020
    DATA_GET_THRM_THRESHOLDS            =  CMDH_COMP_ID | 0x08, // @gs010a
    DATA_STORE_IPS_DATA                 =  CMDH_COMP_ID | 0x09, // @rt001a
    DATA_GET_IPS_DATA                   =  CMDH_COMP_ID | 0x0A, // @gs015a
    DATA_GET_RESET_PREP_ERRL            =  CMDH_COMP_ID | 0x0B, // @jh009a 
};
#endif
