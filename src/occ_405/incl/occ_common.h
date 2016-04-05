/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/incl/occ_common.h $                                   */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2015                        */
/* [+] International Business Machines Corp.                              */
/*                                                                        */
/*                                                                        */
/* Licensed under the Apache License, Version 2.0 (the "License");        */
/* you may not use this file except in compliance with the License.       */
/* You may obtain a copy of the License at                                */
/*                                                                        */
/*     http://www.apache.org/licenses/LICENSE-2.0                         */
/*                                                                        */
/* Unless required by applicable law or agreed to in writing, software    */
/* distributed under the License is distributed on an "AS IS" BASIS,      */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or        */
/* implied. See the License for the specific language governing           */
/* permissions and limitations under the License.                         */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */

#ifndef _OCC_COMMON_H
#define _OCC_COMMON_H

#include <common_types.h>
#include <comp_ids.h>

// From Linker Script
extern char _LINEAR_WR_WINDOW_SECTION_BASE[];
extern void _LINEAR_WR_WINDOW_SECTION_SIZE;
extern char _LINEAR_RD_WINDOW_SECTION_BASE[];
extern void _LINEAR_RD_WINDOW_SECTION_SIZE;
extern char _FIR_PARMS_SECTION_BASE[];
extern char _FIR_HEAP_SECTION_BASE[];

// Declare aligned data structures for Async access in a noncacheable section
//
// These macros declare aligned data structures in a noncacheable section, with
// the alignment that is needed by the specified device driver (or more
// accurately, as specified by the hardware device itself.)
//
// All buffers should be initialized - an initialization declaration using
// these macros would look as follows:
//    Example:  DMA_BUFFER(uint8_t g_bcue_test[1024]) = {0};
//
#define PBAX_BUFFER(declaration) \
        declaration __attribute__ ((__aligned__ (8))) __attribute__ ((section (".noncacheable")))

#define OCB_BUFFER(declaration) \
        declaration __attribute__ ((__aligned__ (8))) __attribute__ ((section (".noncacheable")))

#define DMA_BUFFER(declaration) \
        declaration __attribute__ ((__aligned__ (128))) __attribute__ ((section (".noncacheable")))

#define GPE_BUFFER(declaration) \
        declaration __attribute__ ((__aligned__ (8))) __attribute__ ((section (".noncacheable")))

#define FIR_HEAP_BUFFER(declaration) \
        declaration __attribute__ ((section (".firHeap")))

#define FIR_PARMS_BUFFER(declaration) \
        declaration __attribute__ ((section (".firParms")))

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
#define FIR_PARMS_SECTION_BASE_ADDRESS  ((uint32_t) &_FIR_PARMS_SECTION_BASE)
#define FIR_HEAP_SECTION_BASE_ADDRESS   ((uint32_t) &_FIR_HEAP_SECTION_BASE)


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

// CHECKPOINT macros revamped a little to allow a little more reuse with
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

// A new OCC_HALT macro much like SSX_PANIC but for OCC code use.  The
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

// Unique checkpoints
enum
{
    MAIN_STARTED                = 0x01ff,
    SSX_STARTING                = 0x0210,
    SSX_INITIALIZED             = 0x02ff,
    TRACE_INITIALIZED           = 0x0310,
    HOMER_ACCESS_INITS          = 0x0318,
    INITIALIZING_IRQS           = 0x0320,
    IRQS_INITIALIZED            = 0x032f,
    INITIALIZING_IPC            = 0x0330,
    IPC_INITIALIZED             = 0x033f,
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
    FSP_COMM_INITIALIZED        = 0x0eff,
    ABOUT_TO_HALT               = 0x0f00,
};

// Checkpoint flags (one byte bitmap)
enum
{
    CF_FSI_MB_TIMEOUT           = 0x01,
};

#define DEFAULT_TRACE_SIZE 1536
#define MAX_OCCS       8
#define MAX_CORES      24

//Used by G_occ_interrupt_type to distinguish between FSP supported OCCs and other servers.
#define FSP_SUPPORTED_OCC 0x00
#define PSIHB_INTERRUPT   0x01

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

// Skip this typedef in x86 environment
#ifndef OCC_X86_PARSER
typedef uint32_t      size_t ;
#endif

extern const char G_occ_buildname[16];

int memcmp ( const void * ptr1, const void * ptr2, size_t num );

#endif //_OCC_COMMON_H

