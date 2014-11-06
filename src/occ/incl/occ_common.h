/******************************************************************************
// @file occ_common.h
// @brief OCC common functions
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section occ_common_h occ_common.h
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *                      TEAM      06/16/2010  Port
 *   @pb000             pbavari   07/07/2011  Added typedef for uint32_t and
 *                                            removed TPMD_code_header
 *   @01                tapiar    08/07/2011  Moved default trace size to this file
 *   @pb003             pbavari   09/08/2011  Added MAX_OCCS and MAX_CORES define
 *   @pb007             pbavari   09/30/2011  Added HALT_WITH_FIR_SET
 *   @th00a             thallet   02/03/2012  Worst case FW timings in AMEC Sensors
 *   @rc003             rickylie  02/03/2012  Verify & Clean Up OCC Headers & Comments
 *   @th00b             thallet   02/03/2012  Added some convert macros
 *   @pb00E             pbavari   03/11/2012  File name changes
 *   @th00d             thallet   04/23/2012  Added linear window macros
 *   @ai009  865968     ailutsar  01/09/2013  OCC Error log and trace parser web tool enhancement
 *   @gm006  SW224414   milesg    09/16/2013  Reset and FFDC improvements
 *   @gm010  901580     milesg    10/06/2013  Low Level FFDC support
 *   @gm016  909061     milesg    12/10/2013  increased default trace size
 *   @sb002  908891     sbroyles  12/09/2013  FFDC updates
 *   @sb012  910394     sbroyles  01/10/2014  More FFDC updates
 *   @gm022  908890     milesg    01/23/2014  Halt OCC on OCCLFIR[38]
 *  @endverbatim
 *
 *///*************************************************************************/

#ifndef _OCC_COMMON_H
#define _OCC_COMMON_H

//*************************************************************************
// Includes
//*************************************************************************
#include <common_types.h>
#include <comp_ids.h>

//*************************************************************************
// Externs
//*************************************************************************

// From Linker Script
extern void _LINEAR_WR_WINDOW_SECTION_BASE;
// From Linker Script
extern void _LINEAR_WR_WINDOW_SECTION_SIZE;
// From Linker Script
extern void _LINEAR_RD_WINDOW_SECTION_BASE;
// From Linker Script
extern void _LINEAR_RD_WINDOW_SECTION_SIZE;

//*************************************************************************
// Macros
//*************************************************************************

/// Declare aligned data structures for Async access in a noncacheable section
///
/// These macros declare aligned data structures in a noncacheable section, with
/// the alignment that is needed by the specified device driver (or more
/// accurately, as specified by the hardware device itself.)
///
/// All buffers should be initialized - an initialization declaration using
/// these macros would look as follows:
///    Example:  DMA_BUFFER(uint8_t g_bcue_test[1024]) = {0};
///
#define PBAX_BUFFER(declaration) \
        declaration __attribute__ ((__aligned__ (8))) __attribute__ ((section (".noncacheable")))

#define OCB_BUFFER(declaration) \
        declaration __attribute__ ((__aligned__ (8))) __attribute__ ((section (".noncacheable")))

#define DMA_BUFFER(declaration) \
        declaration __attribute__ ((__aligned__ (128))) __attribute__ ((section (".noncacheable")))

#define GPE_BUFFER(declaration) \
        declaration __attribute__ ((__aligned__ (8))) __attribute__ ((section (".noncacheable")))

#define LINEAR_WINDOW_WR_BUFFER(declaration) \
        declaration __attribute__ ((section (".linear_wr")))

#define LINEAR_WINDOW_RD_BUFFER(declaration) \
        declaration __attribute__ ((section (".linear_rd")))

#define PSTATE_TABLE(declaration) \
        declaration __attribute__ ((__aligned__ (1024)))

// Pinned in linker file as address for TMGT cmds to be sent/received
#define CMDH_LINEAR_WINDOW_BASE_ADDRESS ((uint32_t) &_LINEAR_WR_WINDOW_SECTION_BASE)
#define LINEAR_WR_WINDOW_SECTION_SIZE   ((uint32_t) &_LINEAR_WR_WINDOW_SECTION_SIZE)
#define CMDH_OCC_RESPONSE_BASE_ADDRESS  ((uint32_t) &_LINEAR_RD_WINDOW_SECTION_BASE)
#define LINEAR_RD_WINDOW_SECTION_SIZE   ((uint32_t) &_LINEAR_RD_WINDOW_SECTION_SIZE)

// Conversion Macro's

// Get byte 0-1 of uint64
#define     CONVERT_UINT64_UINT16_UPPER(a) \
            ((UINT16)((a>>48) & 0xFFFF))
// Get byte 2-3 of uint64
#define     CONVERT_UINT64_UINT16_MIDUPPER(a) \
            ((UINT16)((a>>32) & 0xFFFF))
// Get byte 4-5 of uint64
#define     CONVERT_UINT64_UINT16_MIDLOWER(a) \
            ((UINT16)((a>>16) & 0xFFFF))
// Get byte 6-7 of uint64
#define     CONVERT_UINT64_UINT16_LOWER(a) \
            ((UINT16)((a>>0) & 0xFFFF))


// Get byte 0 of uint32
#define     CONVERT_UINT32_UINT8_UPPER_HIGH(a) \
            ((UINT8)((a>>24) & 0xFF))
// Get byte 1 of uint32
#define     CONVERT_UINT32_UINT8_UPPER_LOW(a) \
            ((UINT8)((a>>16) & 0xFF))
// Get byte 2 of uint32
#define     CONVERT_UINT32_UINT8_LOWER_HIGH(a) \
            ((UINT8)((a>>8) & 0xFF))
// Get byte 3 of uint32
#define     CONVERT_UINT32_UINT8_LOWER_LOW(a) \
            ((UINT8)((a>>0) & 0xFF))


// Get byte 0-1 of uint32
#define     CONVERT_UINT32_UINT16_UPPER(a) \
            ((UINT16)((a>>16) & 0xFFFF))
// Get byte 1-2 of uint32
#define     CONVERT_UINT32_UINT16_MIDDLE(a) \
            ((UINT16)((a>>8) & 0xFFFF))
// Get byte 2-3 of uint32
#define     CONVERT_UINT32_UINT16_LOWER(a) \
            ((UINT16)((a>>0) & 0xFFFF))

// Get high byte of uint16
#define     CONVERT_UINT16_UINT8_HIGH(a) \
            ((UINT8)((a>>8) & 0xFF))
// Get low byte of uint16
#define     CONVERT_UINT16_UINT8_LOW(a) \
            ((UINT8)(a & 0x00FF))


// Get high nybble of uint8
#define     CONVERT_UINT8_UINT4_HIGH(a) \
            ((UINT8)((a>>4) & 0x0F))
// Get low nybble of uint8
#define     CONVERT_UINT8_UINT4_LOW(a) \
            ((UINT8)(a & 0x0F))

// Convert a two byte uint8 to a uint16
// Always cast LSB to UINT8 to assure this also works with INT's
#define     CONVERT_UINT8_ARRAY_UINT16(a,b) \
            ((a<<8) | ((UINT8)b))

// Convert a 4 byte uint8 to a uint32
#define     CONVERT_UINT8_ARRAY_UINT32(a,b,c,d) \
            ((((UINT32)a)<<24) | (((UINT32)b)<<16) | (((UINT32)c)<<8) | (((UINT32)d)))


// Bit Operation Macros
#define SETBIT(var,bit)   ((var) |= (1<<(bit)))
#define CLEARBIT(var,bit) ((var) &= ~(1<<(bit)))

#define WORDALIGN(n) \
        ((n + 3) & ~3)



//#define UTIL_MUTEX_GET(I_MUTEX, I_TIMEOUT) ssx_semaphore_pend(I_MUTEX,I_TIMEOUT)
//#define UTIL_MUTEX_PUT(I_MUTEX) ssx_semaphore_post(I_MUTEX)

// @sb002 CHECKPOINT macros revamped a little to allow a little more reuse with
// new return codes ('Ex'h).  Note that there is a special case version of this
// code in ll_ffdc.S designed solely for writing an FFDC header in the SSX_PANIC
// path.
#define __CHECKPOINT(_flg, _ckp, _rc)                                          \
{                                                                              \
    G_fsp_msg.rsp->fields.seq = 0;                                             \
    G_fsp_msg.rsp->fields.cmd_type = 0;                                        \
    G_fsp_msg.rsp->fields.rc = _rc;                                            \
    G_fsp_msg.rsp->fields.data_length[0] = 0;                                  \
    G_fsp_msg.rsp->fields.data_length[1] = 3;                                  \
    G_fsp_msg.rsp->fields.data[0] = _flg;                                      \
    G_fsp_msg.rsp->fields.data[1] = (uint8_t)(_ckp >> 8);                      \
    G_fsp_msg.rsp->fields.data[2] = (uint8_t)_ckp;                             \
    dcache_flush_line((void *)CMDH_OCC_RESPONSE_BASE_ADDRESS);                 \
}

// This macro should only be used in the initialization path leading
// up to being able to communicate with the FSP.  After that, the
// response buffer is used for responses and must not be used for
// checkpointing unless the OCC is about to halt.
#define CHECKPOINT_INIT()                                                      \
{                                                                              \
    __CHECKPOINT(0x00, 0x0000, ERRL_RC_INIT_CHCKPNT);                          \
}

// In case we are not able to reach a state where OCC can receive
// commands from the FSP.  Place a checkpoint value in the response
// buffer with the return code of the response set to ERRL_RC_INIT_CHCKPNT.
// Then, if OCC doesn't respond to an FSP command, the FSP will see
// ERRL_RC_INIT_CHCKPNT and log the checkpoint in a special error log.
#define CHECKPOINT(_ckp)                                                       \
{                                                                              \
    __CHECKPOINT(G_fsp_msg.rsp->fields.data[0], _ckp, ERRL_RC_INIT_CHCKPNT);   \
}

// Special purpose flags to be used at programmer's discretion
#define CHECKPOINT_FLAG(_flg)  \
{                                                                              \
    G_fsp_msg.rsp->fields.data[0] |= _flg;                                     \
    __CHECKPOINT(_flg, (G_fsp_msg.rsp->fields.data[1] << 8 | G_fsp_msg.rsp->fields.data[2]), G_fsp_msg.rsp->fields.rc);\
}

// @sb012 Minor rewrite and addition of assembler version
// @sb002 A new OCC_HALT macro much like SSX_PANIC but for OCC code use.  The
// ex_code parm is placed in the exception code of the FFDC header as opposed to
// SSX_PANIC which sets the panic code.  OCC_HALT sets the panic code to 0 to
// differentiate this halt from SSX initiated halts.  This macro mirrors what
// SSX_PANIC does except it calls a different function for saving the FFDC.
// This macro will not return to the caller.  There is also an assembly version
// if needed.
#ifndef __ASSEMBLER__
#define OCC_HALT(ex_code)                                                      \
do {                                                                           \
    barrier();                                                                 \
    asm volatile ("stw  %r3, __occ_panic_save_r3@sda21(0)");                   \
    asm volatile ("mflr %r3");                                                 \
    asm volatile ("stw  %r4, __occ_panic_save_r4@sda21(0)");                   \
    asm volatile ("lis  %%r4, %0"::"i" (ex_code >> 16));                       \
    asm volatile ("ori  %%r4, %%r4, %0"::"i" (ex_code & 0xffff));              \
    asm volatile ("bl   __occ_checkpoint_panic_and_save_ffdc");                \
    asm volatile ("trap");                                                     \
    asm volatile (".long %0" : : "i" (ex_code));                               \
} while (0)
#else  /* __ASSEMBLER__ */
#define OCC_HALT(ex_code) _occ_halt ex_code
    .macro  _occ_halt, ex_code
    stw     %r3, __occ_panic_save_r3@sda21(0)
    mflr    %r3
    stw     %r4, __occ_panic_save_r4@sda21(0)
    lis     %r4, \ex_code@h
    ori     %r4, %r4, \ex_code@l
    bl      __occ_checkpoint_panic_and_save_ffdc
    trap
    .long   \ex_code
    .endm
#endif /* __ASSEMBLER__ */

//*************************************************************************
// Defines/Enums
//*************************************************************************

//Unique checkpoints
enum
{
    MAIN_STARTED                = 0x01ff,
    SSX_STARTING                = 0x0210,
    SSX_INITIALIZED             = 0x02ff,
    TRACE_INITIALIZED           = 0x0310,
    INITIALIZING_IRQS           = 0x0320,
    IRQS_INITIALIZED            = 0x032f,
    MAIN_THREAD_STARTED         = 0x03ff,
    ROLES_INITIALIZED           = 0x04ff,
    SENSORS_INITIALIZED         = 0x05ff,
    PROC_CORE_INITIALIZED       = 0x06ff,
    CENTAUR_INITIALIZED         = 0x07ff,
    SLAVE_OCC_INITIALIZED       = 0x08ff,
    WATCHDOG_INITIALIZED        = 0x09ff,
    RTL_TIMER_INITIALIZED       = 0x0aff,
    SEMS_AND_TIMERS_INITIALIZED = 0x0bff,
    APP_SEMS_CREATED            = 0x0c10,
    APP_MEM_MAPPED              = 0x0c20,
    APP_ADDR_INITIALIZED        = 0x0c30,
    APP_MEM_UNMAP               = 0x0c40,
    APPLETS_INITIALIZED         = 0x0cff,
    CMDH_THREAD_STARTED         = 0x0dff,
    INIT_OCB                    = 0x0e05,
    OCB_INITIALIZED             = 0x0e07,
    INIT_FSI_HOST_MBOX          = 0x0e10,
    FSI_HOST_MBOX_INITIALIZED   = 0x0e20,
    FSP_COMM_INITIALIZED        = 0x0eff,
    ABOUT_TO_HALT               = 0x0f00, // @sb002
    FIRST_FSP_ATTN_SENT         = 0xffff,
};

//Checkpoint flags (one byte bitmap)
enum
{
    CF_FSI_MB_TIMEOUT           = 0x01,
};

// @01a
#define DEFAULT_TRACE_SIZE 1536 //@gm006
//@pb003a - added defines
#define MAX_OCCS       8
#define MAX_CORES      12

//@pb007a - HALT with FIR bits set
// TRAP instruction should also set FIR bits along with halting PPC405
// Set DBCR0 to initial value (setting external debug event) so that
// trap call also sets FIR bits and also does not invoke program interrupt.
#define HALT_WITH_FIR_SET  mtspr(SPRN_DBCR0,PPC405_DBCR0_INITIAL);       \
                           asm volatile("trap")

// Static Assert Macro for Compile time assertions.
//   - This macro can be used both inside and outside of a function.
//   - A value of true will cause the ASSERT to produce this error
//   - This will show up on a compile fail as:
//      <file>:<line> error: size of array '_static_assert' is negative
//   - It would be trivial to use the macro to paste a more descriptive
//     array name for each assert, but we will leave it like this for now.
#define STATIC_ASSERT(cond) extern uint8_t _static_assert[cond ? -1 : 1]  __attribute__ ((unused));

// Convert duration based in SsxTimestamps to microseconds.
#define DURATION_IN_US_UNTIL_NOW_FROM(start_time) \
  (uint32_t) ((ssx_timebase_get() - (SsxTimebase) start_time) / ( SSX_TIMEBASE_FREQUENCY_HZ / 1000000 ))

// Convert duration based in SsxTimestamps to milliseconds.
#define DURATION_IN_MS_UNTIL_NOW_FROM(start_time) \
  (uint32_t) ((ssx_timebase_get() - (SsxTimebase) start_time) / ( SSX_TIMEBASE_FREQUENCY_HZ / 1000 ))

// @ai009c
// Skip this typedef in x86 environment
#ifndef OCC_X86_PARSER
//@pb000a - moved from common_types.h to here
typedef uint32_t      size_t ;
#endif


//*************************************************************************
// Structures
//*************************************************************************
//@pb000d - removed TPMD_code_header

//*************************************************************************
// Globals
//*************************************************************************
extern const char G_occ_buildname[16];

//*************************************************************************
// Function Prototypes
//*************************************************************************
int memcmp ( const void * ptr1, const void * ptr2, size_t num );

//*************************************************************************
// Functions
//*************************************************************************

#endif //_OCC_COMMON_H

