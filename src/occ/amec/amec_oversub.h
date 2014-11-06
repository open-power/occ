/******************************************************************************
// @file amec_oversub.c
// @brief Over-subscription
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section _amec_oversub_h amec_oversub.h
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *   @at010  859992     alvinwan  11/07/2012  Added oversubscription feature
 *   @gs004  883829     gjsilva   05/21/2013  Emulate oversubscription macro
 *   @fk001  879727     fmkassem  04/16/2013  PCAP support.  
 *   @at020  908666     alvinwan  12/16/2013  Oversubscription Error Handling
 *   @gs023  912003     gjsilva   01/16/2014  Generate VRHOT signal and control loop
 *
 *  @endverbatim
 *
 *///*************************************************************************/

#ifndef _AMEC_OVERSUB_H
#define _AMEC_OVERSUB_H
/*----------------------------------------------------------------------------*/
/* Includes                                                                   */
/*----------------------------------------------------------------------------*/
#include <occ_sys_config.h>     // @at010a Added for sys config access
#include <dcom.h>

/*----------------------------------------------------------------------------*/
/* Constants                                                                  */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Globals                                                                    */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Defines                                                                    */
/*----------------------------------------------------------------------------*/
// @at020c
#define AMEC_INTF_GET_OVERSUBSCRIPTION() \
(G_sysConfigData.failsafe_enabled ? g_amec->oversub_status.cmeThrottlePinLive : \
                                   (g_amec->oversub_status.oversubPinLive || G_dcom_slv_inbox_rx.emulate_oversub))

#define AMEC_INTF_GET_OVERSUBSCRIPTION_EMULATION() g_amec->oversub_status.oversubPinMnfg

// @at020c
#define AMEC_INTF_GET_FAILSAFE() \
(G_sysConfigData.failsafe_enabled ? g_amec->oversub_status.oversubPinLive : 0)


/*----------------------------------------------------------------------------*/
/* Typedef / Enum                                                             */
/*----------------------------------------------------------------------------*/
typedef enum oversub_reason
{
   PENDING_OR_INVALID = 0x00,
   FANS_FULL_SPEED    = 0x01,
   FAN_ERRROR         = 0x02,
   FAN_WARNING        = 0x03,
   ITE_FAILSAFE       = 0x04,

   INDETERMINATE      = 0xFF
}oversub_reason_t;

typedef struct oversub_status
{
    // Way to emulate oversub for MNFG or Developers
    uint32_t    oversubPinMnfg      : 1;

    // Live Status of oversub Pin
    uint32_t    oversubPinLive      : 1;

    // AMEC status of oversub pin, so it doesn't
    // change mid-RTL
    uint32_t    oversubLatchAmec    : 1;

    // Used for SRC logging of performance loss,
    // need to have countdown b/c we don't get
    // APSS gpio signals as quick as we get
    // oversub.
    uint8_t    oversubReasonLatchCount;
    oversub_reason_t    oversubReason;

    // For debug, tracks time oversub last went inactive
    SsxTimebase oversubInactiveTime;

    // For debug, tracks time oversub last went active
    SsxTimebase oversubActiveTime;

    // Live status of CME throttle pin, doesn't change mid-RTL
    uint32_t cmeThrottlePinLive     :1;

    // Status of CME Throttle, doesn't get cleared until MM/TMGT
    // tells us to clear it/un-throttle.
    uint32_t cmeThrottleLatchAmec   :1;

    // Way to emulate cmeThrottle for MNFG or Developers
    uint32_t cmeThrottlePinMnfg     :1;
}oversub_status_t;


/*----------------------------------------------------------------------------*/
/* Function Prototypes                                                        */
/*----------------------------------------------------------------------------*/

/**
 *  @brief  Oversubscription ISR
 *
 *  This function
 *
 *  @return No return from this function.
 */
void amec_oversub_isr(void);


/**
 *  @brief  Oversubscription check
 *
 *  This function
 *
 *  @return No return from this function.
 */
void amec_oversub_check(void);

/**
  * @brief Oversubscription set Pmax clip
  *
  * This function will set the pmax clip register
  * 
  * @return No return from this function
  */
void amec_oversub_pmax_clip(Pstate i_pstate);

bool apss_gpio_get(uint8_t i_pin_number, uint8_t *o_pin_value);

#endif //_AMEC_OVERSUB_H
