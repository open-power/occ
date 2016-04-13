/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/pgp/pgp_trace.h $                                     */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2014,2016                        */
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
#ifndef __PGP_TRACE_H__
#define __PGP_TRACE_H__

/// \file pgp_trace.h
/// \brief Implementation of SSX_TRACE macros for PgP
///
/// Kernel and application tracing uses the PBA "OCI Marker Space", a 64KB OCI
/// register space implemented by PBA. When enabled in the PBA, any OCI write
/// transactions of any size are ACK'ed by the PBA and the data is ignored.
/// This creates an OCI transaction record that can be captured by the NHTM
/// for later analysis.
///
/// SSX provides a generic SSX_TRACE() macro that accepts a single
/// parameter. On PgP, valid parameters are integers in the range
/// 0x0,...,0xffff.  The PgP SSX kernel reserves trace addresses
/// 0xe000,...,0xffff for kernel event traces. Applications are free to use
/// the other trace addresses in the range 0x0000,...,0xdfff as they see fit.
///
/// Application tracing is globally disabled by default, and is enabled by
/// defining the switch SSX_TRACE_ENABLE=1.  Kernel event tracing is also
/// globally disabled by default, and is enabled by defining the switch
/// SSX_KERNEL_TRACE_ENABLE=1. Kernel event tracing adds overhead to every
/// interrupt handler and kernel API so should probably remain disabled unless
/// required.
///
/// The Simics PBA model supports trace reporting, either to stdout or to a
/// file. To enable trace reporting set pba->trace_report = 1. To capture
/// traces to a file other than stdout, set pba->trace_file = \<filename\>.  The
/// Simics model understands SSX kernel trace events and produces a readable
/// commentary of kernel traces events - user events will be reported simply
/// as the integer tag.

#include "pgp_common.h"

/// Output an OCI Trace Marker
///
/// See the comments for the file pgp_trace.h

#if SSX_TRACE_ENABLE
#define SSX_TRACE(event) out8(PBA_OCI_MARKER_BASE + (event), 0)
#endif

#if SSX_KERNEL_TRACE_ENABLE

// Note: The *BASE constants are defined in pgp_common.h

#define SSX_KERNEL_TRACE(event) out8(PBA_OCI_MARKER_BASE + (event), 0)

#define SSX_TRACE_THREAD_SLEEP(priority) \
    SSX_KERNEL_TRACE(SSX_TRACE_THREAD_SLEEP_BASE + priority)

#define SSX_TRACE_THREAD_WAKEUP(priority) \
    SSX_KERNEL_TRACE(SSX_TRACE_THREAD_WAKEUP_BASE + priority)

#define SSX_TRACE_THREAD_SEMAPHORE_PEND(priority) \
    SSX_KERNEL_TRACE(SSX_TRACE_THREAD_SEMAPHORE_PEND_BASE + priority)

#define SSX_TRACE_THREAD_SEMAPHORE_POST(priority) \
    SSX_KERNEL_TRACE(SSX_TRACE_THREAD_SEMAPHORE_POST_BASE + priority)

#define SSX_TRACE_THREAD_SEMAPHORE_TIMEOUT(priority) \
    SSX_KERNEL_TRACE(SSX_TRACE_THREAD_SEMAPHORE_TIMEOUT_BASE + priority)

#define SSX_TRACE_THREAD_SUSPENDED(priority) \
    SSX_KERNEL_TRACE(SSX_TRACE_THREAD_SUSPENDED_BASE + priority)

#define SSX_TRACE_THREAD_DELETED(priority) \
    SSX_KERNEL_TRACE(SSX_TRACE_THREAD_DELETED_BASE + priority)

#define SSX_TRACE_THREAD_COMPLETED(priority) \
    SSX_KERNEL_TRACE(SSX_TRACE_THREAD_COMPLETED_BASE + priority)

#define SSX_TRACE_THREAD_MAPPED_RUNNABLE(priority) \
    SSX_KERNEL_TRACE(SSX_TRACE_THREAD_MAPPED_RUNNABLE_BASE + priority)

#define SSX_TRACE_THREAD_MAPPED_SEMAPHORE_PEND(priority) \
    SSX_KERNEL_TRACE(SSX_TRACE_THREAD_MAPPED_SEMAPHORE_PEND_BASE + priority)

#define SSX_TRACE_THREAD_MAPPED_SLEEPING(priority) \
    SSX_KERNEL_TRACE(SSX_TRACE_THREAD_MAPPED_SLEEPING_BASE + priority)

#endif  /* SSX_KERNEL_TRACE_ENABLE */


#ifdef __ASSEMBLER__

// NB: CPP macros are not expanded as arguments to .if in GAS macro
// definitions.  That's why e.g. we have to use _liw instead of _liwa.

#if SSX_KERNEL_TRACE_ENABLE

        .macro  SSX_TRACE_CRITICAL_IRQ_ENTRY, irqreg, scratch
        _liw    \scratch, (PBA_OCI_MARKER_BASE + SSX_TRACE_CRITICAL_IRQ_ENTRY_BASE)
        stbx    \irqreg, \irqreg, \scratch
        eieio
        .endm

        .macro  SSX_TRACE_CRITICAL_IRQ_EXIT, scratch0, scratch1
        _liw    \scratch0, (PBA_OCI_MARKER_BASE + SSX_TRACE_CRITICAL_IRQ_EXIT_BASE)
        mfusprg0 \scratch1
        extrwi  \scratch1, \scratch1, 8, 16
        stbx    \scratch1, \scratch0, \scratch1
        eieio
        .endm

        .macro  SSX_TRACE_NONCRITICAL_IRQ_ENTRY, irqreg, scratch
        _liw    \scratch, (PBA_OCI_MARKER_BASE + SSX_TRACE_NONCRITICAL_IRQ_ENTRY_BASE)
        stbx    \irqreg, \irqreg, \scratch
        eieio
        .endm

        .macro  SSX_TRACE_NONCRITICAL_IRQ_EXIT, scratch0, scratch1
        _liw    \scratch0, (PBA_OCI_MARKER_BASE + SSX_TRACE_NONCRITICAL_IRQ_EXIT_BASE)
        mfusprg0 \scratch1
        extrwi  \scratch1, \scratch1, 8, 16
        stbx    \scratch1, \scratch0, \scratch1
        eieio
        .endm

        .macro  SSX_TRACE_THREAD_SWITCH, priority, scratch
        _liw    \scratch, (PBA_OCI_MARKER_BASE + SSX_TRACE_THREAD_SWITCH_BASE)
        stbx    \priority, \priority, \scratch
        eieio
        .endm

#else  /* SSX_KERNEL_TRACE_ENABLE */

        .macro  SSX_TRACE_CRITICAL_IRQ_ENTRY, irq, scratch
        .endm

        .macro  SSX_TRACE_CRITICAL_IRQ_EXIT, scratch0, scratch1
        .endm

        .macro  SSX_TRACE_NONCRITICAL_IRQ_ENTRY, irq, scratch
        .endm

        .macro  SSX_TRACE_NONCRITICAL_IRQ_EXIT, scratch0, scratch1
        .endm

        .macro  SSX_TRACE_THREAD_SWITCH, priority, scratch
        .endm

#endif  /* SSX_KERNEL_TRACE_ENABLE */
             
#endif  /* __ASSEMBLER__ */

#endif  /* __PGP_TRACE_H__ */
