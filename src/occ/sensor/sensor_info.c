/******************************************************************************
// @file sensor_info.c
// @brief Common sensor List for product applets
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section sensor_info.c SENSOR_INFO.c
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *   @th005             thallet   11/15/2011  created
 *   @th00a             thallet   02/03/2012  Worst case FW timings in AMEC Sensors 
 *   @rc003             rickylie  02/03/2012  Verify & Clean Up OCC Headers & Comments
 *   @th00b             thallet   02/28/12    Fix frequency on 3 sensors           
 *   @at003             alvinwan  03/19/2012  Move sensor_info_t to sensor.h
 *   @ly001  853751     lychen    09/17/2012  Support DPS algorithm
 *   @th026  865074     thallet   12/21/2012  Updated Centaur sensors
 *   @th032             thallet   04/26/2013  Tuleta HW Bringup Changes
 *   @gm002  885429     milesg    05/30/2013  Updated type and location values
 *   @at016  891144     alvinwan  06/10/2013  OCC Power Cap Testing
 *   @gs010  899888     gjsilva   09/24/2013  Fix scale factor for UTIL sensors
 *   @gm012  905097     milesg    10/31/2013  Change core sensor strings to use hw numbering
 *   @gs015  905166     gjsilva   11/04/2013  Full support for IPS function
 *   @fk002  905632     fmkassem  11/05/2013  Remove CriticalPathMonitor code
 *   @gm013  907548     milesg    11/22/2013  Memory therm monitoring support
 *   @mw633             mware     02/08/2014  Correct scaling on per core memory read and write bandwidth sensors.
 *   @gs026  915840     gjsilva   02/13/2014  Support for Nvidia GPU power measurement
 *   @mw655  918066     mware     03/11/2014  2x higher scaling on per chip memory read/write bandwidth and core memory bandwidth sensors.
 *   @mw671             mware     05/06/2014  Removed 2x higher scaling on per chip memory read/write bandwidth and per core.
 *   @mw682  933716     mware     07/28/2014  Renamed MCPIFD to be NOTBZE and MCPIFI to be NOTFIN.
 *
 *  @endverbatim
 *
 *///*************************************************************************/
 
#ifndef APPLET_BUILD
#warning "This file is only in OCC App Source Tree for convenience of updating sensors in one place."
#error   "This file can only be included and built into an applet due to large table size"
#endif

//*************************************************************************
// Includes
//*************************************************************************
#include <occ_common.h>         // STATIC_ASSERT macro
#include <sensor.h>         // For Sensor defines

//*************************************************************************
// Structures
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************
#define AMEEFP_2MS_IN_HZ    AMEFP(5,2) //  500 Hz
#define AMEEFP_250US_IN_HZ  AMEFP(4,3) // 4000 Hz
#define AMEEFP_3S_IN_HZ     AMEFP(333,-3) // 0.333 Hz
#define AMEFP_SCALE_0_16384 AMEFP(610352,-8) // scalar so that digital 16384=100% // @ly001a

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Macros
//*************************************************************************

// This will get the string when given the GSID
#define SENSOR_GSID_TO_STRING(gsid)  G_sensor_list[gsid].name;

// This will define the fields of the "sensor" member of the sensor_info_t
#define SENSOR_VALUES(units, type, location, number, frequency, scaleFactor) \
  .sensor = { units, type, location, number, frequency, scaleFactor },}

// This will put a single sensor entry into the sensor list table
#define SENSOR_INFO_T_ENTRY(sensor_name, units, type, location, number, frequency, scaleFactor)  \
  [sensor_name] = {.name = #sensor_name,   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}

// This will paste a number onto a sensor name base to create the full enum
// representation of the sensor name
#define SENSOR_W_NUM(sensor_name, num) sensor_name##num

// This will paste a number onto a sensor name base to create the full enum
// representation of the sensor name
#define SENSOR_W_CENTAUR_NUM_HELPER(sensor_name, memc,centL,cent,ppL,pp) sensor_name##memc##centL##cent##ppL##pp
#define SENSOR_W_CENTAUR_NUM(sensor_name, memc,cent,pp) SENSOR_W_CENTAUR_NUM_HELPER(sensor_name,memc,C,cent,P,pp)

// This will stringify the enum so to create the sensor name.  This will help
// save keystrokes, as well as reduce typos & copy paste errors.
#define SENSOR_STRING(sensor_name) #sensor_name

// This will stringify the enum so to create the sensor name.  This will help
// save keystrokes, as well as reduce typos & copy paste errors.
#define CENTAUR_SENSOR_STRING_HELPER(sensor_name, memc,centL,cent,ppL,pp) SENSOR_STRING(sensor_name##memc##centL##cent##ppL##pp)
#define CENTAUR_SENSOR_STRING(sensor_name,memc,cent,pp)  CENTAUR_SENSOR_STRING_HELPER(sensor_name, memc,C,cent,P,pp)

// This will create a set of 12 sensor entries into the sensor list table.
// (one for each core...)  The base name of the sensor enum must be passed
// and this macro will take care of the paste & stringify operations.
#define SENS_CORE_ENTRY_SET(sensor_name, units, type, location, number, frequency, scaleFactor)  \
  [SENSOR_W_NUM(sensor_name,0)] = {.name = SENSOR_STRING(sensor_name ## 1),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,1)] = {.name = SENSOR_STRING(sensor_name ## 2),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,2)] = {.name = SENSOR_STRING(sensor_name ## 3),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,3)] = {.name = SENSOR_STRING(sensor_name ## 4),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,4)] = {.name = SENSOR_STRING(sensor_name ## 5),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,5)] = {.name = SENSOR_STRING(sensor_name ## 6),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,6)] = {.name = SENSOR_STRING(sensor_name ## 9),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,7)] = {.name = SENSOR_STRING(sensor_name ## 10),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,8)] = {.name = SENSOR_STRING(sensor_name ## 11),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,9)] = {.name = SENSOR_STRING(sensor_name ## 12),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,10)] = {.name = SENSOR_STRING(sensor_name ## 13),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,11)] = {.name = SENSOR_STRING(sensor_name ## 14),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },} 

// This will create a set of 8 sensor entries into the sensor list table.
// (one for each memc...)  The base name of the sensor enum must be passed
// and this macro will take care of the paste & stringify operations.
#define SENS_MEMC_ENTRY_SET(sensor_name, units, type, location, number, frequency, scaleFactor)  \
  [SENSOR_W_NUM(sensor_name,0)] = {.name = SENSOR_STRING(sensor_name ## 0),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,1)] = {.name = SENSOR_STRING(sensor_name ## 1),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,2)] = {.name = SENSOR_STRING(sensor_name ## 2),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,3)] = {.name = SENSOR_STRING(sensor_name ## 3),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,4)] = {.name = SENSOR_STRING(sensor_name ## 4),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,5)] = {.name = SENSOR_STRING(sensor_name ## 5),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,6)] = {.name = SENSOR_STRING(sensor_name ## 6),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,7)] = {.name = SENSOR_STRING(sensor_name ## 7),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}


// This will create a set of 16 sensor entries into the sensor list table.
// (one for each centaur...)  The base name of the sensor enum must be passed
// and this macro will take care of the paste & stringify operations.
#define SEN_CENTR_ENTRY_SET(sensor_name, units, type, location, number, frequency, scaleFactor)  \
  [SENSOR_W_CENTAUR_NUM(sensor_name,0,0,0)] = {.name = CENTAUR_SENSOR_STRING(sensor_name,0,0,0),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_CENTAUR_NUM(sensor_name,0,0,1)] = {.name = CENTAUR_SENSOR_STRING(sensor_name,0,0,1),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_CENTAUR_NUM(sensor_name,1,0,0)] = {.name = CENTAUR_SENSOR_STRING(sensor_name,1,0,0),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_CENTAUR_NUM(sensor_name,1,0,1)] = {.name = CENTAUR_SENSOR_STRING(sensor_name,1,0,1),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_CENTAUR_NUM(sensor_name,2,0,0)] = {.name = CENTAUR_SENSOR_STRING(sensor_name,2,0,0),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_CENTAUR_NUM(sensor_name,2,0,1)] = {.name = CENTAUR_SENSOR_STRING(sensor_name,2,0,1),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_CENTAUR_NUM(sensor_name,3,0,0)] = {.name = CENTAUR_SENSOR_STRING(sensor_name,3,0,0),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_CENTAUR_NUM(sensor_name,3,0,1)] = {.name = CENTAUR_SENSOR_STRING(sensor_name,3,0,1),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_CENTAUR_NUM(sensor_name,4,0,0)] = {.name = CENTAUR_SENSOR_STRING(sensor_name,4,0,0),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_CENTAUR_NUM(sensor_name,4,0,1)] = {.name = CENTAUR_SENSOR_STRING(sensor_name,4,0,1),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_CENTAUR_NUM(sensor_name,5,0,0)] = {.name = CENTAUR_SENSOR_STRING(sensor_name,5,0,0),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_CENTAUR_NUM(sensor_name,5,0,1)] = {.name = CENTAUR_SENSOR_STRING(sensor_name,5,0,1),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_CENTAUR_NUM(sensor_name,6,0,0)] = {.name = CENTAUR_SENSOR_STRING(sensor_name,6,0,0),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_CENTAUR_NUM(sensor_name,6,0,1)] = {.name = CENTAUR_SENSOR_STRING(sensor_name,6,0,1),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_CENTAUR_NUM(sensor_name,7,0,0)] = {.name = CENTAUR_SENSOR_STRING(sensor_name,7,0,0),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_CENTAUR_NUM(sensor_name,7,0,1)] = {.name = CENTAUR_SENSOR_STRING(sensor_name,7,0,1),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}


// This table takes care of the ordering of the sensors (by GSID) and all parameters needed for AMEC or AMESTER.  The order
// that is in the table below doesn't matter because we use designated initializers.
// If anything more than the barebones sensor_t is need, an applet will need to be called in order to gather that data.
// For refernce:
//    AMEFP(1, 3);       // 1000:1 scale factor
//    AMEFP(1, 0);       // 1:1 scale factor
//    AMEFP(1,-1);       // 1:0.1 scale factor
//    AMEFP(1,-3);       // 1:0.001 scale factor
//
const sensor_info_t G_sensor_info[]   =
{
  /* ==FirmwareSensors==    NameString             Units                      Type              Location             Number                Freq      ScaleFactor */
  SENSOR_INFO_T_ENTRY(       AMEintdur,  "us\0",    AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(       AMESSdur0,  "us\0",    AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(       AMESSdur1,  "us\0",    AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(       AMESSdur2,  "us\0",    AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(       AMESSdur3,  "us\0",    AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(       AMESSdur4,  "us\0",    AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(       AMESSdur5,  "us\0",    AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(       AMESSdur6,  "us\0",    AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(       AMESSdur7,  "us\0",    AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(     PROBE250US0, "n/a\0", AMEC_SENSOR_TYPE_GENERIC,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(     PROBE250US1, "n/a\0", AMEC_SENSOR_TYPE_GENERIC,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(     PROBE250US2, "n/a\0", AMEC_SENSOR_TYPE_GENERIC,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(     PROBE250US3, "n/a\0", AMEC_SENSOR_TYPE_GENERIC,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(     PROBE250US4, "n/a\0", AMEC_SENSOR_TYPE_GENERIC,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(     PROBE250US5, "n/a\0", AMEC_SENSOR_TYPE_GENERIC,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(     PROBE250US6, "n/a\0", AMEC_SENSOR_TYPE_GENERIC,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(     PROBE250US7, "n/a\0", AMEC_SENSOR_TYPE_GENERIC,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(     GPEtickdur0,  "us\0",    AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(     GPEtickdur1,  "us\0",    AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(      RTLtickdur,  "us\0",    AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),

  /* ==SystemSensors==      NameString  Units                      Type              Location             Number                Freq      ScaleFactor   */
  SENSOR_INFO_T_ENTRY(     TEMPAMBIENT,   "C\0",    AMEC_SENSOR_TYPE_TEMP,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(        ALTITUDE,   "m\0",    AMEC_SENSOR_TYPE_TEMP,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(        PWR250US,   "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ), // @at016c
  SENSOR_INFO_T_ENTRY(     PWR250USFAN,   "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ), // @at016c
  SENSOR_INFO_T_ENTRY(      PWR250USIO,   "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ), // @at016c
  SENSOR_INFO_T_ENTRY(   PWR250USSTORE,   "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ), // @at016c
  SENSOR_INFO_T_ENTRY(     PWR250USGPU,   "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(     FANSPEEDAVG, "RPM\0",    AMEC_SENSOR_TYPE_TEMP,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),

  /* ==ChipSensors==        NameString  Units                      Type              Location             Number                Freq      ScaleFactor   */
  SENSOR_INFO_T_ENTRY(       TODclock0,  "us\0",    AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_ALL, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP( 16, 0)  ),      // @mw586
  SENSOR_INFO_T_ENTRY(       TODclock1,  "sec\0",   AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_ALL, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP( 1048576,-6)  ), // @mw586
  SENSOR_INFO_T_ENTRY(       TODclock2,  "day\0",   AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_ALL, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP( 795364,-6)  ), // @mw586

  /* ==ProcSensors==        NameString  Units                      Type              Location             Number                Freq      ScaleFactor   */
  SENSOR_INFO_T_ENTRY(      FREQA2MSP0, "MHz\0",    AMEC_SENSOR_TYPE_FREQ, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(        IPS2MSP0, "MIP\0",    AMEC_SENSOR_TYPE_PERF, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ), // @mw584
  SENSOR_INFO_T_ENTRY(      MEMSP2MSP0,   "%\0",    AMEC_SENSOR_TYPE_TIME, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(      PWR250USP0,   "W\0",   AMEC_SENSOR_TYPE_POWER, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ), // @at016c
  SENSOR_INFO_T_ENTRY(    PWR250USVDD0,   "W\0",   AMEC_SENSOR_TYPE_POWER, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ), // @at016c
  SENSOR_INFO_T_ENTRY(    CUR250USVDD0,   "A\0", AMEC_SENSOR_TYPE_CURRENT, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1,-2)  ), // @mw586
  SENSOR_INFO_T_ENTRY(    PWR250USVCS0,   "W\0",   AMEC_SENSOR_TYPE_POWER, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ), // @at016c
  SENSOR_INFO_T_ENTRY(    PWR250USMEM0,   "W\0",   AMEC_SENSOR_TYPE_POWER, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ), // @at016c
  SENSOR_INFO_T_ENTRY(   SLEEPCNT2MSP0,   "#\0",    AMEC_SENSOR_TYPE_PERF, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(    WINKCNT2MSP0,   "#\0",    AMEC_SENSOR_TYPE_PERF, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(       SP250USP0,   "%\0",    AMEC_SENSOR_TYPE_FREQ, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(       TEMP2MSP0,   "C\0",    AMEC_SENSOR_TYPE_TEMP, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(   TEMP2MSP0PEAK,   "C\0",    AMEC_SENSOR_TYPE_TEMP, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(       UTIL2MSP0,   "%\0",    AMEC_SENSOR_TYPE_UTIL, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1,-2)  ),
  SENSOR_INFO_T_ENTRY(  VRFAN250USPROC, "pin\0",    AMEC_SENSOR_TYPE_TEMP,  AMEC_SENSOR_LOC_VRM, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(  VRHOT250USPROC, "pin\0",    AMEC_SENSOR_TYPE_TEMP,  AMEC_SENSOR_LOC_VRM, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),

  /* ==ReguSensors==        NameString  Units                      Type              Location             Number                Freq      ScaleFactor   */
  SENSOR_INFO_T_ENTRY(  UVOLT250USP0V0,  "mV\0", AMEC_SENSOR_TYPE_VOLTAGE,  AMEC_SENSOR_LOC_VRM, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, -1)  ), //@cl003
  SENSOR_INFO_T_ENTRY(  UVOLT250USP0V1,  "mV\0", AMEC_SENSOR_TYPE_VOLTAGE,  AMEC_SENSOR_LOC_VRM, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, -1)  ), //@cl003
  SENSOR_INFO_T_ENTRY(   VOLT250USP0V0,  "mV\0", AMEC_SENSOR_TYPE_VOLTAGE,  AMEC_SENSOR_LOC_VRM, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, -1)  ), //@cl003
  SENSOR_INFO_T_ENTRY(   VOLT250USP0V1,  "mV\0", AMEC_SENSOR_TYPE_VOLTAGE,  AMEC_SENSOR_LOC_VRM, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, -1)  ), //@cl003

  /* ==CoreSensors==        NameString  Units                      Type              Location             Number                Freq      ScaleFactor   */
//  SENS_CORE_ENTRY_SET(      CPM2MSP0C , "bit\0",     AMEC_SENSOR_TYPE_CPM, AMEC_SENSOR_LOC_CORE, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ), //CPM - Commented out as requested by Malcolm
  SENS_CORE_ENTRY_SET(   FREQ250USP0C , "MHz\0",    AMEC_SENSOR_TYPE_FREQ, AMEC_SENSOR_LOC_CORE, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENS_CORE_ENTRY_SET(    FREQA2MSP0C , "MHz\0",    AMEC_SENSOR_TYPE_FREQ, AMEC_SENSOR_LOC_CORE, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ), // @mw584
  SENS_CORE_ENTRY_SET(      IPS2MSP0C , "MIP\0",    AMEC_SENSOR_TYPE_PERF, AMEC_SENSOR_LOC_CORE, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ), // @mw584
  SENS_CORE_ENTRY_SET(   NOTBZE2MSP0C , "cyc\0",    AMEC_SENSOR_TYPE_PERF, AMEC_SENSOR_LOC_CORE, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ), // @mw682
  SENS_CORE_ENTRY_SET(   NOTFIN2MSP0C , "cyc\0",    AMEC_SENSOR_TYPE_PERF, AMEC_SENSOR_LOC_CORE, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ), // @mw682
  SENS_CORE_ENTRY_SET(    SPURR2MSP0C ,   "%\0",    AMEC_SENSOR_TYPE_PERF, AMEC_SENSOR_LOC_CORE, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENS_CORE_ENTRY_SET(     TEMP2MSP0C ,   "C\0",    AMEC_SENSOR_TYPE_TEMP, AMEC_SENSOR_LOC_CORE, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENS_CORE_ENTRY_SET(     UTIL2MSP0C ,   "%\0",    AMEC_SENSOR_TYPE_UTIL, AMEC_SENSOR_LOC_CORE, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1,-2)  ),
  SENS_CORE_ENTRY_SET(     NUTIL3SP0C ,   "%\0",    AMEC_SENSOR_TYPE_UTIL, AMEC_SENSOR_LOC_CORE, AMEC_SENSOR_NONUM,    AMEEFP_3S_IN_HZ,   AMEFP(  1,-2)  ),
  SENS_CORE_ENTRY_SET(     MSTL2MSP0C , "cpi\0",    AMEC_SENSOR_TYPE_PERF, AMEC_SENSOR_LOC_CORE, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENS_CORE_ENTRY_SET(      CMT2MSP0C ,   "%\0",    AMEC_SENSOR_TYPE_PERF, AMEC_SENSOR_LOC_CORE, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENS_CORE_ENTRY_SET(     CMBW2MSP0C , "GBs\0",    AMEC_SENSOR_TYPE_PERF, AMEC_SENSOR_LOC_CORE, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  128, -5)  ),  // @mw671
  SENS_CORE_ENTRY_SET(        PPICP0C ,   "%\0",    AMEC_SENSOR_TYPE_PERF, AMEC_SENSOR_LOC_CORE, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENS_CORE_ENTRY_SET(  PWRPX250USP0C ,   "W\0",   AMEC_SENSOR_TYPE_POWER, AMEC_SENSOR_LOC_CORE, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),

  /* ==MemSensors==         NameString  Units                      Type              Location             Number                Freq      ScaleFactor   */
  SENSOR_INFO_T_ENTRY(   VRFAN250USMEM, "pin\0",    AMEC_SENSOR_TYPE_TEMP,  AMEC_SENSOR_LOC_VRM, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(   VRHOT250USMEM, "pin\0",    AMEC_SENSOR_TYPE_TEMP,  AMEC_SENSOR_LOC_VRM, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENS_MEMC_ENTRY_SET(       MRD2MSP0M, "GBs\0",    AMEC_SENSOR_TYPE_PERF,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  128, -5)  ),  // @mw671
  SENS_MEMC_ENTRY_SET(       MWR2MSP0M, "GBs\0",    AMEC_SENSOR_TYPE_PERF,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  128, -5)  ),  // @mw671
  SENS_MEMC_ENTRY_SET(      MIRC2MSP0M, "eps\0",    AMEC_SENSOR_TYPE_PERF,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENS_MEMC_ENTRY_SET(         MLP2P0M, "eps\0",    AMEC_SENSOR_TYPE_PERF,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENS_MEMC_ENTRY_SET(   TEMPDIMMAXP0M,   "C\0",    AMEC_SENSOR_TYPE_TEMP,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENS_MEMC_ENTRY_SET(    LOCDIMMAXP0M, "loc\0",    AMEC_SENSOR_TYPE_TEMP,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),

  /* ==CentaurSensors==     NameString  Units                      Type              Location             Number                Freq      ScaleFactor   */
  SEN_CENTR_ENTRY_SET(       MAC2MSP0M, "rps\0",    AMEC_SENSOR_TYPE_PERF,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SEN_CENTR_ENTRY_SET(       MPU2MSP0M, "rps\0",    AMEC_SENSOR_TYPE_PERF,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SEN_CENTR_ENTRY_SET(      MIRB2MSP0M, "eps\0",    AMEC_SENSOR_TYPE_PERF,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SEN_CENTR_ENTRY_SET(      MIRL2MSP0M, "eps\0",    AMEC_SENSOR_TYPE_PERF,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SEN_CENTR_ENTRY_SET(      MIRM2MSP0M, "eps\0",    AMEC_SENSOR_TYPE_PERF,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SEN_CENTR_ENTRY_SET(      MIRH2MSP0M, "eps\0",    AMEC_SENSOR_TYPE_PERF,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SEN_CENTR_ENTRY_SET(       MTS2MSP0M, "cnt\0",    AMEC_SENSOR_TYPE_PERF,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SEN_CENTR_ENTRY_SET(      MEMSP2MSPM,   "%\0",    AMEC_SENSOR_TYPE_PERF,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SEN_CENTR_ENTRY_SET(      M4RD2MSP0M, "GBs\0",    AMEC_SENSOR_TYPE_PERF,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  128, -5)  ),  // @mw671
  SEN_CENTR_ENTRY_SET(      M4WR2MSP0M, "GBs\0",    AMEC_SENSOR_TYPE_PERF,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  128, -5)  ),  // @mw671

  
  /* ==MemSummarySensors==  NameString  Units                      Type              Location             Number                Freq      ScaleFactor   */
  SENSOR_INFO_T_ENTRY(     TEMP2MSCENT,   "C\0",    AMEC_SENSOR_TYPE_TEMP,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(     TEMP2MSDIMM,   "C\0",    AMEC_SENSOR_TYPE_TEMP,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(        MEMSP2MS,   "%\0",    AMEC_SENSOR_TYPE_PERF,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  
  // @ly001a - start
  /* ==PartSummarySensors==  NameString  Units                      Type              Location             Number                Freq      ScaleFactor   */
  SENSOR_INFO_T_ENTRY(  UTIL2MSSLCG000,   "%\0",    AMEC_SENSOR_TYPE_UTIL,  AMEC_SENSOR_LOC_LPAR, AMEC_SENSOR_NONUM,  AMEEFP_2MS_IN_HZ,  AMEFP_SCALE_0_16384),
  SENSOR_INFO_T_ENTRY(  UTIL2MSSLCG001,   "%\0",    AMEC_SENSOR_TYPE_UTIL,  AMEC_SENSOR_LOC_LPAR, AMEC_SENSOR_NONUM,  AMEEFP_2MS_IN_HZ,  AMEFP_SCALE_0_16384),
  SENSOR_INFO_T_ENTRY(  UTIL2MSSLCG002,   "%\0",    AMEC_SENSOR_TYPE_UTIL,  AMEC_SENSOR_LOC_LPAR, AMEC_SENSOR_NONUM,  AMEEFP_2MS_IN_HZ,  AMEFP_SCALE_0_16384),
  SENSOR_INFO_T_ENTRY(  UTIL2MSSLCG003,   "%\0",    AMEC_SENSOR_TYPE_UTIL,  AMEC_SENSOR_LOC_LPAR, AMEC_SENSOR_NONUM,  AMEEFP_2MS_IN_HZ,  AMEFP_SCALE_0_16384),
  SENSOR_INFO_T_ENTRY(  UTIL2MSSLCG004,   "%\0",    AMEC_SENSOR_TYPE_UTIL,  AMEC_SENSOR_LOC_LPAR, AMEC_SENSOR_NONUM,  AMEEFP_2MS_IN_HZ,  AMEFP_SCALE_0_16384),
  SENSOR_INFO_T_ENTRY(  UTIL2MSSLCG005,   "%\0",    AMEC_SENSOR_TYPE_UTIL,  AMEC_SENSOR_LOC_LPAR, AMEC_SENSOR_NONUM,  AMEEFP_2MS_IN_HZ,  AMEFP_SCALE_0_16384),
  SENSOR_INFO_T_ENTRY(  UTIL2MSSLCG006,   "%\0",    AMEC_SENSOR_TYPE_UTIL,  AMEC_SENSOR_LOC_LPAR, AMEC_SENSOR_NONUM,  AMEEFP_2MS_IN_HZ,  AMEFP_SCALE_0_16384),
  SENSOR_INFO_T_ENTRY(  UTIL2MSSLCG007,   "%\0",    AMEC_SENSOR_TYPE_UTIL,  AMEC_SENSOR_LOC_LPAR, AMEC_SENSOR_NONUM,  AMEEFP_2MS_IN_HZ,  AMEFP_SCALE_0_16384),
  SENSOR_INFO_T_ENTRY(  UTIL2MSSLCG008,   "%\0",    AMEC_SENSOR_TYPE_UTIL,  AMEC_SENSOR_LOC_LPAR, AMEC_SENSOR_NONUM,  AMEEFP_2MS_IN_HZ,  AMEFP_SCALE_0_16384),
  SENSOR_INFO_T_ENTRY(  UTIL2MSSLCG009,   "%\0",    AMEC_SENSOR_TYPE_UTIL,  AMEC_SENSOR_LOC_LPAR, AMEC_SENSOR_NONUM,  AMEEFP_2MS_IN_HZ,  AMEFP_SCALE_0_16384),
  SENSOR_INFO_T_ENTRY(  UTIL2MSSLCG010,   "%\0",    AMEC_SENSOR_TYPE_UTIL,  AMEC_SENSOR_LOC_LPAR, AMEC_SENSOR_NONUM,  AMEEFP_2MS_IN_HZ,  AMEFP_SCALE_0_16384),
  SENSOR_INFO_T_ENTRY(  UTIL2MSSLCG011,   "%\0",    AMEC_SENSOR_TYPE_UTIL,  AMEC_SENSOR_LOC_LPAR, AMEC_SENSOR_NONUM,  AMEEFP_2MS_IN_HZ,  AMEFP_SCALE_0_16384),
  // @ly001a - end
};

// Cause a compile error if we don't have all the sensors in the enum in the initialization list.  
STATIC_ASSERT(   (NUMBER_OF_SENSORS_IN_LIST != (sizeof(G_sensor_info)/sizeof(sensor_info_t)))  );
STATIC_ASSERT(   (MAX_AMEC_SENSORS < (sizeof(G_sensor_info)/sizeof(sensor_info_t)))   );

//*************************************************************************
// Globals
//*************************************************************************

//*************************************************************************
// Function Prototypes
//*************************************************************************

//*************************************************************************
// Functions
//*************************************************************************
