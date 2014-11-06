/**
 *      @page ChangeLogs Change Logs
 *      @section _amec_controller_centaur_h amec_controller_centaur.h
 *      @verbatim
 *      
 *  Flag     Def/Fea    Userid    Date      Description
 *  -------- ---------- --------  --------  --------------------------------------
 *  @mw582              mware     11/20/2013  New file
 *
 *  @endverbatim
 */

#ifndef _AMEC_ANALYTICS_H
#define _AMEC_ANALYTICS_H

/*----------------------------------------------------------------------------*/
/* Includes                                                                   */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Defines/Constants                                                          */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Function Declarations                                                      */
/*----------------------------------------------------------------------------*/

// Function that enables stream buffer recording
void amec_analytics_sb_recording(void);

// Function that is called by AMEC State Machine for analytics
void amec_analytics_main(void);

#endif // _AMEC_ANALYTICS_H
