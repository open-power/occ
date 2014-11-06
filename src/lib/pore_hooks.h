#ifndef __PORE_HOOKS_H__
#define __PORE_HOOKS_H__

// $Id: pore_hooks.h,v 1.1.1.1 2013/12/11 20:49:20 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/lib/pore_hooks.h,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file pore_hooks.h
/// \brief Support for PORE hooks in Simics
///
/// Our Simics model of the PORE supports "hooks", that is, special forms of
/// comments that include C++ code that is extracted and made available at
/// simulation time in the Simics environment.
///
/// Besides hooks that do simple printf() type tracing, logging and tracing
/// hooks are also provided that make use of the Simics log-level facility.
/// This allows precise control over which PORE objects are logged/traced, and
/// at which level.
///
/// In the Simics environment, hook routines have the following prototype:
///
///     void f(const PoreAddress& i_address,
///            const HookType i_type,
///            PoreSimics& io_pore);

// Define the "1-liner" syntax

#define HOOK_MARKER HOOK_INSERT_MARKER(#,#)
#define HOOK_INSERT_MARKER(x,y) x##y##1@


/// \defgroup standard_io_hooks Standard I/O Logging and Tracing Hooks
///
/// Standard I/O printing. The *TRACE* forms prefix the output with the file
/// name and line number. 
///
/// @{

#define PORE_PRINTF(...) HOOK_MARKER printf(__VA_ARGS__);

#define PORE_FPRINTF(stream, ...) HOOK_MARKER fprintf(stream, __VA_ARGS__);

#define PORE_TRACEF(fmt, ...)                                                \
    HOOK_MARKER printf("%s:d:" fmt, __FILE__, __LINE__, ##__VA_ARGS__);

#define PORE_FTRACEF(stream, fmt, ...)                                       \
    HOOK_MARKER printf(stream, "%s:d:" fmt, __FILE__, __LINE__, ##__VA_ARGS__);

/// @}

/// \defgroup quickie_debugging_prints Quickie Debugging Print Hooks
///
/// Quickie debugging prints. You provide a register name and string (w/o
/// newline), the macro formats the data.
///
/// @{

#define PORE_PRINT_REG(msg, reg, fmt, fn)                        \
    PORE_PRINTF(msg " : " #reg " = " FMT_##fmt "\n", fn)

#define PORE_TRACE_REG(msg, reg, fmt, fn)                        \
    PORE_TRACEF(msg " : " #reg " = " FMT_##fmt "\n", fn)

#define PORE_PRINT_D0(msg)      PORE_PRINT_REG(msg, D0, DX, d0())
#define PORE_PRINT_D1(msg)      PORE_PRINT_REG(msg, D1, DX, d1())
#define PORE_PRINT_A0(msg)      PORE_PRINT_REG(msg, A0, AX, a0())
#define PORE_PRINT_A1(msg)      PORE_PRINT_REG(msg, A1, AX, a1())
#define PORE_PRINT_P0(msg)      PORE_PRINT_REG(msg, P0, PX, p0())
#define PORE_PRINT_P1(msg)      PORE_PRINT_REG(msg, P1, PX, p1())
#define PORE_PRINT_CTR(msg)     PORE_PRINT_REG(msg, CTR, CTR, ctr())
#define PORE_PRINT_SPRG0(msg)   PORE_PRINT_REG(msg, SPRG0, SPRG0, sprg0())
#define PORE_PRINT_STATUS(msg)  PORE_PRINT_REG(msg, STATUS, STATUS, status())
#define PORE_PRINT_CONTROL(msg) PORE_PRINT_REG(msg, CONTROL, CONTROL, control())

#define PORE_TRACE_D0(msg)      PORE_TRACE_REG(msg, D0, DX, d0())
#define PORE_TRACE_D1(msg)      PORE_TRACE_REG(msg, D1, DX, d1())
#define PORE_TRACE_A0(msg)      PORE_TRACE_REG(msg, A0, AX, a0())
#define PORE_TRACE_A1(msg)      PORE_TRACE_REG(msg, A1, AX, a1())
#define PORE_TRACE_P0(msg)      PORE_TRACE_REG(msg, P0, PX, p0())
#define PORE_TRACE_P1(msg)      PORE_TRACE_REG(msg, P1, PX, p1())
#define PORE_TRACE_CTR(msg)     PORE_TRACE_REG(msg, CTR, CTR, ctr())
#define PORE_TRACE_SPRG0(msg)   PORE_TRACE_REG(msg, SPRG0, SPRG0, sprg0())
#define PORE_TRACE_STATUS(msg)  PORE_TRACE_REG(msg, STATUS, STATUS, status())
#define PORE_TRACE_CONTROL(msg) PORE_TRACE_REG(msg, CONTROL, CONTROL, control())

/// @}

/// \defgroup simics_style_logging Simics-style Logging Hooks
///
/// Simics-style logging.  All of these will produce a Simics prefix detailing
/// the unit that failed. The *_TRACE_* forms add the file name and line number
/// to the Simics info, print a newline and then format the trace message on
/// the following line. 
///
/// @{

#define SIM_LOG_INFO(level, group, ...) HOOK_MARKER             \
    SIM_log_info(level, io_pore.d_log, group, __VA_ARGS__);

#define SIM_LOG_ERROR(group, ...) HOOK_MARKER           \
    SIM_log_error(io_pore.d_log, group, __VA_ARGS__);

#define SIM_TRACE_INFO(level, group, fmt, ...) HOOK_MARKER              \
    SIM_log_info(level, io_pore.d_log, group,                           \
                 "%s:%d\n" fmt, __FILE__, __LINE__,## __VA_ARGS__);

#define SIM_TRACE_ERROR(group, fmt, ...) HOOK_MARKER                    \
    SIM_log_error(io_pore.d_log, group,                                 \
                  "%s:%d\n" fmt, __FILE__, __LINE__, ##__VA_ARGS__);

/// @}

/// \defgroup vcl_style_3_level_printing VCL-style 3-Level Logging Hooks
///
/// Define VCL-style 3-level logging and tracing, with programmable Simics
/// log-level selection. All logs are controlled by (?) group 0. Note that
/// setting the Simics log-level to 4 produces gobs of output from every part
/// of the system, however here at the debug level of 3 we only get messages
/// from hooks.
///
/// @{

#ifndef SIMICS_LOG_LEVEL_OUTPUT
#define SIMICS_LOG_LEVEL_OUTPUT 1
#endif

#ifndef SIMICS_LOG_LEVEL_INFO
#define SIMICS_LOG_LEVEL_INFO 2
#endif

#ifndef SIMICS_LOG_LEVEL_DEBUG
#define SIMICS_LOG_LEVEL_DEBUG 3
#endif

#define PORE_LOG_OUTPUT(...) SIM_LOG_INFO(SIMICS_LOG_LEVEL_OUTPUT, 0, __VA_ARGS__)
#define PORE_LOG_INFO(...)   SIM_LOG_INFO(SIMICS_LOG_LEVEL_INFO, 0, __VA_ARGS__)
#define PORE_LOG_DEBUG(...)  SIM_LOG_INFO(SIMICS_LOG_LEVEL_DEBUG, 0, __VA_ARGS__)

#define PORE_LOG_ERROR(...)  SIM_LOG_ERROR(0, __VA_ARGS__)

#define PORE_TRACE_OUTPUT(...) SIM_TRACE_INFO(SIMICS_LOG_LEVEL_OUTPUT, 0, __VA_ARGS__)
#define PORE_TRACE_INFO(...)   SIM_TRACE_INFO(SIMICS_LOG_LEVEL_INFO, 0, __VA_ARGS__)
#define PORE_TRACE_DEBUG(...)  SIM_TRACE_INFO(SIMICS_LOG_LEVEL_DEBUG, 0, __VA_ARGS__)

#define PORE_TRACE_ERROR(...)  SIM_TRACE_ERROR(0, __VA_ARGS__)

/// @}

/// Break Simics simulation
#define SIM_BREAK_SIMULATION(msg) \
    HOOK_MARKER SIM_break_simulation(msg); io_pore.dumpAll();


/// A PORE Assertion
#define PORE_ASSERT(assertion)                                  \
    HOOK_MARKER                                                 \
    if (!(assertion)) {                                         \
        SIM_log_error(io_pore.d_log, 0,                         \
                      "Assertion below failed\n" #assertion);   \
        SIM_break_simulation("Assertion failure");              \
    }
    

/// Dump the PORE state
#define PORE_DUMP(...) LOG_OUTPUT(__VA_ARGS__) io_pore.dumpAll();

#endif // __PORE_HOOKS_H__
