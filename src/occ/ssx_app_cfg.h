/******************************************************************************
// @file ssx_app_cfg.h
// @brief Common application configuration for OCC
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section ssx_app_cfg.h SSX_APP_CFG.H
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *   @rc003             rickylie  02/03/2012  Verify & Clean Up OCC Headers & Comments
 *   @th043  892554     thallet   07/23/2013  Automatic Nominal/Active state change
 *   @gm010  901580     milesg    10/06/2013  Low Level FFDC support
 *   @sb002  908891     sbroyles  12/09/2013  FFDC updates
 *   @sb012  910394     sbroyles  01/10/2014  More FFDC updates
 *
 *  @endverbatim
 *
 *///*************************************************************************/
#ifndef __SSX_APP_CFG_H__
#define __SSX_APP_CFG_H__
//*************************************************************************
// Includes
//*************************************************************************

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Macros
//*************************************************************************

// @sb012 These versions of SSX_PANIC are being changed so that they exactly
// mirror each other and are exactly structured at 8 instructions only and
// make only one branch to outside code.
#ifndef __ASSEMBLER__
#ifndef SSX_PANIC
#define SSX_PANIC(code)                                             \
do {                                                                \
    barrier();                                                      \
    asm volatile ("stw  %r3, __occ_panic_save_r3@sda21(0)");        \
    asm volatile ("mflr %r3");                                      \
    asm volatile ("stw  %r4, __occ_panic_save_r4@sda21(0)");        \
    asm volatile ("lis  %%r4, %0"::"i" (code >> 16));               \
    asm volatile ("ori  %%r4, %%r4, %0"::"i" (code & 0xffff));      \
    asm volatile ("bl   __ssx_checkpoint_panic_and_save_ffdc");         \
    asm volatile ("trap");                                          \
    asm volatile (".long %0" : : "i" (code));                       \
} while (0)
#endif // SSX_PANIC
#else  /* __ASSEMBLER__ */
#ifndef SSX_PANIC
// This macro cannot be more than 8 instructions long, but it can be less than
// 8.
#define SSX_PANIC(code) _ssx_panic code
    .macro  _ssx_panic, code
    stw     %r3, __occ_panic_save_r3@sda21(0)
    mflr    %r3
    stw     %r4, __occ_panic_save_r4@sda21(0)
    lis     %r4, \code@h
    ori     %r4, %r4, \code@l
    bl      __ssx_checkpoint_panic_and_save_ffdc
    trap
    .long   \code
    .endm
#endif // SSX_PANIC
#endif /* __ASSEMBLER__ */

//*************************************************************************
// Defines/Enums
//*************************************************************************
#define INIT_SEC_NM_STR     "initSection"
#define INIT_SECTION __attribute__ ((section (INIT_SEC_NM_STR)))

#define SIMICS_ENVIRONMENT 0

/// Since OCC Firmware desires time intervals longer than 7 seconds, we will
/// change the interval to be a uint64_t instead of a uint32_t so we don't
/// hit the overflow condition
#define SSX_TIME_INTERVAL_TYPE uint64_t

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

#endif /* __SSX_APP_CFG_H__ */


